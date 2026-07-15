#import "Basic";
String :: #import "String";
Math :: #import "Math";

#import "nodal_analysis";
#import "sym_algebra";
#import "nonlinear_solve";

// Thermal voltage kT/q at room temperature (~300K).
// For diodes/BJTs, the "vt" parameter is computed as
// VT_ROOM * N (for "ideality factor" N), since N isn't
// used anywhere else.
VT_ROOM :: 0.02585;

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

// A parsed SPICE `.model <name> <kind> [(key=val ...)]` card. Referenced by
// name from `D`/`Q` element lines. Unset parameters fall back to standard
// SPICE defaults (see parse_model_line / find_model).
Model_Def :: struct {
    name:      string;
    kind:      string; // "D", "NPN", "PNP" -- kept for diagnostics; PNP is not yet handled specially
    is_val:    float32;
    n_val:     float32; // diode-only; folded into vt_val at parse time
    betaf_val: float32;
    betar_val: float32;
}

// Parse a `.model` line (leading "." already stripped/trimmed by the
// caller), e.g. "model DCLIP D" or "model Q1M NPN (Is=2e-14 Bf=200 Br=4)".
// Missing parameters get standard SPICE defaults.
parse_model_line :: (line: string) -> Model_Def {
    tokens_raw := String.split(line, #char " ");
    tokens: [..] string;
    for t : tokens_raw  if t.count > 0  array_add(*tokens, t);

    model: Model_Def;
    model.is_val    = 1.0e-14;
    model.n_val     = 1.0;
    model.betaf_val = 100.0;
    model.betar_val = 1.0;

    if tokens.count < 3  return model; // malformed .model line; caller will get an empty-named model

    model.name = tokens[1];
    model.kind = String.to_upper_copy(tokens[2]);

    if tokens.count > 3 {
        joined := String.join(..array_view(tokens, 3, tokens.count - 3), separator = " ");
        joined, _ = String.replace(joined, "(", " ");
        joined, _ = String.replace(joined, ")", " ");
        joined, _ = String.replace(joined, ",", " ");
        kv_tokens_raw := String.split(joined, #char " ");
        for kv : kv_tokens_raw {
            if kv.count == 0  continue;
            parts := String.split(kv, #char "=");
            if parts.count != 2  continue;
            key := String.to_upper_copy(String.trim(parts[0]));
            val, success := string_to_float(String.trim(parts[1]));
            if !success  continue;
            if key == {
                case "IS"; model.is_val = val;
                case "N";  model.n_val = val;
                case "BF"; model.betaf_val = val;
                case "BR"; model.betar_val = val;
            }
        }
    }

    return model;
}

// Look up a parsed .model card by name, falling back to a default-parameter model.
find_model :: (model_defs: [] Model_Def, name: string) -> Model_Def {
    for m : model_defs  if m.name == name  return m;
    model: Model_Def;
    model.name      = name;
    model.is_val    = 1.0e-14;
    model.n_val     = 1.0;
    model.betaf_val = 100.0;
    model.betar_val = 1.0;
    return model;
}

// Register a (name, value) pair in `values` at most once -- multiple
// elements referencing the same .model card must not produce duplicate
// Params struct fields.
add_value_once :: (values: *[..] Element_Value, name: string, value: float32) {
    for v : values.*  if v.name == name  return;
    array_add(values, Element_Value.{ name = name, value = value });
}

Netlist_Info :: struct {
    elements : [..] Element;
    values : [..] Element_Value;

    inputs : [..] string;
    outputs : [..] string;

    // Newton-Raphson solver defaults.
    newton_tol_sq   := 1.0e-5;
    newton_max_iter := 20;
}

parse_netlist :: (netlist_str: string) -> Netlist_Info {
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

    using netlist_info : Netlist_Info;

    add_element :: (name: string, type: Element_Type, terminals: [] string, no_combine := false) #expand {
        element := Element.{ name = name, type = type, no_combine = no_combine };
        array_copy(*element.terminals, terminals);
        array_add(*elements, element);
    }

    lines := String.split(netlist_str, "\n");

    // Strip a ';'-style inline comment. Callers must do this AFTER checking for the "netlist_skip".
    strip_comment :: (line: string) -> string {
        parts := String.split(line, #char ";");
        return ifx parts.count > 0 then String.trim(parts[0]) else line;
    }

    // First pass: collect .model cards.
    model_defs: [..] Model_Def;
    for line_raw : lines {
        line := String.trim(line_raw);
        if line.count == 0  continue;
        if line[0] != #char "."  continue;
        line = String.trim(String.slice(line, 1, line.count - 1));
        if String.begins_with(line, "model") {
            array_add(*model_defs, parse_model_line(strip_comment(line)));
        }
    }

    for line_raw : lines {
        line := String.trim(line_raw);

        if line.count == 0  continue;
        if line[0] == #char "*" || line[0] == #char "." {
            // trim comment start char
            line = String.trim(String.slice(line, 1, line.count - 1));
            if String.begins_with(line, "output:") {
                output := String.trim(String.slice(line, 7, line.count - 7));
                print("Adding output node: %\n", output);
                array_add(*outputs, output);
            }
            // `nr_solve(max_iter=<int>, tol=<val>)`.
            if String.begins_with(line, "nr_solve") {
                rest := String.slice(line, 8, line.count - 8);
                rest, _ = String.replace(rest, "(", " ");
                rest, _ = String.replace(rest, ")", " ");
                rest, _ = String.replace(rest, ",", " ");
                kv_tokens_raw := String.split(rest, #char " ");
                for kv : kv_tokens_raw {
                    if kv.count == 0  continue;
                    parts := String.split(kv, #char "=");
                    if parts.count != 2  continue;
                    key := String.to_upper_copy(String.trim(parts[0]));
                    val, success := string_to_float(String.trim(parts[1]));
                    if !success  continue;
                    if key == {
                        case "TOL";      newton_tol_sq = val;
                        case "MAX_ITER"; newton_max_iter = cast(int) val;
                    }
                }
            }

            continue;
        }

        if String.contains(line, "netlist_skip")  continue;

        has_no_combine := String.contains(line, "no_combine");
        line = strip_comment(line);

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
                is_fixed := false;
                for token : tokens {
                    if token == "fixed" { is_fixed = true; break; }
                }
                // A `DC <value>` source is also a fixed/constant source and
                // needs its Params default recorded here.
                if tokens.count > 4 && String.to_upper_copy(tokens[3]) == "DC" {
                    is_fixed = true;
                    dc_value := parse_value(tokens[4]);
                    if !Math.is_nan_is_inf(dc_value)  add_value_once(*values, name, dc_value);
                }

                if is_fixed {
                    etype = .FIXED_VOLTAGE;
                } else {
                    array_add(*inputs, n1); // treat voltage source as input node
                    continue; // don't include in elements
                }
            case "D";
                // D<name> <anode> <cathode> <model>
                model := find_model(model_defs, tokens[3]);
                is_name := tprint("%_Is", model.name);
                vt_name := tprint("%_vt", model.name);
                add_value_once(*values, is_name, model.is_val);
                add_value_once(*values, vt_name, cast(float32)(model.n_val * VT_ROOM));

                element := Element.{ name = name, type = .DIODE, no_combine = has_no_combine };
                array_add(*element.terminals, n1); // anode
                array_add(*element.terminals, n2); // cathode
                element.nonlinear = .{ is_name = is_name, vt_name = vt_name };
                array_add(*elements, element);
                continue;
            case "Q";
                // Q<name> <collector> <base> <emitter> <model>.
                emitter    := tokens[3];
                model      := find_model(model_defs, tokens[4]);
                is_name    := tprint("%_Is", model.name);
                vt_name    := tprint("%_vt", model.name);
                betaf_name := tprint("%_BetaF", model.name);
                betar_name := tprint("%_BetaR", model.name);
                add_value_once(*values, is_name, model.is_val);
                add_value_once(*values, vt_name, cast(float32)(model.n_val * VT_ROOM));
                add_value_once(*values, betaf_name, model.betaf_val);
                add_value_once(*values, betar_name, model.betar_val);

                element := Element.{ name = name, type = .BJT };
                array_add(*element.terminals, n1);      // collector
                array_add(*element.terminals, n2);      // base
                array_add(*element.terminals, emitter); // emitter
                element.nonlinear = .{ is_name = is_name, vt_name = vt_name, betaf_name = betaf_name, betar_name = betar_name };
                array_add(*elements, element);
                continue;
            case;
                continue; // ignore unsupported parts
        }
        add_element(name, etype, .[n1, n2], has_no_combine);
    }

    return netlist_info;
}

generate_params_struct :: (elements: [] Element,
                           values: [] Element_Value,
                           type_name: string = "float",
                           lang: string = "cpp") -> string {
    context.print_style.default_format_float.mode = .SCIENTIFIC;
    context.print_style.default_format_float.zero_removal = .ONE_ZERO_AFTER_DECIMAL;

    suffix := ifx type_name == "float" then "f" else ""; // only "float" gets literal f-suffixes; "double" etc. get bare literals
    write_value :: (name: string) -> string #expand {
        for val : values {
            if val.name == name {
                if lang == "c"  return tprint("    % %; // = %\n", type_name, name, val.value);
                          else  return tprint("    %1 %2 = %3%4;\n", type_name, name, val.value, suffix);
            }
        }
        return tprint("    % %;\n", type_name, name);
    }

    builder: String_Builder;
    if lang == "c"  append(*builder, "typedef struct {\n");
    else            append(*builder, "struct Params {\n");

    // DIODE/DIODE_PAIR/BJT parameters belong to a .model card, which may be
    // shared by several elements, so we guard against emitting the same Params
    // field twice.
    emitted_nonlinear_names: [..] string;
    emit_nonlinear_param :: (pname: string) #expand {
        if array_find(emitted_nonlinear_names, pname)  return;
        array_add(*emitted_nonlinear_names, pname);
        append(*builder, write_value(pname));
    }

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
            case .FIXED_VOLTAGE;
                append(*builder, write_value(el.name));
            case .DIODE; #through;
            case .DIODE_PAIR;
                emit_nonlinear_param(el.nonlinear.is_name);
                emit_nonlinear_param(el.nonlinear.vt_name);
            case .BJT;
                emit_nonlinear_param(el.nonlinear.is_name);
                emit_nonlinear_param(el.nonlinear.vt_name);
                emit_nonlinear_param(el.nonlinear.betaf_name);
                emit_nonlinear_param(el.nonlinear.betar_name);
        }
    }

    if lang == "c"  append(*builder, "} Params;\n");
    else            append(*builder, "};\n");
    return builder_to_string(*builder);
}

// `extra_state_names` holds nonlinear-cluster junction-voltage unknowns
// (Nonlinear_Cluster.unknowns[].name), which persist across samples as the
// per-sample Newton-Raphson warm start.
generate_state_struct :: (elements: [] Element, extra_state_names: [] string, type_name: string = "float", lang: string = "cpp") -> string, [] string {
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

    for name : extra_state_names {
        array_add(*states, name);
        if lang == "c"  append(*builder, tprint("    % %;\n",    type_name, name));
        else            append(*builder, tprint("    % % {};\n", type_name, name));
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
    suffix       := ifx type_name == "float" then "f" else ""; // only "float" gets literal f-suffixes; "double" etc. get bare literals

    // DIODE/DIODE_PAIR/BJT I-V-law expressions (built in nonlinear_solve.jai)
    // reference Is/vt/BetaF/BetaR as bare symbol names (e.g. "D1_Is").
    emitted_nonlinear_aliases: [..] string;
    emit_nonlinear_alias :: (pname: string) #expand {
        if pname.count == 0  return; // BJT-only fields left blank on DIODE/DIODE_PAIR elements
        if array_find(emitted_nonlinear_aliases, pname)  return;
        array_add(*emitted_nonlinear_aliases, pname);
        append(*builder, tprint("const %1 %2 = params.%2;\n", auto_or_type, pname));
    }

    for el : elements {
        if el.type == .DIODE || el.type == .DIODE_PAIR || el.type == .BJT {
            emit_nonlinear_alias(el.nonlinear.is_name);
            emit_nonlinear_alias(el.nonlinear.vt_name);
            emit_nonlinear_alias(el.nonlinear.betaf_name);
            emit_nonlinear_alias(el.nonlinear.betar_name);
        }
    }
    if emitted_nonlinear_aliases.count > 0  append(*builder, "\n");

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
                append(*builder, tprint("const %1 g%2 = 1.0%4 / %3;\n\n", auto_or_type, el.name, val_name, suffix));
            case .CAPACITOR;
                append(*builder, tprint("const %1 g%2 = 2.0%4 * sample_rate * %3;\n\n", auto_or_type, el.name, val_name, suffix));
            case .INDUCTOR;
                append(*builder, tprint("const %1 g%2 = 1.0%4 / (2.0%4 * sample_rate * %3);\n\n", auto_or_type, el.name, val_name, suffix));
            case .RC_SERIES;
                res_name := el.rc_internal.res_name;
                cap_name := el.rc_internal.cap_name;
                append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, res_name, print_param_expr(el.rc_internal.res_expr)));
                append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, cap_name, print_param_expr(el.rc_internal.cap_expr)));
                append(*builder, tprint("const %1 g%2 = (2.0%5 * sample_rate * %3) / (1.0%5 + 2.0%5 * sample_rate * %4 * %3);\n",   auto_or_type, el.name, cap_name, res_name, suffix));
                append(*builder, tprint("const %1 gn%2 = g%2 * (2.0%5 * sample_rate * %3 * %4 - 1.0%5) / (2.0%5 * sample_rate * %4);\n\n", auto_or_type, el.name, res_name, cap_name, suffix));
            case .RC_PARALLEL;
                res_name := el.rc_internal.res_name;
                cap_name := el.rc_internal.cap_name;
                append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, res_name, print_param_expr(el.rc_internal.res_expr)));
                append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, cap_name, print_param_expr(el.rc_internal.cap_expr)));
                append(*builder, tprint("const %1 g%2 = 2.0%5 * sample_rate * %3 + (1.0%5 / %4);\n", auto_or_type, el.name, cap_name, res_name, suffix));
                append(*builder, tprint("const %1 gz%2 = 4.0%4 * sample_rate * %3;\n\n", auto_or_type, el.name, cap_name, suffix));
            case .FIXED_VOLTAGE;
                // Define the parameter value as a named constant, then alias the
                // (possibly renamed) node to it.  Both names may appear in generated
                // expressions: the param name comes from the equation solve RHS, while
                // the node name comes from element's terminal references.
                param_name := el.name;
                node_name  := el.terminals[0];
                append(*builder, tprint("const %1 %2 = params.%2;\n\n", auto_or_type, param_name));
                if node_name != param_name {
                    append(*builder, tprint("const %1 %2 = %3;\n\n", auto_or_type, node_name, param_name));
                }
        }
    }

    return builder_to_string(*builder);
}

// SPICE-style Newton-Raphson junction-voltage step limiter, mirroring
// ngspice's DEVpnjlim: bounds the proposed new junction voltage so that a
// subsequent exp(v/vt) evaluation can never overflow, by capping the
// per-iteration step once v exceeds vcrit. Emitted once per header..
generate_junction_limiter_helper :: (type_name: string, lang: string) -> string {
    if lang == "c" {
        template := #string DONE
static % limit_junction_voltage(% v_new, % v_old, % vt, % vcrit) {
    if (v_new > vcrit && fabs(v_new - v_old) > 2 * vt) {
        if (v_old > 0) {
            % arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * log(arg) : vcrit;
        } else {
            v_new = vt * log(v_new / vt);
        }
    }
    return v_new;
}
DONE
        return tprint(template, type_name, type_name, type_name, type_name, type_name, type_name);
    }

    return #string DONE
[[maybe_unused]] static auto limit_junction_voltage = [](auto v_new, auto v_old, auto vt, auto vcrit)
{
    if (v_new > vcrit && std::abs(v_new - v_old) > 2 * vt)
    {
        if (v_old > 0)
        {
            const auto arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * std::log(arg) : vcrit;
        }
        else
        {
            v_new = vt * std::log(v_new / vt);
        }
    }
    return v_new;
};
DONE;
}

// vcrit = vt * log(vt / (sqrt(2) * Is)) per junction-voltage unknown, derived
// once from Params fields.
generate_vcrit_defs :: (clusters: [] Nonlinear_Cluster, type_name: string, lang: string) -> string {
    auto_or_type := ifx lang == "c" then type_name else "auto";
    log_fn       := ifx lang == "c" then "log" else "std::log";
    sqrt_fn      := ifx lang == "c" then "sqrt" else "std::sqrt";

    builder: String_Builder;
    emitted: [..] string;
    for c : clusters {
        for u : c.unknowns {
            if array_find(emitted, u.vt_name)  continue;
            array_add(*emitted, u.vt_name);
            print_to_builder(*builder, "const %1 vcrit_%2 = %2 * %3(%2 / (%4(2.0) * %5));\n",
                             auto_or_type, u.vt_name, log_fn, sqrt_fn, u.is_name);
        }
    }
    if emitted.count > 0  append(*builder, "\n");
    return builder_to_string(*builder);
}

// Newton-Raphson convergence tolerance (volts^2) and max iteration count,
// defined once and referenced by every cluster's solve below.
generate_newton_constants :: (newton_tol_sq: float, newton_max_iter: int, type_name: string, lang: string) -> string {
    decl_type := ifx lang == "c" then type_name else "auto";
    const_kw  := ifx lang == "c" then "static const" else "static constexpr";
    builder: String_Builder;
    print_to_builder(*builder, "%1 %2 newton_tol_sq = %3;\n", const_kw, decl_type, newton_tol_sq);
    print_to_builder(*builder, "%1 int newton_max_iter = %2;\n\n", const_kw, newton_max_iter);
    return builder_to_string(*builder);
}

// Newton-Raphson solve for one cluster, to be inserted inside the per-sample
// loop. Iteration count/tolerance come from the shared `newton_max_iter` /
// `newton_tol_sq` constants (see generate_newton_constants), not literals.
generate_cluster_newton_code :: (cluster: Nonlinear_Cluster, exprs: [] Expr, options: Codegen_Options) -> string {
    builder: String_Builder;
    print_to_builder(*builder, "// --- Newton-Raphson solve: %\n", cluster.name);

    emit_topological_order(cluster.invariant_items, exprs, *builder, options);

    append(*builder, "for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)\n{\n");
    inner_builder: String_Builder;
    emit_topological_order(cluster.iteration_items, exprs, *inner_builder, options);

    decl_type := ifx options.lang == "c" then options.type_name else "auto";
    print_to_builder(*inner_builder, "\n%1 residual_norm_sq = 0;\n", decl_type);
    for u : cluster.unknowns  print_to_builder(*inner_builder, "residual_norm_sq += %1 * %1;\n", u.residual_name);
    print_to_builder(*inner_builder, "%1 step_norm_sq = 0;\n", decl_type);
    for u : cluster.unknowns  print_to_builder(*inner_builder, "step_norm_sq += %1 * %1;\n", u.delta_name);
    append(*inner_builder, "\n");
    for u : cluster.unknowns {
        print_to_builder(*inner_builder, "%1 = limit_junction_voltage(%1 + %2, %1, %3, vcrit_%3);\n",
                         u.name, u.delta_name, u.vt_name);
    }
    append(*inner_builder, "\nif (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)\n    break;\n");

    inner_text := builder_to_string(*inner_builder);
    indented := String.replace(inner_text, "\n", "\n    ");
    indented, _ = String.replace(indented, "\n    \n", "\n\n"); // don't indent blank lines
    append(*builder, "    ");
    append(*builder, indented);
    append(*builder, "\n}\n");

    return builder_to_string(*builder);
}

// Builds the DC-equivalent circuit (see generate_kcl_equations(dc_mode=true))
// over its own Expr array, solves it, and additionally derives the
// steady-state z-value for each reactive element.
build_dc_operating_point :: (elements: [] Element, inputs: [] string) -> (clusters: [] Nonlinear_Cluster, exprs: [..] Expr, z_init_vars: [] Gen_Var) {
    dc_solve_vars, dc_equations, dc_exprs := generate_kcl_equations(elements, inputs, true);
    dc_solutions := solve(dc_equations, dc_solve_vars, *dc_exprs);

    dc_clusters := build_nonlinear_clusters(elements, dc_solve_vars, dc_solutions, *dc_exprs);

    z_init_vars: [..] Gen_Var;
    for el : elements {
        if el.type != .CAPACITOR && el.type != .RC_SERIES && el.type != .RC_PARALLEL  continue;

        // copy_expr: pos_v/neg_v can alias a solution node shared by another
        // reactive element on the same terminal -- simplify() below mutates
        // in place, so a bare lookup would corrupt it for a later iteration.
        pos_v := copy_expr(lookup_node_voltage(el.terminals[0], dc_solve_vars, dc_solutions, *dc_exprs, inputs), *dc_exprs);
        neg_v := copy_expr(lookup_node_voltage(el.terminals[1], dc_solve_vars, dc_solutions, *dc_exprs, inputs), *dc_exprs);

        z_expr: Expr_ID;
        if el.type == .RC_PARALLEL {
            // z tracks only the capacitor half; gz is already 2x that conductance.
            gz_sym := expr_symbol(tprint("gz%", el.name), *dc_exprs);
            z_expr = expr_div(expr_mul(gz_sym, expr_sub(pos_v, neg_v, *dc_exprs), *dc_exprs), expr_const(2, *dc_exprs), *dc_exprs);
        } else {
            g_sym := expr_symbol(tprint("g%", el.name), *dc_exprs);
            z_expr = expr_mul(g_sym, expr_sub(pos_v, neg_v, *dc_exprs), *dc_exprs);
        }
        z_expr = simplify(z_expr, *dc_exprs);
        array_add(*z_init_vars, Gen_Var.{ name = tprint("z%", el.name), expr_id = z_expr });
    }

    return dc_clusters, dc_exprs, z_init_vars;
}

// Emits a `reset()` function that solves the circuit's DC operating point
// analytically and writes the result directly into every channel's State.
// Each circuit input gets its own `<input>_dc` parameter (default 0.0f in C++)
// giving the constant value that input is assumed to be held at for DC analysis
// purposes. Since the DC operating point doesn't depend on channel index,
// it's solved once and copied to all `num_channels` entries of `state`.
generate_reset_function :: (elements: [] Element, inputs: [] string, lang: string, type_name: string) -> string {
    indent :: (in: string, prefix := "    ") -> string {
        if in.count == 0  return in;
        ends_with_newline := in[in.count - 1] == #char "\n";
        out := String.replace(in, "\n", tprint("\n%", prefix));
        out = tprint("%1%2", prefix, out);
        if ends_with_newline  out.count -= prefix.count;
        return out;
    }

    dc_clusters, dc_exprs, z_init_vars := build_dc_operating_point(elements, inputs);
    decl_type := ifx lang == "c" then type_name else "auto";

    builder: String_Builder;
    append(*builder, "static void reset (Params params, State* state, int num_channels, float sample_rate");
    for input : inputs {
        if lang == "c"  print_to_builder(*builder, ", float %1_dc", input);
        else            print_to_builder(*builder, ", float %1_dc = 0.0f", input);
    }
    append(*builder, ")\n{\n");

    append(*builder, indent(generate_impedance_defs(elements, type_name, lang)));
    if dc_clusters.count > 0  append(*builder, indent(generate_vcrit_defs(dc_clusters, type_name, lang)));
    for input : inputs  print_to_builder(*builder, "    const %1 %2 = %2_dc;\n", decl_type, input);
    if inputs.count > 0  append(*builder, "\n");

    // Newton unknowns start at 0 (cold-start guess).
    for c : dc_clusters  for u : c.unknowns  print_to_builder(*builder, "    %1 %2 = 0;\n", type_name, u.name);
    if dc_clusters.count > 0  append(*builder, "\n");

    for c : dc_clusters {
        newton_code := generate_cluster_newton_code(c, dc_exprs, .{ lang = lang, type_name = type_name });
        append(*builder, indent(newton_code));
    }

    for zv : z_init_vars {
        print_to_builder(*builder, "    const %1 %2 = %3;\n", decl_type, zv.name, print_expr(zv.expr_id, dc_exprs));
    }
    if z_init_vars.count > 0  append(*builder, "\n");

    append(*builder, "    for (int ch = 0; ch < num_channels; ++ch)\n    {\n");
    for c : dc_clusters  for u : c.unknowns  print_to_builder(*builder, "        state[ch].%1 = %1;\n", u.name);
    for zv : z_init_vars  print_to_builder(*builder, "        state[ch].%1 = %1;\n", zv.name);
    append(*builder, "    }\n");

    append(*builder, "}\n");
    return builder_to_string(*builder);
}

netlist_codegen :: (netlist_str: string, type_name: string = "float", lang: string = "cpp", namespace: string = "") -> string {
    indent :: (in: string, prefix := "    ") -> string {
        ends_with_newline := in[in.count - 1] == #char "\n";
        out := String.replace(in, "\n", tprint("\n%", prefix));
        out = tprint("%1%2", prefix, out);
        if ends_with_newline  out.count -= prefix.count;
        return out;
    }

    // elements, inputs, outputs, values, newton_tol_sq, newton_max_iter := parse_netlist(netlist_str);
    net := parse_netlist(netlist_str);
    // print("%\n", net.elements);
    // print("%\n", net.inputs);
    // print("%\n", net.values);

    if net.outputs.count == 0  array_add(*net.outputs, "vo");

    has_nonlinear := false;
    for el : net.elements  if el.type == .DIODE || el.type == .DIODE_PAIR || el.type == .BJT { has_nonlinear = true; break; }

    outer_code: string;
    loop_code:  string;
    outer_flops, inner_flops: Flop_Count;
    clusters: [] Nonlinear_Cluster;
    cluster_exprs: [] Expr; // only valid/used when has_nonlinear

    if has_nonlinear {
        // Inlined equivalent of solve_and_codegen(), with build_nonlinear_clusters()
        // spliced in between solve() and generate_expressions() -- see
        // nonlinear_solve.jai's module comment for why this ordering matters.
        reduce_circuit(*net.elements, net.inputs, net.outputs);
        solve_vars, equations, exprs := generate_kcl_equations(net.elements, net.inputs);
        solutions := solve(equations, solve_vars, *exprs);

        clusters = build_nonlinear_clusters(net.elements, solve_vars, solutions, *exprs);

        // Junction-voltage unknowns (vD1, vBCQ1, ...) only appear in the tree
        // *after* nonlinear_solve.jai substitutes them in for branch-current
        // symbols.
        cluster_variant_names: [..] string;
        for c : clusters  for u : c.unknowns  array_add(*cluster_variant_names, u.name);

        gen_vars := generate_expressions(net.elements, net.outputs, solve_vars, solutions, *exprs);

        gen_var_ids: [..] Expr_ID;
        for gv : gen_vars  array_add(*gen_var_ids, gv.expr_id);

        cse_vars := cse(gen_var_ids, *exprs);
        sort_cse_topological(*cse_vars, exprs);

        licm_variant_base := build_variant_base(net.elements, net.inputs);
        for gv : gen_vars  array_add(*licm_variant_base, gv.name);
        for n : cluster_variant_names  array_add(*licm_variant_base, n);
        licm(gen_var_ids, *cse_vars, *exprs, licm_variant_base);
        recip_hoist(gen_var_ids, *cse_vars, *exprs, licm_variant_base);

        outer_code, loop_code, outer_flops, inner_flops =
            codegen(net.elements, net.inputs, gen_vars, cse_vars, exprs, .{ lang = lang, type_name = type_name }, cluster_variant_names);
        cluster_exprs = exprs;
    } else {
        outer_code, loop_code, outer_flops, inner_flops =
            solve_and_codegen(*net.elements, net.inputs, net.outputs, .{ lang = lang, type_name = type_name });
    }
    // print("%\n", outer_code);
    // print("%\n", loop_code);

    flop_str :: (f: Flop_Count) -> string #expand {
        b: String_Builder;
        sep := "";
        if f.muls > 0 { append(*b, sep); print_to_builder(*b, "% mul", f.muls); sep = ", "; }
        if f.divs > 0 { append(*b, sep); print_to_builder(*b, "% div", f.divs); sep = ", "; }
        if f.adds > 0 { append(*b, sep); print_to_builder(*b, "% add", f.adds); sep = ", "; }
        total := f.muls + f.divs + f.adds;
        if total == 0  return "0 ops";
        return tprint("% ops (%)", total, builder_to_string(*b));
    }
    print("  Outer loop: %\n  Inner loop: %\n", flop_str(outer_flops), flop_str(inner_flops));

    decl_type := ifx lang == "c" then type_name else "auto";

    builder: String_Builder;
    append(*builder, "#pragma once\n\n");
    if has_nonlinear {
        append(*builder, ifx lang == "c" then "#include <math.h>\n\n" else "#include <cmath>\n\n");
    }

    if namespace.count > 0 && lang == "cpp" {
        append(*builder, tprint("namespace %\n{\n\n", namespace));
    }

    params_struct_code := generate_params_struct(net.elements, net.values, type_name, lang);
    append(*builder, params_struct_code);
    append(*builder, "\n");

    extra_state_names: [..] string;
    for c : clusters  for u : c.unknowns  array_add(*extra_state_names, u.name);
    state_struct_code, states := generate_state_struct(net.elements, extra_state_names, type_name, lang);
    append(*builder, state_struct_code);
    append(*builder, "\n");

    if has_nonlinear {
        append(*builder, generate_junction_limiter_helper(type_name, lang));
        append(*builder, "\n");
        append(*builder, generate_newton_constants(net.newton_tol_sq, net.newton_max_iter, type_name, lang));
    }

    append(*builder, "static void compute (");
    if net.inputs.count == 1 {
        append(*builder, "const float* const* input, ");
    } else {
        for input : net.inputs  append(*builder, tprint("const float* const* input_%, ", input));
    }
    if net.outputs.count == 1 {
        append(*builder, "float** output, ");
    } else {
        for output : net.outputs  append(*builder, tprint("float** output_%, ", output));
    }
    append(*builder, "int num_channels, int num_samples, Params params, State* state, float sample_rate)\n");
    append(*builder, "{\n");

    append(*builder, indent(generate_impedance_defs(net.elements, type_name, lang)));
    if has_nonlinear  append(*builder, indent(generate_vcrit_defs(clusters, type_name, lang)));
    if outer_code.count > 0  append(*builder, indent(outer_code));

    append(*builder, "    for (int ch = 0; ch < num_channels; ++ch)\n");
    append(*builder, "    {\n");
    for state : states  append(*builder, tprint("        %1 %2 = state[ch].%2;\n", decl_type, state));

    append(*builder, "        for (int n = 0; n < num_samples; ++n)\n");
    append(*builder, "        {\n");

    if net.inputs.count == 1 {
        append(*builder, tprint("            const %1 %2 = input[ch][n];\n\n", decl_type, net.inputs[0]));
    } else {
        for input : net.inputs {
            append(*builder, tprint("            const %1 %2 = input_%2[ch][n];\n", decl_type, input));
        }
        append(*builder, "\n");
    }

    if has_nonlinear {
        for c : clusters {
            newton_code := generate_cluster_newton_code(c, cluster_exprs, .{ lang = lang, type_name = type_name });
            append(*builder, tprint("%\n", indent(newton_code, "            ")));
        }
    }

    append(*builder, tprint("%\n", indent(loop_code, "            ")));

    if net.outputs.count == 1 {
        append(*builder, tprint("            output[ch][n] = %;\n", net.outputs[0]));
    } else {
        for output : net.outputs {
            append(*builder, tprint("            output_%1[ch][n] = %1;\n", output));
        }
    }
    append(*builder, "        }\n");

    for state : states append(*builder, tprint("        state[ch].%1 = %1;\n", state));
    append(*builder, "    }\n");

    append(*builder, "}\n");

    if has_nonlinear {
        append(*builder, "\n");
        append(*builder, generate_reset_function(net.elements, net.inputs, lang, type_name));
    }

    if namespace.count > 0 && lang == "cpp" {
        append(*builder, tprint("\n} // namespace %\n", namespace));
    }

    return builder_to_string(*builder);
}
