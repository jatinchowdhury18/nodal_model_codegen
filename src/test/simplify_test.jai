#import "sym_algebra";
#import "Basic";

check :: (label: string, e: Expr_ID, expected: string, exprs: [] Expr) {
    got := print_expr(e, exprs);
    if got == expected {
        print("PASS  [%]: %\n", label, got);
    } else {
        print("FAIL  [%]: got  %\n       expected  %\n", label, got, expected);
    }
}

main :: () {
    // 1. (gR1 * gC1) / (gR1 * gR2)  →  gC1 / gR2
    {
        exprs: [..] Expr;
        gR1 := expr_symbol("gR1", *exprs);
        gR2 := expr_symbol("gR2", *exprs);
        gC1 := expr_symbol("gC1", *exprs);
        e := expr_div(expr_mul(gR1, gC1, *exprs), expr_mul(gR1, gR2, *exprs), *exprs);
        simplify(e, *exprs);
        check("sym_cancel", e, "(gC1 / gR2)", exprs);
    }

    // 2. a / a  →  1
    {
        exprs: [..] Expr;
        R := expr_symbol("R", *exprs);
        e := expr_div(R, R, *exprs);
        simplify(e, *exprs);
        check("self_cancel", e, "1", exprs);
    }

    // 3. (R1*C1*v + R2*C1*v2) / (R1*R2*C1)  →  (v + ... )/ (R1*R2)
    //    i.e. C1 cancels from every numerator term and the denominator
    {
        exprs: [..] Expr;
        R1  := expr_symbol("R1",  *exprs);
        R2  := expr_symbol("R2",  *exprs);
        C1  := expr_symbol("C1",  *exprs);
        v   := expr_symbol("v",   *exprs);
        v2  := expr_symbol("v2",  *exprs);
        num := expr_add(expr_mul(expr_mul(R1, C1, *exprs), v,  *exprs),
                        expr_mul(expr_mul(R2, C1, *exprs), v2, *exprs),
                        *exprs);
        den := expr_mul(expr_mul(R1, R2, *exprs), C1, *exprs);
        e   := expr_div(num, den, *exprs);
        simplify(e, *exprs);
        print("sym_sum_cancel: %\n", print_expr(e, exprs));
        // Should have no C1 in result
    }

    // 4. Integer GCD: 6 / (4 * R)  →  3 / (2 * R)
    //    (only the CONST-numerator partial-GCD path fires here)
    {
        exprs: [..] Expr;
        R := expr_symbol("R", *exprs);
        e := expr_div(expr_const(6, *exprs), expr_mul(expr_const(4, *exprs), R, *exprs), *exprs);
        simplify(e, *exprs);
        check("int_gcd", e, "(3 / (2 * R))", exprs);
    }

    // 5. Exact integer cancel: (6*R) / 3  →  (2*R)
    {
        exprs: [..] Expr;
        R := expr_symbol("R", *exprs);
        e := expr_div(expr_mul(expr_const(6, *exprs), R, *exprs), expr_const(3, *exprs), *exprs);
        simplify(e, *exprs);
        check("int_exact", e, "(2 * R)", exprs);
    }

    // 6. MUL lift: a * (b/c)  →  (a*b)/c  then cancel if possible
    //    R1 * (R2 / R1)  →  R2
    {
        exprs: [..] Expr;
        R1 := expr_symbol("R1", *exprs);
        R2 := expr_symbol("R2", *exprs);
        e  := expr_mul(R1, expr_div(R2, R1, *exprs), *exprs);
        simplify(e, *exprs);
        check("mul_lift", e, "R2", exprs);
    }

    // 7. Full solve + simplify: 1x1  gR1*(vo-vi) + gC1*vo = 0  →  vo = (gR1*vi) / (gR1+gC1)
    {
        exprs: [..] Expr;
        vo  := expr_symbol("vo",  *exprs);
        vi  := expr_symbol("vi",  *exprs);
        gR1 := expr_symbol("gR1", *exprs);
        gC1 := expr_symbol("gC1", *exprs);
        eq  := expr_add(expr_mul(gR1, expr_sub(vo, vi, *exprs), *exprs),
                        expr_mul(gC1, vo, *exprs), *exprs);
        solutions := solve(.[eq], .[vo], *exprs);
        sol := solutions[0];
        simplify(sol, *exprs);
        print("1x1 solve+simplify: vo = %\n", print_expr(sol, exprs));
    }
}
