#import "Basic";
File :: #import "File";

#import "netlist_codegen";

main :: () {
    args := get_command_line_arguments();
    netlist_file := args[1];
    header_file := args[2];
    type_name := ifx args.count > 3 then args[3] else "float";
    print("Converting netlist file % to % (type: %)\n", netlist_file, header_file, type_name);

    netlist_file_str, fread_success := File.read_entire_file(netlist_file);
    assert(fread_success, "Unable to open file!");

    gen_code := netlist_codegen(netlist_file_str, type_name);

    out_file, success := File.file_open(header_file, true);
    assert(success, "Unable to open output file!");
    File.file_write(*out_file, gen_code);
}
