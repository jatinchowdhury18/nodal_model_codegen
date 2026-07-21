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
    // 1. d/dx(x) = 1
    {
        exprs: [..] Expr;
        x := expr_symbol("x", *exprs);
        d := differentiate(x, x, *exprs);
        simplify(d, *exprs);
        check("d_self", d, "1.0", exprs);
    }

    // 2. d/dx(y) = 0  (differentiating w.r.t. an unrelated symbol)
    {
        exprs: [..] Expr;
        x := expr_symbol("x", *exprs);
        y := expr_symbol("y", *exprs);
        d := differentiate(y, x, *exprs);
        simplify(d, *exprs);
        check("d_unrelated", d, "0.0", exprs);
    }

    // 3. d/dx(3*x) = 3   (MUL by a constant)
    {
        exprs: [..] Expr;
        x := expr_symbol("x", *exprs);
        e := expr_mul(expr_const(3, *exprs), x, *exprs);
        d := differentiate(e, x, *exprs);
        simplify(d, *exprs);
        check("d_const_mul", d, "3.0", exprs);
    }

    // 4. Product rule: d/dx(x*x) = x + x
    {
        exprs: [..] Expr;
        x := expr_symbol("x", *exprs);
        e := expr_mul(x, x, *exprs);
        d := differentiate(e, x, *exprs);
        simplify(d, *exprs);
        check("d_product_self", d, "(x + x)", exprs);
    }

    // 5. Quotient rule: d/dx(x/c) = 1/c  (c a symbol unrelated to x)
    {
        exprs: [..] Expr;
        x := expr_symbol("x", *exprs);
        c := expr_symbol("c", *exprs);
        e := expr_div(x, c, *exprs);
        d := differentiate(e, x, *exprs);
        simplify(d, *exprs);
        check("d_quotient", d, "(1.0 / c)", exprs);
    }

    // 6. Chain rule: d/dv(exp(v/vt)) simplifies to exp(v/vt) / vt
    //    This is the building block for diode (Shockley) and BJT
    //    (Ebers-Moll) Jacobian entries -- printed (not string-`check`ed)
    //    since the exact CSE-free textual form is secondary to eyeballing
    //    that it's algebraically sane.
    {
        exprs: [..] Expr;
        v  := expr_symbol("v",  *exprs);
        vt := expr_symbol("vt", *exprs);
        e  := expr_exp(expr_div(v, vt, *exprs), *exprs);
        d  := differentiate(e, v, *exprs);
        simplify(d, *exprs);
        print("d_exp_chain: %\n", print_expr(d, exprs));
    }

    // 7. Antiparallel diode-pair I-V law: i = 2*Is*sinh(v/vt), built as
    //    Is*(exp(v/vt) - 1/exp(v/vt)) so exp(v/vt) is shared via CSE.
    //    d(i)/dv should be algebraically 2*Is*cosh(v/vt)/vt, i.e.
    //    Is*(exp(v/vt) + 1/exp(v/vt))/vt after simplification.
    {
        exprs: [..] Expr;
        v   := expr_symbol("v",   *exprs);
        vt  := expr_symbol("vt",  *exprs);
        is_ := expr_symbol("Is",  *exprs);
        e_v_vt := expr_exp(expr_div(v, vt, *exprs), *exprs);
        inv_e   := expr_div(expr_const(1, *exprs), e_v_vt, *exprs);
        i_expr  := expr_mul(is_, expr_sub(e_v_vt, inv_e, *exprs), *exprs);
        d := differentiate(i_expr, v, *exprs);
        simplify(d, *exprs);
        print("d_diode_pair: %\n", print_expr(d, exprs));
    }

    // 8. Ebers-Moll BJT collector current, differentiated w.r.t. both
    //    junction voltages -- exercises product rule + chain rule together
    //    with two independent EXP subexpressions, same shape as the
    //    fdBCdvBC / fdBCdvBE entries in korg-pedal-modelling's big_muff.cpp.
    //    iC = Is*((exp(vBE/vt) - exp(vBC/vt)) - (exp(vBC/vt)-1)/BetaR)
    {
        exprs: [..] Expr;
        vBE   := expr_symbol("vBE",   *exprs);
        vBC   := expr_symbol("vBC",   *exprs);
        vt    := expr_symbol("vt",    *exprs);
        is_   := expr_symbol("Is",    *exprs);
        betar := expr_symbol("BetaR", *exprs);

        exp_be := expr_exp(expr_div(vBE, vt, *exprs), *exprs);
        exp_bc := expr_exp(expr_div(vBC, vt, *exprs), *exprs);
        iC := expr_mul(is_,
                  expr_sub(
                      expr_sub(exp_be, exp_bc, *exprs),
                      expr_div(expr_sub(exp_bc, expr_const(1, *exprs), *exprs), betar, *exprs),
                      *exprs),
                  *exprs);

        d_bc := differentiate(iC, vBC, *exprs);
        simplify(d_bc, *exprs);
        print("diC_dvBC: %\n", print_expr(d_bc, exprs));

        d_be := differentiate(iC, vBE, *exprs);
        simplify(d_be, *exprs);
        print("diC_dvBE: %\n", print_expr(d_be, exprs));
    }
}
