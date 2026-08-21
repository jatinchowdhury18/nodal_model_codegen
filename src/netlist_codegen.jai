#import "Basic";
Command_Line :: #import "Command_Line";
Process :: #import "Process";

#import "netlist_codegen";
#import "backend"; // File, resolve_cpu_profile, CPU_Profile

Args :: struct {
    lang:         string = "cpp";     @"?Output language for generated code. Supports: cpp (default), c."
    type_name:    string = "float";   @"?Numeric type to use for generated code. (Default: float)"
    namespace:    string = "";        @"?Wrap generated code in a namespace (C++ only, disabled by default)."
    cpu:          string = "generic"; @"?Target CPU profile for the extraction cost model. Built-in: generic (default), m1, zen4, cortex_a8. Or a path to a custom cpu_profiles/*.txt file."
    opt_extract:  bool   = true;      @"?Enable basis-vector linear coefficient extraction (enabled by default)."
    stats:        bool   = false;     @"?Print netlist_codegen()'s wall time and bytes-allocated to stderr after codegen finishes."
}

Alloc_Stats :: struct {
    underlying:      Allocator;
    bytes_allocated: s64;
    alloc_count:     s64;
}

counting_allocator_proc :: (mode: Allocator_Mode, size: s64, old_size: s64, old_memory: *void, allocator_data: *void) -> *void {
    stats := cast(*Alloc_Stats) allocator_data;
    if mode == {
        case .ALLOCATE;
            stats.bytes_allocated += size;
            stats.alloc_count += 1;
        case .RESIZE;
            if size > old_size  stats.bytes_allocated += size - old_size;
            stats.alloc_count += 1;
    }
    return stats.underlying.proc(mode, size, old_size, old_memory, stats.underlying.data);
}

argument_flags :: Command_Line.Default_Argument_Flags;

get_codegen_version :: () -> string {
    _, output, _, _ := Process.run_command("git", "rev-parse", "--short", "HEAD", capture_and_return_output=true);
    if output.count == 0  return "unknown";
    return copy_string(String.trim_right(output, "\n\r"));
}

CODEGEN_VERSION :: #run get_codegen_version();

main :: () {
    success, args, is_set, free_args := Command_Line.parse_arguments(Args, argument_flags);
    if !success  exit(1);

    if args.lang != "cpp" && args.lang != "c" {
        print("Unknown language '%'! Supported languages are: cpp, c\n", args.lang);
        Command_Line.show_help(Args, argument_flags, .[]);
        exit(1);
    }

    if free_args.count < 2 {
        print("Usage: netlist_codegen <netlist_file> <header_file> [options]\n");
        Command_Line.show_help(Args, argument_flags, .[]);
        exit(1);
    }

    cpu_profile, cpu_found := resolve_cpu_profile(args.cpu);
    if !cpu_found {
        print("Unknown CPU profile '%' (not a built-in name, and not a readable file path)!\n", args.cpu);
        exit(1);
    }

    netlist_file := free_args[0];
    header_file  := free_args[1];

    print("Converting netlist file % to % (lang: %, type: %, namespace: '%', cpu: '%')\n",
          netlist_file, header_file, args.lang, args.type_name, args.namespace, cpu_profile.name);

    netlist_file_str, fread_success := File.read_entire_file(netlist_file);
    assert(fread_success, "Unable to open file!");

    stats: Alloc_Stats;
    stats.underlying = context.allocator;
    context.allocator = Allocator.{counting_allocator_proc, *stats};
    temp_before := context.temporary_storage.total_bytes_occupied;
    start_time := seconds_since_init();

    gen_code := netlist_codegen(netlist_file_str, args.type_name, args.lang, args.namespace, netlist_file, cpu_profile, args.opt_extract);

    elapsed_seconds := seconds_since_init() - start_time;
    context.allocator = stats.underlying;

    if args.stats {
        temp_after := context.temporary_storage.total_bytes_occupied;
        print("netlist_codegen() stats: % ms, % MB allocated (% allocations), % MB temp storage\n",
              formatFloat(elapsed_seconds * 1000, trailing_width = 3, zero_removal = .NO),
              formatFloat(cast(float64) stats.bytes_allocated / (1024 * 1024), trailing_width = 2, zero_removal = .NO),
              stats.alloc_count,
              formatFloat(cast(float64) (temp_after - temp_before) / (1024 * 1024), trailing_width = 2, zero_removal = .NO),
              to_standard_error = true);
    }

    // Prepend auto-generated header comment with version and full CLI invocation.
    cli_args := get_command_line_arguments();
    header: String_Builder;
    print_to_builder(*header, "// Auto-generated with netlist_codegen version %.\n// Command: netlist_codegen", CODEGEN_VERSION);
    for i : 1..cli_args.count - 1  print_to_builder(*header, " %", cli_args[i]);
    append(*header, "\n\n");
    append(*header, gen_code);
    gen_code = builder_to_string(*header);

    out_file, open_success := File.file_open(header_file, for_writing=true, keep_existing_content=false);
    assert(open_success, "Unable to open output file!");
    File.file_write(*out_file, gen_code);
    File.file_close(*out_file);
}
