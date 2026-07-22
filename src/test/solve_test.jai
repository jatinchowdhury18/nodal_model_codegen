#import "sym_algebra";
#import "Basic";
#import "Process";

main :: () {
    { // 1x1: gR1*(vo - vi) + gC1*vo - zC1 = 0  =>  vo = (gR1*vi + zC1) / (gR1 + gC1)
        pool: Expr_Pool;
        vo  := expr_symbol("vo",  *pool);
        vi  := expr_symbol("vi",  *pool);
        gR1 := expr_symbol("gR1", *pool);
        gC1 := expr_symbol("gC1", *pool);
        zC1 := expr_symbol("zC1", *pool);
        eq := expr_add(
            expr_mul(gR1, expr_sub(vo, vi, *pool), *pool),
            expr_sub(expr_mul(gC1, vo, *pool), zC1, *pool),
            *pool);
        solutions := solve(.[eq], .[vo], *pool);
        wolfram_verify("1x1", .["vo"], .[eq], solutions, pool.exprs);
    }

    { // 2x2: two-node resistor ladder
        // eq1: gR1*(v1-vi) + gR2*(v1-v2) = 0
        // eq2: gR2*(v2-v1) + gR3*v2 = 0
        pool: Expr_Pool;
        v1  := expr_symbol("v1",  *pool);
        v2  := expr_symbol("v2",  *pool);
        vi  := expr_symbol("vi",  *pool);
        gR1 := expr_symbol("gR1", *pool);
        gR2 := expr_symbol("gR2", *pool);
        gR3 := expr_symbol("gR3", *pool);
        eq1 := expr_add(
            expr_mul(gR1, expr_sub(v1, vi, *pool), *pool),
            expr_mul(gR2, expr_sub(v1, v2, *pool), *pool),
            *pool);
        eq2 := expr_add(
            expr_mul(gR2, expr_sub(v2, v1, *pool), *pool),
            expr_mul(gR3, v2, *pool),
            *pool);
        solutions := solve(.[eq1, eq2], .[v1, v2], *pool);
        wolfram_verify("2x2", .["v1", "v2"], .[eq1, eq2], solutions, pool.exprs);
    }

    { // 4x4: four-node resistor ladder
        // eq1: gR1*(v1-vi) + gR2*(v1-v2) = 0
        // eq2: gR2*(v2-v1) + gR3*(v2-v3) = 0
        // eq3: gR3*(v3-v2) + gR4*(v3-v4) = 0
        // eq4: gR4*(v4-v3) + gR5*v4      = 0
        pool: Expr_Pool;
        v1  := expr_symbol("v1",  *pool);
        v2  := expr_symbol("v2",  *pool);
        v3  := expr_symbol("v3",  *pool);
        v4  := expr_symbol("v4",  *pool);
        vi  := expr_symbol("vi",  *pool);
        gR1 := expr_symbol("gR1", *pool);
        gR2 := expr_symbol("gR2", *pool);
        gR3 := expr_symbol("gR3", *pool);
        gR4 := expr_symbol("gR4", *pool);
        gR5 := expr_symbol("gR5", *pool);
        eq1 := expr_add(expr_mul(gR1, expr_sub(v1, vi, *pool), *pool), expr_mul(gR2, expr_sub(v1, v2, *pool), *pool), *pool);
        eq2 := expr_add(expr_mul(gR2, expr_sub(v2, v1, *pool), *pool), expr_mul(gR3, expr_sub(v2, v3, *pool), *pool), *pool);
        eq3 := expr_add(expr_mul(gR3, expr_sub(v3, v2, *pool), *pool), expr_mul(gR4, expr_sub(v3, v4, *pool), *pool), *pool);
        eq4 := expr_add(expr_mul(gR4, expr_sub(v4, v3, *pool), *pool), expr_mul(gR5, v4, *pool), *pool);
        solutions := solve(.[eq1, eq2, eq3, eq4], .[v1, v2, v3, v4], *pool);
        wolfram_verify("4x4", .["v1", "v2", "v3", "v4"], .[eq1, eq2, eq3, eq4], solutions, pool.exprs);
    }

    { // 3x3 sparse: opamp buffer (iE appears in only one equation)
        // eq1: gR1*(vi-v1) - gR2*v1 = 0
        // eq2: gR3*(vi-v2) + iE = 0
        // eq3: v2 - v1 = 0
        pool: Expr_Pool;
        iE  := expr_symbol("iE",  *pool);
        v1  := expr_symbol("v1",  *pool);
        v2  := expr_symbol("v2",  *pool);
        vi  := expr_symbol("vi",  *pool);
        gR1 := expr_symbol("gR1", *pool);
        gR2 := expr_symbol("gR2", *pool);
        gR3 := expr_symbol("gR3", *pool);
        eq1 := expr_sub(expr_mul(gR1, expr_sub(vi, v1, *pool), *pool), expr_mul(gR2, v1, *pool), *pool);
        eq2 := expr_add(expr_mul(gR3, expr_sub(vi, v2, *pool), *pool), iE, *pool);
        eq3 := expr_sub(v2, v1, *pool);
        solutions := solve(.[eq1, eq2, eq3], .[iE, v1, v2], *pool);
        wolfram_verify("3x3", .["iE", "v1", "v2"], .[eq1, eq2, eq3], solutions, pool.exprs);
    }

    { // 10x10: full nodal circuit system
        // Variables: { iE8, vo10, vo, v33, vn9, v36, iE10, iE9, vr45, v37 }
        pool: Expr_Pool;

        // unknowns
        iE8  := expr_symbol("iE8",  *pool);
        vo10 := expr_symbol("vo10", *pool);
        vo   := expr_symbol("vo",   *pool);
        v33  := expr_symbol("v33",  *pool);
        vn9  := expr_symbol("vn9",  *pool);
        v36  := expr_symbol("v36",  *pool);
        iE10 := expr_symbol("iE10", *pool);
        iE9  := expr_symbol("iE9",  *pool);
        vr45 := expr_symbol("vr45", *pool);
        v37  := expr_symbol("v37",  *pool);

        // parameters
        vi   := expr_symbol("vi",   *pool);
        gR33 := expr_symbol("gR33", *pool);
        gR35 := expr_symbol("gR35", *pool);
        gR37 := expr_symbol("gR37", *pool);
        gR38 := expr_symbol("gR38", *pool);
        gR39 := expr_symbol("gR39", *pool);
        gR40 := expr_symbol("gR40", *pool);
        gR41 := expr_symbol("gR41", *pool);
        gR42 := expr_symbol("gR42", *pool);
        gR43 := expr_symbol("gR43", *pool);
        gR44 := expr_symbol("gR44", *pool);
        gR45 := expr_symbol("gR45", *pool);
        gR47 := expr_symbol("gR47", *pool);
        gR48 := expr_symbol("gR48", *pool);
        gR66 := expr_symbol("gR66", *pool);
        gC16 := expr_symbol("gC16", *pool);
        gC17 := expr_symbol("gC17", *pool);
        gC18 := expr_symbol("gC18", *pool);
        gC19 := expr_symbol("gC19", *pool);
        gC20 := expr_symbol("gC20", *pool);
        zC16 := expr_symbol("zC16", *pool);
        zC17 := expr_symbol("zC17", *pool);
        zC18 := expr_symbol("zC18", *pool);
        zC19 := expr_symbol("zC19", *pool);
        zC20 := expr_symbol("zC20", *pool);

        zero      := expr_const(0, *pool);
        v33_m_v37 := expr_sub(v33, v37, *pool);
        vn9_m_v36 := expr_sub(vn9, v36, *pool);

        // eq1: (gR33*(vi-0) + gR37*(vo10-0)) - gR35*(0-vo) = 0
        eq1 := expr_sub(
            expr_add(expr_mul(gR33, expr_sub(vi,   zero, *pool), *pool),
                     expr_mul(gR37, expr_sub(vo10, zero, *pool), *pool), *pool),
            expr_mul(gR35, expr_sub(zero, vo, *pool), *pool), *pool);

        // eq2: -(gR37*(vo10-0)) - gR66*(vo10-v33) + gR48*(0-vo10) + iE10 = 0
        eq2 := expr_add(
            expr_add(
                expr_sub(expr_neg(expr_mul(gR37, expr_sub(vo10, zero, *pool), *pool), *pool),
                         expr_mul(gR66, expr_sub(vo10, v33, *pool), *pool), *pool),
                expr_mul(gR48, expr_sub(zero, vo10, *pool), *pool), *pool),
            iE10, *pool);

        // eq3: gR35*(0-vo) - gR40*(vo-v33) + iE8 = 0
        eq3 := expr_add(
            expr_sub(expr_mul(gR35, expr_sub(zero, vo,  *pool), *pool),
                     expr_mul(gR40, expr_sub(vo,   v33, *pool), *pool), *pool),
            iE8, *pool);

        // eq4: gR40*(vo-v33) + gR39*(vi-v33) + gR66*(vo10-v33)
        //      - gR41*(v33-0) - gR38*(v33-0) - (gC16*(v33-v37)-zC16) - (gC17*(v33-v37)-zC17) = 0
        eq4_a := expr_add(expr_mul(gR40, expr_sub(vo,   v33, *pool), *pool), expr_mul(gR39, expr_sub(vi,   v33, *pool), *pool), *pool);
        eq4_b := expr_add(eq4_a, expr_mul(gR66, expr_sub(vo10, v33, *pool), *pool), *pool);
        eq4_c := expr_sub(eq4_b, expr_mul(gR41, expr_sub(v33, zero, *pool), *pool), *pool);
        eq4_d := expr_sub(eq4_c, expr_mul(gR38, expr_sub(v33, zero, *pool), *pool), *pool);
        eq4_e := expr_sub(eq4_d, expr_sub(expr_mul(gC16, v33_m_v37, *pool), zC16, *pool), *pool);
        eq4   := expr_sub(eq4_e, expr_sub(expr_mul(gC17, v33_m_v37, *pool), zC17, *pool), *pool);

        // eq5: -(gR44*(vn9-0)) - gR45*(vn9-vr45) = 0
        eq5 := expr_sub(
            expr_neg(expr_mul(gR44, expr_sub(vn9, zero, *pool), *pool), *pool),
            expr_mul(gR45, expr_sub(vn9, vr45, *pool), *pool), *pool);

        // eq6: -(gR47*(v36-0)) + gR43*(vn9-v36) + (gC18*(vn9-v36)-zC18)
        //      + (gC19*(vn9-v36)-zC19) + (gC20*(vr45-v36)-zC20) + iE9 = 0
        eq6_a := expr_add(expr_neg(expr_mul(gR47, expr_sub(v36, zero, *pool), *pool), *pool),
                          expr_mul(gR43, vn9_m_v36, *pool), *pool);
        eq6_b := expr_add(eq6_a, expr_sub(expr_mul(gC18, vn9_m_v36, *pool), zC18, *pool), *pool);
        eq6_c := expr_add(eq6_b, expr_sub(expr_mul(gC19, vn9_m_v36, *pool), zC19, *pool), *pool);
        eq6_d := expr_add(eq6_c, expr_sub(expr_mul(gC20, expr_sub(vr45, v36, *pool), *pool), zC20, *pool), *pool);
        eq6   := expr_add(eq6_d, iE9, *pool);

        // eq7: gR47*(v36-0) - gR48*(0-vo10) = 0
        eq7 := expr_sub(
            expr_mul(gR47, expr_sub(v36,  zero, *pool), *pool),
            expr_mul(gR48, expr_sub(zero, vo10, *pool), *pool), *pool);

        // eq8: -(gR43*(vn9-v36)) - (gC18*(vn9-v36)-zC18) - (gC19*(vn9-v36)-zC19)
        //      + gR42*(v37-vn9) = 0
        eq8_a := expr_neg(expr_mul(gR43, vn9_m_v36, *pool), *pool);
        eq8_b := expr_sub(eq8_a, expr_sub(expr_mul(gC18, vn9_m_v36, *pool), zC18, *pool), *pool);
        eq8_c := expr_sub(eq8_b, expr_sub(expr_mul(gC19, vn9_m_v36, *pool), zC19, *pool), *pool);
        eq8   := expr_add(eq8_c, expr_mul(gR42, expr_sub(v37, vn9, *pool), *pool), *pool);

        // eq9: gR45*(vn9-vr45) - (gC20*(vr45-v36)-zC20) = 0
        eq9 := expr_sub(
            expr_mul(gR45, expr_sub(vn9,  vr45, *pool), *pool),
            expr_sub(expr_mul(gC20, expr_sub(vr45, v36, *pool), *pool), zC20, *pool), *pool);

        // eq10: -(gR42*(v37-vn9)) + (gC16*(v33-v37)-zC16) + (gC17*(v33-v37)-zC17) = 0
        eq10_a := expr_neg(expr_mul(gR42, expr_sub(v37, vn9, *pool), *pool), *pool);
        eq10_b := expr_add(eq10_a, expr_sub(expr_mul(gC16, v33_m_v37, *pool), zC16, *pool), *pool);
        eq10   := expr_add(eq10_b, expr_sub(expr_mul(gC17, v33_m_v37, *pool), zC17, *pool), *pool);

        var_names := string.["iE8", "vo10", "vo", "v33", "vn9", "v36", "iE10", "iE9", "vr45", "v37"];
        solve_for := Expr_ID.[iE8, vo10, vo, v33, vn9, v36, iE10, iE9, vr45, v37];
        solutions := solve(.[eq1, eq2, eq3, eq4, eq5, eq6, eq7, eq8, eq9, eq10], solve_for, *pool);
        wolfram_verify("10x10", var_names,
            .[eq1, eq2, eq3, eq4, eq5, eq6, eq7, eq8, eq9, eq10], solutions, pool.exprs);
    }
}

// Verifies Jai's solutions against Wolfram's own Solve, using
// TrueQ@PossibleZeroQ[jai_sol - ref_sol] for fast symbolic+numeric comparison.
// Both equation strings and solution strings are built via print_expr.
wolfram_verify :: (label: string, var_names: [] string, equations: [] Expr_ID,
                   solutions: [] Expr_ID, exprs: [] Expr) {
    assert(var_names.count == solutions.count);
    sb: String_Builder;

    // vars = {v1, v2, ...}
    append(*sb, "vars = {");
    for i: 0..var_names.count-1 {
        if i > 0  append(*sb, ", ");
        append(*sb, var_names[i]);
    }
    append(*sb, "};\n");

    // jaiSols = {sol1, sol2, ...}  -- built from print_expr
    append(*sb, "jaiSols = {");
    for i: 0..solutions.count-1 {
        if i > 0  append(*sb, ", ");
        append(*sb, print_expr(solutions[i], exprs));
    }
    append(*sb, "};\n");

    // eqns = {lhs1 == 0, lhs2 == 0, ...}  -- built from print_expr
    append(*sb, "eqns = {");
    for i: 0..equations.count-1 {
        if i > 0  append(*sb, ", ");
        print_to_builder(*sb, "(%) == 0", print_expr(equations[i], exprs));
    }
    append(*sb, "};\n");

    // use Wolfram's Solve as ground-truth reference, then compare:
    //   TrueQ@PossibleZeroQ[jai_sol - ref_sol]  (fast symbolic+numeric check)
    append(*sb, "refSol = Quiet[Solve[eqns, vars]];\n");
    print_to_builder(*sb, "If[Length[refSol] == 0, Print[\"%: SINGULAR\"]; Exit[1]];\n", label);
    append(*sb, "ref = vars /. refSol[[1]];\n");
    append(*sb, "results = MapThread[TrueQ@PossibleZeroQ[#1 - #2] &, {jaiSols, ref}];\n");
    append(*sb, "allPass = And @@ results;\n");
    print_to_builder(*sb, "Print[\"%: \", If[allPass, \"PASS\", \"FAIL\"]]; Exit[]\n", label);

    mm_code := builder_to_string(*sb);
    result, output, error_str, timed_out := run_command(
        "wolframscript", "-code", mm_code,
        capture_and_return_output = true,
        timeout_ms = 300000);

    if timed_out {
        print("[%] WolframScript timed out after 300s.\n", label);
        return;
    }
    if result.type != .EXITED || result.exit_code != 0 {
        print("[%] WolframScript error: %\n", label, error_str);
        return;
    }
    print("%", output);
}
