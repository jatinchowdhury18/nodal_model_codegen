#import "Compiler";
#import "Basic";

#run build();

build :: () {
    set_build_options_dc(.{do_output = false});

    build_target("solve_test.exe",         "solve_test.jai");
    build_target("simplify_test.exe",      "simplify_test.jai");
    build_target("circuit_test.exe",       "circuit_test.jai");
    build_target("netlist_test.exe",       "netlist_test.jai");
    build_target("differentiate_test.exe", "differentiate_test.jai");
    build_target("nonlinear_test.exe",     "nonlinear_test.jai");
}

build_target :: (name: string, source: string) {
    w := compiler_create_workspace(name);

    options := get_build_options(w);
    options.output_executable_name = name;

    import_path: [..] string;
    array_add(*import_path, ..options.import_path);
    array_add(*import_path, "../modules");
    options.import_path = import_path;

    set_build_options(options, w);
    add_build_file(source, w);
}
