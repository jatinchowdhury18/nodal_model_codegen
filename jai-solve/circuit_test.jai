#import "nodal_analysis";
Solve :: #import "solve";
#import "Basic";

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
    outer_code, inner_code := solve_and_codegen(circuit, .["vi"], .["vo"]);
    print("-----------\n");
    if outer_code.count > 0 {
        print("// OUTER LOOP\n");
        print(outer_code);
    }
    print("// INNER LOOP\n");
    print(inner_code);
}
