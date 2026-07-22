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
        pool: Expr_Pool;
        gR1 := expr_symbol("gR1", *pool);
        gR2 := expr_symbol("gR2", *pool);
        gC1 := expr_symbol("gC1", *pool);
        e := expr_div(expr_mul(gR1, gC1, *pool), expr_mul(gR1, gR2, *pool), *pool);
        simplify(e, *pool);
        check("sym_cancel", e, "(gC1 / gR2)", pool.exprs);
    }

    // 2. a / a  →  1
    {
        pool: Expr_Pool;
        R := expr_symbol("R", *pool);
        e := expr_div(R, R, *pool);
        simplify(e, *pool);
        check("self_cancel", e, "1.0", pool.exprs);
    }

    // 3. (R1*C1*v + R2*C1*v2) / (R1*R2*C1)  →  (v + ... )/ (R1*R2)
    //    i.e. C1 cancels from every numerator term and the denominator
    {
        pool: Expr_Pool;
        R1  := expr_symbol("R1",  *pool);
        R2  := expr_symbol("R2",  *pool);
        C1  := expr_symbol("C1",  *pool);
        v   := expr_symbol("v",   *pool);
        v2  := expr_symbol("v2",  *pool);
        num := expr_add(expr_mul(expr_mul(R1, C1, *pool), v,  *pool),
                        expr_mul(expr_mul(R2, C1, *pool), v2, *pool),
                        *pool);
        den := expr_mul(expr_mul(R1, R2, *pool), C1, *pool);
        e   := expr_div(num, den, *pool);
        simplify(e, *pool);
        print("sym_sum_cancel: %\n", print_expr(e, pool.exprs));
        // Should have no C1 in result
    }

    // 4. Integer GCD: 6 / (4 * R)  →  3 / (2 * R)
    //    (only the CONST-numerator partial-GCD path fires here)
    {
        pool: Expr_Pool;
        R := expr_symbol("R", *pool);
        e := expr_div(expr_const(6, *pool), expr_mul(expr_const(4, *pool), R, *pool), *pool);
        simplify(e, *pool);
        check("int_gcd", e, "(3.0 / (2.0 * R))", pool.exprs);
    }

    // 5. Exact integer cancel: (6*R) / 3  →  (2*R)
    {
        pool: Expr_Pool;
        R := expr_symbol("R", *pool);
        e := expr_div(expr_mul(expr_const(6, *pool), R, *pool), expr_const(3, *pool), *pool);
        simplify(e, *pool);
        check("int_exact", e, "(2.0 * R)", pool.exprs);
    }

    // 6. MUL lift: a * (b/c)  →  (a*b)/c  then cancel if possible
    //    R1 * (R2 / R1)  →  R2
    {
        pool: Expr_Pool;
        R1 := expr_symbol("R1", *pool);
        R2 := expr_symbol("R2", *pool);
        e  := expr_mul(R1, expr_div(R2, R1, *pool), *pool);
        simplify(e, *pool);
        check("mul_lift", e, "R2", pool.exprs);
    }

    // 7. Full solve + simplify: 1x1  gR1*(vo-vi) + gC1*vo = 0  →  vo = (gR1*vi) / (gR1+gC1)
    {
        pool: Expr_Pool;
        vo  := expr_symbol("vo",  *pool);
        vi  := expr_symbol("vi",  *pool);
        gR1 := expr_symbol("gR1", *pool);
        gC1 := expr_symbol("gC1", *pool);
        eq  := expr_add(expr_mul(gR1, expr_sub(vo, vi, *pool), *pool),
                        expr_mul(gC1, vo, *pool), *pool);
        solutions := solve(.[eq], .[vo], *pool);
        sol := solutions[0];
        simplify(sol, *pool);
        print("1x1 solve+simplify: vo = %\n", print_expr(sol, pool.exprs));
    }
}
