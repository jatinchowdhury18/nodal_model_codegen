#import "solve";
#import "Basic";

main :: () {
    { // test printing
        exprs: [..] Expr;
        vo  := expr_symbol("vo", *exprs);
        vi  := expr_symbol("vi", *exprs);
        gR1 := expr_symbol("gR1", *exprs);
        sub := expr_sub(vo, vi, *exprs);        // (vo - vi)
        expr := expr_mul(gR1, sub, *exprs);     // gR1 * (vo - vi)
        print("%\n", print_expr(expr, exprs));

        // gR1*(vo - vi) + (gC1*vo - zC1)
        gC1 := expr_symbol("gC1", *exprs);
        zC1 := expr_symbol("zC1", *exprs);
        term1 := expr_mul(gR1, expr_sub(vo, vi, *exprs), *exprs);
        term2 := expr_sub(expr_mul(gC1, vo, *exprs), zC1, *exprs);
        eq := expr_add(term1, term2, *exprs);
        print("%\n", print_expr(eq, exprs));
    }

    { // test substitution
        exprs: [..] Expr;
        iR1 := expr_symbol("iR1", *exprs);
        iC1 := expr_symbol("iC1", *exprs);
        expr := expr_add(iR1, iC1, *exprs);
        print("%\n", print_expr(expr, exprs));

        // Overwrite the symbol's slot with the root of the replacement expression.
        // All existing references to the symbol ID automatically see the new value.
        exprs[iR1] = exprs[expr_mul("gR1", expr_sub("vo", "vi", *exprs), *exprs)];
        exprs[iC1] = exprs[expr_sub(expr_mul("gC1", "vo", *exprs), "zC1", *exprs)];
        print("%\n", print_expr(expr, exprs));
    }

    { // test linearize: gR1*(vo - vi) + (gC1*vo - zC1) in terms of {vo, vi}
        exprs: [..] Expr;
        term1 := expr_mul("gR1", expr_sub("vo", "vi", *exprs), *exprs); // gR1 * (vo - vi)
        term2 := expr_sub(expr_mul("gC1", "vo", *exprs), "zC1", *exprs); // gC1*vo - zC1
        eq    := expr_add(term1, term2, *exprs);

        var_names : [] string   = .["vo"];
        vars      : [] Expr_ID = .[expr_symbol("vo", *exprs)];
        lin := linearize(eq, vars, *exprs);

        for i: 0..vars.count-1
            if lin.coeffs[i] != NO_EXPR  fold(lin.coeffs[i], *exprs);
        if lin.constant != NO_EXPR  fold(lin.constant, *exprs);

        for i: 0..vars.count-1 {
            coeff_str := ifx lin.coeffs[i] != NO_EXPR then print_expr(lin.coeffs[i], exprs) else "0";
            print("coeff(%) = %\n", var_names[i], coeff_str);
        }
        const_str := ifx lin.constant != NO_EXPR then print_expr(lin.constant, exprs) else "0";
        print("constant  = %\n", const_str);
    }

    { // 1x1: gR1*(vo - vi) + gC1*vo - zC1 = 0, solve for vo
        // Expected: vo = (gR1*vi + zC1) / (gR1 + gC1)
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
        print("vo = %\n", print_expr(solutions[0], exprs));
    }

    { // 2x2: two-node resistor ladder
        // Eq 1: gR1*(v1 - vi) + gR2*(v1 - v2) = 0
        // Eq 2: gR2*(v2 - v1) + gR3*v2        = 0
        // Expected: v1 = gR1*vi*(gR2+gR3) / det
        //           v2 = gR1*gR2*vi      / det
        //   where det = (gR1+gR2)*(gR2+gR3) - gR2^2
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
        print("v1 = %\n", print_expr(solutions[0], exprs));
        print("v2 = %\n", print_expr(solutions[1], exprs));
    }

    { // 4x4: four-node resistor ladder
        // Eq 1: gR1*(v1-vi) + gR2*(v1-v2) = 0
        // Eq 2: gR2*(v2-v1) + gR3*(v2-v3) = 0
        // Eq 3: gR3*(v3-v2) + gR4*(v3-v4) = 0
        // Eq 4: gR4*(v4-v3) + gR5*v4      = 0
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

        eq1 := expr_add(
            expr_mul(gR1, expr_sub(v1, vi, *exprs), *exprs),
            expr_mul(gR2, expr_sub(v1, v2, *exprs), *exprs),
            *exprs);

        eq2 := expr_add(
            expr_mul(gR2, expr_sub(v2, v1, *exprs), *exprs),
            expr_mul(gR3, expr_sub(v2, v3, *exprs), *exprs),
            *exprs);

        eq3 := expr_add(
            expr_mul(gR3, expr_sub(v3, v2, *exprs), *exprs),
            expr_mul(gR4, expr_sub(v3, v4, *exprs), *exprs),
            *exprs);

        eq4 := expr_add(
            expr_mul(gR4, expr_sub(v4, v3, *exprs), *exprs),
            expr_mul(gR5, v4, *exprs),
            *exprs);

        solutions := solve(.[eq1, eq2, eq3, eq4], .[v1, v2, v3, v4], *exprs);
        print("v1 = %\n", print_expr(solutions[0], exprs));
        print("v2 = %\n", print_expr(solutions[1], exprs));
        print("v3 = %\n", print_expr(solutions[2], exprs));
        print("v4 = %\n", print_expr(solutions[3], exprs));
    }
}

// eqn = ({ 0 == iR1 + iC1 (*vo*) } /. { iR1 -> gR1 * (vo - vi), iC1 -> gC1 * (vo - 0) - zC1 } /. {  })
// slnp=Solve[eqn, {vo}];
// voexpr = slnp[[1]][[1]][[2]] // FullSimplify;
// tC1 = gC1 * (voexpr - 0);

// oex = Experimental`OptimizeExpression[{voexpr, tC1},
//    "OptimizationLevel" -> 2, "OptimizationSymbol" -> temp];
// Print[Total[Cases[oex, a_Times :> Length[a], Infinity]]]
// Print[CForm[oex] /. {Power[x_, 2] :> x*x, Power[x_, -1] :> 1/x}];
