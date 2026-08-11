#import "nodal_analysis";
#import "sym_algebra";
#import "codegen";
#import "Basic";

make_arr :: (list: [] $T) -> [..] T {
    dest: [..] T;
    array_copy (*dest, list);
    return dest;
}

check_str :: (label: string, got: string, expected: string) {
    if got == expected  print("PASS  [%]: %\n", label, got);
    else                print("FAIL  [%]: got '%', expected '%'\n", label, got, expected);
}

check_bool :: (label: string, got: bool, expected: bool) {
    if got == expected  print("PASS  [%]\n", label);
    else                print("FAIL  [%]: got %, expected %\n", label, got, expected);
}

// Non-ideal op-amp gain-chain node naming (opamp_gain_node/opamp_bandwidth_node):
// collapses to real out_node whenever nothing follows in the chain, else
// allocates the right synthetic name. See nodal_analysis.jai.
test_opamp_node_naming :: () {
    print("--- opamp gain-chain node naming ---\n");

    amp :: (opamp: Opamp_Model) -> Element {
        el := Element.{ name = "Eop", type = .AMP };
        array_add(*el.terminals, "vo"); // out
        array_add(*el.terminals, "n1"); // neg
        array_add(*el.terminals, "0");  // pos
        el.opamp = opamp;
        return el;
    }

    // Aol only: no synthetic nodes at all.
    {
        el := amp(.{ has_aol = true });
        check_str("Aol-only gain node", opamp_gain_node(el), "vo");
        check_str("Aol-only bandwidth node", opamp_bandwidth_node(el), "vo");
    }
    // Aol + Rout: one synthetic node.
    {
        el := amp(.{ has_aol = true, has_rout = true });
        check_str("Aol+Rout gain node", opamp_gain_node(el), "vgain_Eop");
        check_str("Aol+Rout bandwidth node", opamp_bandwidth_node(el), "vgain_Eop");
    }
    // Aol + GBW, no Rout: lowpass output feeds straight to the real out_node.
    {
        el := amp(.{ has_aol = true, has_gbw = true });
        check_str("Aol+GBW gain node", opamp_gain_node(el), "vgain_Eop");
        check_str("Aol+GBW bandwidth node", opamp_bandwidth_node(el), "vo");
    }
    // Aol + GBW + Rout: full chain, two synthetic nodes.
    {
        el := amp(.{ has_aol = true, has_gbw = true, has_rout = true });
        check_str("full chain gain node", opamp_gain_node(el), "vgain_Eop");
        check_str("full chain bandwidth node", opamp_bandwidth_node(el), "vbw_Eop");
    }
}

// Inverting amp (R1 from vi to n1, RF from n1 to vo, pos tied to 0) with a
// finite Aol. Verifies generate_kcl_equations' explicit-VCVS path against
// a hand-derived closed form:
//   vo = -Aol*gR1*vi / (gR1 + gRF + gRF*Aol)
test_finite_gain_inverting_amp :: () {
    print("--- finite-gain inverting amp ---\n");

    circuit: [..] Element;
    array_add(*circuit, Element.{ name = "R1", type = .RESISTOR, terminals = make_arr(string.["vi", "n1"]) });
    array_add(*circuit, Element.{ name = "RF", type = .RESISTOR, terminals = make_arr(string.["n1", "vo"]) });
    amp := Element.{ name = "Eop", type = .AMP, opamp = .{ has_aol = true, aol_name = "Aol" } };
    array_add(*amp.terminals, "vo"); // out
    array_add(*amp.terminals, "n1"); // neg
    array_add(*amp.terminals, "0");  // pos
    array_add(*circuit, amp);

    solve_vars, equations, pool := generate_kcl_equations(circuit, .["vi"]);
    check_bool("no free op-amp current unknown (explicit VCVS, not merge)", solve_vars.count == 2, true); // n1, vo

    solutions := solve(equations, solve_vars, *pool);
    vo_idx, found := 0, false;
    for sv, i : solve_vars  if print_expr(sv, pool.exprs) == "vo" { vo_idx = i; found = true; }
    check_bool("vo is a solved var", found, true);

    check_str("vo closed form", print_expr(solutions[vo_idx], pool.exprs),
        "(-(((gR1 * vi) * Aol) / ((gR1 + gRF) + (gRF * Aol))))");
}

main :: () {
    test_opamp_node_naming();
    test_finite_gain_inverting_amp();

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
