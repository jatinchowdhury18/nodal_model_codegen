#import "Basic";
#import "sym_algebra";

Element_Type :: enum {
    RESISTOR;
    CAPACITOR;
    INDUCTOR;
    AMP;
    FIXED_VOLTAGE;
    RC_SERIES;
    RC_PARALLEL;
    DIODE;       // 2-terminal: [anode, cathode]. Shockley I-V law.
    DIODE_PAIR;  // 2-terminal: [anode, cathode]. Antiparallel diode pair collapsed
                 // to one junction-voltage unknown via a closed-form sinh I-V law.
    BJT;         // 3-terminal: [collector, base, emitter]. Ebers-Moll I-V law.
    JFET;        // 3-terminal: [drain, gate, source]. Square-law I-V law.
}

Combo_Op_Type :: enum {
    NONE;       // leaf: use the element's own name as the param directly
    SUM;        // a + b  (series R+R, parallel C||C)
    RECIP_SUM;  // a*b/(a+b)  (series C+C, parallel R||R)
}

// Recursive expression tree for a combined parameter value.
// Leaf nodes (op == .NONE) hold a single param name.
// Branch nodes combine two sub-expressions.
Param_Expr :: struct {
    op:        Combo_Op_Type;
    leaf_name: string;       // valid only when op == .NONE
    left:      *Param_Expr;  // valid only when op != .NONE
    right:     *Param_Expr;  // valid only when op != .NONE
}

// Stores the R and C identity info for a combined RC element.
RC_Internal :: struct {
    res_name: string;
    res_expr: *Param_Expr;
    cap_name: string;
    cap_expr: *Param_Expr;
}

// Model-parameter references for a nonlinear (DIODE / DIODE_PAIR / BJT / JFET)
// element. Each `*_name` names a Params struct field (parsed from a SPICE
// `.model` card by netlist_codegen.jai), same as a RESISTOR's params.<name>.
Nonlinear_Model :: struct {
    is_name:    string; // saturation current, e.g. "D1_Is"
    vt_name:    string; // thermal voltage (already N/emission-coefficient-scaled), e.g. "D1_vt"
    betaf_name: string; // BJT only; empty string for DIODE / DIODE_PAIR
    betar_name: string; // BJT only; empty string for DIODE / DIODE_PAIR
    vp_name:    string; // JFET only; pinch-off voltage
    beta_name:  string; // JFET only; transconductance coefficient
}

// -----------------------------------------------------------------------
// Nonlinear device registry -- adding a new nonlinear element type means
// filling out one case in nonlinear_device_spec() (below), device_iv_law()
// (nonlinear_solve.jai) and device_model_param_names() (below), plus the
// SPICE line parsing in netlist_codegen.jai (not generalized here -- that
// part is inherently SPICE-grammar-specific, not device-physics-specific).
// -----------------------------------------------------------------------

// Per-iteration Newton-step limiter for one unknown -- caps a bad step
// before it wanders into a region the I-V law approximates badly.
Limiter_Kind :: enum {
    NONE;           // plain Newton step
    EXP_JUNCTION;   // diode/BJT-style: clamp near vcrit = vt*log(vt/(sqrt(2)*Is))
    JFET_PINCHOFF;  // JFET-style: clamp at Vp (see generate_jfet_limiter_helper)
    AMP_CLIP;       // op-amp output clip/slew: hard rail clamp (see generate_opamp_clip_limiter_helper)
}

// One Newton unknown (junction voltage) a device contributes: named
// `name_prefix + el.name`, valued terminals[pos_terminal] - terminals[neg_terminal].
Device_Unknown :: struct {
    name_prefix:  string;
    pos_terminal: int;
    neg_terminal: int;
    limiter:      Limiter_Kind;
}

// One term in a device's KCL contribution: the branch current named
// `i<branch_suffix><el.name>` (suffix "" for a single-branch device) either
// enters or leaves terminals[terminal_idx]. Terms sharing a terminal
// accumulate -- e.g. BJT's iE = iB + iC is two ADD terms on the emitter,
// not a pre-summed expression.
Device_Kcl_Term :: struct {
    terminal_idx:  int;
    branch_suffix: string;
    entering:      bool; // true: current enters this terminal (KCL .ADD); false: leaves (.SUB)
}

// Everything generic nonlinear-device code needs about one Element_Type.
// The actual I-V law stays a named function in nonlinear_solve.jai (that's
// real physics, not something worth generalizing away) -- see device_iv_law().
Nonlinear_Device_Spec :: struct {
    unknowns:  [] Device_Unknown;
    kcl_terms: [] Device_Kcl_Term;
}

// Empty for any non-nonlinear (or unhandled) type -- every generic pass
// below treats an empty spec as "this element contributes nothing."
nonlinear_device_spec :: (type: Element_Type) -> Nonlinear_Device_Spec {
    if type == {
        case .DIODE; #through;
        case .DIODE_PAIR;
            return .{
                unknowns  = Device_Unknown.[ .{ name_prefix = "v", pos_terminal = 0, neg_terminal = 1, limiter = .EXP_JUNCTION } ],
                kcl_terms = Device_Kcl_Term.[
                    .{ terminal_idx = 0, branch_suffix = "", entering = false }, // anode -= i
                    .{ terminal_idx = 1, branch_suffix = "", entering = true },  // cathode += i
                ],
            };
        case .BJT;
            return .{
                unknowns  = Device_Unknown.[
                    .{ name_prefix = "vBC", pos_terminal = 1, neg_terminal = 0, limiter = .EXP_JUNCTION },
                    .{ name_prefix = "vBE", pos_terminal = 1, neg_terminal = 2, limiter = .EXP_JUNCTION },
                ],
                kcl_terms = Device_Kcl_Term.[
                    .{ terminal_idx = 0, branch_suffix = "C", entering = false }, // collector -= iC
                    .{ terminal_idx = 1, branch_suffix = "B", entering = false }, // base -= iB
                    .{ terminal_idx = 2, branch_suffix = "B", entering = true },  // emitter += iB
                    .{ terminal_idx = 2, branch_suffix = "C", entering = true },  // emitter += iC
                ],
            };
        case .JFET;
            return .{
                unknowns  = Device_Unknown.[ .{ name_prefix = "vGS", pos_terminal = 1, neg_terminal = 2, limiter = .JFET_PINCHOFF } ],
                kcl_terms = Device_Kcl_Term.[
                    .{ terminal_idx = 0, branch_suffix = "", entering = false }, // drain -= iD
                    .{ terminal_idx = 2, branch_suffix = "", entering = true },  // source += iD
                ],
            };
    }
    empty: Nonlinear_Device_Spec;
    return empty;
}

// Given a limiter kind and a device's Nonlinear_Model, pick out the Params
// field names its Newton-step limiter needs (see generate_cluster_newton_code
// / generate_vcrit_defs in netlist_codegen.jai).
device_limiter_names :: (kind: Limiter_Kind, m: Nonlinear_Model) -> (vt_name: string, is_name: string, vp_name: string) {
    if kind == {
        case .EXP_JUNCTION;  return m.vt_name, m.is_name, "";
        case .JFET_PINCHOFF; return "", "", m.vp_name;
    }
    return "", "", "";
}

// Params struct field names a device's .model card populates -- used for
// both Params-struct emission and impedance-alias emission.
device_model_param_names :: (type: Element_Type, m: Nonlinear_Model) -> [..] string {
    names: [..] string;
    if type == {
        case .DIODE; #through;
        case .DIODE_PAIR;
            array_add(*names, m.is_name, m.vt_name);
        case .BJT;
            array_add(*names, m.is_name, m.vt_name, m.betaf_name, m.betar_name);
        case .JFET;
            array_add(*names, m.beta_name, m.vp_name);
    }
    return names;
}

// True if any element in `elements` has an unknown using this limiter kind
// -- used to decide which limiter helper functions to emit.
circuit_uses_limiter_kind :: (elements: [] Element, kind: Limiter_Kind) -> bool {
    for el : elements {
        spec := nonlinear_device_spec(el.type);
        for u : spec.unknowns  if u.limiter == kind  return true;

        // AMP_CLIP isn't in nonlinear_device_spec(): whether an AMP is
        // nonlinear depends on instance fields, not just Element_Type.
        if kind == .AMP_CLIP && el.type == .AMP && opamp_has_clip_stage(el)  return true;
    }
    return false;
}

// Non-ideal op-amp params, parsed from a `.model <name> OPAMP (...)` card.
// Each `has_*` flag is the actual opt-in. `*_name` fields name a Params struct
// field. Zero value == today's ideal op-amp.
Opamp_Model :: struct {
    has_aol:    bool; aol_name:    string; // open-loop gain
    has_gbw:    bool; gbw_name:    string; // gain-bandwidth product (dominant pole)
    has_rin:    bool; rin_name:    string; // differential input resistance
    has_rout:   bool; rout_name:   string; // output series resistance
    has_vsat_p: bool; vsat_p_name: string; // positive output rail
    has_vsat_n: bool; vsat_n_name: string; // negative output rail
    has_slew:   bool; slew_name:   string; // max |dVout/dt|, V/s
    has_vos:    bool; vos_name:    string; // input-referred offset voltage
    has_ios:    bool; ios_name:    string; // input offset current
    has_ibias:  bool; ibias_name:  string; // input bias current
}

// True if none of an AMP's non-ideal characteristics are set -- i.e. it's
// still today's ideal op-amp (structural node merge, no numeric model).
opamp_is_ideal :: (el: Element) -> bool {
    o := el.opamp;
    return !(o.has_aol || o.has_gbw || o.has_rin || o.has_rout || o.has_vsat_p
             || o.has_vsat_n || o.has_slew || o.has_vos || o.has_ios || o.has_ibias);
}

// Forces the numeric finite-gain VCVS path (a real "gain node" + hard
// constraint equation) instead of the structural pos/neg node merge.
needs_explicit_vcvs :: (el: Element) -> bool {
    o := el.opamp;
    return o.has_aol || o.has_rin || o.has_rout || o.has_vsat_p || o.has_vsat_n || o.has_slew || o.has_gbw;
}

// True if the op-amp has an output clipping/slew stage -- i.e. a Newton
// unknown (vclip_<name>) sits between the gain chain and Rout/out_node.
opamp_has_clip_stage :: (el: Element) -> bool {
    return el.opamp.has_vsat_p || el.opamp.has_vsat_n || el.opamp.has_slew;
}

// vgain_<name>: the ideal (infinite-bandwidth) gain-stage node, holding
// Aol*(pos - neg + Vos). Collapses to the real out_node directly when
// nothing else (GBW lowpass, clip/slew stage, Rout) follows it in the
// chain -- e.g. an Aol-only op-amp needs no synthetic nodes at all.
opamp_gain_node :: (el: Element) -> string {
    if el.opamp.has_gbw || opamp_has_clip_stage(el) || el.opamp.has_rout  return tprint("vgain_%", el.name);
    return el.terminals[0];
}

// One-pole-lowpassed version of vgain_<name> (meaningful only when
// has_gbw -- an internal Rcomp/Ccomp synthesized upstream). Collapses to
// vgain_<name> directly if there's no lowpass, or to out_node directly if
// there's a lowpass but nothing (clip/slew, Rout) follows it.
opamp_bandwidth_node :: (el: Element) -> string {
    if !el.opamp.has_gbw  return opamp_gain_node(el);
    if opamp_has_clip_stage(el) || el.opamp.has_rout  return tprint("vbw_%", el.name);
    return el.terminals[0];
}

// Clipped/slew-limited output (a Newton unknown, solved in
// nonlinear_solve.jai). Unlike gain/bandwidth above, never collapses to
// out_node: it's a free symbol with no KCL row of its own, so out_node gets
// an explicit equality constraint against it instead when Rout is absent.
opamp_clip_node :: (el: Element) -> string {
    if !opamp_has_clip_stage(el)  return opamp_bandwidth_node(el);
    return tprint("vclip_%", el.name);
}

Element :: struct {
    name: string;

    type: Element_Type;

    terminals: [..] string;

    // For same-type combined elements (R+R, C+C, R||R, C||C):
    // non-null when this element's parameter is a combo expression.
    combo_expr: *Param_Expr;  // null => use params.{name} directly

    // For RC_SERIES / RC_PARALLEL combined elements.
    rc_internal: RC_Internal;

    // For DIODE / DIODE_PAIR / BJT / JFET elements.
    nonlinear: Nonlinear_Model;

    // For AMP elements only. Zero value == today's ideal op-amp.
    opamp: Opamp_Model;

    // Set from a "; no_combine" inline netlist comment (R/C/D only) -- opts
    // this element out of series/parallel/antiparallel-pair reduction.
    no_combine: bool;
};

// Return a Param_Expr leaf for an element.
// If the element already carries a combo_expr, return that; otherwise make a fresh leaf.
get_param_expr :: (el: Element) -> *Param_Expr {
    if el.combo_expr  return el.combo_expr;
    expr := New(Param_Expr);
    expr.op = .NONE;
    expr.leaf_name = el.name;
    return expr;
}

make_combo_expr :: (op: Combo_Op_Type, left: *Param_Expr, right: *Param_Expr) -> *Param_Expr {
    expr := New(Param_Expr);
    expr.op    = op;
    expr.left  = left;
    expr.right = right;
    return expr;
}

// Build an RC_SERIES Element from a resistor element and a capacitor element,
// given the "other" terminal for each (the terminal NOT shared with the internal node).
make_rc_series_element :: (res_el: Element, res_other: string,
                           cap_el: Element, cap_other: string) -> Element {
    new_el: Element;
    new_el.name = tprint("%1%2", res_el.name, cap_el.name);
    new_el.type = .RC_SERIES;
    array_add(*new_el.terminals, res_other);
    array_add(*new_el.terminals, cap_other);
    new_el.rc_internal.res_name = res_el.name;
    new_el.rc_internal.res_expr = get_param_expr(res_el);
    new_el.rc_internal.cap_name = cap_el.name;
    new_el.rc_internal.cap_expr = get_param_expr(cap_el);
    return new_el;
}

// Attempt ONE series combination. Returns true if a combination was made.
// Series reduction: elements sharing a node that is not input, output, or ground,
// and that node has exactly two elements connected to it.
reduce_series_once :: (elements: *[..] Element, inputs: [] string, outputs: [] string) -> bool {
    all_nodes: [..] string;
    defer array_free(all_nodes);
    for el : elements.* {
        for terminal : el.terminals {
            array_add_if_unique(*all_nodes, terminal);
        }
    }

    for node : all_nodes {
        if node == "0" continue;
        if array_find(inputs, node)  continue;
        if array_find(outputs, node) continue;

        connected: [..] int;
        defer array_free(connected);
        for el, idx : elements.* {
            for terminal : el.terminals {
                if terminal == node {
                    array_add(*connected, idx);
                    break;
                }
            }
        }
        if connected.count != 2 continue;

        idx1 := connected[0];
        idx2 := connected[1];
        el1  := (elements.*)[idx1];
        el2  := (elements.*)[idx2];
        if el1.no_combine || el2.no_combine  continue;
        other1 := ifx el1.terminals[0] == node then el1.terminals[1] else el1.terminals[0];
        other2 := ifx el2.terminals[0] == node then el2.terminals[1] else el2.terminals[0];

        new_el: Element;
        combined := false;

        if el1.type == .RESISTOR && el2.type == .RESISTOR {
            print("Combining series resistors: % + %\n", el1.name, el2.name);
            new_el.name = tprint("%1%2", el1.name, el2.name);
            new_el.type = .RESISTOR;
            array_add(*new_el.terminals, other1);
            array_add(*new_el.terminals, other2);
            new_el.combo_expr = make_combo_expr(.SUM, get_param_expr(el1), get_param_expr(el2));
            combined = true;
        } else if el1.type == .CAPACITOR && el2.type == .CAPACITOR {
            print("Combining series capacitors: % + %\n", el1.name, el2.name);
            new_el.name = tprint("%1%2", el1.name, el2.name);
            new_el.type = .CAPACITOR;
            array_add(*new_el.terminals, other1);
            array_add(*new_el.terminals, other2);
            new_el.combo_expr = make_combo_expr(.RECIP_SUM, get_param_expr(el1), get_param_expr(el2));
            combined = true;
        } else if el1.type == .RESISTOR && el2.type == .CAPACITOR {
            print("Combining series resistor and capacitor: % + %\n", el1.name, el2.name);
            new_el  = make_rc_series_element(el1, other1, el2, other2);
            combined = true;
        } else if el1.type == .CAPACITOR && el2.type == .RESISTOR {
            print("Combining series resistor and capacitor: % + %\n", el2.name, el1.name);
            new_el  = make_rc_series_element(el2, other2, el1, other1);
            combined = true;
        }

        if combined {
            hi := max(idx1, idx2);
            lo := min(idx1, idx2);
            array_unordered_remove_by_index(elements, hi);
            array_unordered_remove_by_index(elements, lo);
            array_add(elements, new_el);
            return true;
        }
    }
    return false;
}

// Attempt ONE parallel combination. Returns true if a combination was made.
// Priority: R||R, then C||C, then R||C.
reduce_parallel_once :: (elements: *[..] Element) -> bool {
    // Same-type parallel (R||R and C||C)
    for i : 0..elements.count-1 {
        el1 := (elements.*)[i];
        if el1.type != .RESISTOR && el1.type != .CAPACITOR  continue;

        for j : i+1..elements.count-1 {
            el2 := (elements.*)[j];
            if el2.type != el1.type  continue;
            if el1.no_combine || el2.no_combine  continue;

            n1a := el1.terminals[0]; n2a := el1.terminals[1];
            n1b := el2.terminals[0]; n2b := el2.terminals[1];
            same_nodes := (n1a == n1b && n2a == n2b) || (n1a == n2b && n2a == n1b);
            if !same_nodes  continue;

            new_el: Element;
            new_el.name = tprint("%1%2", el1.name, el2.name);
            new_el.type = el1.type;
            array_add(*new_el.terminals, n1a);
            array_add(*new_el.terminals, n2a);

            if el1.type == .RESISTOR {
                print("Combining parallel resistors: % || %\n", el1.name, el2.name);
                new_el.combo_expr = make_combo_expr(.RECIP_SUM, get_param_expr(el1), get_param_expr(el2));
            } else {
                print("Combining parallel capacitors: % || %\n", el1.name, el2.name);
                new_el.combo_expr = make_combo_expr(.SUM, get_param_expr(el1), get_param_expr(el2));
            }

            // Remove higher index first to avoid shift issues
            array_unordered_remove_by_index(elements, j);
            array_unordered_remove_by_index(elements, i);
            array_add(elements, new_el);
            return true;
        }
    }

    // Mixed parallel (R||C)
    for i : 0..elements.count-1 {
        el1 := (elements.*)[i];
        if el1.type != .RESISTOR  continue;

        for j : 0..elements.count-1 {
            if j == i  continue;
            el2 := (elements.*)[j];
            if el2.type != .CAPACITOR  continue;
            if el1.no_combine || el2.no_combine  continue;

            n1a := el1.terminals[0]; n2a := el1.terminals[1];
            n1b := el2.terminals[0]; n2b := el2.terminals[1];
            same_nodes := (n1a == n1b && n2a == n2b) || (n1a == n2b && n2a == n1b);
            if !same_nodes  continue;

            print("Combining parallel resistor and capacitor: % || %\n", el1.name, el2.name);

            new_el: Element;
            new_el.name = tprint("%1%2", el1.name, el2.name);
            new_el.type = .RC_PARALLEL;
            array_add(*new_el.terminals, n1a);
            array_add(*new_el.terminals, n2a);
            new_el.rc_internal.res_name = el1.name;
            new_el.rc_internal.res_expr = get_param_expr(el1);
            new_el.rc_internal.cap_name = el2.name;
            new_el.rc_internal.cap_expr = get_param_expr(el2);

            hi := max(i, j);
            lo := min(i, j);
            array_unordered_remove_by_index(elements, hi);
            array_unordered_remove_by_index(elements, lo);
            array_add(elements, new_el);
            return true;
        }
    }

    return false;
}

// Attempt ONE antiparallel diode-pair collapse. Returns true if one was made.
// Two DIODEs sharing both terminals with reversed polarity and the same
// .model collapse into a DIODE_PAIR. Diodes with different .model params
// (e.g. asymmetric clippers) are left as two separate DIODEs.
reduce_diode_pairs_once :: (elements: *[..] Element) -> bool {
    for i : 0..elements.count-1 {
        el1 := (elements.*)[i];
        if el1.type != .DIODE  continue;

        for j : i+1..elements.count-1 {
            el2 := (elements.*)[j];
            if el2.type != .DIODE  continue;
            if el1.no_combine || el2.no_combine  continue;

            same_pair := el1.terminals[0] == el2.terminals[1] && el1.terminals[1] == el2.terminals[0];
            if !same_pair  continue;

            same_model := el1.nonlinear.is_name == el2.nonlinear.is_name
                       && el1.nonlinear.vt_name == el2.nonlinear.vt_name;
            if !same_model  continue;

            print("Combining antiparallel diode pair: % || %\n", el1.name, el2.name);

            new_el: Element;
            new_el.name = tprint("%1%2", el1.name, el2.name);
            new_el.type = .DIODE_PAIR;
            array_add(*new_el.terminals, el1.terminals[0]);
            array_add(*new_el.terminals, el1.terminals[1]);
            new_el.nonlinear = el1.nonlinear;

            hi := max(i, j);
            lo := min(i, j);
            array_unordered_remove_by_index(elements, hi);
            array_unordered_remove_by_index(elements, lo);
            array_add(elements, new_el);
            return true;
        }
    }
    return false;
}

// Reduce the circuit by combining series/parallel elements until no more
// reductions are possible. Mirrors the Python reduce_circuit() function.
reduce_circuit :: (elements: *[..] Element, inputs: [] string, outputs: [] string) {
    prev_count := -1;
    while elements.count != prev_count {
        prev_count = elements.count;
        reduce_series_once(elements, inputs, outputs);
        reduce_parallel_once(elements);
        reduce_diode_pairs_once(elements);
    }
}

// Options controlling the target language for code generation.
Codegen_Options :: struct {
    lang:      string = "cpp";   // "cpp" or "c"
    type_name: string = "float"; // numeric type used for variable declarations
}

// Counts of floating-point operations in a block of generated code.
Flop_Count :: struct {
    muls: int;
    divs: int;
    adds: int; // ADD, SUB, and NEG all counted here
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

    if kind == .ADD || kind == .SUB || kind == .MUL || kind == .DIV {
        count_flops_expr(exprs[e].left,  exprs, result);
        count_flops_expr(exprs[e].right, exprs, result);
    } else if kind == .NEG {
        count_flops_expr(exprs[e].left, exprs, result);
    }
}

count_flops :: (items: [] Gen_Var, exprs: [] Expr) -> Flop_Count {
    result: Flop_Count;
    for item : items  count_flops_expr(item.expr_id, exprs, *result);
    return result;
}

// A generated variable: a named expression that will become a `const auto` (C++) or `const <type>` (C).
Gen_Var :: struct {
    name:    string;
    expr_id: Expr_ID;
}

// `dc_mode`: build the DC-equivalent circuit instead. Capacitors are open
// circuits at DC, so CAPACITOR/RC_SERIES are dropped entirely; RC_PARALLEL
// still conducts through its resistor half. INDUCTOR DC (short circuit)
// isn't implemented yet. (@TODO)
generate_kcl_equations :: (elements_in: [] Element, inputs: [] string, dc_mode := false) -> []Expr_ID, []Expr_ID, [..]Expr, []Element {
    // Op-amp handling below (substitute_node) mutates terminals in place.
    // Callers reuse the same Element array across an AC-mode call and a
    // separate DC-mode call (build_dc_operating_point) -- work on a local
    // copy so one call's substitutions can't corrupt the other's terminals.
    elements: [..] Element;
    for el : elements_in {
        el_copy := el;
        fresh_terminals: [..] string;
        for t : el.terminals  array_add(*fresh_terminals, t);
        el_copy.terminals = fresh_terminals;
        array_add(*elements, el_copy);
    }

    eqn_nodes: [..] string;

    // vclip_<name> is a free Newton symbol, like a diode's branch current --
    // but unlike a branch current, Rout's synthesized resistor uses it as a
    // literal terminal string, so it'd otherwise get pulled into
    // eqn_nodes/solve_vars and solved as an ordinary linear unknown.
    clip_node_names: [..] string;
    for el : elements {
        if el.type == .AMP && opamp_has_clip_stage(el)  array_add(*clip_node_names, opamp_clip_node(el));
    }

    for el : elements {
        for terminal : el.terminals {
            if terminal == "0" continue;
            if array_find(inputs, terminal) continue;
            if array_find(clip_node_names, terminal) continue;
            if el.type == .FIXED_VOLTAGE    continue;
            array_add_if_unique(*eqn_nodes, terminal);
        }
    }

    // vgain_<name>/vbw_<name> need their own KCL row for the hard constraint
    // below -- usually guaranteed by being a synthesized Rcomp/Ccomp/Rout
    // terminal, but a clip/slew stage reroutes Rout to vclip_<name> instead,
    // orphaning them when GBW isn't also set. Add them explicitly.
    for el : elements {
        if el.type != .AMP || !needs_explicit_vcvs(el)  continue;
        array_add_if_unique(*eqn_nodes, opamp_gain_node(el));
        array_add_if_unique(*eqn_nodes, opamp_bandwidth_node(el));
    }

    exprs : [..] Expr;
    equations : [..] Expr_ID;
    for _ : eqn_nodes array_add(*equations, NO_EXPR);

    Sub_Expr :: struct {
        id: Expr_ID;
        replace_id: Expr_ID;
    }
    currents: [..] Sub_Expr;
    voltages: [..] Sub_Expr;

    solve_vars: [..]Expr_ID;
    for node : eqn_nodes array_add(*solve_vars, expr_symbol(node, *exprs));

    // All 2-terminal elements beside op-amps, fixed voltage sources, and
    // nonlinear devices (those get their branch currents wired up below,
    // generically, via nonlinear_device_spec()).
    for el : elements {
        if el.type == .AMP || el.type == .FIXED_VOLTAGE  continue;
        if nonlinear_device_spec(el.type).kcl_terms.count > 0  continue;
        if dc_mode {
            if el.type == .CAPACITOR || el.type == .RC_SERIES  continue; // open circuit at DC -- excluded entirely, not left as a free symbol
            assert(el.type != .INDUCTOR, "generate_kcl_equations(dc_mode=true): INDUCTOR DC handling (short circuit) is not implemented");
        }

        pos_node := el.terminals[0];
        neg_node := el.terminals[1];
        current_symbol := expr_symbol(tprint("i%", el.name), *exprs);
        pos_node_symbol := ifx pos_node == "0" then expr_const(0, *exprs) else expr_symbol(pos_node, *exprs);
        neg_node_symbol := ifx neg_node == "0" then expr_const(0, *exprs) else expr_symbol(neg_node, *exprs);

        for node, idx : eqn_nodes {
            if node == pos_node equations[idx] = combine(equations[idx], current_symbol, .SUB, *exprs); // current leaving this node
            if node == neg_node equations[idx] = combine(equations[idx], current_symbol, .ADD, *exprs); // current entering this node
        }

        current_expr := NO_EXPR;
        impedance_symbol := expr_symbol(tprint("g%", el.name), *exprs);
        state_symbol := expr_symbol(tprint("z%", el.name), *exprs);
        if el.type == {
            case .RESISTOR;
                current_expr = expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *exprs), *exprs);
            case .CAPACITOR;
                current_expr = expr_sub(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *exprs), *exprs), state_symbol, *exprs);
            case .INDUCTOR;
                current_expr = expr_add(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *exprs), *exprs), state_symbol, *exprs);
            case .RC_SERIES;
                current_expr = expr_sub(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *exprs), *exprs), state_symbol, *exprs);
            case .RC_PARALLEL;
                if dc_mode {
                    // only the resistor half conducts at DC -- use its raw param, not the trapezoidal g
                    res_symbol := expr_symbol(el.rc_internal.res_name, *exprs);
                    current_expr = expr_div(expr_sub(pos_node_symbol, neg_node_symbol, *exprs), res_symbol, *exprs);
                } else {
                    current_expr = expr_sub(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *exprs), *exprs), state_symbol, *exprs);
                }
        }
        // assert(current_expr != NO_EXPR);
        if current_expr != NO_EXPR array_add(*currents, .{ current_symbol, current_expr });
    }

    // Nonlinear devices: branch currents stay free symbols here (no
    // current_expr substitution, unlike the linear pass above), so solve()
    // gives a closed-form node voltage parametrized by them. Terms sharing a
    // terminal accumulate -- see Device_Kcl_Term's comment (nonlinear_solve.jai
    // picks these symbols back up when building each device's I-V law).
    for el : elements {
        spec := nonlinear_device_spec(el.type);
        for term : spec.kcl_terms {
            symbol := expr_symbol(tprint("i%1%2", term.branch_suffix, el.name), *exprs);
            node   := el.terminals[term.terminal_idx];
            op     := ifx term.entering then Expr_Kind.ADD else Expr_Kind.SUB;
            for n, idx : eqn_nodes {
                if n == node  equations[idx] = combine(equations[idx], symbol, op, *exprs);
            }
        }
    }

    // Repoints other elements' terminals from an eliminated node to its
    // replacement. Skips FIXED_VOLTAGE elements: their terminal is already
    // handled by the constraint pass above.
    substitute_node :: (node_to_sub: string, replacement_node: string) #expand {
        for *el : elements {
            if el.type == .FIXED_VOLTAGE  continue;
            for node, node_idx : el.terminals {
                if node == node_to_sub {
                    el.terminals[node_idx] = replacement_node;
                }
            }
        }
    }

    // Rows already holding an exact constraint (fixed voltage, op-amp pin,
    // diode-partner fallback) -- the gmin pass below must skip these.
    pinned: [..] bool;
    for _ : eqn_nodes  array_add(*pinned, false);

    // Constraint equations for fixed voltage sources
    for el : elements {
        if el.type != .FIXED_VOLTAGE  continue;
        fixed_node   := el.terminals[0];
        node_symbol  := expr_symbol(fixed_node, *exprs);
        param_symbol := expr_symbol(el.name,    *exprs);
        for node, idx : eqn_nodes {
            if node == fixed_node {
                equations[idx] = expr_sub(node_symbol, param_symbol, *exprs);
                pinned[idx] = true;
                break;
            }
        }
    }

    // Fill the KCL row for `node` with node = value, but only if the row is
    // still empty (eliminating a node from solve_vars doesn't remove its
    // KCL row; at DC a node whose only neighbors are dropped
    // capacitors/RC_SERIES is left empty, under-constraining the system).
    pin_node_equation :: (node: string, value: Expr_ID) #expand {
        for n, idx : eqn_nodes {
            if n == node {
                if equations[idx] == NO_EXPR {
                    // At DC `value` can itself be undefined (e.g. another
                    // floating node) -- ground instead, like gmin below.
                    pin_value := ifx dc_mode then expr_const(0, *exprs) else value;
                    equations[idx] = expr_sub(expr_symbol(node, *exprs), pin_value, *exprs);
                    pinned[idx] = true;
                }
                break;
            }
        }
    }

    // op-amps
    for el : elements {
        if el.type != .AMP continue;
        out_node := el.terminals[0];
        neg_node := el.terminals[1];
        pos_node := el.terminals[2];
        o := el.opamp;

        // Ibias/Ios: constant currents drawn from each input node. Applies
        // either way below (ideal merge or explicit VCVS).
        if o.has_ibias || o.has_ios {
            bias_symbol := ifx o.has_ibias then expr_symbol(o.ibias_name, *exprs) else NO_EXPR;
            half_ios     := ifx o.has_ios   then expr_div(expr_symbol(o.ios_name, *exprs), expr_const(2, *exprs), *exprs) else NO_EXPR;
            ibias_pos := combine(bias_symbol, half_ios, .ADD, *exprs);
            ibias_neg := combine(bias_symbol, half_ios, .SUB, *exprs);
            for node, idx : eqn_nodes {
                if node == pos_node  equations[idx] = combine(equations[idx], ibias_pos, .SUB, *exprs);
                if node == neg_node  equations[idx] = combine(equations[idx], ibias_neg, .SUB, *exprs);
            }
        }

        vos_symbol := ifx o.has_vos then expr_symbol(o.vos_name, *exprs) else NO_EXPR;

        if needs_explicit_vcvs(el) {
            // Explicit finite-gain VCVS: overwrite the gain node's row with
            // a hard constraint (same idea as FIXED_VOLTAGE below), instead
            // of merging pos/neg symbols. Rin/Rout/GBW are synthesized as
            // ordinary R/C elements upstream.
            pos_symbol := ifx pos_node == "0" then expr_const(0, *exprs) else expr_symbol(pos_node, *exprs);
            neg_symbol := ifx neg_node == "0" then expr_const(0, *exprs) else expr_symbol(neg_node, *exprs);
            diff_expr := expr_sub(pos_symbol, neg_symbol, *exprs);
            if vos_symbol != NO_EXPR  diff_expr = expr_add(diff_expr, vos_symbol, *exprs);
            rhs := expr_mul(expr_symbol(o.aol_name, *exprs), diff_expr, *exprs);

            gain_node := opamp_gain_node(el);
            for node, idx : eqn_nodes {
                if node == gain_node {
                    equations[idx] = expr_sub(expr_symbol(gain_node, *exprs), rhs, *exprs);
                    pinned[idx] = true;
                    break;
                }
            }

            // vclip_<name> has no KCL row of its own, so without Rout to
            // wire it to out_node, out_node needs an explicit constraint.
            if opamp_has_clip_stage(el) && !o.has_rout {
                clip_node := opamp_clip_node(el);
                for node, idx : eqn_nodes {
                    if node == out_node {
                        equations[idx] = expr_sub(expr_symbol(out_node, *exprs), expr_symbol(clip_node, *exprs), *exprs);
                        pinned[idx] = true;
                        break;
                    }
                }
            }
            continue;
        }

        // Ideal (infinite-gain) op-amp: merge pos/neg node symbols, same as
        // before, optionally offset by Vos (constraint: pos - neg + Vos = 0).
        current_symbol := expr_symbol(tprint("i%", el.name), *exprs);
        for node, idx : eqn_nodes {
            if node == out_node  equations[idx] = combine(equations[idx], current_symbol, .ADD, *exprs); // current entering this node
        }
        array_add(*solve_vars, current_symbol);

        if pos_node == "0" {
            target := ifx vos_symbol != NO_EXPR then vos_symbol else expr_const(0, *exprs); // neg = Vos
            print("Setting % → 0\n", neg_node);
            array_add(*voltages, .{ expr_symbol(neg_node, *exprs), target });
            array_unordered_remove_by_value(*solve_vars, expr_symbol(neg_node, *exprs));
            pin_node_equation(neg_node, target);
            if vos_symbol == NO_EXPR  substitute_node(neg_node, "0");
        } else if neg_node == "0" {
            target := ifx vos_symbol != NO_EXPR then expr_neg(vos_symbol, *exprs) else expr_const(0, *exprs); // pos = -Vos
            print("Setting % → 0\n", pos_node);
            array_add(*voltages, .{ expr_symbol(pos_node, *exprs), target });
            array_unordered_remove_by_value(*solve_vars, expr_symbol(pos_node, *exprs));
            pin_node_equation(pos_node, target);
            if vos_symbol == NO_EXPR  substitute_node(pos_node, "0");
        } else {
            target := ifx vos_symbol != NO_EXPR then expr_sub(expr_symbol(neg_node, *exprs), vos_symbol, *exprs) else expr_symbol(neg_node, *exprs); // pos = neg - Vos
            print("Setting % → %\n", pos_node, neg_node);
            array_add(*voltages, .{ expr_symbol(pos_node, *exprs), target });
            array_unordered_remove_by_value(*solve_vars, expr_symbol(pos_node, *exprs));
            pin_node_equation(pos_node, target);
            if vos_symbol == NO_EXPR  substitute_node(pos_node, neg_node);
        }
    }

    // substitute current and voltage expressions
    for current_sub : currents  substitute(current_sub.id, current_sub.replace_id, *exprs);
    for voltage_sub : voltages  substitute(voltage_sub.id, voltage_sub.replace_id, *exprs);

    // At DC, a node reachable only through a dropped capacitor and a diode
    // branch gets a KCL row with no solve_var at all (just "-iDiode = 0").
    // Zero current through an odd I-V law means zero junction voltage, so
    // just pin the node to its diode partner's voltage.
    if dc_mode {
        for node, idx : eqn_nodes {
            if equations[idx] == NO_EXPR continue;
            if expr_depends_on(equations[idx], solve_vars, exprs) continue;

            for el : elements {
                if el.type != .DIODE && el.type != .DIODE_PAIR continue;
                other := "";
                if      el.terminals[0] == node then other = el.terminals[1];
                else if el.terminals[1] == node then other = el.terminals[0];
                if other.count == 0 continue;

                other_symbol := ifx other == "0" then expr_const(0, *exprs) else expr_symbol(other, *exprs);
                equations[idx] = expr_sub(expr_symbol(node, *exprs), other_symbol, *exprs);
                pinned[idx] = true;
                break;
            }
        }
    }

    // gmin: a group of nodes can be jointly floating at DC even if no single
    // row is empty (e.g. a resistor chain hanging off a diode-pinned node --
    // every row is real, but the group has no path to ground). A tiny
    // leakage conductance on every non-pinned node breaks that degeneracy,
    // same as real SPICE's gmin-to-ground regularization.
    if dc_mode {
        // expr_const is integer-only -- build 1e-9 as 1/1e9.
        gmin := expr_div(expr_const(1, *exprs), expr_const(1_000_000_000, *exprs), *exprs);
        for node, idx : eqn_nodes {
            if equations[idx] == NO_EXPR continue;
            if pinned[idx] continue;
            leak := expr_mul(gmin, expr_symbol(node, *exprs), *exprs);
            equations[idx] = expr_sub(equations[idx], leak, *exprs);
        }
    }

    return solve_vars, equations, exprs, elements;
}

generate_expressions :: (elements: [] Element,
                         outputs: [] string,
                         solve_vars: [] Expr_ID,
                         solutions: [] Expr_ID,
                         exprs: *[..] Expr) -> [] Gen_Var {
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
        node_symbol := expr_symbol(node, exprs);
        found_solve_var, solve_idx := array_find(solve_vars, node_symbol);
        if !found_solve_var  return;

        for gv : gen_vars  if gv.name == node  return;
        add_gen_var(node, simplify(solutions[solve_idx], exprs));
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
                add_gen_var(expr_name, expr_mul(tprint("g%", el.name), expr_sub(pos_node, neg_node, exprs), exprs));

            case .RC_SERIES; #through;
            case .RC_PARALLEL;
                pos_node := el.terminals[0];
                neg_node := el.terminals[1];
                add_expression_for_node(pos_node);
                add_expression_for_node(neg_node);

                expr_name := tprint("v%", el.name);
                add_gen_var(expr_name, expr_sub(pos_node, neg_node, exprs));
        }
    }

    return gen_vars;
}

// Look up the closed-form linear MNA solution for `node`.
lookup_node_voltage :: (node: string, solve_vars: [] Expr_ID, solutions: [] Expr_ID, exprs: *[..] Expr, inputs: [] string = .[]) -> Expr_ID {
    if node == "0"  return expr_const(0, exprs);
    if array_find(inputs, node)  return expr_symbol(node, exprs);
    node_symbol := expr_symbol(node, exprs);
    found, idx := array_find(solve_vars, node_symbol);
    assert(found, "Nonlinear element terminal references a node that isn't part of the solved MNA system");
    return solutions[idx];
}

// One junction-voltage unknown belonging to a nonlinear element: "vD<name>"
// for DIODE/DIODE_PAIR, "vBC<name>"/"vBE<name>" for BJT.
Junction_Voltage :: struct {
    name:    string;
    expr_id: Expr_ID;
}

// For each nonlinear element, build the closed-form expression for its own
// junction voltage(s), driven by nonlinear_device_spec().
generate_junction_voltage_expressions :: (elements: [] Element,
                                          solve_vars: [] Expr_ID,
                                          solutions: [] Expr_ID,
                                          exprs: *[..] Expr) -> [] Junction_Voltage {
    junctions: [..] Junction_Voltage;

    for el : elements {
        spec := nonlinear_device_spec(el.type);
        for u : spec.unknowns {
            pos_v := lookup_node_voltage(el.terminals[u.pos_terminal], solve_vars, solutions, exprs);
            neg_v := lookup_node_voltage(el.terminals[u.neg_terminal], solve_vars, solutions, exprs);
            v := simplify(expr_sub(pos_v, neg_v, exprs), exprs);
            array_add(*junctions, .{ name = tprint("%1%2", u.name_prefix, el.name), expr_id = v });
        }
    }

    return junctions;
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
emit_topological_order :: (items: [] Gen_Var, exprs: [] Expr, builder: *String_Builder, options: Codegen_Options = .{}) {
    decl_type := ifx options.lang == "c" then options.type_name else "auto";
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
            exprs: [] Expr, options: Codegen_Options = .{},
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
    emit_topological_order(outer_items, exprs, *outer_builder, options);
    emit_topological_order(inner_items, exprs, *inner_builder, options);
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

solve_and_codegen :: (elements: *[..] Element, inputs: [] string, outputs: [] string, options: Codegen_Options = .{}) -> (outer_code: string, inner_code: string, outer_flops: Flop_Count, inner_flops: Flop_Count) {
    reduce_circuit(elements, inputs, outputs);

    solve_vars, equations, exprs, kcl_elements := generate_kcl_equations(elements.*, inputs);

    solutions := solve(equations, solve_vars, *exprs);

    // kcl_elements (not elements.*): op-amp-merged nodes (e.g. vpos -> vneg)
    // only have their aliased terminal there.
    gen_vars := generate_expressions(kcl_elements, outputs, solve_vars, solutions, *exprs);

    // Collect gen_var expression IDs for CSE.
    gen_var_ids: [..] Expr_ID;
    for gv : gen_vars  array_add(*gen_var_ids, gv.expr_id);

    cse_vars := cse(gen_var_ids, *exprs);
    sort_cse_topological(*cse_vars, exprs);  // ensure deps appear before dependents

    // Build the variant base for LICM (inputs + state vars + gen_var names).
    licm_variant_base := build_variant_base(kcl_elements, inputs);
    for gv : gen_vars  array_add(*licm_variant_base, gv.name);
    licm(gen_var_ids, *cse_vars, *exprs, licm_variant_base);
    recip_hoist(gen_var_ids, *cse_vars, *exprs, licm_variant_base);

    outer_code, inner_code, outer_flops, inner_flops := codegen(kcl_elements, inputs, gen_vars, cse_vars, exprs, options);
    return outer_code, inner_code, outer_flops, inner_flops;
}
