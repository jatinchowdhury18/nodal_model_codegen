// Builds netlist_codegen_c_api.jai as a static library (netlist_codegen_lib.a)
// for embedding in a host C/C++ program. Run from this directory:
//   jai build_lib.jai
#import "Compiler";
#import "Basic";

#run build();

build :: () {
    set_build_options_dc(.{do_output = false});

    w := compiler_create_workspace("netlist_codegen_lib");
    options := get_build_options(w);
    options.output_executable_name = "netlist_codegen_lib";
    options.output_type = .STATIC_LIBRARY;
    options.text_output_flags = .OUTPUT_LINK_LINE;

    import_path: [..] string;
    array_add(*import_path, ..options.import_path);
    array_add(*import_path, "modules");
    options.import_path = import_path;

    set_optimization(*options, .OPTIMIZED);
    set_build_options(options, w);
    add_build_file("netlist_codegen_c_api.jai", w);
}
