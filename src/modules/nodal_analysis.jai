#import "Basic";
#import "sym_algebra";

Element_Type :: enum {
    RESISTOR;
    CAPACITOR;
    INDUCTOR;
    AMP;
    RC_SERIES;
    RC_PARALLEL;
}

Combo_Op_Type :: enum {
    NONE;       // leaf: use the element's own name as the param directly
    SUM;        // a + b  (series R+R, parallel C||C)
    RECIP_SUM;  // a*b/(a+b)  (series C+C, parallel R||R)
}

// Recursive expression tree for a combined parameter value.
// Leaf nodes (op == .NONE) hold a single param name.
// Branch nodes combine two sub-expressions.
Param_Expr :: struct {
    op:        Combo_Op_Type;
    leaf_name: string;       // valid only when op == .NONE
    left:      *Param_Expr;  // valid only when op != .NONE
    right:     *Param_Expr;  // valid only when op != .NONE
}

// Stores the R and C identity info for a combined RC element.
RC_Internal :: struct {
    res_name: string;
    res_expr: *Param_Expr;
    cap_name: string;
    cap_expr: *Param_Expr;
}

Element :: struct {
    name: string;

    type: Element_Type;

    terminals: [..] string;

    // For same-type combined elements (R+R, C+C, R||R, C||C):
    // non-null when this element's parameter is a combo expression.
    combo_expr: *Param_Expr;  // null => use params.{name} directly

    // For RC_SERIES / RC_PARALLEL combined elements.
    rc_internal: RC_Internal;
};

// Return a Param_Expr leaf for an element.
// If the element already carries a combo_expr, return that; otherwise make a fresh leaf.
get_param_expr :: (el: Element) -> *Param_Expr {
    if el.combo_expr  return el.combo_expr;
    expr := New(Param_Expr);
    expr.op = .NONE;
    expr.leaf_name = el.name;
    return expr;
}

make_combo_expr :: (op: Combo_Op_Type, left: *Param_Expr, right: *Param_Expr) -> *Param_Expr {
    expr := New(Param_Expr);
    expr.op    = op;
    expr.left  = left;
    expr.right = right;
    return expr;
}

// Build an RC_SERIES Element from a resistor element and a capacitor element,
// given the "other" terminal for each (the terminal NOT shared with the internal node).
make_rc_series_element :: (res_el: Element, res_other: string,
                           cap_el: Element, cap_other: string) -> Element {
    new_el: Element;
    new_el.name = tprint("%1%2", res_el.name, cap_el.name);
    new_el.type = .RC_SERIES;
    array_add(*new_el.terminals, res_other);
    array_add(*new_el.terminals, cap_other);
    new_el.rc_internal.res_name = res_el.name;
    new_el.rc_internal.res_expr = get_param_expr(res_el);
    new_el.rc_internal.cap_name = cap_el.name;
    new_el.rc_internal.cap_expr = get_param_expr(cap_el);
    return new_el;
}

// Attempt ONE series combination. Returns true if a combination was made.
// Series reduction: elements sharing a node that is not input, output, or ground,
// and that node has exactly two elements connected to it.
reduce_series_once :: (elements: *[..] Element, inputs: [] string, outputs: [] string) -> bool {
    all_nodes: [..] string;
    defer array_free(all_nodes);
    for el : elements.* {
        for terminal : el.terminals {
            array_add_if_unique(*all_nodes, terminal);
        }
    }

    for node : all_nodes {
        if node == "0" continue;
        if array_find(inputs, node)  continue;
        if array_find(outputs, node) continue;

        connected: [..] int;
        defer array_free(connected);
        for el, idx : elements.* {
            for terminal : el.terminals {
                if terminal == node {
                    array_add(*connected, idx);
                    break;
                }
            }
        }
        if connected.count != 2 continue;

        idx1 := connected[0];
        idx2 := connected[1];
        el1  := (elements.*)[idx1];
        el2  := (elements.*)[idx2];
        other1 := ifx el1.terminals[0] == node then el1.terminals[1] else el1.terminals[0];
        other2 := ifx el2.terminals[0] == node then el2.terminals[1] else el2.terminals[0];

        new_el: Element;
        combined := false;

        if el1.type == .RESISTOR && el2.type == .RESISTOR {
            print("Combining series resistors: % + %\n", el1.name, el2.name);
            new_el.name = tprint("%1%2", el1.name, el2.name);
            new_el.type = .RESISTOR;
            array_add(*new_el.terminals, other1);
            array_add(*new_el.terminals, other2);
            new_el.combo_expr = make_combo_expr(.SUM, get_param_expr(el1), get_param_expr(el2));
            combined = true;
        } else if el1.type == .CAPACITOR && el2.type == .CAPACITOR {
            print("Combining series capacitors: % + %\n", el1.name, el2.name);
            new_el.name = tprint("%1%2", el1.name, el2.name);
            new_el.type = .CAPACITOR;
            array_add(*new_el.terminals, other1);
            array_add(*new_el.terminals, other2);
            new_el.combo_expr = make_combo_expr(.RECIP_SUM, get_param_expr(el1), get_param_expr(el2));
            combined = true;
        } else if el1.type == .RESISTOR && el2.type == .CAPACITOR {
            print("Combining series resistor and capacitor: % + %\n", el1.name, el2.name);
            new_el  = make_rc_series_element(el1, other1, el2, other2);
            combined = true;
        } else if el1.type == .CAPACITOR && el2.type == .RESISTOR {
            print("Combining series resistor and capacitor: % + %\n", el2.name, el1.name);
            new_el  = make_rc_series_element(el2, other2, el1, other1);
            combined = true;
        }

        if combined {
            hi := max(idx1, idx2);
            lo := min(idx1, idx2);
            array_unordered_remove_by_index(elements, hi);
            array_unordered_remove_by_index(elements, lo);
            array_add(elements, new_el);
            return true;
        }
    }
    return false;
}

// Attempt ONE parallel combination. Returns true if a combination was made.
// Priority: R||R, then C||C, then R||C.
reduce_parallel_once :: (elements: *[..] Element) -> bool {
    // Same-type parallel (R||R and C||C)
    for i : 0..elements.count-1 {
        el1 := (elements.*)[i];
        if el1.type != .RESISTOR && el1.type != .CAPACITOR  continue;

        for j : i+1..elements.count-1 {
            el2 := (elements.*)[j];
            if el2.type != el1.type  continue;

            n1a := el1.terminals[0]; n2a := el1.terminals[1];
            n1b := el2.terminals[0]; n2b := el2.terminals[1];
            same_nodes := (n1a == n1b && n2a == n2b) || (n1a == n2b && n2a == n1b);
            if !same_nodes  continue;

            new_el: Element;
            new_el.name = tprint("%1%2", el1.name, el2.name);
            new_el.type = el1.type;
            array_add(*new_el.terminals, n1a);
            array_add(*new_el.terminals, n2a);

            if el1.type == .RESISTOR {
                print("Combining parallel resistors: % || %\n", el1.name, el2.name);
                new_el.combo_expr = make_combo_expr(.RECIP_SUM, get_param_expr(el1), get_param_expr(el2));
            } else {
                print("Combining parallel capacitors: % || %\n", el1.name, el2.name);
                new_el.combo_expr = make_combo_expr(.SUM, get_param_expr(el1), get_param_expr(el2));
            }

            // Remove higher index first to avoid shift issues
            array_unordered_remove_by_index(elements, j);
            array_unordered_remove_by_index(elements, i);
            array_add(elements, new_el);
            return true;
        }
    }

    // Mixed parallel (R||C)
    for i : 0..elements.count-1 {
        el1 := (elements.*)[i];
        if el1.type != .RESISTOR  continue;

        for j : 0..elements.count-1 {
            if j == i  continue;
            el2 := (elements.*)[j];
            if el2.type != .CAPACITOR  continue;

            n1a := el1.terminals[0]; n2a := el1.terminals[1];
            n1b := el2.terminals[0]; n2b := el2.terminals[1];
            same_nodes := (n1a == n1b && n2a == n2b) || (n1a == n2b && n2a == n1b);
            if !same_nodes  continue;

            print("Combining parallel resistor and capacitor: % || %\n", el1.name, el2.name);

            new_el: Element;
            new_el.name = tprint("%1%2", el1.name, el2.name);
            new_el.type = .RC_PARALLEL;
            array_add(*new_el.terminals, n1a);
            array_add(*new_el.terminals, n2a);
            new_el.rc_internal.res_name = el1.name;
            new_el.rc_internal.res_expr = get_param_expr(el1);
            new_el.rc_internal.cap_name = el2.name;
            new_el.rc_internal.cap_expr = get_param_expr(el2);

            hi := max(i, j);
            lo := min(i, j);
            array_unordered_remove_by_index(elements, hi);
            array_unordered_remove_by_index(elements, lo);
            array_add(elements, new_el);
            return true;
        }
    }

    return false;
}

// Reduce the circuit by combining series/parallel elements until no more
// reductions are possible. Mirrors the Python reduce_circuit() function.
reduce_circuit :: (elements: *[..] Element, inputs: [] string, outputs: [] string) {
    prev_count := -1;
    while elements.count != prev_count {
        prev_count = elements.count;
        reduce_series_once(elements, inputs, outputs);
        reduce_parallel_once(elements);
    }
}

// Options controlling the target language for code generation.
Codegen_Options :: struct {
    lang:      string = "cpp";   // "cpp" or "c"
    type_name: string = "float"; // numeric type used for variable declarations
}

// Counts of floating-point operations in a block of generated code.
Flop_Count :: struct {
    muls: int;
    divs: int;
    adds: int; // ADD, SUB, and NEG all counted here
}

// Recursively walk an expression tree and accumulate op counts into `result`.
// Stops at SYMBOL and CONST leaves; does not cross into sub-expressions that
// are already named CSE variables (they appear as SYMBOL nodes).
count_flops_expr :: (e: Expr_ID, exprs: [] Expr, result: *Flop_Count) {
    if e == NO_EXPR  return;
    kind := exprs[e].kind;
    if      kind == .MUL                                    result.muls += 1;
    else if kind == .DIV                                    result.divs += 1;
    else if kind == .ADD || kind == .SUB || kind == .NEG    result.adds += 1;

    if kind == .ADD || kind == .SUB || kind == .MUL || kind == .DIV {
        count_flops_expr(exprs[e].left,  exprs, result);
        count_flops_expr(exprs[e].right, exprs, result);
    } else if kind == .NEG {
        count_flops_expr(exprs[e].left, exprs, result);
    }
}

count_flops :: (items: [] Gen_Var, exprs: [] Expr) -> Flop_Count {
    result: Flop_Count;
    for item : items  count_flops_expr(item.expr_id, exprs, *result);
    return result;
}

// A generated variable: a named expression that will become a `const auto` (C++) or `const <type>` (C).
Gen_Var :: struct {
    name:    string;
    expr_id: Expr_ID;
}

generate_kcl_equations :: (elements: [] Element, inputs: [] string) -> []Expr_ID, []Expr_ID, [..]Expr {
    eqn_nodes: [..] string;

    for el : elements {
        for terminal : el.terminals {
            if terminal == "0" continue;
            if array_find(inputs, terminal) continue;
            array_add_if_unique(*eqn_nodes, terminal);
        }
    }

    exprs : [..] Expr;
    equations : [..] Expr_ID;
    for _ : eqn_nodes array_add(*equations, NO_EXPR);

    Sub_Expr :: struct {
        id: Expr_ID;
        replace_id: Expr_ID;
    }
    currents: [..] Sub_Expr;
    voltages: [..] Sub_Expr;

    solve_vars: [..]Expr_ID;
    for node : eqn_nodes array_add(*solve_vars, expr_symbol(node, *exprs));

    // all linear elemements beside op-amps
    for el : elements {
        if el.type == .AMP continue;

        pos_node := el.terminals[0];
        neg_node := el.terminals[1];
        current_symbol := expr_symbol(tprint("i%", el.name), *exprs);
        pos_node_symbol := ifx pos_node == "0" then expr_const(0, *exprs) else expr_symbol(pos_node, *exprs);
        neg_node_symbol := ifx neg_node == "0" then expr_const(0, *exprs) else expr_symbol(neg_node, *exprs);

        for node, idx : eqn_nodes {
            if node == pos_node equations[idx] = combine(equations[idx], current_symbol, .SUB, *exprs); // current leaving this node
            if node == neg_node equations[idx] = combine(equations[idx], current_symbol, .ADD, *exprs); // current entering this node
        }

        current_expr := NO_EXPR;
        impedance_symbol := expr_symbol(tprint("g%", el.name), *exprs);
        state_symbol := expr_symbol(tprint("z%", el.name), *exprs);
        if el.type == {
            case .RESISTOR;
                current_expr = expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *exprs), *exprs);
            case .CAPACITOR;
                current_expr = expr_sub(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *exprs), *exprs), state_symbol, *exprs);
            case .INDUCTOR;
                current_expr = expr_add(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *exprs), *exprs), state_symbol, *exprs);
            case .RC_SERIES;
                current_expr = expr_sub(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *exprs), *exprs), state_symbol, *exprs);
            case .RC_PARALLEL;
                current_expr = expr_sub(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *exprs), *exprs), state_symbol, *exprs);
        }
        // assert(current_expr != NO_EXPR);
        if current_expr != NO_EXPR array_add(*currents, .{ current_symbol, current_expr });
    }

    // @TODO: I am not sure this is doing anything?
    substitute_node :: (node_to_sub: string, replacement_node: string) #expand {
        for *el : elements {
            for node, node_idx : el.terminals {
                if node == node_to_sub {
                    el.terminals[node_idx] = replacement_node;
                }
            }
        }
    }

    // just op-amps
    for el : elements {
        if el.type != .AMP continue;
        out_node := el.terminals[0];
        neg_node := el.terminals[1];
        pos_node := el.terminals[2];

        current_symbol := expr_symbol(tprint("i%", el.name), *exprs);
        for node, idx : eqn_nodes {
            if node == out_node  equations[idx] = combine(equations[idx], current_symbol, .ADD, *exprs); // current entering this node
        }
        array_add(*solve_vars, current_symbol);

        if pos_node == "0" {
            print("Setting % → 0\n", neg_node);
            array_add(*voltages, .{ expr_symbol(neg_node, *exprs), expr_const(0, *exprs) });
            array_unordered_remove_by_value(*solve_vars, expr_symbol(neg_node, *exprs));
            substitute_node(neg_node, "0");
        } else if neg_node == "0" {
            print("Setting % → 0\n", pos_node);
            array_add(*voltages, .{ expr_symbol(pos_node, *exprs), expr_const(0, *exprs) });
            array_unordered_remove_by_value(*solve_vars, expr_symbol(pos_node, *exprs));
            substitute_node(pos_node, "0");
        } else {
            print("Setting % → %\n", pos_node, neg_node);
            array_add(*voltages, .{ expr_symbol(pos_node, *exprs), expr_symbol(neg_node, *exprs) });
            array_unordered_remove_by_value(*solve_vars, expr_symbol(pos_node, *exprs));
            substitute_node(pos_node, neg_node);
        }
    }

    // substitute current and voltage expressions
    for current_sub : currents  substitute(current_sub.id, current_sub.replace_id, *exprs);
    for voltage_sub : voltages  substitute(voltage_sub.id, voltage_sub.replace_id, *exprs);

    return solve_vars, equations, exprs;
}

generate_expressions :: (elements: [] Element,
                         outputs: [] string,
                         solve_vars: [] Expr_ID,
                         solutions: [] Expr_ID,
                         exprs: *[..] Expr) -> [] Gen_Var {
    gen_vars: [..] Gen_Var;

    add_gen_var :: (name: string, e: Expr_ID) #expand {
        array_add(*gen_vars, Gen_Var.{ name = name, expr_id = e });
    }

    add_expression_for_node :: (node: string) #expand {
        node_symbol := expr_symbol(node, exprs);
        found_solve_var, solve_idx := array_find(solve_vars, node_symbol);
        if !found_solve_var  return;

        for gv : gen_vars  if gv.name == node  return;
        add_gen_var(node, simplify(solutions[solve_idx], exprs));
    }

    for output : outputs {
        add_expression_for_node(output);
    }

    for el : elements {
        if el.type == {
            case .CAPACITOR; #through;
            case .INDUCTOR;
                pos_node := el.terminals[0];
                neg_node := el.terminals[1];
                add_expression_for_node(pos_node);
                add_expression_for_node(neg_node);

                expr_name := tprint("t%", el.name);
                add_gen_var(expr_name, expr_mul(tprint("g%", el.name), expr_sub(pos_node, neg_node, exprs), exprs));

            case .RC_SERIES; #through;
            case .RC_PARALLEL;
                pos_node := el.terminals[0];
                neg_node := el.terminals[1];
                add_expression_for_node(pos_node);
                add_expression_for_node(neg_node);

                expr_name := tprint("v%", el.name);
                add_gen_var(expr_name, expr_sub(pos_node, neg_node, exprs));
        }
    }

    return gen_vars;
}

// Build the set of base loop-variant symbol names: inputs + state variables.
build_variant_base :: (elements: [] Element, inputs: [] string) -> [..] string {
    names: [..] string;
    for inp : inputs  array_add(*names, inp);
    for el : elements {
        if el.type == .CAPACITOR || el.type == .INDUCTOR
           || el.type == .RC_SERIES || el.type == .RC_PARALLEL {
            array_add(*names, tprint("z%", el.name));
        }
    }
    return names;
}

// Emit items into builder as `const auto name = expr;` lines in topological
// order: each item is written only after all inner-loop symbol dependencies it
// references have already been written.
emit_topological_order :: (items: [] Gen_Var, exprs: [] Expr, builder: *String_Builder, options: Codegen_Options = .{}) {
    decl_type := ifx options.lang == "c" then options.type_name else "auto";
    emitted: [..] bool;
    for _ : 0..items.count-1  array_add(*emitted, false);

    remaining := items.count;
    while remaining > 0 {
        made_progress := false;
        for i : 0..items.count-1 {
            if emitted[i]  continue;
            deps: [..] string;
            collect_symbol_names(items[i].expr_id, exprs, *deps);
            ready := true;
            for dep : deps {
                for j : 0..items.count-1 {
                    if j != i && items[j].name == dep && !emitted[j] {
                        ready = false;
                        break;
                    }
                }
                if !ready  break;
            }
            if ready {
                emitted[i]  = true;
                remaining  -= 1;
                made_progress = true;
                print_to_builder(builder, "const % % = %;\n",
                                 decl_type, items[i].name, print_expr(items[i].expr_id, exprs));
            }
        }
        if !made_progress  break;  // Cycle guard.
    }
}

// Generate code for the given gen_vars and CSE temporaries.
//
// Returns two strings:
//   outer_code — loop-invariant CSE temporaries (hoist outside the sample loop)
//   inner_code — loop-variant CSE temps + gen_var assignments + state updates
codegen :: (elements: [] Element, inputs: [] string,
            gen_vars: [] Gen_Var, cse_vars: [] CSE_Var,
            exprs: [] Expr, options: Codegen_Options = .{}) -> (outer_code: string, inner_code: string, outer_flops: Flop_Count, inner_flops: Flop_Count) {

    variant_base := build_variant_base(elements, inputs);
    // Gen_vars are computed inside the per-sample loop; treat their names as
    // loop-variant base symbols so that any CSE temp depending on them is
    // correctly classified as loop-variant.
    for gv : gen_vars  array_add(*variant_base, gv.name);

    // Accumulate names of CSE temps that are loop-variant so we can propagate
    // variant-ness through the (topologically ordered) temp list.
    variant_cse_names: [..] string;

    outer_builder: String_Builder;
    inner_builder: String_Builder;

    // Collect loop-variant CSE temps + gen_vars (inner) and loop-invariant CSE
    // temps (outer) separately; both will be emitted in topological order so
    // that every temporary is declared after all the items it depends on.
    outer_items: [..] Gen_Var;
    inner_items: [..] Gen_Var;

    for cv : cse_vars {
        if expr_is_loop_variant(cv.expr_id, exprs, variant_base, variant_cse_names) {
            array_add(*variant_cse_names, cv.name);
            array_add(*inner_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
        } else {
            array_add(*outer_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
        }
    }

    for gv : gen_vars {
        array_add(*inner_items, gv);
    }

    // Count FLOPs for both groups before emitting.
    outer_flops := count_flops(outer_items, exprs);
    inner_flops := count_flops(inner_items, exprs);

    // Emit both groups in topological order (dependencies before uses).
    emit_topological_order(outer_items, exprs, *outer_builder, options);
    emit_topological_order(inner_items, exprs, *inner_builder, options);
    print_to_builder(*inner_builder, "\n");

    for el : elements {
        if el.type == {
            case .CAPACITOR;
                print_to_builder(*inner_builder, "z%1 = 2 * t%1 - z%1;\n", el.name);
                inner_flops.muls += 1;  inner_flops.adds += 1;  // 2*t, minus
            case .INDUCTOR;
                print_to_builder(*inner_builder, "z%1 = 2 * t%1 + z%1;\n", el.name);
                inner_flops.muls += 1;  inner_flops.adds += 1;  // 2*t, plus
            case .RC_SERIES;
                print_to_builder(*inner_builder, "z%1 = g%1 * (1 - gn%1) * v%1 + gn%1 * z%1; // RC series\n", el.name);
                inner_flops.muls += 3;  inner_flops.adds += 2;  // 3 muls, (1-gn), plus
            case .RC_PARALLEL;
                print_to_builder(*inner_builder, "z%1 = gz%1 * v%1 - z%1; // RC parallel\n", el.name);
                inner_flops.muls += 1;  inner_flops.adds += 1;  // gz*v, minus
        }
    }

    return builder_to_string(*outer_builder), builder_to_string(*inner_builder), outer_flops, inner_flops;
}

solve_and_codegen :: (elements: *[..] Element, inputs: [] string, outputs: [] string, options: Codegen_Options = .{}) -> (outer_code: string, inner_code: string, outer_flops: Flop_Count, inner_flops: Flop_Count) {
    reduce_circuit(elements, inputs, outputs);

    solve_vars, equations, exprs := generate_kcl_equations(elements.*, inputs);

    solutions := solve(equations, solve_vars, *exprs);

    gen_vars := generate_expressions(elements.*, outputs, solve_vars, solutions, *exprs);

    // Collect gen_var expression IDs for CSE.
    gen_var_ids: [..] Expr_ID;
    for gv : gen_vars  array_add(*gen_var_ids, gv.expr_id);

    cse_vars := cse(gen_var_ids, *exprs);
    sort_cse_topological(*cse_vars, exprs);  // ensure deps appear before dependents

    // Build the variant base for LICM (inputs + state vars + gen_var names).
    licm_variant_base := build_variant_base(elements.*, inputs);
    for gv : gen_vars  array_add(*licm_variant_base, gv.name);
    licm(gen_var_ids, *cse_vars, *exprs, licm_variant_base);
    recip_hoist(gen_var_ids, *cse_vars, *exprs, licm_variant_base);

    outer_code, inner_code, outer_flops, inner_flops := codegen(elements.*, inputs, gen_vars, cse_vars, exprs, options);
    return outer_code, inner_code, outer_flops, inner_flops;
}
