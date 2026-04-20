#import "Basic";
#import "solve";

Element_Type :: enum {
    RESISTOR;
    CAPACITOR;
    INDUCTOR;
    AMP;
    RC_SERIES;
    RC_PARALLEL;
}

Element :: struct {
    name: string;

    type: Element_Type;

    terminals: [..] string;
};

// A generated variable: a named expression that will become a C++ `const auto`.
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
                current_expr = expr_add(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *exprs), *exprs), state_symbol, *exprs);
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

// Generate code for the given gen_vars and CSE temporaries.
//
// Returns two strings:
//   outer_code — loop-invariant CSE temporaries (hoist outside the sample loop)
//   inner_code — loop-variant CSE temps + gen_var assignments + state updates
codegen :: (elements: [] Element, inputs: [] string,
            gen_vars: [] Gen_Var, cse_vars: [] CSE_Var,
            exprs: [] Expr) -> (outer_code: string, inner_code: string) {

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

    // Collect all inner-loop items: loop-variant CSE temps + gen_vars.
    // We topologically sort them together so that a CSE temp that references
    // a gen_var name (e.g. _t6 = vn9 - v36) is emitted *after* that gen_var.
    inner_items: [..] Gen_Var;

    for cv : cse_vars {
        if expr_is_loop_variant(cv.expr_id, exprs, variant_base, variant_cse_names) {
            array_add(*variant_cse_names, cv.name);
            array_add(*inner_items, Gen_Var.{ name = cv.name, expr_id = cv.expr_id });
        } else {
            print_to_builder(*outer_builder, "const auto % = %;\n", cv.name, print_expr(cv.expr_id, exprs));
        }
    }

    for gv : gen_vars {
        array_add(*inner_items, gv);
    }

    // Build the flat list of inner item names for dependency look-up.
    inner_names: [..] string;
    for item : inner_items  array_add(*inner_names, item.name);

    // Iterative topological sort: emit each item once all of its inner-loop
    // symbol dependencies have already been emitted.
    emitted: [..] bool;
    for _ : 0..inner_items.count-1  array_add(*emitted, false);

    remaining := inner_items.count;
    while remaining > 0 {
        made_progress := false;
        for i : 0..inner_items.count-1 {
            if emitted[i]  continue;
            refs: [..] string;
            collect_symbol_names(inner_items[i].expr_id, exprs, *refs);
            ready := true;
            for ref : refs {
                found, j := array_find(inner_names, ref);
                if found && j != i && !emitted[j] {
                    ready = false;
                    break;
                }
            }
            if ready {
                emitted[i] = true;
                remaining -= 1;
                made_progress = true;
                print_to_builder(*inner_builder, "const auto % = %;\n",
                                 inner_items[i].name, print_expr(inner_items[i].expr_id, exprs));
            }
        }
        if !made_progress  break;   // Safety exit -- would mean a cycle.
    }
    print_to_builder(*inner_builder, "\n");

    for el : elements {
        if el.type == {
            case .CAPACITOR;
                print_to_builder(*inner_builder, "z%1 = 2 * t%1 - z%1;\n", el.name);
            case .INDUCTOR;
                print_to_builder(*inner_builder, "z%1 = 2 * t%1 + z%1;\n", el.name);
            case .RC_SERIES;
                print_to_builder(*inner_builder, "z%1 = g%1 * (1 - gn%1) * v%1 + gn%1 * z%1; // RC series\n", el.name);
            case .RC_PARALLEL;
                print_to_builder(*inner_builder, "z%1 = gz%1 * v%1 - z%1; // RC parallel\n", el.name);
        }
    }

    return builder_to_string(*outer_builder), builder_to_string(*inner_builder);
}

solve_and_codegen :: (elements: [] Element, inputs: [] string, outputs: [] string) -> (outer_code: string, inner_code: string) {
    solve_vars, equations, exprs := generate_kcl_equations(elements, inputs);

    solutions := solve(equations, solve_vars, *exprs);

    gen_vars := generate_expressions(elements, outputs, solve_vars, solutions, *exprs);

    // Collect gen_var expression IDs for CSE.
    gen_var_ids: [..] Expr_ID;
    for gv : gen_vars  array_add(*gen_var_ids, gv.expr_id);

    cse_vars := cse(gen_var_ids, *exprs);

    // Build the variant base for LICM (inputs + state vars + gen_var names).
    licm_variant_base := build_variant_base(elements, inputs);
    for gv : gen_vars  array_add(*licm_variant_base, gv.name);
    licm(gen_var_ids, *cse_vars, *exprs, licm_variant_base);
    recip_hoist(gen_var_ids, *cse_vars, *exprs, licm_variant_base);

    outer_code, inner_code := codegen(elements, inputs, gen_vars, cse_vars, exprs);
    return outer_code, inner_code;
}
