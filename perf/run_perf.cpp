// Timing driver for the perf harness. Doesn't run netlist_codegen itself --
// run_perf.py generates perf/generated/<case>.h (one per representative
// circuit, each wrapped in its own C++ namespace via `-namespace <case>`)
// before this gets compiled, then invokes this binary to do the actual
// timed runs + HW counter reads + JSON write. Codegen-time stats (FLOPs,
// temp count, codegen ms) get merged into the same JSON by run_perf.py
// afterward -- this binary only knows about runtime measurement.
//
// One hardcoded Traits struct per case (name, Params/State types, whether
// it has a reset()), a generic collect() warmup+timed loop, and
// event_collector for HW counters.

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "performance_counters/event_counter.h"

#include "generated/rc_lowpass.h"
#include "generated/eq_filter2.h"
#include "generated/diode_clipper.h"
#include "generated/common_emitter.h"
#include "generated/pedal_drive.h"
#include "generated/pedal_model.h"
#include "generated/tube_compressor.h"
#include "generated/eq73.h"

static constexpr int N_WARMUP = 2;
static constexpr int N_TIMED  = 10;
static constexpr int NUM_SAMPLES  = 1 << 16;
static constexpr float SAMPLE_RATE = 48000.0f;

// ── Case traits ──────────────────────────────────────────────────────────
// One per representative circuit.
// vi_dc values match what tests/<case>/test.py already validated against
// LTspice where applicable -- doesn't matter much for pure perf timing, but
// keeps generated-code behavior sane for the nonlinear cases (Newton solve
// warm-starting from a reasonable operating point instead of the origin).

struct RcLowpassTraits {
    static constexpr const char* name = "rc_lowpass";
    using Params = rc_lowpass::Params;
    using State  = rc_lowpass::State;
    static constexpr bool has_reset = false;
    static constexpr auto compute = rc_lowpass::compute;
};

struct EqFilter2Traits {
    static constexpr const char* name = "eq_filter2";
    using Params = eq_filter2::Params;
    using State  = eq_filter2::State;
    static constexpr bool has_reset = false;
    static constexpr auto compute = eq_filter2::compute;
};

struct DiodeClipperTraits {
    static constexpr const char* name = "diode_clipper";
    using Params = diode_clipper::Params;
    using State  = diode_clipper::State;
    static constexpr bool has_reset = true;
    static constexpr float vi_dc = 0.0f;
    static constexpr auto compute = diode_clipper::compute;
    static constexpr auto reset   = diode_clipper::reset;
};

struct CommonEmitterTraits {
    static constexpr const char* name = "common_emitter";
    using Params = common_emitter::Params;
    using State  = common_emitter::State;
    static constexpr bool has_reset = true;
    static constexpr float vi_dc = -0.1f;
    static constexpr auto compute = common_emitter::compute;
    static constexpr auto reset   = common_emitter::reset;
};

struct PedalDriveTraits {
    static constexpr const char* name = "pedal_drive";
    using Params = pedal_drive::Params;
    using State  = pedal_drive::State;
    static constexpr bool has_reset = true;
    static constexpr float vi_dc = 0.0f;
    static constexpr auto compute = pedal_drive::compute;
    static constexpr auto reset   = pedal_drive::reset;
};

struct PedalModelTraits {
    static constexpr const char* name = "pedal_model";
    using Params = pedal_model::Params;
    using State  = pedal_model::State;
    static constexpr bool has_reset = true;
    static constexpr float vi_dc = 0.0f;
    static constexpr auto compute = pedal_model::compute;
    static constexpr auto reset   = pedal_model::reset;
};

// The largest nonlinear cluster in the fixture set (k=5: two triodes plus a
// sidechain diode), so it is the case where per-iteration cost and iteration
// count both matter most.
struct TubeCompressorTraits {
    static constexpr const char* name = "tube_compressor";
    using Params = tube_compressor::Params;
    using State  = tube_compressor::State;
    static constexpr bool has_reset = true;
    static constexpr float vi_dc = 0.0f;
    static constexpr auto compute = tube_compressor::compute;
    static constexpr auto reset   = tube_compressor::reset;
};

struct Eq73Traits {
    static constexpr const char* name = "eq73";
    using Params = eq73::Params;
    using State  = eq73::State;
    static constexpr bool has_reset = true;
    static constexpr float vi_dc = 0.0f;
    static constexpr auto compute = eq73::compute;
    static constexpr auto reset   = eq73::reset;
};

// ── Event collection ────────────────

template <typename Fn>
event_aggregate collect(event_collector& ec, Fn&& fn) {
    event_aggregate agg;
    agg.has_events = ec.has_events();
    for (int i = 0; i < N_WARMUP; ++i) fn();
    for (int i = 0; i < N_TIMED; ++i) {
        ec.start();
        fn();
        agg << ec.end();
    }
    return agg;
}

// ── Result printing ─────────────────────────────────────────────────────

static void print_header(bool has_events) {
    std::cout << "  " << std::left << std::setw(16) << "case"
              << std::right << std::setw(10) << "ns/smp" << std::setw(10) << "Msmp/s";
    if (has_events)
        std::cout << std::setw(12) << "cyc/smp" << std::setw(12) << "ins/smp"
                   << std::setw(8) << "IPC" << std::setw(12) << "L1Dmiss/smp";
    std::cout << '\n';
}

static void print_row(const char* label, const event_aggregate& agg, int N) {
    const double ns_per   = agg.best.elapsed_ns() / N;
    const double msamples = 1e3 / ns_per;
    std::cout << "  " << std::left << std::setw(16) << label
               << std::right << std::fixed
               << std::setw(10) << std::setprecision(2) << ns_per
               << std::setw(10) << std::setprecision(2) << msamples;
    if (agg.has_events)
        std::cout << std::setw(12) << std::setprecision(1) << agg.best.cycles() / N
                   << std::setw(12) << std::setprecision(1) << agg.best.instructions() / N
                   << std::setw(8) << std::setprecision(2) << agg.best.instructions() / agg.best.cycles()
                   << std::setw(12) << std::setprecision(2) << agg.best.l1d_misses() / N;
    std::cout << '\n';
}

// ── JSON output ──────────────────────────────────────────────────────────
// Hand-rolled -- one flat object, no need to pull in a JSON library for
// this. run_perf.py loads it back with Python's json module afterward to
// merge in codegen-time stats (FLOPs, temp count) that only the codegen CLI
// invocation (not this binary) knows about.

// HW counters can be unavailable (e.g. no root on macOS kperf/kpc) -- in
// that case cycles/instructions/etc are all 0, and IPC (a division) would
// come out `nan`, which isn't valid JSON. Emit `null` for anything that
// depends on counters we don't actually have instead.
static void write_json(const std::string& path, const char* case_name,
                        const event_aggregate& agg, int N) {
    const bool hw = agg.has_events;
    std::ofstream f(path);
    f << "{\n";
    f << "  \"circuit\": \"" << case_name << "\",\n";
    f << "  \"num_samples\": " << N << ",\n";
    f << "  \"runtime\": {\n";
    f << "    \"ns_per_sample\": " << (agg.best.elapsed_ns() / N) << ",\n";
    f << "    \"has_hw_counters\": " << (hw ? "true" : "false") << ",\n";
    f << "    \"cycles_per_sample\": "        << (hw ? std::to_string(agg.best.cycles() / N)              : "null") << ",\n";
    f << "    \"instructions_per_sample\": "  << (hw ? std::to_string(agg.best.instructions() / N)        : "null") << ",\n";
    f << "    \"ipc\": "                      << (hw ? std::to_string(agg.best.instructions() / agg.best.cycles()) : "null") << ",\n";
    f << "    \"l1d_load_misses_per_sample\": " << (hw ? std::to_string(agg.best.l1d_load_misses() / N)   : "null") << ",\n";
    f << "    \"l1d_store_misses_per_sample\": " << (hw ? std::to_string(agg.best.l1d_store_misses() / N) : "null") << ",\n";
    // "Requests" = total L1D accesses, not just misses -- the register-
    // pressure proxy (see event_counter.h). Linux via perf_event_open's
    // PERF_TYPE_HW_CACHE, Apple via LD_UNIT_UOP/ST_UNIT_UOP (unvalidated
    // against a disassembly count so far); 0 on Windows, not wired up there.
    f << "    \"l1d_load_requests_per_sample\": " << (hw ? std::to_string(agg.best.l1d_load_requests() / N)   : "null") << ",\n";
    f << "    \"l1d_store_requests_per_sample\": " << (hw ? std::to_string(agg.best.l1d_store_requests() / N) : "null") << "\n";
    f << "  }\n";
    f << "}\n";
}

// ── Per-case runner ──────────────────────────────────────────────────────

template <typename Traits>
static void run_case(event_collector& ec, const std::string& out_dir,
                      const std::vector<float>& input_buf) {
    std::vector<float> output_buf(input_buf.size());
    typename Traits::Params params{};

    auto agg = collect(ec, [&] {
        typename Traits::State state{};
        const float* in_ch[1]  = { input_buf.data() };
        float*       out_ch[1] = { output_buf.data() };
        if constexpr (Traits::has_reset) {
            Traits::reset(params, &state, 1, SAMPLE_RATE, Traits::vi_dc);
        }
        Traits::compute(in_ch, out_ch, 1, (int) input_buf.size(), params, &state, SAMPLE_RATE);
    });

    print_header(agg.has_events);
    print_row(Traits::name, agg, (int) input_buf.size());

    write_json(out_dir + "/" + Traits::name + ".json", Traits::name, agg, (int) input_buf.size());
}

// ── main ─────────────────────────────────────────────────────────────────

static void print_usage(const char* argv0) {
    std::cerr << "Usage: " << argv0 << " --out-dir <dir> [--case <name>]...\n"
              << "  --case rc_lowpass | eq_filter2 | diode_clipper | common_emitter | pedal_drive |\n"
              << "         pedal_model | tube_compressor | eq73\n"
              << "  (default: all cases)\n";
}

int main(int argc, char** argv) {
    std::set<std::string> cases;
    std::string out_dir = "results";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--case" && i + 1 < argc) {
            cases.insert(argv[++i]);
        } else if (arg == "--out-dir" && i + 1 < argc) {
            out_dir = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }
    const bool run_all = cases.empty();

    event_collector ec;
    if (!ec.has_events())
        std::cout << "hardware counters: disabled (run with sudo for cycles/instructions/L1D)\n";

    // Small-amplitude two-tone signal -- big enough to exercise the nonlinear
    // cases' Newton solve away from the trivial all-zero fixed point, small
    // enough to stay well inside where the diode/BJT models are well-behaved.
    std::vector<float> input_buf(NUM_SAMPLES);
    for (int n = 0; n < NUM_SAMPLES; ++n) {
        const float t = (float) n / SAMPLE_RATE;
        input_buf[n] = 0.05f * sinf(2.0f * 3.14159265f * 440.0f * t)
                     + 0.03f * sinf(2.0f * 3.14159265f * 1000.0f * t);
    }

    if (run_all || cases.count("rc_lowpass"))     run_case<RcLowpassTraits>(ec, out_dir, input_buf);
    if (run_all || cases.count("eq_filter2"))     run_case<EqFilter2Traits>(ec, out_dir, input_buf);
    if (run_all || cases.count("diode_clipper"))  run_case<DiodeClipperTraits>(ec, out_dir, input_buf);
    if (run_all || cases.count("common_emitter")) run_case<CommonEmitterTraits>(ec, out_dir, input_buf);
    if (run_all || cases.count("pedal_drive"))    run_case<PedalDriveTraits>(ec, out_dir, input_buf);
    if (run_all || cases.count("pedal_model"))    run_case<PedalModelTraits>(ec, out_dir, input_buf);
    if (run_all || cases.count("tube_compressor")) run_case<TubeCompressorTraits>(ec, out_dir, input_buf);
    if (run_all || cases.count("eq73"))           run_case<Eq73Traits>(ec, out_dir, input_buf);

    return 0;
}
