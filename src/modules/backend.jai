// The foundational types every other codegen module threads through:
// Backend (language-specific emission choices), CPU/ops info, math
// approximation settings, op-count book-keeping, and some pure C/C++
// emission primitives.

#import "Basic";
String :: #import "String";
File   :: #import "File";

// Re-indents a multi-line code block by `prefix` (used when splicing one
// generated block into another, e.g. impedance defs into the outer loop).
indent :: (in: string, prefix := "    ") -> string {
    if in.count == 0  return in;
    builder: String_Builder;
    append(*builder, prefix);
    for i : 0..in.count-1 {
        c := in[i];
        append(*builder, c);
        if c == #char "\n" && i != in.count - 1  append(*builder, prefix);
    }
    return builder_to_string(*builder);
}

// "Global" properties that inform the code generation.
Backend :: struct {
    // language-specifics
    is_cpp:    bool   = true;
    type_name: string = "float";     // numeric type used for variable declarations
    decl_type: string = "auto";      // "auto" (C++, type inference) or type_name (C)
    const_kw:  string = "static constexpr"; // vs "static const" (C)

    // math stuff
    transcendental_approx: Transcendental_Approx; // exp/log/pow approximation settings

    // optimization stuff
    cpu: CPU_Profile = CPU_PROFILE_GENERIC;  // target CPU for scheduling/extraction cost model
    block_size: int = DEFAULT_BLOCK_SIZE; // block length used for the affine extraction cost model
    enable_extraction: bool; // basis-vector linear coefficient extraction
    instrument:        bool; // emit Newton iteration/solve counters on State
    opt_port_matrix:   bool; // N-port (Thevenin) formulation of nonlinear clusters
}

make_backend :: (lang: string, type_name: string, cpu: CPU_Profile = CPU_PROFILE_GENERIC, transcendental_approx: Transcendental_Approx = .{},
                 enable_extraction := false, instrument := false, opt_port_matrix := false, block_size := DEFAULT_BLOCK_SIZE) -> Backend {
    is_cpp := lang != "c";
    return .{
        is_cpp    = is_cpp,
        type_name = type_name,
        decl_type = ifx is_cpp then "auto" else type_name,
        const_kw  = ifx is_cpp then "static constexpr" else "static const",
        cpu       = cpu,
        block_size = block_size,
        transcendental_approx = transcendental_approx,
        enable_extraction = enable_extraction,
        instrument = instrument,
        opt_port_matrix = opt_port_matrix,
    };
}

// Default for Backend.block_size, overridable via `-block_size`.
DEFAULT_BLOCK_SIZE :: 64;

// Rough weighted op-cost estimate for a Flop_Count, using a CPU_Profile's
// per-op latencies. Transcendentals aren't included, but that shouldn't matter.
estimate_flop_cost :: (flops: Flop_Count, cpu: CPU_Profile) -> float {
    return flops.muls * cpu.mul.latency + flops.divs * cpu.div.latency + flops.adds * cpu.add.latency;
}

// Resource-bound companion to estimate_flop_cost: total reciprocal throughput.
//mul/div/add share the same FP ports; memory ops use different ports, so
// the callers `max` them in later.
estimate_flop_resource :: (flops: Flop_Count, cpu: CPU_Profile) -> float {
    return flops.muls * cpu.mul.throughput + flops.divs * cpu.div.throughput + flops.adds * cpu.add.throughput;
}

// exp/log/pow approximation settings, overridable per netlist via
// `* exp_approx(order=N, c1=0/1, exact=0/1)`. Approximation order
// and C1 continuity are borrowed from github.com/Chowdhury-DSP/math_approx.
// `exact` falls back to std::exp/std::log/std::pow.
Transcendental_Approx :: struct {
    order: int = 6;
    c1_continuous: bool = true;
    exact: bool = false;
}

// Opening line for a Params/State struct. C++ names the struct up front;
// a C typedef struct is named on the closing line instead (see struct_close).
struct_open :: (backend: Backend, name: string) -> string {
    return ifx backend.is_cpp then tprint("struct % {\n", name) else "typedef struct {\n";
}

struct_close :: (backend: Backend, name: string) -> string {
    return ifx backend.is_cpp then "};\n" else tprint("} %;\n", name);
}

// A struct field with no initializer. C++ zero-inits with `{}`, C just declares it.
field_decl :: (backend: Backend, type_name: string, name: string) -> string {
    return ifx backend.is_cpp then tprint("    % % {};\n", type_name, name) else tprint("    % %;\n", type_name, name);
}

// A struct field with a literal initial value. C structs can't have default member
// initializers, so the value is recorded as a comment instead.
field_decl_with_value :: (backend: Backend, type_name: string, name: string, value_str: string) -> string {
    if backend.is_cpp  return tprint("    %1 %2 = %3;\n", type_name, name, value_str);
    return tprint("    % %; // = %\n", type_name, name, value_str);
}

// A function parameter with a default value - C doesn't support default
// parameter values, C++ does.
param_decl_with_default :: (backend: Backend, type_name: string, name: string, default_str: string) -> string {
    if backend.is_cpp  return tprint(", %1 %2 = %3", type_name, name, default_str);
    return tprint(", %1 %2", type_name, name);
}

// sum/recip_sum helpers referenced by generate_impedance_defs - C++ lambdas,
// C macros (C has no closures, and these never capture anything anyway).
sum_recip_sum_defs :: (backend: Backend) -> string {
    if backend.is_cpp {
        return "[[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };\n[[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };\n\n";
    }
    return "#define sum(a, b) ((a) + (b))\n#define recip_sum(a, b) ((a) * (b) / ((a) + (b)))\n\n";
}

// Per-op latency/throughput estimate for one CPU target. Numbers are
// abstract "cycles"; throughput is cycles-between-independent-issues
// (lower = more pipelined), not IPC.
Op_Info :: struct {
    latency:    float;
    throughput: float;
}

// Everything the (future) scheduler / linear-extraction cost model needs to
// know about a target CPU. Loaded from a `cpu_profiles/*.txt` file: built-ins
// are baked into the binary at compile time, and `-cpu=<path>` loads a custom
// one at runtime.
CPU_Profile :: struct {
    name: string;
    num_registers: int;
    add:  Op_Info;
    mul:  Op_Info;
    div:  Op_Info;
    load:  Op_Info;
    store: Op_Info;
}

// Parses the cpu_profiles/*.txt text format: `key = value` lines, `#`
// comments, blank lines ignored. Unknown keys are ignored (forward-
// compatible with fields this version doesn't know about yet); keys that
// are never set keep their zero-value default.
parse_cpu_profile :: (text: string, name: string) -> CPU_Profile {
    profile: CPU_Profile;
    profile.name = name;

    set_op :: (op: *Op_Info, field: string, value: float) {
        if      field == "latency"     op.latency = value;
        else if field == "throughput"  op.throughput = value;
    }

    for line : String.split(text, #char "\n") {
        trimmed := String.trim(line);
        if trimmed.count == 0 || trimmed[0] == #char "#"  continue;

        eq_idx := String.find_index_from_left(trimmed, #char "=");
        if eq_idx == -1  continue;
        key       := String.trim(String.slice(trimmed, 0, eq_idx));
        value_str := String.trim(String.slice(trimmed, eq_idx + 1, trimmed.count - eq_idx - 1));

        if key == "num_registers" {
            n, ok := string_to_int(value_str);
            if ok  profile.num_registers = n;
            continue;
        }

        value, ok := string_to_float(value_str);
        if !ok  continue;

        if      String.begins_with(key, "add.")    set_op(*profile.add,   String.slice(key, 4, key.count - 4), value);
        else if String.begins_with(key, "mul.")    set_op(*profile.mul,   String.slice(key, 4, key.count - 4), value);
        else if String.begins_with(key, "div.")    set_op(*profile.div,   String.slice(key, 4, key.count - 4), value);
        else if String.begins_with(key, "load.")   set_op(*profile.load,  String.slice(key, 5, key.count - 5), value);
        else if String.begins_with(key, "store.")  set_op(*profile.store, String.slice(key, 6, key.count - 6), value);
    }

    return profile;
}

// Parses one of the repo's cpu_profiles/*.txt files at compile time.
load_builtin_cpu_profile :: (name: string) -> CPU_Profile {
    dir  := String.path_strip_filename(#filepath);
    path := tprint("%../../cpu_profiles/%.txt", dir, name);
    text, ok := File.read_entire_file(path);
    assert(ok, "Missing built-in CPU profile file: %", path);
    return parse_cpu_profile(text, name);
}

CPU_PROFILE_GENERIC   :: #run load_builtin_cpu_profile("generic");
CPU_PROFILE_M1        :: #run load_builtin_cpu_profile("m1");
CPU_PROFILE_ZEN4      :: #run load_builtin_cpu_profile("zen4");
CPU_PROFILE_CORTEX_A8 :: #run load_builtin_cpu_profile("cortex_a8");

// Resolves a `-cpu` argument to a CPU_Profile.
resolve_cpu_profile :: (name_or_path: string) -> CPU_Profile, bool {
    if name_or_path == {
        case "generic";     return CPU_PROFILE_GENERIC, true;
        case "m1";          return CPU_PROFILE_M1, true;
        case "zen4";        return CPU_PROFILE_ZEN4, true;
        case "cortex_a8";   return CPU_PROFILE_CORTEX_A8, true;
    }

    text, ok := File.read_entire_file(name_or_path);
    if !ok  return .{}, false;
    return parse_cpu_profile(text, name_or_path), true;
}

// Counts of floating-point operations in a block of generated code.
Flop_Count :: struct {
    muls: int;
    divs: int;
    adds: int; // ADD, SUB, and NEG all counted here
    transcendentals: int; // EXP, LOG, POW, only nonzero in `exact` mode: see count_flops_expr() in codegen.jai
}
