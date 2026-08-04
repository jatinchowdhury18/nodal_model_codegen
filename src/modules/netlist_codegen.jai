#import "Basic";
String :: #import "String";
Math :: #import "Math";

#import "nodal_analysis";
#import "sym_algebra";
#import "nonlinear_solve";
#import "codegen";

// Thermal voltage kT/q at room temperature (~300K).
// For diodes/BJTs, the "vt" parameter is computed as
// VT_ROOM * N (for "ideality factor" N), since N isn't
// used anywhere else.
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

    // TRIODE only (Dempwolf & Zolzer model) -- see triode_iv_laws() (nonlinear_solve.jai).
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

// Parse a bare number with an optional trailing SPICE SI-scale suffix
// (M=1e6, k=1e3, m=1e-3, u=1e-6, n=1e-9, p=1e-12), e.g. "2m" -> 0.002.
// Used both for element values (R/C/L/...) and .model key=val parameters.
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

// Parse a `.model` line (leading "." already stripped/trimmed by the
// caller), e.g. "model DCLIP D" or "model Q1M NPN (Is=2e-14 Bf=200 Br=4)".
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

// Part numbers like "2N5485" aren't valid C identifiers; prefix with "_" when
// building Params field names (e.g. "2N5485_Beta"). find_model() must keep
// using the raw, un-prefixed name for lookups.
ident_safe_name :: (name: string) -> string {
    if name.count > 0 && is_digit(name[0])  return tprint("_%", name);
    return name;
}

// Look up a parsed .model card by name, falling back to a default-parameter model.
find_model :: (model_defs: [] Model_Def, name: string) -> Model_Def {
    for m : model_defs  if m.name == name  return m;
    model := default_model();
    model.name = name;
    return model;
}

// Peek at a `.model <name> <kind> ...` line's kind token (leading "."
// already stripped/trimmed by the caller) without fully parsing it -- used
// to route a `.model` line to parse_model_line() or parse_opamp_model_line().
peek_model_kind :: (line: string) -> Model_Kind {
    tokens_raw := String.split(line, #char " ");
    tokens: [..] string;
    for t : tokens_raw  if t.count > 0  array_add(*tokens, t);
    if tokens.count < 3  return .UNKNOWN;
    return parse_model_kind(tokens[2]);
}

// A parsed SPICE-style `.model <name> OPAMP (Aol=.. GBW=.. Ccomp=.. Rin=..
// Rout=.. Vsat+=.. Vsat-=.. slew=.. Vos=.. Ios=.. Ibias=..)` card. Every
// param is independently optional -- unlike D/Q models there's no SPICE
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

// Parse a `.model <name> OPAMP (...)` line (leading "." already stripped/
// trimmed by the caller). Mirrors parse_model_line's key=value scanning.
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

// Look up a parsed .model OPAMP card by name. A not-found name (or no card
// at all) falls back to an all-"has_*=false" model, i.e. ideal.
find_opamp_model :: (model_defs: [] Opamp_Model_Def, name: string) -> Opamp_Model_Def {
    for m : model_defs  if m.name == name  return m;
    model: Opamp_Model_Def;
    model.name = name;
    return model;
}

// Register a (name, value) pair in `values` at most once -- multiple
// elements referencing the same .model card must not produce duplicate
// Params struct fields.
add_value_once :: (values: *[..] Element_Value, name: string, value: float32) {
    for v : values.*  if v.name == name  return;
    array_add(values, Element_Value.{ name = name, value = value });
}

Netlist_Info :: struct {
    elements : [..] Element;
    values : [..] Element_Value;

    inputs : [..] string;
    outputs : [..] string;

    // Newton-Raphson solver defaults.
    newton_tol_sq   := 1.0e-5;
    newton_max_iter := 20;
}

parse_netlist :: (netlist_str: string) -> Netlist_Info {
    using netlist_info : Netlist_Info;

    add_element :: (name: string, type: Element_Type, terminals: [] string, no_combine := false) #expand {
        element := Element.{ name = name, type = type, no_combine = no_combine };
        array_copy(*element.terminals, terminals);
        array_add(*elements, element);
    }

    lines := String.split(netlist_str, "\n");

    // Strip a ';'-style inline comment. Callers must do this AFTER checking for the "netlist_skip".
    strip_comment :: (line: string) -> string {
        parts := String.split(line, #char ";");
        return ifx parts.count > 0 then String.trim(parts[0]) else line;
    }

    // A trailing "; nodal_model: <name> [out=..] [pos=..] [neg=..]" reference
    // on an element line -- names an op-amp model defined by a
    // `.model <name> OPAMP (...)` card elsewhere in the file. Needed because,
    // unlike D/Q/J/X, an op-amp instance has no dedicated element line of its
    // own that already carries a model-name token in a fixed position. Only
    // needs overriding when the real SPICE part's pin order doesn't match.
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

    // First pass: collect .model cards (D/NPN/PNP/JFET/TRIODE/OPAMP). SPICE
    // has no native op-amp or triode primitive, but (like TRIODE) an OPAMP
    // .model card is still just an ordinary line. A `.SUBCKT ... .ENDS` block
    // is skipped entirely here.
    model_defs: [..] Model_Def;
    opamp_model_defs: [..] Opamp_Model_Def;
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
                array_add(*opamp_model_defs, parse_opamp_model_line(line));
            } else {
                array_add(*model_defs, parse_model_line(line));
            }
        }
    }

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
            // `nr_solve(max_iter=<int>, tol=<val>)`.
            if String.begins_with(line, "nr_solve") {
                rest := String.slice(line, 8, line.count - 8);
                rest, _ = String.replace(rest, "(", " ");
                rest, _ = String.replace(rest, ")", " ");
                rest, _ = String.replace(rest, ",", " ");
                kv_tokens_raw := String.split(rest, #char " ");
                for kv : kv_tokens_raw {
                    if kv.count == 0  continue;
                    parts := String.split(kv, #char "=");
                    if parts.count != 2  continue;
                    key := String.to_upper_copy(String.trim(parts[0]));
                    val, success := string_to_float(String.trim(parts[1]));
                    if !success  continue;
                    if key == {
                        case "TOL";      newton_tol_sq = val;
                        case "MAX_ITER"; newton_max_iter = cast(int) val;
                    }
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
        if ! Math.is_nan_is_inf(value)  array_add(*values, Element_Value.{ name, value }); // unparsed value (e.g. input voltage source) defaults to 0

        // `; nodal_model: <name>` opts ANY element line (a real SPICE `E`,
        // an `X` instantiating a library part, ...) into our non-ideal
        // op-amp model, and completely ignores whatever else the line says.
        // out/pos/neg default to that line's tokens[1]/[3]/[4] unless
        // overridden in the reference.
        if nodal_model_ref.name.count > 0 {
            model := find_opamp_model(opamp_model_defs, nodal_model_ref.name);
            out_node := ifx nodal_model_ref.out_override.count > 0 then nodal_model_ref.out_override else tokens[1];
            pos_node := ifx nodal_model_ref.pos_override.count > 0 then nodal_model_ref.pos_override else tokens[3];
            neg_node := ifx nodal_model_ref.neg_override.count > 0 then nodal_model_ref.neg_override else tokens[4];

            // Rin/Rout/GBW/Ccomp/Vsat/slew only make sense on a
            // distinguishable finite-gain output node, so they all require
            // Aol too.
            needs_aol := model.has_rin || model.has_rout || model.has_vsat_p || model.has_vsat_n || model.has_slew || model.has_gbw || model.has_ccomp;
            if needs_aol && !model.has_aol {
                log("Warning: nodal_model '%' sets Rin/Rout/Vsat/slew/GBW/Ccomp without Aol; treating '%' as ideal.\n", nodal_model_ref.name, name);
                add_element(name, .AMP, .[out_node, neg_node, pos_node]);
                continue;
            }

            opamp: Opamp_Model;
            if model.has_aol    { opamp.has_aol = true;    opamp.aol_name    = tprint("%_Aol", name);   add_value_once(*values, opamp.aol_name, model.aol_val); }
            if model.has_gbw || model.has_ccomp  opamp.has_gbw = true; // no Params field of its own -- only used below to derive the synthesized Ccomp/Rcomp defaults
            if model.has_rin    { opamp.has_rin = true;    opamp.rin_name    = tprint("%_Rin", name);   add_value_once(*values, opamp.rin_name, model.rin_val); }
            if model.has_rout   { opamp.has_rout = true;   opamp.rout_name   = tprint("%_Rout", name);  add_value_once(*values, opamp.rout_name, model.rout_val); }
            // clamp_opamp_output always takes both rails, so both fields
            // get populated whenever any clip characteristic is set.
            if model.has_vsat_p || model.has_vsat_n || model.has_slew {
                opamp.has_vsat_p = model.has_vsat_p;
                opamp.has_vsat_n = model.has_vsat_n;
                opamp.vsat_p_name = tprint("%_VsatP", name);
                opamp.vsat_n_name = tprint("%_VsatN", name);
                p_val := ifx model.has_vsat_p then model.vsat_p_val else 1.0e30;
                n_val := ifx model.has_vsat_n then model.vsat_n_val else -1.0e30;
                add_value_once(*values, opamp.vsat_p_name, cast(float32) p_val);
                add_value_once(*values, opamp.vsat_n_name, cast(float32) n_val);
            }
            if model.has_slew   { opamp.has_slew = true;   opamp.slew_name   = tprint("%_Slew", name);  add_value_once(*values, opamp.slew_name, model.slew_val); }
            if model.has_vos    { opamp.has_vos = true;    opamp.vos_name    = tprint("%_Vos", name);   add_value_once(*values, opamp.vos_name, model.vos_val); }
            if model.has_ios    { opamp.has_ios = true;    opamp.ios_name    = tprint("%_Ios", name);   add_value_once(*values, opamp.ios_name, model.ios_val); }
            if model.has_ibias  { opamp.has_ibias = true;  opamp.ibias_name  = tprint("%_Ibias", name); add_value_once(*values, opamp.ibias_name, model.ibias_val); }

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

                rcomp_name := tprint("%_Rcomp", name);
                rcomp_el := Element.{ name = rcomp_name, type = .RESISTOR, no_combine = true };
                array_add(*rcomp_el.terminals, opamp_gain_node(element));
                array_add(*rcomp_el.terminals, opamp_bandwidth_node(element));
                add_value_once(*values, rcomp_name, cast(float32) r_comp);
                array_add(*elements, rcomp_el);

                ccomp_name := tprint("%_Ccomp", name);
                ccomp_el := Element.{ name = ccomp_name, type = .CAPACITOR, no_combine = true };
                array_add(*ccomp_el.terminals, opamp_bandwidth_node(element));
                array_add(*ccomp_el.terminals, "0");
                add_value_once(*values, ccomp_name, cast(float32) c_comp);
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
                // Ideal op-amp: `E<name> out ref pos neg <gain>` (gain
                // discarded -- infinite gain). Non-ideal behavior is opted
                // into with a `; nodal_model: <name>` comment instead (see
                // above), which works on this or any other element line.
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
                    if !Math.is_nan_is_inf(dc_value)  add_value_once(*values, name, dc_value);
                }

                if is_fixed {
                    etype = .FIXED_VOLTAGE;
                } else {
                    array_add(*inputs, n1); // treat voltage source as input node
                    continue; // don't include in elements
                }
            case "D";
                // D<name> <anode> <cathode> <model>
                model := find_model(model_defs, tokens[3]);
                is_name := tprint("%_Is", ident_safe_name(model.name));
                vt_name := tprint("%_vt", ident_safe_name(model.name));
                add_value_once(*values, is_name, model.is_val);
                add_value_once(*values, vt_name, cast(float32)(model.n_val * VT_ROOM));

                element := Element.{ name = name, type = .DIODE, no_combine = has_no_combine };
                array_add(*element.terminals, n1); // anode
                array_add(*element.terminals, n2); // cathode
                element.nonlinear = .{ is_name = is_name, vt_name = vt_name };
                array_add(*elements, element);
                continue;
            case "Q";
                // Q<name> <collector> <base> <emitter> <model>.
                emitter    := tokens[3];
                model      := find_model(model_defs, tokens[4]);
                if model.kind == {
                    case .NPN; #through;
                    case .PNP;
                        is_name    := tprint("%_Is", ident_safe_name(model.name));
                        vt_name    := tprint("%_vt", ident_safe_name(model.name));
                        betaf_name := tprint("%_BetaF", ident_safe_name(model.name));
                        betar_name := tprint("%_BetaR", ident_safe_name(model.name));
                        add_value_once(*values, is_name, model.is_val);
                        add_value_once(*values, vt_name, cast(float32)(model.n_val * VT_ROOM));
                        add_value_once(*values, betaf_name, model.betaf_val);
                        add_value_once(*values, betar_name, model.betar_val);

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
                // J<name> <drain> <gate> <source> <model>. PJF mirrors NJF
                // (see nonlinear_device_spec's .PJF case) -- only the element
                // type differs.
                source := tokens[3];
                model  := find_model(model_defs, tokens[4]);
                if model.kind == {
                    case .NJF; #through;
                    case .PJF;
                        beta_name := tprint("%_Beta", ident_safe_name(model.name));
                        vp_name   := tprint("%_vp", ident_safe_name(model.name));
                        add_value_once(*values, beta_name, model.beta_val);
                        add_value_once(*values, vp_name, model.vp_val);

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
                if !Math.is_nan_is_inf(gm_value)  add_value_once(*values, name, gm_value);

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
                // X<name> <term1> <term2> ... <termN> <model> -- generic
                // subcircuit instantiation. Real SPICE always puts the
                // subckt/model name *last*, regardless of terminal count, so
                // we look it up positionally from the end.
                if tokens.count < 3 {
                    log("Warning: malformed X-line '%'; ignoring.\n", name);
                    continue;
                }
                model     := find_model(model_defs, tokens[tokens.count - 1]);
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
                        add_value_once(*values, gk_name,    model.gk_val);
                        add_value_once(*values, mu_name,    model.mu_val);
                        add_value_once(*values, gamma_name, model.gamma_val);
                        add_value_once(*values, ck_name,    model.ck_val);
                        add_value_once(*values, gg_name,    model.gg_val);
                        add_value_once(*values, xi_name,    model.xi_val);
                        add_value_once(*values, cg_name,    model.cg_val);
                        add_value_once(*values, ig0_name,   model.ig0_val);

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

netlist_codegen :: (netlist_str: string, type_name: string = "float", lang: string = "cpp", namespace: string = "") -> string {
    backend := make_backend(lang, type_name);
    net := parse_netlist(netlist_str);

    if net.outputs.count == 0 {
        log("Adding output node: vo\n");
        array_add(*net.outputs, "vo");
    }

    has_nonlinear := false;
    for el : net.elements {
        if nonlinear_device_spec(el.type).unknowns.count > 0 { has_nonlinear = true; break; }
        // AMP_CLIP isn't in nonlinear_device_spec() -- see circuit_uses_limiter_kind.
        if el.type == .AMP && opamp_has_clip_stage(el) { has_nonlinear = true; break; }
    }

    outer_code: string;
    loop_code:  string;
    outer_flops, inner_flops: Flop_Count;
    clusters: [] Nonlinear_Cluster;
    cluster_exprs: [] Expr; // only valid/used when has_nonlinear

    flop_str :: (f: Flop_Count) -> string #expand {
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
    flop_total :: (f: Flop_Count) -> int #expand { return f.muls + f.divs + f.adds + f.transcendentals; }

    if has_nonlinear {
        log("Nonlinear system: resolving with Newton-Raphson (tol²=%, max_iter=%)\n", net.newton_tol_sq, net.newton_max_iter);

        // Same as solve_and_codegen(), but with build_nonlinear_clusters()
        // spliced in between solve() and generate_expressions() -- see
        // nonlinear_solve.jai for why the ordering matters.
        reduce_circuit(*net.elements, net.inputs, net.outputs);
        solve_vars, equations, pool, kcl_elements := generate_kcl_equations(net.elements, net.inputs);
        solutions := solve(equations, solve_vars, *pool);

        clusters = build_nonlinear_clusters(kcl_elements, solve_vars, solutions, *pool);

        for c : clusters {
            unknown_names: [..] string;
            for u : c.unknowns  array_add(*unknown_names, u.name);
            log("  Cluster \"%\": % joint unknown(s): %\n", c.name, c.unknowns.count, String.join(..unknown_names, separator = ", "));
            if c.amp_clip_idx >= 0 {
                log("    Active-set clipping on % -- generating pinned-high/pinned-low reduced Newton systems\n", c.unknowns[c.amp_clip_idx].name);
            }
        }

        // Junction-voltage unknowns (vD1, vBCQ1, ...) only appear in the tree
        // *after* nonlinear_solve.jai substitutes them in for branch-current
        // symbols.
        cluster_variant_names: [..] string;
        for c : clusters  for u : c.unknowns  array_add(*cluster_variant_names, u.name);

        gen_vars := generate_expressions(kcl_elements, net.outputs, solve_vars, solutions, *pool);

        gen_var_ids: [..] Expr_ID;
        for gv : gen_vars  array_add(*gen_var_ids, gv.expr_id);

        cse_vars := cse(gen_var_ids, *pool);
        sort_cse_topological(*cse_vars, pool.exprs);

        licm_variant_base := build_variant_base(kcl_elements, net.inputs);
        for gv : gen_vars  array_add(*licm_variant_base, gv.name);
        for n : cluster_variant_names  array_add(*licm_variant_base, n);
        licm(gen_var_ids, *cse_vars, *pool, licm_variant_base);
        recip_hoist(gen_var_ids, *cse_vars, *pool, licm_variant_base);

        outer_code, loop_code, outer_flops, inner_flops =
            codegen(kcl_elements, net.inputs, gen_vars, cse_vars, pool.exprs, backend, cluster_variant_names);
        cluster_exprs = pool.exprs;
    } else {
        outer_code, loop_code, outer_flops, inner_flops =
            solve_and_codegen(*net.elements, net.inputs, net.outputs, backend);
    }

    log("  Outer loop: %\n  Inner loop: %\n", flop_str(outer_flops), flop_str(inner_flops));

    newton_worst_case := 0;
    if has_nonlinear {
        for c : clusters {
            setup_flops := count_flops(c.invariant_items, cluster_exprs);
            iter_flops  := count_flops(c.iteration_items, cluster_exprs);
            log("  Cluster \"%\" Newton setup: %\n", c.name, flop_str(setup_flops));
            log("  Cluster \"%\" Newton iteration: % × up to % iters\n", c.name, flop_str(iter_flops), net.newton_max_iter);
            if c.invariant_items.count == 0 {
                log("    No Newton-invariant subexpressions found -- nothing hoisted out of the per-iteration loop\n");
            }
            newton_worst_case += flop_total(setup_flops) + net.newton_max_iter * flop_total(iter_flops);
        }
    }
    log("  Estimated worst-case ops/sample: % (% outer + % inner%)\n",
        flop_total(outer_flops) + flop_total(inner_flops) + newton_worst_case,
        flop_total(outer_flops), flop_total(inner_flops),
        ifx has_nonlinear then tprint(" + % Newton (worst case)", newton_worst_case) else "");

    builder: String_Builder;
    append(*builder, "#pragma once\n\n");
    if has_nonlinear {
        append(*builder, math_include(backend));
    }

    if namespace.count > 0 && backend.is_cpp {
        append(*builder, tprint("namespace %\n{\n\n", namespace));
    }

    params_struct_code := generate_params_struct(net.elements, net.values, backend);
    append(*builder, params_struct_code);
    append(*builder, "\n");

    extra_state_names: [..] string;
    for c : clusters  for u : c.unknowns  array_add(*extra_state_names, u.name);
    state_struct_code, states := generate_state_struct(net.elements, extra_state_names, backend);
    append(*builder, state_struct_code);
    append(*builder, "\n");

    if has_nonlinear {
        if circuit_uses_limiter_kind(net.elements, .EXP_JUNCTION) {
            append(*builder, generate_junction_limiter_helper(backend));
            append(*builder, "\n");
        }
        if circuit_uses_limiter_kind(net.elements, .JFET_PINCHOFF) {
            append(*builder, generate_jfet_limiter_helper(backend));
            append(*builder, "\n");
        }
        if circuit_uses_limiter_kind(net.elements, .AMP_CLIP) {
            append(*builder, generate_opamp_clip_limiter_helper(backend));
            append(*builder, "\n");
        }
        append(*builder, generate_newton_constants(net.newton_tol_sq, net.newton_max_iter, backend));
    }

    append(*builder, "static void compute (");
    if net.inputs.count == 1 {
        append(*builder, "const float* const* input, ");
    } else {
        for input : net.inputs  append(*builder, tprint("const float* const* input_%, ", input));
    }
    if net.outputs.count == 1 {
        append(*builder, "float** output, ");
    } else {
        for output : net.outputs  append(*builder, tprint("float** output_%, ", output));
    }
    append(*builder, "int num_channels, int num_samples, Params params, State* state, float sample_rate)\n");
    append(*builder, "{\n");

    append(*builder, indent(generate_impedance_defs(net.elements, backend)));
    if has_nonlinear  append(*builder, indent(generate_vcrit_defs(clusters, backend)));
    if outer_code.count > 0  append(*builder, indent(outer_code));

    append(*builder, "    for (int ch = 0; ch < num_channels; ++ch)\n");
    append(*builder, "    {\n");
    for state : states  append(*builder, tprint("        %1 %2 = state[ch].%2;\n", backend.decl_type, state));

    append(*builder, "        for (int n = 0; n < num_samples; ++n)\n");
    append(*builder, "        {\n");

    if net.inputs.count == 1 {
        append(*builder, tprint("            const %1 %2 = input[ch][n];\n\n", backend.decl_type, net.inputs[0]));
    } else {
        for input : net.inputs {
            append(*builder, tprint("            const %1 %2 = input_%2[ch][n];\n", backend.decl_type, input));
        }
        append(*builder, "\n");
    }

    if has_nonlinear {
        for c : clusters {
            newton_code := generate_cluster_newton_code(c, cluster_exprs, backend);
            append(*builder, tprint("%\n", indent(newton_code, "            ")));
        }
    }

    append(*builder, tprint("%\n", indent(loop_code, "            ")));

    if net.outputs.count == 1 {
        append(*builder, tprint("            output[ch][n] = %;\n", net.outputs[0]));
    } else {
        for output : net.outputs {
            append(*builder, tprint("            output_%1[ch][n] = %1;\n", output));
        }
    }
    append(*builder, "        }\n");

    for state : states append(*builder, tprint("        state[ch].%1 = %1;\n", state));
    append(*builder, "    }\n");

    append(*builder, "}\n");

    if has_nonlinear {
        append(*builder, "\n");
        append(*builder, generate_reset_function(net.elements, net.inputs, backend));
    }

    if namespace.count > 0 && backend.is_cpp {
        append(*builder, tprint("\n} // namespace %\n", namespace));
    }

    return builder_to_string(*builder);
}
