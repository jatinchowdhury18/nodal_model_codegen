// Newton-Raphson system construction for nonlinear elements (DIODE /
// DIODE_PAIR / BJT / JFET), on top of the linear MNA core in
// nodal_analysis.jai / sym_algebra.jai. Per-type unknowns/branch currents
// are data (nonlinear_device_spec(), nodal_analysis.jai); the I-V law per
// type is code (device_iv_law(), below).
//
// generate_kcl_equations() leaves nonlinear branch currents as free symbols,
// so solve() gives closed-form node voltages parametrized by them. From
// there: (1) group devices into "clusters", connected components whose
// branch currents appear in each other's junction-voltage expressions, so
// they need one joint Newton solve; (2) build each device's I-V law as an
// Expr in its own junction voltage(s); (3) substitute those laws in for the
// branch-current symbols; (4) build residuals f_j = closed_form_j -
// unknown_j, differentiate for the Jacobian, solve J*delta = -f; (5) run
// cse()/licm()/recip_hoist() with the cluster's unknowns as the variant
// base, hoisting Newton-iteration-invariant subexpressions one loop level in.
//
// This module does not handle any code generation or damping/convergence/limiter
// control flow. Those concerns are handled in netlist_codegen.jai.

#import "Basic";
#import "Hash_Table";
#import "backend";
#import "sym_algebra";
#import "nodal_analysis";

// Which limiter/junction model a Newton unknown uses, and that model's
// Params/State field names. Shared shape between a device's raw unknown
// (Raw_Unknown) and its cluster-assigned counterpart (Cluster_Unknown).
Unknown_Model :: struct {
    limiter_kind: Limiter_Kind;
    vt_name:      string; // valid when limiter_kind == .EXP_JUNCTION; Params field for thermal voltage
    is_name:      string; // valid when limiter_kind == .EXP_JUNCTION; Params field for saturation current
    vp_name:      string; // valid when limiter_kind == .JFET_PINCHOFF; Params field for pinch-off voltage
    vsat_p_name:  string; // valid when limiter_kind == .AMP_CLIP; Params field for positive output rail
    vsat_n_name:  string; // valid when limiter_kind == .AMP_CLIP; Params field for negative output rail
    has_slew:     bool;   // valid when limiter_kind == .AMP_CLIP; true if this op-amp also has slew set
    slew_name:    string; // valid when has_slew; Params field for max |dVout/dt|, V/s
    zslew_name:   string; // valid when has_slew; State field for the previous sample's output
}

// One Newton unknown belonging to a cluster (a device's junction voltage).
Cluster_Unknown :: struct {
    name:          string;       // e.g. "vBCQ1"; also the State field name
    model:         Unknown_Model;
    residual_name: string;       // Gen_Var name holding this unknown's residual f_j
    delta_name:    string;       // Gen_Var name holding this unknown's Newton delta

    // Valid only when this unknown is one of the *other* unknowns in a
    // cluster with an active-set AMP_CLIP unknown - see the comment there.
    // Residual/delta for this unknown with the clip unknown held fixed at
    // the positive/negative rail instead of solved for, i.e. the *correct*
    // equations to use on a Newton iteration where the clip unknown is
    // pinned, rather than the free-system ones.
    residual_name_hi: string;
    delta_name_hi:    string;
    residual_name_lo: string;
    delta_name_lo:    string;
}

// A group of nonlinear devices solved via one joint Newton-Raphson system.
Nonlinear_Cluster :: struct {
    name:     string; // diagnostic name, e.g. "Q1_D1D2" (joined device names)
    unknowns: [..] Cluster_Unknown;

    // Newton-invariant CSE temps (don't depend on this cluster's unknowns),
    // emitted once before the per-sample Newton loop.
    invariant_items: [..] Gen_Var;

    // Newton-variant CSE temps plus the residual/delta Gen_Vars, topologically
    // ordered. Recomputed every Newton iteration.
    iteration_items: [..] Gen_Var;

    // Index into `unknowns` of the single AMP_CLIP unknown handled with
    // proper active-set rail clamping, or -1 if this cluster has none,
    // or more than one (unsupported).
    amp_clip_idx: int = -1;

    // The reduced (unknowns.count - 1)-unknown Newton system with the clip
    // unknown fixed at its positive/negative rail. Same shape as
    // invariant_items/iteration_items above, just for the "clip is pinned" case.
    hi_invariant_items: [..] Gen_Var;
    hi_iteration_items: [..] Gen_Var;
    lo_invariant_items: [..] Gen_Var;
    lo_iteration_items: [..] Gen_Var;

    // N-port (Thevenin) form, populated only under -opt_port_matrix and only
    // for eligible clusters. Everything above stays populated regardless, so
    // the symbolic path is always available.
    port: Port_Form;
}

// The N-port (Thevenin) equivalent of the linear subnetwork seen from one
// cluster's k device ports:
//     v_j  =  voc_j  +  sum_m  z[j*k + m] * i_m
//
// Rows are indexed by Newton unknown, columns by device branch current.
// The identity term in J = Z*G - I is indexed by unknown, and G is indexed
// [port][unknown]. Devices contributing two unknowns and two currents (BJT,
// TRIODE) make G block-diagonal rather than diagonal (see `block`).
Port_Form :: struct {
    valid: bool;         // false: this cluster keeps the symbolic path
    k:     int;          // unknowns == # of ports
    z:     [..] Expr_ID; // k*k row-major; NO_EXPR means structurally zero
    voc:   [..] Expr_ID; // k; NO_EXPR means structurally zero
    block: [..] int;     // k; device index owning port m
    port_names: [..] string; // k; branch-current symbol names, for diagnostics

    // Device laws, in the cluster's own unknowns only.
    iv: [..] Expr_ID;    // k; i_m(v)
    g:  [..] Expr_ID;    // k*k row-major [port m][unknown n]; NO_EXPR off-block

    // Post-CSE emission lists, filled in by build_nonlinear_clusters().
    // Three regions, outermost first:
    //   z_items   - function preamble, once per compute() call
    //   voc_items - per sample, before the Newton loop
    //   invariant_items / iteration_items - device laws, hoisted or per iteration
    z_items:   [..] Gen_Var;
    z_names:   [..] string;        // k*k; "" where Z_jm is structurally zero
    voc_items: [..] Gen_Var;
    voc_names: [..] string;        // k; "" where voc_j is structurally zero
    invariant_items: [..] Gen_Var; // hoisted out of the Newton loop
    iteration_items: [..] Gen_Var; // recomputed per Newton iteration: CSE temps, i_m, G_mn
    iv_names: [..] string;         // k; Gen_Var name holding i_m
    g_names:  [..] string;         // k*k; Gen_Var name holding G_mn, "" where off-block

    // J_jn as a named Gen_Var, only populated when k > PORT_CRAMER_MAX_K: the
    // pivoted-LU text block (emit_port_solve_block) needs J in a named local
    // per entry to read into its array.
    j_names: [..] string;          // k*k; Gen_Var name holding J_jn
}

// Should this cluster use the N-port form or symbolic derivation?
use_port_form :: (cluster: Nonlinear_Cluster, backend: Backend) -> bool {
    return backend.opt_port_matrix && cluster.port.valid;
}

// -----------------------------------------------------------------------
// Device I-V laws
// -----------------------------------------------------------------------

// Single-diode Shockley I-V law: i = Is*(exp(v/vt) - 1).
diode_iv_law :: (junction_voltage_name: string, model: Nonlinear_Model, pool: *Expr_Pool) -> Expr_ID {
    v   := expr_symbol(junction_voltage_name, pool);
    vt  := expr_symbol(model.vt_name, pool);
    is_ := expr_symbol(model.is_name, pool);
    e   := expr_exp(expr_div(v, vt, pool), pool);
    return expr_mul(is_, expr_sub(e, expr_const(1, pool), pool), pool);
}

// Antiparallel diode-pair I-V law: i = Is*(exp(v/vt) - 1/exp(v/vt))
// Could also be written as: i =  2*Is*sinh(v/vt).
diode_pair_iv_law :: (junction_voltage_name: string, model: Nonlinear_Model, pool: *Expr_Pool) -> Expr_ID {
    v     := expr_symbol(junction_voltage_name, pool);
    vt    := expr_symbol(model.vt_name, pool);
    is_   := expr_symbol(model.is_name, pool);
    e     := expr_exp(expr_div(v, vt, pool), pool);
    inv_e := expr_div(expr_const(1, pool), e, pool);
    return expr_mul(is_, expr_sub(e, inv_e, pool), pool);
}

// Ebers-Moll BJT base/collector current I-V laws:
//   iB = Is*((exp(vBE/vt)-1)/BetaF + (exp(vBC/vt)-1)/BetaR)
//   iC = Is*((exp(vBE/vt)-exp(vBC/vt)) - (exp(vBC/vt)-1)/BetaR)
bjt_iv_laws :: (vbc_name: string, vbe_name: string, model: Nonlinear_Model, pool: *Expr_Pool) -> (ib: Expr_ID, ic: Expr_ID) {
    vbc := expr_symbol(vbc_name, pool);
    vbe := expr_symbol(vbe_name, pool);
    vt    := expr_symbol(model.vt_name,    pool);
    is_   := expr_symbol(model.is_name,    pool);
    betaf := expr_symbol(model.betaf_name, pool);
    betar := expr_symbol(model.betar_name, pool);

    exp_be := expr_exp(expr_div(vbe, vt, pool), pool);
    exp_bc := expr_exp(expr_div(vbc, vt, pool), pool);

    ib := expr_mul(is_,
              expr_add(
                  expr_div(expr_sub(exp_be, expr_const(1, pool), pool), betaf, pool),
                  expr_div(expr_sub(exp_bc, expr_const(1, pool), pool), betar, pool),
                  pool),
              pool);

    ic := expr_mul(is_,
              expr_sub(
                  expr_sub(exp_be, exp_bc, pool),
                  expr_div(expr_sub(exp_bc, expr_const(1, pool), pool), betar, pool),
                  pool),
              pool);

    return ib, ic;
}

// SPICE Level-1 JFET drain-current I-V law: iD = Beta*(vGS - Vp)^2.
//
// d(iD)/d(vGS) hits exactly 0 at pinch-off. Harmless solved alone, but if
// this device shares a Newton cluster with another one (see clustering
// above), a zero Jacobian entry there can ill-condition their shared linear
// solve. A 1mV epsilon floor keeps it off exact zero, same idea as gmin.
jfet_iv_law :: (junction_voltage_name: string, model: Nonlinear_Model, pool: *Expr_Pool) -> Expr_ID {
    vgs  := expr_symbol(junction_voltage_name, pool);
    vp   := expr_symbol(model.vp_name, pool);
    beta := expr_symbol(model.beta_name, pool);
    epsilon := expr_div(expr_const(1, pool), expr_const(1000, pool), pool);
    diff := expr_add(expr_sub(vgs, vp, pool), epsilon, pool);
    return expr_mul(beta, expr_mul(diff, diff, pool), pool);
}

// Dempwolf & Zolzer triode model (DAFx-11):
//   Hk = log(1+exp(Ck*(vPK/mu + vGK))) / Ck,   Ik = Gk*Hk^gamma
//   Hg = log(1+exp(Cg*vGK)) / Cg,              Ig = Gg*Hg^xi + Ig0
//   Ia = Ik - Ig  (anode/plate current)
triode_iv_laws :: (vpk_name: string, vgk_name: string, model: Nonlinear_Model, pool: *Expr_Pool) -> (ip: Expr_ID, ig: Expr_ID) {
    vpk := expr_symbol(vpk_name, pool);
    vgk := expr_symbol(vgk_name, pool);
    gk    := expr_symbol(model.gk_name,    pool);
    mu    := expr_symbol(model.mu_name,    pool);
    gamma := expr_symbol(model.gamma_name, pool);
    ck    := expr_symbol(model.ck_name,    pool);
    gg    := expr_symbol(model.gg_name,    pool);
    xi    := expr_symbol(model.xi_name,    pool);
    cg    := expr_symbol(model.cg_name,    pool);
    ig0   := expr_symbol(model.ig0_name,   pool);

    x_k := expr_add(expr_div(vpk, mu, pool), vgk, pool);
    hk  := expr_div(expr_log(expr_add(expr_const(1, pool), expr_exp(expr_mul(ck, x_k, pool), pool), pool), pool), ck, pool);
    ik  := expr_mul(gk, expr_pow(hk, gamma, pool), pool);

    hg  := expr_div(expr_log(expr_add(expr_const(1, pool), expr_exp(expr_mul(cg, vgk, pool), pool), pool), pool), cg, pool);
    ig  := expr_add(expr_mul(gg, expr_pow(hg, xi, pool), pool), ig0, pool);

    ip := expr_sub(ik, ig, pool);
    return ip, ig;
}

// One device current, in terms of its junction-voltage unknown(s): the
// branch-current symbol `i<suffix><el.name>` (see Device_Kcl_Term) gets
// substituted with `expr`.
Device_Branch_Current :: struct {
    suffix: string;
    expr:   Expr_ID;
}

// Dispatches to the right I-V law function above for `el.type`. This is the
// one place that needs a new case when adding a device type; everything
// else in this file is driven generically by nonlinear_device_spec().
device_iv_law :: (el: Element, unknown_names: [] string, pool: *Expr_Pool) -> [] Device_Branch_Current {
    result: [..] Device_Branch_Current;
    if el.type == {
        case .DIODE;
            iv := diode_iv_law(unknown_names[0], el.nonlinear, pool);
            array_add(*result, .{ suffix = "", expr = iv });
        case .DIODE_PAIR;
            iv := diode_pair_iv_law(unknown_names[0], el.nonlinear, pool);
            array_add(*result, .{ suffix = "", expr = iv });
        case .NPN;
            vbc_name := ""; vbe_name := "";
            for n : unknown_names {
                if String.begins_with(n, "vBC")  vbc_name = n;
                if String.begins_with(n, "vBE")  vbe_name = n;
            }
            ib, ic := bjt_iv_laws(vbc_name, vbe_name, el.nonlinear, pool);
            array_add(*result, .{ suffix = "B", expr = ib });
            array_add(*result, .{ suffix = "C", expr = ic });
        case .PNP;
            vcb_name := ""; veb_name := "";
            for n : unknown_names {
                if String.begins_with(n, "vCB")  vcb_name = n;
                if String.begins_with(n, "vEB")  veb_name = n;
            }
            ib, ic := bjt_iv_laws(vcb_name, veb_name, el.nonlinear, pool);
            array_add(*result, .{ suffix = "B", expr = ib });
            array_add(*result, .{ suffix = "C", expr = ic });
        case .NJF;
            iv := jfet_iv_law(unknown_names[0], el.nonlinear, pool);
            array_add(*result, .{ suffix = "", expr = iv });
        case .PJF;
            iv := jfet_iv_law(unknown_names[0], el.nonlinear, pool);
            array_add(*result, .{ suffix = "", expr = iv });
        case .TRIODE;
            vpk_name := ""; vgk_name := "";
            for n : unknown_names {
                if String.begins_with(n, "vPK")  vpk_name = n;
                if String.begins_with(n, "vGK")  vgk_name = n;
            }
            ip, ig := triode_iv_laws(vpk_name, vgk_name, el.nonlinear, pool);
            array_add(*result, .{ suffix = "P", expr = ip });
            array_add(*result, .{ suffix = "G", expr = ig });
    }
    return result;
}

// Branch-current symbol name(s) this device contributes as free symbols in
// the linear KCL system (see nodal_analysis.generate_kcl_equations). An
// AMP's clip/slew unknown isn't a branch current. It's the driving voltage
// itself, used directly by Rout/out_node's stamp (see opamp_clip_node), but
// it plays the same "free symbol used elsewhere in the linear system"
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
    model:        Unknown_Model;
    linear_form:  Expr_ID; // closed-form node-voltage-difference; still references raw branch-current symbols
    device_idx:   int;     // index into `devices`
}

// -----------------------------------------------------------------------
// AMP_CLIP active-set reduced system
// -----------------------------------------------------------------------

// Deep-copies `e`, replacing every leaf reference to the symbol named
// `old_name` with `replacement`.
copy_expr_replacing_symbol :: (e: Expr_ID, old_name: string, replacement: Expr_ID, pool: *Expr_Pool) -> Expr_ID {
    ex := pool.exprs[e];
    if ex.kind == .SYMBOL  return ifx ex.name == old_name then replacement else e;
    if ex.kind == .NULL || ex.kind == .CONST  return copy_expr(e, pool);
    if is_unary(ex.kind)  return make_unary(ex.kind, copy_expr_replacing_symbol(ex.left, old_name, replacement, pool), pool);
    return expr_bin(ex.kind, copy_expr_replacing_symbol(ex.left, old_name, replacement, pool),
                              copy_expr_replacing_symbol(ex.right, old_name, replacement, pool), pool);
}

// Builds the reduced (k-1)-unknown Newton system for an AMP_CLIP cluster
// with the clip unknown (cluster_unknown_names[clip_idx]) held fixed at
// one rail (rail_param_name) instead of solved for jointly.
//
// A post-step rail clamp is fine when the clip unknown is the only
// nonlinearity around, but wrong once it's jointly solved with another
// device: the other unknowns' closed forms would keep assuming the clip
// unknown is free even on iterations where it's actually pinned. So we
// substitute the clip symbol for a constant *before* differentiating,
// giving the equations that are actually true with that rail fixed, rather
// than clamping the free-system solution after the fact.
//
// Mirrors build_nonlinear_clusters()'s free-system derivation with one
// fewer unknown/equation. Must be called with `f_ids` in pristine, pre-CSE
// form, since CSE rewrites them in place to reference temp symbols local to
// the free system's own generated code, which this copy would otherwise
// wrongly inherit as dangling references.
derive_amp_clip_reduced_system :: (
        f_ids: [] Expr_ID, cluster_unknown_names: [] string, clip_idx: int,
        rail_param_name: string, pool: *Expr_Pool, temp_prefix: string)
        -> (other_names: [..] string, res_names: [..] string, delta_names: [..] string,
            invariant_items: [..] Gen_Var, iteration_items: [..] Gen_Var) {
    clip_name := cluster_unknown_names[clip_idx];
    rail_symbol := expr_symbol(rail_param_name, pool);

    other_names: [..] string;
    f_reduced: [..] Expr_ID;
    for name, i : cluster_unknown_names {
        if i == clip_idx  continue;
        array_add(*other_names, name);
        f := copy_expr_replacing_symbol(f_ids[i], clip_name, rail_symbol, pool);
        array_add(*f_reduced, simplify(f, pool));
    }

    k := other_names.count;
    jacobian: [..] Expr_ID;
    array_reserve(*jacobian, k * k);
    for j : 0..k-1 {
        for m : 0..k-1 {
            unknown_m := expr_symbol(other_names[m], pool);
            dj := differentiate(f_reduced[j], unknown_m, pool);
            array_add(*jacobian, simplify(dj, pool));
        }
    }

    delta_ids: [..] Expr_ID;
    for name : other_names  array_add(*delta_ids, expr_symbol(tprint("delta_%1_%2", name, temp_prefix), pool));

    delta_equations: [..] Expr_ID;
    for j : 0..k-1 {
        eq := f_reduced[j];
        for m : 0..k-1  eq = expr_add(eq, expr_mul(jacobian[j*k + m], delta_ids[m], pool), pool);
        array_add(*delta_equations, eq);
    }
    delta_solutions: [..] Expr_ID;
    if k > 0  delta_solutions = solve(delta_equations, delta_ids, pool);

    res_names: [..] string;
    delta_names: [..] string;
    gen_vars: [..] Gen_Var;
    for name, j : other_names {
        res_name := tprint("res_%1_%2", name, temp_prefix);
        delta_name := tprint("delta_%1_%2", name, temp_prefix);
        array_add(*res_names, res_name);
        array_add(*delta_names, delta_name);
        array_add(*gen_vars, Gen_Var.{ name = res_name, expr_id = f_reduced[j] });
        array_add(*gen_vars, Gen_Var.{ name = delta_name, expr_id = delta_solutions[j] });
    }

    invariant_items: [..] Gen_Var;
    iteration_items: [..] Gen_Var;
    if gen_vars.count > 0 {
        gen_var_ids: [..] Expr_ID;
        for gv : gen_vars  array_add(*gen_var_ids, gv.expr_id);

        cse_vars := cse(gen_var_ids, pool, temp_prefix);
        sort_cse_topological(*cse_vars, pool.exprs);
        licm(gen_var_ids, *cse_vars, pool, other_names, temp_prefix);
        recip_hoist(gen_var_ids, *cse_vars, pool, other_names, temp_prefix);

        variant_cse_names: [..] string;
        for cv : cse_vars {
            if expr_is_loop_variant(cv.expr_id, pool.exprs, other_names, variant_cse_names) {
                array_add(*variant_cse_names, cv.name);
                array_add(*iteration_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
            } else {
                array_add(*invariant_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
            }
        }
        for gv : gen_vars  array_add(*iteration_items, gv);
    }

    return other_names, res_names, delta_names, invariant_items, iteration_items;
}

// -----------------------------------------------------------------------
// Top-level entry point.
// -----------------------------------------------------------------------
//
// Call after generate_kcl_equations()/solve() have produced solve_vars/
// solutions over `pool`, and after reduce_circuit()'s DIODE_PAIR pass.
//
// Side effect: every nonlinear branch-current symbol (i<name>, iB<name>/
// iC<name>) gets substituted with its device's I-V law, in terms of the
// junction-voltage unknowns.
// N-port (Thevenin) extraction for one cluster. Decomposes each unknown's
// closed-form junction voltage into voc_j + sum_m Z_jm*i_m by calling
// linearize() against the cluster's branch-current symbols.
//
// ORDERING CONSTRAINT: this must run *before* build_nonlinear_clusters()
// substitutes the I-V laws in for the branch-current symbols. substitute()
// rewrites every matching SYMBOL node in the pool, so afterwards there are
// no port symbols left for linearize() to decompose.
//
// Returns an invalid Port_Form for clusters this doesn't handle, in which
// case the caller keeps the symbolic path for that cluster.
extract_port_form :: (members: [] int,
                      devices: [] Element,
                      raw_unknowns: [] Raw_Unknown,
                      port_ids_by_device: [] [..] Expr_ID,
                      port_syms_by_device: [] [..] Expr_ID,
                      port_names_by_device: [] [..] string,
                      pool: *Expr_Pool) -> Port_Form {
    result: Port_Form;

    // Port form doesn't support op-amp clip stages (for now)
    for d_idx : members {
        if devices[d_idx].type == .AMP  return result;
    }

    // Ports and unknowns, both in ascending device order. root_members and
    // raw_unknowns are filled in the same order and share the same indices.
    unknown_syms: [..] Expr_ID;
    members_set: Table(int, bool);
    for m : members  table_add(*members_set, m, true);
    for ru : raw_unknowns {
        if !table_contains(*members_set, ru.device_idx)  continue;
        array_add(*unknown_syms, expr_symbol(ru.name, pool));
    }

    port_syms: [..] Expr_ID;
    for d_idx : members {
        for ps, i : port_syms_by_device[d_idx] {
            array_add(*port_syms, ps);
            array_add(*result.block, d_idx);
            array_add(*result.port_names, port_names_by_device[d_idx][i]);
            array_add(*result.iv, copy_expr(port_ids_by_device[d_idx][i], pool));
        }
    }

    // Double-check that the system is square...
    if port_syms.count != unknown_syms.count  return result;

    k := unknown_syms.count;
    result.k = k;
    array_reserve(*result.z, k * k);

    // One branch current can have several SYMBOL nodes in the pool, so match
    // on name (as the substitution loop does) rather than on the node ID that
    // linearize() compares, and fold every alias into one column. Missing one
    // is silent: linearize() would treat it as a constant and fold a device
    // current into voc.
    var_ids:  [..] Expr_ID;
    var_port: [..] int; // parallel to var_ids: which port each alias belongs to
    pool_count := pool.exprs.count;
    for node_id : 0..pool_count-1 {
        ex := pool.exprs[node_id];
        if ex.kind != .SYMBOL  continue;
        for pn, m : result.port_names {
            if ex.name != pn  continue;
            array_add(*var_ids, node_id);
            array_add(*var_port, m);
            break;
        }
    }

    for ru : raw_unknowns {
        if !table_contains(*members_set, ru.device_idx)  continue;

        form := copy_expr(ru.linear_form, pool);
        simplify(form, pool);
        lin := linearize(form, var_ids, pool);

        array_add(*result.voc, lin.constant);
        row_start := result.z.count;
        for m : 0..k-1  array_add(*result.z, NO_EXPR);
        for c, v : lin.coeffs {
            if c == NO_EXPR  continue;
            slot := row_start + var_port[v];
            result.z[slot] = combine(result.z[slot], c, .ADD, pool);
        }
    }

    // Device laws' derivatives.
    for m : 0..k-1 {
        for n : 0..k-1 {
            // G is block-diagonal: i_m depends only on its own device's
            // unknowns, so everything else is zero and never gets emitted.
            if result.block[m] != result.block[n] { array_add(*result.g, NO_EXPR); continue; }
            dg := differentiate(result.iv[m], unknown_syms[n], pool);
            simplify(dg, pool);
            array_add(*result.g, dg);
        }
    }

    // Z never references port currents or unknowns, so it's constant for the whole compute().
    for v : result.voc {
        if v == NO_EXPR  continue;
        names: [..] string;
        collect_symbol_names(v, pool.exprs, *names);
        for n : names  for pn : result.port_names  assert(n != pn, "Port voc references a branch current (%)!", n);
    }
    for z : result.z {
        if z == NO_EXPR  continue;
        names: [..] string;
        collect_symbol_names(z, pool.exprs, *names);
        for n : names {
            for pn : result.port_names  assert(n != pn, "Port matrix Z references a branch current (%)!", n);
            for us : unknown_syms  assert(n != pool.exprs[us].name, "Port matrix Z references a Newton unknown (%)!", n);
        }
    }

    result.valid = true;
    return result;
}

// Above this cluster size, Cramer's factorial op count outweighs its shorter
// dependency chain and the pivoted elimination path is used instead (see
// emit_unrolled_lu_solve / emit_port_solve_block in codegen.jai). At or below
// it, J*delta = -F goes through the same solve() the symbolic Newton path
// already uses for its own delta system regardless of size.
// @TODO: eventually we should do something more dynamic using our knowledge
// of the target CPU.
PORT_CRAMER_MAX_K :: 3;

// Names one k-long array of Port_Form Expr_IDs (i_m, voc_j) into Gen_Vars
// "_<cluster>_<tag><i>", skipping NO_EXPR entries (recorded as "" in `names`).
name_port_exprs_1d :: (ids: [] Expr_ID, cluster_name: string, tag: string, names: *[..] string, gvs: *[..] Gen_Var) {
    for e, i : ids {
        if e == NO_EXPR { array_add(names, ""); continue; }
        name := tprint("_%1_%2%3", cluster_name, tag, i);
        array_add(names, name);
        array_add(gvs, Gen_Var.{ name = name, expr_id = e });
    }
}

// Same, for a k*k row-major array (Z_jm, G_mn, J_jn): "_<cluster>_<tag><row>_<col>".
name_port_exprs_2d :: (ids: [] Expr_ID, k: int, cluster_name: string, tag: string, names: *[..] string, gvs: *[..] Gen_Var) {
    for row : 0..k-1 {
        for col : 0..k-1 {
            e := ids[row*k + col];
            if e == NO_EXPR { array_add(names, ""); continue; }
            name := tprint("_%1_%2%3_%4", cluster_name, tag, row, col);
            array_add(names, name);
            array_add(gvs, Gen_Var.{ name = name, expr_id = e });
        }
    }
}

// F_j = voc_j + sum_m Z_jm*i_m - v_j, as an Expr tree over the Z/voc/iv symbols by name.
port_residual_expr :: (pf: *Port_Form, j: int, unknown_name: string, pool: *Expr_Pool) -> Expr_ID {
    k := pf.k;
    result: Expr_ID;
    have := false;
    if pf.voc_names[j].count > 0 { result = expr_symbol(pf.voc_names[j], pool); have = true; }
    for m : 0..k-1 {
        if pf.z_names[j*k + m].count == 0  continue;
        term := expr_mul(expr_symbol(pf.z_names[j*k + m], pool), expr_symbol(pf.iv_names[m], pool), pool);
        if have  result = expr_add(result, term, pool);
        else   { result = term; have = true; }
    }
    if !have  result = expr_const(0, pool);
    return expr_sub(result, expr_symbol(unknown_name, pool), pool);
}

// J_jn = sum_m Z_jm*G_mn - (j == n), as an Expr tree.
port_jacobian_expr :: (pf: *Port_Form, j: int, n: int, pool: *Expr_Pool) -> Expr_ID {
    k := pf.k;
    result: Expr_ID;
    have := false;
    for m : 0..k-1 {
        if pf.z_names[j*k + m].count == 0  continue;
        if pf.g_names[m*k + n].count == 0  continue;
        term := expr_mul(expr_symbol(pf.z_names[j*k + m], pool), expr_symbol(pf.g_names[m*k + n], pool), pool);
        if have  result = expr_add(result, term, pool);
        else   { result = term; have = true; }
    }
    // For off-diagonals that are actually empty, we emit NO_EXPR.
    if !have {
        if j == n  return expr_const(-1, pool);
        return NO_EXPR;
    }
    if j == n  result = expr_sub(result, expr_const(1, pool), pool);
    return result;
}

build_nonlinear_clusters :: (elements: [] Element,
                             solve_vars: [] Expr_ID,
                             solutions: [] Expr_ID,
                             pool: *Expr_Pool,
                             backend: Backend = .{}) -> [] Nonlinear_Cluster {
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
            pos_v := copy_expr(lookup_node_voltage(el.terminals[u.pos_terminal], solve_vars, solutions, pool), pool);
            neg_v := copy_expr(lookup_node_voltage(el.terminals[u.neg_terminal], solve_vars, solutions, pool), pool);
            v := simplify(expr_sub(pos_v, neg_v, pool), pool);
            vt_name, is_name, vp_name := device_limiter_names(u.limiter, el.nonlinear);
            model := Unknown_Model.{ limiter_kind = u.limiter, vt_name = vt_name, is_name = is_name, vp_name = vp_name };
            array_add(*raw_unknowns, .{ name = tprint("%1%2", u.name_prefix, el.name), model = model,
                                         linear_form = v, device_idx = device_idx });
        }
    }

    // Op-amp output clip/slew stage. Not in nonlinear_device_spec(): unlike
    // DIODE/DIODE_PAIR/BJT/JFET, whether an AMP is nonlinear depends on its
    // instance-level opamp fields, not just its Element_Type (see
    // opamp_has_clip_stage). Its unknown is a single node's closed-form
    // voltage (opamp_bandwidth_node's solved value, i.e. the ideal/band-
    // limited gain-stage output before clipping), not a terminal-pair
    // difference, and it needs no I-V law substitution. Its free symbol
    // (vclip_<name>, see branch_current_names) *is* the Newton unknown, so
    // there's nothing to substitute the way a diode's current->voltage
    // hand-off needs.
    for el : elements {
        if el.type != .AMP || !opamp_has_clip_stage(el)  continue;

        device_idx := devices.count;
        array_add(*devices, el);
        v := copy_expr(lookup_node_voltage(opamp_bandwidth_node(el), solve_vars, solutions, pool), pool);
        simplify(v, pool);
        model := Unknown_Model.{ limiter_kind = .AMP_CLIP,
                                  vsat_p_name = el.opamp.vsat_p_name, vsat_n_name = el.opamp.vsat_n_name,
                                  has_slew = el.opamp.has_slew, slew_name = el.opamp.slew_name,
                                  zslew_name = tprint("zslew_%", el.name) };
        array_add(*raw_unknowns, .{ name = tprint("vclip_%", el.name), model = model,
                                     linear_form = v, device_idx = device_idx });
    }

    if devices.count == 0 {
        empty: [] Nonlinear_Cluster;
        return empty;
    }

    // device_idx -> indices into raw_unknowns, CSR-style: device d's indices
    // are device_to_ru_flat[device_to_ru_offsets[d] .. device_to_ru_offsets[d+1]-1].
    device_to_ru_counts: [..] int;
    array_resize(*device_to_ru_counts, devices.count);
    for ru : raw_unknowns  device_to_ru_counts[ru.device_idx] += 1;

    device_to_ru_offsets: [..] int;
    array_resize(*device_to_ru_offsets, devices.count + 1);
    for d : 0..devices.count-1  device_to_ru_offsets[d + 1] = device_to_ru_offsets[d] + device_to_ru_counts[d];

    device_to_ru_flat: [..] int;
    array_resize(*device_to_ru_flat, raw_unknowns.count);
    cursor := array_copy(device_to_ru_offsets);
    for ru, i : raw_unknowns {
        device_to_ru_flat[cursor[ru.device_idx]] = i;
        cursor[ru.device_idx] += 1;
    }

    device_to_ru :: (flat: [] int, offsets: [] int, d: int) -> [] int {
        return array_view(flat, offsets[d], offsets[d + 1] - offsets[d]);
    }

    // name -> index into raw_unknowns.
    raw_unknown_by_name: Table(string, int);
    for ru, i : raw_unknowns  table_set(*raw_unknown_by_name, ru.name, i);

    // Union devices whose branch-current symbols appear in each other's
    // linear_form (checked both directions; see module comment).
    device_refs: [..] Table(string, bool);
    for d : 0..devices.count-1 {
        refs_set: Table(string, bool);
        for ru_i : device_to_ru(device_to_ru_flat, device_to_ru_offsets, d) {
            refs: [..] string;
            collect_symbol_names(raw_unknowns[ru_i].linear_form, pool.exprs, *refs);
            for n : refs  if !table_contains(*refs_set, n)  table_add(*refs_set, n, true);
        }
        array_add(*device_refs, refs_set);
    }

    uf := uf_make(devices.count);
    for a : 0..devices.count-1 {
        a_names := branch_current_names(devices[a]);
        for b : a+1..devices.count-1 {
            b_names := branch_current_names(devices[b]);
            coupled := false;
            for n : b_names  if table_contains(*device_refs[a], n)  coupled = true;
            for n : a_names  if table_contains(*device_refs[b], n)  coupled = true;

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
    // Same data grouped per device (for the N-port extraction).
    port_ids_by_device:   [..] [..] Expr_ID;
    port_syms_by_device:  [..] [..] Expr_ID;
    port_names_by_device: [..] [..] string;
    array_resize(*port_ids_by_device,   devices.count);
    array_resize(*port_syms_by_device,  devices.count);
    array_resize(*port_names_by_device, devices.count);
    for d, idx : devices {
        unknown_names: [..] string;
        for ru_i : device_to_ru(device_to_ru_flat, device_to_ru_offsets, idx)  array_add(*unknown_names, raw_unknowns[ru_i].name);

        for bc : device_iv_law(d, unknown_names, pool) {
            name := tprint("i%1%2", bc.suffix, d.name);
            array_add(*iv_law_ids,   bc.expr);
            array_add(*iv_law_names, name);
            array_add(*port_ids_by_device[idx],   bc.expr);
            array_add(*port_syms_by_device[idx],  expr_symbol(name, pool));
            array_add(*port_names_by_device[idx], name);
        }
    }

    // N-port extraction, per cluster. Must run here, before the substitution
    // loop below, which erases the branch-current symbols from the pool.
    port_forms: [..] Port_Form; // parallel to root_members
    array_resize(*port_forms, root_members.count);
    if backend.opt_port_matrix {
        for members, ci : root_members {
            port_forms[ci] = extract_port_form(members, devices, raw_unknowns, port_ids_by_device,
                                               port_syms_by_device, port_names_by_device, pool);

            pf := port_forms[ci];
            name_parts: [..] string;
            for d_idx : members  array_add(*name_parts, devices[d_idx].name);
            cname := String.join(..name_parts, separator = "_");
            if !pf.valid {
                log("  Cluster \"%\" N-port: not eligible, keeping the symbolic path\n", cname);
                continue;
            }
            nz := 0;
            pattern: String_Builder;
            for j : 0..pf.k-1 {
                if j > 0  append(*pattern, "/");
                for m : 0..pf.k-1 {
                    if pf.z[j*pf.k + m] == NO_EXPR { append(*pattern, "."); continue; }
                    nz += 1;
                    append(*pattern, "X");
                }
            }
            // Z entries are ratios of MNA cofactors and could be large. A wild
            // number here is the early warning that Z should be computed numerically.
            z_size := 0;
            for z : pf.z  if z != NO_EXPR  z_size += cse_expr_size(z, pool.exprs);
            voc_size := 0;
            for v : pf.voc  if v != NO_EXPR  voc_size += cse_expr_size(v, pool.exprs);
            log("  Cluster \"%1\" N-port: k=%2, Z nonzeros %3/%4 [%5], tree size: Z %6, voc %7\n",
                cname, pf.k, nz, pf.k * pf.k, builder_to_string(*pattern), z_size, voc_size);
        }
    }

    // Substitute every branch-current symbol with its I-V law, in place.
    for id, i : iv_law_ids {
        name := iv_law_names[i];
        matches: [..] Expr_ID;
        for ex, node_id : pool.exprs  if ex.kind == .SYMBOL && ex.name == name  array_add(*matches, node_id);
        for m : matches  substitute(m, id, pool);
    }

    // Build one Nonlinear_Cluster per connected component.
    clusters: [..] Nonlinear_Cluster;
    for members, ci : root_members {
        cluster: Nonlinear_Cluster;

        name_parts: [..] string;
        for d_idx : members  array_add(*name_parts, devices[d_idx].name);
        cluster.name = String.join(..name_parts, separator = "_");
        cluster.port = port_forms[ci];

        // Collect this cluster's unknowns (all raw_unknowns whose device is a member).
        members_set: Table(int, bool);
        for m : members  table_add(*members_set, m, true);
        cluster_unknown_names: [..] string;
        for ru : raw_unknowns {
            if !table_contains(*members_set, ru.device_idx)  continue;
            array_add(*cluster_unknown_names, ru.name);
        }

        // Don'tdo symbolic derivation if using N-port form.
        derive_symbolic := !(backend.opt_port_matrix && cluster.port.valid);
        f_ids:     [..] Expr_ID; // parallel to cluster_unknown_names
        if derive_symbolic  {
            for name : cluster_unknown_names {
                _, ru_idx := table_find(*raw_unknown_by_name, name);
                closed_form := copy_expr(raw_unknowns[ru_idx].linear_form, pool);
                simplify(closed_form, pool);
                unknown_symbol := expr_symbol(name, pool);
                f := simplify(expr_sub(closed_form, unknown_symbol, pool), pool);
                array_add(*f_ids, f);
            }
        }

        k := cluster_unknown_names.count;

        // Jacobian: J[j][m] = d(f_j)/d(unknown_m)
        delta_solutions: [] Expr_ID;
        if derive_symbolic {
        jacobian: [..] Expr_ID; // row-major, k*k
        array_reserve(*jacobian, k * k);
        for j : 0..k-1 {
            for m : 0..k-1 {
                unknown_m := expr_symbol(cluster_unknown_names[m], pool);
                dj := differentiate(f_ids[j], unknown_m, pool);
                simplify(dj, pool);
                array_add(*jacobian, dj);
            }
        }

        // Newton delta solve: J*delta = -f, i.e. f_j + sum_m(J[j][m]*delta_m) = 0.
        delta_ids: [..] Expr_ID;
        for name : cluster_unknown_names  array_add(*delta_ids, expr_symbol(tprint("delta_%", name), pool));

        delta_equations: [..] Expr_ID;
        for j : 0..k-1 {
            eq := f_ids[j];
            for m : 0..k-1 {
                eq = expr_add(eq, expr_mul(jacobian[j*k + m], delta_ids[m], pool), pool);
            }
            array_add(*delta_equations, eq);
        }
        delta_solutions = solve(delta_equations, delta_ids, pool);
        }

        // Named Gen_Vars: residual (for the convergence check) and delta
        // (the Newton update) per unknown.
        // The residual/delta *names* are shared by both paths (the N-port
        // emitter declares locals with these names), so cluster.unknowns is
        // populated either way; only the Gen_Vars backing them are symbolic.
        gen_vars: [..] Gen_Var;
        for name, j : cluster_unknown_names {
            residual_name := tprint("res_%", name);
            delta_name    := tprint("delta_%", name);
            if derive_symbolic {
                array_add(*gen_vars, Gen_Var.{ name = residual_name, expr_id = f_ids[j] });
                array_add(*gen_vars, Gen_Var.{ name = delta_name,    expr_id = delta_solutions[j] });
            }

            _, name_ru_idx := table_find(*raw_unknown_by_name, name);
            ru := raw_unknowns[name_ru_idx];
            array_add(*cluster.unknowns, Cluster_Unknown.{
                name = name, model = ru.model,
                residual_name = residual_name, delta_name = delta_name });
        }

        // Cluster-unique temp prefix, since this Newton-loop code shares a
        // C++ scope with the linear pipeline's own temps (and other clusters').
        temp_prefix := tprint("_%1_t", cluster.name);

        // Active-set handling for AMP_CLIP's rail clamp. Must run
        // before the free system's own cse() call below, since it needs
        // f_ids in their pre-CSE form. Only handles exactly one AMP_CLIP
        // unknown per cluster; 0 or 2+ keep today's plain post-step clamp.
        amp_clip_idx := -1;
        amp_clip_count := 0;
        for u, i : cluster.unknowns {
            if u.model.limiter_kind != .AMP_CLIP  continue;
            amp_clip_count += 1;
            amp_clip_idx = i;
        }
        if amp_clip_count == 1 {
            cluster.amp_clip_idx = amp_clip_idx;
            clip_unknown := cluster.unknowns[amp_clip_idx];

            hi_other, hi_res, hi_delta, hi_inv, hi_iter := derive_amp_clip_reduced_system(
                f_ids, cluster_unknown_names, amp_clip_idx, clip_unknown.model.vsat_p_name, pool, tprint("%1_hi", temp_prefix));
            lo_other, lo_res, lo_delta, lo_inv, lo_iter := derive_amp_clip_reduced_system(
                f_ids, cluster_unknown_names, amp_clip_idx, clip_unknown.model.vsat_n_name, pool, tprint("%1_lo", temp_prefix));

            for name, i : hi_other {
                for *cu : cluster.unknowns {
                    if cu.name != name  continue;
                    cu.residual_name_hi = hi_res[i];
                    cu.delta_name_hi = hi_delta[i];
                }
            }
            for name, i : lo_other {
                for *cu : cluster.unknowns {
                    if cu.name != name  continue;
                    cu.residual_name_lo = lo_res[i];
                    cu.delta_name_lo = lo_delta[i];
                }
            }

            cluster.hi_invariant_items = hi_inv;
            cluster.hi_iteration_items = hi_iter;
            cluster.lo_invariant_items = lo_inv;
            cluster.lo_iteration_items = lo_iter;
        }

        // N-port form's device laws, over a smaller expression set: i_m and G_mn
        // reference only this cluster's own unknowns and model params. A BJT's iB
        // and iC share both exp() terms, and a triode's iP and iG share the whole
        // cathode-current subtree, so CSE matters here even though the expressions are small.
        if cluster.port.valid {
            pf := *cluster.port;
            port_gen_vars: [..] Gen_Var;
            name_port_exprs_1d(pf.iv, cluster.name, "i", *pf.iv_names, *port_gen_vars);
            name_port_exprs_2d(pf.g, pf.k, cluster.name, "g", *pf.g_names, *port_gen_vars);

            // Z and voc must be CSE'd in ONE pass. cse() rewrites pool nodes in
            // place, so two separate passes let the second rewrite subtrees the
            // first has already captured. They share a lot: both are cofactor ratios
            // over the same MNA determinant.
            z_gen_vars:   [..] Gen_Var;
            voc_gen_vars: [..] Gen_Var;
            name_port_exprs_2d(pf.z, pf.k, cluster.name, "Z", *pf.z_names, *z_gen_vars);
            name_port_exprs_1d(pf.voc, cluster.name, "voc", *pf.voc_names, *voc_gen_vars);

            zv_ids: [..] Expr_ID;
            for gv : z_gen_vars    array_add(*zv_ids, gv.expr_id);
            for gv : voc_gen_vars  array_add(*zv_ids, gv.expr_id);
            zv_prefix := tprint("_%1_zt", cluster.name);
            zv_cse := cse(zv_ids, pool, zv_prefix);
            sort_cse_topological(*zv_cse, pool.exprs);

            // Split the shared temps by reachability from Z.
            z_seed_names: [..] string;
            z_seed_names.allocator = temp;
            for gv : z_gen_vars  collect_symbol_names(gv.expr_id, pool.exprs, *z_seed_names);
            in_z_items := collect_region_closure(z_seed_names, zv_cse, pool.exprs);

            in_z: Table(string, bool);
            for cv : in_z_items  table_add(*in_z, cv.name, true);

            for cv : zv_cse {
                if table_contains(*in_z, cv.name)  array_add(*pf.z_items,   Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
                else                               array_add(*pf.voc_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
            }
            for gv : z_gen_vars    array_add(*pf.z_items,   gv);
            for gv : voc_gen_vars  array_add(*pf.voc_items, gv);

            // F_j (residual) and J_jn (Jacobian), as Expr trees over the z/voc/iv/g names.
            F_ids: [..] Expr_ID; // k, in cluster.unknowns order
            for j : 0..pf.k-1  array_add(*F_ids, port_residual_expr(pf, j, cluster.unknowns[j].name, pool));
            for j : 0..pf.k-1  array_add(*port_gen_vars, Gen_Var.{ name = cluster.unknowns[j].residual_name, expr_id = F_ids[j] });

            J_ids: [..] Expr_ID; // k*k
            for j : 0..pf.k-1  for n : 0..pf.k-1  array_add(*J_ids, port_jacobian_expr(pf, j, n, pool));

            if pf.k <= PORT_CRAMER_MAX_K {
                // Same delta_equations[j] = F_j + sum_n J_jn*delta_n shape the
                // symbolic path builds for its own Newton delta system; solve()
                // does the Cramer's-rule solve, with its memoized cofactor cache.
                port_delta_ids: [..] Expr_ID;
                for j : 0..pf.k-1  array_add(*port_delta_ids, expr_symbol(cluster.unknowns[j].delta_name, pool));
                delta_equations: [..] Expr_ID;
                for j : 0..pf.k-1 {
                    eq := F_ids[j];
                    for n : 0..pf.k-1 {
                        // Emit NO_EXPR for structural zero values.
                        if J_ids[j*pf.k + n] == NO_EXPR  continue;
                        eq = expr_add(eq, expr_mul(J_ids[j*pf.k + n], port_delta_ids[n], pool), pool);
                    }
                    array_add(*delta_equations, eq);
                }
                deltas := solve(delta_equations, port_delta_ids, pool);
                for j : 0..pf.k-1  array_add(*port_gen_vars, Gen_Var.{ name = cluster.unknowns[j].delta_name, expr_id = deltas[j] });
            } else {
                // k > PORT_CRAMER_MAX_K: partial pivoting is control flow that
                // Expr trees can't represent, so the solve itself stays a
                // hand-emitted pivoted-LU text block. That block needs J in a
                // named local per entry to read into its array; delta comes out
                // of the LU solve, not from here.
                name_port_exprs_2d(J_ids, pf.k, cluster.name, "J", *pf.j_names, *port_gen_vars);
            }

            port_ids: [..] Expr_ID;
            for gv : port_gen_vars  array_add(*port_ids, gv.expr_id);

            // F and J reference i_m/G_mn by their already-named symbols, not by
            // re-deriving them, so those names must count as loop-variant too.
            port_variant_base: [..] string;
            for n : cluster_unknown_names  array_add(*port_variant_base, n);
            for n : pf.iv_names  if n.count > 0  array_add(*port_variant_base, n);
            for n : pf.g_names   if n.count > 0  array_add(*port_variant_base, n);

            port_prefix := tprint("_%1_pt", cluster.name);
            port_cse := cse(port_ids, pool, port_prefix);
            sort_cse_topological(*port_cse, pool.exprs);
            licm(port_ids, *port_cse, pool, port_variant_base, port_prefix);
            recip_hoist(port_ids, *port_cse, pool, port_variant_base, port_prefix);

            port_variant_names: [..] string;
            for cv : port_cse {
                if expr_is_loop_variant(cv.expr_id, pool.exprs, port_variant_base, port_variant_names) {
                    array_add(*port_variant_names, cv.name);
                    array_add(*pf.iteration_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
                } else {
                    array_add(*pf.invariant_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
                }
            }
            for gv : port_gen_vars  array_add(*pf.iteration_items, gv);
        }

        // Same CSE/LICM/recip_hoist recipe as solve_and_codegen(), but the
        // loop-variant base is this cluster's own unknowns, one loop level
        // in from the per-sample base, so it hoists Newton-invariant
        // subexpressions automatically.
        gen_var_ids: [..] Expr_ID;
        for gv : gen_vars  array_add(*gen_var_ids, gv.expr_id);

        cse_vars := cse(gen_var_ids, pool, temp_prefix);
        sort_cse_topological(*cse_vars, pool.exprs);
        licm(gen_var_ids, *cse_vars, pool, cluster_unknown_names, temp_prefix);
        recip_hoist(gen_var_ids, *cse_vars, pool, cluster_unknown_names, temp_prefix);

        variant_cse_names: [..] string;
        for cv : cse_vars {
            if expr_is_loop_variant(cv.expr_id, pool.exprs, cluster_unknown_names, variant_cse_names) {
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
