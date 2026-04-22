import subprocess
import sys
from enum import Enum

class Element_Type(Enum):
    RESISTOR = 1
    CAPACITOR = 2
    INDUCTOR = 3
    AMP = 4
    RC_SERIES = 5
    RC_PARALLEL = 6

class Combo_Op_Type(Enum):
    SUM = 1
    RECIP_SUM = 2

def reduce_circuit(elements, inputs, outputs):
    def get_expr(n, el):
        expr = n
        if len(el) > 3: expr = el[3]
        return expr

    def add_sum(n1, n2, el1, el2):
        expr1 = get_expr(n1, el1)
        expr2 = get_expr(n2, el2)
        return (Combo_Op_Type.SUM, expr1, expr2)

    def add_recip_sum(n1, n2, el1, el2):
        expr1 = get_expr(n1, el1)
        expr2 = get_expr(n2, el2)
        return (Combo_Op_Type.RECIP_SUM, expr1, expr2)

    def reduce_series(elements, inputs, outputs):
        def make_rc_series(n_res, el_res, other_res, n_cap, el_cap, other_cap):
            print(f"Combining series resistor and capacitor: {n_res} + {n_cap}")
            new_name = f"{n_res}{n_cap}"
            elements[new_name] = [Element_Type.RC_SERIES, other_res, other_cap, {f"{n_res}": get_expr(n_res, el_res), f"{n_cap}": get_expr(n_cap, el_cap)}]

        # Build a map: node -> list of all elements connected to that node
        node_map = {}
        for name, el in elements.items():
            connections = el[1:3]
            if el[0] == Element_Type.AMP: connections = el[1:4]
            for node in connections:
                node_map.setdefault(node, []).append(name)

        # Only reduce if the node is not input/output/ground and has exactly two elements connected
        for node, connected in node_map.items():
            if node in inputs or node in outputs or node == "0":
                continue
            if len(connected) == 2:
                n1, n2 = connected
                el1, el2 = elements[n1], elements[n2]
                other1 = el1[2] if el1[1] == node else el1[1]
                other2 = el2[2] if el2[1] == node else el2[1]
                if el1[0] == Element_Type.RESISTOR and el2[0] == Element_Type.RESISTOR:
                    print(f"Combining series resistors: {n1} + {n2}")
                    new_name = f"{n1}{n2}"
                    elements[new_name] = [Element_Type.RESISTOR, other1, other2, add_sum(n1, n2, el1, el2)]
                    del elements[n1]
                    del elements[n2]
                    break
                if el1[0] == Element_Type.CAPACITOR and el2[0] == Element_Type.CAPACITOR:
                    print(f"Combining series capacitors: {n1} + {n2}")
                    new_name = f"{n1}{n2}"
                    elements[new_name] = [Element_Type.CAPACITOR, other1, other2, add_recip_sum(n1, n2, el1, el2)]
                    del elements[n1]
                    del elements[n2]
                    break
                if el1[0] == Element_Type.RESISTOR and el2[0] == Element_Type.CAPACITOR:
                    make_rc_series(n1, el1, other1, n2, el2, other2)
                    del elements[n1]
                    del elements[n2]
                    break
                if el1[0] == Element_Type.CAPACITOR and el2[0] == Element_Type.RESISTOR:
                    make_rc_series(n2, el2, other2, n1, el1, other1)
                    del elements[n1]
                    del elements[n2]
                    break
        return elements

    def reduce_parallel(elements):
        # Map (node1, node2) -> list of resistor names (order nodes for uniqueness)
        res_pair_map = {}
        cap_pair_map = {}
        for name, el in elements.items():
            if el[0] == Element_Type.RESISTOR:
                n1, n2 = el[1], el[2]
                key = tuple(sorted([n1, n2]))
                res_pair_map.setdefault(key, []).append(name)
            if el[0] == Element_Type.CAPACITOR:
                n1, n2 = el[1], el[2]
                key = tuple(sorted([n1, n2]))
                cap_pair_map.setdefault(key, []).append(name)

        # R || R
        for (n1, n2), names in res_pair_map.items():
            if len(names) > 1:
                nA, nB = names[:2]
                print(f"Combining parallel resistors: {nA} || {nB}")
                expr = add_recip_sum(nA, nB, elements[nA], elements[nB])
                new_name = f"{nA}{nB}"
                elements[new_name] = [Element_Type.RESISTOR, n1, n2, expr]
                del elements[nA]
                del elements[nB]
                return elements

        # C || C
        for (n1, n2), names in cap_pair_map.items():
            if len(names) > 1:
                nA, nB = names[:2]
                print(f"Combining parallel capacitors: {nA} || {nB}")
                expr = add_sum(nA, nB, elements[nA], elements[nB])
                new_name = f"{nA}{nB}"
                elements[new_name] = [Element_Type.CAPACITOR, n1, n2, expr]
                del elements[nA]
                del elements[nB]
                return elements

        # R || C
        for key in set(res_pair_map.keys()) & set(cap_pair_map.keys()):
            res_names = res_pair_map[key]
            cap_names = cap_pair_map[key]
            if res_names and cap_names:
                res_name = res_names[0]
                cap_name = cap_names[0]
                n1, n2 = key
                print(f"Combining parallel resistor and capacitor: {res_name} || {cap_name}")
                elements[f"{res_name}{cap_name}"] = [
                    Element_Type.RC_PARALLEL,
                    n1,
                    n2,
                    {
                        f"{res_name}": get_expr(res_name, elements[res_name]),
                        f"{cap_name}": get_expr(cap_name, elements[cap_name])
                    }
                ]
                del elements[res_name]
                del elements[cap_name]
                return elements

        return elements

    prev_elements_count = 0
    while len(elements) != prev_elements_count:
        prev_elements_count = len(elements)
        # Reduction: R + R or C + C
        elements = reduce_series(elements, inputs, outputs)
        # Reduction: R || R or C || C
        elements = reduce_parallel(elements)

    return elements


def generate_kcl_equations(elements, inputs):
    eqn_nodes = set()
    for el in elements.values():
        eqn_nodes.update(
            n for n in el[1:]
            if not isinstance(n, (tuple, dict))
        )
    eqn_nodes.remove("0")
    eqn_nodes.difference_update(inputs)
    eqn_nodes = list(eqn_nodes)

    equations = []
    for _ in eqn_nodes:
        equations.append([[], []])

    currents = []
    voltages = []
    solve_vars = list(eqn_nodes)
    for name, el in zip(elements.keys(), elements.values()):
        kind = el[0]
        if kind != Element_Type.AMP:
            pos_node = el[1]
            neg_node = el[2]
            for idx, node in enumerate(eqn_nodes):
                if node == pos_node: equations[idx][1].append(f"i{name}") # current is leaving this node
                if node == neg_node: equations[idx][0].append(f"i{name}") # current is entering this node

            current_str = f"i{name} -> "
            if kind == Element_Type.RESISTOR:
                current_str += f"g{name} * ({pos_node} - {neg_node})"
            elif kind == Element_Type.CAPACITOR:
                current_str += f"g{name} * ({pos_node} - {neg_node}) - z{name}"
            elif kind == Element_Type.INDUCTOR:
                current_str += f"g{name} * ({pos_node} - {neg_node}) + z{name}"
            elif kind == Element_Type.RC_SERIES:
                current_str += f"g{name} * ({pos_node} - {neg_node}) - z{name}"
            elif kind == Element_Type.RC_PARALLEL:
                current_str += f"g{name} * ({pos_node} - {neg_node}) - z{name}"
            currents.append(current_str)

    def substitute_node(node_to_sub, replacement_node):
        for key, el in zip(elements.keys(), elements.values()):
            for node_idx, node in enumerate(el):
                if node == node_to_sub:
                    el[node_idx] = replacement_node
                    elements[key] == el

    for name, el in zip(elements.keys(), elements.values()):
        kind = el[0]
        if kind == Element_Type.AMP:
            out_node = el[1]
            neg_node = el[2]
            pos_node = el[3]
            for idx, node in enumerate(eqn_nodes):
                if node == out_node: equations[idx][0].append(f"i{name}") # current is entering this node
            solve_vars.append(f"i{name}")

            if pos_node == "0":
                # print(f"Settings {neg_node} to 0")
                voltages.append(f"{neg_node} -> 0")
                solve_vars.remove(neg_node)
                substitute_node(neg_node, "0")
            elif neg_node == "0":
                # print(f"Settings {pos_node} to 0")
                voltages.append(f"{pos_node} -> 0")
                solve_vars.remove(pos_node)
                substitute_node(pos_node, "0")
            else:
                # print(f"Settings {pos_node} to {neg_node}")
                voltages.append(f"{pos_node} -> {neg_node}")
                solve_vars.remove(pos_node)
                substitute_node(pos_node, neg_node)

    assert(len(solve_vars) == len(eqn_nodes))

    equation_strings = []
    for idx, eqn in enumerate(equations):
        # if len(eqn[0]) == 0 and len(eqn[1]) == 0: continue
        new_equation = " + ".join(eqn[0]) if len(eqn[0]) > 0 else "0"
        new_equation += " == "
        new_equation += " + ".join(eqn[1]) if len(eqn[1]) > 0 else "0"
        new_equation += f" (*{eqn_nodes[idx]}*)"
        equation_strings.append(new_equation)

    assert(len(solve_vars) == len(equation_strings))

    return solve_vars, f"eqn = ({{ {", ".join(equation_strings)} }} /. {{ {", ".join(currents)} }} /. {{ {", ".join(voltages)} }})"

def generate_expressions(elements, outputs, solve_vars, simplify="FullSimplify"):
    expressions = {}
    gen_vars = []
    def add_expression_for_node(node):
        if node not in solve_vars: return
        if node in expressions.keys(): return
        expr_idx = solve_vars.index(node)
        expr_str = f"{node}expr = slnp[[1]][[{expr_idx+1}]][[2]] // {simplify};" # @TODO: more granular Simplify vs. FullSimplify?
        expressions[node] = expr_str

    def get_node_expr(node):
        if node in expressions.keys(): return f"{node}expr"
        return node

    for output in outputs:
        add_expression_for_node(output)
        gen_vars.append(f"{output}expr")

    for name, el in zip(elements.keys(), elements.values()):
        if el[0] == Element_Type.CAPACITOR or el[0] == Element_Type.INDUCTOR:
            add_expression_for_node(el[1])
            add_expression_for_node(el[2])

            key = f"t{name}"
            pos_node = get_node_expr(el[1])
            neg_node = get_node_expr(el[2])
            expressions[key] = f"t{name} = g{name} * ({pos_node} - {neg_node});"
            gen_vars.append(f"t{name}")

        if el[0] == Element_Type.RC_SERIES or el[0] == Element_Type.RC_PARALLEL:
            add_expression_for_node(el[1])
            add_expression_for_node(el[2])

            key = f"v{name}"
            pos_node = get_node_expr(el[1])
            neg_node = get_node_expr(el[2])
            expressions[key] = f"v{name} = ({pos_node} - {neg_node});"
            gen_vars.append(f"v{name}")
    # print(expressions)

    return gen_vars, "\n".join(expressions.values())

def generate_script_command(mm_eqns, mm_exprs, solve_vars, gen_vars):
    mm_solve = f"slnp=Solve[eqn, {{{", ".join(solve_vars)}}}];"

    mm_codegen = f"""
    oex = Experimental`OptimizeExpression[{{{", ".join(gen_vars)}}},
       "OptimizationLevel" -> 2, "OptimizationSymbol" -> temp];
    Print[Total[Cases[oex, a_Times :> Length[a], Infinity]]]
    Print[CForm[oex] /. {{Power[x_, 2] :> x*x, Power[x_, -1] :> 1/x}}];
    """

    mm_cmd = f"{mm_eqns}\n{mm_solve}\n{mm_exprs}\n{mm_codegen}"
    return mm_cmd

def process_codegen(oex, elements, inputs, gen_vars):
    oex = oex[45:-4]
    oex = oex[oex.find("},") + 3:]
    temps, outs = oex.split("{")
    temps = temps[:-2].split("; ")
    outs = outs.split(", ")
    # print(temps)
    # print(outs)

    loop_variant_vars = inputs
    for name, el in zip(elements.keys(), elements.values()):
        if el[0] == Element_Type.CAPACITOR or el[0] == Element_Type.INDUCTOR \
            or el[0] == Element_Type.RC_SERIES or el[0] == Element_Type.RC_PARALLEL:
            loop_variant_vars.append(f"z{name}")

    loop_variant_temps = []
    for temp in temps:
        lhs, rhs = temp.split(" = ")
        for var in loop_variant_vars:
            if var in rhs:
                loop_variant_temps.append(temp)
                loop_variant_vars.append(lhs)
                break

    loop_invariant_temps = []
    for temp in temps:
        if temp not in loop_variant_temps:
            loop_invariant_temps.append(temp)

    ool_code_str = []
    for temp in loop_invariant_temps:
        ool_code_str.append(f"const auto {temp};")
    code_str = []
    for temp in loop_variant_temps:
        code_str.append(f"const auto {temp};")
    code_str.append("")
    for var, out in zip(gen_vars, outs):
        code_str.append(f"const auto {var.removesuffix("expr")} = {out};")
    code_str.append("")
    for name, el in zip(elements.keys(), elements.values()):
        if el[0] == Element_Type.CAPACITOR:
            code_str.append(f"z{name} = 2 * t{name} - z{name};")
        elif el[0] == Element_Type.INDUCTOR:
            code_str.append(f"z{name} = 2 * t{name} + z{name};")
        elif el[0] == Element_Type.RC_SERIES:
            code_str.append(f"z{name} = g{name} * (1 - gn{name}) * v{name} + gn{name} * z{name}; // RC series")
        elif el[0] == Element_Type.RC_PARALLEL:
            code_str.append(f"z{name} = gz{name} * v{name} - z{name}; // RC parallel")

    return "\n".join(ool_code_str), "\n".join(code_str)

def solve_and_codegen(elements, inputs, outputs, return_code=False, out_file=None, reduce=True, simplify="FullSimplify"):
    if reduce:
        elements = reduce_circuit(elements, inputs, outputs)

    solve_vars, mm_eqns = generate_kcl_equations(elements, inputs)
    print(f"Solving for variables: {solve_vars}")
    gen_vars, mm_exprs = generate_expressions(elements, outputs, solve_vars, simplify=simplify)
    print(f"Generating expressions: {gen_vars}")
    mm_cmd = generate_script_command(mm_eqns, mm_exprs, solve_vars, gen_vars)

    # print(mm_cmd)
    # exit(0)
    print("Generating code...")
    result = subprocess.run(
        ["wolframscript", "-code", mm_cmd],
        capture_output=True,
        text=True
    )
    # print(result)

    stdout_parts = result.stdout.split('\n')
    print(f"# Multiplies: {stdout_parts[0]}")

    outer_code, inner_code = process_codegen(stdout_parts[1], elements, inputs, gen_vars)

    if out_file is None and not return_code:
        print(outer_code)
        print("\n")
        print(inner_code)
    elif out_file is not None:
        with open(out_file, 'w') as f:
            f.write("// OUTER LOOP\n")
            f.write(outer_code)
            f.write("\n// INNER LOOP\n")
            f.write(inner_code)

    if return_code:
        return elements, outer_code, inner_code
