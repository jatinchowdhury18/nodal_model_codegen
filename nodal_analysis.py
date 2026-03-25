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

def generate_kcl_equations(elements, inputs):
    eqn_nodes = set()
    for el in elements.values():
        # for node_idx in range(1, len(el))
        eqn_nodes.update(el[1:])
    eqn_nodes.remove("0")
    eqn_nodes.difference_update(inputs)

    equations = []
    for _ in eqn_nodes:
        equations.append([[], []])

    currents = []
    voltages = []
    solve_vars = list(eqn_nodes)
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
                for key, el in zip(elements.keys(), elements.values()):
                    for node_idx, node in enumerate(el):
                        if node == neg_node:
                            el[node_idx] = "0"
                            elements[key] == el
            elif neg_node == "0":
                # print(f"Settings {pos_node} to 0")
                voltages.append(f"{pos_node} -> 0")
                solve_vars.remove(pos_node)
                for key, el in zip(elements.keys(), elements.values()):
                    for node_idx, node in enumerate(el):
                        if node == pos_node:
                            el[node_idx] = "0"
                            elements[key] == el
            else:
                # print(f"Settings {pos_node} to {neg_node}")
                voltages.append(f"{pos_node} -> {neg_node}")
                solve_vars.remove(pos_node)
                for key, el in zip(elements.keys(), elements.values()):
                    for node_idx, node in enumerate(el):
                        if node == pos_node:
                            el[node_idx] = neg_node
                            elements[key] == el

        else:
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

    assert(len(solve_vars) == len(eqn_nodes))

    equation_strings = []
    for eqn in equations:
        # if len(eqn[0]) == 0 and len(eqn[1]) == 0: continue
        new_equation = " + ".join(eqn[0]) if len(eqn[0]) > 0 else "0"
        new_equation += " == "
        new_equation += " + ".join(eqn[1]) if len(eqn[1]) > 0 else "0"
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

def solve_and_codegen(elements, inputs, outputs, return_code=False, out_file=None):
    solve_vars, mm_eqns = generate_kcl_equations(elements, inputs)
    print(f"Solving for variables: {solve_vars}")
    gen_vars, mm_exprs = generate_expressions(elements, outputs, solve_vars)
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
        return outer_code, inner_code
