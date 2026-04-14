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
    solve_vars, equations, exprs := generate_kcl_equations(circuit, .["vi"]);

    for eqn : equations print(Solve.print_expr(eqn, exprs));
    print("\n[ ");
    for var : solve_vars print("%, ", exprs[var]);
    print("]\n");

    solutions := Solve.solve(equations, solve_vars, *exprs);
    for var, idx : solve_vars {
        print("% = %\n", exprs[var].name, Solve.print_expr(solutions[idx], exprs));
    }
    gen_vars := generate_expressions(circuit, .["vo"], solve_vars, solutions, *exprs);

    print("-----------\n");
    code := codegen(circuit, gen_vars, exprs);
    print(code);
}
