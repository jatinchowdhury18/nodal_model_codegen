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

        solve_vars, equations, exprs := generate_kcl_equations(circuit, .["vi"]);
        print("solve_vars count: %\n", solve_vars.count); // expect 1 (vd) -- iD1 is NOT an MNA unknown

        solutions := solve(equations, solve_vars, *exprs);
        junctions := generate_junction_voltage_expressions(circuit, solve_vars, solutions, *exprs);
        for j : junctions  print("%: %\n", j.name, print_expr(j.expr_id, exprs));

        clusters := build_nonlinear_clusters(circuit, solve_vars, solutions, *exprs);
        print("cluster count: %\n", clusters.count); // expect 1 (single device, single unknown)
        for c : clusters  print_cluster(c, exprs);
    }

    // 2. Lone BJT: base fed through R1 from vi, collector tied to a supply
    //    node vcc (fixed voltage), emitter to ground through R2. Expect two
    //    free branch currents (iBQ1, iCQ1), two solved node voltages
    //    (vb, ve -- vcc is itself an MNA unknown constrained to a fixed
    //    value), and junction voltages vBCQ1 = vb - vcc, vBEQ1 = vb - ve.
    {
        print("--- lone BJT ---\n");
        circuit: [..] Element;
        array_add(*circuit, Element.{ name = "R1", type = .RESISTOR, terminals = make_arr(string.["vb", "vi"]) });
        array_add(*circuit, Element.{ name = "R2", type = .RESISTOR, terminals = make_arr(string.["ve", "0"]) });
        array_add(*circuit, Element.{ name = "VCC", type = .FIXED_VOLTAGE, terminals = make_arr(string.["vcc"]) });
        array_add(*circuit, Element.{ name = "Q1", type = .BJT, terminals = make_arr(string.["vcc", "vb", "ve"]),
                                       nonlinear = .{ is_name = "Q1_Is", vt_name = "Q1_vt", betaf_name = "Q1_BetaF", betar_name = "Q1_BetaR" } });

        solve_vars, equations, exprs := generate_kcl_equations(circuit, .["vi"]);
        print("solve_vars count: %\n", solve_vars.count); // expect 3 (vb, ve, vcc) -- iBQ1/iCQ1 are NOT MNA unknowns

        solutions := solve(equations, solve_vars, *exprs);
        junctions := generate_junction_voltage_expressions(circuit, solve_vars, solutions, *exprs);
        for j : junctions  print("%: %\n", j.name, print_expr(j.expr_id, exprs));

        clusters := build_nonlinear_clusters(circuit, solve_vars, solutions, *exprs);
        print("cluster count: %\n", clusters.count); // expect 1 (single device, both junctions coupled)
        for c : clusters  print_cluster(c, exprs);
    }

    // 3. Coupled devices: a diode D1 sharing a node with a BJT Q1's base,
    //    so their branch currents must interact in the closed-form linear
    //    system and get merged into ONE joint cluster (2 devices, 3
    //    unknowns: vD1, vBCQ1, vBEQ1) -- exercising clustering for real,
    //    ahead of the full clip-stage test fixture in Phase 3.
    {
        print("--- coupled diode + BJT ---\n");
        circuit: [..] Element;
        array_add(*circuit, Element.{ name = "R1", type = .RESISTOR, terminals = make_arr(string.["vb", "vi"]) });
        array_add(*circuit, Element.{ name = "R2", type = .RESISTOR, terminals = make_arr(string.["ve", "0"]) });
        array_add(*circuit, Element.{ name = "VCC", type = .FIXED_VOLTAGE, terminals = make_arr(string.["vcc"]) });
        array_add(*circuit, Element.{ name = "Q1", type = .BJT, terminals = make_arr(string.["vcc", "vb", "ve"]),
                                       nonlinear = .{ is_name = "Q1_Is", vt_name = "Q1_vt", betaf_name = "Q1_BetaF", betar_name = "Q1_BetaR" } });
        // D1 shares node "vb" (the BJT's base) with its anode -- couples the two devices.
        array_add(*circuit, Element.{ name = "D1", type = .DIODE, terminals = make_arr(string.["vb", "0"]),
                                       nonlinear = .{ is_name = "D1_Is", vt_name = "D1_vt" } });

        solve_vars, equations, exprs := generate_kcl_equations(circuit, .["vi"]);
        solutions := solve(equations, solve_vars, *exprs);

        clusters := build_nonlinear_clusters(circuit, solve_vars, solutions, *exprs);
        print("cluster count: %\n", clusters.count); // expect 1 (coupled)
        for c : clusters  print_cluster(c, exprs);
    }

    // 4. DC operating-point solve (dc_mode=true): a common-emitter-shaped
    //    circuit with a base-bias network (R1/R2/VCC) and an input coupling
    //    capacitor C1 whose terminal is a raw circuit input ("vi"), not a
    //    solved node. Exercises generate_kcl_equations(dc_mode=true)
    //    (capacitors excluded/open), lookup_node_voltage() resolving a raw
    //    input node (needed to derive C1's DC-consistent z-state), and --
    //    the actual bug this test was written to catch -- that EVERY branch
    //    current symbol (iBQ1, iCQ1) actually gets substituted with its I-V
    //    law everywhere it appears, including in OTHER nodes' closed forms
    //    (here, "vb"'s formula depends on iBQ1 via KCL) built earlier by
    //    build_nonlinear_clusters() itself. Two real, non-obvious bugs were
    //    found this way and are now fixed:
    //      (a) build_nonlinear_clusters()'s own vbc/vbe construction shared
    //          (not copied) `base_v` into two separate simplify() calls,
    //          each of which mutates nodes in place while recursing into
    //          children -- corrupting "vb"'s own shared closed-form node as
    //          a side effect. Fixed with copy_expr().
    //      (b) fold()'s NEG-of-NEG collapse (-(-x) -> x) does a full struct
    //          copy when x is a SYMBOL, silently creating a second node
    //          with the same name but a different ID -- so substituting
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
        array_add(*circuit, Element.{ name = "Q1", type = .BJT, terminals = make_arr(string.["vc", "vb", "ve"]),
                                       nonlinear = .{ is_name = "Q1_Is", vt_name = "Q1_vt", betaf_name = "Q1_BetaF", betar_name = "Q1_BetaR" } });
        array_add(*circuit, Element.{ name = "C1", type = .CAPACITOR, terminals = make_arr(string.["vi", "vb"]) });

        dc_solve_vars, dc_equations, dc_exprs := generate_kcl_equations(circuit, .["vi"], true);
        dc_solutions := solve(dc_equations, dc_solve_vars, *dc_exprs);
        dc_clusters := build_nonlinear_clusters(circuit, dc_solve_vars, dc_solutions, *dc_exprs);
        print("cluster count: %\n", dc_clusters.count); // expect 1 (Q1's vBC/vBE)

        // No node's closed form should still reference a branch-current
        // symbol after build_nonlinear_clusters() -- every one of them must
        // have been substituted with its device's I-V law.
        all_ok := true;
        for node : dc_solve_vars {
            refs: [..] string;
            collect_symbol_names(node, dc_exprs, *refs);
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
        vi_v := lookup_node_voltage("vi", dc_solve_vars, dc_solutions, *dc_exprs, .["vi"]);
        check("dc_input_node_lookup", vi_v, "vi", dc_exprs);
    }
}
