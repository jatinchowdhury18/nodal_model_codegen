// Exercises the nonlinear-element building blocks in nodal_analysis.jai
// (KCL generalization for DIODE/DIODE_PAIR/BJT, junction-voltage closed
// forms) and nonlinear_solve.jai (clustering, I-V substitution, residual/
// Jacobian/delta construction), at the level below the full netlist/codegen
// pipeline.

#import "nodal_analysis";
#import "sym_algebra";
#import "nonlinear_solve";
#import "Basic";

check :: (label: string, e: Expr_ID, expected: string, exprs: [] Expr) {
    got := print_expr(e, exprs);
    if got == expected {
        print("PASS  [%]: %\n", label, got);
    } else {
        print("FAIL  [%]: got  %\n       expected  %\n", label, got, expected);
    }
}

check_bool :: (label: string, got: bool, expected: bool) {
    if got == expected  print("PASS  [%]\n", label);
    else                print("FAIL  [%]: got %, expected %\n", label, got, expected);
}

check_str :: (label: string, got: string, expected: string) {
    if got == expected  print("PASS  [%]: %\n", label, got);
    else                print("FAIL  [%]: got '%', expected '%'\n", label, got, expected);
}

print_cluster :: (c: Nonlinear_Cluster, exprs: [] Expr) {
    print("cluster '%': % unknown(s)\n", c.name, c.unknowns.count);
    for u : c.unknowns  print("  unknown %: vt=%, residual=%, delta=%\n", u.name, u.vt_name, u.residual_name, u.delta_name);
    print("  invariant items (%):\n", c.invariant_items.count);
    for gv : c.invariant_items  print("    % = %\n", gv.name, print_expr(gv.expr_id, exprs));
    print("  iteration items (%):\n", c.iteration_items.count);
    for gv : c.iteration_items  print("    % = %\n", gv.name, print_expr(gv.expr_id, exprs));
}

make_arr :: (list: [] $T) -> [..] T {
    dest: [..] T;
    array_copy(*dest, list);
    return dest;
}

main :: () {
    // 1. Single diode to ground: R1 from vi to vd, D1 anode=vd, cathode=0.
    //    Expect one MNA unknown (vd), solved in closed form parametrized by
    //    the diode's free branch current iD1, and a junction-voltage
    //    expression vD1 = vd - 0 = vd (in terms of vi, iD1).
    {
        print("--- single diode ---\n");
        circuit: [..] Element;
        array_add(*circuit, Element.{ name = "R1", type = .RESISTOR, terminals = make_arr(string.["vd", "vi"]) });
        array_add(*circuit, Element.{ name = "D1", type = .DIODE, terminals = make_arr(string.["vd", "0"]),
                                       nonlinear = .{ is_name = "D1_Is", vt_name = "D1_vt" } });

        solve_vars, equations, pool := generate_kcl_equations(circuit, .["vi"]);
        print("solve_vars count: %\n", solve_vars.count); // expect 1 (vd); iD1 is NOT an MNA unknown

        solutions := solve(equations, solve_vars, *pool);
        junctions := generate_junction_voltage_expressions(circuit, solve_vars, solutions, *pool);
        for j : junctions  print("%: %\n", j.name, print_expr(j.expr_id, pool.exprs));

        clusters := build_nonlinear_clusters(circuit, solve_vars, solutions, *pool);
        print("cluster count: %\n", clusters.count); // expect 1 (single device, single unknown)
        for c : clusters  print_cluster(c, pool.exprs);
    }

    // 2. Lone BJT: base fed through R1 from vi, collector tied to a supply
    //    node vcc (fixed voltage), emitter to ground through R2. Expect two
    //    free branch currents (iBQ1, iCQ1), two solved node voltages
    //    (vb, ve; vcc is itself an MNA unknown constrained to a fixed
    //    value), and junction voltages vBCQ1 = vb - vcc, vBEQ1 = vb - ve.
    {
        print("--- lone BJT ---\n");
        circuit: [..] Element;
        array_add(*circuit, Element.{ name = "R1", type = .RESISTOR, terminals = make_arr(string.["vb", "vi"]) });
        array_add(*circuit, Element.{ name = "R2", type = .RESISTOR, terminals = make_arr(string.["ve", "0"]) });
        array_add(*circuit, Element.{ name = "VCC", type = .FIXED_VOLTAGE, terminals = make_arr(string.["vcc"]) });
        array_add(*circuit, Element.{ name = "Q1", type = .NPN, terminals = make_arr(string.["vcc", "vb", "ve"]),
                                       nonlinear = .{ is_name = "Q1_Is", vt_name = "Q1_vt", betaf_name = "Q1_BetaF", betar_name = "Q1_BetaR" } });

        solve_vars, equations, pool := generate_kcl_equations(circuit, .["vi"]);
        print("solve_vars count: %\n", solve_vars.count); // expect 3 (vb, ve, vcc); iBQ1/iCQ1 are NOT MNA unknowns

        solutions := solve(equations, solve_vars, *pool);
        junctions := generate_junction_voltage_expressions(circuit, solve_vars, solutions, *pool);
        for j : junctions  print("%: %\n", j.name, print_expr(j.expr_id, pool.exprs));

        clusters := build_nonlinear_clusters(circuit, solve_vars, solutions, *pool);
        print("cluster count: %\n", clusters.count); // expect 1 (single device, both junctions coupled)
        for c : clusters  print_cluster(c, pool.exprs);
    }

    // 2b. Lone TRIODE: common-cathode-shaped stage. Plate through Rp to a
    //     supply node vp (fixed voltage), grid through Rg to vi, cathode
    //     through Rk to ground. Mirrors the lone-BJT case above: two free
    //     branch currents (iPX1, iGX1, from the plate/grid kcl_terms), and
    //     junction voltages vPKX1 = vpl - vk, vGKX1 = vg - vk, jointly solved
    //     in one cluster (both unknowns depend on both terminals via Ik/Ig).
    {
        print("--- lone TRIODE ---\n");
        circuit: [..] Element;
        array_add(*circuit, Element.{ name = "Rp", type = .RESISTOR, terminals = make_arr(string.["vp", "vpl"]) });
        array_add(*circuit, Element.{ name = "Rg", type = .RESISTOR, terminals = make_arr(string.["vg", "vi"]) });
        array_add(*circuit, Element.{ name = "Rk", type = .RESISTOR, terminals = make_arr(string.["vk", "0"]) });
        array_add(*circuit, Element.{ name = "VP", type = .FIXED_VOLTAGE, terminals = make_arr(string.["vp"]) });
        array_add(*circuit, Element.{ name = "X1", type = .TRIODE, terminals = make_arr(string.["vpl", "vg", "vk"]),
                                       nonlinear = .{ gk_name = "X1_Gk", mu_name = "X1_Mu", gamma_name = "X1_Gamma",
                                                       ck_name = "X1_Ck", gg_name = "X1_Gg", xi_name = "X1_Xi",
                                                       cg_name = "X1_Cg", ig0_name = "X1_Ig0" } });

        solve_vars, equations, pool := generate_kcl_equations(circuit, .["vi"]);
        print("solve_vars count: %\n", solve_vars.count); // expect 4 (vpl, vg, vk, vp); iPX1/iGX1 are NOT MNA unknowns

        solutions := solve(equations, solve_vars, *pool);
        junctions := generate_junction_voltage_expressions(circuit, solve_vars, solutions, *pool);
        for j : junctions  print("%: %\n", j.name, print_expr(j.expr_id, pool.exprs));

        clusters := build_nonlinear_clusters(circuit, solve_vars, solutions, *pool);
        check_bool("one cluster (vPK/vGK jointly coupled)", clusters.count == 1, true);
        check_bool("two joint unknowns", clusters[0].unknowns.count == 2, true);
        u0 := clusters[0].unknowns[0];
        u1 := clusters[0].unknowns[1];
        check_bool("unknowns named vPKX1/vGKX1", (u0.name == "vPKX1" && u1.name == "vGKX1") || (u0.name == "vGKX1" && u1.name == "vPKX1"), true);
        check_bool("no step limiter (Dempwolf I-V law is smooth everywhere)", u0.limiter_kind == .NONE && u1.limiter_kind == .NONE, true);

        // The Jacobian/residual construction must actually reference the
        // model's own params (Gk/Gamma/Gg/Xi), not just pass them through
        // untouched; otherwise the I-V law substitution silently no-opped.
        references_model := false;
        for gv : clusters[0].invariant_items  if String.contains(print_expr(gv.expr_id, pool.exprs), "X1_Gk")  references_model = true;
        for gv : clusters[0].iteration_items  if String.contains(print_expr(gv.expr_id, pool.exprs), "X1_Gk")  references_model = true;
        check_bool("Newton system references the triode's model params", references_model, true);
        print_cluster(clusters[0], pool.exprs);
    }

    // 3. Coupled devices: a diode D1 sharing a node with a BJT Q1's base,
    //    so their branch currents must interact in the closed-form linear
    //    system and get merged into ONE joint cluster (2 devices, 3
    //    unknowns: vD1, vBCQ1, vBEQ1), exercising clustering for real,
    //    ahead of the full clip-stage test fixture in Phase 3.
    {
        print("--- coupled diode + BJT ---\n");
        circuit: [..] Element;
        array_add(*circuit, Element.{ name = "R1", type = .RESISTOR, terminals = make_arr(string.["vb", "vi"]) });
        array_add(*circuit, Element.{ name = "R2", type = .RESISTOR, terminals = make_arr(string.["ve", "0"]) });
        array_add(*circuit, Element.{ name = "VCC", type = .FIXED_VOLTAGE, terminals = make_arr(string.["vcc"]) });
        array_add(*circuit, Element.{ name = "Q1", type = .NPN, terminals = make_arr(string.["vcc", "vb", "ve"]),
                                       nonlinear = .{ is_name = "Q1_Is", vt_name = "Q1_vt", betaf_name = "Q1_BetaF", betar_name = "Q1_BetaR" } });
        // D1 shares node "vb" (the BJT's base) with its anode, coupling the two devices.
        array_add(*circuit, Element.{ name = "D1", type = .DIODE, terminals = make_arr(string.["vb", "0"]),
                                       nonlinear = .{ is_name = "D1_Is", vt_name = "D1_vt" } });

        solve_vars, equations, pool := generate_kcl_equations(circuit, .["vi"]);
        solutions := solve(equations, solve_vars, *pool);

        clusters := build_nonlinear_clusters(circuit, solve_vars, solutions, *pool);
        print("cluster count: %\n", clusters.count); // expect 1 (coupled)
        for c : clusters  print_cluster(c, pool.exprs);
    }

    // 4. DC operating-point solve (dc_mode=true): a common-emitter-shaped
    //    circuit with a base-bias network (R1/R2/VCC) and an input coupling
    //    capacitor C1 whose terminal is a raw circuit input ("vi"), not a
    //    solved node. Exercises generate_kcl_equations(dc_mode=true)
    //    (capacitors excluded/open) and lookup_node_voltage() resolving a raw
    //    input node (needed to derive C1's DC-consistent z-state). The
    //    actual bug this test was written to catch is that EVERY branch
    //    current symbol (iBQ1, iCQ1) actually gets substituted with its I-V
    //    law everywhere it appears, including in OTHER nodes' closed forms
    //    (here, "vb"'s formula depends on iBQ1 via KCL) built earlier by
    //    build_nonlinear_clusters() itself. Two real, non-obvious bugs were
    //    found this way and are now fixed:
    //      (a) build_nonlinear_clusters()'s own vbc/vbe construction shared
    //          (not copied) `base_v` into two separate simplify() calls,
    //          each of which mutates nodes in place while recursing into
    //          children, corrupting "vb"'s own shared closed-form node as
    //          a side effect. Fixed with copy_expr().
    //      (b) fold()'s NEG-of-NEG collapse (-(-x) -> x) does a full struct
    //          copy when x is a SYMBOL, silently creating a second node
    //          with the same name but a different ID, so substituting
    //          "the" iBQ1 node by name-based lookup (which only finds one
    //          match) can miss the other one. Fixed by having the
    //          substitution loop scan for and replace ALL matching-named
    //          symbol nodes, not just the first.
    {
        print("--- DC operating point: common-emitter-shaped circuit ---\n");
        circuit: [..] Element;
        array_add(*circuit, Element.{ name = "R1", type = .RESISTOR, terminals = make_arr(string.["vp", "vb"]) });
        array_add(*circuit, Element.{ name = "R2", type = .RESISTOR, terminals = make_arr(string.["vb", "0"]) });
        array_add(*circuit, Element.{ name = "RE", type = .RESISTOR, terminals = make_arr(string.["ve", "0"]) });
        array_add(*circuit, Element.{ name = "RC", type = .RESISTOR, terminals = make_arr(string.["vp", "vc"]) });
        array_add(*circuit, Element.{ name = "VCC", type = .FIXED_VOLTAGE, terminals = make_arr(string.["vp"]) });
        array_add(*circuit, Element.{ name = "Q1", type = .NPN, terminals = make_arr(string.["vc", "vb", "ve"]),
                                       nonlinear = .{ is_name = "Q1_Is", vt_name = "Q1_vt", betaf_name = "Q1_BetaF", betar_name = "Q1_BetaR" } });
        array_add(*circuit, Element.{ name = "C1", type = .CAPACITOR, terminals = make_arr(string.["vi", "vb"]) });

        dc_solve_vars, dc_equations, dc_pool := generate_kcl_equations(circuit, .["vi"], true);
        dc_solutions := solve(dc_equations, dc_solve_vars, *dc_pool);
        dc_clusters := build_nonlinear_clusters(circuit, dc_solve_vars, dc_solutions, *dc_pool);
        print("cluster count: %\n", dc_clusters.count); // expect 1 (Q1's vBC/vBE)

        // No node's closed form should still reference a branch-current
        // symbol after build_nonlinear_clusters(); every one of them must
        // have been substituted with its device's I-V law.
        all_ok := true;
        for node : dc_solve_vars {
            refs: [..] string;
            collect_symbol_names(node, dc_pool.exprs, *refs);
            for r : refs {
                if array_find(string.["iBQ1", "iCQ1"], r) {
                    print("FAIL: a node's closed form still references unsubstituted branch current '%'\n", r);
                    all_ok = false;
                }
            }
        }
        if all_ok  print("PASS  [dc_no_leaked_branch_currents]\n");

        // C1's terminal "vi" is a raw circuit input, not a solved node --
        // lookup_node_voltage() must resolve it via the `inputs` parameter
        // rather than asserting.
        vi_v := lookup_node_voltage("vi", dc_solve_vars, dc_solutions, *dc_pool, .["vi"]);
        check("dc_input_node_lookup", vi_v, "vi", dc_pool.exprs);
    }

    // 5. Op-amp output clip (AMP_CLIP): inverting amp (R1 from vi to n1, RF
    //    from n1 to vo, pos tied to 0) with a finite Aol and Vsat+/Vsat- set,
    //    no Rout. Exercises the AMP-specific device-collection loop in
    //    build_nonlinear_clusters() (distinct from the generic
    //    nonlinear_device_spec()-driven diode/BJT/JFET path (see its
    //    comment) and generate_kcl_equations()'s vclip_<name> exclusion from
    //    eqn_nodes/solve_vars.
    //
    //    Two real bugs were found and fixed getting this circuit's math to
    //    check out. A naive smoke test alone didn't catch either; both
    //    needed comparing generated closed forms/output against a hand
    //    derivation to notice something was numerically wrong:
    //      (a) Rout's synthesized resistor (netlist_codegen.jai) is wired
    //          using "vclip_<name>" as a literal terminal string, so the
    //          generic eqn_nodes population loop (which treats any element's
    //          terminal as a real KCL-governed node) wrongly pulled it into
    //          solve_vars, turning what should be a free Newton symbol
    //          (same role as a diode's branch current) into an ordinary
    //          linear unknown, silently discarding the whole clip nonlinearity.
    //          Fixed by excluding clip-stage node names from eqn_nodes,
    //          exactly like input nodes already are.
    //      (b) differentiate()'s SYMBOL case compared by raw Expr_ID, not by
    //          name. That's the same "second node, same name, different ID"
    //          hazard already documented for substitution (fold()'s
    //          `-(-x) -> x` rule can produce one). Since a clip's residual
    //          is plain linear (unlike diode/BJT/JFET's transcendental I-V
    //          laws), its Jacobian entry is exactly the kind of small,
    //          heavily-folded expression likely to trigger this: an
    //          occurrence of vclip_<name> from a duplicate node
    //          differentiated to 0 instead of 1, undercounting the
    //          feedback term entirely. Fixed by comparing symbol names.
    {
        print("--- op-amp output clip ---\n");
        circuit: [..] Element;
        array_add(*circuit, Element.{ name = "R1", type = .RESISTOR, terminals = make_arr(string.["vi", "n1"]) });
        array_add(*circuit, Element.{ name = "RF", type = .RESISTOR, terminals = make_arr(string.["n1", "vo"]) });
        amp := Element.{ name = "Eop", type = .AMP,
            opamp = .{ has_aol = true, aol_name = "Eop_Aol",
                       has_vsat_p = true, has_vsat_n = true, vsat_p_name = "Eop_VsatP", vsat_n_name = "Eop_VsatN" } };
        array_add(*amp.terminals, "vo");
        array_add(*amp.terminals, "n1");
        array_add(*amp.terminals, "0");
        array_add(*circuit, amp);

        solve_vars, equations, pool := generate_kcl_equations(circuit, .["vi"]);
        check_bool("vclip_Eop excluded from solve_vars", solve_vars.count == 3, true); // n1, vo, vgain_Eop

        solutions := solve(equations, solve_vars, *pool);
        clusters := build_nonlinear_clusters(circuit, solve_vars, solutions, *pool);
        check_bool("one AMP_CLIP cluster", clusters.count == 1, true);
        check_bool("one unknown in the cluster", clusters[0].unknowns.count == 1, true);
        u := clusters[0].unknowns[0];
        check_str("unknown name", u.name, "vclip_Eop");
        check_bool("limiter kind is AMP_CLIP", u.limiter_kind == .AMP_CLIP, true);
        check_str("vsat_p_name threaded through", u.vsat_p_name, "Eop_VsatP");
        check_str("vsat_n_name threaded through", u.vsat_n_name, "Eop_VsatN");

        // "vo"'s closed form must reference vclip_Eop. If it doesn't
        // (bug (a) above), the whole clip nonlinearity silently vanishes
        // from the circuit's actual output.
        vo_idx := -1;
        for sv, i : solve_vars  if print_expr(sv, pool.exprs) == "vo"  vo_idx = i;
        vo_refs: [..] string;
        collect_symbol_names(solutions[vo_idx], pool.exprs, *vo_refs);
        check_bool("vo depends on vclip_Eop", array_find(vo_refs, "vclip_Eop"), true);

        // The Jacobian (d(residual)/d(vclip_Eop)) must pick up the feedback
        // term through RF, not just the trivial "-1" from `f = closed_form -
        // vclip_Eop`'s own subtraction (bug (b) above). That means some
        // invariant/iteration item must actually reference Aol, not just the
        // residual's own affine "+vclip_Eop" term.
        references_aol := false;
        for gv : clusters[0].invariant_items  if String.contains(print_expr(gv.expr_id, pool.exprs), "Eop_Aol")  references_aol = true;
        for gv : clusters[0].iteration_items  if String.contains(print_expr(gv.expr_id, pool.exprs), "Eop_Aol")  references_aol = true;
        check_bool("Jacobian/delta construction references Aol (not a trivial constant)", references_aol, true);
    }

    // 6. Op-amp slew-rate limiting, no Vsat at all (regression guard for a
    //    bug found while testing this: clamp_opamp_output's generated call
    //    always passes both Vsat rail args, so a slew-only op-amp needs
    //    vsat_p_name/vsat_n_name populated with permissive defaults too --
    //    the gate for that was originally `has_vsat_p || has_vsat_n`, which
    //    stayed false for a slew-only model and left the rail args as empty
    //    strings in the generated code).
    {
        print("--- op-amp slew, no Vsat ---\n");
        circuit: [..] Element;
        array_add(*circuit, Element.{ name = "R1", type = .RESISTOR, terminals = make_arr(string.["vi", "n1"]) });
        array_add(*circuit, Element.{ name = "RF", type = .RESISTOR, terminals = make_arr(string.["n1", "vo"]) });
        amp := Element.{ name = "Eop", type = .AMP,
            opamp = .{ has_aol = true, aol_name = "Eop_Aol",
                       has_vsat_p = false, has_vsat_n = false, vsat_p_name = "Eop_VsatP", vsat_n_name = "Eop_VsatN",
                       has_slew = true, slew_name = "Eop_Slew" } };
        array_add(*amp.terminals, "vo");
        array_add(*amp.terminals, "n1");
        array_add(*amp.terminals, "0");
        array_add(*circuit, amp);

        solve_vars, equations, pool := generate_kcl_equations(circuit, .["vi"]);
        check_bool("slew alone still routes through the free vclip symbol", solve_vars.count == 3, true);

        solutions := solve(equations, solve_vars, *pool);
        clusters := build_nonlinear_clusters(circuit, solve_vars, solutions, *pool);
        check_bool("one cluster", clusters.count == 1, true);
        u := clusters[0].unknowns[0];
        check_bool("has_slew threaded through", u.has_slew, true);
        check_str("slew_name threaded through", u.slew_name, "Eop_Slew");
        check_str("zslew_name derived from element name", u.zslew_name, "zslew_Eop");
        // vsat_p_name/vsat_n_name must still be non-empty even though Vsat
        // itself is unset; clamp_opamp_output's call always needs both.
        check_bool("vsat_p_name non-empty despite no Vsat", u.vsat_p_name.count > 0, true);
        check_bool("vsat_n_name non-empty despite no Vsat", u.vsat_n_name.count > 0, true);
    }
}
