#import "Basic";
#import "solve";

Element_Type :: enum {
    RESISTOR;
    CAPACITOR;
    INDUCTOR;
    // @TODO: implement stuff for these elements...
    AMP;
    RC_SERIES;
    RC_PARALLEL;
}

Element :: struct {
    name: string;

    type: Element_Type;

    terminals: [..] string;
};

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

        for node, idx : eqn_nodes {
            if node == pos_node equations[idx] = combine(equations[idx], current_symbol, .SUB, *exprs); // current leaving this node
            if node == neg_node equations[idx] = combine(equations[idx], current_symbol, .ADD, *exprs); // current entering this node
        }

        current_expr := NO_EXPR;
        impedance_symbol := expr_symbol(tprint("g%", el.name), *exprs);
        state_symbol := expr_symbol(tprint("z%", el.name), *exprs);
        if el.type == {
            case .RESISTOR;
                current_expr = expr_mul(impedance_symbol, expr_sub(pos_node, neg_node, *exprs), *exprs);
            case .CAPACITOR;
                current_expr = expr_sub(expr_mul(impedance_symbol, expr_sub(pos_node, neg_node, *exprs), *exprs), state_symbol, *exprs);
            case .INDUCTOR;
                current_expr = expr_add(expr_mul(impedance_symbol, expr_sub(pos_node, neg_node, *exprs), *exprs), state_symbol, *exprs);
            case .RC_SERIES;
                current_expr = expr_sub(expr_mul(impedance_symbol, expr_sub(pos_node, neg_node, *exprs), *exprs), state_symbol, *exprs);
            case .RC_PARALLEL;
                current_expr = expr_add(expr_mul(impedance_symbol, expr_sub(pos_node, neg_node, *exprs), *exprs), state_symbol, *exprs);
        }
        // assert(current_expr != NO_EXPR);
        if current_expr != NO_EXPR array_add(*currents, .{ current_symbol, current_expr });
    }

    // just op-amps
    for el : elements {
        if el.type != .AMP continue;
        // @TODO: op-amp stuff here...
    }

    // substitute current and voltage expressions
    for current_sub : currents exprs[current_sub.id] = exprs[current_sub.replace_id];
    for voltage_sub : voltages exprs[voltage_sub.id] = exprs[voltage_sub.replace_id];

    return solve_vars, equations, exprs;
}

// def generate_kcl_equations(elements, inputs):
//     def substitute_node(node_to_sub, replacement_node):
//         for key, el in zip(elements.keys(), elements.values()):
//             for node_idx, node in enumerate(el):
//                 if node == node_to_sub:
//                     el[node_idx] = replacement_node
//                     elements[key] == el

//     for name, el in zip(elements.keys(), elements.values()):
//         kind = el[0]
//         if kind == Element_Type.AMP:
//             out_node = el[1]
//             neg_node = el[2]
//             pos_node = el[3]
//             for idx, node in enumerate(eqn_nodes):
//                 if node == out_node: equations[idx][0].append(f"i{name}") # current is entering this node
//             solve_vars.append(f"i{name}")

//             if pos_node == "0":
//                 # print(f"Settings {neg_node} to 0")
//                 voltages.append(f"{neg_node} -> 0")
//                 solve_vars.remove(neg_node)
//                 substitute_node(neg_node, "0")
//             elif neg_node == "0":
//                 # print(f"Settings {pos_node} to 0")
//                 voltages.append(f"{pos_node} -> 0")
//                 solve_vars.remove(pos_node)
//                 substitute_node(pos_node, "0")
//             else:
//                 # print(f"Settings {pos_node} to {neg_node}")
//                 voltages.append(f"{pos_node} -> {neg_node}")
//                 solve_vars.remove(pos_node)
//                 substitute_node(pos_node, neg_node)

//     assert(len(solve_vars) == len(eqn_nodes))
//     return solve_vars, f"eqn = ({{ {", ".join(equation_strings)} }} /. {{ {", ".join(currents)} }} /. {{ {", ".join(voltages)} }})"

generate_expressions :: (elements: [] Element,
                         outputs: [] string,
                         solve_vars: [] Expr_ID,
                         solutions: [] Expr_ID,
                         exprs: *[..] Expr) -> [] Expr_ID {
    gen_vars: [..] Expr_ID;

    add_gen_var :: (name: string, e: Expr_ID) #expand {
        array_add(*gen_vars, e);
        exprs.*[gen_vars[gen_vars.count-1]].name = name;
    }

    add_expression_for_node :: (node: string) #expand {
        node_symbol := expr_symbol(node, exprs);
        found_solve_var, solve_idx := array_find(solve_vars, node_symbol);
        if !found_solve_var  return;

        for var : gen_vars  if exprs.*[var].name == node  return;
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

    // for var : gen_vars print("%, ", exprs.*[var]);
    return gen_vars;
}

codegen :: (elements: [] Element, gen_vars: [] Expr_ID, exprs: [] Expr) -> string {
    // @TODO: in loop  and out of loop temporaries
    // loop_variant_vars = inputs
    // for name, el in zip(elements.keys(), elements.values()):
    //     if el[0] == Element_Type.CAPACITOR or el[0] == Element_Type.INDUCTOR \
    //         or el[0] == Element_Type.RC_SERIES or el[0] == Element_Type.RC_PARALLEL:
    //         loop_variant_vars.append(f"z{name}")

    builder: String_Builder;

    for var : gen_vars {
        print_to_builder(*builder, "const auto % = %;\n", exprs[var].name, print_expr(var, exprs));
    }
    print_to_builder(*builder, "\n");

    for el : elements {
        if el.type == {
            case .CAPACITOR;
                print_to_builder(*builder, "z%1 = 2 * t%1 - z%1;", el.name);
            case .INDUCTOR;
                print_to_builder(*builder, "z%1 = 2 * t%1 + z%1;", el.name);
            case .RC_SERIES;
                print_to_builder(*builder, "z%1 = g%1 * (1 - gn%1) * v%1 + gn%1 * z%1; // RC series", el.name);
            case .RC_PARALLEL;
                print_to_builder(*builder, "z%1 = gz%1 * v%1 - z%1; // RC parallel", el.name);
        }
    }

    return builder_to_string(*builder);
}

solve_and_codegen :: (elements: [] Element, inputs: [] string, outputs: [] string) -> string {
    solve_vars, equations, exprs := generate_kcl_equations(elements, inputs);

    // for eqn : equations print(print_expr(eqn, exprs));
    // print("\n[ ");
    // for var : solve_vars print("%, ", exprs[var]);
    // print("]\n");

    solutions := solve(equations, solve_vars, *exprs);
    // for var, idx : solve_vars {
    //     print("% = %\n", exprs[var].name, print_expr(solutions[idx], exprs));
    // }
    gen_vars := generate_expressions(elements, outputs, solve_vars, solutions, *exprs);

    return codegen(elements, gen_vars, exprs);
}
