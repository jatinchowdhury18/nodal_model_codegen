// Newton-Raphson system construction for nonlinear elements (DIODE /
// DIODE_PAIR / BJT / JFET), on top of the linear MNA core in
// nodal_analysis.jai / sym_algebra.jai. Which unknowns/branch currents each
// device type contributes is data (nonlinear_device_spec(), nodal_analysis.jai);
// the actual I-V law per type is code (device_iv_law(), below).
//
// generate_kcl_equations() leaves nonlinear branch currents (i<name>,
// iB<name>/iC<name>) as free symbols, so solve() gives closed-form node
// voltages parametrized by those currents. From there we:
//   1. Group devices into "clusters" -- connected components whose branch
//      currents show up in each other's junction-voltage expressions, so
//      they need one joint Newton-Raphson solve.
//   2. Build each device's I-V law (Shockley for diodes, Ebers-Moll
//      for BJT) as an Expr in its own junction voltage(s).
//   3. Substitute those I-V laws in for the branch-current symbols.
//   4. Build residuals f_j = closed_form_j - unknown_j, differentiate for
//      the Jacobian, and solve J*delta = -f.
//   5. Run cse()/licm()/recip_hoist() on the result, using the cluster's own
//      unknowns as the variant base -- hoists Newton-iteration-invariant
//      subexpressions the same way licm() hoists per-sample-invariant ones,
//      just one loop level in.
//
// This module doesn't emit any C/C++ or handle damping/convergence/limiter
// control flow -- that's imperative and lives in netlist_codegen.jai.

#import "Basic";
String :: #import "String";
#import "sym_algebra";
#import "nodal_analysis";

// One Newton unknown belonging to a cluster (a device's junction voltage).
Cluster_Unknown :: struct {
    name:          string;       // e.g. "vBCQ1" -- also the State field name
    limiter_kind:  Limiter_Kind;
    vt_name:       string;       // valid when limiter_kind == .EXP_JUNCTION; Params field for thermal voltage
    is_name:       string;       // valid when limiter_kind == .EXP_JUNCTION; Params field for saturation current
    vp_name:       string;       // valid when limiter_kind == .JFET_PINCHOFF; Params field for pinch-off voltage
    vsat_p_name:   string;       // valid when limiter_kind == .AMP_CLIP; Params field for positive output rail
    vsat_n_name:   string;       // valid when limiter_kind == .AMP_CLIP; Params field for negative output rail
    has_slew:      bool;         // valid when limiter_kind == .AMP_CLIP; true if this op-amp also has slew set
    slew_name:     string;       // valid when has_slew; Params field for max |dVout/dt|, V/s
    zslew_name:    string;       // valid when has_slew; State field for the previous sample's output
    residual_name: string;       // Gen_Var name holding this unknown's residual f_j
    delta_name:    string;       // Gen_Var name holding this unknown's Newton delta
}

// A group of nonlinear devices solved via one joint Newton-Raphson system.
Nonlinear_Cluster :: struct {
    name:     string; // diagnostic name, e.g. "Q1_D1D2" (joined device names)
    unknowns: [..] Cluster_Unknown;

    // Newton-invariant CSE temps (don't depend on this cluster's unknowns) --
    // emitted once before the per-sample Newton loop.
    invariant_items: [..] Gen_Var;

    // Newton-variant CSE temps plus the residual/delta Gen_Vars, topologically
    // ordered. Recomputed every Newton iteration.
    iteration_items: [..] Gen_Var;
}

// -----------------------------------------------------------------------
// Device I-V laws
// -----------------------------------------------------------------------

// Single-diode Shockley I-V law: i = Is*(exp(v/vt) - 1).
diode_iv_law :: (junction_voltage_name: string, model: Nonlinear_Model, exprs: *[..] Expr) -> Expr_ID {
    v   := expr_symbol(junction_voltage_name, exprs);
    vt  := expr_symbol(model.vt_name, exprs);
    is_ := expr_symbol(model.is_name, exprs);
    e   := expr_exp(expr_div(v, vt, exprs), exprs);
    return expr_mul(is_, expr_sub(e, expr_const(1, exprs), exprs), exprs);
}

// Antiparallel diode-pair I-V law: i = Is*(exp(v/vt) - 1/exp(v/vt))
// Could also be written as: i =  2*Is*sinh(v/vt).
diode_pair_iv_law :: (junction_voltage_name: string, model: Nonlinear_Model, exprs: *[..] Expr) -> Expr_ID {
    v     := expr_symbol(junction_voltage_name, exprs);
    vt    := expr_symbol(model.vt_name, exprs);
    is_   := expr_symbol(model.is_name, exprs);
    e     := expr_exp(expr_div(v, vt, exprs), exprs);
    inv_e := expr_div(expr_const(1, exprs), e, exprs);
    return expr_mul(is_, expr_sub(e, inv_e, exprs), exprs);
}

// Ebers-Moll BJT base/collector current I-V laws:
//   iB = Is*((exp(vBE/vt)-1)/BetaF + (exp(vBC/vt)-1)/BetaR)
//   iC = Is*((exp(vBE/vt)-exp(vBC/vt)) - (exp(vBC/vt)-1)/BetaR)
bjt_iv_laws :: (vbc_name: string, vbe_name: string, model: Nonlinear_Model, exprs: *[..] Expr) -> (ib: Expr_ID, ic: Expr_ID) {
    vbc := expr_symbol(vbc_name, exprs);
    vbe := expr_symbol(vbe_name, exprs);
    vt    := expr_symbol(model.vt_name,    exprs);
    is_   := expr_symbol(model.is_name,    exprs);
    betaf := expr_symbol(model.betaf_name, exprs);
    betar := expr_symbol(model.betar_name, exprs);

    exp_be := expr_exp(expr_div(vbe, vt, exprs), exprs);
    exp_bc := expr_exp(expr_div(vbc, vt, exprs), exprs);

    ib := expr_mul(is_,
              expr_add(
                  expr_div(expr_sub(exp_be, expr_const(1, exprs), exprs), betaf, exprs),
                  expr_div(expr_sub(exp_bc, expr_const(1, exprs), exprs), betar, exprs),
                  exprs),
              exprs);

    ic := expr_mul(is_,
              expr_sub(
                  expr_sub(exp_be, exp_bc, exprs),
                  expr_div(expr_sub(exp_bc, expr_const(1, exprs), exprs), betar, exprs),
                  exprs),
              exprs);

    return ib, ic;
}

// SPICE Level-1 JFET drain-current I-V law: iD = Beta*(vGS - Vp)^2.
//
// d(iD)/d(vGS) hits exactly 0 at pinch-off. Harmless solved alone, but if
// this device shares a Newton cluster with another one (see clustering
// above), a zero Jacobian entry there can ill-condition their shared linear
// solve. 1mV epsilon floor keeps it off exact zero -- same idea as gmin.
jfet_iv_law :: (junction_voltage_name: string, model: Nonlinear_Model, exprs: *[..] Expr) -> Expr_ID {
    vgs  := expr_symbol(junction_voltage_name, exprs);
    vp   := expr_symbol(model.vp_name, exprs);
    beta := expr_symbol(model.beta_name, exprs);
    epsilon := expr_div(expr_const(1, exprs), expr_const(1000, exprs), exprs);
    diff := expr_add(expr_sub(vgs, vp, exprs), epsilon, exprs);
    return expr_mul(beta, expr_mul(diff, diff, exprs), exprs);
}

// One device current, in terms of its junction-voltage unknown(s): the
// branch-current symbol `i<suffix><el.name>` (see Device_Kcl_Term) gets
// substituted with `expr`.
Device_Branch_Current :: struct {
    suffix: string;
    expr:   Expr_ID;
}

// Dispatches to the right I-V law function above for `el.type`. This is the
// one place that needs a new case when adding a device type -- everything
// else in this file is driven generically by nonlinear_device_spec().
device_iv_law :: (el: Element, unknown_names: [] string, exprs: *[..] Expr) -> [] Device_Branch_Current {
    result: [..] Device_Branch_Current;
    if el.type == {
        case .DIODE;
            iv := diode_iv_law(unknown_names[0], el.nonlinear, exprs);
            array_add(*result, .{ suffix = "", expr = iv });
        case .DIODE_PAIR;
            iv := diode_pair_iv_law(unknown_names[0], el.nonlinear, exprs);
            array_add(*result, .{ suffix = "", expr = iv });
        case .BJT;
            vbc_name := ""; vbe_name := "";
            for n : unknown_names {
                if String.begins_with(n, "vBC")  vbc_name = n;
                if String.begins_with(n, "vBE")  vbe_name = n;
            }
            ib, ic := bjt_iv_laws(vbc_name, vbe_name, el.nonlinear, exprs);
            array_add(*result, .{ suffix = "B", expr = ib });
            array_add(*result, .{ suffix = "C", expr = ic });
        case .JFET;
            iv := jfet_iv_law(unknown_names[0], el.nonlinear, exprs);
            array_add(*result, .{ suffix = "", expr = iv });
    }
    return result;
}

// Branch-current symbol name(s) this device contributes as free symbols in
// the linear KCL system (see nodal_analysis.generate_kcl_equations). An
// AMP's clip/slew unknown isn't a branch current (it's the driving voltage
// itself, used directly by Rout/out_node's stamp -- see opamp_clip_node),
// but it plays the same "free symbol used elsewhere in the linear system"
// role for clustering purposes, so it's included here too.
branch_current_names :: (el: Element) -> [..] string {
    names: [..] string;
    spec := nonlinear_device_spec(el.type);
    for term : spec.kcl_terms  array_add_if_unique(*names, tprint("i%1%2", term.branch_suffix, el.name));
    if el.type == .AMP && opamp_has_clip_stage(el)  array_add_if_unique(*names, tprint("vclip_%", el.name));
    return names;
}

// -----------------------------------------------------------------------
// Clustering: connected components of devices whose branch currents appear
// in each other's closed-form junction-voltage expressions.
// -----------------------------------------------------------------------

Union_Find :: struct {
    parent: [..] int;
}

uf_make :: (n: int) -> Union_Find {
    uf: Union_Find;
    for i : 0..n-1  array_add(*uf.parent, i);
    return uf;
}

uf_find :: (uf: *Union_Find, x: int) -> int {
    root := x;
    while uf.parent[root] != root  root = uf.parent[root];
    cur := x;
    while uf.parent[cur] != root {
        next := uf.parent[cur];
        uf.parent[cur] = root;
        cur = next;
    }
    return root;
}

uf_union :: (uf: *Union_Find, a: int, b: int) {
    ra := uf_find(uf, a);
    rb := uf_find(uf, b);
    if ra != rb  uf.parent[ra] = rb;
}

// One nonlinear device plus its not-yet-substituted junction-voltage
// unknown(s), before clustering/I-V substitution.
Raw_Unknown :: struct {
    name:         string; // e.g. "vBCQ1"
    limiter_kind: Limiter_Kind;
    vt_name:      string;
    is_name:      string;
    vp_name:      string;
    vsat_p_name:  string;
    vsat_n_name:  string;
    has_slew:     bool;
    slew_name:    string;
    zslew_name:   string;
    linear_form:  Expr_ID; // closed-form node-voltage-difference; still references raw branch-current symbols
    device_idx:   int;     // index into `devices`
}

// -----------------------------------------------------------------------
// Top-level entry point.
// -----------------------------------------------------------------------
//
// Call after generate_kcl_equations()/solve() have produced solve_vars/
// solutions over `exprs`, and after reduce_circuit()'s DIODE_PAIR pass.
//
// Side effect: every nonlinear branch-current symbol (i<name>, iB<name>/
// iC<name>) gets substituted with its device's I-V law, in terms of the
// junction-voltage unknowns.
build_nonlinear_clusters :: (elements: [] Element,
                             solve_vars: [] Expr_ID,
                             solutions: [] Expr_ID,
                             exprs: *[..] Expr) -> [] Nonlinear_Cluster {
    // Collect nonlinear devices and their raw (pre-substitution) junction
    // voltage unknowns.
    devices: [..] Element;
    raw_unknowns: [..] Raw_Unknown;

    for el : elements {
        spec := nonlinear_device_spec(el.type);
        if spec.unknowns.count == 0  continue;

        device_idx := devices.count;
        array_add(*devices, el);
        for u : spec.unknowns {
            pos_v := copy_expr(lookup_node_voltage(el.terminals[u.pos_terminal], solve_vars, solutions, exprs), exprs);
            neg_v := copy_expr(lookup_node_voltage(el.terminals[u.neg_terminal], solve_vars, solutions, exprs), exprs);
            v := simplify(expr_sub(pos_v, neg_v, exprs), exprs);
            vt_name, is_name, vp_name := device_limiter_names(u.limiter, el.nonlinear);
            array_add(*raw_unknowns, .{ name = tprint("%1%2", u.name_prefix, el.name), limiter_kind = u.limiter,
                                         vt_name = vt_name, is_name = is_name, vp_name = vp_name,
                                         linear_form = v, device_idx = device_idx });
        }
    }

    // Op-amp output clip/slew stage. Not in nonlinear_device_spec() -- unlike
    // DIODE/DIODE_PAIR/BJT/JFET, whether an AMP is nonlinear depends on its
    // instance-level opamp fields, not just its Element_Type (see
    // opamp_has_clip_stage). Its unknown is a single node's closed-form
    // voltage (opamp_bandwidth_node's solved value, i.e. the ideal/band-
    // limited gain-stage output before clipping), not a terminal-pair
    // difference, and it needs no I-V law substitution -- its free symbol
    // (vclip_<name>, see branch_current_names) *is* the Newton unknown, so
    // there's nothing to substitute the way a diode's current->voltage
    // hand-off needs.
    for el : elements {
        if el.type != .AMP || !opamp_has_clip_stage(el)  continue;

        device_idx := devices.count;
        array_add(*devices, el);
        v := copy_expr(lookup_node_voltage(opamp_bandwidth_node(el), solve_vars, solutions, exprs), exprs);
        simplify(v, exprs);
        array_add(*raw_unknowns, .{ name = tprint("vclip_%", el.name), limiter_kind = .AMP_CLIP,
                                     vsat_p_name = el.opamp.vsat_p_name, vsat_n_name = el.opamp.vsat_n_name,
                                     has_slew = el.opamp.has_slew, slew_name = el.opamp.slew_name,
                                     zslew_name = tprint("zslew_%", el.name),
                                     linear_form = v, device_idx = device_idx });
    }

    if devices.count == 0 {
        empty: [] Nonlinear_Cluster;
        return empty;
    }

    // Union devices whose branch-current symbols appear in each other's
    // linear_form (checked both directions -- see module comment).
    uf := uf_make(devices.count);
    for a : 0..devices.count-1 {
        a_names := branch_current_names(devices[a]);
        for b : a+1..devices.count-1 {
            b_names := branch_current_names(devices[b]);
            coupled := false;

            for ru : raw_unknowns {
                if ru.device_idx != a  continue;
                refs: [..] string;
                collect_symbol_names(ru.linear_form, exprs.*, *refs);
                for n : b_names  if array_find(refs, n)  coupled = true;
            }
            for ru : raw_unknowns {
                if ru.device_idx != b  continue;
                refs: [..] string;
                collect_symbol_names(ru.linear_form, exprs.*, *refs);
                for n : a_names  if array_find(refs, n)  coupled = true;
            }

            if coupled  uf_union(*uf, a, b);
        }
    }

    // Group device indices by cluster root.
    roots: [..] int;         // one entry per distinct cluster, in first-seen order
    root_members: [..] [..] int; // parallel to `roots`: device indices in that cluster
    for d : 0..devices.count-1 {
        root := uf_find(*uf, d);
        found, idx := array_find(roots, root);
        if !found {
            array_add(*roots, root);
            new_members: [..] int;
            array_add(*root_members, new_members);
            idx = roots.count - 1;
        }
        array_add(*root_members[idx], d);
    }

    // Build the I-V law for every device up front, via device_iv_law()'s
    // per-type dispatch.
    iv_law_ids:   [..] Expr_ID; // parallel entries: branch current expr...
    iv_law_names: [..] string;  // ...and the symbol name it replaces
    for d, idx : devices {
        unknown_names: [..] string;
        for ru : raw_unknowns  if ru.device_idx == idx  array_add(*unknown_names, ru.name);

        for bc : device_iv_law(d, unknown_names, exprs) {
            array_add(*iv_law_ids,   bc.expr);
            array_add(*iv_law_names, tprint("i%1%2", bc.suffix, d.name));
        }
    }

    // Substitute every branch-current symbol with its I-V law, in place.
    for id, i : iv_law_ids {
        name := iv_law_names[i];
        matches: [..] Expr_ID;
        for ex, node_id : exprs.*  if ex.kind == .SYMBOL && ex.name == name  array_add(*matches, node_id);
        for m : matches  substitute(m, id, exprs);
    }

    // Build one Nonlinear_Cluster per connected component.
    clusters: [..] Nonlinear_Cluster;
    for members, ci : root_members {
        cluster: Nonlinear_Cluster;

        name_parts: [..] string;
        for d_idx : members  array_add(*name_parts, devices[d_idx].name);
        cluster.name = String.join(..name_parts, separator = "_");

        // Collect this cluster's unknowns (all raw_unknowns whose device is a member).
        cluster_unknown_names: [..] string;
        for ru : raw_unknowns {
            is_member := array_find(members, ru.device_idx);
            if !is_member  continue;
            array_add(*cluster_unknown_names, ru.name);
        }

        f_ids:     [..] Expr_ID; // parallel to cluster_unknown_names
        for name : cluster_unknown_names {
            ru_idx := -1;
            for ru, i : raw_unknowns  if ru.name == name  ru_idx = i;
            closed_form := copy_expr(raw_unknowns[ru_idx].linear_form, exprs);
            simplify(closed_form, exprs);
            unknown_symbol := expr_symbol(name, exprs);
            f := simplify(expr_sub(closed_form, unknown_symbol, exprs), exprs);
            array_add(*f_ids, f);
        }

        k := cluster_unknown_names.count;

        // Jacobian: J[j][m] = d(f_j)/d(unknown_m)
        jacobian: [..] Expr_ID; // row-major, k*k
        for j : 0..k-1 {
            for m : 0..k-1 {
                unknown_m := expr_symbol(cluster_unknown_names[m], exprs);
                dj := differentiate(f_ids[j], unknown_m, exprs);
                simplify(dj, exprs);
                array_add(*jacobian, dj);
            }
        }

        // Newton delta solve: J*delta = -f, i.e. f_j + sum_m(J[j][m]*delta_m) = 0.
        delta_ids: [..] Expr_ID;
        for name : cluster_unknown_names  array_add(*delta_ids, expr_symbol(tprint("delta_%", name), exprs));

        delta_equations: [..] Expr_ID;
        for j : 0..k-1 {
            eq := f_ids[j];
            for m : 0..k-1 {
                eq = expr_add(eq, expr_mul(jacobian[j*k + m], delta_ids[m], exprs), exprs);
            }
            array_add(*delta_equations, eq);
        }
        delta_solutions := solve(delta_equations, delta_ids, exprs);

        // Named Gen_Vars: residual (for the convergence check) and delta
        // (the Newton update) per unknown.
        gen_vars: [..] Gen_Var;
        for name, j : cluster_unknown_names {
            residual_name := tprint("res_%", name);
            delta_name    := tprint("delta_%", name);
            array_add(*gen_vars, Gen_Var.{ name = residual_name, expr_id = f_ids[j] });
            array_add(*gen_vars, Gen_Var.{ name = delta_name,    expr_id = delta_solutions[j] });

            limiter_kind: Limiter_Kind; vt_name := ""; is_name := ""; vp_name := ""; vsat_p_name := ""; vsat_n_name := "";
            has_slew := false; slew_name := ""; zslew_name := "";
            for ru : raw_unknowns {
                if ru.name != name  continue;
                limiter_kind = ru.limiter_kind;
                vt_name = ru.vt_name;
                is_name = ru.is_name;
                vp_name = ru.vp_name;
                vsat_p_name = ru.vsat_p_name;
                vsat_n_name = ru.vsat_n_name;
                has_slew = ru.has_slew;
                slew_name = ru.slew_name;
                zslew_name = ru.zslew_name;
            }
            array_add(*cluster.unknowns, Cluster_Unknown.{
                name = name, limiter_kind = limiter_kind, vt_name = vt_name, is_name = is_name, vp_name = vp_name,
                vsat_p_name = vsat_p_name, vsat_n_name = vsat_n_name,
                has_slew = has_slew, slew_name = slew_name, zslew_name = zslew_name,
                residual_name = residual_name, delta_name = delta_name });
        }

        // Same CSE/LICM/recip_hoist recipe as solve_and_codegen(), but the
        // loop-variant base is this cluster's own unknowns -- one loop level
        // in from the per-sample base -- so it hoists Newton-invariant
        // subexpressions automatically.
        gen_var_ids: [..] Expr_ID;
        for gv : gen_vars  array_add(*gen_var_ids, gv.expr_id);

        // Cluster-unique temp prefix, since this Newton-loop code shares a
        // C++ scope with the linear pipeline's own temps (and other clusters').
        temp_prefix := tprint("_%1_t", cluster.name);

        cse_vars := cse(gen_var_ids, exprs, temp_prefix);
        sort_cse_topological(*cse_vars, exprs.*);
        licm(gen_var_ids, *cse_vars, exprs, cluster_unknown_names, temp_prefix);
        recip_hoist(gen_var_ids, *cse_vars, exprs, cluster_unknown_names, temp_prefix);

        variant_cse_names: [..] string;
        for cv : cse_vars {
            if expr_is_loop_variant(cv.expr_id, exprs.*, cluster_unknown_names, variant_cse_names) {
                array_add(*variant_cse_names, cv.name);
                array_add(*cluster.iteration_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
            } else {
                array_add(*cluster.invariant_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
            }
        }
        for gv : gen_vars  array_add(*cluster.iteration_items, gv);

        array_add(*clusters, cluster);
    }

    return clusters;
}
