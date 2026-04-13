#import "Basic";

Expr_Kind :: enum {
    CONST;
    SYMBOL;
    NEG; // unary; operand is `left`
    ADD;
    SUB;
    MUL;
    DIV;
}

Expr_ID :: s64;

Expr :: struct {
    kind: Expr_Kind;

    // @TODO: we could union some of this stuff?

    value: s64; // For CONST
    name: string; // For SYMBOL

    // For binary ops
    left: Expr_ID;
    right: Expr_ID;
}

expr_symbol :: (name: string, exprs: *[..] Expr) -> Expr_ID {
    id := exprs.count;
    array_add(exprs, Expr.{ kind = .SYMBOL, name = name });
    return id;
}

expr_neg :: (a: Expr_ID, exprs: *[..] Expr) -> Expr_ID {
    id := exprs.count;
    array_add(exprs, Expr.{ kind = .NEG, left = a });
    return id;
}

expr_const :: (value: s64, exprs: *[..] Expr) -> Expr_ID {
    id := exprs.count;
    array_add(exprs, Expr.{ kind = .CONST, value = value });
    return id;
}

expr_bin :: (kind: Expr_Kind, a: Expr_ID, b: Expr_ID, exprs: *[..] Expr) -> Expr_ID {
    id := exprs.count;
    array_add(exprs, Expr.{ kind = kind, left = a, right = b });
    return id;
}

expr_add :: (a: Expr_ID, b: Expr_ID, exprs: *[..] Expr) -> Expr_ID { return expr_bin(.ADD, a, b, exprs); }
expr_sub :: (a: Expr_ID, b: Expr_ID, exprs: *[..] Expr) -> Expr_ID { return expr_bin(.SUB, a, b, exprs); }
expr_mul :: (a: Expr_ID, b: Expr_ID, exprs: *[..] Expr) -> Expr_ID { return expr_bin(.MUL, a, b, exprs); }
expr_div :: (a: Expr_ID, b: Expr_ID, exprs: *[..] Expr) -> Expr_ID { return expr_bin(.DIV, a, b, exprs); }

//==================================================

// Sentinel meaning "zero / absent expression"
NO_EXPR : Expr_ID : -1;

// Represents:  sum_i(coeffs[i] * vars[i])  +  constant
// where coeffs is parallel to the `vars` array passed to linearize().
// NO_EXPR in any slot means that term is structurally zero.
Linear :: struct {
    coeffs:   [..] Expr_ID;
    constant: Expr_ID;
}

linear_is_constant :: (l: Linear) -> bool {
    for l.coeffs  if it != NO_EXPR  return false;
    return true;
}

// Combine two optional expression IDs with ADD or SUB, treating NO_EXPR as zero.
// For SUB with `a` absent: produces (0 - b), i.e. negation.
combine :: (a: Expr_ID, b: Expr_ID, op: Expr_Kind, exprs: *[..] Expr) -> Expr_ID {
    if a == NO_EXPR && b == NO_EXPR  return NO_EXPR;
    if a == NO_EXPR {
        if op == .ADD  return b;
        assert(op == .SUB);
        return expr_neg(b, exprs);
    }
    if b == NO_EXPR  return a; // a +/- 0 = a
    return expr_bin(op, a, b, exprs);
}

// Decompose `e` into a linear combination of `vars`.
// Each entry in the returned Linear.coeffs is parallel to `vars`.
// Asserts if the expression is not linear in the given variables.
linearize :: (e: Expr_ID, vars: [] Expr_ID, exprs: *[..] Expr) -> Linear {
    result: Linear;
    for i: 0..vars.count-1  array_add(*result.coeffs, NO_EXPR);
    result.constant = NO_EXPR;

    // Copy the node before recursing: array_add inside recursive calls may
    // reallocate, but Expr is a value type so this local copy stays valid.
    expr := exprs.*[e];

    if expr.kind == {
        case .CONST;
            result.constant = e;

        case .SYMBOL;
            for i: 0..vars.count-1 {
                if vars[i] == e {
                    result.coeffs[i] = expr_const(1, exprs);
                    return result;
                }
            }
            result.constant = e; // a parameter symbol, not one of the vars we care about

        case .NEG;
            la := linearize(expr.left, vars, exprs);
            for i: 0..vars.count-1
                result.coeffs[i] = combine(NO_EXPR, la.coeffs[i], .SUB, exprs);
            result.constant = combine(NO_EXPR, la.constant, .SUB, exprs);

        case .ADD; #through;
        case .SUB;
            la := linearize(expr.left,  vars, exprs);
            lb := linearize(expr.right, vars, exprs);
            for i: 0..vars.count-1
                result.coeffs[i] = combine(la.coeffs[i], lb.coeffs[i], expr.kind, exprs);
            result.constant = combine(la.constant, lb.constant, expr.kind, exprs);

        case .MUL;
            la := linearize(expr.left,  vars, exprs);
            lb := linearize(expr.right, vars, exprs);
            la_const := linear_is_constant(la);
            lb_const := linear_is_constant(lb);
            assert(la_const || lb_const, "Expression is not linear in the given variables!");
            scalar   := ifx la_const then la.constant else lb.constant;
            var_part := ifx la_const then lb           else la;
            if scalar != NO_EXPR {
                for i: 0..vars.count-1 {
                    if var_part.coeffs[i] != NO_EXPR
                        result.coeffs[i] = expr_mul(scalar, var_part.coeffs[i], exprs);
                }
                if var_part.constant != NO_EXPR
                    result.constant = expr_mul(scalar, var_part.constant, exprs);
            }

        case .DIV;
            la := linearize(expr.left,  vars, exprs);
            lb := linearize(expr.right, vars, exprs);
            assert(linear_is_constant(lb), "Expression is not linear! (non-constant divisor)");
            divisor := lb.constant;
            if divisor != NO_EXPR {
                for i: 0..vars.count-1 {
                    if la.coeffs[i] != NO_EXPR
                        result.coeffs[i] = expr_div(la.coeffs[i], divisor, exprs);
                }
                if la.constant != NO_EXPR
                    result.constant = expr_div(la.constant, divisor, exprs);
            }
    }

    return result;
}

//==================================================
// Fold constants and algebraic identities in place, bottom-up.
// Children are fully folded before the parent is checked, so one pass suffices.
fold :: (e: Expr_ID, exprs: *[..] Expr) {
    expr := exprs.*[e];
    if expr.kind == {
        case .CONST;  return;
        case .SYMBOL; return;

        case .NEG;
            fold(expr.left, exprs);
            child := exprs.*[expr.left];
            if child.kind == .CONST {
                exprs.*[e] = .{ kind = .CONST, value = -child.value }; // -C     → CONST(-C)
            } else if child.kind == .NEG {
                exprs.*[e] = exprs.*[child.left];                       // -(-x)  → x
            } else if child.kind == .ADD {
                na := expr_neg(child.left,  exprs);                     // -(a+b) → (-a) + (-b)
                nb := expr_neg(child.right, exprs);
                fold(na, exprs);
                fold(nb, exprs);
                exprs.*[e] = .{ kind = .ADD, left = na, right = nb };
            } else if child.kind == .SUB {
                exprs.*[e] = .{ kind = .SUB, left = child.right, right = child.left }; // -(a-b) → b-a
            }

        case; // ADD, SUB, MUL, DIV
            fold(expr.left,  exprs);
            fold(expr.right, exprs);
            left  := exprs.*[expr.left];
            right := exprs.*[expr.right];

            // Evaluate arithmetic on two literal constants
            if left.kind == .CONST && right.kind == .CONST {
                result: s64;
                if expr.kind == {
                    case .ADD; result = left.value + right.value;
                    case .SUB; result = left.value - right.value;
                    case .MUL; result = left.value * right.value;
                    case .DIV;
                        if right.value != 0 && left.value % right.value == 0
                            result = left.value / right.value;
                        else return; // not exactly divisible; leave as-is
                }
                exprs.*[e] = .{ kind = .CONST, value = result };
                return;
            }

            if expr.kind == .ADD {
                if left.kind  == .CONST && left.value  == 0 { exprs.*[e] = right; return; } // 0 + x → x
                if right.kind == .CONST && right.value == 0 { exprs.*[e] = left;  return; } // x + 0 → x
            }
            if expr.kind == .SUB {
                if right.kind == .CONST && right.value == 0 { exprs.*[e] = left;  return; } // x - 0 → x
                if left.kind  == .CONST && left.value  == 0 { exprs.*[e] = .{ kind = .NEG, left = expr.right }; return; } // 0 - x → -x
            }
            if expr.kind == .MUL {
                if left.kind  == .CONST && left.value  ==  0 { exprs.*[e] = left;  return; } // 0 * x → 0
                if right.kind == .CONST && right.value ==  0 { exprs.*[e] = right; return; } // x * 0 → 0
                if left.kind  == .CONST && left.value  ==  1 { exprs.*[e] = right; return; } // 1 * x → x
                if right.kind == .CONST && right.value ==  1 { exprs.*[e] = left;  return; } // x * 1 → x
                if left.kind  == .CONST && left.value  == -1 { exprs.*[e] = .{ kind = .NEG, left = expr.right }; return; } // -1 * x → -x
                if right.kind == .CONST && right.value == -1 { exprs.*[e] = .{ kind = .NEG, left = expr.left  }; return; } // x * -1 → -x
                // Pull NEG out of MUL: a*(-b) → -(a*b), (-a)*b → -(a*b)
                // Re-fold afterwards to collapse any resulting NEG(NEG(...))
                if right.kind == .NEG {
                    inner := expr_mul(expr.left, right.left, exprs);
                    fold(inner, exprs);
                    exprs.*[e] = .{ kind = .NEG, left = inner };
                    fold(e, exprs);
                    return;
                }
                if left.kind == .NEG {
                    inner := expr_mul(left.left, expr.right, exprs);
                    fold(inner, exprs);
                    exprs.*[e] = .{ kind = .NEG, left = inner };
                    fold(e, exprs);
                    return;
                }
            }
            if expr.kind == .DIV {
                if right.kind == .CONST && right.value == 1 { exprs.*[e] = left; return; } // x / 1 → x
            }
    }
}

//==================================================
// Symbolically solve a square system of n equations (each equal to zero) for n unknowns,
// using fraction-free Gauss-Jordan elimination.
// Each elimination step uses:  row = row*pivot - pivot_row*factor  (no DIV until the end)
// Returns one solution Expr_ID per unknown, in the same order as solve_for.
solve :: (equations: [] Expr_ID, solve_for: [] Expr_ID, exprs: *[..] Expr) -> [..] Expr_ID {
    n := equations.count;
    assert(n == solve_for.count, "Need one equation per unknown!");

    // Build n x (n+1) augmented matrix [A | rhs], stored row-major.
    //   A[i][j]  = coefficient of solve_for[j] in equation i
    //   rhs[i]   = -constant term  (so the system becomes A*x = rhs)
    cols := n + 1;
    mat: [..] Expr_ID;
    for _: 0..n*cols-1  array_add(*mat, NO_EXPR);

    for i: 0..n-1 {
        lin := linearize(equations[i], solve_for, exprs);
        for j: 0..n-1 {
            mat[i * cols + j] = lin.coeffs[j];
            if mat[i * cols + j] != NO_EXPR  fold(mat[i * cols + j], exprs);
        }
        rhs_i := ifx lin.constant != NO_EXPR then expr_neg(lin.constant, exprs) else NO_EXPR;
        if rhs_i != NO_EXPR  fold(rhs_i, exprs);
        mat[i * cols + n] = rhs_i;
    }

    // Gauss-Jordan elimination (fraction-free)
    for col: 0..n-1 {
        // Find a non-zero pivot in this column at or below the diagonal
        pivot_row := -1;
        for row: col..n-1 {
            if mat[row * cols + col] != NO_EXPR { pivot_row = row; break; }
        }
        assert(pivot_row != -1, "System is singular or underdetermined!");

        // Bring pivot row to the diagonal position
        if pivot_row != col {
            for j: 0..cols-1 {
                tmp                         := mat[col       * cols + j];
                mat[col       * cols + j]    = mat[pivot_row * cols + j];
                mat[pivot_row * cols + j]    = tmp;
            }
        }

        pivot := mat[col * cols + col];

        // Eliminate this column from every other row
        for row: 0..n-1 {
            if row == col  continue;
            factor := mat[row * cols + col];
            if factor == NO_EXPR  continue; // already zero

            for j: 0..cols-1 {
                a  := mat[row * cols + j];
                b  := mat[col * cols + j];
                // new = a*pivot - b*factor  (fraction-free: no division introduced)
                ap := ifx a != NO_EXPR then expr_mul(a, pivot, exprs) else NO_EXPR;
                bf := ifx b != NO_EXPR then expr_mul(b, factor, exprs) else NO_EXPR;
                new_val := combine(ap, bf, .SUB, exprs);
                if new_val != NO_EXPR  fold(new_val, exprs);
                mat[row * cols + j] = new_val;
            }
        }
    }

    // Read off solutions: diag[i] * var[i] = rhs[i]  →  var[i] = rhs[i] / diag[i]
    result: [..] Expr_ID;
    for i: 0..n-1 {
        diag := mat[i * cols + i];
        rhs  := mat[i * cols + n];
        assert(diag != NO_EXPR, "System is singular!");
        sol: Expr_ID;
        if rhs == NO_EXPR {
            sol = expr_const(0, exprs);
        } else {
            sol = expr_div(rhs, diag, exprs);
            fold(sol, exprs);
        }
        array_add(*result, sol);
    }

    return result;
}

//==================================================
print_expr :: (e: Expr_ID, exprs: [] Expr) -> string {
    expr_to_builder :: (builder: *String_Builder, e: Expr_ID, exprs: [] Expr) {
        if exprs[e].kind == {
            case .CONST;
                print_to_builder(builder, "%", exprs[e].value);

            case .SYMBOL;
                append(builder, exprs[e].name);

            case .NEG;
                append(builder, "(-");
                expr_to_builder(builder, exprs[e].left, exprs);
                append(builder, ")");

            case;
                append(builder, "(");
                expr_to_builder(builder, exprs[e].left, exprs);
                if exprs[e].kind == {
                    case .ADD; append(builder, " + ");
                    case .SUB; append(builder, " - ");
                    case .MUL; append(builder, " * ");
                    case .DIV; append(builder, " / ");
                }
                expr_to_builder(builder, exprs[e].right, exprs);
                append(builder, ")");
        }
    }

    builder: String_Builder;
    expr_to_builder(*builder, e, exprs);
    return builder_to_string(*builder);
}

//==================================================
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

        vo  := expr_symbol("vo", *exprs);
        vi  := expr_symbol("vi", *exprs);
        gR1 := expr_symbol("gR1", *exprs);
        gC1 := expr_symbol("gC1", *exprs);
        zC1 := expr_symbol("zC1", *exprs);

        // Overwrite the symbol's slot with the root of the replacement expression.
        // All existing references to the symbol ID automatically see the new value.
        exprs[iR1] = exprs[expr_mul(gR1, expr_sub(vo, vi, *exprs), *exprs)];
        exprs[iC1] = exprs[expr_sub(expr_mul(gC1, vo, *exprs), zC1, *exprs)];
        print("%\n", print_expr(expr, exprs));
    }

    { // test linearize: gR1*(vo - vi) + (gC1*vo - zC1) in terms of {vo, vi}
        exprs: [..] Expr;
        vo  := expr_symbol("vo",  *exprs);
        vi  := expr_symbol("vi",  *exprs);
        gR1 := expr_symbol("gR1", *exprs);
        gC1 := expr_symbol("gC1", *exprs);
        zC1 := expr_symbol("zC1", *exprs);

        term1 := expr_mul(gR1, expr_sub(vo, vi, *exprs), *exprs); // gR1 * (vo - vi)
        term2 := expr_sub(expr_mul(gC1, vo, *exprs), zC1, *exprs); // gC1*vo - zC1
        eq    := expr_add(term1, term2, *exprs);

        // var_names : [] string   = .["vo", "vi"];
        // vars      : [] Expr_ID = .[vo, vi];
        var_names : [] string   = .["vo"];
        vars      : [] Expr_ID = .[vo];
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
