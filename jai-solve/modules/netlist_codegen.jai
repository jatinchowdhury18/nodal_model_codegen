#import "Basic";
String :: #import "String";
Math :: #import "Math";

#import "nodal_analysis";

print_param_expr :: (expr: *Param_Expr, param_prefix: string = "params.") -> string {
    build_param_expr :: (e: *Param_Expr, param_prefix: string, builder: *String_Builder) {
        if e == null {
            append(builder, "/* null expr */");
            return;
        }
        if e.op == .NONE {
            print_to_builder(builder, "%1%2", param_prefix, e.leaf_name);
            return;
        }
        if e.op == .SUM  append(builder, "sum(");
        else             append(builder, "recip_sum(");
        build_param_expr(e.left, param_prefix, builder);
        append(builder, ", ");
        build_param_expr(e.right, param_prefix, builder);
        append(builder, ")");
    }

    builder: String_Builder;
    build_param_expr(expr, param_prefix, *builder);
    return builder_to_string(*builder);
}

// Walk a Param_Expr tree and collect all leaf names into `names`.
collect_param_leaf_names :: (expr: *Param_Expr, names: *[..] string) {
    if expr == null  return;
    if expr.op == .NONE {
        array_add_if_unique(names, expr.leaf_name);
        return;
    }
    collect_param_leaf_names(expr.left,  names);
    collect_param_leaf_names(expr.right, names);
}

Element_Value :: struct {
    name: string;
    value: float;
};

parse_netlist :: (netlist_str: string) -> [..] Element, [] string, [] Element_Value {
    parse_value :: (val_str: string) -> float32 {
        val, success, remainder := string_to_float(val_str);
        if !success return Math.FLOAT32_NAN;
        if remainder.count == 0 { return val; }
        if remainder[0] == {
            case #char "M";
                val *= 1.0e6;
            case #char "k";
                val *= 1.0e3;
            case #char "m";
                val *= 1.0e-3;
            case #char "u";
                val *= 1.0e-6;
            case #char "n";
                val *= 1.0e-9;
            case #char "p";
                val *= 1.0e-12;
        }
        return val;
    }

    elements: [..] Element;
    inputs: [..] string;
    values: [..] Element_Value;

    add_element :: (name: string, type: Element_Type, terminals: [] string) #expand {
        element := Element.{ name = name, type = type };
        array_copy(*element.terminals, terminals);
        array_add(*elements, element);
    }

    lines := String.split(netlist_str, "\n");
    for line_raw : lines {
        line := String.trim(line_raw);

        if line.count == 0 || line[0] == #char "*" || line[0] == #char "."  continue;

        if String.contains(line, "netlist_skip")  continue;

        tokens_raw := String.split(line, #char " ");
        tokens: [..] string;
        for t : tokens_raw  if t.count > 0  array_add(*tokens, t);
        // print("%\n", tokens);

        name := tokens[0];
        n1, n2 := tokens[1], tokens[2];
        value := parse_value(tokens[3]);
        if ! Math.is_nan_is_inf(value) array_add(*values, Element_Value.{ name, value });
        prefix := String.to_upper(name[0]);

        etype: Element_Type;
        if prefix == {
            case "R";
                etype = .RESISTOR;
            case "C";
                etype = .CAPACITOR;
            case "L";
                etype = .INDUCTOR;
            case "E";
                add_element(name, .AMP, .[tokens[1], tokens[4], tokens[3]]);
                continue;
            case "V";
                array_add(*inputs, n1); // treat voltage source as input node
                continue; // don't include in elements
            case;
                continue; // ignore unsupported parts
        }
        add_element(name, etype, .[n1, n2]);
    }

    return elements, inputs, values;
}

generate_params_struct :: (elements: [] Element,
                           values: [] Element_Value,
                           type_name: string = "float",
                           lang: string = "cpp") -> string {
    context.print_style.default_format_float.mode = .SCIENTIFIC;
    context.print_style.default_format_float.zero_removal = .ONE_ZERO_AFTER_DECIMAL;

    write_value :: (name: string) -> string #expand {
        if lang == "c" {
            return tprint("    % %;\n", type_name, name);
        }
        for val : values {
            if val.name == name return tprint("    % % = %f;\n", type_name, name, val.value);
        }
        return tprint("    % %;\n", type_name, name);
    }

    builder: String_Builder;
    if lang == "c"  append(*builder, "typedef struct {\n");
    else            append(*builder, "struct Params {\n");

    for el : elements {
        if el.type == {
            case .RESISTOR; #through;
            case .CAPACITOR; #through;
            case .INDUCTOR;
                if el.combo_expr == null {
                    append(*builder, write_value(el.name));
                } else {
                    leaf_names: [..] string;
                    collect_param_leaf_names(el.combo_expr, *leaf_names);
                    for pname : leaf_names  append(*builder, write_value(pname));
                }
            case .RC_SERIES; #through;
            case .RC_PARALLEL;
                leaf_names: [..] string;
                collect_param_leaf_names(el.rc_internal.res_expr, *leaf_names);
                collect_param_leaf_names(el.rc_internal.cap_expr, *leaf_names);
                for pname : leaf_names  append(*builder, write_value(pname));
        }
    }

    if lang == "c"  append(*builder, "} Params;\n");
    else            append(*builder, "};\n");
    return builder_to_string(*builder);
}

generate_state_struct :: (elements: [] Element, type_name: string = "float", lang: string = "cpp") -> string, [] string {
    states: [..] string;

    builder: String_Builder;
    if lang == "c"  append(*builder, "typedef struct {\n");
    else            append(*builder, "struct State {\n");

    for el : elements {
        if el.type == {
            case .CAPACITOR; #through;
            case .INDUCTOR; #through;
            case .RC_SERIES; #through;
            case .RC_PARALLEL;
                array_add(*states, tprint("z%", el.name));
                if lang == "c"  append(*builder, tprint("    % %;\n",      type_name, states[states.count-1]));
                else            append(*builder, tprint("    % % {};\n",   type_name, states[states.count-1]));
        }
    }

    if lang == "c"  append(*builder, "} State;\n");
    else            append(*builder, "};\n");
    return builder_to_string(*builder), states;
}

generate_impedance_defs :: (elements: [] Element, type_name: string = "float", lang: string = "cpp") -> string {
    builder: String_Builder;

    if lang == "c" {
        append(*builder, "#define sum(a, b) ((a) + (b))\n");
        append(*builder, "#define recip_sum(a, b) ((a) * (b) / ((a) + (b)))\n\n");
    } else {
        append(*builder, "[[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };\n");
        append(*builder, "[[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };\n\n");
    }

    auto_or_type := ifx lang == "c" then type_name else "auto";

    for el : elements {
        // For same-type combined elements (R+R, C+C, R||R, C||C), emit:
        //   const {auto_or_type} {name} = {combo_expr};
        // and then use {name} (not params.{name}) in the impedance formula.
        if el.combo_expr != null && (el.type == .RESISTOR || el.type == .CAPACITOR || el.type == .INDUCTOR) {
            append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, el.name, print_param_expr(el.combo_expr)));
        }

        val_name := ifx el.combo_expr != null then el.name else tprint("params.%", el.name);

        if el.type == {
            case .RESISTOR;
                append(*builder, tprint("const %1 g%2 = 1.0f / %3;\n\n", auto_or_type, el.name, val_name));
            case .CAPACITOR;
                append(*builder, tprint("const %1 g%2 = 2.0f * sample_rate * %3;\n\n", auto_or_type, el.name, val_name));
            case .INDUCTOR;
                append(*builder, tprint("const %1 g%2 = 1.0f / (2.0f * sample_rate * %3);\n\n", auto_or_type, el.name, val_name));
            case .RC_SERIES;
                res_name := el.rc_internal.res_name;
                cap_name := el.rc_internal.cap_name;
                append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, res_name, print_param_expr(el.rc_internal.res_expr)));
                append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, cap_name, print_param_expr(el.rc_internal.cap_expr)));
                append(*builder, tprint("const %1 g%2 = (2.0f * sample_rate * %3) / (1.0f + 2.0f * sample_rate * %4 * %3);\n",   auto_or_type, el.name, cap_name, res_name));
                append(*builder, tprint("const %1 gn%2 = g%2 * (2.0f * sample_rate * %3 * %4 - 1.0f) / (2.0f * sample_rate * %4);\n\n", auto_or_type, el.name, res_name, cap_name));
            case .RC_PARALLEL;
                res_name := el.rc_internal.res_name;
                cap_name := el.rc_internal.cap_name;
                append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, res_name, print_param_expr(el.rc_internal.res_expr)));
                append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, cap_name, print_param_expr(el.rc_internal.cap_expr)));
                append(*builder, tprint("const %1 g%2 = 2.0f * sample_rate * %3 + (1.0f / %4);\n", auto_or_type, el.name, cap_name, res_name));
                append(*builder, tprint("const %1 gz%2 = 4.0f * sample_rate * %3;\n\n", auto_or_type, el.name, cap_name));
        }
    }

    return builder_to_string(*builder);
}

netlist_codegen :: (netlist_str: string, type_name: string = "float", lang: string = "cpp") -> string {
    indent :: (in: string, prefix := "    ") -> string {
        ends_with_newline := in[in.count - 1] == #char "\n";
        out := String.replace(in, "\n", tprint("\n%", prefix));
        out = tprint("%1%2", prefix, out);
        if ends_with_newline  out.count -= prefix.count;
        return out;
    }

    elements, inputs, values := parse_netlist(netlist_str);
    // print("%\n", elements);
    // print("%\n", inputs);
    // print("%\n", values);

    outputs := string.["vo"];
    outer_code, loop_code := solve_and_codegen(*elements, inputs, outputs, .{ lang = lang, type_name = type_name });
    // print("%\n", outer_code);
    // print("%\n", loop_code);

    decl_type := ifx lang == "c" then type_name else "auto";

    builder: String_Builder;
    append(*builder, "#pragma once\n\n");

    params_struct_code := generate_params_struct(elements, values, type_name, lang);
    append(*builder, params_struct_code);
    append(*builder, "\n");

    state_struct_code, states := generate_state_struct(elements, type_name, lang);
    append(*builder, state_struct_code);
    append(*builder, "\n");

    append(*builder, "static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)\n");
    append(*builder, "{\n");

    append(*builder, indent(generate_impedance_defs(elements, type_name, lang)));
    if outer_code.count > 0  append(*builder, indent(outer_code));

    append(*builder, "    for (int ch = 0; ch < num_channels; ++ch)\n");
    append(*builder, "    {\n");
    for state : states  append(*builder, tprint("        %1 %2 = state[ch].%2;\n", decl_type, state));

    append(*builder, "        for (int n = 0; n < num_samples; ++n)\n");
    append(*builder, "        {\n");
    // @TODO: multiple inputs
    append(*builder, tprint("            const %1 %2 = input[ch][n];\n\n", decl_type, inputs[0]));

    append(*builder, tprint("%\n", indent(loop_code, "            ")));

    // @TODO: multiple outputs
    append(*builder, tprint("            output[ch][n] = %;\n", outputs[0]));
    append(*builder, "        }\n");

    for state : states append(*builder, tprint("        state[ch].%1 = %1;\n", state));
    append(*builder, "    }\n");

    append(*builder, "}\n");

    return builder_to_string(*builder);
}