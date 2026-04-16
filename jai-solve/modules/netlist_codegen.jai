#import "Basic";
String :: #import "String";
Math :: #import "Math";

#import "nodal_analysis";

Element_Value :: struct {
    name: string;
    value: float;
};

parse_netlist :: (netlist_str: string) -> [] Element, [] string, [] Element_Value {
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

        tokens := String.split(line, #char " ");
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
                           values: [] Element_Value) -> string {
    context.print_style.default_format_float.mode = .SCIENTIFIC;
    context.print_style.default_format_float.zero_removal = .ONE_ZERO_AFTER_DECIMAL;

    write_value :: (name: string) -> string #expand {
        for val : values {
            if val.name == name return tprint("    float % = %f;\n", name, val.value);
        }
        return tprint("    float %;\n", name);
    }

    builder: String_Builder;
    append(*builder, "struct Params {\n");

    for el : elements {
        if el.type == {
            case .RESISTOR; #through;
            case .CAPACITOR; #through;
            case .INDUCTOR;
                append(*builder, write_value(el.name));
                // @TODO: else if params need to be extracted...
                // params := extract_param_names(el[3]);
                // for pname in params {
                //     append(&builder, write_value(pname));
                // }
        }
    }

    append(*builder, "};\n");
    return builder_to_string(*builder);
}

generate_state_struct :: (elements: [] Element) -> string, [] string {
    states: [..] string;

    builder: String_Builder;
    append(*builder, "struct State {\n");

    for el : elements {
        if el.type == {
            case .CAPACITOR; #through;
            case .INDUCTOR; #through;
            case .RC_SERIES; #through;
            case .RC_PARALLEL;
                array_add(*states, tprint("z%", el.name));
                append(*builder, tprint("    float % {};\n", states[states.count-1]));
        }
    }

    append(*builder, "};\n");
    return builder_to_string(*builder), states;
}

generate_impedance_defs :: (elements: [] Element) -> string {
//     def param_expr(expr, param_prefix="params."):
//         if isinstance(expr, str):
//             return f"{param_prefix}{expr}"
//         op = expr[0]
//         args = expr[1:]
//         if op == Combo_Op_Type.SUM:
//             # sum(a, b, c)
//             return f"sum({', '.join(param_expr(arg, param_prefix) for arg in args)})"
//         elif op == Combo_Op_Type.RECIP_SUM:
//             # recip_sum(a, b, c)
//             return f"recip_sum({', '.join(param_expr(arg, param_prefix) for arg in args)})"
//         else:
//             raise ValueError(f"Unknown operation: {op}")

    builder: String_Builder;
    append(*builder, "[[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };\n");
    append(*builder, "[[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };\n\n");

    for el : elements {
        val_name := tprint("params.%", el.name);

        // @TODO:
        // if len(el) > 3 and isinstance(el[3], tuple):
        //     val_name = name
        //     code += f"const auto {val_name} = {param_expr(el[3])};\n"

        if el.type == {
            case .RESISTOR;
                append(*builder, tprint("const auto g% = 1.0f / %;\n\n", el.name, val_name));
            case .CAPACITOR;
                append(*builder, tprint("const auto g% = 2.0f * sample_rate * %;\n\n", el.name, val_name));
            case .INDUCTOR;
                append(*builder, tprint("const auto g% = 1.0f / (2.0f * sample_rate * %);\n\n", el.name, val_name));
            case .RC_SERIES;
                // rc_internals = el[3]
                // res_name, cap_name = list(rc_internals.keys())[:2]
                // code += f"const auto {res_name} = {param_expr(rc_internals[res_name])};\n"
                // code += f"const auto {cap_name} = {param_expr(rc_internals[cap_name])};\n"
                // code += f"const auto g{name} = (2.0f * sample_rate * {cap_name}) / (1.0f + 2.0f * sample_rate * {res_name} * {cap_name});\n"
                // code += f"const auto gn{name} = g{name} * (2.0f * sample_rate * {res_name} * {cap_name} - 1.0f) / (2.0f * sample_rate * {cap_name});\n\n"
            case .RC_PARALLEL;
                // rc_internals = el[3]
                // res_name, cap_name = list(rc_internals.keys())[:2]
                // code += f"const auto {res_name} = {param_expr(rc_internals[res_name])};\n"
                // code += f"const auto {cap_name} = {param_expr(rc_internals[cap_name])};\n"
                // code += f"const auto g{name} = 2.0f * sample_rate * {cap_name} + (1.0f / {res_name});\n"
                // code += f"const auto gz{name} = 4.0f * sample_rate * {cap_name};\n\n"
        }
    }

    return builder_to_string(*builder);
}

netlist_codegen :: (netlist_str: string) -> string {
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
    loop_code := solve_and_codegen(elements, inputs, outputs);
    // print("%\n", loop_code);

    builder: String_Builder;
    append(*builder, "#pragma once\n\n");

    params_struct_code := generate_params_struct(elements, values);
    append(*builder, params_struct_code);
    append(*builder, "\n");

    state_struct_code, states := generate_state_struct(elements);
    append(*builder, state_struct_code);
    append(*builder, "\n");

    append(*builder, "static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)\n");
    append(*builder, "{\n");

    append(*builder, indent(generate_impedance_defs(elements)));
    // append(*builder, indent(outer_code)); @TODO

    append(*builder, "    for (int ch = 0; ch < num_channels; ++ch)\n");
    append(*builder, "    {\n");
    for state : states append(*builder, tprint("        auto %1 = state[ch].%1;\n", state));

    append(*builder, "        for (int n = 0; n < num_samples; ++n)\n");
    append(*builder, "        {\n");
    // @TODO: multiple inputs
    append(*builder, tprint("            const auto % = input[ch][n];\n\n", inputs[0]));

    append(*builder, tprint("%\n\n", indent(loop_code, "            ")));

    // @TODO: multiple outputs
    append(*builder, tprint("            output[ch][n] = %;\n\n", outputs[0]));
    append(*builder, "        }\n");

    for state : states append(*builder, tprint("        state[ch].%1 = %1;\n", state));
    append(*builder, "    }\n");

    append(*builder, "}\n");

    return builder_to_string(*builder);
}

    //     f.write(indent(inner_code, "            ") + "\n\n")
