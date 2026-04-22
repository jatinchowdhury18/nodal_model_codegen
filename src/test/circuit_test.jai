#import "nodal_analysis";
#import "Basic";

make_arr :: (list: [] $T) -> [..] T {
    dest: [..] T;
    array_copy (*dest, list);
    return dest;
}

main :: () {
    circuit: [..] Element;
    array_add(*circuit, Element.{name="R1", type=.RESISTOR, terminals=make_arr(string.["vo", "vi"]) });
    array_add(*circuit, Element.{name="C1", type=.CAPACITOR, terminals=make_arr(string.["vo", "0"]) });
    outer_code, inner_code := solve_and_codegen(*circuit, .["vi"], .["vo"]);
    print("-----------\n");
    if outer_code.count > 0 {
        print("// OUTER LOOP\n");
        print(outer_code);
    }
    print("// INNER LOOP\n");
    print(inner_code);
}
