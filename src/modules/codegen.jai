// Turns a solved circuit (Element list + closed-form MNA/Newton
// expressions) into C/C++ text: FLOP counting, every `generate_*` string
// emitter (Params/State structs, impedance defs, limiter helpers,
// vcrit/newton constants, per-cluster Newton code, reset()), and the
// pre-emission orchestration (CSE-temp classification, topological
// emission).
//
// Sits above nonlinear_solve.jai in the dependency DAG (sym_algebra <-
// nodal_analysis <- nonlinear_solve <- codegen <- netlist_codegen).

#import "Basic";
#import "Hash_Table";

#import "backend";
#import "transcendental_approx";
#import "sym_algebra";
#import "nodal_analysis";
#import "nonlinear_solve";

// Recursively walk an expression tree and accumulate op counts into `result`.
// Stops at SYMBOL and CONST leaves; does not cross into sub-expressions that
// are already named CSE variables (they appear as SYMBOL nodes).
// Approximated transcendentals are broken down into a add/mul cost,
// exact transcendentals are recorded as "transcendentals".
count_flops_expr :: (e: Expr_ID, exprs: [] Expr, result: *Flop_Count, approx: Transcendental_Approx) {
    if e == NO_EXPR  return;
    kind := exprs[e].kind;
    if      kind == .MUL                                 result.muls += 1;
    else if kind == .DIV                                 result.divs += 1;
    else if kind == .ADD || kind == .SUB || kind == .NEG  result.adds += 1;
    else if kind == .EXP {
        if approx.exact  result.transcendentals += 1;
        else  add_flop_count(result, exp_approx_flop_count(approx));
    } else if kind == .LOG {
        if approx.exact  result.transcendentals += 1;
        else  add_flop_count(result, log_approx_flop_count(approx));
    } else if kind == .POW {
        if approx.exact  result.transcendentals += 1;
        else  add_flop_count(result, pow_approx_flop_count(approx));
    }

    if kind == .ADD || kind == .SUB || kind == .MUL || kind == .DIV || kind == .POW {
        count_flops_expr(exprs[e].left,  exprs, result, approx);
        count_flops_expr(exprs[e].right, exprs, result, approx);
    } else if kind == .NEG || kind == .EXP || kind == .LOG {
        count_flops_expr(exprs[e].left, exprs, result, approx);
    }
}

count_flops :: (items: [] Gen_Var, exprs: [] Expr, approx: Transcendental_Approx = .{}) -> Flop_Count {
    result: Flop_Count;
    for item : items  count_flops_expr(item.expr_id, exprs, *result, approx);
    return result;
}

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

// unit_name suffixes the struct name, used for sub-circuit units (Params_shelf, ...).
generate_params_struct :: (elements: [] Element,
                           values: [] Element_Value,
                           backend: Backend = .{},
                           unit_name := "") -> string {
    context.print_style.default_format_float.mode = .SCIENTIFIC;
    context.print_style.default_format_float.zero_removal = .ONE_ZERO_AFTER_DECIMAL;

    suffix := ifx backend.type_name == "float" then "f" else ""; // only "float" gets literal f-suffixes; "double" etc. get bare literals
    write_value :: (name: string) -> string #expand {
        for val : values {
            if val.name == name {
                value_str := ifx backend.is_cpp then tprint("%1%2", val.value, suffix) else tprint("%", val.value);
                return field_decl_with_value(backend, backend.type_name, name, value_str);
            }
        }
        return tprint("    % %;\n", backend.type_name, name);
    }

    struct_name := tprint("Params%", unit_name);
    builder: String_Builder;
    append(*builder, struct_open(backend, struct_name));

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
            case .FIXED_VOLTAGE; #through;
            case .VCCS; #through;
            case .CURRENT_SOURCE;
                append(*builder, write_value(el.name));
            case .DIODE; #through;
            case .DIODE_PAIR; #through;
            case .NPN; #through;
            case .PNP; #through;
            case .NJF; #through;
            case .PJF; #through;
            case .TRIODE;
                for pname : device_model_param_names(el.type, el.nonlinear)  emit_nonlinear_param(pname);
            case .AMP;
                // Rin/Rout/GBW have no fields of their own here; they're
                // synthesized RESISTOR/CAPACITOR elements.
                if el.opamp.has_aol    append(*builder, write_value(el.opamp.aol_name));
                if opamp_has_clip_stage(el) {
                    append(*builder, write_value(el.opamp.vsat_p_name));
                    append(*builder, write_value(el.opamp.vsat_n_name));
                }
                if el.opamp.has_slew   append(*builder, write_value(el.opamp.slew_name));
                if el.opamp.has_vos    append(*builder, write_value(el.opamp.vos_name));
                if el.opamp.has_ios    append(*builder, write_value(el.opamp.ios_name));
                if el.opamp.has_ibias  append(*builder, write_value(el.opamp.ibias_name));
        }
    }

    append(*builder, struct_close(backend, struct_name));
    return builder_to_string(*builder);
}

// extra_state_names: nonlinear-cluster junction-voltage unknowns, persisted
// across samples as the Newton warm start.
// There are two `-instrument` counters for one nonlinear cluster:
// - Number of Newton "solves"
// - How many iterations they took
newton_counter_names :: (cluster_name: string) -> (solves: string, iters: string) {
    return tprint("nr_solves_%", cluster_name), tprint("nr_iters_%", cluster_name);
}

NEWTON_COUNTER_TYPE :: "long long";

generate_state_struct :: (elements: [] Element, extra_state_names: [] string, backend: Backend = .{}, unit_name := "", counter_names: [] string = .[]) -> string, [] string {
    states: [..] string;
    struct_name := tprint("State%", unit_name);

    builder: String_Builder;
    append(*builder, struct_open(backend, struct_name));

    for el : elements {
        if el.type == {
            case .CAPACITOR; #through;
            case .INDUCTOR; #through;
            case .RC_SERIES; #through;
            case .RC_PARALLEL;
                array_add(*states, tprint("z%", el.name));
                append(*builder, field_decl(backend, backend.type_name, states[states.count-1]));
            case .AMP;
                // zslew_<name>: previous sample's converged output, distinct
                // from vclip_<name>'s own warm-start (unlike a diode, slew
                // needs *last sample's* value as a fixed reference).
                if el.opamp.has_slew {
                    array_add(*states, tprint("zslew_%", el.name));
                    append(*builder, field_decl(backend, backend.type_name, states[states.count-1]));
                }
        }
    }

    for name : extra_state_names {
        array_add(*states, name);
        append(*builder, field_decl(backend, backend.type_name, name));
    }

    // -instrument counters.
    for name : counter_names  append(*builder, field_decl(backend, NEWTON_COUNTER_TYPE, name));

    append(*builder, struct_close(backend, struct_name));
    return builder_to_string(*builder), states;
}

generate_impedance_defs :: (elements: [] Element, backend: Backend = .{}) -> string {
    builder: String_Builder;

    append(*builder, sum_recip_sum_defs(backend));

    auto_or_type := backend.decl_type;
    suffix       := ifx backend.type_name == "float" then "f" else ""; // only "float" gets literal f-suffixes; "double" etc. get bare literals

    // DIODE/DIODE_PAIR/BJT/JFET I-V-law expressions (built in
    // nonlinear_solve.jai) and the AMP KCL branch (nodal_analysis.jai)
    // reference their Params fields as bare symbol names (e.g. "D1_Is",
    // "Eop_Aol"), so alias each one to a local const here.
    emitted_bare_aliases: [..] string;
    emit_bare_alias :: (pname: string) #expand {
        if pname.count == 0  return; // e.g. BJT-only fields left blank on DIODE/DIODE_PAIR elements
        if array_find(emitted_bare_aliases, pname)  return;
        array_add(*emitted_bare_aliases, pname);
        append(*builder, tprint("const %1 %2 = params.%2;\n", auto_or_type, pname));
    }

    for el : elements {
        for pname : device_model_param_names(el.type, el.nonlinear)  emit_bare_alias(pname);
        if el.type == .AMP {
            if el.opamp.has_aol    emit_bare_alias(el.opamp.aol_name);
            if opamp_has_clip_stage(el) {
                emit_bare_alias(el.opamp.vsat_p_name);
                emit_bare_alias(el.opamp.vsat_n_name);
            }
            if el.opamp.has_slew   emit_bare_alias(el.opamp.slew_name);
            if el.opamp.has_vos    emit_bare_alias(el.opamp.vos_name);
            if el.opamp.has_ios    emit_bare_alias(el.opamp.ios_name);
            if el.opamp.has_ibias  emit_bare_alias(el.opamp.ibias_name);
        }
    }
    if emitted_bare_aliases.count > 0  append(*builder, "\n");

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
            case .VCCS;
                append(*builder, tprint("const %1 g%2 = %3;\n\n", auto_or_type, el.name, val_name));
            case .CURRENT_SOURCE;
                append(*builder, tprint("const %1 %2 = %3;\n\n", auto_or_type, el.name, val_name));
        }
    }

    return builder_to_string(*builder);
}

// ===========================================================================
// det(A) magnitude-underflow check (see sym_algebra.jai's Magnitude_Check).
// ===========================================================================

// Conservative standalone-component defaults, used only when a component's
// value can't be found in `values` directly. Picked to bias toward the
// SMALL-magnitude side of typical audio component ranges.
DET_A_CHECK_DEFAULT_R :: 1.0e6;     // 1 Mohm  -> small conductance
DET_A_CHECK_DEFAULT_C :: 100.0e-12; // 100 pF  -> small conductance
DET_A_CHECK_DEFAULT_L :: 10.0;      // 10 H    -> small conductance

// Sample rate isn't known at codegen time, so we'll use 48k as a plausible example.
DET_A_CHECK_NOMINAL_SAMPLE_RATE :: 48000.0;

// Threshold for the active precision. float32's true denormal floor is
// ~1.4e-45; this leaves real margin above that. @TUNE_ME
DET_A_CHECK_THRESHOLD_FLOAT32 :: 1.0e-25;

find_element_value :: (values: [] Element_Value, name: string) -> (found: bool, value: float64) {
    for v : values  if v.name == name  return true, cast(float64) v.value;
    return false, 0;
}

// Build the symbol -> magnitude lookup table the det(A) underflow check
// needs. `elements` must be the POST-reduce_circuit element list (RC_SERIES/
// RC_PARALLEL merges, combo_expr assignment already applied) since that's
// what determines which symbol names actually appear in det(A).
build_det_a_magnitude_table :: (elements: [] Element, values: [] Element_Value) -> Table(string, float64) {
    table: Table(string, float64);

    // Direct pass-through: covers nonlinear/op-amp model params by their bare
    // Params-field name (no fallback needed). Also covers CURRENT_SOURCE/FIXED_VOLTAGE,
    // whose symbols are their bare element name.
    for v : values  table_set(*table, v.name, cast(float64) v.value);

    lookup_or_default :: (name: string, default_value: float64) -> float64 #expand {
        found, v := find_element_value(values, name);
        return ifx found then v else default_value;
    }

    // R/L/C (and RC_SERIES/RC_PARALLEL, VCCS) don't appear in det(A) under
    // their own name. solve() sees the derived admittance symbol
    // ("g" + name), computed here with the same formulas generate_impedance_defs()
    // emits as code, so this overrides/extends the pass-through above.
    for el : elements {
        if el.type == {
            case .RESISTOR;
                r := lookup_or_default(el.name, DET_A_CHECK_DEFAULT_R);
                table_set(*table, tprint("g%", el.name), 1.0 / r);
            case .CAPACITOR;
                c := lookup_or_default(el.name, DET_A_CHECK_DEFAULT_C);
                table_set(*table, tprint("g%", el.name), 2.0 * DET_A_CHECK_NOMINAL_SAMPLE_RATE * c);
            case .INDUCTOR;
                l := lookup_or_default(el.name, DET_A_CHECK_DEFAULT_L);
                table_set(*table, tprint("g%", el.name), 1.0 / (2.0 * DET_A_CHECK_NOMINAL_SAMPLE_RATE * l));
            case .RC_SERIES;
                r  := lookup_or_default(el.rc_internal.res_name, DET_A_CHECK_DEFAULT_R);
                c  := lookup_or_default(el.rc_internal.cap_name, DET_A_CHECK_DEFAULT_C);
                sr := DET_A_CHECK_NOMINAL_SAMPLE_RATE;
                table_set(*table, tprint("g%", el.name), (2.0 * sr * c) / (1.0 + 2.0 * sr * r * c));
                // gn (state-decay coefficient) is a ratio near 1, not a
                // shrinking factor, so it's left at the default_magnitude fallback.
            case .RC_PARALLEL;
                r  := lookup_or_default(el.rc_internal.res_name, DET_A_CHECK_DEFAULT_R);
                c  := lookup_or_default(el.rc_internal.cap_name, DET_A_CHECK_DEFAULT_C);
                sr := DET_A_CHECK_NOMINAL_SAMPLE_RATE;
                table_set(*table, tprint("g%", el.name), 2.0 * sr * c + 1.0 / r);
            case .VCCS;
                gm := lookup_or_default(el.name, 1.0); // no standard "conservative" default for transconductance, so this is a neutral fallback
                table_set(*table, tprint("g%", el.name), gm);
        }
    }

    return table;
}

// Builds the Magnitude_Check to pass into solve() for the det(A) underflow
// warning: disabled entirely for non-float32 backends or when there's
// no component data to estimate from.
build_magnitude_check :: (elements: [] Element, values: [] Element_Value, backend: Backend, label: string) -> Magnitude_Check {
    if backend.type_name != "float" || values.count == 0  return .{};
    return .{
        enabled    = true,
        threshold  = DET_A_CHECK_THRESHOLD_FLOAT32,
        magnitudes = build_det_a_magnitude_table(elements, values),
        label      = label,
    };
}

// Junction-voltage step limiter (like ngspice's DEVpnjlim): caps the
// per-iteration step once v exceeds vcrit, so exp(v/vt) can't overflow.
// Emitted once per header.
generate_junction_limiter_helper :: (backend: Backend) -> string {
    if !backend.is_cpp {
        type_name := backend.type_name;
        abs_fn    := math_fn(backend, "fabs");
        log_fn    := transcendental_fn_name(backend, "log");
        template := #string DONE
static %1 limit_junction_voltage(%1 v_new, %1 v_old, %1 vt, %1 vcrit) {
    if (v_new > vcrit && %2(v_new - v_old) > 2 * vt) {
        if (v_old > 0) {
            %1 arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * %3(arg) : vcrit;
        } else {
            v_new = vt * %3(v_new / vt);
        }
    }
    return v_new;
}
DONE
        return tprint(template, type_name, abs_fn, log_fn);
    }

    abs_fn := math_fn(backend, "abs");
    log_fn := transcendental_fn_name(backend, "log");
    template := #string DONE
[[maybe_unused]] static auto limit_junction_voltage = [](auto v_new, auto v_old, auto vt, auto vcrit)
{
    if (v_new > vcrit && %1(v_new - v_old) > 2 * vt)
    {
        if (v_old > 0)
        {
            const auto arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * %2(arg) : vcrit;
        }
        else
        {
            v_new = vt * %2(v_new / vt);
        }
    }
    return v_new;
};
DONE
    return tprint(template, abs_fn, log_fn);
}

generate_junction_limiter_sym_helper :: (backend: Backend) -> string {
    if !backend.is_cpp {
        type_name := backend.type_name;
        abs_fn    := math_fn(backend, "fabs");
        log_fn    := transcendental_fn_name(backend, "log");
        template := #string DONE
static %1 limit_junction_voltage_sym(%1 v_new, %1 v_old, %1 vt, %1 vcrit) {
    if (v_new > vcrit && %2(v_new - v_old) > 2 * vt) {
        if (v_old > 0) {
            %1 arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * %3(arg) : vcrit;
        } else {
            v_new = vt * %3(v_new / vt);
        }
    } else if (v_new < -vcrit && %2(v_new - v_old) > 2 * vt) {
        if (v_old < 0) {
            %1 arg = 1 + (v_old - v_new) / vt;
            v_new = arg > 0 ? v_old - vt * %3(arg) : -vcrit;
        } else {
            v_new = -vt * %3(-v_new / vt);
        }
    }
    return v_new;
}
DONE
        return tprint(template, type_name, abs_fn, log_fn);
    }

    abs_fn := math_fn(backend, "abs");
    log_fn := transcendental_fn_name(backend, "log");
    cpp_template := #string DONE
[[maybe_unused]] static auto limit_junction_voltage_sym = [](auto v_new, auto v_old, auto vt, auto vcrit)
{
    if (v_new > vcrit && %1(v_new - v_old) > 2 * vt)
    {
        if (v_old > 0)
        {
            const auto arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * %2(arg) : vcrit;
        }
        else
        {
            v_new = vt * %2(v_new / vt);
        }
    }
    else if (v_new < -vcrit && %1(v_new - v_old) > 2 * vt)
    {
        if (v_old < 0)
        {
            const auto arg = 1 + (v_old - v_new) / vt;
            v_new = arg > 0 ? v_old - vt * %2(arg) : -vcrit;
        }
        else
        {
            v_new = -vt * %2(-v_new / vt);
        }
    }
    return v_new;
};
DONE
    return tprint(cpp_template, abs_fn, log_fn);
}

// JFET vGS step limiter: clamps Newton steps at pinch-off (vGS < Vp).
// No upper clamp - the quadratic stays valid (if approximate) above vGS=0.
generate_jfet_limiter_helper :: (backend: Backend) -> string {
    if !backend.is_cpp {
        type_name := backend.type_name;
        template := #string DONE
static % limit_jfet_vgs(% v_new, % vp) {
    if (v_new < vp) return vp;
    return v_new;
}
DONE
        return tprint(template, type_name, type_name, type_name);
    }

    return #string DONE
[[maybe_unused]] static auto limit_jfet_vgs = [](auto v_new, auto vp)
{
    if (v_new < vp) return vp;
    return v_new;
};
DONE;
}

// Hard rail clamp applied as a post-step limiter - the Newton
// residual itself is just the plain linear VCVS equation. Slew (v_prev/
// slew_max_step) clamps first, then the rails; unknowns with no slew set
// pass a permissive slew_max_step so that branch is a no-op.
generate_opamp_clip_limiter_helper :: (backend: Backend) -> string {
    if !backend.is_cpp {
        type_name := backend.type_name;
        template := #string DONE
static % clamp_opamp_output(% v_new, % v_prev, % slew_max_step, % vsat_n, % vsat_p) {
    if (v_new - v_prev > slew_max_step)  v_new = v_prev + slew_max_step;
    if (v_new - v_prev < -slew_max_step) v_new = v_prev - slew_max_step;
    if (v_new > vsat_p) return vsat_p;
    if (v_new < vsat_n) return vsat_n;
    return v_new;
}
DONE
        return tprint(template, type_name, type_name, type_name, type_name, type_name, type_name);
    }

    return #string DONE
[[maybe_unused]] static auto clamp_opamp_output = [](auto v_new, auto v_prev, auto slew_max_step, auto vsat_n, auto vsat_p)
{
    if (v_new - v_prev > slew_max_step)  v_new = v_prev + slew_max_step;
    if (v_new - v_prev < -slew_max_step) v_new = v_prev - slew_max_step;
    if (v_new > vsat_p) return vsat_p;
    if (v_new < vsat_n) return vsat_n;
    return v_new;
};
DONE;
}

// vcrit = vt * log(vt / (sqrt(2) * Is)) per junction-voltage unknown, derived
// once from Params fields.
generate_vcrit_defs :: (clusters: [] Nonlinear_Cluster, backend: Backend) -> string {
    auto_or_type := backend.decl_type;
    log_fn       := math_fn(backend, "log");
    sqrt_fn      := math_fn(backend, "sqrt");

    builder: String_Builder;
    emitted: [..] string;
    for c : clusters {
        for u : c.unknowns {
            if u.model.limiter_kind != .EXP_JUNCTION && u.model.limiter_kind != .EXP_JUNCTION_SYM  continue;
            if array_find(emitted, u.model.vt_name)  continue;
            array_add(*emitted, u.model.vt_name);
            print_to_builder(*builder, "const %1 vcrit_%2 = %2 * %3(%2 / (%4(2.0) * %5));\n",
                             auto_or_type, u.model.vt_name, log_fn, sqrt_fn, u.model.is_name);
        }
    }
    if emitted.count > 0  append(*builder, "\n");
    return builder_to_string(*builder);
}

// Newton-Raphson convergence tolerance (volts^2) and max iteration count,
// defined once and referenced by every cluster's solve below.
generate_newton_constants :: (newton_tol_sq: float, newton_max_iter: int, backend: Backend) -> string {
    builder: String_Builder;
    print_to_builder(*builder, "%1 %2 newton_tol_sq = %3;\n", backend.const_kw, backend.decl_type, newton_tol_sq);
    print_to_builder(*builder, "%1 int newton_max_iter = %2;\n\n", backend.const_kw, newton_max_iter);
    return builder_to_string(*builder);
}

// Per-unknown state update given whichever delta applies this iteration.
// Handles EXP_JUNCTION/JFET_PINCHOFF/NONE only - used both by the plain
// per-unknown loop below and by the active-set AMP_CLIP path for the
// *other* unknowns jointly solved with the clip unknown.
emit_unknown_update :: (builder: *String_Builder, u: Cluster_Unknown, delta_expr: string) {
    if u.model.limiter_kind == {
        case .EXP_JUNCTION;
            print_to_builder(builder, "%1 = limit_junction_voltage(%1 + (%2), %1, %3, vcrit_%3);\n",
                             u.name, delta_expr, u.model.vt_name);
        case .EXP_JUNCTION_SYM;
            print_to_builder(builder, "%1 = limit_junction_voltage_sym(%1 + (%2), %1, %3, vcrit_%3);\n",
                             u.name, delta_expr, u.model.vt_name);
        case .JFET_PINCHOFF;
            print_to_builder(builder, "%1 = limit_jfet_vgs(%1 + (%2), %3);\n", u.name, delta_expr, u.model.vp_name);
        case;
            print_to_builder(builder, "%1 = %1 + (%2);\n", u.name, delta_expr);
    }
}

// Runtime dispatch for a cluster's single active-set AMP_CLIP unknown
// and the other unknowns jointly solved with it. Every iteration,
// re-derives from scratch which of three systems is the correct one to use:
//   - free:  the joint system, used whenever the unconstrained target lands inside the rails.
//   - hi/lo: the reduced system with the clip unknown fixed at the violated rail.
generate_amp_clip_active_set_step :: (cluster: Nonlinear_Cluster, builder: *String_Builder, backend: Backend, apply_slew: bool) {
    c := cluster.unknowns[cluster.amp_clip_idx];
    others: [..] Cluster_Unknown;
    for u : cluster.unknowns  if u.name != c.name  array_add(*others, u);

    // slew-limit the unconstrained target first, then check *that*
    // against the rails. If no slew is set, use a permissive slew_max_step
    // that never trips, reusing the target itself as v_prev.
    v_prev := ifx c.model.has_slew && apply_slew then c.model.zslew_name else "_natural";
    slew_max_step := ifx c.model.has_slew && apply_slew then tprint("(%/sample_rate)", c.model.slew_name) else "1.0e30";

    print_to_builder(builder, "const %1 _natural = (%1)(%2 + %3);\n", backend.type_name, c.name, c.delta_name);
    print_to_builder(builder, "%1 _v = _natural;\n", backend.decl_type);
    print_to_builder(builder, "if (_v - %1 > %2)  _v = %1 + %2;\n", v_prev, slew_max_step);
    print_to_builder(builder, "if (_v - %1 < -%2) _v = %1 - %2;\n", v_prev, slew_max_step);

    print_to_builder(builder, "%1 res_%2_active = 0.0, delta_%2_active = 0.0;\n", backend.decl_type, c.name);
    for u : others  print_to_builder(builder, "%1 res_%2_active = 0.0, delta_%2_active = 0.0;\n", backend.decl_type, u.name);

    print_to_builder(builder, "if (_v > %1) {\n", c.model.vsat_p_name);
    print_to_builder(builder, "    delta_%1_active = %2 - %1;\n    %1 = %2;\n", c.name, c.model.vsat_p_name);
    for u : others  print_to_builder(builder, "    res_%1_active = %2; delta_%1_active = %3;\n", u.name, u.residual_name_hi, u.delta_name_hi);
    print_to_builder(builder, "} else if (_v < %1) {\n", c.model.vsat_n_name);
    print_to_builder(builder, "    delta_%1_active = %2 - %1;\n    %1 = %2;\n", c.name, c.model.vsat_n_name);
    for u : others  print_to_builder(builder, "    res_%1_active = %2; delta_%1_active = %3;\n", u.name, u.residual_name_lo, u.delta_name_lo);
    append(builder, "} else {\n");
    print_to_builder(builder, "    res_%1_active = %2; delta_%1_active = _v - %1;\n    %1 = _v;\n", c.name, c.residual_name);
    for u : others  print_to_builder(builder, "    res_%1_active = %2; delta_%1_active = %3;\n", u.name, u.residual_name, u.delta_name);
    append(builder, "}\n\n");

    print_to_builder(builder, "%1 residual_norm_sq = res_%2_active * res_%2_active", backend.decl_type, c.name);
    for u : others  print_to_builder(builder, " + res_%1_active * res_%1_active", u.name);
    append(builder, ";\n");
    print_to_builder(builder, "%1 step_norm_sq = delta_%2_active * delta_%2_active", backend.decl_type, c.name);
    for u : others  print_to_builder(builder, " + delta_%1_active * delta_%1_active", u.name);
    append(builder, ";\n\n");

    for u : others  emit_unknown_update(builder, u, tprint("delta_%_active", u.name));
}

// Emits one Newton-invariant region (invariant_items / hi_invariant_items /
// lo_invariant_items). `other_variant_names` poisons linearity for
// anything that touches them... this shouldn't normally happen since
// invariant_items are Newton-invariant by construction, but is kept as a
// defensive guard.
emit_invariant_region :: (items: [] Gen_Var, exprs: [] Expr, backend: Backend,
                          enable_extraction: bool, basis_names: [] string, other_variant_names: [] string,
                          outer_builder: *String_Builder, inner_builder: *String_Builder) {
    if enable_extraction {
        // Every item in this list is a potential downstream terminal.
        item_names: [..] string;
        for it : items  array_add(*item_names, it.name);

        accepted, region_items := try_extract_region(items, item_names, exprs, backend, basis_names, other_variant_names);
        if accepted.count > 0 {
            emit_extracted_region(region_items, accepted, exprs, basis_names, outer_builder, inner_builder, backend);

            // The remainder is "whatever's needed by the non-accepted items".
            // A CSE temp shared between an accepted and a non-accepted item
            // needs both: once inside the basis loop, once out here.
            remaining_items := remaining_region_after_extraction(item_names, accepted, items, exprs);
            emit_topological_order(remaining_items, exprs, inner_builder, backend);
            return;
        }
    }
    emit_topological_order(items, exprs, inner_builder, backend);
}

// Indents every line of `text`, except a blank line stays blank.
indent_lines :: (text: string, indent: string) -> string {
    builder: String_Builder;
    for i : 0..text.count-1 {
        c := text[i];
        append(*builder, c);
        if c == #char "\n" {
            next_is_newline := (i + 1 < text.count) && (text[i + 1] == #char "\n");
            if !next_is_newline  append(*builder, indent);
        }
    }
    return builder_to_string(*builder);
}

// Newton-Raphson solve for one cluster, to be inserted inside the per-sample
// loop. `basis_names` should be extraction_basis_names()'s output (inputs +
// z-state only; `non_basis_variant_names` should include that function's
// output, so anything referencing a branch current directly gets correctly
// excluded from extraction instead of misclassified as basis-independent.
// Both are only used for the Newton-invariant regions. Invariant extraction
// can be disabled with enable_extraction=false.
generate_cluster_newton_code :: (cluster: Nonlinear_Cluster, exprs: [] Expr, backend: Backend, apply_slew := true, max_iter := "newton_max_iter",
                                 enable_extraction := false, basis_names: [] string = .[], non_basis_variant_names: [] string = .[],
                                 count_iterations := false) -> (outer_code: string, inner_code: string) {
    outer_builder: String_Builder;
    builder: String_Builder;
    print_to_builder(*builder, "// --- Newton-Raphson solve: %\n", cluster.name);

    other_variant_names: [..] string;
    for u : cluster.unknowns  array_add(*other_variant_names, u.name);
    for n : non_basis_variant_names  array_add(*other_variant_names, n);

    emit_invariant_region(cluster.invariant_items, exprs, backend, enable_extraction, basis_names, other_variant_names, *outer_builder, *builder);
    if cluster.amp_clip_idx >= 0 {
        emit_invariant_region(cluster.hi_invariant_items, exprs, backend, enable_extraction, basis_names, other_variant_names, *outer_builder, *builder);
        emit_invariant_region(cluster.lo_invariant_items, exprs, backend, enable_extraction, basis_names, other_variant_names, *outer_builder, *builder);
    }

    if count_iterations {
        solves_name, _ := newton_counter_names(cluster.name);
        print_to_builder(*builder, "++%;\n", solves_name);
    }

    print_to_builder(*builder, "for (int newton_iter = 0; newton_iter < %; ++newton_iter)\n{\n", max_iter);
    inner_builder: String_Builder;
    if count_iterations {
        _, iters_name := newton_counter_names(cluster.name);
        print_to_builder(*inner_builder, "++%;\n", iters_name);
    }
    emit_topological_order(cluster.iteration_items, exprs, *inner_builder, backend);
    if cluster.amp_clip_idx >= 0 {
        emit_topological_order(cluster.hi_iteration_items, exprs, *inner_builder, backend);
        emit_topological_order(cluster.lo_iteration_items, exprs, *inner_builder, backend);
    }

    if cluster.amp_clip_idx >= 0 {
        append(*inner_builder, "\n");
        generate_amp_clip_active_set_step(cluster, *inner_builder, backend, apply_slew);
    } else {
        print_to_builder(*inner_builder, "\n%1 residual_norm_sq = 0.0;\n", backend.decl_type);
        for u : cluster.unknowns  print_to_builder(*inner_builder, "residual_norm_sq += %1 * %1;\n", u.residual_name);
        print_to_builder(*inner_builder, "%1 step_norm_sq = 0.0;\n", backend.decl_type);
        for u : cluster.unknowns  print_to_builder(*inner_builder, "step_norm_sq += %1 * %1;\n", u.delta_name);
        append(*inner_builder, "\n");
        for u : cluster.unknowns {
            if u.model.limiter_kind == .AMP_CLIP {
                // Plain post-step clamp, only reached for 0 or 2+ AMP_CLIP
                // unknowns per cluster, which is unsupported by the active-set
                // path above. Every real circuit here has amp_clip_idx >= 0 instead,
                // so this is untested against an actual C fixture.
                v_prev := ifx u.model.has_slew && apply_slew then u.model.zslew_name else "_v";
                slew_max_step := ifx u.model.has_slew && apply_slew then tprint("(%/sample_rate)", u.model.slew_name) else "1.0e30";
                print_to_builder(*inner_builder, "const %8 _v = (%5)(%1 + %2);\n%1 = clamp_opamp_output(_v, %6, %7, %3, %4);\n",
                                 u.name, u.delta_name, u.model.vsat_n_name, u.model.vsat_p_name, backend.type_name, v_prev, slew_max_step, backend.decl_type);
            } else {
                emit_unknown_update(*inner_builder, u, u.delta_name);
            }
        }
    }
    append(*inner_builder, "\nif (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)\n    break;\n");

    inner_text := builder_to_string(*inner_builder);
    indented := indent_lines(inner_text, "    ");
    append(*builder, "    ");
    append(*builder, indented);
    append(*builder, "\n}\n");

    return builder_to_string(*outer_builder), builder_to_string(*builder);
}

// Emits a Gaussian elimination solve with partial pivoting.
// The solve loops are generated directly (and unrolled).
// Pivot search and row swap happen at run-time, since we
// don't have enough numerical information to make those
// decisions at codegen-time. The solver runs in-place,
// leaving the solution in `b`.
//
// For k <= 6 the cost is k(k-1)/2 comparisons against k^3/3 eliminations.
emit_unrolled_lu_solve :: (k: int, a: string, b: string, backend: Backend) -> string {
    builder: String_Builder;
    ty      := backend.type_name;
    abs_fn  := ifx backend.is_cpp then math_fn(backend, "abs") else math_fn(backend, "fabs");

    for col : 0..k-1 {
        if col < k - 1 {
            // Partial pivot. We want to keep every array subscript constant, so the matrix stays in registers.
            print_to_builder(*builder, "{\n    int _p = %1;\n    %2 _bv = %3(%4[%1][%1]);\n", col, ty, abs_fn, a);
            for r : col+1..k-1 {
                print_to_builder(*builder, "    { const %1 _m = %2(%3[%4][%5]); if (_m > _bv) { _bv = _m; _p = %4; } }\n",
                                 ty, abs_fn, a, r, col);
            }
            for r : col+1..k-1 {
                print_to_builder(*builder, "    %1 (_p == %2) {\n", ifx r == col+1 then "if" else "else if", r);
                for c : col..k-1 {
                    print_to_builder(*builder, "        { %1 _t = %2[%3][%4]; %2[%3][%4] = %2[%5][%4]; %2[%5][%4] = _t; }\n", ty, a, col, c, r);
                }
                print_to_builder(*builder, "        { %1 _t = %2[%3]; %2[%3] = %2[%4]; %2[%4] = _t; }\n", ty, b, col, r);
                append(*builder, "    }\n");
            }
            append(*builder, "}\n");
        }

        // Eliminate below the pivot.
        if col < k - 1 {
            print_to_builder(*builder, "const %1 _r%2 = (%1)1 / %3[%2][%2];\n", ty, col, a);
            for r : col+1..k-1 {
                print_to_builder(*builder, "{\n    const %1 _f = %2[%3][%4] * _r%4;\n", ty, a, r, col);
                for c : col+1..k-1 {
                    print_to_builder(*builder, "    %1[%2][%3] -= _f * %1[%4][%3];\n", a, r, c, col);
                }
                print_to_builder(*builder, "    %1[%2] -= _f * %1[%3];\n}\n", b, r, col);
            }
        }
    }

    // Back substitution. Every diagonal entry is final once elimination
    // finishes, so all k reciprocals are independent of each other and issue in
    // parallel; only the multiply-accumulate chain remains serial.
    print_to_builder(*builder, "const %1 _rb%2 = (%1)1 / %3[%2][%2];\n", ty, k-1, a);
    for row : 0..k-2  print_to_builder(*builder, "const %1 _rb%2 = _r%2;\n", ty, row);
    for < row : 0..k-1 {
        print_to_builder(*builder, "%1[%2] = (%1[%2]", b, row);
        for c : row+1..k-1  print_to_builder(*builder, " - %1[%2][%3] * %4[%3]", a, row, c, b);
        print_to_builder(*builder, ") * _rb%1;\n", row);
    }

    return builder_to_string(*builder);
}

// Newton-Raphson solve for one cluster in its N-port (Thevenin) form:
//     F_j =  voc_j + sum_m Z_jm*i_m(v) - v_j
//     J_jn = sum_m Z_jm*G_mn(v) - (j == n)
//     solve J*delta = -F, then v += limit(delta)
//
// generate_unit() will pick either this or generate_cluster_newton_code().
// While structurally the same, this function generates C code rather than
// an Expr tree.
generate_cluster_port_newton_code :: (cluster: Nonlinear_Cluster, exprs: [] Expr, backend: Backend, max_iter := "newton_max_iter",
                                      enable_extraction := false, basis_names: [] string = .[], non_basis_variant_names: [] string = .[],
                                      count_iterations := false) -> (outer_code: string, inner_code: string) {
    pf := cluster.port;
    k  := pf.k;

    outer_builder: String_Builder;
    builder:       String_Builder;
    print_to_builder(*builder, "// --- Newton-Raphson solve (N-port): %\n", cluster.name);

    // Z: constant for fixed params and sample rate, computed before per-sample loop.
    emit_topological_order(pf.z_items, exprs, *outer_builder, backend);

    other_variant_names: [..] string;
    for u : cluster.unknowns  array_add(*other_variant_names, u.name);
    for n : non_basis_variant_names  array_add(*other_variant_names, n);

    // voc: per sample, above the Newton loop. Affine in the inputs and the reactive state.
    emit_invariant_region(pf.voc_items, exprs, backend, enable_extraction, basis_names, other_variant_names, *outer_builder, *builder);
    emit_invariant_region(pf.invariant_items, exprs, backend, enable_extraction, basis_names, other_variant_names, *outer_builder, *builder);

    if count_iterations {
        solves_name, _ := newton_counter_names(cluster.name);
        print_to_builder(*builder, "++%;\n", solves_name);
    }

    print_to_builder(*builder, "for (int newton_iter = 0; newton_iter < %; ++newton_iter)\n{\n", max_iter);

    inner: String_Builder;
    if count_iterations {
        _, iters_name := newton_counter_names(cluster.name);
        print_to_builder(*inner, "++%;\n", iters_name);
    }

    // Device laws i_m(v), block-diagonal G_mn = di_m/dv_n, the residual F_j,
    // and (k <= PORT_CRAMER_MAX_K) the Cramer's-rule delta.
    emit_topological_order(pf.iteration_items, exprs, *inner, backend);

    // k > PORT_CRAMER_MAX_K: partial pivoting is control flow that Expr trees
    // can't represent, so the linear solve itself is a C code text block.
    if k > PORT_CRAMER_MAX_K  append(*inner, emit_port_solve_block(cluster, backend));

    // Convergence test and state update.
    ty := backend.decl_type;
    print_to_builder(*inner, "\n%1 residual_norm_sq = 0.0;\n", ty);
    for u : cluster.unknowns  print_to_builder(*inner, "residual_norm_sq += %1 * %1;\n", u.residual_name);
    print_to_builder(*inner, "%1 step_norm_sq = 0.0;\n", ty);
    for u : cluster.unknowns  print_to_builder(*inner, "step_norm_sq += %1 * %1;\n", u.delta_name);
    append(*inner, "\n");
    for u : cluster.unknowns  emit_unknown_update(*inner, u, u.delta_name);
    append(*inner, "\nif (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)\n    break;\n");

    append(*builder, "    ");
    append(*builder, indent_lines(builder_to_string(*inner), "    "));
    append(*builder, "\n}\n");

    return builder_to_string(*outer_builder), builder_to_string(*builder);
}

// The pivoted-LU linear solve for one N-port Newton iteration, k > PORT_CRAMER_MAX_K.
emit_port_solve_block :: (cluster: Nonlinear_Cluster, backend: Backend) -> string {
    pf := cluster.port;
    k  := pf.k;
    inner: String_Builder;
    ty := backend.decl_type;

    print_to_builder(*inner, "\n%1 _J[%2][%2];\n", backend.type_name, k);
    for j : 0..k-1 {
        for n : 0..k-1 {
            name := pf.j_names[j*k + n];
            if name.count > 0  print_to_builder(*inner, "_J[%1][%2] = %3;\n", j, n, name);
            else               print_to_builder(*inner, "_J[%1][%2] = (%3)0;\n", j, n, backend.type_name);
        }
    }

    print_to_builder(*inner, "%1 _b[%2] = { ", backend.type_name, k);
    for j : 0..k-1  print_to_builder(*inner, "%1-%2", ifx j > 0 then ", " else "", cluster.unknowns[j].residual_name);
    append(*inner, " };\n");

    append(*inner, emit_unrolled_lu_solve(k, "_J", "_b", backend));

    for j : 0..k-1  print_to_builder(*inner, "const %1 %2 = _b[%3];\n", ty, cluster.unknowns[j].delta_name, j);

    return builder_to_string(*inner);
}

// Arithmetic ops in a block of emitted C, counted from the text.
count_flops_text :: (code: string) -> Flop_Count {
    result: Flop_Count;
    i := 0;
    while i < code.count {
        c := code[i];
        // Skip // comments to end of line.
        if c == #char "/" && i + 1 < code.count && code[i + 1] == #char "/" {
            while i < code.count && code[i] != #char "\n"  i += 1;
            continue;
        }
        if c == {
            case #char "*"; result.muls += 1;
            case #char "/"; result.divs += 1;
            case #char "+"; result.adds += 1;
            case #char "-"; result.adds += 1; // includes unary negation; close enough
        }
        i += 1;
    }
    return result;
}

// Per-iteration cost of one N-port Newton loop: the device-law expressions
// plus the emitted linear-solve block.
port_newton_iteration_flops :: (cluster: Nonlinear_Cluster, exprs: [] Expr, backend: Backend) -> Flop_Count {
    result := count_flops(cluster.port.iteration_items, exprs, backend.transcendental_approx);
    if cluster.port.k > PORT_CRAMER_MAX_K {
        block := count_flops_text(emit_port_solve_block(cluster, backend));
        result.muls += block.muls;
        result.divs += block.divs;
        result.adds += block.adds;
    }
    return result;
}

// Builds and solves the DC-equivalent circuit (see generate_kcl_equations
// dc_mode), and derives the steady-state z-value for each reactive element.
// outputs: also solves the DC voltage at each of these nodes (returned as
// output_dc_vars, name "<node>_dc_out").
build_dc_operating_point :: (elements: [] Element, inputs: [] string, outputs: [] string = .[], backend: Backend = .{}) -> (clusters: [] Nonlinear_Cluster, pool: Expr_Pool, z_init_vars: [] Gen_Var, output_dc_vars: [] Gen_Var) {
    dc_solve_vars, dc_equations, dc_pool, dc_elements := generate_kcl_equations(elements, inputs, true);
    dc_solutions := solve(dc_equations, dc_solve_vars, *dc_pool);

    dc_clusters := build_nonlinear_clusters(dc_elements, dc_solve_vars, dc_solutions, *dc_pool, backend);

    z_init_vars: [..] Gen_Var;
    for el : dc_elements {
        if el.type != .CAPACITOR && el.type != .RC_SERIES && el.type != .RC_PARALLEL  continue;

        // copy_expr: pos_v/neg_v can alias a solution node shared by another
        // reactive element on the same terminal, and simplify() below mutates
        // in place, so a bare lookup would corrupt it for a later iteration.
        pos_v := copy_expr(lookup_node_voltage(el.terminals[0], dc_solve_vars, dc_solutions, *dc_pool, inputs), *dc_pool);
        neg_v := copy_expr(lookup_node_voltage(el.terminals[1], dc_solve_vars, dc_solutions, *dc_pool, inputs), *dc_pool);

        z_expr: Expr_ID;
        if el.type == .RC_PARALLEL {
            // z tracks only the capacitor half; gz is already 2x that conductance.
            gz_sym := expr_symbol(tprint("gz%", el.name), *dc_pool);
            z_expr = expr_div(expr_mul(gz_sym, expr_sub(pos_v, neg_v, *dc_pool), *dc_pool), expr_const(2, *dc_pool), *dc_pool);
        } else {
            g_sym := expr_symbol(tprint("g%", el.name), *dc_pool);
            z_expr = expr_mul(g_sym, expr_sub(pos_v, neg_v, *dc_pool), *dc_pool);
        }
        z_expr = simplify(z_expr, *dc_pool);
        array_add(*z_init_vars, Gen_Var.{ name = tprint("z%", el.name), expr_id = z_expr });
    }

    output_dc_vars: [..] Gen_Var;
    for output : outputs {
        v := simplify(copy_expr(lookup_node_voltage(output, dc_solve_vars, dc_solutions, *dc_pool, inputs), *dc_pool), *dc_pool);
        array_add(*output_dc_vars, Gen_Var.{ name = tprint("%_dc_out", output), expr_id = v });
    }

    return dc_clusters, dc_pool, z_init_vars, output_dc_vars;
}

// Emits a `reset()` that solves the DC operating point and writes it into
// every channel's State. Each input gets its own `<input>_dc` param. When
// `outputs` is exactly one node, reset() also returns that node's settled
// DC voltage (instead of void).
generate_reset_function :: (elements: [] Element, inputs: [] string, outputs: [] string, backend: Backend, unit_name := "", counter_names: [] string = .[]) -> string {
    dc_clusters, dc_pool, z_init_vars, output_dc_vars := build_dc_operating_point(elements, inputs, outputs, backend);
    returns_dc := outputs.count == 1;

    builder: String_Builder;
    print_to_builder(*builder, "static % reset% (Params% params, State%* state, int num_channels, float sample_rate",
                      ifx returns_dc then "float" else "void", unit_name, unit_name, unit_name);
    for input : inputs {
        append(*builder, param_decl_with_default(backend, "float", tprint("%_dc", input), "0.0f"));
    }
    append(*builder, ")\n{\n");

    append(*builder, indent(generate_impedance_defs(elements, backend)));
    if dc_clusters.count > 0  append(*builder, indent(generate_vcrit_defs(dc_clusters, backend)));
    for input : inputs  print_to_builder(*builder, "    const %1 %2 = %2_dc;\n", backend.decl_type, input);
    if inputs.count > 0  append(*builder, "\n");

    // Newton unknowns start at 0 (cold-start guess). No zslew_<name> local
    // needed here since the DC solve runs with apply_slew=false.
    for c : dc_clusters  for u : c.unknowns  print_to_builder(*builder, "    %1 %2 = 0;\n", backend.type_name, u.name);
    if dc_clusters.count > 0  append(*builder, "\n");

    // DC solve runs once before processing starts and needs to converge
    // reliably on the first try, so it gets a much higher iteration cap
    // than the per-sample solve, still bounded to avoid an infinite loop.
    for c : dc_clusters {
        // reset() doesn't have a per-sample loop, so we don't bother with
        // extracting/hoisting invariants.
        outer_code, newton_code: string;
        if use_port_form(c, backend) {
            outer_code, newton_code = generate_cluster_port_newton_code(c, dc_pool.exprs, backend, max_iter = "10000", enable_extraction = false);
        } else {
            outer_code, newton_code = generate_cluster_newton_code(c, dc_pool.exprs, backend, apply_slew = false, max_iter = "10000", enable_extraction = false);
        }
        if outer_code.count > 0  append(*builder, indent(outer_code));
        append(*builder, indent(newton_code));
    }

    reset_suffix := ifx backend.type_name == "float" then "f" else "";
    for zv : z_init_vars  emit_const_decl(*builder, backend, zv.name, zv.expr_id, dc_pool.exprs, reset_suffix, "    ");
    if z_init_vars.count > 0  append(*builder, "\n");
    for ov : output_dc_vars  emit_const_decl(*builder, backend, ov.name, ov.expr_id, dc_pool.exprs, reset_suffix, "    ");
    if output_dc_vars.count > 0  append(*builder, "\n");

    append(*builder, "    for (int ch = 0; ch < num_channels; ++ch)\n    {\n");
    for c : dc_clusters {
        for u : c.unknowns {
            print_to_builder(*builder, "        state[ch].%1 = %1;\n", u.name);
            // zslew_<name> starts at the same DC-settled value.
            if u.model.has_slew  print_to_builder(*builder, "        state[ch].%1 = %2;\n", u.model.zslew_name, u.name);
        }
    }
    for zv : z_init_vars  print_to_builder(*builder, "        state[ch].%1 = %1;\n", zv.name);
    // -instrument counters. reset()'s own DC solve is deliberately not counted.
    for name : counter_names  print_to_builder(*builder, "        state[ch].%1 = 0;\n", name);
    append(*builder, "    }\n");

    if returns_dc  print_to_builder(*builder, "    return %;\n", output_dc_vars[0].name);
    append(*builder, "}\n");
    return builder_to_string(*builder);
}

generate_expressions :: (elements: [] Element,
                         outputs: [] string,
                         solve_vars: [] Expr_ID,
                         solutions: [] Expr_ID,
                         pool: *Expr_Pool) -> [] Gen_Var {
    gen_vars: [..] Gen_Var;

    add_gen_var :: (name: string, e: Expr_ID) #expand {
        array_add(*gen_vars, Gen_Var.{ name = name, expr_id = e });
    }

    add_expression_for_node :: (node: string) #expand {
        // Skip nodes whose voltage is already defined by a FIXED_VOLTAGE element
        // in the outer code (generate_impedance_defs emits them there).
        for fv_el : elements {
            if fv_el.type == .FIXED_VOLTAGE && fv_el.terminals[0] == node  return;
        }
        node_symbol := expr_symbol(node, pool);
        found_solve_var, solve_idx := array_find(solve_vars, node_symbol);
        if !found_solve_var  return;

        for gv : gen_vars  if gv.name == node  return;
        add_gen_var(node, simplify(solutions[solve_idx], pool));
    }

    for output : outputs {
        add_expression_for_node(output);
    }

    for el : elements {
        if el.type == {
            case .CAPACITOR; #through;
            case .INDUCTOR;
                pos_node := el.terminals[0];
                neg_node := el.terminals[1];
                add_expression_for_node(pos_node);
                add_expression_for_node(neg_node);

                expr_name := tprint("t%", el.name);
                add_gen_var(expr_name, expr_mul(tprint("g%", el.name), expr_sub(pos_node, neg_node, pool), pool));

            case .RC_SERIES; #through;
            case .RC_PARALLEL;
                pos_node := el.terminals[0];
                neg_node := el.terminals[1];
                add_expression_for_node(pos_node);
                add_expression_for_node(neg_node);

                expr_name := tprint("v%", el.name);
                add_gen_var(expr_name, expr_sub(pos_node, neg_node, pool));
        }
    }

    return gen_vars;
}

// Nonlinear branch-current placeholder symbols (i<suffix><name>),
// that can't be used as an extraction basis axis.
nonlinear_branch_current_names :: (elements: [] Element) -> [..] string {
    names: [..] string;
    for el : elements {
        spec := nonlinear_device_spec(el.type);
        for term : spec.kcl_terms {
            array_add_if_unique(*names, tprint("i%1%2", term.branch_suffix, el.name));
        }
    }
    return names;
}

// Build the set of base loop-variant symbol names: inputs + state variables
// + nonlinear branch currents (free per-sample symbols, not solved-for MNA
// unknowns; see generate_kcl_equations). Used for the (purely boolean)
// loop-variance classification. For extraction's basis axes specifically,
// use extraction_basis_names instead (branch currents aren't real variables).
build_variant_base :: (elements: [] Element, inputs: [] string) -> [..] string {
    names: [..] string;
    for inp : inputs  array_add(*names, inp);
    for el : elements {
        if el.type == .CAPACITOR || el.type == .INDUCTOR
           || el.type == .RC_SERIES || el.type == .RC_PARALLEL {
            array_add(*names, tprint("z%", el.name));
        }
    }
    for n : nonlinear_branch_current_names(elements)  array_add(*names, n);
    return names;
}

// Extraction basis: inputs + z-state only. Unlike build_variant_base, this
// deliberately excludes nonlinear branch-current placeholders, so they can't
// be a dot-product axis. Any gen_var/CSE temp that references one directly
// should be treated as depending on something outside the basis (not extractable),
// which is why callers also need nonlinear_branch_current_names in their
// classify_region_for_basis's `other_variant_names`.
extraction_basis_names :: (elements: [] Element, inputs: [] string) -> [..] string {
    names: [..] string;
    for inp : inputs  array_add(*names, inp);
    for el : elements {
        if el.type == .CAPACITOR || el.type == .INDUCTOR
           || el.type == .RC_SERIES || el.type == .RC_PARALLEL {
            array_add(*names, tprint("z%", el.name));
        }
    }
    return names;
}

// Emit items into builder as `const auto name = expr;` lines in topological
// order: each item is written only after all inner-loop symbol dependencies it
// references have already been written.
emit_topological_order :: (items: [] Gen_Var, exprs: [] Expr, builder: *String_Builder, backend: Backend = .{}) {
    literal_suffix := ifx backend.type_name == "float" then "f" else "";

    name_to_idx: Table(string, int);
    for item, i : items  table_add(*name_to_idx, item.name, i);

    // Every item name must be unique to avoid C/C++ duplicate variable names
    assert(name_to_idx.count == items.count, "emit_topological_order: duplicate Gen_Var name in items");

    emitted: [..] bool;
    for _ : 0..items.count-1  array_add(*emitted, false);

    expr_ids: [..] Expr_ID;
    expr_ids.allocator = temp;
    for item : items  array_add(*expr_ids, item.expr_id);
    deps_flat, deps_offsets := collect_symbol_names_csr(expr_ids, exprs);

    remaining := items.count;
    while remaining > 0 {
        made_progress := false;
        for i : 0..items.count-1 {
            if emitted[i]  continue;
            ready := true;
            for dep : deps_csr_slice(deps_flat, deps_offsets, i) {
                found, j := table_find(*name_to_idx, dep);
                if found && j != i && !emitted[j] {
                    ready = false;
                    break;
                }
            }
            if ready {
                emitted[i]  = true;
                remaining  -= 1;
                made_progress = true;
                emit_const_decl(builder, backend, items[i].name, items[i].expr_id, exprs, literal_suffix);
            }
        }
        if !made_progress  break;  // Cycle guard.
    }
}

// Register-pressure list scheduling: not used for emission order (see
// emit_topological_order), kept purely as a peak-live-value estimator, a
// register-pressure proxy that should_extract uses for spill-cost
// accounting. Greedily picks, among ready items, whichever kills the most
// already-live deps; once live count is at/above num_registers, ties
// additionally go to whichever ready item has fewer of its own deps.
schedule_for_register_pressure :: (items: [] Gen_Var, exprs: [] Expr, num_registers: int) -> (order: [] int, peak_live: int) {
    name_to_idx: Table(string, int);
    for item, i : items  table_add(*name_to_idx, item.name, i);
    // No duplicate var names!
    assert(name_to_idx.count == items.count, "schedule_for_register_pressure: duplicate Gen_Var name in items");

    expr_ids: [..] Expr_ID;
    expr_ids.allocator = temp;
    for item : items  array_add(*expr_ids, item.expr_id);
    deps_flat, deps_offsets := collect_symbol_names_csr(expr_ids, exprs);

    // remaining_uses[j]: how many other items in this list still reference items[j].name.
    remaining_uses: [..] int;
    for _ : 0..items.count-1  array_add(*remaining_uses, 0);
    for i : 0..items.count-1 {
        for dep : deps_csr_slice(deps_flat, deps_offsets, i) {
            found, j := table_find(*name_to_idx, dep);
            if found && j != i  remaining_uses[j] += 1;
        }
    }

    emitted: [..] bool;
    for _ : 0..items.count-1  array_add(*emitted, false);

    order: [..] int;
    live := 0;
    peak_live := 0;

    remaining := items.count;
    while remaining > 0 {
        best_i := -1;
        best_kills := -1;
        best_dep_count := 0;
        for i : 0..items.count-1 {
            if emitted[i]  continue;
            i_deps := deps_csr_slice(deps_flat, deps_offsets, i);
            ready := true;
            for dep : i_deps {
                found, j := table_find(*name_to_idx, dep);
                if found && j != i && !emitted[j] {
                    ready = false;
                    break;
                }
            }
            if !ready  continue;

            kills := 0;
            for dep : i_deps {
                found, j := table_find(*name_to_idx, dep);
                if found && remaining_uses[j] == 1  kills += 1;
            }
            if kills > best_kills {
                best_kills = kills;
                best_i = i;
                best_dep_count = i_deps.count;
            } else if kills == best_kills && live >= num_registers && i_deps.count < best_dep_count {
                best_i = i;
                best_dep_count = i_deps.count;
            }
        }
        if best_i == -1  break;  // Cycle guard.

        emitted[best_i] = true;
        remaining -= 1;
        live += 1; // best_i's own value is now live
        for dep : deps_csr_slice(deps_flat, deps_offsets, best_i) {
            found, j := table_find(*name_to_idx, dep);
            if found {
                remaining_uses[j] -= 1;
                if remaining_uses[j] == 0  live -= 1; // that dep just died
            }
        }
        if live > peak_live  peak_live = live;
        array_add(*order, best_i);
    }
    return order, peak_live;
}

// Classify every item in a topologically-sorted region against a chosen
// basis, propagating through the chain the same way the outer/inner CSE
// split already does for loop-variance. `other_variant_names` is anything
// sample-variant but not part of the basis.
classify_region_for_basis :: (items: [] Gen_Var, exprs: [] Expr, basis_names: [] string, other_variant_names: [] string) -> [] Basis_Class {
    classified: Table(string, Basis_Class);
    classified.allocator = temp;
    result: [..] Basis_Class;
    for item : items {
        cls := classify_expr_for_basis(item.expr_id, exprs, basis_names, other_variant_names, classified);
        table_add(*classified, item.name, cls);
        array_add(*result, cls);
    }
    return result;
}

// After extracting `accepted` terminals out of `items`, returns the
// transitive closure of items still needed for the straight-line remainder.
remaining_region_after_extraction :: (all_terminal_names: [] string, accepted: [] string, items: [] Gen_Var, exprs: [] Expr) -> [] Gen_Var {
    accepted_set: Table(string, bool);
    accepted_set.allocator = temp;
    for a : accepted  table_add(*accepted_set, a, true);

    remaining_terminal_names: [..] string;
    remaining_terminal_names.allocator = temp;
    for n : all_terminal_names  if !table_contains(*accepted_set, n)  array_add(*remaining_terminal_names, n);

    closure := collect_region_closure(remaining_terminal_names, items, exprs);
    result: [..] Gen_Var;
    for it : closure  if !table_contains(*accepted_set, it.name)  array_add(*result, it);
    return result;
}

// Computes "straight-line" cost vs. "extracted" cost.
// Accounts for register spill, and accounts for inner-loop
// vs. outer-loop ops based on ASSUMED_BLOCK_SIZE.
should_extract :: (region_items: [] Gen_Var, terminal_names: [] string, basis_names: [] string, exprs: [] Expr, backend: Backend) -> bool {
    if basis_names.count == 0 || region_items.count == 0  return false;

    region_flops := count_flops(region_items, exprs, backend.transcendental_approx);
    region_cost  := estimate_flop_cost(region_flops, backend.cpu);

    _, peak_live := schedule_for_register_pressure(region_items, exprs, backend.cpu.num_registers);
    spill_count := max(0, peak_live - backend.cpu.num_registers);
    spill_cost  := cast(float) spill_count * (backend.cpu.load.latency + backend.cpu.store.latency);

    basis_count := basis_names.count;
    straightline_cost := region_cost + spill_cost;
    // Per-sample dot product: basis_count mul-adds (c0 + c[0]*b0 + ... +
    // c[n-1]*b[n-1] has exactly basis_count '+' ops). See emit_extracted_region.
    extracted_cost := (cast(float) (basis_count + 1)) * region_cost / ASSUMED_BLOCK_SIZE
                     + cast(float) (terminal_names.count * basis_count) * (backend.cpu.mul.latency + backend.cpu.add.latency);

    return extracted_cost < straightline_cost;
}

// Gradual extraction: Greedily grows an accepted set one terminal at
// a time, most individually expensive first, keeping each addition only
// if should_extract still favors the resulting bundle. A big terminal
// can single-handedly justify amortizing the basis loop for several small
// ones riding along with it, but a lone cheap terminal in an otherwise-cheap
// region won't force anything to be extracted just because it's technically linear.
select_extraction_terminals :: (eligible: [] string, full_items: [] Gen_Var, exprs: [] Expr, basis_names: [] string, backend: Backend) -> [] string {
    costs: [..] float;
    costs.allocator = temp;
    for t : eligible {
        solo_closure := collect_region_closure(.[t], full_items, exprs);
        flops := count_flops(solo_closure, exprs, backend.transcendental_approx);
        array_add(*costs, estimate_flop_cost(flops, backend.cpu));
    }

    order: [..] int;
    order.allocator = temp;
    for i : 0..eligible.count-1  array_add(*order, i);
    for i : 1..order.count-1 { // insertion sort by cost descending; eligible.count is always small
        j := i;
        while j > 0 && costs[order[j]] > costs[order[j-1]] {
            tmp := order[j]; order[j] = order[j-1]; order[j-1] = tmp;
            j -= 1;
        }
    }

    accepted: [..] string;
    for idx : order {
        trial: [..] string;
        for a : accepted  array_add(*trial, a);
        array_add(*trial, eligible[idx]);

        trial_closure := collect_region_closure(trial, full_items, exprs);
        if should_extract(trial_closure, trial, basis_names, exprs, backend)  accepted = trial;
    }
    return accepted;
}

// Classifies `items` against the basis, restricts eligibility to whichever
// of `candidate_names` come out linear-in-basis, and greedily selects the
// accepted subset via select_extraction_terminals. Returns the accepted
// terminal names and their transitive closure over `items` (both empty if
// extraction is disabled or nothing gets accepted).
try_extract_region :: (items: [] Gen_Var, candidate_names: [] string, exprs: [] Expr, backend: Backend,
                       basis_names: [] string, other_variant_names: [] string) -> (accepted: [] string, region_items: [] Gen_Var) {
    if !backend.enable_extraction || items.count == 0 || basis_names.count == 0  return .[], .[];

    classes := classify_region_for_basis(items, exprs, basis_names, other_variant_names);
    eligible: [..] string;
    eligible.allocator = temp;
    for it, i : items {
        if !classes[i].depends || !classes[i].linear  continue;
        is_candidate := false;
        for c : candidate_names  if it.name == c { is_candidate = true; break; }
        if is_candidate  array_add(*eligible, it.name);
    }
    if eligible.count == 0  return .[], .[];

    accepted := select_extraction_terminals(eligible, items, exprs, basis_names, backend);
    if accepted.count == 0  return .[], .[];

    return accepted, collect_region_closure(accepted, items, exprs);
}

// Emits a block-level basis-vector coefficient loop, plus per-sample
// dot-product code (inner_builder) for `terminal_names`, whose closure over
// `region_items` is affine (not necessarily *homogeneous* since many real
// circuits have a nonzero DC/bias offset) in `basis_names`. `region_items`
// must already be topologically sorted.
//
// Pure unit-basis-vector superposition (evaluate at each e_i, sum) only
// reconstructs a HOMOGENEOUS function (f(0) = 0); it silently gives the
// wrong answer the moment there's a constant term, since that offset
// would get counted once per nonzero input instead of once total. So
// this runs basis_count+1 evaluations: _k=0 with every basis var at 0
// to capture the offset alone, then one per unit vector.
emit_extracted_region :: (region_items: [] Gen_Var, terminal_names: [] string, exprs: [] Expr,
                          basis_names: [] string, outer_builder: *String_Builder, inner_builder: *String_Builder, backend: Backend) {
    basis_count := basis_names.count;

    for t : terminal_names {
        print_to_builder(outer_builder, "%1 c0_%2;\n", backend.type_name, t);
        print_to_builder(outer_builder, "%1 c_%2[%3];\n", backend.type_name, t, basis_count);
    }
    append(outer_builder, "\n");

    print_to_builder(outer_builder, "for (int _k = 0; _k <= %; ++_k)\n{\n", basis_count);
    k_builder: String_Builder;
    literal_suffix := ifx backend.type_name == "float" then "f" else "";
    one  := tprint("1.0%", literal_suffix);
    zero := tprint("0.0%", literal_suffix);
    for bi : 0..basis_count-1 {
        // _k == 0 is the all-zero offset evaluation; basis[bi] goes to 1 at _k == bi+1.
        print_to_builder(*k_builder, "const %1 %2 = (_k == %3) ? %4 : %5;\n", backend.decl_type, basis_names[bi], bi + 1, one, zero);
    }
    emit_topological_order(region_items, exprs, *k_builder, backend);
    append(*k_builder, "if (_k == 0) {\n");
    for t : terminal_names  print_to_builder(*k_builder, "    c0_%1 = %1;\n", t);
    append(*k_builder, "} else {\n");
    for t : terminal_names  print_to_builder(*k_builder, "    c_%1[_k - 1] = %1 - c0_%1;\n", t);
    append(*k_builder, "}\n");

    append(outer_builder, indent(builder_to_string(*k_builder)));
    append(outer_builder, "}\n\n");

    for t : terminal_names {
        print_to_builder(inner_builder, "const %1 %2 = c0_%3", backend.decl_type, t, t);
        for bi : 0..basis_count-1 {
            print_to_builder(inner_builder, " + c_%1[%2] * %3", t, bi, basis_names[bi]);
        }
        append(inner_builder, ";\n");
    }
}

// Generate code for the given gen_vars and CSE temporaries.
//
// Returns two strings:
//   outer_code: loop-invariant CSE temporaries (hoist outside the sample loop)
//   inner_code: loop-variant CSE temps + gen_var assignments + state updates
// `extra_variant_names` lets a caller add more loop-variant names on top of
// build_variant_base(). The nonlinear pipeline uses this to mark junction
// voltages (vD1, vBCQ1, ...) as sample-variant.
codegen :: (elements: [] Element, inputs: [] string,
            gen_vars: [] Gen_Var, cse_vars: [] CSE_Var,
            exprs: [] Expr, backend: Backend = .{},
            extra_variant_names: [] string = .[]) -> (outer_code: string, inner_code: string, outer_flops: Flop_Count, inner_flops: Flop_Count, outer_temp_count: int, inner_temp_count: int) {

    variant_base := build_variant_base(elements, inputs);
    // Gen_vars are computed inside the per-sample loop; treat their names as
    // loop-variant base symbols so that any CSE temp depending on them is
    // correctly classified as loop-variant.
    for gv : gen_vars  array_add(*variant_base, gv.name);
    for n : extra_variant_names  array_add(*variant_base, n);

    // Accumulate names of CSE temps that are loop-variant so we can propagate
    // variant-ness through the (topologically ordered) temp list.
    variant_cse_names: [..] string;

    outer_builder: String_Builder;
    inner_builder: String_Builder;

    // Collect loop-variant CSE temps + gen_vars (inner) and loop-invariant CSE
    // temps (outer) separately; both will be emitted in topological order so
    // that every temporary is declared after all the items it depends on.
    outer_items: [..] Gen_Var;
    inner_cse_items: [..] Gen_Var;

    for cv : cse_vars {
        if expr_is_loop_variant(cv.expr_id, exprs, variant_base, variant_cse_names) {
            array_add(*variant_cse_names, cv.name);
            array_add(*inner_cse_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
        } else {
            array_add(*outer_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
        }
    }

    // Emit outer_items (block-invariant CSE) first, so the extraction
    // basis can reference them by name.
    emit_topological_order(outer_items, exprs, *outer_builder, backend);

    active_gen_vars: [..] Gen_Var;
    for gv : gen_vars  array_add(*active_gen_vars, gv);

    outer_flops: Flop_Count;
    inner_flops: Flop_Count;

    // Basis-vector extraction: among the gen_vars, find whichever are linear
    // in inputs/z-state/branch currents and, if the cost model says it's worth
    // it, replace their per-sample computation with a once-per-block coefficient
    // loop + a per-sample dot product. Everything not extracted  falls through
    // to the straight-line path below unchanged.
    if backend.enable_extraction {
        full_inner: [..] Gen_Var;
        for it : inner_cse_items  array_add(*full_inner, it);
        for gv : active_gen_vars  array_add(*full_inner, gv);

        basis_names := extraction_basis_names(elements, inputs);
        non_basis_variant_names: [..] string;
        for n : extra_variant_names  array_add(*non_basis_variant_names, n);
        for n : nonlinear_branch_current_names(elements)  array_add(*non_basis_variant_names, n);

        // Only the named outputs (gen_vars) are extraction candidates.
        gen_var_names: [..] string;
        for gv : gen_vars  array_add(*gen_var_names, gv.name);

        accepted, region_items := try_extract_region(full_inner, gen_var_names, exprs, backend, basis_names, non_basis_variant_names);
        if accepted.count > 0 {
            emit_extracted_region(region_items, accepted, exprs, basis_names, *outer_builder, *inner_builder, backend);

            // Straight-line remainder = closure of whatever gen_vars didn't get
            // accepted, recomputed from scratch.
            remaining_closure := remaining_region_after_extraction(gen_var_names, accepted, full_inner, exprs);

            new_inner_cse: [..] Gen_Var;
            new_gen_vars: [..] Gen_Var;
            for it : remaining_closure {
                is_gen_var := false;
                for gv : gen_vars  if gv.name == it.name { is_gen_var = true; break; }
                if is_gen_var  array_add(*new_gen_vars, it); else  array_add(*new_inner_cse, it);
            }
            inner_cse_items = new_inner_cse;
            active_gen_vars = new_gen_vars;

            // Best-effort stats: the coefficient loop is a one-time
            // (per-block) cost, folded into outer_flops; the per-sample
            // cost becomes a dot product instead of the extracted region.
            region_flops := count_flops(region_items, exprs, backend.transcendental_approx);
            add_flop_count(*outer_flops, region_flops);
            inner_flops.muls += accepted.count * basis_names.count;
            inner_flops.adds += accepted.count * basis_names.count;
        }
    }

    // Temp count: gen_vars aren't temporaries.
    outer_temp_count := outer_items.count;
    inner_temp_count := inner_cse_items.count;

    inner_items: [..] Gen_Var;
    for it : inner_cse_items  array_add(*inner_items, it);
    for gv : active_gen_vars  array_add(*inner_items, gv);

    // Count FLOPs for both groups (added on top of whatever extraction
    // above already accounted for). outer_items were already emitted above.
    add_flop_count(*outer_flops, count_flops(outer_items, exprs, backend.transcendental_approx));
    add_flop_count(*inner_flops, count_flops(inner_items, exprs, backend.transcendental_approx));

    emit_topological_order(inner_items, exprs, *inner_builder, backend);
    print_to_builder(*inner_builder, "\n");

    for el : elements {
        if el.type == {
            case .CAPACITOR;
                print_to_builder(*inner_builder, "z%1 = 2 * t%1 - z%1;\n", el.name);
                inner_flops.muls += 1;  inner_flops.adds += 1;  // 2*t, minus
            case .INDUCTOR;
                print_to_builder(*inner_builder, "z%1 = 2 * t%1 + z%1;\n", el.name);
                inner_flops.muls += 1;  inner_flops.adds += 1;  // 2*t, plus
            case .RC_SERIES;
                print_to_builder(*inner_builder, "z%1 = g%1 * (1 - gn%1) * v%1 + gn%1 * z%1; // RC series\n", el.name);
                inner_flops.muls += 3;  inner_flops.adds += 2;  // 3 muls, (1-gn), plus
            case .RC_PARALLEL;
                print_to_builder(*inner_builder, "z%1 = gz%1 * v%1 - z%1; // RC parallel\n", el.name);
                inner_flops.muls += 1;  inner_flops.adds += 1;  // gz*v, minus
            case .AMP;
                // Captures this sample's converged output for next
                // sample's slew comparison.
                if el.opamp.has_slew {
                    print_to_builder(*inner_builder, "zslew_%1 = vclip_%1;\n", el.name);
                }
        }
    }

    return builder_to_string(*outer_builder), builder_to_string(*inner_builder), outer_flops, inner_flops, outer_temp_count, inner_temp_count;
}

solve_and_codegen :: (elements: *[..] Element, inputs: [] string, outputs: [] string,
                      values: [] Element_Value = .[], backend: Backend = .{}, label: string = "") -> (outer_code: string, inner_code: string, outer_flops: Flop_Count, inner_flops: Flop_Count, outer_temp_count: int, inner_temp_count: int) {
    reduce_circuit(elements, inputs, outputs);

    solve_vars, equations, pool, kcl_elements := generate_kcl_equations(elements.*, inputs);

    check := build_magnitude_check(elements.*, values, backend, label);
    solutions := solve(equations, solve_vars, *pool, check);

    // kcl_elements (not elements.*): op-amp-merged nodes (e.g. vpos -> vneg)
    // only have their aliased terminal there.
    gen_vars := generate_expressions(kcl_elements, outputs, solve_vars, solutions, *pool);

    // Collect gen_var expression IDs for CSE.
    gen_var_ids: [..] Expr_ID;
    for gv : gen_vars  array_add(*gen_var_ids, gv.expr_id);

    cse_vars := cse(gen_var_ids, *pool);
    sort_cse_topological(*cse_vars, pool.exprs);  // ensure deps appear before dependents

    // Build the variant base for LICM (inputs + state vars + gen_var names).
    licm_variant_base := build_variant_base(kcl_elements, inputs);
    for gv : gen_vars  array_add(*licm_variant_base, gv.name);
    licm(gen_var_ids, *cse_vars, *pool, licm_variant_base);
    recip_hoist(gen_var_ids, *cse_vars, *pool, licm_variant_base);

    outer_code, inner_code, outer_flops, inner_flops, outer_temp_count, inner_temp_count := codegen(kcl_elements, inputs, gen_vars, cse_vars, pool.exprs, backend);
    return outer_code, inner_code, outer_flops, inner_flops, outer_temp_count, inner_temp_count;
}
