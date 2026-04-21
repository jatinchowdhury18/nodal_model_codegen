#import "Basic";
File :: #import "File";
Command_Line :: #import "Command_Line";

#import "netlist_codegen";

Args :: struct {
    lang:      string = "cpp";  // Output language: "cpp" or "c"
    type_name: string = "float"; // Numeric type: "float" or "double"
}

main :: () {
    success, args, is_set, free_args := Command_Line.parse_arguments(Args);
    if !success  exit(1);

    if free_args.count < 2 {
        print("Usage: netlist_codegen [options] <netlist_file> <header_file>\n");
        exit(1);
    }

    netlist_file := free_args[0];
    header_file  := free_args[1];

    print("Converting netlist file % to % (lang: %, type: %)\n",
          netlist_file, header_file, args.lang, args.type_name);

    netlist_file_str, fread_success := File.read_entire_file(netlist_file);
    assert(fread_success, "Unable to open file!");

    gen_code := netlist_codegen(netlist_file_str, args.type_name, args.lang);

    out_file, open_success := File.file_open(header_file, for_writing=true, keep_existing_content=false);
    assert(open_success, "Unable to open output file!");
    File.file_write(*out_file, gen_code);
    File.file_close(*out_file);
}