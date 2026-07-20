#import "Basic";
#import "sym_algebra";

Element_Type :: enum {
    RESISTOR;
    CAPACITOR;
    INDUCTOR;
    AMP;
    FIXED_VOLTAGE;
    RC_SERIES;
    RC_PARALLEL;
    DIODE;       // 2-terminal: [anode, cathode]. Shockley I-V law.
    DIODE_PAIR;  // 2-terminal: [anode, cathode]. Antiparallel diode pair collapsed
                 // to one junction-voltage unknown via a closed-form sinh I-V law.
    BJT;         // 3-terminal: [collector, base, emitter]. Ebers-Moll I-V law.
    JFET;        // 3-terminal: [drain, gate, source]. Square-law I-V law.
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

// Model-parameter references for a nonlinear (DIODE / DIODE_PAIR / BJT / JFET)
// element. Each `*_name` names a Params struct field (parsed from a SPICE
// `.model` card by netlist_codegen.jai), same as a RESISTOR's params.<name>.
Nonlinear_Model :: struct {
    is_name:    string; // saturation current, e.g. "D1_Is"
    vt_name:    string; // thermal voltage (already N/emission-coefficient-scaled), e.g. "D1_vt"
    betaf_name: string; // BJT only; empty string for DIODE / DIODE_PAIR
    betar_name: string; // BJT only; empty string for DIODE / DIODE_PAIR
    vp_name:    string; // JFET only; pinch-off voltage
    beta_name:  string; // JFET only; transconductance coefficient
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

    // For DIODE / DIODE_PAIR / BJT elements.
    nonlinear: Nonlinear_Model;

    // Set from a "; no_combine" inline netlist comment (R/C/D only) -- opts
    // this element out of series/parallel/antiparallel-pair reduction.
    no_combine: bool;
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
        if el1.no_combine || el2.no_combine  continue;
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
            if el1.no_combine || el2.no_combine  continue;

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
            if el1.no_combine || el2.no_combine  continue;

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

// Attempt ONE antiparallel diode-pair collapse. Returns true if one was made.
// Two DIODEs sharing both terminals with reversed polarity and the same
// .model collapse into a DIODE_PAIR. Diodes with different .model params
// (e.g. asymmetric clippers) are left as two separate DIODEs.
reduce_diode_pairs_once :: (elements: *[..] Element) -> bool {
    for i : 0..elements.count-1 {
        el1 := (elements.*)[i];
        if el1.type != .DIODE  continue;

        for j : i+1..elements.count-1 {
            el2 := (elements.*)[j];
            if el2.type != .DIODE  continue;
            if el1.no_combine || el2.no_combine  continue;

            same_pair := el1.terminals[0] == el2.terminals[1] && el1.terminals[1] == el2.terminals[0];
            if !same_pair  continue;

            same_model := el1.nonlinear.is_name == el2.nonlinear.is_name
                       && el1.nonlinear.vt_name == el2.nonlinear.vt_name;
            if !same_model  continue;

            print("Combining antiparallel diode pair: % || %\n", el1.name, el2.name);

            new_el: Element;
            new_el.name = tprint("%1%2", el1.name, el2.name);
            new_el.type = .DIODE_PAIR;
            array_add(*new_el.terminals, el1.terminals[0]);
            array_add(*new_el.terminals, el1.terminals[1]);
            new_el.nonlinear = el1.nonlinear;

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
        reduce_diode_pairs_once(elements);
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

// `dc_mode`: build the DC-equivalent circuit instead. Capacitors are open
// circuits at DC, so CAPACITOR/RC_SERIES are dropped entirely; RC_PARALLEL
// still conducts through its resistor half. INDUCTOR DC (short circuit)
// isn't implemented yet. (@TODO)
generate_kcl_equations :: (elements: [] Element, inputs: [] string, dc_mode := false) -> []Expr_ID, []Expr_ID, [..]Expr {
    eqn_nodes: [..] string;

    for el : elements {
        for terminal : el.terminals {
            if terminal == "0" continue;
            if array_find(inputs, terminal) continue;
            if el.type == .FIXED_VOLTAGE    continue;
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

    // All 2-terminal elements beside op-amps, fixed voltage sources, and BJTs
    for el : elements {
        if el.type == .AMP || el.type == .FIXED_VOLTAGE || el.type == .BJT || el.type == .JFET  continue;
        if dc_mode {
            if el.type == .CAPACITOR || el.type == .RC_SERIES  continue; // open circuit at DC -- excluded entirely, not left as a free symbol
            assert(el.type != .INDUCTOR, "generate_kcl_equations(dc_mode=true): INDUCTOR DC handling (short circuit) is not implemented");
        }

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
                if dc_mode {
                    // only the resistor half conducts at DC -- use its raw param, not the trapezoidal g
                    res_symbol := expr_symbol(el.rc_internal.res_name, *exprs);
                    current_expr = expr_div(expr_sub(pos_node_symbol, neg_node_symbol, *exprs), res_symbol, *exprs);
                } else {
                    current_expr = expr_sub(expr_mul(impedance_symbol, expr_sub(pos_node_symbol, neg_node_symbol, *exprs), *exprs), state_symbol, *exprs);
                }
        }
        // assert(current_expr != NO_EXPR);
        if current_expr != NO_EXPR array_add(*currents, .{ current_symbol, current_expr });
    }

    for el : elements {
        if el.type == .BJT {
            collector_node := el.terminals[0];
            base_node      := el.terminals[1];
            emitter_node   := el.terminals[2];

            ib_symbol := expr_symbol(tprint("iB%", el.name), *exprs);
            ic_symbol := expr_symbol(tprint("iC%", el.name), *exprs);
            ie_expr   := expr_add(ib_symbol, ic_symbol, *exprs); // iE = iB + iC

            for node, idx : eqn_nodes {
                if node == collector_node equations[idx] = combine(equations[idx], ic_symbol, .SUB, *exprs);
                if node == base_node      equations[idx] = combine(equations[idx], ib_symbol, .SUB, *exprs);
                if node == emitter_node   equations[idx] = combine(equations[idx], ie_expr,   .ADD, *exprs);
            }
        } else if el.type == .JFET {
            drain_node  := el.terminals[0];
            gate_node   := el.terminals[1];
            source_node := el.terminals[2];

            // Square-law JFET: single branch current flows drain -> source;
            // the gate draws no current in this model.
            id_symbol := expr_symbol(tprint("i%", el.name), *exprs);

            for node, idx : eqn_nodes {
                if node == drain_node  equations[idx] = combine(equations[idx], id_symbol, .SUB, *exprs);
                if node == source_node equations[idx] = combine(equations[idx], id_symbol, .ADD, *exprs);
            }
        }
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

    // Constraint equations for fixed voltage sources
    for el : elements {
        if el.type != .FIXED_VOLTAGE  continue;
        fixed_node   := el.terminals[0];
        node_symbol  := expr_symbol(fixed_node, *exprs);
        param_symbol := expr_symbol(el.name,    *exprs);
        for node, idx : eqn_nodes {
            if node == fixed_node {
                equations[idx] = expr_sub(node_symbol, param_symbol, *exprs);
                break;
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

    // At DC, a node reachable only through a dropped capacitor and a diode
    // branch gets a KCL row with no solve_var at all (just "-iDiode = 0").
    // Zero current through an odd I-V law means zero junction voltage, so
    // just pin the node to its diode partner's voltage.
    if dc_mode {
        for node, idx : eqn_nodes {
            if equations[idx] == NO_EXPR continue;
            if expr_depends_on(equations[idx], solve_vars, exprs) continue;

            for el : elements {
                if el.type != .DIODE && el.type != .DIODE_PAIR continue;
                other := "";
                if      el.terminals[0] == node then other = el.terminals[1];
                else if el.terminals[1] == node then other = el.terminals[0];
                if other.count == 0 continue;

                other_symbol := ifx other == "0" then expr_const(0, *exprs) else expr_symbol(other, *exprs);
                equations[idx] = expr_sub(expr_symbol(node, *exprs), other_symbol, *exprs);
                break;
            }
        }
    }

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
        // Skip nodes whose voltage is already defined by a FIXED_VOLTAGE element
        // in the outer code (generate_impedance_defs emits them there).
        for fv_el : elements {
            if fv_el.type == .FIXED_VOLTAGE && fv_el.terminals[0] == node  return;
        }
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

// Look up the closed-form linear MNA solution for `node`.
lookup_node_voltage :: (node: string, solve_vars: [] Expr_ID, solutions: [] Expr_ID, exprs: *[..] Expr, inputs: [] string = .[]) -> Expr_ID {
    if node == "0"  return expr_const(0, exprs);
    if array_find(inputs, node)  return expr_symbol(node, exprs);
    node_symbol := expr_symbol(node, exprs);
    found, idx := array_find(solve_vars, node_symbol);
    assert(found, "Nonlinear element terminal references a node that isn't part of the solved MNA system");
    return solutions[idx];
}

// One junction-voltage unknown belonging to a nonlinear element: "vD<name>"
// for DIODE/DIODE_PAIR, "vBC<name>"/"vBE<name>" for BJT.
Junction_Voltage :: struct {
    name:    string;
    expr_id: Expr_ID;
}

// For each nonlinear (DIODE / DIODE_PAIR / BJT) element, build the closed-form
// expression for its own junction voltage(s).
generate_junction_voltage_expressions :: (elements: [] Element,
                                          solve_vars: [] Expr_ID,
                                          solutions: [] Expr_ID,
                                          exprs: *[..] Expr) -> [] Junction_Voltage {
    junctions: [..] Junction_Voltage;

    for el : elements {
        if el.type == {
            case .DIODE; #through;
            case .DIODE_PAIR;
                anode_v   := lookup_node_voltage(el.terminals[0], solve_vars, solutions, exprs);
                cathode_v := lookup_node_voltage(el.terminals[1], solve_vars, solutions, exprs);
                v := simplify(expr_sub(anode_v, cathode_v, exprs), exprs);
                array_add(*junctions, .{ name = tprint("v%", el.name), expr_id = v });

            case .BJT;
                collector_v := lookup_node_voltage(el.terminals[0], solve_vars, solutions, exprs);
                base_v      := lookup_node_voltage(el.terminals[1], solve_vars, solutions, exprs);
                emitter_v   := lookup_node_voltage(el.terminals[2], solve_vars, solutions, exprs);
                vbc := simplify(expr_sub(base_v, collector_v, exprs), exprs);
                vbe := simplify(expr_sub(base_v, emitter_v,   exprs), exprs);
                array_add(*junctions, .{ name = tprint("vBC%", el.name), expr_id = vbc });
                array_add(*junctions, .{ name = tprint("vBE%", el.name), expr_id = vbe });

            case .JFET;
                gate_v   := lookup_node_voltage(el.terminals[1], solve_vars, solutions, exprs);
                source_v := lookup_node_voltage(el.terminals[2], solve_vars, solutions, exprs);
                vgs := simplify(expr_sub(gate_v, source_v, exprs), exprs);
                array_add(*junctions, .{ name = tprint("vGS%", el.name), expr_id = vgs });
        }
    }

    return junctions;
}

// Build the set of base loop-variant symbol names: inputs + state variables
// + nonlinear branch currents (free per-sample symbols, not solved-for MNA
// unknowns -- see generate_kcl_equations).
build_variant_base :: (elements: [] Element, inputs: [] string) -> [..] string {
    names: [..] string;
    for inp : inputs  array_add(*names, inp);
    for el : elements {
        if el.type == .CAPACITOR || el.type == .INDUCTOR
           || el.type == .RC_SERIES || el.type == .RC_PARALLEL {
            array_add(*names, tprint("z%", el.name));
        }
        if el.type == .DIODE || el.type == .DIODE_PAIR {
            array_add(*names, tprint("i%", el.name));
        }
        if el.type == .BJT {
            array_add(*names, tprint("iB%", el.name));
            array_add(*names, tprint("iC%", el.name));
        }
        if el.type == .JFET {
            array_add(*names, tprint("i%", el.name));
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
// `extra_variant_names` lets a caller add more loop-variant names on top of
// build_variant_base() -- the nonlinear pipeline uses this to mark junction
// voltages (vD1, vBCQ1, ...) as sample-variant.
codegen :: (elements: [] Element, inputs: [] string,
            gen_vars: [] Gen_Var, cse_vars: [] CSE_Var,
            exprs: [] Expr, options: Codegen_Options = .{},
            extra_variant_names: [] string = .[]) -> (outer_code: string, inner_code: string, outer_flops: Flop_Count, inner_flops: Flop_Count) {

    variant_base := build_variant_base(elements, inputs);
    // Gen_vars are computed inside the per-sample loop; treat their names as
    // loop-variant base symbols so that any CSE temp depending on them is
    // correctly classified as loop-variant.
    for gv : gen_vars  array_add(*variant_base, gv.name);
    for n : extra_variant_names  array_add(*variant_base, n);

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
