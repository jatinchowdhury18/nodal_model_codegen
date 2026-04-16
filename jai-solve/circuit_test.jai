#import "nodal_analysis";
Solve :: #import "solve";
#import "Basic";

// RC Lowpass Filter
// circuit: {
//   'R1': [<Element_Type.RESISTOR: 1>, 'vo', 'vi'],
//   'C1': [<Element_Type.CAPACITOR: 2>, 'vo', '0']
// }
// inputs: ['vi']
// outputs: ['vo']

make_arr :: (list: [] $T) -> [..] T {
    dest: [..] T;
    array_copy (*dest, list);
    return dest;
}

main :: () {
    circuit := Element.[
        Element.{"R1", .RESISTOR, make_arr(string.["vo", "vi"]) },
        Element.{"C1", .CAPACITOR, make_arr(string.["vo", "0"]) },
    ];
    code := solve_and_codegen(circuit, .["vi"], .["vo"]);
    print("-----------\n");
    print(code);
}
