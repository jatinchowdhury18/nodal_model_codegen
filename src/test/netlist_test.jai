#import "Basic";

#import "netlist_codegen";
#import "nodal_analysis";

test_netlist :: #string END
* RC Lowpass Filter
V1 vi 0 PULSE(-1 1 0 20m 1u 1u 20m)
R1 vo vi 1k
C1 vo 0 1u IC=0
.tran 0 80m 0 20.0u
.wav "vin.wav" 32 48k V(vi)
.wav "vout.wav" 32 48k V(vo)
.options plotwinsize=0
.backanno
.end
END

check_bool :: (label: string, got: bool, expected: bool) {
    if got == expected  print("PASS  [%]\n", label);
    else                print("FAIL  [%]: got %, expected %\n", label, got, expected);
}

check_str :: (label: string, got: string, expected: string) {
    if got == expected  print("PASS  [%]: %\n", label, got);
    else                print("FAIL  [%]: got '%', expected '%'\n", label, got, expected);
}

find_element :: (elements: [] Element, name: string) -> Element {
    for el : elements  if el.name == name  return el;
    assert(false, "element '%' not found", name);
    return .{};
}

find_value :: (values: [] Element_Value, name: string) -> float32, bool {
    for v : values  if v.name == name  return v.value, true;
    return 0, false;
}

// Op-amp parsing tests: op-amp non-idealities opt in via a
// `* nodal_model: <name> OPAMP (...)` comment defining a named model, plus a
// `; nodal_model: <name>` trailing comment on ANY element line (E, X, ...)
// referencing it -- this is what LTspice-safe (comment-only) means: neither
// line requires SPICE to understand anything new, so the same netlist can
// run standalone in LTspice too.
test_opamp_parsing :: () {
    print("--- opamp parsing ---\n");

    // Untagged E line: gain discarded, Element.opamp stays zero-valued.
    {
        info := parse_netlist(#string END
Eop vo 0 n2 n3 1e6
END);
        eop := find_element(info.elements, "Eop");
        check_bool("untagged E line is ideal", opamp_is_ideal(eop), true);
    }

    // Tagged E line with a full model definition. out/pos/neg default to
    // this line's own tokens[1]/[3]/[4] (no override needed).
    {
        info := parse_netlist(#string END
* nodal_model: NONIDEAL OPAMP (Aol=2e5 Rin=1Meg Rout=75 Vsat+=14.5 Vsat-=-14.5 slew=0.5Meg Vos=1m Ios=20n Ibias=80n)
Eop vo 0 n2 n3 1e6 ; nodal_model: NONIDEAL
END);
        eop := find_element(info.elements, "Eop");
        check_bool("full model is not ideal", opamp_is_ideal(eop), false);
        check_bool("has_aol", eop.opamp.has_aol, true);
        check_bool("has_rin", eop.opamp.has_rin, true);
        check_bool("has_rout", eop.opamp.has_rout, true);
        check_bool("has_vsat_p", eop.opamp.has_vsat_p, true);
        check_bool("has_vsat_n", eop.opamp.has_vsat_n, true);
        check_bool("has_slew", eop.opamp.has_slew, true);
        check_bool("has_vos", eop.opamp.has_vos, true);
        check_bool("has_ios", eop.opamp.has_ios, true);
        check_bool("has_ibias", eop.opamp.has_ibias, true);
        check_str("aol_name", eop.opamp.aol_name, "Eop_Aol");
        check_bool("terminals default to tokens[1]/[3]/[4]", eop.terminals[0] == "vo" && eop.terminals[2] == "n2" && eop.terminals[1] == "n3", true);

        aol_val, found := find_value(info.values, "Eop_Aol");
        check_bool("Eop_Aol present in values", found, true);
        check_bool("Eop_Aol value correct", aol_val == 200000.0, true);
    }

    // pos/neg overrides: needed for a 2-pin behavioral `value={...}` E
    // source, which has no positional control pins at all.
    {
        info := parse_netlist(#string END
* nodal_model: BEHAV OPAMP (Aol=1e5 Vsat+=4.5 Vsat-=-4.5)
Eop vo 0 value={max(min(1e5*(0-V(n1)),4.5),-4.5)} ; nodal_model: BEHAV pos=0 neg=n1
END);
        eop := find_element(info.elements, "Eop");
        check_bool("override: out defaults to tokens[1]", eop.terminals[0] == "vo", true);
        check_bool("override: pos", eop.terminals[2] == "0", true);
        check_bool("override: neg", eop.terminals[1] == "n1", true);
    }

    // Partial params: only Vsat+ set, Vsat- left off.
    {
        info := parse_netlist(#string END
* nodal_model: ASYM OPAMP (Aol=1e5 Vsat+=5)
Eop vo 0 n2 n3 1e6 ; nodal_model: ASYM
END);
        eop := find_element(info.elements, "Eop");
        check_bool("partial: has_vsat_p", eop.opamp.has_vsat_p, true);
        check_bool("partial: has_vsat_n", eop.opamp.has_vsat_n, false);
    }

    // Validation: Rout set without Aol falls back to ideal.
    {
        info := parse_netlist(#string END
* nodal_model: BAD OPAMP (Rout=75)
Eop vo 0 n2 n3 1e6 ; nodal_model: BAD
END);
        eop := find_element(info.elements, "Eop");
        check_bool("Rout without Aol falls back to ideal", opamp_is_ideal(eop), true);
    }

    // Validation: Rin set without Aol also falls back to ideal.
    {
        info := parse_netlist(#string END
* nodal_model: BAD2 OPAMP (Rin=1Meg)
Eop vo 0 n2 n3 1e6 ; nodal_model: BAD2
END);
        eop := find_element(info.elements, "Eop");
        check_bool("Rin without Aol falls back to ideal", opamp_is_ideal(eop), true);
    }

    // Vos/Ibias/Ios alone (no Aol) is a valid non-ideal configuration --
    // doesn't need the explicit-VCVS path, so no Aol requirement.
    {
        info := parse_netlist(#string END
* nodal_model: OFFSETONLY OPAMP (Vos=1m Ibias=80n)
Eop vo 0 n2 n3 1e6 ; nodal_model: OFFSETONLY
END);
        eop := find_element(info.elements, "Eop");
        check_bool("Vos/Ibias without Aol stays non-ideal", opamp_is_ideal(eop), false);
        check_bool("Vos/Ibias without Aol doesn't need explicit VCVS", needs_explicit_vcvs(eop), false);
    }

    // Slew alone (no Vsat at all): needs_explicit_vcvs still requires Aol
    // (needs_aol includes has_slew), and vsat_p_name/vsat_n_name must still
    // get populated with permissive defaults -- clamp_opamp_output's
    // generated call always passes both Vsat rail args regardless of
    // whether Vsat itself was configured (regression guard for a real bug:
    // this was originally gated on `has_vsat_p || has_vsat_n` alone, which
    // stayed false for a slew-only model and left the rail args empty).
    {
        info := parse_netlist(#string END
* nodal_model: SLEWONLY OPAMP (Aol=1e5 slew=500k)
Eop vo 0 n2 n3 1e6 ; nodal_model: SLEWONLY
END);
        eop := find_element(info.elements, "Eop");
        check_bool("slew-only has_vsat_p stays false", eop.opamp.has_vsat_p, false);
        check_bool("slew-only has_vsat_n stays false", eop.opamp.has_vsat_n, false);
        check_bool("slew-only vsat_p_name still populated", eop.opamp.vsat_p_name.count > 0, true);
        check_bool("slew-only vsat_n_name still populated", eop.opamp.vsat_n_name.count > 0, true);
        p_val, p_found := find_value(info.values, eop.opamp.vsat_p_name);
        n_val, n_found := find_value(info.values, eop.opamp.vsat_n_name);
        check_bool("permissive VsatP default registered", p_found && p_val > 1.0e20, true);
        check_bool("permissive VsatN default registered", n_found && n_val < -1.0e20, true);
    }

    // Slew without Aol falls back to ideal too (needs_aol includes has_slew).
    {
        info := parse_netlist(#string END
* nodal_model: BAD3 OPAMP (slew=500k)
Eop vo 0 n2 n3 1e6 ; nodal_model: BAD3
END);
        eop := find_element(info.elements, "Eop");
        check_bool("slew without Aol falls back to ideal", opamp_is_ideal(eop), true);
    }

    // Engineering suffixes (Meg/k/m/u/n/p) must be parsed, not truncated.
    {
        info := parse_netlist(#string END
* nodal_model: SUFFIXED OPAMP (Aol=1e5 GBW=1Meg Rin=1Meg Rout=75)
Eop vo 0 n2 n3 1e6 ; nodal_model: SUFFIXED
END);
        rin_val, _ := find_value(info.values, "Eop_Rin");
        check_bool("Rin=1Meg parses to 1e6, not 1", rin_val == 1.0e6, true);
    }

    // Ccomp gives the compensation cap's value directly, instead of
    // deriving it from Aol/GBW.
    {
        info := parse_netlist(#string END
* nodal_model: EXPLICIT_C OPAMP (Aol=1e5 Ccomp=15.9m)
Eop vo 0 n2 n3 1e6 ; nodal_model: EXPLICIT_C
END);
        eop := find_element(info.elements, "Eop");
        check_bool("Ccomp alone still activates the GBW/bandwidth stage", eop.opamp.has_gbw, true);
        ccomp_val, found := find_value(info.values, "Eop_Ccomp");
        check_bool("Eop_Ccomp uses the explicit value directly", found && ccomp_val == 15.9e-3, true);
    }

    // Rin/Rout/GBW are synthesized as ordinary RESISTOR/CAPACITOR elements
    // wired into the gain-node chain, not fields read directly by the AMP
    // KCL branch -- verify they actually show up in info.elements.
    {
        info := parse_netlist(#string END
* nodal_model: SYNTH OPAMP (Aol=1e5 GBW=1Meg Rin=1Meg Rout=75)
Eop vo 0 n2 n3 1e6 ; nodal_model: SYNTH
END);
        rin_el := find_element(info.elements, "Eop_Rin");
        check_bool("synthesized Rin terminals", rin_el.terminals[0] == "n2" && rin_el.terminals[1] == "n3", true);
        check_bool("synthesized Rin no_combine", rin_el.no_combine, true);

        rcomp_el := find_element(info.elements, "Eop_Rcomp");
        check_bool("synthesized Rcomp: gain -> bandwidth node", rcomp_el.terminals[0] == "vgain_Eop" && rcomp_el.terminals[1] == "vbw_Eop", true);

        ccomp_el := find_element(info.elements, "Eop_Ccomp");
        check_bool("synthesized Ccomp: bandwidth node -> gnd", ccomp_el.terminals[0] == "vbw_Eop" && ccomp_el.terminals[1] == "0", true);

        rout_el := find_element(info.elements, "Eop_Rout");
        check_bool("synthesized Rout: bandwidth node -> out", rout_el.terminals[0] == "vbw_Eop" && rout_el.terminals[1] == "vo", true);
    }

    // Two instances sharing one named model get independent (non-deduped)
    // per-instance params, unlike D/Q model dedup -- and the tag works
    // regardless of which line it's attached to.
    {
        info := parse_netlist(#string END
* nodal_model: SHARED OPAMP (Aol=1e5)
Eop1 vo1 0 n2 n3 1e6 ; nodal_model: SHARED
Eop2 vo2 0 n4 n5 1e6 ; nodal_model: SHARED
END);
        eop1 := find_element(info.elements, "Eop1");
        eop2 := find_element(info.elements, "Eop2");
        check_str("instance 1 name", eop1.opamp.aol_name, "Eop1_Aol");
        check_str("instance 2 name", eop2.opamp.aol_name, "Eop2_Aol");
        _, found1 := find_value(info.values, "Eop1_Aol");
        _, found2 := find_value(info.values, "Eop2_Aol");
        check_bool("instance 1 value present", found1, true);
        check_bool("instance 2 value present", found2, true);
    }
}

main :: () {
    code := netlist_codegen(test_netlist);
    print("----------------------------------\n");
    print(code);

    test_opamp_parsing();
}
