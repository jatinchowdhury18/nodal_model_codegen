#import "Basic";
File    :: #import "File";
Command_Line :: #import "Command_Line";
Process :: #import "Process";

#import "netlist_codegen";

Args :: struct {
    lang:      string = "cpp";   @"?Output language for generated code. Supports: cpp (default), c."
    type_name: string = "float"; @"?Numeric type to use for generated code. (Default: float)"
    namespace: string = "";      @"?Wrap generated code in a namespace (C++ only, disabled by default)."
}

argument_flags :: Command_Line.Default_Argument_Flags;

get_codegen_version :: () -> string {
    _, output, _, _ := Process.run_command("git", "rev-parse", "--short", "HEAD", capture_and_return_output=true);
    if output.count == 0  return "unknown";
    while output.count > 0 && (output[output.count-1] == #char "\n" || output[output.count-1] == #char "\r") {
        output.count -= 1;
    }
    return copy_string(output);
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

    netlist_file := free_args[0];
    header_file  := free_args[1];

    print("Converting netlist file % to % (lang: %, type: %, namespace: '%')\n",
          netlist_file, header_file, args.lang, args.type_name, args.namespace);

    netlist_file_str, fread_success := File.read_entire_file(netlist_file);
    assert(fread_success, "Unable to open file!");

    gen_code := netlist_codegen(netlist_file_str, args.type_name, args.lang, args.namespace, netlist_file);

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