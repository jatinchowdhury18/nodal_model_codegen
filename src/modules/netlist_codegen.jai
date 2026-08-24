#import "Basic";
#import "Hash_Table";
Math :: #import "Math";

#import "nodal_analysis";
#import "sym_algebra";
#import "nonlinear_solve";
#import "backend";
#import "transcendental_approx";
#import "codegen";

// Thermal voltage kT/q at room temperature (~300K).
// For diodes/BJTs, the "vt" parameter is computed as
// VT_ROOM * N (for "ideality factor" N).
VT_ROOM :: 0.02585;

// The SPICE-side device kind on a `.model <name> <kind> [(key=val ...)]`
// card. D/NPN/PNP/NJF/PJF are real SPICE device types; OPAMP and TRIODE
// aren't. UNKNOWN covers any other kind we don't parse (e.g. a real SPICE
// resistor/capacitor tolerance model).
Model_Kind :: enum {
    UNKNOWN;
    D;
    NPN;
    PNP;
    NJF;
    PJF;
    TRIODE;
    OPAMP;
}

parse_model_kind :: (s: string) -> Model_Kind {
    upper := String.to_upper_copy(s);
    if upper == {
        case "D";      return .D;
        case "NPN";    return .NPN;
        case "PNP";    return .PNP;
        case "NJF";    return .NJF;
        case "PJF";    return .PJF;
        case "TRIODE"; return .TRIODE;
        case "OPAMP";  return .OPAMP;
    }
    return .UNKNOWN;
}

// A parsed SPICE `.model <name> <kind> [(key=val ...)]` card. Referenced by
// name from `D`/`Q`/`J`/`X` element lines. Unset parameters fall back to
// standard SPICE defaults.
Model_Def :: struct {
    name:      string;
    kind:      Model_Kind;
    is_val:    float32; // diode/BJT saturation current
    n_val:     float32; // diode-only; folded into vt_val at parse time
    betaf_val: float32;
    betar_val: float32;
    vp_val:    float32; // JFET-only pinch-off voltage (VTO)
    beta_val:  float32; // JFET-only transconductance coefficient (BETA).

    // TRIODE only (Dempwolf & Zolzer model). See triode_iv_laws() (nonlinear_solve.jai).
    gk_val:    float32; // cathode-current perveance
    mu_val:    float32; // amplification factor
    gamma_val: float32; // cathode-current exponent
    ck_val:    float32; // cathode-current softplus adaption factor
    gg_val:    float32; // grid-current perveance
    xi_val:    float32; // grid-current exponent
    cg_val:    float32; // grid-current softplus adaption factor
    ig0_val:   float32; // grid-current stability offset
}

// Standard SPICE default model parameters.
default_model :: () -> Model_Def {
    model: Model_Def;
    model.is_val    = 1.0e-14;
    model.n_val     = 1.0;
    model.betaf_val = 100.0;
    model.betar_val = 1.0;
    model.vp_val    = -2.0;
    model.beta_val  = 1.0e-4;

    // TRIODE defaults are the "RSD-1" 12AX7 fit from Table 1 of Dempwolf & Zolzer (DAFx-11).
    model.gk_val    = 2.242e-3;
    model.mu_val    = 103.2;
    model.gamma_val = 1.26;
    model.ck_val    = 3.40;
    model.gg_val    = 6.177e-4;
    model.xi_val    = 1.314;
    model.cg_val    = 9.901;
    model.ig0_val   = 8.025e-8;
    return model;
}

// Parse a bare number with an optional trailing SPICE SI-scale suffix.
parse_spice_value :: (val_str: string) -> float32 {
    val, success, remainder := string_to_float(val_str);
    if !success return Math.FLOAT32_NAN;
    if remainder.count == 0 { return val; }
    if remainder[0] == {
        case #char "M";
            val *= 1.0e6;
        case #char "k";
            val *= 1.0e3;
        case #char "m";
            val *= 1.0e-3;
        case #char "u";
            val *= 1.0e-6;
        case #char "n";
            val *= 1.0e-9;
        case #char "p";
            val *= 1.0e-12;
    }
    return val;
}

// Parse a `.model` line, e.g. "model Q1M NPN (Is=2e-14 Bf=200 Br=4)".
// Missing parameters get standard SPICE defaults.
parse_model_line :: (line: string) -> Model_Def {
    normalized, _ := String.replace(line, "(", " (");
    tokens_raw := String.split(normalized, #char " ");
    tokens: [..] string;
    for t : tokens_raw  if t.count > 0  array_add(*tokens, t);

    model := default_model();

    if tokens.count < 3  return model; // malformed .model line; caller gets an empty-named model

    model.name = tokens[1];
    model.kind = parse_model_kind(tokens[2]);

    if tokens.count > 3 {
        joined := String.join(..array_view(tokens, 3, tokens.count - 3), separator = " ");
        joined, _ = String.replace(joined, "(", " ");
        joined, _ = String.replace(joined, ")", " ");
        joined, _ = String.replace(joined, ",", " ");
        kv_tokens_raw := String.split(joined, #char " ");
        for kv : kv_tokens_raw {
            if kv.count == 0  continue;
            parts := String.split(kv, #char "=");
            if parts.count != 2  continue;
            key := String.to_upper_copy(String.trim(parts[0]));
            val := parse_spice_value(String.trim(parts[1]));
            if Math.is_nan_is_inf(val)  continue;
            if key == {
                case "IS"; model.is_val = val;
                case "N";  model.n_val = val;
                case "BF"; model.betaf_val = val;
                case "BR"; model.betar_val = val;
                case "VTO"; model.vp_val = val;
                case "BETA"; model.beta_val = val;
                case "GK";    model.gk_val = val;
                case "MU";    model.mu_val = val;
                case "GAMMA"; model.gamma_val = val;
                case "CK";    model.ck_val = val;
                case "GG";    model.gg_val = val;
                case "XI";    model.xi_val = val;
                case "CG";    model.cg_val = val;
                case "IG0";   model.ig0_val = val;
            }
        }
    }

    return model;
}

// Part numbers like "2N5485" aren't valid C identifiers,
// so we prefix with "_" (e.g. "_2N5485_Beta"). find_model()
// must keep using the raw, un-prefixed name for lookups.
ident_safe_name :: (name: string) -> string {
    if name.count > 0 && is_digit(name[0])  return tprint("_%", name);
    return name;
}

// Look up a parsed .model card by name, falling back to a default-parameter model.
find_model :: (model_defs: *Table(string, Model_Def), name: string) -> Model_Def {
    found, model := table_find(model_defs, name);
    if found  return model;
    result := default_model();
    result.name = name;
    return result;
}

// Peek at a `.model <name> <kind> ...` line's kind token
// without fully parsing it. Used to route a `.model` line
// to parse_model_line() or parse_opamp_model_line().
peek_model_kind :: (line: string) -> Model_Kind {
    tokens_raw := String.split(line, #char " ");
    tokens: [..] string;
    for t : tokens_raw  if t.count > 0  array_add(*tokens, t);
    if tokens.count < 3  return .UNKNOWN;
    return parse_model_kind(tokens[2]);
}

// A parsed SPICE-style `.model <name> OPAMP (Aol=.. GBW=.. Ccomp=.. Rin=..
// Rout=.. Vsat+=.. Vsat-=.. slew=.. Vos=.. Ios=.. Ibias=..)` card. Every
// param is independently optional. Unlike D/Q models there's no SPICE
// default to fall back to, since "unset" means "ideal for that
// characteristic", so we track presence with `has_*` flags instead.
Opamp_Model_Def :: struct {
    name:       string;
    has_aol:    bool; aol_val:    float32;
    has_gbw:    bool; gbw_val:    float32;
    has_ccomp:  bool; ccomp_val:  float32; // explicit compensation cap (F); takes precedence over GBW if both are set
    has_rin:    bool; rin_val:    float32;
    has_rout:   bool; rout_val:   float32;
    has_vsat_p: bool; vsat_p_val: float32;
    has_vsat_n: bool; vsat_n_val: float32;
    has_slew:   bool; slew_val:   float32;
    has_vos:    bool; vos_val:    float32;
    has_ios:    bool; ios_val:    float32;
    has_ibias:  bool; ibias_val:  float32;
}

// Parse a `.model <name> OPAMP (...)` line.
parse_opamp_model_line :: (line: string) -> Opamp_Model_Def {
    tokens_raw := String.split(line, #char " ");
    tokens: [..] string;
    for t : tokens_raw  if t.count > 0  array_add(*tokens, t);

    model: Opamp_Model_Def;
    if tokens.count < 3  return model; // malformed .model line

    model.name = tokens[1];

    if tokens.count > 3 {
        joined := String.join(..array_view(tokens, 3, tokens.count - 3), separator = " ");
        joined, _ = String.replace(joined, "(", " ");
        joined, _ = String.replace(joined, ")", " ");
        joined, _ = String.replace(joined, ",", " ");
        kv_tokens_raw := String.split(joined, #char " ");
        for kv : kv_tokens_raw {
            if kv.count == 0  continue;
            parts := String.split(kv, #char "=");
            if parts.count != 2  continue;
            key := String.to_upper_copy(String.trim(parts[0]));
            val := parse_spice_value(String.trim(parts[1])); // suffix-aware (1Meg, 10k, 1u, ...)
            if Math.is_nan_is_inf(val)  continue;
            if key == {
                case "AOL";   model.has_aol = true;    model.aol_val = val;
                case "GBW";   model.has_gbw = true;    model.gbw_val = val;
                case "CCOMP"; model.has_ccomp = true;  model.ccomp_val = val;
                case "RIN";   model.has_rin = true;    model.rin_val = val;
                case "ROUT";  model.has_rout = true;   model.rout_val = val;
                case "VSAT+"; model.has_vsat_p = true; model.vsat_p_val = val;
                case "VSAT-"; model.has_vsat_n = true; model.vsat_n_val = val;
                case "SLEW";  model.has_slew = true;   model.slew_val = val;
                case "VOS";   model.has_vos = true;    model.vos_val = val;
                case "IOS";   model.has_ios = true;    model.ios_val = val;
                case "IBIAS"; model.has_ibias = true;  model.ibias_val = val;
            }
        }
    }

    return model;
}

// Look up a parsed .model OPAMP card by name.
// Falls back to an ideal model if not found.
find_opamp_model :: (model_defs: *Table(string, Opamp_Model_Def), name: string) -> Opamp_Model_Def {
    found, model := table_find(model_defs, name);
    if found  return model;
    result: Opamp_Model_Def;
    result.name = name;
    return result;
}

// Register a (name, value) pair in `values` at most once. `value_index`
// mirrors `values` (name -> index into it) so this and find_element_value()
// don't have to linear-scan `values` on every call.
add_value_once :: (values: *[..] Element_Value, value_index: *Table(string, int), name: string, value: float32) {
    if table_contains(value_index, name)  return;
    table_add(value_index, name, values.count);
    array_add(values, Element_Value.{ name = name, value = value });
}

Netlist_Info :: struct {
    elements : [..] Element;
    values : [..] Element_Value;
    value_index : Table(string, int); // name -> index into `values`

    inputs : [..] string;
    outputs : [..] string;

    // `* sub-circuit: <name> <in_node> <out_node>`.
    sub_circuits : [..] Sub_Circuit_Decl;

    // Newton-Raphson solver defaults.
    newton_tol_sq   := 1.0e-5;
    newton_max_iter := 20;

    // exp/log/pow approximation settings, overridable per netlist via
    // `* exp_approx(order=N, c1=0/1, exact=0/1)`. See generate_transcendental_approx_helpers().
    transcendental_approx: Transcendental_Approx;
}

// One `key=value` pair from a `keyword(key1=val1, key2=val2, ...)` directive line.
Kv_Pair :: struct {
    key: string; // upper-cased
    val: float;
}

// Parses a `keyword(key1=val1, key2=val2, ...)` line into key/value pairs
parse_kv_directive :: (line: string, keyword_len: int) -> [] Kv_Pair {
    rest := String.slice(line, keyword_len, line.count - keyword_len);
    rest, _ = String.replace(rest, "(", " ");
    rest, _ = String.replace(rest, ")", " ");
    rest, _ = String.replace(rest, ",", " ");

    result: [..] Kv_Pair;
    for kv : String.split(rest, #char " ") {
        if kv.count == 0  continue;
        parts := String.split(kv, #char "=");
        if parts.count != 2  continue;
        val, success := string_to_float(String.trim(parts[1]));
        if !success  continue;
        array_add(*result, Kv_Pair.{ key = String.to_upper_copy(String.trim(parts[0])), val = val });
    }
    return result;
}

parse_netlist :: (netlist_str: string) -> Netlist_Info {
    using netlist_info : Netlist_Info;

    add_element :: (name: string, type: Element_Type, terminals: [] string, no_combine := false) #expand {
        element := Element.{ name = name, type = type, no_combine = no_combine };
        array_copy(*element.terminals, terminals);
        array_add(*elements, element);
    }

    // Registers "<base_name>_<suffix>" = val in `values` and returns its name.
    reg_param :: (base_name: string, suffix: string, val: float32) -> string #expand {
        pname := tprint("%_%", base_name, suffix);
        add_value_once(*values, *value_index, pname, val);
        return pname;
    }

    // By default we use the op-amp equivalent pins from SPICE's
    // VCVS ("E") model. But for more complicated pin-outs,
    // we allow the nodes to be overidden with a trailing comment:
    // "; nodal_model: <name> [out=..] [pos=..] [neg=..]".
    Nodal_Model_Ref :: struct {
        name: string;
        out_override: string;
        pos_override: string;
        neg_override: string;
    }
    find_nodal_model_ref :: (line: string) -> Nodal_Model_Ref {
        result: Nodal_Model_Ref;
        parts := String.split(line, "nodal_model:");
        if parts.count < 2  return result;

        tokens_raw := String.split(String.trim(parts[1]), #char " ");
        tokens: [..] string;
        for t : tokens_raw  if t.count > 0  array_add(*tokens, t);
        if tokens.count == 0  return result;

        result.name = tokens[0];
        for i : 1..tokens.count-1 {
            kv := String.split(tokens[i], #char "=");
            if kv.count != 2  continue;
            key := String.to_upper_copy(kv[0]);
            if key == {
                case "OUT"; result.out_override = kv[1];
                case "POS"; result.pos_override = kv[1];
                case "NEG"; result.neg_override = kv[1];
            }
        }
        return result;
    }

    // `; nodal_model: <name>` opts ANY element line (a real SPICE `E`, an `X`
    // instantiating a library part, ...) into our non-ideal op-amp model, and
    // completely ignores whatever else the line says. out/pos/neg default to
    // that line's tokens[1]/[3]/[4] unless overridden in `ref`.
    handle_nodal_model_element :: (name: string, tokens: [] string, ref: Nodal_Model_Ref) #expand {
        model := find_opamp_model(*opamp_model_defs, ref.name);
        out_node := ifx ref.out_override.count > 0 then ref.out_override else tokens[1];
        pos_node := ifx ref.pos_override.count > 0 then ref.pos_override else tokens[3];
        neg_node := ifx ref.neg_override.count > 0 then ref.neg_override else tokens[4];

        // Rin/Rout/GBW/Ccomp/Vsat/slew only make sense on a distinguishable
        // finite-gain output node, so they all require Aol too.
        needs_aol := model.has_rin || model.has_rout || model.has_vsat_p || model.has_vsat_n || model.has_slew || model.has_gbw || model.has_ccomp;
        if needs_aol && !model.has_aol {
            log("Warning: nodal_model '%' sets Rin/Rout/Vsat/slew/GBW/Ccomp without Aol; treating '%' as ideal.\n", ref.name, name);
            add_element(name, .AMP, .[out_node, neg_node, pos_node]);
            continue;
        }

        opamp: Opamp_Model;
        if model.has_aol    { opamp.has_aol = true;    opamp.aol_name    = reg_param(name, "Aol", model.aol_val); }
        if model.has_gbw || model.has_ccomp  opamp.has_gbw = true; // no Params field of its own; only used below to derive the synthesized Ccomp/Rcomp defaults
        if model.has_rin    { opamp.has_rin = true;    opamp.rin_name    = reg_param(name, "Rin", model.rin_val); }
        if model.has_rout   { opamp.has_rout = true;   opamp.rout_name   = reg_param(name, "Rout", model.rout_val); }
        // clamp_opamp_output always takes both rails, so both fields
        // get populated whenever any clip characteristic is set.
        if model.has_vsat_p || model.has_vsat_n || model.has_slew {
            opamp.has_vsat_p = model.has_vsat_p;
            opamp.has_vsat_n = model.has_vsat_n;
            p_val := ifx model.has_vsat_p then model.vsat_p_val else 1.0e30;
            n_val := ifx model.has_vsat_n then model.vsat_n_val else -1.0e30;
            opamp.vsat_p_name = reg_param(name, "VsatP", cast(float32) p_val);
            opamp.vsat_n_name = reg_param(name, "VsatN", cast(float32) n_val);
        }
        if model.has_slew   { opamp.has_slew = true;   opamp.slew_name   = reg_param(name, "Slew", model.slew_val); }
        if model.has_vos    { opamp.has_vos = true;    opamp.vos_name    = reg_param(name, "Vos", model.vos_val); }
        if model.has_ios    { opamp.has_ios = true;    opamp.ios_name    = reg_param(name, "Ios", model.ios_val); }
        if model.has_ibias  { opamp.has_ibias = true;  opamp.ibias_name  = reg_param(name, "Ibias", model.ibias_val); }

        element := Element.{ name = name, type = .AMP, opamp = opamp };
        array_add(*element.terminals, out_node);
        array_add(*element.terminals, neg_node);
        array_add(*element.terminals, pos_node);
        array_add(*elements, element);

        // Rin/Rout/GBW are synthesized as ordinary RESISTOR/CAPACITOR
        // elements wired into the gain-node chain.
        if opamp.has_rin {
            rin_el := Element.{ name = opamp.rin_name, type = .RESISTOR, no_combine = true };
            array_add(*rin_el.terminals, pos_node);
            array_add(*rin_el.terminals, neg_node);
            array_add(*elements, rin_el);
        }
        if opamp.has_gbw {
            // Ccomp, if given, is used directly (the real compensation cap value);
            // otherwise GBW only fixes the Aol/GBW *ratio* (the dominant pole), so the
            // cap's default is derived once here from the literal model values.
            r_comp := 1.0; // internal normalization resistor; only the R*C product matters
            c_comp: float64;
            if model.has_ccomp {
                c_comp = model.ccomp_val;
            } else {
                pole_hz := model.gbw_val / model.aol_val;
                c_comp = 1.0 / (2.0 * Math.PI * r_comp * pole_hz);
            }

            rcomp_name := reg_param(name, "Rcomp", cast(float32) r_comp);
            rcomp_el := Element.{ name = rcomp_name, type = .RESISTOR, no_combine = true };
            array_add(*rcomp_el.terminals, opamp_gain_node(element));
            array_add(*rcomp_el.terminals, opamp_bandwidth_node(element));
            array_add(*elements, rcomp_el);

            ccomp_name := reg_param(name, "Ccomp", cast(float32) c_comp);
            ccomp_el := Element.{ name = ccomp_name, type = .CAPACITOR, no_combine = true };
            array_add(*ccomp_el.terminals, opamp_bandwidth_node(element));
            array_add(*ccomp_el.terminals, "0");
            array_add(*elements, ccomp_el);
        }
        if opamp.has_rout {
            rout_el := Element.{ name = opamp.rout_name, type = .RESISTOR, no_combine = true };
            array_add(*rout_el.terminals, opamp_clip_node(element));
            array_add(*rout_el.terminals, out_node);
            array_add(*elements, rout_el);
        }
        continue;
    }

    lines := String.split(netlist_str, "\n");

    // Strip a ';'-style inline comment. Callers must do this AFTER checking for the "netlist_skip".
    strip_comment :: (line: string) -> string {
        parts := String.split(line, #char ";");
        return ifx parts.count > 0 then String.trim(parts[0]) else line;
    }

    // First pass: collect .model cards (D/NPN/PNP/JFET/TRIODE/OPAMP). SPICE
    // has no native op-amp or triode primitive, but (like TRIODE) an OPAMP
    // .model card is still just an ordinary line. A `.SUBCKT ... .ENDS` block
    // is skipped entirely here.
    model_defs: Table(string, Model_Def);
    opamp_model_defs: Table(string, Opamp_Model_Def);
    {
        in_subckt := false;
        for line_raw : lines {
            line := String.trim(line_raw);
            if line.count == 0  continue;
            if line[0] != #char "."  continue;
            line = String.trim(String.slice(line, 1, line.count - 1));

            if String.begins_with_nocase(line, "subckt") { in_subckt = true;  continue; }
            if String.begins_with_nocase(line, "ends")   { in_subckt = false; continue; }
            if in_subckt  continue; // skip any internal .model/.ic/etc. cards inside the subckt body

            if !String.begins_with(line, "model")  continue;
            line = strip_comment(line);
            if peek_model_kind(line) == .OPAMP {
                model := parse_opamp_model_line(line);
                if !table_contains(*opamp_model_defs, model.name)  table_add(*opamp_model_defs, model.name, model);
            } else {
                model := parse_model_line(line);
                if !table_contains(*model_defs, model.name)  table_add(*model_defs, model.name, model);
            }
        }
    }

    // Second pass: build elements/inputs/outputs/sub-circuits from every
    // non-.model line (dot-directives, then per-element-type parsing below).
    in_subckt := false;
    for line_raw : lines {
        line := String.trim(line_raw);

        if line.count == 0  continue;

        if line[0] == #char "." {
            body := String.trim(String.slice(line, 1, line.count - 1));
            if String.begins_with_nocase(body, "subckt") { in_subckt = true;  continue; }
            if String.begins_with_nocase(body, "ends")   { in_subckt = false; continue; }
        }
        if in_subckt  continue; // skip the triode subckt's internal E/G/R/C/.model body lines

        if line[0] == #char "*" || line[0] == #char "." {
            // trim comment start char
            line = String.trim(String.slice(line, 1, line.count - 1));
            if String.begins_with(line, "output:") {
                output := String.trim(String.slice(line, 7, line.count - 7));
                log("Adding output node: %\n", output);
                array_add(*outputs, output);
            }
            if String.begins_with(line, "sub-circuit:") {
                rest := String.trim(String.slice(line, 12, line.count - 12));
                rest_tokens_raw := String.split(rest, #char " ");
                rest_tokens: [..] string;
                for t : rest_tokens_raw  if t.count > 0  array_add(*rest_tokens, t);
                if rest_tokens.count == 3 {
                    decl: Sub_Circuit_Decl;
                    decl.name     = rest_tokens[0];
                    decl.in_node  = rest_tokens[1];
                    decl.out_node = rest_tokens[2];
                    log("Sub-circuit %: % -> %\n", decl.name, decl.in_node, decl.out_node);
                    array_add(*sub_circuits, decl);
                } else {
                    log("Warning: malformed 'sub-circuit:' line '%'; expected '<name> <in_node> <out_node>'.\n", rest);
                }
            }
            // `nr_solve(max_iter=<int>, tol=<val>)`.
            if String.begins_with(line, "nr_solve") {
                for kv : parse_kv_directive(line, 8) {
                    if kv.key == {
                        case "TOL";      newton_tol_sq = kv.val;
                        case "MAX_ITER"; newton_max_iter = cast(int) kv.val;
                    }
                }
            }

            // `exp_approx(order=<int>, c1=0/1, exact=0/1)`. Overrides the
            // default exp/log/pow approximation (order 6, C1-continuous).
            // `exact=1` falls back to std::exp/std::log/std::pow instead.
            if String.begins_with(line, "exp_approx") {
                for kv : parse_kv_directive(line, 10) {
                    if kv.key == {
                        case "ORDER"; transcendental_approx.order = cast(int) kv.val;
                        case "C1";    transcendental_approx.c1_continuous = kv.val != 0;
                        case "EXACT"; transcendental_approx.exact = kv.val != 0;
                    }
                }
                // pow2_approx_coeffs() supports orders 1-7 (3-7 if c1) and
                // log2_approx_coeffs() supports orders 1-6 (3-6 if c1), so
                // the valid range is the intersection of those ranges.
                min_order := ifx transcendental_approx.c1_continuous then 3 else 1;
                max_order := 6;
                if !transcendental_approx.exact && (transcendental_approx.order < min_order || transcendental_approx.order > max_order) {
                    log("Warning: exp_approx order=% is out of range (supported: %-% given c1=%); falling back to order=6.\n",
                        transcendental_approx.order, min_order, max_order, cast(int) transcendental_approx.c1_continuous);
                    transcendental_approx.order = 6;
                }
            }

            continue;
        }

        if String.contains(line, "netlist_skip")  continue;

        has_no_combine := String.contains(line, "no_combine");
        nodal_model_ref := find_nodal_model_ref(line);
        line = strip_comment(line);

        tokens_raw := String.split(line, #char " ");
        tokens: [..] string;
        for t : tokens_raw  if t.count > 0  array_add(*tokens, t);

        name := tokens[0];
        n1, n2 := tokens[1], tokens[2];
        value := parse_spice_value(tokens[3]);
        if ! Math.is_nan_is_inf(value)  add_value_once(*values, *value_index, name, value); // unparsed value (e.g. input voltage source) defaults to 0

        if nodal_model_ref.name.count > 0  handle_nodal_model_element(name, tokens, nodal_model_ref);

        prefix := String.to_upper(name[0]);

        etype: Element_Type;
        if prefix == {
            case "R";
                etype = .RESISTOR;
            case "C";
                etype = .CAPACITOR;
            case "L";
                etype = .INDUCTOR;
            case "E";
                // Ideal op-amp: `E<name> out ref pos neg <gain>`
                // (gain is ignored here).
                add_element(name, .AMP, .[tokens[1], tokens[4], tokens[3]]);
                continue;
            case "V";
                is_fixed := false;
                for token : tokens {
                    if token == "fixed" { is_fixed = true; break; }
                }
                // A `DC <value>` source is also a fixed/constant source and
                // needs its Params default recorded here.
                if tokens.count > 4 && String.to_upper_copy(tokens[3]) == "DC" {
                    is_fixed = true;
                    dc_value := parse_spice_value(tokens[4]);
                    if !Math.is_nan_is_inf(dc_value)  add_value_once(*values, *value_index, name, dc_value);
                }

                if is_fixed {
                    etype = .FIXED_VOLTAGE;
                } else {
                    array_add(*inputs, n1); // treat voltage source as input node
                    continue; // don't include in elements
                }
            case "D";
                // D<name> <anode> <cathode> <model>
                model := find_model(*model_defs, tokens[3]);
                is_name := tprint("%_Is", ident_safe_name(model.name));
                vt_name := tprint("%_vt", ident_safe_name(model.name));
                add_value_once(*values, *value_index, is_name, model.is_val);
                add_value_once(*values, *value_index, vt_name, cast(float32)(model.n_val * VT_ROOM));

                element := Element.{ name = name, type = .DIODE, no_combine = has_no_combine };
                array_add(*element.terminals, n1); // anode
                array_add(*element.terminals, n2); // cathode
                element.nonlinear = .{ is_name = is_name, vt_name = vt_name };
                array_add(*elements, element);
                continue;
            case "Q";
                // Q<name> <collector> <base> <emitter> <model>.
                emitter    := tokens[3];
                model      := find_model(*model_defs, tokens[4]);
                if model.kind == {
                    case .NPN; #through;
                    case .PNP;
                        is_name    := tprint("%_Is", ident_safe_name(model.name));
                        vt_name    := tprint("%_vt", ident_safe_name(model.name));
                        betaf_name := tprint("%_BetaF", ident_safe_name(model.name));
                        betar_name := tprint("%_BetaR", ident_safe_name(model.name));
                        add_value_once(*values, *value_index, is_name, model.is_val);
                        add_value_once(*values, *value_index, vt_name, cast(float32)(model.n_val * VT_ROOM));
                        add_value_once(*values, *value_index, betaf_name, model.betaf_val);
                        add_value_once(*values, *value_index, betar_name, model.betar_val);

                        bjt_type := ifx model.kind == .PNP then Element_Type.PNP else Element_Type.NPN;
                        element := Element.{ name = name, type = bjt_type };
                        array_add(*element.terminals, n1);      // collector
                        array_add(*element.terminals, n2);      // base
                        array_add(*element.terminals, emitter); // emitter
                        element.nonlinear = .{ is_name = is_name, vt_name = vt_name, betaf_name = betaf_name, betar_name = betar_name };
                        array_add(*elements, element);
                }
                continue;
            case "J";
                // J<name> <drain> <gate> <source> <model>. PJF mirrors NJF.
                source := tokens[3];
                model  := find_model(*model_defs, tokens[4]);
                if model.kind == {
                    case .NJF; #through;
                    case .PJF;
                        beta_name := tprint("%_Beta", ident_safe_name(model.name));
                        vp_name   := tprint("%_vp", ident_safe_name(model.name));
                        add_value_once(*values, *value_index, beta_name, model.beta_val);
                        add_value_once(*values, *value_index, vp_name, model.vp_val);

                        jfet_type := ifx model.kind == .PJF then Element_Type.PJF else Element_Type.NJF;
                        element := Element.{ name = name, type = jfet_type };
                        array_add(*element.terminals, n1);     // drain
                        array_add(*element.terminals, n2);     // gate
                        array_add(*element.terminals, source); // source
                        element.nonlinear = .{ beta_name = beta_name, vp_name = vp_name };
                        array_add(*elements, element);
                }
                continue;
            case "G";
                // G<name> n+ n- nc+ nc- value  (linear VCCS: I = value*(V(nc+)-V(nc-)), sourced at n+/sunk at n-)
                ctrl_pos := tokens[3];
                ctrl_neg := tokens[4];
                gm_value := parse_spice_value(tokens[5]);
                if !Math.is_nan_is_inf(gm_value)  add_value_once(*values, *value_index, name, gm_value);

                element := Element.{ name = name, type = .VCCS };
                array_add(*element.terminals, n1);
                array_add(*element.terminals, n2);
                array_add(*element.terminals, ctrl_pos);
                array_add(*element.terminals, ctrl_neg);
                array_add(*elements, element);
                continue;
            case "I";
                // I<name> n+ n- value (constant current, sourced at n+/sunk at n-).
                // The generic `value` parse above (tokens[3]) already registered it.
                add_element(name, .CURRENT_SOURCE, .[n1, n2], has_no_combine);
                continue;
            case "X";
                // X<name> <term1> <term2> ... <termN> <model>
                // Generic subcircuit instantiation. Real SPICE always puts the
                // subckt/model name *last*, regardless of terminal count, so
                // we look it up positionally from the end.
                if tokens.count < 3 {
                    log("Warning: malformed X-line '%'; ignoring.\n", name);
                    continue;
                }
                model     := find_model(*model_defs, tokens[tokens.count - 1]);
                terminals := array_view(tokens, 1, tokens.count - 2);
                if model.kind == {
                    case .TRIODE;
                        if terminals.count != 3 {
                            log("Warning: TRIODE model '%' on '%' expects 3 terminals (plate/grid/cathode), got %; ignoring.\n",
                                model.name, name, terminals.count);
                            continue;
                        }
                        gk_name    := tprint("%_Gk", ident_safe_name(model.name));
                        mu_name    := tprint("%_Mu", ident_safe_name(model.name));
                        gamma_name := tprint("%_Gamma", ident_safe_name(model.name));
                        ck_name    := tprint("%_Ck", ident_safe_name(model.name));
                        gg_name    := tprint("%_Gg", ident_safe_name(model.name));
                        xi_name    := tprint("%_Xi", ident_safe_name(model.name));
                        cg_name    := tprint("%_Cg", ident_safe_name(model.name));
                        ig0_name   := tprint("%_Ig0", ident_safe_name(model.name));
                        add_value_once(*values, *value_index, gk_name,    model.gk_val);
                        add_value_once(*values, *value_index, mu_name,    model.mu_val);
                        add_value_once(*values, *value_index, gamma_name, model.gamma_val);
                        add_value_once(*values, *value_index, ck_name,    model.ck_val);
                        add_value_once(*values, *value_index, gg_name,    model.gg_val);
                        add_value_once(*values, *value_index, xi_name,    model.xi_val);
                        add_value_once(*values, *value_index, cg_name,    model.cg_val);
                        add_value_once(*values, *value_index, ig0_name,   model.ig0_val);

                        element := Element.{ name = name, type = .TRIODE };
                        array_add(*element.terminals, terminals[0]); // plate
                        array_add(*element.terminals, terminals[1]); // grid
                        array_add(*element.terminals, terminals[2]); // cathode
                        element.nonlinear = .{
                            gk_name = gk_name, mu_name = mu_name, gamma_name = gamma_name, ck_name = ck_name,
                            gg_name = gg_name, xi_name = xi_name, cg_name = cg_name, ig0_name = ig0_name,
                        };
                        array_add(*elements, element);
                }
                continue;
            case;
                log("Warning: unsupported element '%' (prefix '%'); ignoring.\n", name, prefix);
                continue;
        }
        add_element(name, etype, .[n1, n2], has_no_combine);
    }

    return netlist_info;
}

// True if any element in `elements` needs the Newton-Raphson path (a real
// nonlinear I-V law, or an op-amp with an active clip/slew stage).
circuit_has_nonlinear :: (elements: [] Element) -> bool {
    for el : elements {
        if nonlinear_device_spec(el.type).unknowns.count > 0  return true;
        if el.type == .AMP && opamp_has_clip_stage(el)  return true;
    }
    return false;
}

// True if any element's I-V law needs exp/log/pow.
circuit_uses_transcendentals :: (elements: [] Element) -> bool {
    for el : elements {
        if el.type == .DIODE || el.type == .DIODE_PAIR || el.type == .NPN || el.type == .PNP || el.type == .TRIODE  return true;
    }
    return false;
}

flop_str :: (f: Flop_Count) -> string {
    b: String_Builder;
    sep := "";
    if f.muls > 0 { append(*b, sep); print_to_builder(*b, "% mul", f.muls); sep = ", "; }
    if f.divs > 0 { append(*b, sep); print_to_builder(*b, "% div", f.divs); sep = ", "; }
    if f.adds > 0 { append(*b, sep); print_to_builder(*b, "% add", f.adds); sep = ", "; }
    if f.transcendentals > 0 { append(*b, sep); print_to_builder(*b, "% transcendental", f.transcendentals); sep = ", "; }
    total := f.muls + f.divs + f.adds + f.transcendentals;
    if total == 0  return "0 ops";
    return tprint("% ops (%)", total, builder_to_string(*b));
}
flop_total :: (f: Flop_Count) -> int { return f.muls + f.divs + f.adds + f.transcendentals; }

element_group_has_node :: (elements: [] Element, node: string) -> bool {
    for el : elements  for term : el.terminals  if term == node  return true;
    return false;
}

// Emits one complete unit for `elements`: Params<suffix>/State<suffix>,
// compute() and reset() for every circuit.
generate_unit :: (elements: [..] Element, inputs: [] string, outputs: [] string, values: [] Element_Value,
                  backend: Backend, label: string, suffix: string,
                  newton_tol_sq: float64, newton_max_iter: int) -> (code: string, has_nonlinear: bool) {
    elements_copy := elements; // reduce_circuit mutates in place
    has_nonlinear := circuit_has_nonlinear(elements_copy);

    outer_code, loop_code: string;
    outer_flops, inner_flops: Flop_Count;
    outer_temp_count, inner_temp_count: int;
    clusters: [] Nonlinear_Cluster;
    cluster_exprs: [] Expr;

    if has_nonlinear {
        log("Nonlinear system: resolving with Newton-Raphson (tol²=%, max_iter=%)\n", newton_tol_sq, newton_max_iter);

        // Same as solve_and_codegen(), but with build_nonlinear_clusters()
        // spliced in between solve() and generate_expressions(). See
        // nonlinear_solve.jai for why the ordering matters.
        reduce_circuit(*elements_copy, inputs, outputs);
        solve_vars, equations, pool, kcl_elements := generate_kcl_equations(elements_copy, inputs);
        check := build_magnitude_check(elements_copy, values, backend, label);
        solutions := solve(equations, solve_vars, *pool, check);

        clusters = build_nonlinear_clusters(kcl_elements, solve_vars, solutions, *pool, backend);
        for c : clusters {
            unknown_names: [..] string;
            for u : c.unknowns  array_add(*unknown_names, u.name);
            log("  Cluster \"%\": % joint unknown(s): %\n", c.name, c.unknowns.count, String.join(..unknown_names, separator = ", "));
            if c.amp_clip_idx >= 0 {
                log("    Active-set clipping on %, generating pinned-high/pinned-low reduced Newton systems\n", c.unknowns[c.amp_clip_idx].name);
            }
        }

        // Junction-voltage unknowns only appear in the tree *after*
        // nonlinear_solve.jai substitutes them in for branch-current symbols.
        cluster_variant_names: [..] string;
        for c : clusters  for u : c.unknowns  array_add(*cluster_variant_names, u.name);

        gen_vars := generate_expressions(kcl_elements, outputs, solve_vars, solutions, *pool);
        gen_var_ids: [..] Expr_ID;
        for gv : gen_vars  array_add(*gen_var_ids, gv.expr_id);

        cse_vars := cse(gen_var_ids, *pool);
        sort_cse_topological(*cse_vars, pool.exprs);

        licm_variant_base := build_variant_base(kcl_elements, inputs);
        for gv : gen_vars  array_add(*licm_variant_base, gv.name);
        for n : cluster_variant_names  array_add(*licm_variant_base, n);
        licm(gen_var_ids, *cse_vars, *pool, licm_variant_base);
        recip_hoist(gen_var_ids, *cse_vars, *pool, licm_variant_base);

        outer_code, loop_code, outer_flops, inner_flops, outer_temp_count, inner_temp_count =
            codegen(kcl_elements, inputs, gen_vars, cse_vars, pool.exprs, backend, cluster_variant_names);
        cluster_exprs = pool.exprs;
    } else {
        outer_code, loop_code, outer_flops, inner_flops, outer_temp_count, inner_temp_count =
            solve_and_codegen(*elements_copy, inputs, outputs, values, backend, label);
    }

    log("  Outer loop: %\n  Inner loop: %\n", flop_str(outer_flops), flop_str(inner_flops));
    log("  Temp vars: % outer, % inner, % total\n", outer_temp_count, inner_temp_count, outer_temp_count + inner_temp_count);

    newton_worst_case := 0;
    if has_nonlinear {
        for c : clusters {
            use_port  := use_port_form(c, backend);
            setup_flops: Flop_Count;
            if use_port {
                setup_flops = count_flops(c.port.invariant_items, cluster_exprs, backend.transcendental_approx);
                voc_flops  := count_flops(c.port.voc_items, cluster_exprs, backend.transcendental_approx);
                setup_flops.muls += voc_flops.muls;
                setup_flops.divs += voc_flops.divs;
                setup_flops.adds += voc_flops.adds;
                setup_flops.transcendentals += voc_flops.transcendentals;
            } else {
                setup_flops = count_flops(c.invariant_items, cluster_exprs, backend.transcendental_approx);
            }
            iter_flops  := ifx use_port
                           then port_newton_iteration_flops(c, cluster_exprs, backend)
                           else count_flops(c.iteration_items, cluster_exprs, backend.transcendental_approx);
            log("  Cluster \"%1\" Newton setup: %2%3\n", c.name, flop_str(setup_flops), ifx use_port then " (N-port)" else "");
            log("  Cluster \"%1\" Newton iteration: %2 × up to %3 iters%4\n", c.name, flop_str(iter_flops), newton_max_iter, ifx use_port then " (N-port)" else "");
            if use_port {
                laws := count_flops(c.port.iteration_items, cluster_exprs, backend.transcendental_approx);
                log("    device laws: %\n", flop_str(laws));
                log("    symbolic path: %\n", flop_str(count_flops(c.iteration_items, cluster_exprs, backend.transcendental_approx)));
            }
            if c.invariant_items.count == 0 {
                log("    No Newton-invariant subexpressions found, nothing hoisted out of the per-iteration loop\n");
            }
            newton_worst_case += flop_total(setup_flops) + newton_max_iter * flop_total(iter_flops);
        }
    }
    log("  Estimated worst-case ops/sample: % (% outer + % inner%)\n",
        flop_total(outer_flops) + flop_total(inner_flops) + newton_worst_case,
        flop_total(outer_flops), flop_total(inner_flops),
        ifx has_nonlinear then tprint(" + % Newton (worst case)", newton_worst_case) else "");

    builder: String_Builder;

    append(*builder, generate_params_struct(elements_copy, values, backend, suffix));
    append(*builder, "\n");

    // v[n-1] (the unknown itself, warm start) and v[n-2] (for the
    // extrapolated initial guess below).
    extra_state_names: [..] string;
    for c : clusters  for u : c.unknowns {
        array_add(*extra_state_names, u.name);
        array_add(*extra_state_names, newton_prev_state_name(u.name));
    }

    // -instrument: one solve counter and one iteration counter per cluster.
    counter_names: [..] string;
    if backend.instrument && has_nonlinear {
        for c : clusters {
            solves_name, iters_name := newton_counter_names(c.name);
            array_add(*counter_names, solves_name);
            array_add(*counter_names, iters_name);
        }
    }
    state_struct_code, states := generate_state_struct(elements_copy, extra_state_names, backend, suffix, counter_names);
    append(*builder, state_struct_code);
    append(*builder, "\n");

    print_to_builder(*builder, "static void compute% (", suffix);
    if inputs.count == 1 {
        append(*builder, "const float* const* input, ");
    } else {
        for input : inputs  append(*builder, tprint("const float* const* input_%, ", input));
    }
    if outputs.count == 1 {
        append(*builder, "float** output, ");
    } else {
        for output : outputs  append(*builder, tprint("float** output_%, ", output));
    }
    print_to_builder(*builder, "int num_channels, int num_samples, Params% params, State%* state, float sample_rate)\n", suffix, suffix);
    append(*builder, "{\n");

    // basis_names for basis-vector extraction per-cluster: inputs + z-state only.
    // Anything referencing a branch current directly still needs to be excluded
    // from extraction, not treated as basis-independent, so pass those along too.
    basis_names := extraction_basis_names(elements_copy, inputs);
    branch_current_names := nonlinear_branch_current_names(elements_copy);
    cluster_newton_outer: [..] string;
    cluster_newton_inner: [..] string;
    if has_nonlinear {
        for c : clusters {
            c_outer, c_inner: string;
            if use_port_form(c, backend) {
                c_outer, c_inner = generate_cluster_port_newton_code(c, cluster_exprs, backend, enable_extraction = true, basis_names = basis_names,
                                                                    non_basis_variant_names = branch_current_names, count_iterations = backend.instrument);
            } else {
                c_outer, c_inner = generate_cluster_newton_code(c, cluster_exprs, backend, enable_extraction = true, basis_names = basis_names, non_basis_variant_names = branch_current_names,
                                                               count_iterations = backend.instrument);
            }
            array_add(*cluster_newton_outer, c_outer);
            array_add(*cluster_newton_inner, c_inner);
        }
    }

    append(*builder, indent(generate_impedance_defs(elements_copy, backend)));
    if has_nonlinear  append(*builder, indent(generate_vcrit_defs(clusters, backend)));
    if outer_code.count > 0  append(*builder, indent(outer_code));
    for c_outer : cluster_newton_outer  if c_outer.count > 0  append(*builder, indent(c_outer));

    append(*builder, "    for (int ch = 0; ch < num_channels; ++ch)\n    {\n");
    for state : states  append(*builder, tprint("        %1 %2 = state[ch].%2;\n", backend.decl_type, state));
    for name : counter_names  append(*builder, tprint("        %1 %2 = state[ch].%2;\n", NEWTON_COUNTER_TYPE, name));

    append(*builder, "        for (int n = 0; n < num_samples; ++n)\n        {\n");

    if inputs.count == 1 {
        append(*builder, tprint("            const %1 %2 = input[ch][n];\n\n", backend.decl_type, inputs[0]));
    } else {
        for input : inputs  append(*builder, tprint("            const %1 %2 = input_%2[ch][n];\n", backend.decl_type, input));
        append(*builder, "\n");
    }

    if has_nonlinear {
        // Extrapolated initial guess: v_pred = 2*v[n-1] - v[n-2], applied as
        // a step through the unknown's own limiter.
        for c : clusters {
            for u : c.unknowns {
                prev_name := newton_prev_state_name(u.name);
                print_to_builder(*builder, "            { const %1 _prev_step = %2 - %3; %3 = %2;\n", backend.decl_type, u.name, prev_name);
                emit_unknown_update(*builder, u, "_prev_step");
                append(*builder, "            }\n");
            }
        }
        append(*builder, "\n");
        for newton_code : cluster_newton_inner {
            append(*builder, tprint("%\n", indent(newton_code, "            ")));
        }
    }

    append(*builder, tprint("%\n", indent(loop_code, "            ")));

    if outputs.count == 1 {
        append(*builder, tprint("            output[ch][n] = %;\n", outputs[0]));
    } else {
        for output : outputs  append(*builder, tprint("            output_%1[ch][n] = %1;\n", output));
    }
    append(*builder, "        }\n");

    for state : states  append(*builder, tprint("        state[ch].%1 = %1;\n", state));
    for name : counter_names  append(*builder, tprint("        state[ch].%1 = %1;\n", name));
    append(*builder, "    }\n}\n");

    append(*builder, "\n");
    append(*builder, generate_reset_function(elements_copy, inputs, outputs, backend, suffix, counter_names));

    return builder_to_string(*builder), has_nonlinear;
}

// A struct wrapping every named sub-circuit's own struct/function as a
// field, plus compute()/reset() that just call each sub-circuit's
// compute_<name>()/reset_<name>() in sequence, in place.
generate_joined_unit :: (decls: [] Sub_Circuit_Decl, backend: Backend) -> string {
    builder: String_Builder;

    append(*builder, struct_open(backend, "Params"));
    for d : decls  print_to_builder(*builder, "    Params_% %;\n", d.name, d.name);
    append(*builder, struct_close(backend, "Params"));
    append(*builder, "\n");

    append(*builder, struct_open(backend, "State"));
    for d : decls  print_to_builder(*builder, "    State_% %;\n", d.name, d.name);
    append(*builder, struct_close(backend, "State"));
    append(*builder, "\n");

    append(*builder, "static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)\n{\n");
    append(*builder, "    for (int ch = 0; ch < num_channels; ++ch)\n    {\n");
    for d, i : decls {
        in_arg := ifx i == 0 then "&input[ch]" else "&output[ch]";
        print_to_builder(*builder, "        compute_%1(%2, &output[ch], 1, num_samples, params.%1, &state[ch].%1, sample_rate);\n", d.name, in_arg);
    }
    append(*builder, "    }\n}\n");

    // Joined reset(): seed the first stage's DC level from the caller, then
    // chain each stage's settled DC output into the next stage's input bias.
    print_to_builder(*builder, "\nstatic void reset (Params params, State* state, int num_channels, float sample_rate%1)\n{\n",
                      param_decl_with_default(backend, "float", tprint("%_dc", decls[0].in_node), "0.0f"));
    append(*builder, "    for (int ch = 0; ch < num_channels; ++ch)\n    {\n");
    prev_dc_var := "";
    for d, i : decls {
        chain_arg := ifx i == 0
            then tprint(", %_dc", decls[0].in_node)
            else ifx d.in_node == decls[i-1].out_node then tprint(", %", prev_dc_var) else "";
        dc_var := tprint("%_dc", d.name);
        print_to_builder(*builder, "        const float %1 = reset_%2(params.%2, &state[ch].%2, 1, sample_rate%3);\n", dc_var, d.name, chain_arg);
        prev_dc_var = dc_var;
    }
    append(*builder, "    }\n}\n");

    return builder_to_string(*builder);
}

// Main entry point!
netlist_codegen :: (netlist_str: string, type_name: string = "float", lang: string = "cpp", namespace: string = "", label: string = "", cpu_profile: CPU_Profile = CPU_PROFILE_GENERIC,
                    enable_extraction := true, opt_port_matrix := true, instrument := false) -> string {
    // parse the netlist into our internal structure
    net := parse_netlist(netlist_str);

    // choose the language backend (C, C++, ...)
    backend := make_backend(lang, type_name, cpu_profile, net.transcendental_approx, enable_extraction, instrument, opt_port_matrix);

    if net.outputs.count == 0 {
        log("Adding output node: vo\n");
        array_add(*net.outputs, "vo");
    }

    // break up the circuit into sub-circuits
    partitions := partition_circuit(net.elements, net.sub_circuits);

    builder: String_Builder;
    append(*builder, "#pragma once\n\n");

    file_has_nonlinear := circuit_has_nonlinear(net.elements);
    if file_has_nonlinear  append(*builder, math_include(backend));

    if namespace.count > 0 && backend.is_cpp  append(*builder, tprint("namespace %\n{\n\n", namespace));

    // Nonlinear solver helpers
    if file_has_nonlinear {
        if !backend.transcendental_approx.exact && circuit_uses_transcendentals(net.elements) {
            append(*builder, generate_transcendental_approx_helpers(backend));
            append(*builder, "\n");
        }
        if circuit_uses_limiter_kind(net.elements, .EXP_JUNCTION) { append(*builder, generate_junction_limiter_helper(backend)); append(*builder, "\n"); }
        if circuit_uses_limiter_kind(net.elements, .EXP_JUNCTION_SYM) { append(*builder, generate_junction_limiter_sym_helper(backend)); append(*builder, "\n"); }
        if circuit_uses_limiter_kind(net.elements, .JFET_PINCHOFF) { append(*builder, generate_jfet_limiter_helper(backend)); append(*builder, "\n"); }
        if circuit_uses_limiter_kind(net.elements, .AMP_CLIP) { append(*builder, generate_opamp_clip_limiter_helper(backend)); append(*builder, "\n"); }
        append(*builder, generate_newton_constants(net.newton_tol_sq, net.newton_max_iter, backend));
        append(*builder, "\n");
    }

    all_single_io := true;
    for part : partitions {
        local_inputs: [..] string;
        for inp : net.inputs  if element_group_has_node(part.elements, inp)  array_add_if_unique(*local_inputs, inp);
        if part.in_node.count > 0  array_add_if_unique(*local_inputs, part.in_node);

        local_outputs: [..] string;
        for outp : net.outputs  if element_group_has_node(part.elements, outp)  array_add_if_unique(*local_outputs, outp);
        if part.out_node.count > 0  array_add_if_unique(*local_outputs, part.out_node);

        if local_inputs.count != 1 || local_outputs.count != 1  all_single_io = false;

        suffix := ifx part.name.count > 0 then tprint("_%", part.name) else "";
        unit_label := ifx part.name.count > 0 then tprint("%[%]", label, part.name) else label;

        if net.sub_circuits.count > 0 {
            log("[%] % element(s), inputs: [%], outputs: [%]\n", ifx part.name.count > 0 then part.name else "?",
                part.elements.count, String.join(..local_inputs, separator = ", "), String.join(..local_outputs, separator = ", "));
        }

        code, has_nonlinear := generate_unit(part.elements, local_inputs, local_outputs, net.values,
                                             backend, unit_label, suffix, net.newton_tol_sq, net.newton_max_iter);
        append(*builder, code);
        append(*builder, "\n");
    }

    if net.sub_circuits.count > 1 {
        if all_single_io {
            append(*builder, generate_joined_unit(net.sub_circuits, backend));
        } else {
            log("Warning: sub-circuits with more than one input/output aren't supported for the joined compute().\n");
        }
    }

    if namespace.count > 0 && backend.is_cpp  append(*builder, tprint("\n} // namespace %\n", namespace));

    return builder_to_string(*builder);
}
