// Everything that turns a solved circuit (Element list + closed-form MNA/
// Newton expressions) into C/C++ text: the pre-emission orchestration
// (CSE-temp classification, topological emission, FLOP counting) and every
// `generate_*` string emitter (Params/State structs, impedance defs, limiter
// helpers, vcrit/newton constants, per-cluster Newton code, reset()).
//
// Sits above nonlinear_solve.jai in the dependency DAG (sym_algebra <-
// nodal_analysis <- nonlinear_solve <- codegen <- netlist_codegen) -- it
// consumes Nonlinear_Cluster etc. but nothing below it depends on codegen.

#import "Basic";

#import "sym_algebra";
#import "nodal_analysis";
#import "nonlinear_solve";

// Re-indents a multi-line code block by `prefix` (used when splicing one
// generated block into another, e.g. impedance defs into the outer loop).
indent :: (in: string, prefix := "    ") -> string {
    if in.count == 0  return in;
    ends_with_newline := in[in.count - 1] == #char "\n";
    out := String.replace(in, "\n", tprint("\n%", prefix));
    out = tprint("%1%2", prefix, out);
    if ends_with_newline  out.count -= prefix.count;
    return out;
}

// Every language-specific decision the emitters below need computed once only.
// `Backend.{}` still means "C++, float" for callers that don't build one via make_backend().
Backend :: struct {
    is_cpp:    bool   = true;
    type_name: string = "float";     // numeric type used for variable declarations
    decl_type: string = "auto";      // "auto" (C++, type inference) or type_name (C)
    const_kw:  string = "static constexpr"; // vs "static const" (C)
}

make_backend :: (lang: string, type_name: string) -> Backend {
    is_cpp := lang != "c";
    return .{
        is_cpp    = is_cpp,
        type_name = type_name,
        decl_type = ifx is_cpp then "auto" else type_name,
        const_kw  = ifx is_cpp then "static constexpr" else "static const",
    };
}

// C++: "std::<name>"; C: "<name>". Covers every libm function the emitters
// below call (log, sqrt, abs, ...).
math_fn :: (backend: Backend, name: string) -> string {
    return ifx backend.is_cpp then tprint("std::%", name) else name;
}

math_include :: (backend: Backend) -> string {
    return ifx backend.is_cpp then "#include <cmath>\n\n" else "#include <math.h>\n\n";
}

// Opening line for a Params/State struct. C++ names the struct up front;
// a C typedef struct is named on the closing line instead (see struct_close).
struct_open :: (backend: Backend, name: string) -> string {
    return ifx backend.is_cpp then tprint("struct % {\n", name) else "typedef struct {\n";
}

struct_close :: (backend: Backend, name: string) -> string {
    return ifx backend.is_cpp then "};\n" else tprint("} %;\n", name);
}

// A struct field with no initializer. C++ zero-inits with `{}`, C just declares it.
field_decl :: (backend: Backend, type_name: string, name: string) -> string {
    return ifx backend.is_cpp then tprint("    % % {};\n", type_name, name) else tprint("    % %;\n", type_name, name);
}

// A struct field with a literal initial value. C structs can't have default member
// initializers, so the value is recorded as a comment instead.
field_decl_with_value :: (backend: Backend, type_name: string, name: string, value_str: string) -> string {
    if backend.is_cpp  return tprint("    %1 %2 = %3;\n", type_name, name, value_str);
    return tprint("    % %; // = %\n", type_name, name, value_str);
}

// A function parameter with a default value - C doesn't support default
// parameter values, C++ does.
param_decl_with_default :: (backend: Backend, type_name: string, name: string, default_str: string) -> string {
    if backend.is_cpp  return tprint(", %1 %2 = %3", type_name, name, default_str);
    return tprint(", %1 %2", type_name, name);
}

// sum/recip_sum helpers referenced by generate_impedance_defs - C++ lambdas,
// C macros (C has no closures, and these never capture anything anyway).
sum_recip_sum_defs :: (backend: Backend) -> string {
    if backend.is_cpp {
        return "[[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };\n[[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };\n\n";
    }
    return "#define sum(a, b) ((a) + (b))\n#define recip_sum(a, b) ((a) * (b) / ((a) + (b)))\n\n";
}

// Counts of floating-point operations in a block of generated code.
Flop_Count :: struct {
    muls: int;
    divs: int;
    adds: int; // ADD, SUB, and NEG all counted here
    transcendentals: int; // EXP, LOG, POW
}

// Recursively walk an expression tree and accumulate op counts into `result`.
// Stops at SYMBOL and CONST leaves; does not cross into sub-expressions that
// are already named CSE variables (they appear as SYMBOL nodes).
count_flops_expr :: (e: Expr_ID, exprs: [] Expr, result: *Flop_Count) {
    if e == NO_EXPR  return;
    kind := exprs[e].kind;
    if      kind == .MUL                                    result.muls += 1;
    else if kind == .DIV                                    result.divs += 1;
    else if kind == .ADD || kind == .SUB || kind == .NEG    result.adds += 1;
    else if kind == .EXP || kind == .LOG || kind == .POW    result.transcendentals += 1;

    if kind == .ADD || kind == .SUB || kind == .MUL || kind == .DIV || kind == .POW {
        count_flops_expr(exprs[e].left,  exprs, result);
        count_flops_expr(exprs[e].right, exprs, result);
    } else if kind == .NEG || kind == .EXP || kind == .LOG {
        count_flops_expr(exprs[e].left, exprs, result);
    }
}

count_flops :: (items: [] Gen_Var, exprs: [] Expr) -> Flop_Count {
    result: Flop_Count;
    for item : items  count_flops_expr(item.expr_id, exprs, *result);
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

generate_params_struct :: (elements: [] Element,
                           values: [] Element_Value,
                           backend: Backend = .{}) -> string {
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

    builder: String_Builder;
    append(*builder, struct_open(backend, "Params"));

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
            case .BJT; #through;
            case .JFET; #through;
            case .TRIODE;
                for pname : device_model_param_names(el.type, el.nonlinear)  emit_nonlinear_param(pname);
            case .AMP;
                // Rin/Rout/GBW have no fields of their own here -- they're
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

    append(*builder, struct_close(backend, "Params"));
    return builder_to_string(*builder);
}

// `extra_state_names` holds nonlinear-cluster junction-voltage unknowns
// (Nonlinear_Cluster.unknowns[].name), which persist across samples as the
// per-sample Newton-Raphson warm start.
generate_state_struct :: (elements: [] Element, extra_state_names: [] string, backend: Backend = .{}) -> string, [] string {
    states: [..] string;

    builder: String_Builder;
    append(*builder, struct_open(backend, "State"));

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

    append(*builder, struct_close(backend, "State"));
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

// Junction-voltage step limiter (like ngspice's DEVpnjlim): caps the
// per-iteration step once v exceeds vcrit, so exp(v/vt) can't overflow.
// Emitted once per header.
generate_junction_limiter_helper :: (backend: Backend) -> string {
    if !backend.is_cpp {
        type_name := backend.type_name;
        template := #string DONE
static % limit_junction_voltage(% v_new, % v_old, % vt, % vcrit) {
    if (v_new > vcrit && fabs(v_new - v_old) > 2 * vt) {
        if (v_old > 0) {
            % arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * log(arg) : vcrit;
        } else {
            v_new = vt * log(v_new / vt);
        }
    } else if (v_new < -vcrit && fabs(v_new - v_old) > 2 * vt) {
        if (v_old < 0) {
            % arg = 1 + (v_old - v_new) / vt;
            v_new = arg > 0 ? v_old - vt * log(arg) : -vcrit;
        } else {
            v_new = -vt * log(-v_new / vt);
        }
    }
    return v_new;
}
DONE
        return tprint(template, type_name, type_name, type_name, type_name, type_name, type_name, type_name);
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
    else if (v_new < -vcrit && std::abs(v_new - v_old) > 2 * vt)
    {
        if (v_old < 0)
        {
            const auto arg = 1 + (v_old - v_new) / vt;
            v_new = arg > 0 ? v_old - vt * std::log(arg) : -vcrit;
        }
        else
        {
            v_new = -vt * std::log(-v_new / vt);
        }
    }
    return v_new;
};
DONE;
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
            if u.limiter_kind != .EXP_JUNCTION  continue;
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
    if u.limiter_kind == {
        case .EXP_JUNCTION;
            print_to_builder(builder, "%1 = limit_junction_voltage(%1 + (%2), %1, %3, vcrit_%3);\n",
                             u.name, delta_expr, u.vt_name);
        case .JFET_PINCHOFF;
            print_to_builder(builder, "%1 = limit_jfet_vgs(%1 + (%2), %3);\n", u.name, delta_expr, u.vp_name);
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
    v_prev := ifx c.has_slew && apply_slew then c.zslew_name else "_natural";
    slew_max_step := ifx c.has_slew && apply_slew then tprint("(%/sample_rate)", c.slew_name) else "1.0e30";

    print_to_builder(builder, "const %1 _natural = (%1)(%2 + %3);\n", backend.type_name, c.name, c.delta_name);
    print_to_builder(builder, "%1 _v = _natural;\n", backend.decl_type);
    print_to_builder(builder, "if (_v - %1 > %2)  _v = %1 + %2;\n", v_prev, slew_max_step);
    print_to_builder(builder, "if (_v - %1 < -%2) _v = %1 - %2;\n", v_prev, slew_max_step);

    print_to_builder(builder, "%1 res_%2_active = 0.0, delta_%2_active = 0.0;\n", backend.decl_type, c.name);
    for u : others  print_to_builder(builder, "%1 res_%2_active = 0.0, delta_%2_active = 0.0;\n", backend.decl_type, u.name);

    print_to_builder(builder, "if (_v > %1) {\n", c.vsat_p_name);
    print_to_builder(builder, "    delta_%1_active = %2 - %1;\n    %1 = %2;\n", c.name, c.vsat_p_name);
    for u : others  print_to_builder(builder, "    res_%1_active = %2; delta_%1_active = %3;\n", u.name, u.residual_name_hi, u.delta_name_hi);
    print_to_builder(builder, "} else if (_v < %1) {\n", c.vsat_n_name);
    print_to_builder(builder, "    delta_%1_active = %2 - %1;\n    %1 = %2;\n", c.name, c.vsat_n_name);
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

// Newton-Raphson solve for one cluster, to be inserted inside the per-sample loop.
generate_cluster_newton_code :: (cluster: Nonlinear_Cluster, exprs: [] Expr, backend: Backend, apply_slew := true) -> string {
    builder: String_Builder;
    print_to_builder(*builder, "// --- Newton-Raphson solve: %\n", cluster.name);

    emit_topological_order(cluster.invariant_items, exprs, *builder, backend);
    if cluster.amp_clip_idx >= 0 {
        emit_topological_order(cluster.hi_invariant_items, exprs, *builder, backend);
        emit_topological_order(cluster.lo_invariant_items, exprs, *builder, backend);
    }

    append(*builder, "for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)\n{\n");
    inner_builder: String_Builder;
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
            if u.limiter_kind == .AMP_CLIP {
                // Plain post-step clamp, only reached for 0 or 2+ AMP_CLIP
                // unknowns per cluster, which is unsupported by the active-set
                // path above. Every real circuit here has amp_clip_idx >= 0 instead,
                // so this is untested against an actual C fixture.
                v_prev := ifx u.has_slew && apply_slew then u.zslew_name else "_v";
                slew_max_step := ifx u.has_slew && apply_slew then tprint("(%/sample_rate)", u.slew_name) else "1.0e30";
                print_to_builder(*inner_builder, "const %8 _v = (%5)(%1 + %2);\n%1 = clamp_opamp_output(_v, %6, %7, %3, %4);\n",
                                 u.name, u.delta_name, u.vsat_n_name, u.vsat_p_name, backend.type_name, v_prev, slew_max_step, backend.decl_type);
            } else {
                emit_unknown_update(*inner_builder, u, u.delta_name);
            }
        }
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

// Builds and solves the DC-equivalent circuit (see generate_kcl_equations
// dc_mode), and derives the steady-state z-value for each reactive element.
build_dc_operating_point :: (elements: [] Element, inputs: [] string) -> (clusters: [] Nonlinear_Cluster, pool: Expr_Pool, z_init_vars: [] Gen_Var) {
    dc_solve_vars, dc_equations, dc_pool, dc_elements := generate_kcl_equations(elements, inputs, true);
    dc_solutions := solve(dc_equations, dc_solve_vars, *dc_pool);

    dc_clusters := build_nonlinear_clusters(elements, dc_solve_vars, dc_solutions, *dc_pool);

    z_init_vars: [..] Gen_Var;
    for el : dc_elements {
        if el.type != .CAPACITOR && el.type != .RC_SERIES && el.type != .RC_PARALLEL  continue;

        // copy_expr: pos_v/neg_v can alias a solution node shared by another
        // reactive element on the same terminal -- simplify() below mutates
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

    return dc_clusters, dc_pool, z_init_vars;
}

// Emits a `reset()` function that solves the DC operating point analytically
// and writes it into every channel's State. Each input gets its own
// `<input>_dc` parameter (the value it's held at for DC analysis) -- solved
// once and copied to all `num_channels` states.
generate_reset_function :: (elements: [] Element, inputs: [] string, backend: Backend) -> string {
    dc_clusters, dc_pool, z_init_vars := build_dc_operating_point(elements, inputs);

    builder: String_Builder;
    append(*builder, "static void reset (Params params, State* state, int num_channels, float sample_rate");
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

    for c : dc_clusters {
        newton_code := generate_cluster_newton_code(c, dc_pool.exprs, backend, apply_slew = false);
        append(*builder, indent(newton_code));
    }

    for zv : z_init_vars {
        print_to_builder(*builder, "    const %1 %2 = %3;\n", backend.decl_type, zv.name, print_expr(zv.expr_id, dc_pool.exprs));
    }
    if z_init_vars.count > 0  append(*builder, "\n");

    append(*builder, "    for (int ch = 0; ch < num_channels; ++ch)\n    {\n");
    for c : dc_clusters {
        for u : c.unknowns {
            print_to_builder(*builder, "        state[ch].%1 = %1;\n", u.name);
            // zslew_<name> starts at the same DC-settled value.
            if u.has_slew  print_to_builder(*builder, "        state[ch].%1 = %2;\n", u.zslew_name, u.name);
        }
    }
    for zv : z_init_vars  print_to_builder(*builder, "        state[ch].%1 = %1;\n", zv.name);
    append(*builder, "    }\n");

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

// Build the set of base loop-variant symbol names: inputs + state variables
// + nonlinear branch currents (free per-sample symbols, not solved-for MNA
// unknowns -- see generate_kcl_equations).
build_variant_base :: (elements: [] Element, inputs: [] string) -> [..] string {
    names: [..] string;
    for inp : inputs  array_add(*names, inp);
    for el : elements {
        if el.type == .CAPACITOR || el.type == .INDUCTOR
           || el.type == .RC_SERIES || el.type == .RC_PARALLEL {
            array_add(*names, tprint("z%", el.name));
        }
        spec := nonlinear_device_spec(el.type);
        for term : spec.kcl_terms {
            array_add_if_unique(*names, tprint("i%1%2", term.branch_suffix, el.name));
        }
    }
    return names;
}

// Emit items into builder as `const auto name = expr;` lines in topological
// order: each item is written only after all inner-loop symbol dependencies it
// references have already been written.
emit_topological_order :: (items: [] Gen_Var, exprs: [] Expr, builder: *String_Builder, backend: Backend = .{}) {
    decl_type := backend.decl_type;
    emitted: [..] bool;
    for _ : 0..items.count-1  array_add(*emitted, false);

    remaining := items.count;
    while remaining > 0 {
        made_progress := false;
        for i : 0..items.count-1 {
            if emitted[i]  continue;
            deps: [..] string;
            collect_symbol_names(items[i].expr_id, exprs, *deps);
            ready := true;
            for dep : deps {
                for j : 0..items.count-1 {
                    if j != i && items[j].name == dep && !emitted[j] {
                        ready = false;
                        break;
                    }
                }
                if !ready  break;
            }
            if ready {
                emitted[i]  = true;
                remaining  -= 1;
                made_progress = true;
                print_to_builder(builder, "const % % = %;\n",
                                 decl_type, items[i].name, print_expr(items[i].expr_id, exprs));
            }
        }
        if !made_progress  break;  // Cycle guard.
    }
}

// Generate code for the given gen_vars and CSE temporaries.
//
// Returns two strings:
//   outer_code — loop-invariant CSE temporaries (hoist outside the sample loop)
//   inner_code — loop-variant CSE temps + gen_var assignments + state updates
// `extra_variant_names` lets a caller add more loop-variant names on top of
// build_variant_base() -- the nonlinear pipeline uses this to mark junction
// voltages (vD1, vBCQ1, ...) as sample-variant.
codegen :: (elements: [] Element, inputs: [] string,
            gen_vars: [] Gen_Var, cse_vars: [] CSE_Var,
            exprs: [] Expr, backend: Backend = .{},
            extra_variant_names: [] string = .[]) -> (outer_code: string, inner_code: string, outer_flops: Flop_Count, inner_flops: Flop_Count) {

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
    inner_items: [..] Gen_Var;

    for cv : cse_vars {
        if expr_is_loop_variant(cv.expr_id, exprs, variant_base, variant_cse_names) {
            array_add(*variant_cse_names, cv.name);
            array_add(*inner_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
        } else {
            array_add(*outer_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
        }
    }

    for gv : gen_vars {
        array_add(*inner_items, gv);
    }

    // Count FLOPs for both groups before emitting.
    outer_flops := count_flops(outer_items, exprs);
    inner_flops := count_flops(inner_items, exprs);

    // Emit both groups in topological order (dependencies before uses).
    emit_topological_order(outer_items, exprs, *outer_builder, backend);
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

    return builder_to_string(*outer_builder), builder_to_string(*inner_builder), outer_flops, inner_flops;
}

solve_and_codegen :: (elements: *[..] Element, inputs: [] string, outputs: [] string, backend: Backend = .{}) -> (outer_code: string, inner_code: string, outer_flops: Flop_Count, inner_flops: Flop_Count) {
    reduce_circuit(elements, inputs, outputs);

    solve_vars, equations, pool, kcl_elements := generate_kcl_equations(elements.*, inputs);

    solutions := solve(equations, solve_vars, *pool);

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

    outer_code, inner_code, outer_flops, inner_flops := codegen(kcl_elements, inputs, gen_vars, cse_vars, pool.exprs, backend);
    return outer_code, inner_code, outer_flops, inner_flops;
}
