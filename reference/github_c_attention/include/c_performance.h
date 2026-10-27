#ifndef C_PERFORMANCE_H
#define C_PERFORMANCE_H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EE_PERF_PARSE = 0,
    EE_PERF_POSSESSION,
    EE_PERF_ATTENTION,
    EE_PERF_SCHEDULER,
    EE_PERF_FOVEA_REQUEST,
    EE_PERF_TOTAL_FRAME,
    EE_PERF_STAGE_COUNT
} EE_PerformanceStage;

typedef struct {
    uint8_t enabled;
    clock_t starts[EE_PERF_STAGE_COUNT];
    uint64_t total_ticks[EE_PERF_STAGE_COUNT];
    uint32_t samples[EE_PERF_STAGE_COUNT];
} EE_PerformanceStats;

void ee_performance_init(EE_PerformanceStats *stats, uint8_t enabled);
void ee_performance_begin(EE_PerformanceStats *stats, EE_PerformanceStage stage);
void ee_performance_end(EE_PerformanceStats *stats, EE_PerformanceStage stage);
void ee_performance_print(const EE_PerformanceStats *stats);

#ifdef __cplusplus
}
#endif

#endif
