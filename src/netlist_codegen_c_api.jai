// C ABI wrapper around netlist_codegen(), for static-linking this tool
// directly into a host C/C++ program instead of shelling out to the CLI.
//
// Usage from the host:
//   1. Call netlist_codegen_init() once (per process) to get an opaque
//      Jai context pointer; keep it around.
//   2. For each netlist, call netlist_codegen_generate() with that context
//      pointer plus the netlist text (and options) as (pointer, length)
//      pairs, since Jai strings aren't null-terminated and lengths are
//      required both in and out.
//   3. Free the returned buffer with netlist_codegen_free_string() once done
//      with it.
#import "Basic";
#import "netlist_codegen";

// Runtime_Support's __jai_runtime_init/__jai_runtime_fini are already
// #program_export'd by the compiler (see Runtime_Support.jai) whenever
// DEFINE_INITIALIZATION is on, which it is for STATIC_LIBRARY output. They
// aren't declared here; the host just needs to know their C signatures (see
// netlist_codegen_c_api.h).

to_jai_string :: (data: *u8, len: s64) -> string {
    s: string = ---;
    s.data = data;
    s.count = len;
    return s;
}

#program_export
netlist_codegen_generate :: (
    jai_context: *void,
    netlist_text: *u8, netlist_len: s64,
    type_name: *u8, type_name_len: s64,
    lang: *u8, lang_len: s64,
    namespace_text: *u8, namespace_len: s64,
    out_code: **u8, out_code_len: *s64
) -> bool #c_call {
    ctx := (cast(*#Context) jai_context).*;
    push_context ctx {
        result := netlist_codegen(
            to_jai_string(netlist_text, netlist_len),
            to_jai_string(type_name, type_name_len),
            to_jai_string(lang, lang_len),
            to_jai_string(namespace_text, namespace_len));

        out_code.* = result.data;
        out_code_len.* = result.count;
        return true;
    }
}

#program_export
netlist_codegen_free_string :: (jai_context: *void, ptr: *u8, len: s64) #c_call {
    ctx := (cast(*#Context) jai_context).*;
    push_context ctx {
        free(to_jai_string(ptr, len));
    }
}
