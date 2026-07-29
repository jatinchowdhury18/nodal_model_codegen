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
        pool: Expr_Pool;
        x := expr_symbol("x", *pool);
        d := differentiate(x, x, *pool);
        simplify(d, *pool);
        check("d_self", d, "1.0", pool.exprs);
    }

    // 2. d/dx(y) = 0  (differentiating w.r.t. an unrelated symbol)
    {
        pool: Expr_Pool;
        x := expr_symbol("x", *pool);
        y := expr_symbol("y", *pool);
        d := differentiate(y, x, *pool);
        simplify(d, *pool);
        check("d_unrelated", d, "0.0", pool.exprs);
    }

    // 3. d/dx(3*x) = 3   (MUL by a constant)
    {
        pool: Expr_Pool;
        x := expr_symbol("x", *pool);
        e := expr_mul(expr_const(3, *pool), x, *pool);
        d := differentiate(e, x, *pool);
        simplify(d, *pool);
        check("d_const_mul", d, "3.0", pool.exprs);
    }

    // 4. Product rule: d/dx(x*x) = x + x
    {
        pool: Expr_Pool;
        x := expr_symbol("x", *pool);
        e := expr_mul(x, x, *pool);
        d := differentiate(e, x, *pool);
        simplify(d, *pool);
        check("d_product_self", d, "(x + x)", pool.exprs);
    }

    // 5. Quotient rule: d/dx(x/c) = 1/c  (c a symbol unrelated to x)
    {
        pool: Expr_Pool;
        x := expr_symbol("x", *pool);
        c := expr_symbol("c", *pool);
        e := expr_div(x, c, *pool);
        d := differentiate(e, x, *pool);
        simplify(d, *pool);
        check("d_quotient", d, "(1.0 / c)", pool.exprs);
    }

    // 6. Chain rule: d/dv(exp(v/vt)) simplifies to exp(v/vt) / vt
    //    This is the building block for diode (Shockley) and BJT
    //    (Ebers-Moll) Jacobian entries -- printed (not string-`check`ed)
    //    since the exact CSE-free textual form is secondary to eyeballing
    //    that it's algebraically sane.
    {
        pool: Expr_Pool;
        v  := expr_symbol("v",  *pool);
        vt := expr_symbol("vt", *pool);
        e  := expr_exp(expr_div(v, vt, *pool), *pool);
        d  := differentiate(e, v, *pool);
        simplify(d, *pool);
        print("d_exp_chain: %\n", print_expr(d, pool.exprs));
    }

    // 7. Antiparallel diode-pair I-V law: i = 2*Is*sinh(v/vt), built as
    //    Is*(exp(v/vt) - 1/exp(v/vt)) so exp(v/vt) is shared via CSE.
    //    d(i)/dv should be algebraically 2*Is*cosh(v/vt)/vt, i.e.
    //    Is*(exp(v/vt) + 1/exp(v/vt))/vt after simplification.
    {
        pool: Expr_Pool;
        v   := expr_symbol("v",   *pool);
        vt  := expr_symbol("vt",  *pool);
        is_ := expr_symbol("Is",  *pool);
        e_v_vt := expr_exp(expr_div(v, vt, *pool), *pool);
        inv_e   := expr_div(expr_const(1, *pool), e_v_vt, *pool);
        i_expr  := expr_mul(is_, expr_sub(e_v_vt, inv_e, *pool), *pool);
        d := differentiate(i_expr, v, *pool);
        simplify(d, *pool);
        print("d_diode_pair: %\n", print_expr(d, pool.exprs));
    }

    // 8. Ebers-Moll BJT collector current, differentiated w.r.t. both
    //    junction voltages -- exercises product rule + chain rule together
    //    with two independent EXP subexpressions, same shape as the
    //    fdBCdvBC / fdBCdvBE entries in korg-pedal-modelling's big_muff.cpp.
    //    iC = Is*((exp(vBE/vt) - exp(vBC/vt)) - (exp(vBC/vt)-1)/BetaR)
    {
        pool: Expr_Pool;
        vBE   := expr_symbol("vBE",   *pool);
        vBC   := expr_symbol("vBC",   *pool);
        vt    := expr_symbol("vt",    *pool);
        is_   := expr_symbol("Is",    *pool);
        betar := expr_symbol("BetaR", *pool);

        exp_be := expr_exp(expr_div(vBE, vt, *pool), *pool);
        exp_bc := expr_exp(expr_div(vBC, vt, *pool), *pool);
        iC := expr_mul(is_,
                  expr_sub(
                      expr_sub(exp_be, exp_bc, *pool),
                      expr_div(expr_sub(exp_bc, expr_const(1, *pool), *pool), betar, *pool),
                      *pool),
                  *pool);

        d_bc := differentiate(iC, vBC, *pool);
        simplify(d_bc, *pool);
        print("diC_dvBC: %\n", print_expr(d_bc, pool.exprs));

        d_be := differentiate(iC, vBE, *pool);
        simplify(d_be, *pool);
        print("diC_dvBE: %\n", print_expr(d_be, pool.exprs));
    }

    // 9. LOG chain rule: d/dx(log(x/c)) = 1/x  (c a symbol unrelated to x)
    {
        pool: Expr_Pool;
        x := expr_symbol("x", *pool);
        c := expr_symbol("c", *pool);
        e := expr_log(expr_div(x, c, *pool), *pool);
        d := differentiate(e, x, *pool);
        simplify(d, *pool);
        check("d_log_chain", d, "(1.0 / x)", pool.exprs);
    }

    // 10. POW with a constant exponent (the common case -- e.g. the
    //     Dempwolff triode's gamma/xi): d/dx(x^n) = n*x^(n-1), via the
    //     numerically-stable simple power rule, NOT the general
    //     u^v*(v'*log(u)+v*u'/u) formula (which would divide by x -- see the
    //     comment on differentiate()'s POW case for why that's a real
    //     problem, not just an inefficiency, when x can be exactly 0).
    {
        pool: Expr_Pool;
        x := expr_symbol("x", *pool);
        n := expr_symbol("n", *pool);
        e := expr_pow(x, n, *pool);
        d := differentiate(e, x, *pool);
        simplify(d, *pool);
        check("d_pow_const_exponent", d, "(n * pow(x, (n - 1.0)))", pool.exprs);
    }

    // 11. POW with a variable exponent (general fallback -- not used by any
    //     device model in this codebase today, but the code path exists and
    //     should stay correct): d/dx(x^x) = x^x*(log(x)+1).
    {
        pool: Expr_Pool;
        x := expr_symbol("x", *pool);
        e := expr_pow(x, x, *pool);
        d := differentiate(e, x, *pool);
        simplify(d, *pool);
        print("d_pow_var_exponent: %\n", print_expr(d, pool.exprs));
    }

    // 12. Dempwolff triode Hk shape: d/dv(log(1+exp(c*v))^gamma), the actual
    //     building block of the plate/grid-current Jacobian -- printed (not
    //     string-checked, same reasoning as case 6) to eyeball that it's
    //     algebraically sane and, critically, does NOT contain a bare
    //     "/ log(...)" or "/ Hk" division that would underflow to 0/0 deep
    //     in cutoff (see case 10's comment).
    {
        pool: Expr_Pool;
        v     := expr_symbol("v",     *pool);
        c     := expr_symbol("c",     *pool);
        gamma := expr_symbol("gamma", *pool);
        hk := expr_log(expr_add(expr_const(1, *pool), expr_exp(expr_mul(c, v, *pool), *pool), *pool), *pool);
        ik := expr_pow(hk, gamma, *pool);
        d  := differentiate(ik, v, *pool);
        simplify(d, *pool);
        print("d_triode_Hk_pow_gamma: %\n", print_expr(d, pool.exprs));
    }
}
