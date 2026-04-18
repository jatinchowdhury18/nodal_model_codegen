#import "Basic";

Expr_Kind :: enum {
    NULL;
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

expr_symbol :: (id: Expr_ID, exprs: *[..] Expr) -> Expr_ID { return id; }
expr_symbol :: (name: string, exprs: *[..] Expr) -> Expr_ID {
    // @TODO: Is this slow?
    for expr, id : exprs.* {
        if expr.name == name return id;
    }

    id := exprs.count;
    array_add(exprs, Expr.{ kind = .SYMBOL, name = name });
    return id;
}

expr_neg :: (a: Expr_ID, exprs: *[..] Expr) -> Expr_ID {
    id := exprs.count;
    array_add(exprs, Expr.{ kind = .NEG, left = a });
    return id;
}

expr_neg :: (a: string, exprs: *[..] Expr) -> Expr_ID {
    return expr_neg(expr_symbol(a), exprs);
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

expr_add :: (a: $T1, b: $T2, exprs: *[..] Expr) -> Expr_ID { return expr_bin(.ADD, expr_symbol(a, exprs), expr_symbol(b, exprs), exprs); }
expr_sub :: (a: $T1, b: $T2, exprs: *[..] Expr) -> Expr_ID { return expr_bin(.SUB, expr_symbol(a, exprs), expr_symbol(b, exprs), exprs); }
expr_mul :: (a: $T1, b: $T2, exprs: *[..] Expr) -> Expr_ID { return expr_bin(.MUL, expr_symbol(a, exprs), expr_symbol(b, exprs), exprs); }
expr_div :: (a: $T1, b: $T2, exprs: *[..] Expr) -> Expr_ID { return expr_bin(.DIV, expr_symbol(a, exprs), expr_symbol(b, exprs), exprs); }

substitute :: (old_expr: Expr_ID, new_expr: Expr_ID, exprs: *[..] Expr) -> Expr_ID {
    if exprs.*[new_expr].kind == .SYMBOL {
        exprs.*[old_expr] = .{ kind = .NULL };
        for *expr : exprs.* {
            if expr.left == old_expr  expr.left = new_expr;
            if expr.right == old_expr expr.right = new_expr;
        }
        return new_expr;
    }

    exprs.*[old_expr] = exprs.*[new_expr];
    return new_expr;
}

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
    print("Solving %1x%1 equation\n", n);

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
simplify :: (e: Expr_ID, exprs: *[..] Expr) -> Expr_ID {
    // @TODO: better stuff here...
    fold(e, exprs);
    return e;
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
