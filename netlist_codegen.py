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

######################################

netlist_file = sys.argv[1]
cpp_header_file = sys.argv[2]
print(f"Generating {cpp_header_file} from netlist: {netlist_file}")

elements, inputs = parse_netlist(netlist_file)
outputs = ['vo']

outer_code, inner_code = solve_and_codegen(elements, inputs, outputs, return_code = True)
# print(outer_code)
# print("\n")
# print(inner_code)

with open(cpp_header_file, 'w') as f:
    f.write("#pragma once\n")
    f.write("struct Params {\n")
    for name, el in zip(elements.keys(), elements.values()):
        # @TODO: "combo" elements will need more logic here
        # @TODO: compile-time constants for "fixed" elements?
        f.write(f"    float {name};\n")
    f.write("};\n\n")

    f.write("struct State {\n")
    states = []
    for name, el in zip(elements.keys(), elements.values()):
        # @TODO: "combo" elements
        kind = el[0]
        if kind == Element_Type.CAPACITOR or kind == Element_Type.INDUCTOR:
            states.append(f"z{name}")
            f.write(f"    float {states[-1]};\n")
    f.write("};\n\n")

    def indent(text, prefix="    "):
        return "\n".join(prefix + line for line in text.splitlines())

    f.write("static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)\n")
    f.write("{\n")
    for name, el in zip(elements.keys(), elements.values()):
        # @TODO: inductor and "combo" elements
        kind = el[0]
        if kind == Element_Type.RESISTOR:
            f.write(f"    const auto g{name} = 1.0f / params.{name};\n")
        elif kind == Element_Type.CAPACITOR:
            f.write(f"    const auto g{name} = 2.0f * sample_rate * params.{name};\n")
    f.write("\n" + indent(outer_code) + "\n\n")
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
