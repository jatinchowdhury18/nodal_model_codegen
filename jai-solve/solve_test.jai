#import "solve";
#import "Basic";
#import "Process";

main :: () {
    { // 1x1: gR1*(vo - vi) + gC1*vo - zC1 = 0  =>  vo = (gR1*vi + zC1) / (gR1 + gC1)
        exprs: [..] Expr;
        vo  := expr_symbol("vo",  *exprs);
        vi  := expr_symbol("vi",  *exprs);
        gR1 := expr_symbol("gR1", *exprs);
        gC1 := expr_symbol("gC1", *exprs);
        zC1 := expr_symbol("zC1", *exprs);
        eq := expr_add(
            expr_mul(gR1, expr_sub(vo, vi, *exprs), *exprs),
            expr_sub(expr_mul(gC1, vo, *exprs), zC1, *exprs),
            *exprs);
        solutions := solve(.[eq], .[vo], *exprs);
        wolfram_verify("1x1", .["vo"], .[eq], solutions, exprs);
    }

    { // 2x2: two-node resistor ladder
        // eq1: gR1*(v1-vi) + gR2*(v1-v2) = 0
        // eq2: gR2*(v2-v1) + gR3*v2 = 0
        exprs: [..] Expr;
        v1  := expr_symbol("v1",  *exprs);
        v2  := expr_symbol("v2",  *exprs);
        vi  := expr_symbol("vi",  *exprs);
        gR1 := expr_symbol("gR1", *exprs);
        gR2 := expr_symbol("gR2", *exprs);
        gR3 := expr_symbol("gR3", *exprs);
        eq1 := expr_add(
            expr_mul(gR1, expr_sub(v1, vi, *exprs), *exprs),
            expr_mul(gR2, expr_sub(v1, v2, *exprs), *exprs),
            *exprs);
        eq2 := expr_add(
            expr_mul(gR2, expr_sub(v2, v1, *exprs), *exprs),
            expr_mul(gR3, v2, *exprs),
            *exprs);
        solutions := solve(.[eq1, eq2], .[v1, v2], *exprs);
        wolfram_verify("2x2", .["v1", "v2"], .[eq1, eq2], solutions, exprs);
    }

    { // 4x4: four-node resistor ladder
        // eq1: gR1*(v1-vi) + gR2*(v1-v2) = 0
        // eq2: gR2*(v2-v1) + gR3*(v2-v3) = 0
        // eq3: gR3*(v3-v2) + gR4*(v3-v4) = 0
        // eq4: gR4*(v4-v3) + gR5*v4      = 0
        exprs: [..] Expr;
        v1  := expr_symbol("v1",  *exprs);
        v2  := expr_symbol("v2",  *exprs);
        v3  := expr_symbol("v3",  *exprs);
        v4  := expr_symbol("v4",  *exprs);
        vi  := expr_symbol("vi",  *exprs);
        gR1 := expr_symbol("gR1", *exprs);
        gR2 := expr_symbol("gR2", *exprs);
        gR3 := expr_symbol("gR3", *exprs);
        gR4 := expr_symbol("gR4", *exprs);
        gR5 := expr_symbol("gR5", *exprs);
        eq1 := expr_add(expr_mul(gR1, expr_sub(v1, vi, *exprs), *exprs), expr_mul(gR2, expr_sub(v1, v2, *exprs), *exprs), *exprs);
        eq2 := expr_add(expr_mul(gR2, expr_sub(v2, v1, *exprs), *exprs), expr_mul(gR3, expr_sub(v2, v3, *exprs), *exprs), *exprs);
        eq3 := expr_add(expr_mul(gR3, expr_sub(v3, v2, *exprs), *exprs), expr_mul(gR4, expr_sub(v3, v4, *exprs), *exprs), *exprs);
        eq4 := expr_add(expr_mul(gR4, expr_sub(v4, v3, *exprs), *exprs), expr_mul(gR5, v4, *exprs), *exprs);
        solutions := solve(.[eq1, eq2, eq3, eq4], .[v1, v2, v3, v4], *exprs);
        wolfram_verify("4x4", .["v1", "v2", "v3", "v4"], .[eq1, eq2, eq3, eq4], solutions, exprs);
    }

    { // 3x3 sparse: opamp buffer (iE appears in only one equation)
        // eq1: gR1*(vi-v1) - gR2*v1 = 0
        // eq2: gR3*(vi-v2) + iE = 0
        // eq3: v2 - v1 = 0
        exprs: [..] Expr;
        iE  := expr_symbol("iE",  *exprs);
        v1  := expr_symbol("v1",  *exprs);
        v2  := expr_symbol("v2",  *exprs);
        vi  := expr_symbol("vi",  *exprs);
        gR1 := expr_symbol("gR1", *exprs);
        gR2 := expr_symbol("gR2", *exprs);
        gR3 := expr_symbol("gR3", *exprs);
        eq1 := expr_sub(expr_mul(gR1, expr_sub(vi, v1, *exprs), *exprs), expr_mul(gR2, v1, *exprs), *exprs);
        eq2 := expr_add(expr_mul(gR3, expr_sub(vi, v2, *exprs), *exprs), iE, *exprs);
        eq3 := expr_sub(v2, v1, *exprs);
        solutions := solve(.[eq1, eq2, eq3], .[iE, v1, v2], *exprs);
        wolfram_verify("3x3", .["iE", "v1", "v2"], .[eq1, eq2, eq3], solutions, exprs);
    }

    { // 10x10: full nodal circuit system
        // Variables: { iE8, vo10, vo, v33, vn9, v36, iE10, iE9, vr45, v37 }
        exprs: [..] Expr;

        // unknowns
        iE8  := expr_symbol("iE8",  *exprs);
        vo10 := expr_symbol("vo10", *exprs);
        vo   := expr_symbol("vo",   *exprs);
        v33  := expr_symbol("v33",  *exprs);
        vn9  := expr_symbol("vn9",  *exprs);
        v36  := expr_symbol("v36",  *exprs);
        iE10 := expr_symbol("iE10", *exprs);
        iE9  := expr_symbol("iE9",  *exprs);
        vr45 := expr_symbol("vr45", *exprs);
        v37  := expr_symbol("v37",  *exprs);

        // parameters
        vi   := expr_symbol("vi",   *exprs);
        gR33 := expr_symbol("gR33", *exprs);
        gR35 := expr_symbol("gR35", *exprs);
        gR37 := expr_symbol("gR37", *exprs);
        gR38 := expr_symbol("gR38", *exprs);
        gR39 := expr_symbol("gR39", *exprs);
        gR40 := expr_symbol("gR40", *exprs);
        gR41 := expr_symbol("gR41", *exprs);
        gR42 := expr_symbol("gR42", *exprs);
        gR43 := expr_symbol("gR43", *exprs);
        gR44 := expr_symbol("gR44", *exprs);
        gR45 := expr_symbol("gR45", *exprs);
        gR47 := expr_symbol("gR47", *exprs);
        gR48 := expr_symbol("gR48", *exprs);
        gR66 := expr_symbol("gR66", *exprs);
        gC16 := expr_symbol("gC16", *exprs);
        gC17 := expr_symbol("gC17", *exprs);
        gC18 := expr_symbol("gC18", *exprs);
        gC19 := expr_symbol("gC19", *exprs);
        gC20 := expr_symbol("gC20", *exprs);
        zC16 := expr_symbol("zC16", *exprs);
        zC17 := expr_symbol("zC17", *exprs);
        zC18 := expr_symbol("zC18", *exprs);
        zC19 := expr_symbol("zC19", *exprs);
        zC20 := expr_symbol("zC20", *exprs);

        zero      := expr_const(0, *exprs);
        v33_m_v37 := expr_sub(v33, v37, *exprs);
        vn9_m_v36 := expr_sub(vn9, v36, *exprs);

        // eq1: (gR33*(vi-0) + gR37*(vo10-0)) - gR35*(0-vo) = 0
        eq1 := expr_sub(
            expr_add(expr_mul(gR33, expr_sub(vi,   zero, *exprs), *exprs),
                     expr_mul(gR37, expr_sub(vo10, zero, *exprs), *exprs), *exprs),
            expr_mul(gR35, expr_sub(zero, vo, *exprs), *exprs), *exprs);

        // eq2: -(gR37*(vo10-0)) - gR66*(vo10-v33) + gR48*(0-vo10) + iE10 = 0
        eq2 := expr_add(
            expr_add(
                expr_sub(expr_neg(expr_mul(gR37, expr_sub(vo10, zero, *exprs), *exprs), *exprs),
                         expr_mul(gR66, expr_sub(vo10, v33, *exprs), *exprs), *exprs),
                expr_mul(gR48, expr_sub(zero, vo10, *exprs), *exprs), *exprs),
            iE10, *exprs);

        // eq3: gR35*(0-vo) - gR40*(vo-v33) + iE8 = 0
        eq3 := expr_add(
            expr_sub(expr_mul(gR35, expr_sub(zero, vo,  *exprs), *exprs),
                     expr_mul(gR40, expr_sub(vo,   v33, *exprs), *exprs), *exprs),
            iE8, *exprs);

        // eq4: gR40*(vo-v33) + gR39*(vi-v33) + gR66*(vo10-v33)
        //      - gR41*(v33-0) - gR38*(v33-0) - (gC16*(v33-v37)-zC16) - (gC17*(v33-v37)-zC17) = 0
        eq4_a := expr_add(expr_mul(gR40, expr_sub(vo,   v33, *exprs), *exprs), expr_mul(gR39, expr_sub(vi,   v33, *exprs), *exprs), *exprs);
        eq4_b := expr_add(eq4_a, expr_mul(gR66, expr_sub(vo10, v33, *exprs), *exprs), *exprs);
        eq4_c := expr_sub(eq4_b, expr_mul(gR41, expr_sub(v33, zero, *exprs), *exprs), *exprs);
        eq4_d := expr_sub(eq4_c, expr_mul(gR38, expr_sub(v33, zero, *exprs), *exprs), *exprs);
        eq4_e := expr_sub(eq4_d, expr_sub(expr_mul(gC16, v33_m_v37, *exprs), zC16, *exprs), *exprs);
        eq4   := expr_sub(eq4_e, expr_sub(expr_mul(gC17, v33_m_v37, *exprs), zC17, *exprs), *exprs);

        // eq5: -(gR44*(vn9-0)) - gR45*(vn9-vr45) = 0
        eq5 := expr_sub(
            expr_neg(expr_mul(gR44, expr_sub(vn9, zero, *exprs), *exprs), *exprs),
            expr_mul(gR45, expr_sub(vn9, vr45, *exprs), *exprs), *exprs);

        // eq6: -(gR47*(v36-0)) + gR43*(vn9-v36) + (gC18*(vn9-v36)-zC18)
        //      + (gC19*(vn9-v36)-zC19) + (gC20*(vr45-v36)-zC20) + iE9 = 0
        eq6_a := expr_add(expr_neg(expr_mul(gR47, expr_sub(v36, zero, *exprs), *exprs), *exprs),
                          expr_mul(gR43, vn9_m_v36, *exprs), *exprs);
        eq6_b := expr_add(eq6_a, expr_sub(expr_mul(gC18, vn9_m_v36, *exprs), zC18, *exprs), *exprs);
        eq6_c := expr_add(eq6_b, expr_sub(expr_mul(gC19, vn9_m_v36, *exprs), zC19, *exprs), *exprs);
        eq6_d := expr_add(eq6_c, expr_sub(expr_mul(gC20, expr_sub(vr45, v36, *exprs), *exprs), zC20, *exprs), *exprs);
        eq6   := expr_add(eq6_d, iE9, *exprs);

        // eq7: gR47*(v36-0) - gR48*(0-vo10) = 0
        eq7 := expr_sub(
            expr_mul(gR47, expr_sub(v36,  zero, *exprs), *exprs),
            expr_mul(gR48, expr_sub(zero, vo10, *exprs), *exprs), *exprs);

        // eq8: -(gR43*(vn9-v36)) - (gC18*(vn9-v36)-zC18) - (gC19*(vn9-v36)-zC19)
        //      + gR42*(v37-vn9) = 0
        eq8_a := expr_neg(expr_mul(gR43, vn9_m_v36, *exprs), *exprs);
        eq8_b := expr_sub(eq8_a, expr_sub(expr_mul(gC18, vn9_m_v36, *exprs), zC18, *exprs), *exprs);
        eq8_c := expr_sub(eq8_b, expr_sub(expr_mul(gC19, vn9_m_v36, *exprs), zC19, *exprs), *exprs);
        eq8   := expr_add(eq8_c, expr_mul(gR42, expr_sub(v37, vn9, *exprs), *exprs), *exprs);

        // eq9: gR45*(vn9-vr45) - (gC20*(vr45-v36)-zC20) = 0
        eq9 := expr_sub(
            expr_mul(gR45, expr_sub(vn9,  vr45, *exprs), *exprs),
            expr_sub(expr_mul(gC20, expr_sub(vr45, v36, *exprs), *exprs), zC20, *exprs), *exprs);

        // eq10: -(gR42*(v37-vn9)) + (gC16*(v33-v37)-zC16) + (gC17*(v33-v37)-zC17) = 0
        eq10_a := expr_neg(expr_mul(gR42, expr_sub(v37, vn9, *exprs), *exprs), *exprs);
        eq10_b := expr_add(eq10_a, expr_sub(expr_mul(gC16, v33_m_v37, *exprs), zC16, *exprs), *exprs);
        eq10   := expr_add(eq10_b, expr_sub(expr_mul(gC17, v33_m_v37, *exprs), zC17, *exprs), *exprs);

        var_names := string.["iE8", "vo10", "vo", "v33", "vn9", "v36", "iE10", "iE9", "vr45", "v37"];
        solve_for := Expr_ID.[iE8, vo10, vo, v33, vn9, v36, iE10, iE9, vr45, v37];
        solutions := solve(.[eq1, eq2, eq3, eq4, eq5, eq6, eq7, eq8, eq9, eq10], solve_for, *exprs);
        wolfram_verify("10x10", var_names,
            .[eq1, eq2, eq3, eq4, eq5, eq6, eq7, eq8, eq9, eq10], solutions, exprs);
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
