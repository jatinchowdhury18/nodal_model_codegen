from nodal_analysis import *
import re

def parse_netlist(path):
    elements = {}
    inputs = []

    with open(path) as f:
        for line in f:
            line = line.strip()

            # Skip comments / directives
            if not line or line.startswith("*") or line.startswith("."):
                continue

            if "netlist_skip" in line:
                continue

            tokens = re.split(r"\s+", line)
            name = tokens[0]
            n1, n2 = tokens[1], tokens[2]
            prefix = name[0].upper()

            # Map LTspice → your Element_Type
            if prefix == "R":
                etype = Element_Type.RESISTOR
            elif prefix == "C":
                etype = Element_Type.CAPACITOR
            elif prefix == "L":
                etype = Element_Type.INDUCTOR
            elif prefix == "E":
                etype = Element_Type.AMP
                elements[name] = [etype, tokens[1], tokens[4], tokens[3]]
                continue
            elif prefix == "V":
                # treat voltage source as input node
                inputs.append(n1)
                continue  # don't include in elements
            else:
                continue  # ignore unsupported parts

            elements[name] = [etype, n1, n2]

    # --- simple output heuristic (matches your solver expectations) ---
    all_nodes = set()
    for el in elements.values():
        all_nodes.update(el[1:])
    all_nodes.discard("0")

    outputs = list(all_nodes - set(inputs))

    return elements, inputs


def generate_params_struct(elements):
    def extract_param_names(expr):
        """Recursively yield all string (leaf) names from a nested tuple expression."""
        if isinstance(expr, str):
            yield expr
        elif isinstance(expr, tuple):
            for item in expr[1:]:  # skip the operation at expr[0]
                yield from extract_param_names(item)


    code = "struct Params {\n"
    for name, el in zip(elements.keys(), elements.values()):
        # @TODO: "combo" elements will need more logic here
        # @TODO: compile-time constants for "fixed" elements?
        kind = el[0]
        if kind == Element_Type.RESISTOR or kind == Element_Type.CAPACITOR or kind == Element_Type.INDUCTOR:
            if len(el) == 3:
                code += f"    float {name};\n"
            else:
                for pname in set(extract_param_names(el[3])):
                    code += f"    float {pname};\n"
        if kind == Element_Type.RC_SERIES or kind == Element_Type.RC_PARALLEL:
            rc_internals = el[3]
            res_name, cap_name = list(rc_internals.keys())[:2]
            for pname in set(extract_param_names(rc_internals[res_name])):
                code += f"    float {pname};\n"
            for pname in set(extract_param_names(rc_internals[cap_name])):
                code += f"    float {pname};\n"

    code += "};\n"
    return code

def generate_state_struct(elements):
    code = "struct State {\n"
    states = []
    for name, el in zip(elements.keys(), elements.values()):
        # @TODO: "combo" elements
        kind = el[0]
        if (
            kind == Element_Type.CAPACITOR
            or kind == Element_Type.INDUCTOR
            or kind == Element_Type.RC_SERIES
            or kind == Element_Type.RC_PARALLEL
        ):
            states.append(f"z{name}")
            code += f"    float {states[-1]} {{}};\n"
    code += "};\n"
    return code, states

def generate_impedance_defs(elements):
    def param_expr(expr, param_prefix="params."):
        if isinstance(expr, str):
            return f"{param_prefix}{expr}"
        op = expr[0]
        args = expr[1:]
        if op == Combo_Op_Type.SUM:
            # sum(a, b, c)
            return f"sum({', '.join(param_expr(arg, param_prefix) for arg in args)})"
        elif op == Combo_Op_Type.RECIP_SUM:
            # recip_sum(a, b, c)
            return f"recip_sum({', '.join(param_expr(arg, param_prefix) for arg in args)})"
        else:
            raise ValueError(f"Unknown operation: {op}")

    code = "[[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };\n"
    code += "[[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };\n\n"
    for name, el in zip(elements.keys(), elements.values()):
        # @TODO: "combo" elements
        kind = el[0]
        val_name = f"params.{name}"
        if len(el) > 3 and isinstance(el[3], tuple):
            val_name = name
            code += f"const auto {val_name} = {param_expr(el[3])};\n"
        if kind == Element_Type.RESISTOR:
            code += f"const auto g{name} = 1.0f / {val_name};\n\n"
        elif kind == Element_Type.CAPACITOR:
            code += f"const auto g{name} = 2.0f * sample_rate * {val_name};\n\n"
        elif kind == Element_Type.INDUCTOR:
            code += f"const auto g{name} = 1.0f / (2.0f * sample_rate * {val_name});\n\n"
        elif kind == Element_Type.RC_SERIES:
            rc_internals = el[3]
            res_name, cap_name = list(rc_internals.keys())[:2]
            code += f"const auto {res_name} = {param_expr(rc_internals[res_name])};\n"
            code += f"const auto {cap_name} = {param_expr(rc_internals[cap_name])};\n"
            code += f"const auto g{name} = (2.0f * sample_rate * {cap_name}) / (1.0f + 2.0f * sample_rate * {res_name} * {cap_name});\n"
            code += f"const auto gn{name} = g{name} * (2.0f * sample_rate * {res_name} * {cap_name} - 1.0f) / (2.0f * sample_rate * {cap_name});\n\n"
        elif kind == Element_Type.RC_PARALLEL:
            rc_internals = el[3]
            res_name, cap_name = list(rc_internals.keys())[:2]
            code += f"const auto {res_name} = {param_expr(rc_internals[res_name])};\n"
            code += f"const auto {cap_name} = {param_expr(rc_internals[cap_name])};\n"
            code += f"const auto g{name} = 2.0f * sample_rate * {cap_name} + (1.0f / {res_name});\n"
            code += f"const auto gz{name} = 4.0f * sample_rate * {cap_name};\n\n"

    return code

def netlist_codegen(netlist_file, cpp_header_file, reduce_circuit=True):
    print(f"Generating {cpp_header_file} from netlist: {netlist_file}")

    elements, inputs = parse_netlist(netlist_file)
    outputs = ['vo']
    # print(elements)
    # print(inputs)

    elements, outer_code, inner_code = solve_and_codegen(elements, inputs, outputs, return_code=True, reduce=reduce_circuit)
    # print(outer_code)
    # print("\n")
    # print(inner_code)

    with open(cpp_header_file, 'w') as f:
        def indent(text, prefix="    "):
            return "\n".join(prefix + line for line in text.splitlines())

        f.write("#pragma once\n\n")

        params_struct_code = generate_params_struct(elements)
        f.write(params_struct_code + "\n")

        states_struct_code, states = generate_state_struct(elements)
        f.write(states_struct_code + "\n")

        f.write("static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)\n")
        f.write("{\n")

        f.write(indent(generate_impedance_defs(elements)) + "\n")
        f.write(indent(outer_code) + "\n\n")

        f.write("    for (int ch = 0; ch < num_channels; ++ch)\n")
        f.write("    {\n")

        for state in states:
            f.write(f"        auto {state} = state[ch].{state};\n")

        f.write("        for (int n = 0; n < num_samples; ++n)\n")
        f.write("        {\n")

        # @TODO: multiple inputs
        f.write(f"            const auto {inputs[0]} = input[ch][n];\n\n")
        f.write(indent(inner_code, "            ") + "\n\n")
        # @TODO: multiple outputs
        f.write(f"            output[ch][n] = {outputs[0]};\n")

        f.write("        }\n")
        for state in states:
            f.write(f"        state[ch].{state} = {state};\n")

        f.write("    }\n")
        f.write("}\n")

if __name__ == "__main__":
    netlist_file = sys.argv[1]
    cpp_header_file = sys.argv[2]
    netlist_codegen(netlist_file, cpp_header_file)
