// Newton-Raphson system construction for nonlinear elements (DIODE /
// DIODE_PAIR / BJT), on top of the linear MNA core in nodal_analysis.jai /
// sym_algebra.jai.
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
    name:          string; // e.g. "vBCQ1" -- also the State field name
    vt_name:       string; // Params field for thermal voltage (used for codegen's vcrit); blank for JFET
    is_name:       string; // Params field for saturation current (also for vcrit)
    vp_name:       string; // JFET only: Params field for pinch-off voltage (used for codegen's clamp)
    residual_name: string; // Gen_Var name holding this unknown's residual f_j
    delta_name:    string; // Gen_Var name holding this unknown's Newton delta
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
jfet_iv_law :: (junction_voltage_name: string, model: Nonlinear_Model, exprs: *[..] Expr) -> Expr_ID {
    vgs  := expr_symbol(junction_voltage_name, exprs);
    vp   := expr_symbol(model.vp_name, exprs);
    beta := expr_symbol(model.beta_name, exprs);
    diff := expr_sub(vgs, vp, exprs);
    return expr_mul(beta, expr_mul(diff, diff, exprs), exprs);
}

// Branch-current symbol name(s) this device contributes as free symbols in
// the linear KCL system (see nodal_analysis.generate_kcl_equations).
branch_current_names :: (el: Element) -> [..] string {
    names: [..] string;
    if el.type == .DIODE || el.type == .DIODE_PAIR || el.type == .JFET  array_add(*names, tprint("i%", el.name));
    if el.type == .BJT {
        array_add(*names, tprint("iB%", el.name));
        array_add(*names, tprint("iC%", el.name));
    }
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
    name:        string; // e.g. "vBCQ1"
    vt_name:     string;
    is_name:     string;
    vp_name:     string; // JFET only
    linear_form: Expr_ID; // closed-form node-voltage-difference; still references raw branch-current symbols
    device_idx:  int;     // index into `devices`
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
        if el.type == {
            case .DIODE; #through;
            case .DIODE_PAIR;
                device_idx := devices.count;
                array_add(*devices, el);
                anode_v   := copy_expr(lookup_node_voltage(el.terminals[0], solve_vars, solutions, exprs), exprs);
                cathode_v := copy_expr(lookup_node_voltage(el.terminals[1], solve_vars, solutions, exprs), exprs);
                v := simplify(expr_sub(anode_v, cathode_v, exprs), exprs);
                array_add(*raw_unknowns, .{ name = tprint("v%", el.name), vt_name = el.nonlinear.vt_name,
                                             is_name = el.nonlinear.is_name,
                                             linear_form = v, device_idx = device_idx });

            case .BJT;
                device_idx := devices.count;
                array_add(*devices, el);
                collector_v := copy_expr(lookup_node_voltage(el.terminals[0], solve_vars, solutions, exprs), exprs);
                base_v_bc   := copy_expr(lookup_node_voltage(el.terminals[1], solve_vars, solutions, exprs), exprs);
                base_v_be   := copy_expr(lookup_node_voltage(el.terminals[1], solve_vars, solutions, exprs), exprs);
                emitter_v   := copy_expr(lookup_node_voltage(el.terminals[2], solve_vars, solutions, exprs), exprs);
                vbc := simplify(expr_sub(base_v_bc, collector_v, exprs), exprs);
                vbe := simplify(expr_sub(base_v_be, emitter_v,   exprs), exprs);
                array_add(*raw_unknowns, .{ name = tprint("vBC%", el.name), vt_name = el.nonlinear.vt_name,
                                             is_name = el.nonlinear.is_name,
                                             linear_form = vbc, device_idx = device_idx });
                array_add(*raw_unknowns, .{ name = tprint("vBE%", el.name), vt_name = el.nonlinear.vt_name,
                                             is_name = el.nonlinear.is_name,
                                             linear_form = vbe, device_idx = device_idx });

            case .JFET;
                device_idx := devices.count;
                array_add(*devices, el);
                gate_v   := copy_expr(lookup_node_voltage(el.terminals[1], solve_vars, solutions, exprs), exprs);
                source_v := copy_expr(lookup_node_voltage(el.terminals[2], solve_vars, solutions, exprs), exprs);
                vgs := simplify(expr_sub(gate_v, source_v, exprs), exprs);
                // vt_name blank: no exponential to clamp here; vp_name feeds the pinch-off clamp instead.
                array_add(*raw_unknowns, .{ name = tprint("vGS%", el.name), vt_name = "", is_name = "",
                                             vp_name = el.nonlinear.vp_name,
                                             linear_form = vgs, device_idx = device_idx });
        }
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

    // Build the I-V law for every device up front.
    iv_law_ids:   [..] Expr_ID; // parallel to `devices`; for BJT this holds iC (iB handled separately below)
    iv_law_names: [..] string;  // branch-current symbol name this iv_law_ids entry replaces
    for d, idx : devices {
        if d.type == .DIODE || d.type == .DIODE_PAIR {
            // Find this device's (only) unknown name.
            v_name := "";
            for ru : raw_unknowns  if ru.device_idx == idx  v_name = ru.name;
            iv := ifx d.type == .DIODE then diode_iv_law(v_name, d.nonlinear, exprs)
                                       else diode_pair_iv_law(v_name, d.nonlinear, exprs);
            array_add(*iv_law_ids,   iv);
            array_add(*iv_law_names, tprint("i%", d.name));
        } else if d.type == .BJT {
            vbc_name := ""; vbe_name := "";
            for ru : raw_unknowns {
                if ru.device_idx != idx  continue;
                if String.begins_with(ru.name, "vBC")  vbc_name = ru.name;
                if String.begins_with(ru.name, "vBE")  vbe_name = ru.name;
            }
            ib, ic := bjt_iv_laws(vbc_name, vbe_name, d.nonlinear, exprs);
            array_add(*iv_law_ids,   ib);
            array_add(*iv_law_names, tprint("iB%", d.name));
            array_add(*iv_law_ids,   ic);
            array_add(*iv_law_names, tprint("iC%", d.name));
        } else if d.type == .JFET {
            v_name := "";
            for ru : raw_unknowns  if ru.device_idx == idx  v_name = ru.name;
            iv := jfet_iv_law(v_name, d.nonlinear, exprs);
            array_add(*iv_law_ids,   iv);
            array_add(*iv_law_names, tprint("i%", d.name));
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

            vt_name := ""; is_name := ""; vp_name := "";
            for ru : raw_unknowns {
                if ru.name != name  continue;
                vt_name = ru.vt_name;
                is_name = ru.is_name;
                vp_name = ru.vp_name;
            }
            array_add(*cluster.unknowns, Cluster_Unknown.{
                name = name, vt_name = vt_name, is_name = is_name, vp_name = vp_name,
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
