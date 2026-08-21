// sym_algebra: symbolic computer algebra for Jai. Build, simplify, and
// solve expressions, then generate code to evaluate them.
//
// Expressions live in an Expr_Pool (a flat arena + name->id interning
// table), referenced by Expr_ID (an index into pool.exprs). NO_EXPR (-1)
// means "absent/zero". Build with expr_symbol/expr_const/expr_add/.../
// expr_neg; substitute() rewrites a variable in place.
//
// fold() constant-folds and collapses algebraic identities (0+x->x, 1*x->x).
// simplify() cancels common MUL/DIV factors; call fold() first.
//
// linearize()/solve() are the linear MNA solver, for linear expressions only.
// EXP and differentiate() exist to build/solve small nonlinear systems
// (Newton-Raphson for diode/BJT equations) on top of that linear core.
// See nonlinear_solve.jai.
//
// cse()/licm()/recip_hoist() are codegen-prep passes: common subexpression
// elimination, loop-invariant code motion, and reciprocal hoisting (turning
// repeated inner-loop divisions by a hoisted temp into multiplies). Run in
// that order: cse(), then sort_cse_topological(), then licm(), then
// recip_hoist(), before print_expr().

#import "Basic";
#import "Sort";
#import "Hash_Table";
Hash :: #import "Hash";
Math :: #import "Math";

Expr_Kind :: enum {
    NULL;
    CONST;
    SYMBOL;
    NEG; // unary; operand is `left`
    ADD;
    SUB;
    MUL;
    DIV;
    EXP; // unary; operand is `left`. exp(left)
    LOG; // unary; operand is `left`. natural log(left)
    POW; // binary. left^right (right need not be a literal/integer; see differentiate())
}

// Kinds whose node has a single operand (in `left`), rather than the usual
// binary `left`/`right` pair. Tree-walkers that generically recurse over
// `left`/`right` need to check this to avoid reading a bogus `right` child.
is_unary :: (kind: Expr_Kind) -> bool {
    return kind == .NEG || kind == .EXP || kind == .LOG;
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

// The expression arena, plus a name -> id interning table so expr_symbol()
// doesn't have to linear-scan the whole arena on every lookup.
Expr_Pool :: struct {
    exprs:      [..] Expr;
    symbol_ids: Table(string, Expr_ID);
}

expr_symbol :: (id: Expr_ID, pool: *Expr_Pool) -> Expr_ID { return id; }
expr_symbol :: (name: string, pool: *Expr_Pool) -> Expr_ID {
    found, existing_id := table_find(*pool.symbol_ids, name);
    if found  return existing_id;

    id := pool.exprs.count;
    array_add(*pool.exprs, Expr.{ kind = .SYMBOL, name = name });
    table_add(*pool.symbol_ids, name, id);
    return id;
}

expr_neg :: (a: Expr_ID, pool: *Expr_Pool) -> Expr_ID {
    id := pool.exprs.count;
    array_add(*pool.exprs, Expr.{ kind = .NEG, left = a });
    return id;
}

expr_exp :: (a: Expr_ID, pool: *Expr_Pool) -> Expr_ID {
    id := pool.exprs.count;
    array_add(*pool.exprs, Expr.{ kind = .EXP, left = a });
    return id;
}

expr_log :: (a: Expr_ID, pool: *Expr_Pool) -> Expr_ID {
    id := pool.exprs.count;
    array_add(*pool.exprs, Expr.{ kind = .LOG, left = a });
    return id;
}

expr_pow :: (a: $T1, b: $T2, pool: *Expr_Pool) -> Expr_ID { return expr_bin(.POW, expr_symbol(a, pool), expr_symbol(b, pool), pool); }

// Rebuild a unary (NEG/EXP) node with the given operand. Used by passes
// (cse_pass, licm_walk) that hoist a node into a fresh temporary and need
// to reconstruct it generically, without hardcoding NEG.
make_unary :: (kind: Expr_Kind, a: Expr_ID, pool: *Expr_Pool) -> Expr_ID {
    id := pool.exprs.count;
    array_add(*pool.exprs, Expr.{ kind = kind, left = a });
    return id;
}

// Deep-copy a subtree into fresh Expr slots, so later in-place mutation
// (fold/simplify/cse) of the copy can't corrupt a shared original. This is
// needed whenever the same Expr_ID gets handed to more than one independent
// consumer (e.g. a device residual and a circuit output reading the same
// node-voltage subtree).
//
// SYMBOL leaves are returned as-is, not copied. They're already interned
// by name, and sharing them is what lets CSE/print_expr recognize repeated
// uses of the same symbol.
copy_expr :: (e: Expr_ID, pool: *Expr_Pool) -> Expr_ID {
    ex := pool.exprs[e];
    if ex.kind == .SYMBOL || ex.kind == .NULL  return e;
    if ex.kind == .CONST  return expr_const(ex.value, pool);
    if is_unary(ex.kind)  return make_unary(ex.kind, copy_expr(ex.left, pool), pool);
    return expr_bin(ex.kind, copy_expr(ex.left, pool), copy_expr(ex.right, pool), pool);
}

expr_const :: (value: s64, pool: *Expr_Pool) -> Expr_ID {
    id := pool.exprs.count;
    array_add(*pool.exprs, Expr.{ kind = .CONST, value = value });
    return id;
}

expr_bin :: (kind: Expr_Kind, a: Expr_ID, b: Expr_ID, pool: *Expr_Pool) -> Expr_ID {
    id := pool.exprs.count;
    array_add(*pool.exprs, Expr.{ kind = kind, left = a, right = b });
    return id;
}

expr_add :: (a: $T1, b: $T2, pool: *Expr_Pool) -> Expr_ID { return expr_bin(.ADD, expr_symbol(a, pool), expr_symbol(b, pool), pool); }
expr_sub :: (a: $T1, b: $T2, pool: *Expr_Pool) -> Expr_ID { return expr_bin(.SUB, expr_symbol(a, pool), expr_symbol(b, pool), pool); }
expr_mul :: (a: $T1, b: $T2, pool: *Expr_Pool) -> Expr_ID { return expr_bin(.MUL, expr_symbol(a, pool), expr_symbol(b, pool), pool); }
expr_div :: (a: $T1, b: $T2, pool: *Expr_Pool) -> Expr_ID { return expr_bin(.DIV, expr_symbol(a, pool), expr_symbol(b, pool), pool); }

substitute :: (old_expr: Expr_ID, new_expr: Expr_ID, pool: *Expr_Pool) -> Expr_ID {
    if pool.exprs[new_expr].kind == .SYMBOL {
        pool.exprs[old_expr] = .{ kind = .NULL };
        for *expr : pool.exprs {
            if expr.left == old_expr  expr.left = new_expr;
            if expr.right == old_expr expr.right = new_expr;
        }
        return new_expr;
    }

    pool.exprs[old_expr] = pool.exprs[new_expr];
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

// Does `e`'s tree reference any of `vars`? Lets linearize() treat a
// sub-expression (even a transcendental one, like a Jacobian entry with EXP
// in it) as an opaque constant coefficient, as long as it doesn't depend on
// the variables being solved for.
expr_depends_on :: (e: Expr_ID, vars: [] Expr_ID, exprs: [] Expr) -> bool {
    for v : vars  if v == e  return true;
    ex := exprs[e];
    if ex.kind == .CONST || ex.kind == .SYMBOL || ex.kind == .NULL  return false;
    if is_unary(ex.kind)  return expr_depends_on(ex.left, vars, exprs);
    return expr_depends_on(ex.left, vars, exprs) || expr_depends_on(ex.right, vars, exprs);
}

// Combine two optional expression IDs with ADD or SUB, treating NO_EXPR as zero.
// For SUB with `a` absent: produces (0 - b), i.e. negation.
combine :: (a: Expr_ID, b: Expr_ID, op: Expr_Kind, pool: *Expr_Pool) -> Expr_ID {
    if a == NO_EXPR && b == NO_EXPR  return NO_EXPR;
    if a == NO_EXPR {
        if op == .ADD  return b;
        assert(op == .SUB);
        return expr_neg(b, pool);
    }
    if b == NO_EXPR  return a; // a +/- 0 = a
    return expr_bin(op, a, b, pool);
}

// Decompose `e` into a linear combination of `vars`.
// Each entry in the returned Linear.coeffs is parallel to `vars`.
// Asserts if the expression is not linear in the given variables.
linearize :: (e: Expr_ID, vars: [] Expr_ID, pool: *Expr_Pool) -> Linear {
    result: Linear;
    result.coeffs.allocator = temp;
    array_reserve(*result.coeffs, vars.count);
    for i: 0..vars.count-1  array_add(*result.coeffs, NO_EXPR);
    result.constant = NO_EXPR;

    // If this subtree doesn't reference any of `vars` at all, treat it as an
    // opaque constant coefficient without recursing into its structure.
    if !expr_depends_on(e, vars, pool.exprs) {
        result.constant = e;
        return result;
    }

    // Copy the node before recursing: array_add inside recursive calls may
    // reallocate, but Expr is a value type so this local copy stays valid.
    expr := pool.exprs[e];

    if expr.kind == {
        case .CONST;
            result.constant = e;

        case .SYMBOL;
            for i: 0..vars.count-1 {
                if vars[i] == e {
                    result.coeffs[i] = expr_const(1, pool);
                    return result;
                }
            }
            result.constant = e; // a parameter symbol, not one of the vars we care about

        case .NEG;
            la := linearize(expr.left, vars, pool);
            for i: 0..vars.count-1
                result.coeffs[i] = combine(NO_EXPR, la.coeffs[i], .SUB, pool);
            result.constant = combine(NO_EXPR, la.constant, .SUB, pool);

        case .ADD; #through;
        case .SUB;
            la := linearize(expr.left,  vars, pool);
            lb := linearize(expr.right, vars, pool);
            for i: 0..vars.count-1
                result.coeffs[i] = combine(la.coeffs[i], lb.coeffs[i], expr.kind, pool);
            result.constant = combine(la.constant, lb.constant, expr.kind, pool);

        case .MUL;
            la := linearize(expr.left,  vars, pool);
            lb := linearize(expr.right, vars, pool);
            la_const := linear_is_constant(la);
            lb_const := linear_is_constant(lb);
            assert(la_const || lb_const, "Expression is not linear in the given variables!");
            scalar   := ifx la_const then la.constant else lb.constant;
            var_part := ifx la_const then lb           else la;
            if scalar != NO_EXPR {
                for i: 0..vars.count-1 {
                    if var_part.coeffs[i] != NO_EXPR
                        result.coeffs[i] = expr_mul(scalar, var_part.coeffs[i], pool);
                }
                if var_part.constant != NO_EXPR
                    result.constant = expr_mul(scalar, var_part.constant, pool);
            }

        case .DIV;
            la := linearize(expr.left,  vars, pool);
            lb := linearize(expr.right, vars, pool);
            assert(linear_is_constant(lb), "Expression is not linear! (non-constant divisor)");
            divisor := lb.constant;
            if divisor != NO_EXPR {
                for i: 0..vars.count-1 {
                    if la.coeffs[i] != NO_EXPR
                        result.coeffs[i] = expr_div(la.coeffs[i], divisor, pool);
                }
                if la.constant != NO_EXPR
                    result.constant = expr_div(la.constant, divisor, pool);
            }

        case .EXP;
            // EXP is transcendental, never linear. Nonlinear device I-V laws
            // must be kept out of the KCL system that gets linearize()'d/solve()'d.
            assert(false, "Expression is not linear! (contains EXP; nonlinear device equations must not reach the linear MNA solve)");

        case .LOG;
            assert(false, "Expression is not linear! (contains LOG; nonlinear device equations must not reach the linear MNA solve)");

        case .POW;
            assert(false, "Expression is not linear! (contains POW; nonlinear device equations must not reach the linear MNA solve)");
    }

    return result;
}

// ===========================================================================
// Symbolic differentiation: differentiate(e, var, *pool) -> d(e)/d(var).
// `var` must come from the same expr_symbol(name, *pool) call used in `e`.
// Used to build the Newton-Raphson Jacobian (see nonlinear_solve.jai).
//
// Result is unsimplified; run simplify() after. fold()'s identities
// (0+x->x, 0*x->0, 1*x->x) clean up most of the 0/1 clutter.
// ===========================================================================
differentiate :: (e: Expr_ID, var: Expr_ID, pool: *Expr_Pool) -> Expr_ID {
    expr := pool.exprs[e]; // Copy the node before recursing.

    if expr.kind == {
        case .CONST;
            return expr_const(0, pool);

        case .SYMBOL;
            // Compare by name, not by raw ID: fold()'s `-(-x) -> x` rule (and
            // others) can leave a second node with the same name but a
            // different ID floating around.
            if expr.name == pool.exprs[var].name  return expr_const(1, pool);   // d/dvar(var) = 1
            return expr_const(0, pool);                                      // d/dvar(other symbol) = 0

        case .NEG;
            return expr_neg(differentiate(expr.left, var, pool), pool);

        case .ADD;
            return expr_add(differentiate(expr.left, var, pool),
                             differentiate(expr.right, var, pool), pool);

        case .SUB;
            return expr_sub(differentiate(expr.left, var, pool),
                             differentiate(expr.right, var, pool), pool);

        case .MUL;
            // Product rule: d(u*v) = u'*v + u*v'
            du := differentiate(expr.left,  var, pool);
            dv := differentiate(expr.right, var, pool);
            return expr_add(expr_mul(du, expr.right, pool),
                             expr_mul(expr.left, dv, pool), pool);

        case .DIV;
            // Quotient rule: d(u/v) = (u'*v - u*v') / v^2
            du := differentiate(expr.left,  var, pool);
            dv := differentiate(expr.right, var, pool);
            numerator   := expr_sub(expr_mul(du, expr.right, pool),
                                     expr_mul(expr.left, dv, pool), pool);
            denominator := expr_mul(expr.right, expr.right, pool);
            return expr_div(numerator, denominator, pool);

        case .EXP;
            // Chain rule: d(exp(u)) = exp(u) * u'
            du := differentiate(expr.left, var, pool);
            return expr_mul(e, du, pool); // reuse `e` itself as exp(u)

        case .LOG;
            // Chain rule: d(log(u)) = u'/u
            du := differentiate(expr.left, var, pool);
            return expr_div(du, expr.left, pool);

        case .POW;
            // For constant integer v, use the simple power rule:
            //   d(u^v) = v * u^(v-1) * u'
            // Otherwise use:
            //   d(u^v) = u^v * (v'*log(u) + v*u'/u),
            // Although these are equal when v'=0, the second formula
            // divides by u, which can be numerically problematic.
            du := differentiate(expr.left,  var, pool);
            dv := differentiate(expr.right, var, pool);
            fold(dv, pool);
            if pool.exprs[dv].kind == .CONST && pool.exprs[dv].value == 0 {
                v_minus_1 := expr_sub(expr.right, expr_const(1, pool), pool);
                return expr_mul(expr.right, expr_mul(expr_pow(expr.left, v_minus_1, pool), du, pool), pool);
            }
            // General fallback for a genuinely variable exponent.
            term1 := expr_mul(dv, expr_log(expr.left, pool), pool);
            term2 := expr_mul(expr.right, expr_div(du, expr.left, pool), pool);
            return expr_mul(e, expr_add(term1, term2, pool), pool); // reuse `e` itself as u^v
    }

    assert(false, "differentiate: unhandled Expr_Kind");
    return NO_EXPR;
}

//==================================================
// Fold constants and algebraic identities in place, bottom-up.
// Children are fully folded before the parent is checked, so one pass suffices.
//
// fold() and fold_dag() are the same rule set with $memoize baked in at
// compile time (false / true). Why fold_dag() exists: pool.exprs is a DAG
// once symbolic_det()'s cofactor cache is in play (one Expr_ID shared as
// a child of many sum terms), and a plain walk re-folds a shared node once
// per path to it. fold() stays unmemoized - not worth the table checks for
// the small unshared expressions from differentiate()/linearize()/etc.
fold :: (e: Expr_ID, pool: *Expr_Pool) {
    fold_impl(e, pool, null, false);
}

fold_dag :: (e: Expr_ID, pool: *Expr_Pool) {
    visited: Table(Expr_ID, bool);
    fold_impl(e, pool, *visited, true);
    deinit(*visited);
}

// For callers doing several fold_dag() calls in a row (e.g. solve()'s
// det_A/det_Ai/sol calls): reuses `visited`'s allocated slots across calls
// instead of allocating a fresh Table per call. Caller owns `visited`'s
// lifetime (deinit it once, after the last call).
fold_dag :: (e: Expr_ID, pool: *Expr_Pool, visited: *Table(Expr_ID, bool)) {
    table_reset(visited);
    fold_impl(e, pool, visited, true);
}

fold_impl :: (e: Expr_ID, pool: *Expr_Pool, visited: *Table(Expr_ID, bool), $memoize: bool) {
    // Recurse into a child: memoized case skips a node some other path
    // already folded this call, unmemoized case just recurses.
    recurse :: (child: Expr_ID) #expand {
        #if memoize {
            if !table_contains(visited, child) {
                table_add(visited, child, true);
                fold_impl(child, pool, visited, memoize);
            }
        } else {
            fold_impl(child, pool, visited, memoize);
        }
    }

    expr := pool.exprs[e];
    if expr.kind == {
        case .CONST;  return;
        case .SYMBOL; return;

        case .NEG;
            recurse(expr.left);
            child := pool.exprs[expr.left];
            if child.kind == .CONST {
                pool.exprs[e] = .{ kind = .CONST, value = -child.value }; // -C     → CONST(-C)
            } else if child.kind == .NEG {
                pool.exprs[e] = pool.exprs[child.left];                       // -(-x)  → x
            // NOTE: -(a+b) is intentionally NOT distributed to (-a)+(-b).
            // Keeping it factored enables (-a)/(-b)->a/b and (-a)-b->-(a+b) to fire.
            } else if child.kind == .SUB {
                pool.exprs[e] = .{ kind = .SUB, left = child.right, right = child.left }; // -(a-b) → b-a
            }

        case .EXP;
            recurse(expr.left); // no further constant-folding for a transcendental of a non-literal operand

        case .LOG;
            recurse(expr.left); // no further constant-folding for a transcendental of a non-literal operand

        case .POW;
            // For now, we don't attempt to do any constant-folding here.
            recurse(expr.left);
            recurse(expr.right);

        case; // ADD, SUB, MUL, DIV
            recurse(expr.left);
            recurse(expr.right);
            left  := pool.exprs[expr.left];
            right := pool.exprs[expr.right];

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
                pool.exprs[e] = .{ kind = .CONST, value = result };
                return;
            }

            if expr.kind == .ADD {
                if left.kind  == .CONST && left.value  == 0 { pool.exprs[e] = right; return; } // 0 + x → x
                if right.kind == .CONST && right.value == 0 { pool.exprs[e] = left;  return; } // x + 0 → x
                // (-a) + (-b) → -(a+b) : factor out shared negation
                if left.kind == .NEG && right.kind == .NEG {
                    inner := expr_bin(.ADD, left.left, right.left, pool);
                    recurse(inner);
                    pool.exprs[e] = .{ kind = .NEG, left = inner };
                    return;
                }
                // a + (-b) → a - b : normalize to SUB form
                if right.kind == .NEG {
                    pool.exprs[e] = .{ kind = .SUB, left = expr.left, right = right.left };
                    fold_impl(e, pool, visited, memoize);
                    return;
                }
                // (-a) + b → b - a : normalize so the NEG term is on the right
                if left.kind == .NEG {
                    pool.exprs[e] = .{ kind = .SUB, left = expr.right, right = left.left };
                    fold_impl(e, pool, visited, memoize);
                    return;
                }
            }
            if expr.kind == .SUB {
                if right.kind == .CONST && right.value == 0 { pool.exprs[e] = left;  return; } // x - 0 → x
                if left.kind  == .CONST && left.value  == 0 { pool.exprs[e] = .{ kind = .NEG, left = expr.right }; return; } // 0 - x → -x
                // a - (-b) → a + b : eliminate double sign
                if right.kind == .NEG {
                    pool.exprs[e] = .{ kind = .ADD, left = expr.left, right = right.left };
                    fold_impl(e, pool, visited, memoize);
                    return;
                }
                // (-a) - b → -(a+b) : factor out negation from left operand
                if left.kind == .NEG {
                    inner := expr_bin(.ADD, left.left, expr.right, pool);
                    recurse(inner);
                    pool.exprs[e] = .{ kind = .NEG, left = inner };
                    return;
                }
            }
            if expr.kind == .MUL {
                if left.kind  == .CONST && left.value  ==  0 { pool.exprs[e] = left;  return; } // 0 * x → 0
                if right.kind == .CONST && right.value ==  0 { pool.exprs[e] = right; return; } // x * 0 → 0
                if left.kind  == .CONST && left.value  ==  1 { pool.exprs[e] = right; return; } // 1 * x → x
                if right.kind == .CONST && right.value ==  1 { pool.exprs[e] = left;  return; } // x * 1 → x
                if left.kind  == .CONST && left.value  == -1 { pool.exprs[e] = .{ kind = .NEG, left = expr.right }; return; } // -1 * x → -x
                if right.kind == .CONST && right.value == -1 { pool.exprs[e] = .{ kind = .NEG, left = expr.left  }; return; } // x * -1 → -x
                // Pull NEG out of MUL: a*(-b) → -(a*b), (-a)*b → -(a*b)
                // Re-fold afterwards to collapse any resulting NEG(NEG(...))
                if right.kind == .NEG {
                    inner := expr_mul(expr.left, right.left, pool);
                    recurse(inner);
                    pool.exprs[e] = .{ kind = .NEG, left = inner };
                    fold_impl(e, pool, visited, memoize);
                    return;
                }
                if left.kind == .NEG {
                    inner := expr_mul(left.left, expr.right, pool);
                    recurse(inner);
                    pool.exprs[e] = .{ kind = .NEG, left = inner };
                    fold_impl(e, pool, visited, memoize);
                    return;
                }
            }
            if expr.kind == .DIV {
                if right.kind == .CONST && right.value == 1 { pool.exprs[e] = left; return; } // x / 1 → x
                if left.kind  == .CONST && left.value  == 0 { pool.exprs[e] = left; return; } // 0 / x → 0 (x assumed nonzero; other identities here also don't guard divide-by-zero either)
                // (-a) / (-b) → a / b : cancel double negation (check this before single-NEG cases)
                if left.kind == .NEG && right.kind == .NEG {
                    pool.exprs[e] = .{ kind = .DIV, left = left.left, right = right.left };
                    fold_impl(e, pool, visited, memoize);
                    return;
                }
                // (-a) / b → -(a/b) : move negation outside the division
                if left.kind == .NEG {
                    inner := expr_bin(.DIV, left.left, expr.right, pool);
                    recurse(inner);
                    pool.exprs[e] = .{ kind = .NEG, left = inner };
                    return;
                }
                // a / (-b) → -(a/b) : move negation outside the division
                if right.kind == .NEG {
                    inner := expr_bin(.DIV, expr.left, right.left, pool);
                    recurse(inner);
                    pool.exprs[e] = .{ kind = .NEG, left = inner };
                    return;
                }
            }
    }
}

// ===========================================================================
// Cramer's rule solver using memoized cofactor expansion.
//
// Why not Gauss-Jordan: it builds each diagonal entry as a product of
// intermediate pivots. For typical audio-circuit conductances (~1e-3 to
// 1e-5) an 8x8 system produces pivot products of 1e-45 to 1e-60, at or
// below float32's minimum denormal (~1.4e-45), underflowing to zero and
// producing NaN/Inf output.
//
// Cramer's rule expresses det(A) via cofactor (Laplace) expansion, a
// *sum* of products rather than a *product* of pivots, which for the same
// circuit stays around ~6e-16, well within float32's normal range. All n
// solutions share the same det(A) denominator, so CSE extracts it once and
// recip_hoist turns it into a single precomputed reciprocal.
// ===========================================================================

// Count the number of set bits in a 64-bit integer.
popcount_u64 :: (x: u64) -> int {
    n := 0;
    v := x;
    while v != 0 { n += 1; v &= v - 1; }
    return n;
}

// Index of the lowest set bit (0-based).  Precondition: x != 0.
ctz_u64 :: (x: u64) -> int {
    n := 0;
    v := x;
    while (v & 1) == 0 { n += 1; v >>= 1; }
    return n;
}

// Memoization cache entry: the determinant of the sub-matrix identified
// by the given row / column bitmasks within the full n×n matrix.
Det_Cache_Entry :: struct {
    rows : u64;
    cols : u64;
    value: Expr_ID;
}

// Symbolically compute the determinant of the sub-matrix of M (n×n,
// row-major) whose rows and columns are indicated by the bitmasks.
// M[i*n+j] == NO_EXPR is treated as a structural zero (skips that term).
// Results are memoized in `cache` to avoid re-expanding identical minors.
symbolic_det :: (M: [] Expr_ID, n: int,
                 rows: u64, cols: u64,
                 cache: *[..] Det_Cache_Entry,
                 pool: *Expr_Pool) -> Expr_ID {

    // Cache lookup.
    for e: cache.*  if e.rows == rows && e.cols == cols  return e.value;

    k := popcount_u64(rows);
    assert(k == popcount_u64(cols));

    det: Expr_ID;

    if k == 0 {
        // Empty sub-matrix: determinant = 1 (multiplicative identity).
        det = expr_const(1, pool);

    } else if k == 1 {
        r   := ctz_u64(rows);
        c   := ctz_u64(cols);
        det = M[r * n + c];
        if det == NO_EXPR  det = expr_const(0, pool);

    } else {
        // Cofactor expansion along the lowest-numbered remaining row.
        r0       := ctz_u64(rows);
        sub_rows := rows ^ (cast(u64)1 << r0);

        det      = NO_EXPR;
        col_pos  := 0;          // position of cj within the ordered cols set
        remaining := cols;
        while remaining != 0 {
            cj        := ctz_u64(remaining);
            remaining &= remaining - 1;   // clear lowest set bit

            entry := M[r0 * n + cj];
            if entry == NO_EXPR { col_pos += 1; continue; }   // structural zero

            sub_cols := cols ^ (cast(u64)1 << cj);
            minor    := symbolic_det(M, n, sub_rows, sub_cols, cache, pool);

            // Skip if the minor is the constant zero.
            minor_zero := pool.exprs[minor].kind == .CONST && pool.exprs[minor].value == 0;
            if !minor_zero {
                term := expr_mul(entry, minor, pool);
                sign_plus := (col_pos % 2) == 0;
                det = ifx sign_plus then combine(det, term, .ADD, pool)
                                    else combine(det, term, .SUB, pool);
            }
            col_pos += 1;
        }
        if det == NO_EXPR  det = expr_const(0, pool);
    }

    array_add(cache, .{ rows = rows, cols = cols, value = det });
    return det;
}

// Pessimistic worst-case *minimum* magnitude of `expr`, given a symbol name
// -> magnitude lookup. Symbols missing from the table fall back to
// `default_magnitude` (1.0 is a safe no-op: an unknown factor won't shrink
// a product estimate).
//
// Rules (all worst-case, not typical-case; this only needs to catch
// "could plausibly be too small", false positives are fine):
//   CONST        -> abs(value), or 1 if zero
//   SYMBOL       -> magnitudes[name], else default_magnitude
//   NEG          -> magnitude(operand)
//   ADD, SUB     -> pessimistic: assume no cancellation, so use the larger child's magnitude
//   MUL          -> magnitude(left) * magnitude(right)
//   DIV          -> magnitude(left) / magnitude(right)
//   EXP, LOG, POW -> not expected in a linear det(A) tree; fall back to default_magnitude
estimate_min_magnitude :: (expr: Expr_ID, pool: *Expr_Pool,
                           magnitudes: Table(string, float64),
                           default_magnitude: float64 = 1.0) -> float64 {
    if expr == NO_EXPR  return 0;
    ex := pool.exprs[expr];

    if ex.kind == {
        case .CONST;
            v := cast(float64) ex.value;
            if v < 0  v = -v;
            return ifx v == 0 then 1.0 else v;
        case .SYMBOL;
            found, mag := table_find(*magnitudes, ex.name);
            return ifx found then mag else default_magnitude;
        case .NEG;
            return estimate_min_magnitude(ex.left, pool, magnitudes, default_magnitude);
        case .ADD; #through;
        case .SUB;
            l := estimate_min_magnitude(ex.left,  pool, magnitudes, default_magnitude);
            r := estimate_min_magnitude(ex.right, pool, magnitudes, default_magnitude);
            return ifx l > r then l else r;
        case .MUL;
            l := estimate_min_magnitude(ex.left,  pool, magnitudes, default_magnitude);
            r := estimate_min_magnitude(ex.right, pool, magnitudes, default_magnitude);
            return l * r;
        case .DIV;
            l := estimate_min_magnitude(ex.left,  pool, magnitudes, default_magnitude);
            r := estimate_min_magnitude(ex.right, pool, magnitudes, default_magnitude);
            return ifx r == 0 then l else l / r;
        case;
            return default_magnitude;
    }
}

// Caller-supplied policy for the det(A) magnitude-underflow warning.
// When enabled, the solver will use row scaling to try to avoid underflow,
// and will emit a warning is this is not possible.
Magnitude_Check :: struct {
    enabled:    bool;
    threshold:  float64;
    magnitudes: Table(string, float64);
    label:      string; // free-form, only used in the printed warning
}

// Largest estimate_min_magnitude() among a row's nonzero entries. This is the
// "dominant" term, used to pick that row's power-of-two scale factor.
estimate_row_magnitude :: (entries: [] Expr_ID, pool: *Expr_Pool, magnitudes: Table(string, float64)) -> float64 {
    mag: float64 = 0.0;
    for e : entries {
        if e == NO_EXPR  continue;
        m := estimate_min_magnitude(e, pool, magnitudes);
        if m > mag  mag = m;
    }
    return ifx mag > 0 then mag else 1.0;
}

// Power-of-two exponent that brings `magnitude` closest to 1 (round(-log2(magnitude))).
// Clamped well within CONST's s64 range; no real component magnitude needs more.
row_scale_exponent :: (magnitude: float64) -> int {
    if magnitude <= 0  return 0;
    LOG2E :: 1.4426950408889634; // 1/ln(2)
    k := cast(int) Math.floor(-Math.log(magnitude) * LOG2E + 0.5);
    if k >  48  k =  48;
    if k < -48  k = -48;
    return k;
}

// Multiply/divide `id` by 2^k. This is exact in floating point either way,
// since only the exponent field changes. k > 0 grows magnitude, k < 0 shrinks it.
scale_pow2 :: (id: Expr_ID, k: int, pool: *Expr_Pool) -> Expr_ID {
    if id == NO_EXPR || k == 0  return id;
    if k > 0  return expr_mul(id, expr_const(cast(s64)1 << k, pool), pool);
    return expr_div(id, expr_const(cast(s64)1 << (-k), pool), pool);
}

// Solve a square linear system A*x = b using Cramer's rule.
// Every solution x[i] = det(A_i) / det(A), where A_i is A with column i
// replaced by b. All solutions share the same det(A), so CSE extracts it as
// one shared temporary and recip_hoist turns it into a single precomputed
// reciprocal.
solve :: (equations: [] Expr_ID, solve_for: [] Expr_ID, pool: *Expr_Pool,
         check := Magnitude_Check.{}) -> [..] Expr_ID {
    n := equations.count;
    assert(n == solve_for.count, "Need one equation per unknown!");
    assert(n <= 62, "System too large for bitmask Cramer solver (n > 62)");
    print("Solving %1x%1 system\n", n);
    auto_release_temp();

    // -----------------------------------------------------------------------
    // Step 1: Extract coefficient matrix A and RHS vector b.
    //   A[i*n + j] = coefficient of solve_for[j] in equation i
    //   b[i]       = -constant term  (so A*x = b)
    // -----------------------------------------------------------------------
    A: [..] Expr_ID;
    b: [..] Expr_ID;
    A.allocator = temp;
    b.allocator = temp;
    array_reserve(*A, n * n);
    array_reserve(*b, n);

    for i: 0..n-1 {
        lin := linearize(equations[i], solve_for, pool);
        row_start := A.count;
        for j: 0..n-1 {
            a_ij := lin.coeffs[j];
            if a_ij != NO_EXPR  fold(a_ij, pool);
            array_add(*A, a_ij);
        }
        rhs_i := ifx lin.constant != NO_EXPR then expr_neg(lin.constant, pool) else NO_EXPR;
        if rhs_i != NO_EXPR  fold(rhs_i, pool);

        // Step 1b: if this row's coefficients are small enough to plausibly
        // contribute to det(A) underflow, scale the whole row up by a power
        // of two so its dominant term lands near magnitude 1.
        if check.enabled {
            mag := estimate_row_magnitude(array_view(A, row_start, n), pool, check.magnitudes);
            k := row_scale_exponent(mag);
            if k > 0 {
                for j: 0..n-1 {
                    idx := row_start + j;
                    if A[idx] == NO_EXPR  continue;
                    A[idx] = scale_pow2(A[idx], k, pool);
                    fold(A[idx], pool);
                }
                if rhs_i != NO_EXPR {
                    rhs_i = scale_pow2(rhs_i, k, pool);
                    fold(rhs_i, pool);
                }
            }
        }

        array_add(*b, rhs_i);
    }

    all_mask : u64 = (cast(u64)1 << n) - 1;

    // -----------------------------------------------------------------------
    // Step 2: Compute det(A), the shared denominator for every solution.
    // Because symbolic_det uses cofactor expansion, det(A) is expressed as
    // a *sum* of products of conductances, keeping values in float32 range.
    // -----------------------------------------------------------------------
    det_A_cache: [..] Det_Cache_Entry;
    det_A_cache.allocator = temp;
    det_A := symbolic_det(A, n, all_mask, all_mask, *det_A_cache, pool);
    assert(!(pool.exprs[det_A].kind == .CONST && pool.exprs[det_A].value == 0),
           "System matrix is singular!");

    // Shared across every fold_dag() call in this function.
    fold_dag_visited: Table(Expr_ID, bool);
    defer deinit(*fold_dag_visited);
    fold_dag(det_A, pool, *fold_dag_visited);

    if check.enabled {
        est := estimate_min_magnitude(det_A, pool, check.magnitudes);
        if est < check.threshold {
            label_suffix := ifx check.label.count > 0 then tprint(" for \"%\"", check.label) else "";
            log("Warning: det(A)% could plausibly underflow -- estimated worst-case minimum magnitude % is below threshold %. See sym_algebra.jai's Cramer's-rule comment for background; consider splitting the circuit (e.g. a `split_here` directive) if this shows up as actual instability.\n",
                label_suffix, formatFloat(est, mode = .SCIENTIFIC), formatFloat(check.threshold, mode = .SCIENTIFIC));
        }
    }

    // -----------------------------------------------------------------------
    // Step 3: For each variable x[i], build A_i and compute det(A_i).
    // We seed the A_i cache with all sub-matrix results from det(A) that do
    // NOT involve column i; those sub-matrices are identical in A and A_i,
    // so the same Expr_IDs can be reused. This enables the later CSE pass
    // to recognise shared cofactors across all Cramer determinants.
    // -----------------------------------------------------------------------
    result: [..] Expr_ID;
    array_reserve(*result, n);

    for i: 0..n-1 {
        // Build A_i: same as A but with column i replaced by b.
        A_i: [..] Expr_ID;
        A_i.allocator = temp;
        array_reserve(*A_i, n * n);
        for row: 0..n-1 {
            for col: 0..n-1 {
                array_add(*A_i, ifx col == i then b[row] else A[row*n + col]);
            }
        }

        // Pre-populate cache with invariant sub-determinants from det(A).
        det_Ai_cache: [..] Det_Cache_Entry;
        det_Ai_cache.allocator = temp;
        array_reserve(*det_Ai_cache, det_A_cache.count);
        col_i_bit := cast(u64)1 << i;
        for e: det_A_cache {
            if (e.cols & col_i_bit) == 0   // sub-matrix doesn't touch col i
                array_add(*det_Ai_cache, e);
        }

        det_Ai := symbolic_det(A_i, n, all_mask, all_mask, *det_Ai_cache, pool);
        fold_dag(det_Ai, pool, *fold_dag_visited);

        // Numerically zero solution: return directly without a division.
        if pool.exprs[det_Ai].kind == .CONST && pool.exprs[det_Ai].value == 0 {
            array_add(*result, det_Ai);
            continue;
        }

        sol := expr_div(det_Ai, det_A, pool);
        fold_dag(sol, pool, *fold_dag_visited);
        array_add(*result, sol);
    }

    return result;
}

//==================================================
// Simplification helpers

// Euclidean GCD.
gcd :: (a: s64, b: s64) -> s64 {
    a = ifx a < 0 then -a else a;
    b = ifx b < 0 then -b else b;
    while b != 0 { t := b;  b = a % b;  a = t; }
    return a;
}

// Deep structural equality: are two sub-trees the same formula?
exprs_equal :: (a: Expr_ID, b: Expr_ID, exprs: [] Expr) -> bool {
    if a == b  return true;
    ea := exprs[a];
    eb := exprs[b];
    if ea.kind != eb.kind  return false;
    if ea.kind == .CONST   return ea.value == eb.value;
    if ea.kind == .SYMBOL  return ea.name == eb.name;
    if ea.kind == .NEG     return exprs_equal(ea.left, eb.left, exprs);
    return exprs_equal(ea.left, eb.left, exprs) && exprs_equal(ea.right, eb.right, exprs);
}

// Structural hash matching exprs_equal: equal subtrees always hash equal.
// Collisions are fine (cse_pass still confirms with exprs_equal); this is
// just to avoid comparing every pair. `cache` memoizes per cse_pass() call,
// same DAG-sharing reason as fold_dag.
expr_hash :: (e: Expr_ID, exprs: [] Expr, cache: *Table(Expr_ID, u32)) -> u32 {
    found, cached := table_find(cache, e);
    if found  return cached;

    ex := exprs[e];
    h: u32;
    if ex.kind == {
        case .CONST;  h = Hash.get_hash(ex.value, cast(u32) ex.kind);
        case .SYMBOL; h = Hash.get_hash(ex.name,  cast(u32) ex.kind);
        case;
            if is_unary(ex.kind) {
                lh := expr_hash(ex.left, exprs, cache);
                h = Hash.get_hash(lh, cast(u32) ex.kind);
            } else {
                lh := expr_hash(ex.left,  exprs, cache);
                rh := expr_hash(ex.right, exprs, cache);
                h = Hash.get_hash(rh, Hash.get_hash(lh, cast(u32) ex.kind));
            }
    }
    table_add(cache, e, h);
    return h;
}

// Rational product form:  (int_num / int_denom) * ∏ sym_num / ∏ sym_denom
// Invariant: int_denom >= 1,  gcd(|int_num|, int_denom) == 1.
Flat :: struct {
    int_num:   s64;
    int_denom: s64;
    sym_num:   [..] Expr_ID;
    sym_denom: [..] Expr_ID;
}

// Decompose a MUL/DIV/NEG/CONST/SYMBOL sub-tree into Flat product form.
// ADD/SUB nodes are treated as opaque atomic symbolic factors.
flatten :: (e: Expr_ID, exprs: [] Expr) -> Flat {
    f: Flat;
    f.sym_num.allocator   = temp;
    f.sym_denom.allocator = temp;
    f.int_num   = 1;
    f.int_denom = 1;
    expr := exprs[e];
    if expr.kind == {
        case .NULL;   f.int_num = 0;
        case .CONST;  f.int_num = expr.value;
        case .SYMBOL; array_add(*f.sym_num, e);

        case .NEG;
            inner := flatten(expr.left, exprs);
            f.int_num   = -inner.int_num;
            f.int_denom =  inner.int_denom;
            f.sym_num   =  inner.sym_num;
            f.sym_denom =  inner.sym_denom;

        case .MUL;
            a := flatten(expr.left,  exprs);
            b := flatten(expr.right, exprs);
            f.int_num   = a.int_num   * b.int_num;
            f.int_denom = a.int_denom * b.int_denom;
            for s: a.sym_num   array_add(*f.sym_num,   s);
            for s: b.sym_num   array_add(*f.sym_num,   s);
            for s: a.sym_denom array_add(*f.sym_denom, s);
            for s: b.sym_denom array_add(*f.sym_denom, s);

        case .DIV;
            a := flatten(expr.left,  exprs);
            b := flatten(expr.right, exprs);
            // (a.n/a.d) / (b.n/b.d)  =  (a.n * b.d) / (a.d * b.n)
            f.int_num   = a.int_num   * b.int_denom;
            f.int_denom = a.int_denom * b.int_num;
            for s: a.sym_num   array_add(*f.sym_num,   s);
            for s: b.sym_denom array_add(*f.sym_num,   s);
            for s: a.sym_denom array_add(*f.sym_denom, s);
            for s: b.sym_num   array_add(*f.sym_denom, s);

        case;  // ADD, SUB: treat as one opaque symbolic factor
            array_add(*f.sym_num, e);
    }
    // Normalize: keep int_denom > 0 and reduce by GCD.
    if f.int_denom < 0 { f.int_num = -f.int_num;  f.int_denom = -f.int_denom; }
    g := gcd(ifx f.int_num < 0 then -f.int_num else f.int_num, f.int_denom);
    if g > 1 { f.int_num /= g;  f.int_denom /= g; }
    return f;
}

// Left-associative product:  coeff * syms[0] * syms[1] * ...
// Returns expr_const(1, exprs) when coeff == 1 and syms is empty.
build_product :: (coeff: s64, syms: [] Expr_ID, pool: *Expr_Pool) -> Expr_ID {
    result := NO_EXPR;
    if coeff != 1 || syms.count == 0  result = expr_const(coeff, pool);
    for s: syms {
        if result == NO_EXPR  result = s;
        else                  result = expr_bin(.MUL, result, s, pool);
    }
    if result == NO_EXPR  result = expr_const(1, pool);
    return result;
}

// Reconstruct an expression from a Flat.  Caller should fold() afterwards if needed.
rebuild_flat :: (f: Flat, pool: *Expr_Pool) -> Expr_ID {
    if f.int_num == 0  return expr_const(0, pool);
    negative  := f.int_num < 0;
    abs_coeff := ifx negative then -f.int_num else f.int_num;
    num_expr  := build_product(abs_coeff, f.sym_num, pool);
    result: Expr_ID;
    if f.int_denom == 1 && f.sym_denom.count == 0 {
        result = num_expr;
    } else {
        denom_expr := build_product(f.int_denom, f.sym_denom, pool);
        result = expr_bin(.DIV, num_expr, denom_expr, pool);
    }
    if negative  result = expr_neg(result, pool);
    return result;
}

// Try to exactly divide expression `num` by `factor`.
//
// For ADD / SUB / NEG nodes: recurses and requires every branch to succeed.
// For leaf / product nodes:  uses Flat arithmetic to verify exact divisibility.
//
// Returns (true, result_id) on success, (false, NO_EXPR) on failure.
// On failure the expression array may contain a few unreachable scratch nodes;
// those are harmless and will not be printed.
try_cancel :: (num: Expr_ID, factor: Expr_ID, pool: *Expr_Pool) -> (bool, Expr_ID) {
    expr := pool.exprs[num];

    if expr.kind == .NEG {
        ok, inner := try_cancel(expr.left, factor, pool);
        if ok  return true, expr_neg(inner, pool);
        return false, NO_EXPR;
    }

    if expr.kind == .ADD || expr.kind == .SUB {
        ok_l, res_l := try_cancel(expr.left,  factor, pool);
        if !ok_l  return false, NO_EXPR;
        ok_r, res_r := try_cancel(expr.right, factor, pool);
        if !ok_r  return false, NO_EXPR;
        return true, expr_bin(expr.kind, res_l, res_r, pool);
    }

    // Leaf / product node: use Flat arithmetic.
    nf := flatten(num,    pool.exprs);
    df := flatten(factor, pool.exprs);

    // Integer part: (nf.n / nf.d) / (df.n / df.d) = (nf.n * df.d) / (nf.d * df.n).
    // Must reduce to an exact integer (reduced denominator == 1).
    new_n := nf.int_num   * df.int_denom;
    new_d := nf.int_denom * df.int_num;
    if new_d == 0  return false, NO_EXPR;
    if new_d < 0 { new_n = -new_n;  new_d = -new_d; }
    {
        g := gcd(ifx new_n < 0 then -new_n else new_n, new_d);
        if g > 0 { new_n /= g;  new_d /= g; }
    }
    if new_d != 1  return false, NO_EXPR;  // not exact

    // Symbolic part:
    //   combined_n = nf.sym_num   ∪ df.sym_denom
    //   combined_d = nf.sym_denom ∪ df.sym_num
    // Cross-cancel matching pairs (one entry from combined_d with one from combined_n).
    // Success condition: every factor that came from df.sym_num must be cancelled.
    combined_n: [..] Expr_ID;
    combined_d: [..] Expr_ID;
    combined_n.allocator = temp;
    combined_d.allocator = temp;
    array_reserve(*combined_n, nf.sym_num.count + df.sym_denom.count);
    array_reserve(*combined_d, nf.sym_denom.count + df.sym_num.count);
    for s: nf.sym_num    array_add(*combined_n, s);
    for s: df.sym_denom  array_add(*combined_n, s);
    for s: nf.sym_denom  array_add(*combined_d, s);
    for s: df.sym_num    array_add(*combined_d, s);

    used_n: [..] bool;
    used_d: [..] bool;
    used_n.allocator = temp;
    used_d.allocator = temp;
    array_reserve(*used_n, combined_n.count);
    array_reserve(*used_d, combined_d.count);
    for _: 0..combined_n.count-1  array_add(*used_n, false);
    for _: 0..combined_d.count-1  array_add(*used_d, false);

    for i: 0..combined_d.count-1 {
        if used_d[i]  continue;
        for j: 0..combined_n.count-1 {
            if used_n[j]  continue;
            if exprs_equal(combined_d[i], combined_n[j], pool.exprs) {
                used_d[i] = true;
                used_n[j] = true;
                break;
            }
        }
    }

    // Every factor from df.sym_num (the last df.sym_num.count entries of combined_d)
    // must have been cancelled.
    for i: nf.sym_denom.count .. combined_d.count - 1 {
        if !used_d[i]  return false, NO_EXPR;
    }

    result_f: Flat;
    result_f.sym_num.allocator   = temp;
    result_f.sym_denom.allocator = temp;
    result_f.int_num   = new_n;
    result_f.int_denom = 1;
    for i: 0..combined_n.count-1  if !used_n[i]  array_add(*result_f.sym_num,   combined_n[i]);
    for i: 0..combined_d.count-1  if !used_d[i]  array_add(*result_f.sym_denom, combined_d[i]);
    return true, rebuild_flat(result_f, pool);
}

// Simplify a DIV node in-place by cancelling common symbolic and integer factors
// between numerator and denominator.
simplify_div :: (e: Expr_ID, pool: *Expr_Pool) {
    expr := pool.exprs[e];
    num  := expr.left;
    den  := expr.right;

    // a / a  →  1
    if exprs_equal(num, den, pool.exprs) {
        pool.exprs[e] = .{ kind = .CONST, value = 1 };
        return;
    }

    den_flat := flatten(den, pool.exprs);
    if den_flat.int_num == 0  return;  // degenerate denominator; leave as-is

    cur_num := num;

    // Cancel symbolic denominator factors from the numerator one-by-one.
    remaining_sym: [..] Expr_ID;
    remaining_sym.allocator = temp;
    for sym: den_flat.sym_num {
        ok, new_num := try_cancel(cur_num, sym, pool);
        if ok  cur_num = new_num;
        else   array_add(*remaining_sym, sym);
    }

    // Try to cancel the integer coefficient of the denominator (exact division).
    remaining_int := den_flat.int_num;  // > 0 after fold normalises sign
    if den_flat.int_denom == 1 && remaining_int > 1 {
        int_factor := expr_const(remaining_int, pool);
        ok, new_num := try_cancel(cur_num, int_factor, pool);
        if ok { cur_num = new_num;  remaining_int = 1; }
    }

    // Partial integer GCD: if the numerator is a plain CONST, reduce by GCD.
    if den_flat.int_denom == 1 && remaining_int > 1 {
        cur_expr := pool.exprs[cur_num];
        if cur_expr.kind == .CONST && cur_expr.value != 0 {
            abs_val := ifx cur_expr.value < 0 then -cur_expr.value else cur_expr.value;
            g := gcd(abs_val, remaining_int);
            if g > 1 {
                cur_num       = expr_const(cur_expr.value / g, pool);
                remaining_int /= g;
            }
        }
    }

    // If the denominator itself had a denominator (den_flat.sym_denom / den_flat.int_denom),
    // those factors flip to the numerator:  num / (A/B)  =  num * B / A.
    for sym: den_flat.sym_denom {
        cur_num = expr_bin(.MUL, cur_num, sym, pool);
    }
    if den_flat.int_denom != 1 {
        cur_num = expr_bin(.MUL, cur_num, expr_const(den_flat.int_denom, pool), pool);
    }

    // Has the denominator fully cancelled?
    if remaining_sym.count == 0 && remaining_int == 1 {
        pool.exprs[e] = pool.exprs[cur_num];
        fold(e, pool);
        return;
    }

    new_den := build_product(remaining_int, remaining_sym, pool);
    pool.exprs[e] = .{ kind = .DIV, left = cur_num, right = new_den };
    fold(e, pool);
}

// If a MUL has a DIV child, lift it into a single top-level DIV so that
// simplify_div can see the full numerator and denominator.
//   (a/b) * c  →  (a*c) / b
//    a * (b/c) →  (a*b) / c
simplify_mul :: (e: Expr_ID, pool: *Expr_Pool) {
    expr  := pool.exprs[e];
    left  := pool.exprs[expr.left];
    right := pool.exprs[expr.right];
    if left.kind == .DIV {
        new_num := expr_bin(.MUL, left.left, expr.right, pool);
        pool.exprs[e] = .{ kind = .DIV, left = new_num, right = left.right };
        simplify_div(e, pool);
    } else if right.kind == .DIV {
        new_num := expr_bin(.MUL, expr.left, right.left, pool);
        pool.exprs[e] = .{ kind = .DIV, left = new_num, right = right.right };
        simplify_div(e, pool);
    }
}

// Recursive bottom-up pass: simplify children first, then the current node.
simplify_node :: (e: Expr_ID, pool: *Expr_Pool) {
    expr := pool.exprs[e];
    if expr.kind == .CONST || expr.kind == .SYMBOL || expr.kind == .NULL  return;
    if is_unary(expr.kind) {
        simplify_node(expr.left, pool);
    } else {
        simplify_node(expr.left,  pool);
        simplify_node(expr.right, pool);
    }
    expr = pool.exprs[e];  // re-read: a child may have changed its own kind
    if expr.kind == .DIV  simplify_div(e, pool);
    if expr.kind == .MUL  simplify_mul(e, pool);
}

//==================================================
simplify :: (e: Expr_ID, pool: *Expr_Pool) -> Expr_ID {
    auto_release_temp();

    fold(e, pool);             // constant-fold and normalise signs
    simplify_node(e, pool);    // cancel common factors (MUL/DIV lifting)
    fold(e, pool);             // clean up any new constant sub-expressions
    simplify_node(e, pool);    // second pass: catches opportunities exposed above
    fold(e, pool);
    return e;
}


//==================================================
// `suffix` is the target type's literal suffix (e.g. "f" for float, "" for double).
//
// EXP/LOG/POW are emitted as placeholder tokens (`math_fn_exp` etc.).
print_expr :: (e: Expr_ID, exprs: [] Expr, suffix: string = "") -> string {
    expr_to_builder :: (builder: *String_Builder, e: Expr_ID, exprs: [] Expr, suffix: string) {
        if exprs[e].kind == {
            case .CONST;
                print_to_builder(builder, "%.0", exprs[e].value);
                append(builder, suffix);

            case .SYMBOL;
                append(builder, exprs[e].name);

            case .NEG;
                append(builder, "(-");
                expr_to_builder(builder, exprs[e].left, exprs, suffix);
                append(builder, ")");

            case .EXP;
                append(builder, "math_fn_exp(");
                expr_to_builder(builder, exprs[e].left, exprs, suffix);
                append(builder, ")");

            case .LOG;
                append(builder, "math_fn_log(");
                expr_to_builder(builder, exprs[e].left, exprs, suffix);
                append(builder, ")");

            case .POW;
                append(builder, "math_fn_pow(");
                expr_to_builder(builder, exprs[e].left, exprs, suffix);
                append(builder, ", ");
                expr_to_builder(builder, exprs[e].right, exprs, suffix);
                append(builder, ")");

            case;
                append(builder, "(");
                expr_to_builder(builder, exprs[e].left, exprs, suffix);
                if exprs[e].kind == {
                    case .ADD; append(builder, " + ");
                    case .SUB; append(builder, " - ");
                    case .MUL; append(builder, " * ");
                    case .DIV; append(builder, " / ");
                }
                expr_to_builder(builder, exprs[e].right, exprs, suffix);
                append(builder, ")");
        }
    }

    builder: String_Builder;
    expr_to_builder(*builder, e, exprs, suffix);
    return builder_to_string(*builder);
}

//==================================================
CSE_Var :: struct {
    name:    string;
    expr_id: Expr_ID;
}

cse_expr_size :: (e: Expr_ID, exprs: [] Expr) -> int {
    ex := exprs[e];
    if ex.kind == .CONST || ex.kind == .SYMBOL || ex.kind == .NULL  return 0;
    if is_unary(ex.kind)  return 1 + cse_expr_size(ex.left, exprs);
    return 1 + cse_expr_size(ex.left, exprs) + cse_expr_size(ex.right, exprs);
}

// `seen` maps an Expr_ID we've already recorded to its slot in `ids`/`counts`,
// so a repeat visit is O(1) instead of a linear scan. With cofactor-shared
// nodes recurring thousands of times, the scan used to make this O(ids^2).
cse_count_refs :: (e: Expr_ID, exprs: [] Expr,
                   ids: *[..] Expr_ID, counts: *[..] int, seen: *Table(Expr_ID, int)) {
    ex := exprs[e];
    if ex.kind == .CONST || ex.kind == .SYMBOL || ex.kind == .NULL  return;
    found, idx := table_find(seen, e);
    if found {
        counts.*[idx] += 1;
        return;
    }
    array_add(ids,    e);
    array_add(counts, 1);
    table_add(seen, e, ids.count - 1);
    if is_unary(ex.kind) {
        cse_count_refs(ex.left, exprs, ids, counts, seen);
    } else {
        cse_count_refs(ex.left,  exprs, ids, counts, seen);
        cse_count_refs(ex.right, exprs, ids, counts, seen);
    }
}

cse_pass :: (gen_var_ids: [] Expr_ID, cse_vars: *[..] CSE_Var,
             pool: *Expr_Pool, counter: *int, prefix: string) -> bool {
    auto_release_temp();

    ids:    [..] Expr_ID;
    counts: [..] int;
    ids.allocator    = temp;
    counts.allocator = temp;
    seen: Table(Expr_ID, int);
    seen.allocator = temp;
    for v  : gen_var_ids  cse_count_refs(v,          pool.exprs, *ids, *counts, *seen);
    for cv : cse_vars.*   cse_count_refs(cv.expr_id, pool.exprs, *ids, *counts, *seen);
    if ids.count == 0  return false;

    // Union-find: merge structurally-equal nodes into one equivalence class.
    // Bucket by hash first so exprs_equal only checks pairs that already
    // match. `ids` can run into the thousands, so the old all-pairs loop
    // was O(ids^2) per pass, x20 passes.
    hash_cache: Table(Expr_ID, u32);
    hash_cache.allocator = temp;
    hashes: [..] u32;
    hashes.allocator = temp;
    array_resize(*hashes, ids.count);
    for i: 0..ids.count-1  hashes[i] = expr_hash(ids[i], pool.exprs, *hash_cache);

    // Buckets as a flat array of (hash, idx) pairs
    Hash_Idx :: struct { hash: u32; idx: int; }
    bucket_entries: [..] Hash_Idx;
    bucket_entries.allocator = temp;
    array_reserve(*bucket_entries, ids.count);
    for i: 0..ids.count-1  array_add(*bucket_entries, .{ hash = hashes[i], idx = i });
    quick_sort(bucket_entries, (a: Hash_Idx, b: Hash_Idx) -> int { return cast(s64) a.hash - cast(s64) b.hash; });

    parent: [..] int;
    parent.allocator = temp;
    array_reserve(*parent, ids.count);
    for i: 0..ids.count-1  array_add(*parent, i);

    bucket_start := 0;
    while bucket_start < bucket_entries.count {
        bucket_end := bucket_start + 1;
        while bucket_end < bucket_entries.count && bucket_entries[bucket_end].hash == bucket_entries[bucket_start].hash  bucket_end += 1;

        for bi: bucket_start..bucket_end-1 {
            i := bucket_entries[bi].idx;
            if parent[i] != i  continue;
            for bj: bi+1..bucket_end-1 {
                j := bucket_entries[bj].idx;
                if parent[j] != j    continue;
                if ids[i] == ids[j]  continue;
                if exprs_equal(ids[i], ids[j], pool.exprs) {
                    parent[j]   = i;
                    counts[i]  += counts[j];
                }
            }
        }
        bucket_start = bucket_end;
    }

    made_progress := false;
    for i: 0..ids.count-1 {
        if parent[i] != i                        continue;  // not the class representative
        if counts[i] < 2                         continue;  // not shared enough to CSE
        if cse_expr_size(ids[i], pool.exprs) == 0   continue;  // trivial (leaf) node

        // temp_name needs to survive past the function's temp storage marker.
        temp_name := copy_string(tprint("%1%2", prefix, counter.*));
        counter.* += 1;

        ex       := pool.exprs[ids[i]];
        saved_id := ifx is_unary(ex.kind)
                    then make_unary(ex.kind, ex.left, pool)
                    else expr_bin(ex.kind, ex.left, ex.right, pool);
        array_add(cse_vars, .{ name = temp_name, expr_id = saved_id });

        temp_sym_id := expr_symbol(temp_name, pool);
        pool.exprs[ids[i]] = pool.exprs[temp_sym_id];
        for j: 0..ids.count-1 {
            if parent[j] == i && j != i
                pool.exprs[ids[j]] = pool.exprs[temp_sym_id];
        }
        made_progress = true;
    }
    return made_progress;
}

expr_is_loop_variant :: (e: Expr_ID, exprs: [] Expr,
                         base_variant_names: [] string,
                         cse_variant_names:  [] string) -> bool {
    ex := exprs[e];
    if ex.kind == .CONST || ex.kind == .NULL  return false;
    if ex.kind == .SYMBOL {
        for n : base_variant_names  if ex.name == n  return true;
        for n : cse_variant_names   if ex.name == n  return true;
        return false;
    }
    if is_unary(ex.kind) {
        return expr_is_loop_variant(ex.left, exprs, base_variant_names, cse_variant_names);
    }
    return expr_is_loop_variant(ex.left,  exprs, base_variant_names, cse_variant_names)
        || expr_is_loop_variant(ex.right, exprs, base_variant_names, cse_variant_names);
}

// depends: does this expr reference the basis at all. linear: if it does,
// is it an affine combination of it (superposition holds), so a basis-vector
// loop could extract it into per-output coefficients.
Basis_Class :: struct {
    depends: bool;
    linear:  bool;
}

// Walk a (topologically-sorted) region's expr and classify it against a
// chosen basis. `classified` accumulates one entry per already-visited
// item name in the same region. `other_variant_names` is anything we
// know is sample-variant but ISN'T part of the chosen basis.
classify_expr_for_basis :: (e: Expr_ID, exprs: [] Expr,
                            basis_names: [] string,
                            other_variant_names: [] string,
                            classified: Table(string, Basis_Class)) -> Basis_Class {
    ex := exprs[e];
    if ex.kind == .CONST || ex.kind == .NULL  return .{false, true};

    if ex.kind == .SYMBOL {
        for n : basis_names  if ex.name == n  return .{true, true};
        found, cls := table_find(*classified, ex.name);
        if found  return cls;
        for n : other_variant_names  if ex.name == n  return .{true, false};
        return .{false, true}; // an outer-loop param/constant symbol
    }

    if ex.kind == .NEG  return classify_expr_for_basis(ex.left, exprs, basis_names, other_variant_names, classified);

    if ex.kind == .EXP || ex.kind == .LOG {
        child := classify_expr_for_basis(ex.left, exprs, basis_names, other_variant_names, classified);
        if child.depends  return .{true, false}; // transcendental of a variant arg, never linear
        return .{false, true};
    }

    l := classify_expr_for_basis(ex.left,  exprs, basis_names, other_variant_names, classified);
    r := classify_expr_for_basis(ex.right, exprs, basis_names, other_variant_names, classified);

    if ex.kind == .ADD || ex.kind == .SUB {
        return .{ l.depends || r.depends, l.linear && r.linear };
    }
    if ex.kind == .MUL {
        if l.depends && r.depends  return .{true, false}; // product of two variants
        if l.depends  return .{true, l.linear};
        if r.depends  return .{true, r.linear};
        return .{false, true};
    }
    if ex.kind == .DIV {
        if r.depends  return .{true, false}; // variant denominator, never linear
        return .{l.depends, l.linear};
    }
    // POW: a variant base or exponent is never linear (even x^2 isn't, no superposition).
    if l.depends || r.depends  return .{true, false};
    return .{false, true};
}

// `prefix` names the generated temps ("_t0", "_t1", ... by default). Pass a
// distinct prefix if you're emitting more than one cse()'d block into the
// same C/C++ scope, or their temp names will collide.
cse :: (gen_var_ids: [] Expr_ID, pool: *Expr_Pool, prefix: string = "_t") -> [..] CSE_Var {
    cse_vars: [..] CSE_Var;
    counter: int = 0;
    MAX_PASSES :: 20;
    for 0..MAX_PASSES-1 {
        if !cse_pass(gen_var_ids, *cse_vars, pool, *counter, prefix)  break;
    }
    return cse_vars;
}


// Sort cse_vars in-place into topological order: each temp appears after all
// other temps whose names it references.  A single forward pass over cse_vars
// is then sufficient to propagate loop-variant-ness and to emit declarations
// in the correct dependency order.
sort_cse_topological :: (cse_vars: *[..] CSE_Var, exprs: [] Expr) {
    sorted:  [..] CSE_Var;
    emitted: [..] bool;
    for _ : 0..cse_vars.count-1  array_add(*emitted, false);

    name_to_idx: Table(string, int);
    for v, i : cse_vars.*  table_add(*name_to_idx, v.name, i);
    assert(name_to_idx.count == cse_vars.count, "sort_cse_topological: duplicate CSE_Var name");

    expr_ids: [..] Expr_ID;
    expr_ids.allocator = temp;
    for v : cse_vars.*  array_add(*expr_ids, v.expr_id);
    deps_flat, deps_offsets := collect_symbol_names_csr(expr_ids, exprs);

    remaining := cse_vars.count;
    while remaining > 0 {
        made_progress := false;
        for i : 0..cse_vars.count-1 {
            if emitted[i]  continue;
            ready := true;
            for dep : deps_csr_slice(deps_flat, deps_offsets, i) {
                found, j := table_find(*name_to_idx, dep);
                if found && j != i && !emitted[j] {
                    ready = false;
                    break;
                }
            }
            if ready {
                emitted[i] = true;
                remaining -= 1;
                made_progress = true;
                array_add(*sorted, cse_vars.*[i]);
            }
        }
        if !made_progress  break;  // Cycle guard.
    }
    cse_vars.* = sorted;
}
// Walk a loop-variant expression and hoist any loop-invariant sub-expressions
// into new_vars as new CSE temporaries (even if they appear only once).
licm_walk :: (e: Expr_ID, pool: *Expr_Pool,
              new_vars: *[..] CSE_Var, counter: *int,
              variant_base: [] string, variant_cse_names: [] string, prefix: string) -> bool {
    ex := (pool.exprs)[e];
    if ex.kind == .CONST || ex.kind == .SYMBOL || ex.kind == .NULL  return false;

    made_progress := false;
    n_children := ifx is_unary(ex.kind) then 1 else 2;
    for ci : 0..n_children-1 {
        child_id := ifx ci == 0 then ex.left else ex.right;
        ch := (pool.exprs)[child_id];
        if ch.kind == .CONST || ch.kind == .SYMBOL || ch.kind == .NULL  continue;
        if !expr_is_loop_variant(child_id, pool.exprs, variant_base, variant_cse_names) {
            // Loop-invariant compound sub-expression inside a loop-variant context: hoist it.
            temp_name := tprint("%1%2", prefix, counter.*);  counter.* += 1;
            saved_id  := ifx is_unary(ch.kind)
                         then make_unary(ch.kind, ch.left, pool)
                         else expr_bin(ch.kind, ch.left, ch.right, pool);
            array_add(new_vars, .{ name = temp_name, expr_id = saved_id });
            (pool.exprs)[child_id] = (pool.exprs)[expr_symbol(temp_name, pool)];
            made_progress = true;
        } else if licm_walk(child_id, pool, new_vars, counter, variant_base, variant_cse_names, prefix) {
            made_progress = true;
        }
    }

    return made_progress;
}

// Loop-Invariant Code Motion pass: extract loop-invariant sub-expressions from
// loop-variant expressions, even when they appear only once.  Newly extracted
// temporaries are appended to cse_vars; codegen will hoist them outside the
// per-sample loop because they classify as loop-invariant. `prefix` must
// match whatever prefix cse_vars' own names were generated with (see cse()).
licm :: (gen_var_ids: [] Expr_ID, cse_vars: *[..] CSE_Var, pool: *Expr_Pool,
         variant_base: [] string, prefix: string = "_t") {
    // Propagate variant-ness through the existing (topologically ordered) CSE temps
    // so that expr_is_loop_variant correctly handles temps that depend on other temps.
    variant_cse_names: [..] string;
    for cv : cse_vars.* {
        if expr_is_loop_variant(cv.expr_id, pool.exprs, variant_base, variant_cse_names)
            array_add(*variant_cse_names, cv.name);
    }

    new_vars: [..] CSE_Var;
    counter := cse_vars.count;   // Name LICM temps after existing CSE temps to avoid clashes.

    MAX_PASSES :: 10;
    for 0..MAX_PASSES-1 {
        made_progress := false;

        for gv_id : gen_var_ids {
            if expr_is_loop_variant(gv_id, pool.exprs, variant_base, variant_cse_names) {
                if licm_walk(gv_id, pool, *new_vars, *counter, variant_base, variant_cse_names, prefix)
                    made_progress = true;
            }
        }

        for cv : cse_vars.* {
            if expr_is_loop_variant(cv.expr_id, pool.exprs, variant_base, variant_cse_names) {
                if licm_walk(cv.expr_id, pool, *new_vars, *counter, variant_base, variant_cse_names, prefix)
                    made_progress = true;
            }
        }

        if !made_progress  break;
    }

    // Append newly extracted loop-invariant temporaries to cse_vars.
    for nv : new_vars  array_add(cse_vars, nv);
}


// Collect all SYMBOL names referenced anywhere in expression e (recursively).
collect_symbol_names :: (e: Expr_ID, exprs: [] Expr, names: *[..] string) {
    ex := exprs[e];
    if ex.kind == .SYMBOL {
        array_add_if_unique(names, ex.name);
        return;
    }
    if ex.kind == .CONST || ex.kind == .NULL  return;
    if is_unary(ex.kind) {
        collect_symbol_names(ex.left, exprs, names);
        return;
    }
    collect_symbol_names(ex.left,  exprs, names);
    collect_symbol_names(ex.right, exprs, names);
}

// Batch/CSR version of collect_symbol_names: item i's referenced names are
// flat[offsets[i]..offsets[i+1]-1].
collect_symbol_names_csr :: (expr_ids: [] Expr_ID, exprs: [] Expr) -> (flat: [..] string, offsets: [..] int) {
    flat: [..] string;
    offsets: [..] int;
    array_reserve(*offsets, expr_ids.count + 1);
    array_add(*offsets, 0);
    for id : expr_ids {
        mark := get_temporary_storage_mark();
        scratch: [..] string;
        scratch.allocator = temp;
        collect_symbol_names(id, exprs, *scratch);
        for name : scratch  array_add(*flat, name);
        array_add(*offsets, flat.count);
        set_temporary_storage_mark(mark);
    }
    return flat, offsets;
}

deps_csr_slice :: inline (flat: [] string, offsets: [] int, i: int) -> [] string {
    return array_view(flat, offsets[i], offsets[i + 1] - offsets[i]);
}


// Counts, for every name in `name_to_idx`, how many times it shows up as a
// *direct* divisor (right child of a DIV) vs. anywhere else in `e`.
// div_counts/other_counts are parallel to name_to_idx's values.
//
// One walk checks all names at once and is memoized per call, to avoid doing
// redundant re-walks of the same expressions. Memoizing repeat-visits is safe
// here since callers only ever check the counts against zero, never the exact value.
count_sym_uses_multi :: (e: Expr_ID, name_to_idx: *Table(string, int), exprs: [] Expr,
                         div_counts: [] int, other_counts: [] int) {
    visited: Table(Expr_ID, bool);
    count_sym_uses_multi_impl(e, name_to_idx, exprs, div_counts, other_counts, *visited);
    deinit(*visited);
}

count_sym_uses_multi_impl :: (e: Expr_ID, name_to_idx: *Table(string, int), exprs: [] Expr,
                              div_counts: [] int, other_counts: [] int, visited: *Table(Expr_ID, bool)) {
    if table_contains(visited, e)  return;
    table_add(visited, e, true);

    ex := exprs[e];
    if ex.kind == .CONST || ex.kind == .NULL  return;
    if ex.kind == .SYMBOL {
        found, idx := table_find(name_to_idx, ex.name);
        if found  other_counts[idx] += 1;
        return;
    }
    if is_unary(ex.kind) {
        count_sym_uses_multi_impl(ex.left, name_to_idx, exprs, div_counts, other_counts, visited);
        return;
    }
    if ex.kind == .DIV {
        // Treat the right (divisor) child specially.
        right_ex := exprs[ex.right];
        if right_ex.kind == .SYMBOL {
            found, idx := table_find(name_to_idx, right_ex.name);
            if found  div_counts[idx] += 1;
        } else {
            count_sym_uses_multi_impl(ex.right, name_to_idx, exprs, div_counts, other_counts, visited);
        }
        count_sym_uses_multi_impl(ex.left, name_to_idx, exprs, div_counts, other_counts, visited);
        return;
    }
    count_sym_uses_multi_impl(ex.left,  name_to_idx, exprs, div_counts, other_counts, visited);
    count_sym_uses_multi_impl(ex.right, name_to_idx, exprs, div_counts, other_counts, visited);
}

// In expression tree e, replace every DIV(A, sym_name) with MUL(A, recip_sym_id).
replace_div_by_name :: (e: Expr_ID, sym_name: string, recip_sym_id: Expr_ID, pool: *Expr_Pool) {
    ex := (pool.exprs)[e];
    if ex.kind == .CONST || ex.kind == .SYMBOL || ex.kind == .NULL  return;
    if is_unary(ex.kind) {
        replace_div_by_name(ex.left, sym_name, recip_sym_id, pool);
        return;
    }
    if ex.kind == .DIV {
        right_ex := (pool.exprs)[ex.right];
        if right_ex.kind == .SYMBOL && right_ex.name == sym_name {
            (pool.exprs)[e].kind  = .MUL;
            (pool.exprs)[e].right = recip_sym_id;
            replace_div_by_name(ex.left, sym_name, recip_sym_id, pool);
        } else {
            replace_div_by_name(ex.left,  sym_name, recip_sym_id, pool);
            replace_div_by_name(ex.right, sym_name, recip_sym_id, pool);
        }
        return;
    }
    replace_div_by_name(ex.left,  sym_name, recip_sym_id, pool);
    replace_div_by_name(ex.right, sym_name, recip_sym_id, pool);
}

// Pre-compute reciprocals of loop-invariant expressions that appear as divisors
// in the inner sample loop, so every inner-loop divide becomes a multiply.
//
// For each outer-loop CSE temp _tN used as a direct divisor (x / _tN):
//   - If _tN is used ONLY as a divisor in the inner loop AND is not referenced by
//     any other outer-loop expression: flip _tN in place to (1 / original) and
//     replace every inner-loop (x / _tN) with (x * _tN).
//   - Otherwise: add a new outer-loop temp (1 / _tN) and replace inner-loop
//     (x / _tN) with (x * new_temp).
// `prefix` must match whatever prefix cse_vars' own names were generated
// with (see cse()); new dedicated-reciprocal temporaries are named after it.
recip_hoist :: (gen_var_ids: [] Expr_ID, cse_vars: *[..] CSE_Var, pool: *Expr_Pool,
                variant_base: [] string, prefix: string = "_t") {
    // Classify each CSE temp as inner (loop-variant) or outer (loop-invariant).
    variant_cse_names: [..] string;
    variant_set: Table(string, bool);
    for cv : cse_vars.* {
        if expr_is_loop_variant(cv.expr_id, pool.exprs, variant_base, variant_cse_names) {
            array_add(*variant_cse_names, cv.name);
            table_add(*variant_set, cv.name, true);
        }
    }

    // Build a usage-info record for every outer-loop CSE temp.
    Div_Info :: struct {
        name:        string;
        cse_idx:     int;   // Position in cse_vars (for in-place flip).
        inner_div:   int;   // Times used as direct divisor in inner-loop expressions.
        inner_other: int;   // Times used in non-divisor position in inner-loop expressions.
        outer_refs:  int;   // Times referenced (any position) by other outer-loop expressions.
    }
    infos: [..] Div_Info;
    name_to_idx: Table(string, int);
    for cv, idx : cse_vars.* {
        if !table_contains(*variant_set, cv.name) {
            table_add(*name_to_idx, cv.name, infos.count);
            array_add(*infos, .{ name = cv.name, cse_idx = idx });
        }
    }
    if infos.count == 0  return;

    // Count inner-loop uses: one walk per expression, all names at once.
    inner_div:   [..] int;
    inner_other: [..] int;
    array_resize(*inner_div,   infos.count);
    array_resize(*inner_other, infos.count);
    for gv_id : gen_var_ids
        count_sym_uses_multi(gv_id, *name_to_idx, pool.exprs, inner_div, inner_other);
    for cv : cse_vars.* {
        if table_contains(*variant_set, cv.name)
            count_sym_uses_multi(cv.expr_id, *name_to_idx, pool.exprs, inner_div, inner_other);
    }
    for i : 0..infos.count-1 {
        infos[i].inner_div   = inner_div[i];
        infos[i].inner_other = inner_other[i];
    }

    // Count references in *other* outer-loop CSE expressions (guards the in-place flip).
    for cv : cse_vars.* {
        if table_contains(*variant_set, cv.name)  continue;
        d: [..] int; o: [..] int;
        array_resize(*d, infos.count);
        array_resize(*o, infos.count);
        count_sym_uses_multi(cv.expr_id, *name_to_idx, pool.exprs, d, o);
        for j : 0..infos.count-1 {
            if infos[j].name == cv.name  continue;
            infos[j].outer_refs += d[j] + o[j];
        }
    }

    counter := cse_vars.count;

    for info : infos {
        if info.inner_div == 0  continue;

        sym_id := expr_symbol(info.name, pool);

        if info.inner_other == 0 && info.outer_refs == 0 {
            // Safe to flip: temp is used only as a divisor and nothing else depends on it.
            one_id := expr_const(1, pool);
            cse_vars.*[info.cse_idx].expr_id =
                expr_bin(.DIV, one_id, cse_vars.*[info.cse_idx].expr_id, pool);
            for gv_id : gen_var_ids  replace_div_by_name(gv_id, info.name, sym_id, pool);
            for cv : cse_vars.* {
                if table_contains(*variant_set, cv.name)
                    replace_div_by_name(cv.expr_id, info.name, sym_id, pool);
            }
        } else {
            // Temp is used elsewhere too; add a dedicated reciprocal.
            recip_name := tprint("%1%2", prefix, counter);
            counter += 1;
            one_id := expr_const(1, pool);
            recip_expr_id := expr_bin(.DIV, one_id, sym_id, pool);
            array_add(cse_vars, .{ name = recip_name, expr_id = recip_expr_id });

            recip_sym_id := expr_symbol(recip_name, pool);
            for gv_id : gen_var_ids  replace_div_by_name(gv_id, info.name, recip_sym_id, pool);
            for cv : cse_vars.* {
                if table_contains(*variant_set, cv.name)
                    replace_div_by_name(cv.expr_id, info.name, recip_sym_id, pool);
            }
        }
    }
}
