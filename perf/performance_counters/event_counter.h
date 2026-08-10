#ifndef __EVENT_COUNTER_H
#define __EVENT_COUNTER_H

#include <cctype>
#ifndef _MSC_VER
#include <dirent.h>
#endif
#include <cinttypes>

#include <cstring>

#include <chrono>
#include <vector>

#include "linux_perf_events.h"
#ifdef __linux__
#include <libgen.h>
#endif

#if __APPLE__
#include "apple_arm_events.h"
#endif

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "windows_events.h"
#include "windows_events.cpp"
#endif

struct event_count {
  std::chrono::duration<double> elapsed;
  std::vector<unsigned long long> event_counts;
  event_count() : elapsed(0), event_counts{0,0,0,0,0,0,0,0,0} {}
  event_count(const std::chrono::duration<double> _elapsed, const std::vector<unsigned long long> _event_counts) : elapsed(_elapsed), event_counts(_event_counts) {}
  event_count(const event_count& other): elapsed(other.elapsed), event_counts(other.event_counts) { }

  // The types of counters (so we can read the getter more easily)
  enum event_counter_types {
    CPU_CYCLES,
    INSTRUCTIONS,
    BRANCH_MISSES=2,
    BRANCH=4,
    L1D_LOAD_MISSES=5,
    L1D_STORE_MISSES=6,
    // Total L1D accesses ("requests"), not just misses -- matters for
    // register-pressure analysis, since a spill shows up as extra load/store
    // *traffic* (raising requests) before it necessarily raises misses.
    // Linux: perf_event_open's PERF_TYPE_HW_CACHE with
    // PERF_COUNT_HW_CACHE_RESULT_ACCESS. Apple: LD_UNIT_UOP/ST_UNIT_UOP
    // (apple_arm_events.h) -- reverse-engineered event names, not yet
    // cross-checked against a disassembly load/store count.
    L1D_LOAD_REQUESTS=7,
    L1D_STORE_REQUESTS=8
  };

  double elapsed_sec() const { return std::chrono::duration<double>(elapsed).count(); }
  double elapsed_ns() const { return std::chrono::duration<double, std::nano>(elapsed).count(); }
  double cycles() const { return static_cast<double>(event_counts[CPU_CYCLES]); }
  double instructions() const { return static_cast<double>(event_counts[INSTRUCTIONS]); }
  double branch_misses() const { return static_cast<double>(event_counts[BRANCH_MISSES]); }
  double branches() const { return static_cast<double>(event_counts[BRANCH]); }
  double l1d_load_misses() const { return static_cast<double>(event_counts[L1D_LOAD_MISSES]); }
  double l1d_store_misses() const { return static_cast<double>(event_counts[L1D_STORE_MISSES]); }
  double l1d_misses() const { return l1d_load_misses() + l1d_store_misses(); }
  double l1d_load_requests() const { return static_cast<double>(event_counts[L1D_LOAD_REQUESTS]); }
  double l1d_store_requests() const { return static_cast<double>(event_counts[L1D_STORE_REQUESTS]); }
  double l1d_requests() const { return l1d_load_requests() + l1d_store_requests(); }
  event_count& operator=(const event_count& other) {
    this->elapsed = other.elapsed;
    this->event_counts = other.event_counts;
    return *this;
  }
  event_count operator+(const event_count& other) const {
    return event_count(elapsed+other.elapsed, {
      event_counts[0]+other.event_counts[0],
      event_counts[1]+other.event_counts[1],
      event_counts[2]+other.event_counts[2],
      event_counts[3]+other.event_counts[3],
      event_counts[4]+other.event_counts[4],
      event_counts[5]+other.event_counts[5],
      event_counts[6]+other.event_counts[6],
      event_counts[7]+other.event_counts[7],
      event_counts[8]+other.event_counts[8],
    });
  }

  void operator+=(const event_count& other) {
    *this = *this + other;
  }
};

struct event_aggregate {
  bool has_events = false;
  int iterations = 0;
  event_count total{};
  event_count best{};
  event_count worst{};

  event_aggregate() = default;

  void operator<<(const event_count& other) {
    if (iterations == 0 || other.elapsed < best.elapsed) {
      best = other;
    }
    if (iterations == 0 || other.elapsed > worst.elapsed) {
      worst = other;
    }
    iterations++;
    total += other;
  }

  double elapsed_sec() const { return total.elapsed_sec() / iterations; }
  double elapsed_ns() const { return total.elapsed_ns() / iterations; }
  double cycles() const { return total.cycles() / iterations; }
  double instructions() const { return total.instructions() / iterations; }
  double l1d_load_misses() const { return total.l1d_load_misses() / iterations; }
  double l1d_store_misses() const { return total.l1d_store_misses() / iterations; }
  double l1d_misses() const { return total.l1d_misses() / iterations; }
  double l1d_load_requests() const { return total.l1d_load_requests() / iterations; }
  double l1d_store_requests() const { return total.l1d_store_requests() / iterations; }
  double l1d_requests() const { return total.l1d_requests() / iterations; }
};

struct event_collector {
  event_count count{};
  std::chrono::time_point<std::chrono::steady_clock> start_clock{};

#if defined(__linux__)
  LinuxEvents<PERF_TYPE_HARDWARE> linux_events;
  // L1D read/write ACCESS (not just MISS) -- gives total request counts,
  // which raw miss counts alone can't (see L1D_LOAD_REQUESTS comment above).
  static constexpr int l1d_read_access  = PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ  << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16);
  static constexpr int l1d_write_access = PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16);
  LinuxEvents<PERF_TYPE_HW_CACHE> linux_cache_events;
  event_collector() : linux_events(std::vector<int>{
    PERF_COUNT_HW_CPU_CYCLES,
    PERF_COUNT_HW_INSTRUCTIONS,
  }), linux_cache_events(std::vector<int>{
    l1d_read_access,
    l1d_write_access,
  }) {}
  bool has_events() {
    return linux_events.is_working();
  }
#elif __APPLE__
  AppleEvents apple_events;
  performance_counters diff;
  event_collector() : diff(0) {
    apple_events.setup_performance_counters();
  }
  bool has_events() {
    return apple_events.setup_performance_counters();
  }
#elif defined(_WIN32)
  PMC_Tracer tracer {};
  PMC_Source_Mapping pmc_mapping {};
  PMC_Traced_Region region {};
  event_collector() {
    // Pin this thread to a single (non-zero) core and raise its scheduling priority so
    // PMC readings reflect only our own execution: cycle/instruction counters are
    // per-core hardware registers, so migrating cores mid-measurement or being
    // preempted by another thread on the same core silently corrupts the deltas.
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    const DWORD_PTR core_mask = (DWORD_PTR) 1 << (sys_info.dwNumberOfProcessors - 1);
    SetThreadAffinityMask(GetCurrentThread(), core_mask);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    SwitchToThread(); // force the affinity change to take effect before we read the processor number

    const wchar_t* amd_name_array[] =
    {
        L"TotalCycles",
        L"TotalIssues",
        L"BranchInstructions",
        L"BranchMispredictions",
    };
    pmc_mapping = map_pmc_names(amd_name_array);
    start_tracing(&tracer, &pmc_mapping, GetCurrentProcessorNumber());
  }
  ~event_collector() {
    stop_tracing(&tracer);
  }
  bool has_events() {
    // TraceHandle is 0 when StartTraceW failed (e.g. not running as admin)
    return tracer.TraceHandle != 0;
  }
#else
  event_collector() {}
  bool has_events() {
    return false;
  }
#endif

  inline void start() {
#if defined(__linux)
    linux_events.start();
    linux_cache_events.start();
#elif __APPLE__
    if(has_events()) { diff = apple_events.get_counters(); }
#elif defined(_WIN32)
    if(has_events()) start_counting(&tracer, &region);
#endif
    start_clock = std::chrono::steady_clock::now();
  }
  inline event_count& end() {
    const auto end_clock = std::chrono::steady_clock::now();
#if defined(__linux)
    linux_events.end(count.event_counts);
    std::vector<unsigned long long> cache_results(2, 0);
    linux_cache_events.end(cache_results);
    count.event_counts[7] = cache_results[0]; // L1D read (load) requests
    count.event_counts[8] = cache_results[1]; // L1D write (store) requests
#elif __APPLE__ // &&  __aarch64__
    if(has_events()) {
      performance_counters end = apple_events.get_counters();
      diff = end - diff;
    }
    count.event_counts[0] = diff.cycles;
    count.event_counts[1] = diff.instructions;
    count.event_counts[2] = diff.missed_branches;
    count.event_counts[3] = 0;
    count.event_counts[4] = diff.branches;
    count.event_counts[5] = diff.l1d_load_misses;
    count.event_counts[6] = diff.l1d_store_misses;
    // LD_UNIT_UOP/ST_UNIT_UOP (apple_arm_events.h) -- total load/store-unit
    // uops executed, hit or miss. Names are reverse-engineered (Instruments'
    // PMU event list, not officially documented by Apple) and not yet
    // cross-checked against a disassembly load/store count on real
    // hardware -- worth validating before trusting these for real
    // register-pressure comparisons.
    count.event_counts[7] = diff.ld_unit_uops;
    count.event_counts[8] = diff.st_unit_uops;
#elif defined(_WIN32)
    if(has_events()) {
      stop_counting(&tracer, &region);
      const auto result = get_or_wait_for_result(&tracer, &region);
      count.event_counts[0] = result.counters[0]; // cycles
      count.event_counts[1] = result.counters[1]; // instructions
      count.event_counts[2] = result.counters[3]; // missed branches
      count.event_counts[3] = 0;
      count.event_counts[4] = result.counters[2]; // branches
      count.event_counts[7] = 0; // L1D requests -- not wired up on Windows
      count.event_counts[8] = 0;
    }
#endif
    count.elapsed = end_clock - start_clock;
    return count;
  }
};

void pretty_print(event_aggregate aggregate, size_t number_of_floats, std::string name) {
    printf(" %16s ", name.c_str());
    printf(" %8.2f ns/sample ", aggregate.elapsed_ns() / number_of_floats);
    printf("\n");

    printf(" %16s ", "");
    printf(" %8.2f instructions/sample ", aggregate.best.instructions() / number_of_floats);
    printf("\n");
    printf(" %16s ", "");
    printf(" %8.2f cycles/sample  ", aggregate.best.cycles() / number_of_floats);
    printf("\n");

    printf(" %16s ", "");
    printf(" %8.2f branches/sample  ", aggregate.best.branches() / number_of_floats);
    printf("\n");

    printf(" %16s ", "");
    printf(" %8.2f branch miss/sample  ", aggregate.best.branch_misses() / number_of_floats);
    printf("\n");
    printf(" %16s ", "");
    printf(" %8.2f instructions/cycle ",
           aggregate.best.instructions() / aggregate.best.cycles());
    printf("\n");

    printf(" %16s ", "");
    printf(" %8.2f L1D load misses/sample  ", aggregate.best.l1d_load_misses() / number_of_floats);
    printf("\n");
    printf(" %16s ", "");
    printf(" %8.2f L1D store misses/sample  ", aggregate.best.l1d_store_misses() / number_of_floats);
    printf("\n");

    printf(" %16s ", "");
    printf(" %8.2f L1D load requests/sample  ", aggregate.best.l1d_load_requests() / number_of_floats);
    printf("\n");
    printf(" %16s ", "");
    printf(" %8.2f L1D store requests/sample  ", aggregate.best.l1d_store_requests() / number_of_floats);
    printf("\n");
}

#endif
