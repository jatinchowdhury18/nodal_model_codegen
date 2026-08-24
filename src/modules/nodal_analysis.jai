#import "Basic";
#import "Hash_Table";
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
    NPN;         // 3-terminal: [collector, base, emitter]. Ebers-Moll I-V law.
    PNP;         // 3-terminal: [collector, base, emitter]. Mirrors NPN: vCB/vEB unknowns, reversed KCL terms.
    NJF;         // 3-terminal: [drain, gate, source]. Square-law I-V law.
    PJF;         // 3-terminal: [drain, gate, source]. Mirrors NJF: vSG unknown, reversed KCL terms.
    TRIODE;      // 3-terminal: [plate, grid, cathode]. Dempwolf & Zolzer physically-motivated I-V law.
    VCCS;            // 4-terminal: [pos, neg, ctrl_pos, ctrl_neg]. Linear: I = gm*(V(ctrl_pos)-V(ctrl_neg)), sourced at pos/sunk at neg.
    CURRENT_SOURCE;  // 2-terminal: [pos, neg]. Constant current, sourced at pos/sunk at neg.
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

    // TRIODE only (Dempwolf & Zolzer model); empty string for other types.
    gk_name:    string; // cathode-current perveance
    mu_name:    string; // amplification factor
    gamma_name: string; // cathode-current exponent
    ck_name:    string; // cathode-current softplus adaption factor
    gg_name:    string; // grid-current perveance
    xi_name:    string; // grid-current exponent
    cg_name:    string; // grid-current softplus adaption factor
    ig0_name:   string; // grid-current stability offset
}

// -----------------------------------------------------------------------
// Nonlinear device registry. Adding a new nonlinear element type means
// filling out one case in nonlinear_device_spec() (below), device_iv_law()
// (nonlinear_solve.jai) and device_model_param_names() (below), plus the
// SPICE line parsing in netlist_codegen.jai (not generalized here, since
// that part is inherently SPICE-grammar-specific, not device-physics-specific).
// -----------------------------------------------------------------------

// Per-iteration Newton-step limiter for one unknown. Caps a bad step
// before it wanders into a region the I-V law approximates badly.
Limiter_Kind :: enum {
    NONE;           // plain Newton step
    EXP_JUNCTION;   // diode/BJT-style: clamp the *forward* step near vcrit = vt*log(vt/(sqrt(2)*Is))
    EXP_JUNCTION_SYM; // antiparallel diode pair (sinh law): same clamp, both directions
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
// accumulate, e.g. BJT's iE = iB + iC is two ADD terms on the emitter,
// not a pre-summed expression.
Device_Kcl_Term :: struct {
    terminal_idx:  int;
    branch_suffix: string;
    entering:      bool; // true: current enters this terminal (KCL .ADD); false: leaves (.SUB)
}

// Everything generic nonlinear-device code needs about one Element_Type.
// The actual I-V law stays a named function in nonlinear_solve.jai. That's
// real physics, not something worth generalizing away; see device_iv_law().
Nonlinear_Device_Spec :: struct {
    unknowns:  [] Device_Unknown;
    kcl_terms: [] Device_Kcl_Term;
}

// Empty for any non-nonlinear (or unhandled) type. Every generic pass
// below treats an empty spec as "this element contributes nothing."
nonlinear_device_spec :: (type: Element_Type) -> Nonlinear_Device_Spec {
    if type == {
        case .DIODE;
            return .{
                unknowns  = Device_Unknown.[ .{ name_prefix = "v", pos_terminal = 0, neg_terminal = 1, limiter = .EXP_JUNCTION } ],
                kcl_terms = Device_Kcl_Term.[
                    .{ terminal_idx = 0, branch_suffix = "", entering = false }, // anode -= i
                    .{ terminal_idx = 1, branch_suffix = "", entering = true },  // cathode += i
                ],
            };
        case .DIODE_PAIR;
            return .{
                unknowns  = Device_Unknown.[ .{ name_prefix = "v", pos_terminal = 0, neg_terminal = 1, limiter = .EXP_JUNCTION_SYM } ],
                kcl_terms = Device_Kcl_Term.[
                    .{ terminal_idx = 0, branch_suffix = "", entering = false }, // anode -= i
                    .{ terminal_idx = 1, branch_suffix = "", entering = true },  // cathode += i
                ],
            };
        case .NPN;
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
        case .PNP;
            // Mirrors NPN: vCB/vEB in place of vBC/vBE (both reversed relative to
            // base), and current direction reversed (out of base/collector, into emitter).
            return .{
                unknowns  = Device_Unknown.[
                    .{ name_prefix = "vCB", pos_terminal = 0, neg_terminal = 1, limiter = .EXP_JUNCTION },
                    .{ name_prefix = "vEB", pos_terminal = 2, neg_terminal = 1, limiter = .EXP_JUNCTION },
                ],
                kcl_terms = Device_Kcl_Term.[
                    .{ terminal_idx = 0, branch_suffix = "C", entering = true },  // collector += iC
                    .{ terminal_idx = 1, branch_suffix = "B", entering = true },  // base += iB
                    .{ terminal_idx = 2, branch_suffix = "B", entering = false }, // emitter -= iB
                    .{ terminal_idx = 2, branch_suffix = "C", entering = false }, // emitter -= iC
                ],
            };
        case .NJF;
            return .{
                unknowns  = Device_Unknown.[ .{ name_prefix = "vGS", pos_terminal = 1, neg_terminal = 2, limiter = .JFET_PINCHOFF } ],
                kcl_terms = Device_Kcl_Term.[
                    .{ terminal_idx = 0, branch_suffix = "", entering = false }, // drain -= iD
                    .{ terminal_idx = 2, branch_suffix = "", entering = true },  // source += iD
                ],
            };
        case .PJF;
            // Mirrors NJF: vSG in place of vGS, current direction reversed
            // (source to drain, instead of drain to source).
            return .{
                unknowns  = Device_Unknown.[ .{ name_prefix = "vSG", pos_terminal = 2, neg_terminal = 1, limiter = .JFET_PINCHOFF } ],
                kcl_terms = Device_Kcl_Term.[
                    .{ terminal_idx = 0, branch_suffix = "", entering = true },  // drain += iD
                    .{ terminal_idx = 2, branch_suffix = "", entering = false }, // source -= iD
                ],
            };
        case .TRIODE;
            // Ik (cathode current) is a joint function of vPK and vGK, so
            // both need their own Newton unknown. We don't apply any step-limiting
            // here: the Dempwolf model's softplus-based I-V law is smooth and bounded
            // over the whole real line, so a plain Newton step suffices.
            return .{
                unknowns  = Device_Unknown.[
                    .{ name_prefix = "vPK", pos_terminal = 0, neg_terminal = 2, limiter = .NONE },
                    .{ name_prefix = "vGK", pos_terminal = 1, neg_terminal = 2, limiter = .NONE },
                ],
                kcl_terms = Device_Kcl_Term.[
                    .{ terminal_idx = 0, branch_suffix = "P", entering = false }, // plate   -= iP (=Ia=Ik-Ig)
                    .{ terminal_idx = 1, branch_suffix = "G", entering = false }, // grid    -= iG (=Ig)
                    .{ terminal_idx = 2, branch_suffix = "P", entering = true },  // cathode += iP
                    .{ terminal_idx = 2, branch_suffix = "G", entering = true },  // cathode += iG
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
        case .EXP_JUNCTION; #through;
        case .EXP_JUNCTION_SYM; return m.vt_name, m.is_name, "";
        case .JFET_PINCHOFF; return "", "", m.vp_name;
    }
    return "", "", "";
}

// Params struct field names a device's .model card populates. Used for
// both Params-struct emission and impedance-alias emission.
device_model_param_names :: (type: Element_Type, m: Nonlinear_Model) -> [..] string {
    names: [..] string;
    if type == {
        case .DIODE; #through;
        case .DIODE_PAIR;
            array_add(*names, m.is_name, m.vt_name);
        case .NPN; #through;
        case .PNP;
            array_add(*names, m.is_name, m.vt_name, m.betaf_name, m.betar_name);
        case .NJF; #through;
        case .PJF;
            array_add(*names, m.beta_name, m.vp_name);
        case .TRIODE;
            array_add(*names, m.gk_name, m.mu_name, m.gamma_name, m.ck_name,
                              m.gg_name, m.xi_name, m.cg_name, m.ig0_name);
    }
    return names;
}

// True if any element in `elements` has an unknown using this limiter kind.
// Used to decide which limiter helper functions to emit.
circuit_uses_limiter_kind :: (elements: [] Element, kind: Limiter_Kind) -> bool {
    // Helpers are chosen at the top level, before generate_unit() runs
    // reduce_circuit(), so the DIODE_PAIRs the antiparallel collapse is about
    // to create aren't in `elements` yet and have to be anticipated.
    paired: [..] bool;
    paired.allocator = temp;
    array_resize(*paired, elements.count);
    consumed: [..] bool;
    consumed.allocator = temp;
    array_resize(*consumed, elements.count);
    for i : 0..elements.count-1 {
        if consumed[i] || elements[i].type != .DIODE  continue;
        for j : 0..elements.count-1 {
            if j == i || consumed[j]  continue;
            if !diodes_are_antiparallel_pair(elements[i], elements[j])  continue;
            paired[i] = true;   paired[j] = true;
            consumed[i] = true; consumed[j] = true;
            break;
        }
    }

    for el, i : elements {
        if el.type == .DIODE {
            pending := ifx paired[i] then Limiter_Kind.EXP_JUNCTION_SYM else .EXP_JUNCTION;
            if kind == pending  return true;
            continue;
        }

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

// True if none of an AMP's non-ideal characteristics are set, i.e. it's
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

// True if the op-amp has an output clipping/slew stage, i.e. a Newton
// unknown (vclip_<name>) sits between the gain chain and Rout/out_node.
opamp_has_clip_stage :: (el: Element) -> bool {
    return el.opamp.has_vsat_p || el.opamp.has_vsat_n || el.opamp.has_slew;
}

// vgain_<name>: the ideal (infinite-bandwidth) gain-stage node, holding
// Aol*(pos - neg + Vos). Collapses to the real out_node directly when
// nothing else (GBW lowpass, clip/slew stage, Rout) follows it in the
// chain, e.g. an Aol-only op-amp needs no synthetic nodes at all.
opamp_gain_node :: (el: Element) -> string {
    if el.opamp.has_gbw || opamp_has_clip_stage(el) || el.opamp.has_rout  return tprint("vgain_%", el.name);
    return el.terminals[0];
}

// One-pole-lowpassed version of vgain_<name> (meaningful only when
// has_gbw, an internal Rcomp/Ccomp synthesized upstream). Collapses to
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

    // Set from a "; no_combine" inline netlist comment (R/C/D only). Opts
    // this element out of series/parallel/antiparallel-pair reduction.
    no_combine: bool;
};

// A (name, value) pair for a Params struct field, parsed by
// netlist_codegen.jai's parse_netlist, emitted by codegen.jai's
// generate_params_struct.
Element_Value :: struct {
    name: string;
    value: float;
};

// A generated variable: a named expression that will become a `const auto`
// (C++) or `const <type>` (C).
Gen_Var :: struct {
    name:    string;
    expr_id: Expr_ID;
}

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

// Two DIODEs that will collapse into one DIODE_PAIR.
diodes_are_antiparallel_pair :: (el1: Element, el2: Element) -> bool {
    if el1.type != .DIODE || el2.type != .DIODE  return false;
    if el1.no_combine || el2.no_combine  return false;
    if el1.terminals[0] != el2.terminals[1] || el1.terminals[1] != el2.terminals[0]  return false;
    return el1.nonlinear.is_name == el2.nonlinear.is_name
        && el1.nonlinear.vt_name == el2.nonlinear.vt_name;
}

// Index of the element that will collapse with elements[i] into one
// DIODE_PAIR, or -1 if there is none.
find_antiparallel_partner :: (elements: [] Element, i: int) -> int {
    for j : 0..elements.count-1 {
        if j != i && diodes_are_antiparallel_pair(elements[i], elements[j])  return j;
    }
    return -1;
}

// Attempt ONE antiparallel diode-pair collapse. Returns true if one was made.
reduce_diode_pairs_once :: (elements: *[..] Element) -> bool {
    for i : 0..elements.count-1 {
        j := find_antiparallel_partner(elements.*, i);
        if j < 0  continue;

        el1 := (elements.*)[i];
        el2 := (elements.*)[j];
        print("Combining antiparallel diode pair: % || %\n", el1.name, el2.name);

        new_el: Element;
        new_el.name = tprint("%1%2", el1.name, el2.name);
        new_el.type = .DIODE_PAIR;
        array_add(*new_el.terminals, el1.terminals[0]);
        array_add(*new_el.terminals, el1.terminals[1]);
        new_el.nonlinear = el1.nonlinear;

        array_unordered_remove_by_index(elements, max(i, j));
        array_unordered_remove_by_index(elements, min(i, j));
        array_add(elements, new_el);
        return true;
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

// `dc_mode`: build the DC-equivalent circuit instead. Capacitors are open
// circuits at DC, so CAPACITOR/RC_SERIES are dropped entirely; RC_PARALLEL
// still conducts through its resistor half. INDUCTORs are short circuits at
// DC, handled by a dedicated merge pass below, same idea as an ideal
// op-amp's virtual short.
generate_kcl_equations :: (elements_in: [] Element, inputs: [] string, dc_mode := false) -> []Expr_ID, []Expr_ID, Expr_Pool, []Element {
    // Op-amp handling below (substitute_node) mutates terminals in place.
    // Callers reuse the same Element array across an AC-mode call and a
    // separate DC-mode call (build_dc_operating_point), so work on a local
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

    // vclip_<name> is a free Newton symbol, like a diode's branch current.
    // But unlike a branch current, Rout's synthesized resistor uses it as a
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
    // below. This is usually guaranteed by being a synthesized Rcomp/Ccomp/Rout
    // terminal, but a clip/slew stage reroutes Rout to vclip_<name> instead,
    // orphaning them when GBW isn't also set. Add them explicitly.
    for el : elements {
        if el.type != .AMP || !needs_explicit_vcvs(el)  continue;
        array_add_if_unique(*eqn_nodes, opamp_gain_node(el));
        array_add_if_unique(*eqn_nodes, opamp_bandwidth_node(el));
    }

    pool: Expr_Pool;
    equations : [..] Expr_ID;
    for _ : eqn_nodes array_add(*equations, NO_EXPR);

    currents: [..] Substitution;
    voltages: [..] Substitution;

    solve_vars: [..]Expr_ID;
    for node : eqn_nodes array_add(*solve_vars, expr_symbol(node, *pool));

    // All 2-terminal elements beside op-amps, fixed voltage sources, and
    // nonlinear devices (those get their branch currents wired up below,
    // generically, via nonlinear_device_spec()).
    for el : elements {
        if el.type == .AMP || el.type == .FIXED_VOLTAGE  continue;
        if nonlinear_device_spec(el.type).kcl_terms.count > 0  continue;
        if dc_mode {
            if el.type == .CAPACITOR || el.type == .RC_SERIES  continue; // open circuit at DC; excluded entirely, not left as a free symbol
            if el.type == .INDUCTOR  continue; // short circuit at DC; handled by the merge pass below, not here
        }

        pos_node := el.terminals[0];
        neg_node := el.terminals[1];
        current_symbol := expr_symbol(tprint("i%", el.name), *pool);
        pos_node_symbol := ifx pos_node == "0" then expr_const(0, *pool) else expr_symbol(pos_node, *pool);
        neg_node_symbol := ifx neg_node == "0" then expr_const(0, *pool) else expr_symbol(neg_node, *pool);

        for node, idx : eqn_nodes {
            if node == pos_node equations[idx] = combine(equations[idx], current_symbol, .SUB, *pool); // current leaving this node
            if node == neg_node equations[idx] = combine(equations[idx], current_symbol, .ADD, *pool); // current entering this node
        }

        current_expr := NO_EXPR;
        impedance_symbol := expr_symbol(tprint("g%", el.name), *pool);
        state_symbol := expr_symbol(tprint("z%", el.name), *pool);
        if el.type == {
            case .RESISTOR;
                current_expr = expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *pool), *pool);
            case .CAPACITOR;
                current_expr = expr_sub(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *pool), *pool), state_symbol, *pool);
            case .INDUCTOR;
                current_expr = expr_add(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *pool), *pool), state_symbol, *pool);
            case .RC_SERIES;
                current_expr = expr_sub(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *pool), *pool), state_symbol, *pool);
            case .RC_PARALLEL;
                if dc_mode {
                    // only the resistor half conducts at DC; use its raw param, not the trapezoidal g
                    res_symbol := expr_symbol(el.rc_internal.res_name, *pool);
                    current_expr = expr_div(expr_sub(pos_node_symbol, neg_node_symbol, *pool), res_symbol, *pool);
                } else {
                    current_expr = expr_sub(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *pool), *pool), state_symbol, *pool);
                }
            case .VCCS;
                ctrl_pos := el.terminals[2];
                ctrl_neg := el.terminals[3];
                ctrl_pos_symbol := ifx ctrl_pos == "0" then expr_const(0, *pool) else expr_symbol(ctrl_pos, *pool);
                ctrl_neg_symbol := ifx ctrl_neg == "0" then expr_const(0, *pool) else expr_symbol(ctrl_neg, *pool);
                current_expr = expr_mul(impedance_symbol, expr_sub(ctrl_pos_symbol, ctrl_neg_symbol, *pool), *pool);
            case .CURRENT_SOURCE;
                current_expr = expr_symbol(el.name, *pool);
        }
        if current_expr != NO_EXPR array_add(*currents, .{ current_symbol, current_expr });
    }

    // Nonlinear devices: branch currents stay free symbols here (no
    // current_expr substitution, unlike the linear pass above), so solve()
    // gives a closed-form node voltage parametrized by them. Terms sharing a
    // terminal accumulate; see Device_Kcl_Term's comment (nonlinear_solve.jai
    // picks these symbols back up when building each device's I-V law).
    for el : elements {
        spec := nonlinear_device_spec(el.type);
        for term : spec.kcl_terms {
            symbol := expr_symbol(tprint("i%1%2", term.branch_suffix, el.name), *pool);
            node   := el.terminals[term.terminal_idx];
            op     := ifx term.entering then Expr_Kind.ADD else Expr_Kind.SUB;
            for n, idx : eqn_nodes {
                if n == node  equations[idx] = combine(equations[idx], symbol, op, *pool);
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
    // diode-partner fallback). The gmin pass below must skip these.
    pinned: [..] bool;
    for _ : eqn_nodes  array_add(*pinned, false);

    // Constraint equations for fixed voltage sources
    for el : elements {
        if el.type != .FIXED_VOLTAGE  continue;
        fixed_node   := el.terminals[0];
        node_symbol  := expr_symbol(fixed_node, *pool);
        param_symbol := expr_symbol(el.name,    *pool);
        for node, idx : eqn_nodes {
            if node == fixed_node {
                equations[idx] = expr_sub(node_symbol, param_symbol, *pool);
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
                    // floating node), so ground instead, like gmin below.
                    pin_value := ifx dc_mode then expr_const(0, *pool) else value;
                    equations[idx] = expr_sub(expr_symbol(node, *pool), pin_value, *pool);
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
            bias_symbol := ifx o.has_ibias then expr_symbol(o.ibias_name, *pool) else NO_EXPR;
            half_ios     := ifx o.has_ios   then expr_div(expr_symbol(o.ios_name, *pool), expr_const(2, *pool), *pool) else NO_EXPR;
            ibias_pos := combine(bias_symbol, half_ios, .ADD, *pool);
            ibias_neg := combine(bias_symbol, half_ios, .SUB, *pool);
            for node, idx : eqn_nodes {
                if node == pos_node  equations[idx] = combine(equations[idx], ibias_pos, .SUB, *pool);
                if node == neg_node  equations[idx] = combine(equations[idx], ibias_neg, .SUB, *pool);
            }
        }

        vos_symbol := ifx o.has_vos then expr_symbol(o.vos_name, *pool) else NO_EXPR;

        if needs_explicit_vcvs(el) {
            // Explicit finite-gain VCVS: overwrite the gain node's row with
            // a hard constraint (same idea as FIXED_VOLTAGE below), instead
            // of merging pos/neg symbols. Rin/Rout/GBW are synthesized as
            // ordinary R/C elements upstream.
            pos_symbol := ifx pos_node == "0" then expr_const(0, *pool) else expr_symbol(pos_node, *pool);
            neg_symbol := ifx neg_node == "0" then expr_const(0, *pool) else expr_symbol(neg_node, *pool);
            diff_expr := expr_sub(pos_symbol, neg_symbol, *pool);
            if vos_symbol != NO_EXPR  diff_expr = expr_add(diff_expr, vos_symbol, *pool);
            rhs := expr_mul(expr_symbol(o.aol_name, *pool), diff_expr, *pool);

            gain_node := opamp_gain_node(el);
            for node, idx : eqn_nodes {
                if node == gain_node {
                    equations[idx] = expr_sub(expr_symbol(gain_node, *pool), rhs, *pool);
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
                        equations[idx] = expr_sub(expr_symbol(out_node, *pool), expr_symbol(clip_node, *pool), *pool);
                        pinned[idx] = true;
                        break;
                    }
                }
            }
            continue;
        }

        // Ideal (infinite-gain) op-amp: merge pos/neg node symbols, same as
        // before, optionally offset by Vos (constraint: pos - neg + Vos = 0).
        current_symbol := expr_symbol(tprint("i%", el.name), *pool);
        for node, idx : eqn_nodes {
            if node == out_node  equations[idx] = combine(equations[idx], current_symbol, .ADD, *pool); // current entering this node
        }
        array_add(*solve_vars, current_symbol);

        if pos_node == "0" {
            target := ifx vos_symbol != NO_EXPR then vos_symbol else expr_const(0, *pool); // neg = Vos
            print("Setting % → 0\n", neg_node);
            array_add(*voltages, .{ expr_symbol(neg_node, *pool), target });
            array_unordered_remove_by_value(*solve_vars, expr_symbol(neg_node, *pool));
            pin_node_equation(neg_node, target);
            if vos_symbol == NO_EXPR  substitute_node(neg_node, "0");
        } else if neg_node == "0" {
            target := ifx vos_symbol != NO_EXPR then expr_neg(vos_symbol, *pool) else expr_const(0, *pool); // pos = -Vos
            print("Setting % → 0\n", pos_node);
            array_add(*voltages, .{ expr_symbol(pos_node, *pool), target });
            array_unordered_remove_by_value(*solve_vars, expr_symbol(pos_node, *pool));
            pin_node_equation(pos_node, target);
            if vos_symbol == NO_EXPR  substitute_node(pos_node, "0");
        } else {
            target := ifx vos_symbol != NO_EXPR then expr_sub(expr_symbol(neg_node, *pool), vos_symbol, *pool) else expr_symbol(neg_node, *pool); // pos = neg - Vos
            print("Setting % → %\n", pos_node, neg_node);
            array_add(*voltages, .{ expr_symbol(pos_node, *pool), target });
            array_unordered_remove_by_value(*solve_vars, expr_symbol(pos_node, *pool));
            pin_node_equation(pos_node, target);
            if vos_symbol == NO_EXPR  substitute_node(pos_node, neg_node);
        }
    }

    // INDUCTOR DC (short circuit): merge its two terminals into one node,
    // same as an ideal op-amp's virtual short.
    if dc_mode {
        for el : elements {
            if el.type != .INDUCTOR continue;
            pos_node := el.terminals[0];
            neg_node := el.terminals[1];
            current_symbol := expr_symbol(tprint("i%", el.name), *pool);

            for node, idx : eqn_nodes {
                if node == pos_node equations[idx] = combine(equations[idx], current_symbol, .SUB, *pool); // current leaving this node
                if node == neg_node equations[idx] = combine(equations[idx], current_symbol, .ADD, *pool); // current entering this node
            }
            array_add(*solve_vars, current_symbol);

            if pos_node == "0" {
                array_add(*voltages, .{ expr_symbol(neg_node, *pool), expr_const(0, *pool) });
                array_unordered_remove_by_value(*solve_vars, expr_symbol(neg_node, *pool));
                pin_node_equation(neg_node, expr_const(0, *pool));
                substitute_node(neg_node, "0");
            } else if neg_node == "0" {
                array_add(*voltages, .{ expr_symbol(pos_node, *pool), expr_const(0, *pool) });
                array_unordered_remove_by_value(*solve_vars, expr_symbol(pos_node, *pool));
                pin_node_equation(pos_node, expr_const(0, *pool));
                substitute_node(pos_node, "0");
            } else {
                array_add(*voltages, .{ expr_symbol(pos_node, *pool), expr_symbol(neg_node, *pool) });
                array_unordered_remove_by_value(*solve_vars, expr_symbol(pos_node, *pool));
                pin_node_equation(pos_node, expr_symbol(neg_node, *pool));
                substitute_node(pos_node, neg_node);
            }
        }
    }

    // substitute current and voltage expressions.
    all_subs: [..] Substitution;
    all_subs.allocator = temp;
    array_reserve(*all_subs, currents.count + voltages.count);
    for c : currents   array_add(*all_subs, c);
    for v : voltages   array_add(*all_subs, v);
    substitute_batch(all_subs, *pool);

    // At DC, a node reachable only through a dropped capacitor and a diode
    // branch gets a KCL row with no solve_var at all (just "-iDiode = 0").
    // Zero current through an odd I-V law means zero junction voltage, so
    // just pin the node to its diode partner's voltage.
    if dc_mode {
        for node, idx : eqn_nodes {
            if equations[idx] == NO_EXPR continue;
            if expr_depends_on(equations[idx], solve_vars, pool.exprs) continue;

            for el : elements {
                if el.type != .DIODE && el.type != .DIODE_PAIR continue;
                other := "";
                if      el.terminals[0] == node then other = el.terminals[1];
                else if el.terminals[1] == node then other = el.terminals[0];
                if other.count == 0 continue;

                other_symbol := ifx other == "0" then expr_const(0, *pool) else expr_symbol(other, *pool);
                equations[idx] = expr_sub(expr_symbol(node, *pool), other_symbol, *pool);
                pinned[idx] = true;
                break;
            }
        }
    }

    // gmin: a group of nodes can be jointly floating at DC even if no single
    // row is empty (e.g. a resistor chain hanging off a diode-pinned node:
    // every row is real, but the group has no path to ground). A tiny
    // leakage conductance on every non-pinned node breaks that degeneracy,
    // same as real SPICE's gmin-to-ground regularization.
    if dc_mode {
        // expr_const is integer-only, so build 1e-9 as 1/1e9.
        gmin := expr_div(expr_const(1, *pool), expr_const(1_000_000_000, *pool), *pool);
        for node, idx : eqn_nodes {
            if equations[idx] == NO_EXPR continue;
            if pinned[idx] continue;
            leak := expr_mul(gmin, expr_symbol(node, *pool), *pool);
            equations[idx] = expr_sub(equations[idx], leak, *pool);
        }
    }

    return solve_vars, equations, pool, elements;
}

// Look up the closed-form linear MNA solution for `node`.
lookup_node_voltage :: (node: string, solve_vars: [] Expr_ID, solutions: [] Expr_ID, pool: *Expr_Pool, inputs: [] string = .[]) -> Expr_ID {
    if node == "0"  return expr_const(0, pool);
    if array_find(inputs, node)  return expr_symbol(node, pool);
    node_symbol := expr_symbol(node, pool);
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
                                          pool: *Expr_Pool) -> [] Junction_Voltage {
    junctions: [..] Junction_Voltage;

    for el : elements {
        spec := nonlinear_device_spec(el.type);
        for u : spec.unknowns {
            pos_v := lookup_node_voltage(el.terminals[u.pos_terminal], solve_vars, solutions, pool);
            neg_v := lookup_node_voltage(el.terminals[u.neg_terminal], solve_vars, solutions, pool);
            v := simplify(expr_sub(pos_v, neg_v, pool), pool);
            array_add(*junctions, .{ name = tprint("%1%2", u.name_prefix, el.name), expr_id = v });
        }
    }

    return junctions;
}

// -----------------------------------------------------------------------
// Circuit partitioning: `sub-circuit` directive support.
// -----------------------------------------------------------------------

// One `* sub-circuit: <name> <in_node> <out_node>` directive.
Sub_Circuit_Decl :: struct {
    name:     string;
    in_node:  string;
    out_node: string;
}

// One named piece cut apart by partition_circuit().
Circuit_Partition :: struct {
    name:     string;
    elements: [..] Element;
    in_node:  string;
    out_node: string;
}

// Splits `elements` into one piece per decl, in decl order. Ground ("0")
// and every decl's in/out node are excluded from the connectivity graph.
// Elements only union through some other shared node, which is the cut.
// Each resulting group is matched to whichever decl's in_node/out_node it
// touches. Only exact for a decl boundary that's a true ideal-source node.
partition_circuit :: (elements: [] Element, decls: [] Sub_Circuit_Decl) -> [] Circuit_Partition {
    if decls.count == 0 {
        single: Circuit_Partition;
        array_copy(*single.elements, elements);
        result: [..] Circuit_Partition;
        array_add(*result, single);
        return result;
    }

    is_boundary :: (node: string, decls: [] Sub_Circuit_Decl) -> bool #expand {
        for d : decls  if d.in_node == node || d.out_node == node  return true;
        return false;
    }
    is_real :: (node: string, decls: [] Sub_Circuit_Decl) -> bool #expand {
        return node != "0" && !is_boundary(node, decls);
    }

    find :: (parent: [] int, x: int) -> int {
        root := x;
        while parent[root] != root  root = parent[root];
        cur := x;
        while parent[cur] != root { next := parent[cur]; parent[cur] = root; cur = next; }
        return root;
    }
    dsu_union :: (parent: [] int, a: int, b: int) {
        ra := find(parent, a); rb := find(parent, b);
        if ra != rb  parent[ra] = rb;
    }

    // Elements with no real terminal at all (only ground/boundary nodes,
    // e.g. a load resistor from a boundary node straight to ground) can't
    // be placed by shared-node union; they're handled separately below.
    real_idxs, orphan_idxs: [..] int;
    for i : 0..elements.count-1 {
        has_real := false;
        for term : elements[i].terminals  if is_real(term, decls) { has_real = true; break; }
        if has_real  array_add(*real_idxs, i); else array_add(*orphan_idxs, i);
    }

    parent: [..] int;
    for i : 0..elements.count-1  array_add(*parent, i);

    node_elements: Table(string, [..] int);
    node_elements.allocator = temp;
    for i : real_idxs {
        for term : elements[i].terminals {
            if !is_real(term, decls)  continue;
            list := table_find_pointer(*node_elements, term);
            if !list { empty: [..] int; empty.allocator = temp; list = table_add(*node_elements, term, empty); }
            array_add(list, i);
        }
    }
    for list : node_elements  for k : 1..list.count-1  dsu_union(parent, list[0], list[k]);

    group_of_root: Table(int, int);
    group_of_root.allocator = temp;
    groups: [..] [..] int;
    groups.allocator = temp;
    for i : real_idxs {
        r := find(parent, i);
        found, gi := table_find(*group_of_root, r);
        if !found { gi = groups.count; table_add(*group_of_root, r, gi); empty: [..] int; empty.allocator = temp; array_add(*groups, empty); }
        array_add(*groups[gi], i);
    }

    touches :: (elements: [] Element, group: [] int, node: string) -> bool {
        for ei : group  for term : elements[ei].terminals  if term == node  return true;
        return false;
    }

    matched: [..] int; // decl index -> group index, -1 if unmatched
    matched.allocator = temp;
    for d : decls {
        gi := -1;
        for g, i : groups  if touches(elements, g, d.in_node) && touches(elements, g, d.out_node) { gi = i; break; }
        array_add(*matched, gi);
        if gi == -1  log("Warning: no elements match sub-circuit '%' (% -> %).\n", d.name, d.in_node, d.out_node);
    }

    // Orphans can't change any node's voltage either way (a load on an
    // already-driven node doesn't affect it), so any matching decl is fine
    // to attach to. "first in decl order" is just the tiebreak.
    for oi : orphan_idxs {
        for d, di : decls {
            gi := matched[di];
            if gi == -1  continue;
            touches_decl := false;
            for term : elements[oi].terminals  if term == d.in_node || term == d.out_node  { touches_decl = true; break; }
            if touches_decl { array_add(*groups[gi], oi); break; }
        }
    }

    result: [..] Circuit_Partition;
    for d, di : decls {
        part: Circuit_Partition;
        part.name     = d.name;
        part.in_node  = d.in_node;
        part.out_node = d.out_node;
        gi := matched[di];
        if gi != -1  for ei : groups[gi]  array_add(*part.elements, elements[ei]);
        array_add(*result, part);
    }
    return result;
}
