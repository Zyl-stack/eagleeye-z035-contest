#include "c_performance.h"

#include <stdio.h>
#include <string.h>

static const char *stage_name(EE_PerformanceStage stage)
{
    static const char *names[EE_PERF_STAGE_COUNT] = {
        "bc_adapter_parse", "possession", "attention", "scheduler",
        "fovea_request", "total_c_frame"
    };
    return stage < EE_PERF_STAGE_COUNT ? names[stage] : "unknown";
}

void ee_performance_init(EE_PerformanceStats *stats, uint8_t enabled)
{
    if (!stats) return;
    memset(stats, 0, sizeof(*stats));
    stats->enabled = enabled;
}

void ee_performance_begin(EE_PerformanceStats *stats, EE_PerformanceStage stage)
{
    if (stats && stats->enabled && stage < EE_PERF_STAGE_COUNT) stats->starts[stage] = clock();
}

void ee_performance_end(EE_PerformanceStats *stats, EE_PerformanceStage stage)
{
    clock_t end;
    if (!stats || !stats->enabled || stage >= EE_PERF_STAGE_COUNT) return;
    end = clock();
    if (end >= stats->starts[stage]) {
        stats->total_ticks[stage] += (uint64_t)(end - stats->starts[stage]);
        stats->samples[stage]++;
    }
}

void ee_performance_print(const EE_PerformanceStats *stats)
{
    EE_PerformanceStage stage;
    if (!stats || !stats->enabled) return;
    printf("[C_PERFORMANCE] clock-based PC instrumentation; values are not FPGA timing.\n");
    for (stage = EE_PERF_PARSE; stage < EE_PERF_STAGE_COUNT; ++stage) {
        double average_us = stats->samples[stage]
                                ? ((double)stats->total_ticks[stage] * 1000000.0 /
                                   (double)CLOCKS_PER_SEC / (double)stats->samples[stage])
                                : 0.0;
        printf("  %s average_us=%.3f samples=%u\n", stage_name(stage), average_us,
               (unsigned)stats->samples[stage]);
    }
}
