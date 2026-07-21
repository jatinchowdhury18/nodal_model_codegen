#import "Basic";
String :: #import "String";
Math :: #import "Math";

#import "nodal_analysis";
#import "sym_algebra";
#import "nonlinear_solve";

// Thermal voltage kT/q at room temperature (~300K).
// For diodes/BJTs, the "vt" parameter is computed as
// VT_ROOM * N (for "ideality factor" N), since N isn't
// used anywhere else.
VT_ROOM :: 0.02585;

print_param_expr :: (expr: *Param_Expr, param_prefix: string = "params.") -> string {
    build_param_expr :: (e: *Param_Expr, param_prefix: string, builder: *String_Builder) {
        if e == null {
            append(builder, "/* null expr */");
            return;
        }
        if e.op == .NONE {
            print_to_builder(builder, "%1%2", param_prefix, e.leaf_name);
            return;
        }
        if e.op == .SUM  append(builder, "sum(");
        else             append(builder, "recip_sum(");
        build_param_expr(e.left, param_prefix, builder);
        append(builder, ", ");
        build_param_expr(e.right, param_prefix, builder);
        append(builder, ")");
    }

    builder: String_Builder;
    build_param_expr(expr, param_prefix, *builder);
    return builder_to_string(*builder);
}

// Walk a Param_Expr tree and collect all leaf names into `names`.
collect_param_leaf_names :: (expr: *Param_Expr, names: *[..] string) {
    if expr == null  return;
    if expr.op == .NONE {
        array_add_if_unique(names, expr.leaf_name);
        return;
    }
    collect_param_leaf_names(expr.left,  names);
    collect_param_leaf_names(expr.right, names);
}

Element_Value :: struct {
    name: string;
    value: float;
};

// A parsed SPICE `.model <name> <kind> [(key=val ...)]` card. Referenced by
// name from `D`/`Q` element lines. Unset parameters fall back to standard
// SPICE defaults (see parse_model_line / find_model).
Model_Def :: struct {
    name:      string;
    kind:      string; // "D", "NPN", "PNP" -- kept for diagnostics; PNP is not yet handled specially
    is_val:    float32; // diode/BJT saturation current
    n_val:     float32; // diode-only; folded into vt_val at parse time
    betaf_val: float32;
    betar_val: float32;
    vp_val:    float32; // JFET-only pinch-off voltage (VTO)
    beta_val:  float32; // JFET-only transconductance coefficient (BETA).
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
    model.kind = String.to_upper_copy(tokens[2]);

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
    // `* nodal_model: <name> OPAMP (...)` comment elsewhere in the file.
    // Only needs overriding when the real SPICE part's pin order doesn't match.
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

    // First pass: collect .model cards (D/NPN/PNP/JFET) and comment-embedded
    // `nodal_model: <name> OPAMP (...)` op-amp model definitions. OPAMP
    // models live in a comment, not a real `.model` line.
    model_defs: [..] Model_Def;
    opamp_model_defs: [..] Opamp_Model_Def;
    for line_raw : lines {
        line := String.trim(line_raw);
        if line.count == 0  continue;

        if line[0] == #char "*" {
            rest := String.trim(String.slice(line, 1, line.count - 1));
            if String.begins_with(rest, "nodal_model:")  array_add(*opamp_model_defs, parse_opamp_model_line(strip_comment(rest)));
            continue;
        }

        if line[0] != #char "."  continue;
        line = String.trim(String.slice(line, 1, line.count - 1));
        if !String.begins_with(line, "model")  continue;
        array_add(*model_defs, parse_model_line(strip_comment(line)));
    }

    for line_raw : lines {
        line := String.trim(line_raw);

        if line.count == 0  continue;
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
        // print("%\n", tokens);

        name := tokens[0];
        n1, n2 := tokens[1], tokens[2];
        value := parse_spice_value(tokens[3]);
        if ! Math.is_nan_is_inf(value) {
            array_add(*values, Element_Value.{ name, value });
        } else {
            // @TODO: this warning always throws on the input voltage source...
            // log("Warning: couldn't parse value '%' for element '%'; it will default to 0.\n", tokens[3], name);
        }

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
                    case "NPN"; // @TODO: handle PNP here also
                        is_name    := tprint("%_Is", ident_safe_name(model.name));
                        vt_name    := tprint("%_vt", ident_safe_name(model.name));
                        betaf_name := tprint("%_BetaF", ident_safe_name(model.name));
                        betar_name := tprint("%_BetaR", ident_safe_name(model.name));
                        add_value_once(*values, is_name, model.is_val);
                        add_value_once(*values, vt_name, cast(float32)(model.n_val * VT_ROOM));
                        add_value_once(*values, betaf_name, model.betaf_val);
                        add_value_once(*values, betar_name, model.betar_val);

                        element := Element.{ name = name, type = .BJT };
                        array_add(*element.terminals, n1);      // collector
                        array_add(*element.terminals, n2);      // base
                        array_add(*element.terminals, emitter); // emitter
                        element.nonlinear = .{ is_name = is_name, vt_name = vt_name, betaf_name = betaf_name, betar_name = betar_name };
                        array_add(*elements, element);
                }
                continue;
            case "J";
                // J<name> <drain> <gate> <source> <model>.
                source := tokens[3];
                model  := find_model(model_defs, tokens[4]);
                if model.kind == {
                    case "NJF"; // @TODO: handle PJF here also
                        beta_name := tprint("%_Beta", ident_safe_name(model.name));
                        vp_name   := tprint("%_vp", ident_safe_name(model.name));
                        add_value_once(*values, beta_name, model.beta_val);
                        add_value_once(*values, vp_name, model.vp_val);

                        element := Element.{ name = name, type = .JFET };
                        array_add(*element.terminals, n1);     // drain
                        array_add(*element.terminals, n2);     // gate
                        array_add(*element.terminals, source); // source
                        element.nonlinear = .{ beta_name = beta_name, vp_name = vp_name };
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

generate_params_struct :: (elements: [] Element,
                           values: [] Element_Value,
                           type_name: string = "float",
                           lang: string = "cpp") -> string {
    context.print_style.default_format_float.mode = .SCIENTIFIC;
    context.print_style.default_format_float.zero_removal = .ONE_ZERO_AFTER_DECIMAL;

    suffix := ifx type_name == "float" then "f" else ""; // only "float" gets literal f-suffixes; "double" etc. get bare literals
    write_value :: (name: string) -> string #expand {
        for val : values {
            if val.name == name {
                if lang == "c"  return tprint("    % %; // = %\n", type_name, name, val.value);
                          else  return tprint("    %1 %2 = %3%4;\n", type_name, name, val.value, suffix);
            }
        }
        return tprint("    % %;\n", type_name, name);
    }

    builder: String_Builder;
    if lang == "c"  append(*builder, "typedef struct {\n");
    else            append(*builder, "struct Params {\n");

    // DIODE/DIODE_PAIR/BJT parameters belong to a .model card, which may be
    // shared by several elements, so we guard against emitting the same Params
    // field twice.
    emitted_nonlinear_names: [..] string;
    emit_nonlinear_param :: (pname: string) #expand {
        if array_find(emitted_nonlinear_names, pname)  return;
        array_add(*emitted_nonlinear_names, pname);
        append(*builder, write_value(pname));
    }

    for el : elements {
        if el.type == {
            case .RESISTOR; #through;
            case .CAPACITOR; #through;
            case .INDUCTOR;
                if el.combo_expr == null {
                    append(*builder, write_value(el.name));
                } else {
                    leaf_names: [..] string;
                    collect_param_leaf_names(el.combo_expr, *leaf_names);
                    for pname : leaf_names  append(*builder, write_value(pname));
                }
            case .RC_SERIES; #through;
            case .RC_PARALLEL;
                leaf_names: [..] string;
                collect_param_leaf_names(el.rc_internal.res_expr, *leaf_names);
                collect_param_leaf_names(el.rc_internal.cap_expr, *leaf_names);
                for pname : leaf_names  append(*builder, write_value(pname));
            case .FIXED_VOLTAGE;
                append(*builder, write_value(el.name));
            case .DIODE; #through;
            case .DIODE_PAIR; #through;
            case .BJT; #through;
            case .JFET;
                for pname : device_model_param_names(el.type, el.nonlinear)  emit_nonlinear_param(pname);
            case .AMP;
                // Rin/Rout/GBW have no fields of their own here -- they're
                // synthesized RESISTOR/CAPACITOR elements.
                if el.opamp.has_aol    append(*builder, write_value(el.opamp.aol_name));
                if opamp_has_clip_stage(el) {
                    append(*builder, write_value(el.opamp.vsat_p_name));
                    append(*builder, write_value(el.opamp.vsat_n_name));
                }
                if el.opamp.has_slew   append(*builder, write_value(el.opamp.slew_name));
                if el.opamp.has_vos    append(*builder, write_value(el.opamp.vos_name));
                if el.opamp.has_ios    append(*builder, write_value(el.opamp.ios_name));
                if el.opamp.has_ibias  append(*builder, write_value(el.opamp.ibias_name));
        }
    }

    if lang == "c"  append(*builder, "} Params;\n");
    else            append(*builder, "};\n");
    return builder_to_string(*builder);
}

// `extra_state_names` holds nonlinear-cluster junction-voltage unknowns
// (Nonlinear_Cluster.unknowns[].name), which persist across samples as the
// per-sample Newton-Raphson warm start.
generate_state_struct :: (elements: [] Element, extra_state_names: [] string, type_name: string = "float", lang: string = "cpp") -> string, [] string {
    states: [..] string;

    builder: String_Builder;
    if lang == "c"  append(*builder, "typedef struct {\n");
    else            append(*builder, "struct State {\n");

    for el : elements {
        if el.type == {
            case .CAPACITOR; #through;
            case .INDUCTOR; #through;
            case .RC_SERIES; #through;
            case .RC_PARALLEL;
                array_add(*states, tprint("z%", el.name));
                if lang == "c"  append(*builder, tprint("    % %;\n",      type_name, states[states.count-1]));
                else            append(*builder, tprint("    % % {};\n",   type_name, states[states.count-1]));
            case .AMP;
                // zslew_<name>: previous sample's converged output, distinct
                // from vclip_<name>'s own warm-start (unlike a diode, slew
                // needs *last sample's* value as a fixed reference).
                if el.opamp.has_slew {
                    array_add(*states, tprint("zslew_%", el.name));
                    if lang == "c"  append(*builder, tprint("    % %;\n",      type_name, states[states.count-1]));
                    else            append(*builder, tprint("    % % {};\n",   type_name, states[states.count-1]));
                }
        }
    }

    for name : extra_state_names {
        array_add(*states, name);
        if lang == "c"  append(*builder, tprint("    % %;\n",    type_name, name));
        else            append(*builder, tprint("    % % {};\n", type_name, name));
    }

    if lang == "c"  append(*builder, "} State;\n");
    else            append(*builder, "};\n");
    return builder_to_string(*builder), states;
}

generate_impedance_defs :: (elements: [] Element, type_name: string = "float", lang: string = "cpp") -> string {
    builder: String_Builder;

    if lang == "c" {
        append(*builder, "#define sum(a, b) ((a) + (b))\n");
        append(*builder, "#define recip_sum(a, b) ((a) * (b) / ((a) + (b)))\n\n");
    } else {
        append(*builder, "[[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };\n");
        append(*builder, "[[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };\n\n");
    }

    auto_or_type := ifx lang == "c" then type_name else "auto";
    suffix       := ifx type_name == "float" then "f" else ""; // only "float" gets literal f-suffixes; "double" etc. get bare literals

    // DIODE/DIODE_PAIR/BJT/JFET I-V-law expressions (built in
    // nonlinear_solve.jai) and the AMP KCL branch (nodal_analysis.jai)
    // reference their Params fields as bare symbol names (e.g. "D1_Is",
    // "Eop_Aol"), so alias each one to a local const here.
    emitted_bare_aliases: [..] string;
    emit_bare_alias :: (pname: string) #expand {
        if pname.count == 0  return; // e.g. BJT-only fields left blank on DIODE/DIODE_PAIR elements
        if array_find(emitted_bare_aliases, pname)  return;
        array_add(*emitted_bare_aliases, pname);
        append(*builder, tprint("const %1 %2 = params.%2;\n", auto_or_type, pname));
    }

    for el : elements {
        for pname : device_model_param_names(el.type, el.nonlinear)  emit_bare_alias(pname);
        if el.type == .AMP {
            if el.opamp.has_aol    emit_bare_alias(el.opamp.aol_name);
            if opamp_has_clip_stage(el) {
                emit_bare_alias(el.opamp.vsat_p_name);
                emit_bare_alias(el.opamp.vsat_n_name);
            }
            if el.opamp.has_slew   emit_bare_alias(el.opamp.slew_name);
            if el.opamp.has_vos    emit_bare_alias(el.opamp.vos_name);
            if el.opamp.has_ios    emit_bare_alias(el.opamp.ios_name);
            if el.opamp.has_ibias  emit_bare_alias(el.opamp.ibias_name);
        }
    }
    if emitted_bare_aliases.count > 0  append(*builder, "\n");

    for el : elements {
        // For same-type combined elements (R+R, C+C, R||R, C||C), emit:
        //   const {auto_or_type} {name} = {combo_expr};
        // and then use {name} (not params.{name}) in the impedance formula.
        if el.combo_expr != null && (el.type == .RESISTOR || el.type == .CAPACITOR || el.type == .INDUCTOR) {
            append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, el.name, print_param_expr(el.combo_expr)));
        }

        val_name := ifx el.combo_expr != null then el.name else tprint("params.%", el.name);

        if el.type == {
            case .RESISTOR;
                append(*builder, tprint("const %1 g%2 = 1.0%4 / %3;\n\n", auto_or_type, el.name, val_name, suffix));
            case .CAPACITOR;
                append(*builder, tprint("const %1 g%2 = 2.0%4 * sample_rate * %3;\n\n", auto_or_type, el.name, val_name, suffix));
            case .INDUCTOR;
                append(*builder, tprint("const %1 g%2 = 1.0%4 / (2.0%4 * sample_rate * %3);\n\n", auto_or_type, el.name, val_name, suffix));
            case .RC_SERIES;
                res_name := el.rc_internal.res_name;
                cap_name := el.rc_internal.cap_name;
                append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, res_name, print_param_expr(el.rc_internal.res_expr)));
                append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, cap_name, print_param_expr(el.rc_internal.cap_expr)));
                append(*builder, tprint("const %1 g%2 = (2.0%5 * sample_rate * %3) / (1.0%5 + 2.0%5 * sample_rate * %4 * %3);\n",   auto_or_type, el.name, cap_name, res_name, suffix));
                append(*builder, tprint("const %1 gn%2 = g%2 * (2.0%5 * sample_rate * %3 * %4 - 1.0%5) / (2.0%5 * sample_rate * %4);\n\n", auto_or_type, el.name, res_name, cap_name, suffix));
            case .RC_PARALLEL;
                res_name := el.rc_internal.res_name;
                cap_name := el.rc_internal.cap_name;
                append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, res_name, print_param_expr(el.rc_internal.res_expr)));
                append(*builder, tprint("const %1 %2 = %3;\n", auto_or_type, cap_name, print_param_expr(el.rc_internal.cap_expr)));
                append(*builder, tprint("const %1 g%2 = 2.0%5 * sample_rate * %3 + (1.0%5 / %4);\n", auto_or_type, el.name, cap_name, res_name, suffix));
                append(*builder, tprint("const %1 gz%2 = 4.0%4 * sample_rate * %3;\n\n", auto_or_type, el.name, cap_name, suffix));
            case .FIXED_VOLTAGE;
                // Define the parameter value as a named constant, then alias the
                // (possibly renamed) node to it.  Both names may appear in generated
                // expressions: the param name comes from the equation solve RHS, while
                // the node name comes from element's terminal references.
                param_name := el.name;
                node_name  := el.terminals[0];
                append(*builder, tprint("const %1 %2 = params.%2;\n\n", auto_or_type, param_name));
                if node_name != param_name {
                    append(*builder, tprint("const %1 %2 = %3;\n\n", auto_or_type, node_name, param_name));
                }
        }
    }

    return builder_to_string(*builder);
}

// Junction-voltage step limiter (like ngspice's DEVpnjlim): caps the
// per-iteration step once v exceeds vcrit, so exp(v/vt) can't overflow.
// Emitted once per header. Mirrored on the negative side too: a plain DIODE
// never needs it (reverse current just saturates at -Is), but DIODE_PAIR
// shares this limiter and its I-V law is a symmetric sinh, blowing up on
// either side -- an unclamped negative overstep hits the same runaway.
generate_junction_limiter_helper :: (type_name: string, lang: string) -> string {
    if lang == "c" {
        template := #string DONE
static % limit_junction_voltage(% v_new, % v_old, % vt, % vcrit) {
    if (v_new > vcrit && fabs(v_new - v_old) > 2 * vt) {
        if (v_old > 0) {
            % arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * log(arg) : vcrit;
        } else {
            v_new = vt * log(v_new / vt);
        }
    } else if (v_new < -vcrit && fabs(v_new - v_old) > 2 * vt) {
        if (v_old < 0) {
            % arg = 1 + (v_old - v_new) / vt;
            v_new = arg > 0 ? v_old - vt * log(arg) : -vcrit;
        } else {
            v_new = -vt * log(-v_new / vt);
        }
    }
    return v_new;
}
DONE
        return tprint(template, type_name, type_name, type_name, type_name, type_name, type_name, type_name);
    }

    return #string DONE
[[maybe_unused]] static auto limit_junction_voltage = [](auto v_new, auto v_old, auto vt, auto vcrit)
{
    if (v_new > vcrit && std::abs(v_new - v_old) > 2 * vt)
    {
        if (v_old > 0)
        {
            const auto arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * std::log(arg) : vcrit;
        }
        else
        {
            v_new = vt * std::log(v_new / vt);
        }
    }
    else if (v_new < -vcrit && std::abs(v_new - v_old) > 2 * vt)
    {
        if (v_old < 0)
        {
            const auto arg = 1 + (v_old - v_new) / vt;
            v_new = arg > 0 ? v_old - vt * std::log(arg) : -vcrit;
        }
        else
        {
            v_new = -vt * std::log(-v_new / vt);
        }
    }
    return v_new;
};
DONE;
}

// JFET vGS step limiter: clamps Newton steps at pinch-off (vGS < Vp), where
// the square law curves back up instead of cutting off like a real JFET.
// No upper clamp -- the quadratic stays valid (if approximate) above vGS=0.
generate_jfet_limiter_helper :: (type_name: string, lang: string) -> string {
    if lang == "c" {
        template := #string DONE
static % limit_jfet_vgs(% v_new, % vp) {
    if (v_new < vp) return vp;
    return v_new;
}
DONE
        return tprint(template, type_name, type_name, type_name);
    }

    return #string DONE
[[maybe_unused]] static auto limit_jfet_vgs = [](auto v_new, auto vp)
{
    if (v_new < vp) return vp;
    return v_new;
};
DONE;
}

// Hard rail clamp applied as a post-step limiter - the Newton
// residual itself is just the plain linear VCVS equation. Slew (v_prev/
// slew_max_step) clamps first, then the rails; unknowns with no slew set
// pass a permissive slew_max_step so that branch is a no-op.
generate_opamp_clip_limiter_helper :: (type_name: string, lang: string) -> string {
    if lang == "c" {
        template := #string DONE
static % clamp_opamp_output(% v_new, % v_prev, % slew_max_step, % vsat_n, % vsat_p) {
    if (v_new - v_prev > slew_max_step)  v_new = v_prev + slew_max_step;
    if (v_new - v_prev < -slew_max_step) v_new = v_prev - slew_max_step;
    if (v_new > vsat_p) return vsat_p;
    if (v_new < vsat_n) return vsat_n;
    return v_new;
}
DONE
        return tprint(template, type_name, type_name, type_name, type_name, type_name, type_name);
    }

    return #string DONE
[[maybe_unused]] static auto clamp_opamp_output = [](auto v_new, auto v_prev, auto slew_max_step, auto vsat_n, auto vsat_p)
{
    if (v_new - v_prev > slew_max_step)  v_new = v_prev + slew_max_step;
    if (v_new - v_prev < -slew_max_step) v_new = v_prev - slew_max_step;
    if (v_new > vsat_p) return vsat_p;
    if (v_new < vsat_n) return vsat_n;
    return v_new;
};
DONE;
}

// vcrit = vt * log(vt / (sqrt(2) * Is)) per junction-voltage unknown, derived
// once from Params fields.
generate_vcrit_defs :: (clusters: [] Nonlinear_Cluster, type_name: string, lang: string) -> string {
    auto_or_type := ifx lang == "c" then type_name else "auto";
    log_fn       := ifx lang == "c" then "log" else "std::log";
    sqrt_fn      := ifx lang == "c" then "sqrt" else "std::sqrt";

    builder: String_Builder;
    emitted: [..] string;
    for c : clusters {
        for u : c.unknowns {
            if u.limiter_kind != .EXP_JUNCTION  continue;
            if array_find(emitted, u.vt_name)  continue;
            array_add(*emitted, u.vt_name);
            print_to_builder(*builder, "const %1 vcrit_%2 = %2 * %3(%2 / (%4(2.0) * %5));\n",
                             auto_or_type, u.vt_name, log_fn, sqrt_fn, u.is_name);
        }
    }
    if emitted.count > 0  append(*builder, "\n");
    return builder_to_string(*builder);
}

// Newton-Raphson convergence tolerance (volts^2) and max iteration count,
// defined once and referenced by every cluster's solve below.
generate_newton_constants :: (newton_tol_sq: float, newton_max_iter: int, type_name: string, lang: string) -> string {
    decl_type := ifx lang == "c" then type_name else "auto";
    const_kw  := ifx lang == "c" then "static const" else "static constexpr";
    builder: String_Builder;
    print_to_builder(*builder, "%1 %2 newton_tol_sq = %3;\n", const_kw, decl_type, newton_tol_sq);
    print_to_builder(*builder, "%1 int newton_max_iter = %2;\n\n", const_kw, newton_max_iter);
    return builder_to_string(*builder);
}

// Newton-Raphson solve for one cluster, to be inserted inside the per-sample
// loop. Iteration count/tolerance come from the shared `newton_max_iter` /
// `newton_tol_sq` constants (see generate_newton_constants), not literals.
// `apply_slew`: false for reset()'s DC solve -- Newton iterations aren't
// real time steps, so slew-limiting them would corrupt convergence.
generate_cluster_newton_code :: (cluster: Nonlinear_Cluster, exprs: [] Expr, options: Codegen_Options, apply_slew := true) -> string {
    builder: String_Builder;
    print_to_builder(*builder, "// --- Newton-Raphson solve: %\n", cluster.name);

    emit_topological_order(cluster.invariant_items, exprs, *builder, options);

    append(*builder, "for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)\n{\n");
    inner_builder: String_Builder;
    emit_topological_order(cluster.iteration_items, exprs, *inner_builder, options);

    decl_type := ifx options.lang == "c" then options.type_name else "auto";
    print_to_builder(*inner_builder, "\n%1 residual_norm_sq = 0.0;\n", decl_type);
    for u : cluster.unknowns  print_to_builder(*inner_builder, "residual_norm_sq += %1 * %1;\n", u.residual_name);
    print_to_builder(*inner_builder, "%1 step_norm_sq = 0.0;\n", decl_type);
    for u : cluster.unknowns  print_to_builder(*inner_builder, "step_norm_sq += %1 * %1;\n", u.delta_name);
    append(*inner_builder, "\n");
    for u : cluster.unknowns {
        if u.limiter_kind == {
            case .EXP_JUNCTION;
                print_to_builder(*inner_builder, "%1 = limit_junction_voltage(%1 + %2, %1, %3, vcrit_%3);\n",
                                 u.name, u.delta_name, u.vt_name);
            case .JFET_PINCHOFF;
                print_to_builder(*inner_builder, "%1 = limit_jfet_vgs(%1 + %2, %3);\n",
                                 u.name, u.delta_name, u.vp_name);
            case .AMP_CLIP;
                // This residual is plain linear (f = closed_form - vclip),
                // so its Jacobian can simplify to the bare constant -1.
                //
                // No slew set: pass a permissive slew_max_step (never trips)
                // and reuse `v_new` as `v_prev`.
                v_prev := ifx u.has_slew && apply_slew then u.zslew_name else "_v";
                slew_max_step := ifx u.has_slew && apply_slew then tprint("(%/sample_rate)", u.slew_name) else "1.0e30";
                print_to_builder(*inner_builder, "const auto _v = (%5)(%1 + %2);\n%1 = clamp_opamp_output(_v, %6, %7, %3, %4);\n",
                                 u.name, u.delta_name, u.vsat_n_name, u.vsat_p_name, options.type_name, v_prev, slew_max_step);
            case .NONE;
                print_to_builder(*inner_builder, "%1 = %1 + %2;\n", u.name, u.delta_name);
        }
    }
    append(*inner_builder, "\nif (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)\n    break;\n");

    inner_text := builder_to_string(*inner_builder);
    indented := String.replace(inner_text, "\n", "\n    ");
    indented, _ = String.replace(indented, "\n    \n", "\n\n"); // don't indent blank lines
    append(*builder, "    ");
    append(*builder, indented);
    append(*builder, "\n}\n");

    return builder_to_string(*builder);
}

// Builds and solves the DC-equivalent circuit (see generate_kcl_equations
// dc_mode), and derives the steady-state z-value for each reactive element.
build_dc_operating_point :: (elements: [] Element, inputs: [] string) -> (clusters: [] Nonlinear_Cluster, exprs: [..] Expr, z_init_vars: [] Gen_Var) {
    dc_solve_vars, dc_equations, dc_exprs, dc_elements := generate_kcl_equations(elements, inputs, true);
    dc_solutions := solve(dc_equations, dc_solve_vars, *dc_exprs);

    dc_clusters := build_nonlinear_clusters(elements, dc_solve_vars, dc_solutions, *dc_exprs);

    z_init_vars: [..] Gen_Var;
    for el : dc_elements {
        if el.type != .CAPACITOR && el.type != .RC_SERIES && el.type != .RC_PARALLEL  continue;

        // copy_expr: pos_v/neg_v can alias a solution node shared by another
        // reactive element on the same terminal -- simplify() below mutates
        // in place, so a bare lookup would corrupt it for a later iteration.
        pos_v := copy_expr(lookup_node_voltage(el.terminals[0], dc_solve_vars, dc_solutions, *dc_exprs, inputs), *dc_exprs);
        neg_v := copy_expr(lookup_node_voltage(el.terminals[1], dc_solve_vars, dc_solutions, *dc_exprs, inputs), *dc_exprs);

        z_expr: Expr_ID;
        if el.type == .RC_PARALLEL {
            // z tracks only the capacitor half; gz is already 2x that conductance.
            gz_sym := expr_symbol(tprint("gz%", el.name), *dc_exprs);
            z_expr = expr_div(expr_mul(gz_sym, expr_sub(pos_v, neg_v, *dc_exprs), *dc_exprs), expr_const(2, *dc_exprs), *dc_exprs);
        } else {
            g_sym := expr_symbol(tprint("g%", el.name), *dc_exprs);
            z_expr = expr_mul(g_sym, expr_sub(pos_v, neg_v, *dc_exprs), *dc_exprs);
        }
        z_expr = simplify(z_expr, *dc_exprs);
        array_add(*z_init_vars, Gen_Var.{ name = tprint("z%", el.name), expr_id = z_expr });
    }

    return dc_clusters, dc_exprs, z_init_vars;
}

// Emits a `reset()` function that solves the DC operating point analytically
// and writes it into every channel's State. Each input gets its own
// `<input>_dc` parameter (the value it's held at for DC analysis) -- solved
// once and copied to all `num_channels` states.
generate_reset_function :: (elements: [] Element, inputs: [] string, lang: string, type_name: string) -> string {
    indent :: (in: string, prefix := "    ") -> string {
        if in.count == 0  return in;
        ends_with_newline := in[in.count - 1] == #char "\n";
        out := String.replace(in, "\n", tprint("\n%", prefix));
        out = tprint("%1%2", prefix, out);
        if ends_with_newline  out.count -= prefix.count;
        return out;
    }

    dc_clusters, dc_exprs, z_init_vars := build_dc_operating_point(elements, inputs);
    decl_type := ifx lang == "c" then type_name else "auto";

    builder: String_Builder;
    append(*builder, "static void reset (Params params, State* state, int num_channels, float sample_rate");
    for input : inputs {
        if lang == "c"  print_to_builder(*builder, ", float %1_dc", input);
        else            print_to_builder(*builder, ", float %1_dc = 0.0f", input);
    }
    append(*builder, ")\n{\n");

    append(*builder, indent(generate_impedance_defs(elements, type_name, lang)));
    if dc_clusters.count > 0  append(*builder, indent(generate_vcrit_defs(dc_clusters, type_name, lang)));
    for input : inputs  print_to_builder(*builder, "    const %1 %2 = %2_dc;\n", decl_type, input);
    if inputs.count > 0  append(*builder, "\n");

    // Newton unknowns start at 0 (cold-start guess). No zslew_<name> local
    // needed here since the DC solve runs with apply_slew=false.
    for c : dc_clusters  for u : c.unknowns  print_to_builder(*builder, "    %1 %2 = 0;\n", type_name, u.name);
    if dc_clusters.count > 0  append(*builder, "\n");

    for c : dc_clusters {
        newton_code := generate_cluster_newton_code(c, dc_exprs, .{ lang = lang, type_name = type_name }, apply_slew = false);
        append(*builder, indent(newton_code));
    }

    for zv : z_init_vars {
        print_to_builder(*builder, "    const %1 %2 = %3;\n", decl_type, zv.name, print_expr(zv.expr_id, dc_exprs));
    }
    if z_init_vars.count > 0  append(*builder, "\n");

    append(*builder, "    for (int ch = 0; ch < num_channels; ++ch)\n    {\n");
    for c : dc_clusters {
        for u : c.unknowns {
            print_to_builder(*builder, "        state[ch].%1 = %1;\n", u.name);
            // zslew_<name> starts at the same DC-settled value.
            if u.has_slew  print_to_builder(*builder, "        state[ch].%1 = %2;\n", u.zslew_name, u.name);
        }
    }
    for zv : z_init_vars  print_to_builder(*builder, "        state[ch].%1 = %1;\n", zv.name);
    append(*builder, "    }\n");

    append(*builder, "}\n");
    return builder_to_string(*builder);
}

netlist_codegen :: (netlist_str: string, type_name: string = "float", lang: string = "cpp", namespace: string = "") -> string {
    indent :: (in: string, prefix := "    ") -> string {
        if in.count == 0  return in;
        ends_with_newline := in[in.count - 1] == #char "\n";
        out := String.replace(in, "\n", tprint("\n%", prefix));
        out = tprint("%1%2", prefix, out);
        if ends_with_newline  out.count -= prefix.count;
        return out;
    }

    net := parse_netlist(netlist_str);
    // print("%\n", net.elements);
    // print("%\n", net.inputs);
    // print("%\n", net.values);

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

    if has_nonlinear {
        // Same as solve_and_codegen(), but with build_nonlinear_clusters()
        // spliced in between solve() and generate_expressions() -- see
        // nonlinear_solve.jai for why the ordering matters.
        reduce_circuit(*net.elements, net.inputs, net.outputs);
        solve_vars, equations, exprs, kcl_elements := generate_kcl_equations(net.elements, net.inputs);
        solutions := solve(equations, solve_vars, *exprs);

        clusters = build_nonlinear_clusters(kcl_elements, solve_vars, solutions, *exprs);

        // Junction-voltage unknowns (vD1, vBCQ1, ...) only appear in the tree
        // *after* nonlinear_solve.jai substitutes them in for branch-current
        // symbols.
        cluster_variant_names: [..] string;
        for c : clusters  for u : c.unknowns  array_add(*cluster_variant_names, u.name);

        gen_vars := generate_expressions(kcl_elements, net.outputs, solve_vars, solutions, *exprs);

        gen_var_ids: [..] Expr_ID;
        for gv : gen_vars  array_add(*gen_var_ids, gv.expr_id);

        cse_vars := cse(gen_var_ids, *exprs);
        sort_cse_topological(*cse_vars, exprs);

        licm_variant_base := build_variant_base(kcl_elements, net.inputs);
        for gv : gen_vars  array_add(*licm_variant_base, gv.name);
        for n : cluster_variant_names  array_add(*licm_variant_base, n);
        licm(gen_var_ids, *cse_vars, *exprs, licm_variant_base);
        recip_hoist(gen_var_ids, *cse_vars, *exprs, licm_variant_base);

        outer_code, loop_code, outer_flops, inner_flops =
            codegen(kcl_elements, net.inputs, gen_vars, cse_vars, exprs, .{ lang = lang, type_name = type_name }, cluster_variant_names);
        cluster_exprs = exprs;
    } else {
        outer_code, loop_code, outer_flops, inner_flops =
            solve_and_codegen(*net.elements, net.inputs, net.outputs, .{ lang = lang, type_name = type_name });
    }
    // print("%\n", outer_code);
    // print("%\n", loop_code);

    flop_str :: (f: Flop_Count) -> string #expand {
        b: String_Builder;
        sep := "";
        if f.muls > 0 { append(*b, sep); print_to_builder(*b, "% mul", f.muls); sep = ", "; }
        if f.divs > 0 { append(*b, sep); print_to_builder(*b, "% div", f.divs); sep = ", "; }
        if f.adds > 0 { append(*b, sep); print_to_builder(*b, "% add", f.adds); sep = ", "; }
        total := f.muls + f.divs + f.adds;
        if total == 0  return "0 ops";
        return tprint("% ops (%)", total, builder_to_string(*b));
    }
    log("  Outer loop: %\n  Inner loop: %\n", flop_str(outer_flops), flop_str(inner_flops));

    decl_type := ifx lang == "c" then type_name else "auto";

    builder: String_Builder;
    append(*builder, "#pragma once\n\n");
    if has_nonlinear {
        append(*builder, ifx lang == "c" then "#include <math.h>\n\n" else "#include <cmath>\n\n");
    }

    if namespace.count > 0 && lang == "cpp" {
        append(*builder, tprint("namespace %\n{\n\n", namespace));
    }

    params_struct_code := generate_params_struct(net.elements, net.values, type_name, lang);
    append(*builder, params_struct_code);
    append(*builder, "\n");

    extra_state_names: [..] string;
    for c : clusters  for u : c.unknowns  array_add(*extra_state_names, u.name);
    state_struct_code, states := generate_state_struct(net.elements, extra_state_names, type_name, lang);
    append(*builder, state_struct_code);
    append(*builder, "\n");

    if has_nonlinear {
        if circuit_uses_limiter_kind(net.elements, .EXP_JUNCTION) {
            append(*builder, generate_junction_limiter_helper(type_name, lang));
            append(*builder, "\n");
        }
        if circuit_uses_limiter_kind(net.elements, .JFET_PINCHOFF) {
            append(*builder, generate_jfet_limiter_helper(type_name, lang));
            append(*builder, "\n");
        }
        if circuit_uses_limiter_kind(net.elements, .AMP_CLIP) {
            append(*builder, generate_opamp_clip_limiter_helper(type_name, lang));
            append(*builder, "\n");
        }
        append(*builder, generate_newton_constants(net.newton_tol_sq, net.newton_max_iter, type_name, lang));
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

    append(*builder, indent(generate_impedance_defs(net.elements, type_name, lang)));
    if has_nonlinear  append(*builder, indent(generate_vcrit_defs(clusters, type_name, lang)));
    if outer_code.count > 0  append(*builder, indent(outer_code));

    append(*builder, "    for (int ch = 0; ch < num_channels; ++ch)\n");
    append(*builder, "    {\n");
    for state : states  append(*builder, tprint("        %1 %2 = state[ch].%2;\n", decl_type, state));

    append(*builder, "        for (int n = 0; n < num_samples; ++n)\n");
    append(*builder, "        {\n");

    if net.inputs.count == 1 {
        append(*builder, tprint("            const %1 %2 = input[ch][n];\n\n", decl_type, net.inputs[0]));
    } else {
        for input : net.inputs {
            append(*builder, tprint("            const %1 %2 = input_%2[ch][n];\n", decl_type, input));
        }
        append(*builder, "\n");
    }

    if has_nonlinear {
        for c : clusters {
            newton_code := generate_cluster_newton_code(c, cluster_exprs, .{ lang = lang, type_name = type_name });
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
        append(*builder, generate_reset_function(net.elements, net.inputs, lang, type_name));
    }

    if namespace.count > 0 && lang == "cpp" {
        append(*builder, tprint("\n} // namespace %\n", namespace));
    }

    return builder_to_string(*builder);
}
