#if defined(_MSC_VER)
/* fopen is standard C; this only suppresses MSVC's non-portability warning. */
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "attention.h"
#include "attention_scheduler.h"
#include "ball_challenger_gate.h"
#include "bc_adapter.h"
#include "c_config.h"
#include "c_performance.h"
#include "c_runtime_log.h"
#include "fovea_transaction.h"
#include "possession_estimator.h"
#include "roi_smoother.h"

#include <stdio.h>
#include <string.h>

static void print_frame(const EE_GlobalTargetList *list, const EE_FoveaRequest *request)
{
    EE_AttentionConfig config = ee_attention_default_config();
    uint8_t i;

    printf("FRAME %u: target_count=%u\n", (unsigned)list->frame_id, list->count);
    for (i = 0u; i < list->count; ++i) {
        EE_AttentionDebug debug;
        const EE_GlobalTargetDescriptor *target = &list->targets[i];
        ee_attention_compute_debug(target, &config, &debug);
        printf("  id=%u type=%s valid=%u bbox=(%u,%u,%u,%u) motion=%u uncertainty=%u attention=%u\n",
               target->target_id, ee_soccer_target_to_string((EE_SoccerTargetType)target->target_type),
               target->valid, target->x, target->y, target->w, target->h,
               target->motion_level, target->uncertainty, (unsigned)target->attention_score);
    }
    printf("  selected_target=%u\n", request->target_id);
    printf("  fovea_roi=(%u,%u,%u,%u) request_id=%u\n", request->roi_x, request->roi_y,
           request->roi_w, request->roi_h, (unsigned)request->request_id);
}

int main(int argc, char **argv)
{
    EE_BCStreamReader reader;
    EE_BParsedFrame parsed;
    EE_GlobalTargetList list;
    EE_GlobalTargetList scheduler_candidates;
    EE_AttentionScheduler scheduler;
    EE_FoveaRequest request;
    EE_FoveaRequest smoothed_request;
    EE_FoveaTransactionBuilder fovea_transactions;
    EE_CConfig config;
    EE_ROISmoother smoother;
    EE_RuntimeLogger logger;
    EE_PerformanceStats performance;
    FILE *file;
    uint16_t previous_carrier = EE_INVALID_TARGET_ID;
    uint8_t previous_score = 0u;
    uint16_t carrier;
    uint8_t score;
    int result;
    int arg;
    const char *input_path = NULL;
    uint8_t performance_enabled = 0u;
    EE_LogLevel log_level = EE_LOG_LEVEL_NONE;

    for (arg = 1; arg < argc; ++arg) {
        if (strcmp(argv[arg], "--summary") == 0) log_level = EE_LOG_LEVEL_SUMMARY;
        else if (strcmp(argv[arg], "--verbose") == 0) log_level = EE_LOG_LEVEL_VERBOSE;
        else if (strcmp(argv[arg], "--perf") == 0) performance_enabled = 1u;
        else if (!input_path) input_path = argv[arg];
        else { fprintf(stderr, "Unexpected argument: %s\n", argv[arg]); return 1; }
    }

    if (!input_path) { fprintf(stderr, "Usage: bc_stream_handoff <stream> [--summary] [--verbose] [--perf]\n"); return 1; }
    file = fopen(input_path, "r");
    if (!file) {
        fprintf(stderr, "Cannot open %s\n", input_path);
        return 1;
    }
    ee_bc_stream_reader_init(&reader, file);
    config = ee_c_config_default();
    ee_scheduler_init(&scheduler, config.scheduler);
    ee_fovea_transaction_init(&fovea_transactions);
    ee_roi_smoother_init(&smoother, config.roi_smoother);
    ee_runtime_logger_init(&logger, log_level);
    ee_performance_init(&performance, performance_enabled);
    printf("B->C soccer stream PC integration demo\n");

    for (;;) {
        EE_RuntimeFrameLog frame_log;
        ee_performance_begin(&performance, EE_PERF_TOTAL_FRAME);
        ee_performance_begin(&performance, EE_PERF_PARSE);
        result = ee_bc_stream_read_frame(&reader, &parsed);
        ee_performance_end(&performance, EE_PERF_PARSE);
        if (result != 1) break;
        if (!ee_bc_adapt_frame(&parsed, &list)) {
            fprintf(stderr, "Adapter rejected frame %u\n", (unsigned)parsed.frame_id);
            fclose(file);
            return 1;
        }
        if (list.count == 0u) {
            printf("FRAME %u: EMPTY, no Fovea request\n", (unsigned)list.frame_id);
            ee_performance_end(&performance, EE_PERF_TOTAL_FRAME);
            continue;
        }
        ee_performance_begin(&performance, EE_PERF_POSSESSION);
        ee_possession_update_list(&list, previous_carrier, previous_score, &carrier, &score);
        ee_performance_end(&performance, EE_PERF_POSSESSION);
        previous_carrier = carrier;
        previous_score = score;
        ee_performance_begin(&performance, EE_PERF_ATTENTION);
        ee_attention_update_scores_for_scheduler_step(
            list.targets, list.count, &config.attention, scheduler.wait_frames);
        ee_performance_end(&performance, EE_PERF_ATTENTION);
        ee_performance_begin(&performance, EE_PERF_SCHEDULER);
        if (!ee_ball_challenger_gate_apply(&config, &scheduler, &list,
                                           &scheduler_candidates)) {
            fprintf(stderr, "BALL challenger gate rejected frame %u\n", (unsigned)list.frame_id);
            fclose(file);
            return 1;
        }
        if (!ee_scheduler_step(&scheduler, &scheduler_candidates, &request)) {
            fprintf(stderr, "Scheduler found no valid candidate at frame %u\n", (unsigned)list.frame_id);
            fclose(file);
            return 1;
        }
        ee_performance_end(&performance, EE_PERF_SCHEDULER);
        ee_performance_begin(&performance, EE_PERF_FOVEA_REQUEST);
        if (!ee_roi_smoother_step(&smoother, &request, &smoothed_request)) {
            fprintf(stderr, "ROI smoother rejected frame %u\n", (unsigned)list.frame_id);
            fclose(file);
            return 1;
        }
        if (ee_fovea_transaction_build(&fovea_transactions, &smoothed_request,
                                       list.frame_id, &smoothed_request) ==
            EE_FOVEA_TRANSACTION_INVALID) {
            fprintf(stderr, "Fovea transaction builder rejected frame %u\n",
                    (unsigned)list.frame_id);
            fclose(file);
            return 1;
        }
        ee_performance_end(&performance, EE_PERF_FOVEA_REQUEST);
        memset(&frame_log, 0, sizeof(frame_log));
        frame_log.targets = &list;
        frame_log.request = &smoothed_request;
        frame_log.decision = ee_scheduler_last_decision(&scheduler);
        frame_log.possession_target_id = carrier;
        frame_log.possession_score = score;
        ee_runtime_log_frame(&logger, &frame_log);
        print_frame(&list, &smoothed_request);
        ee_performance_end(&performance, EE_PERF_TOTAL_FRAME);
    }
    fclose(file);
    ee_performance_print(&performance);
    return result == 0 ? 0 : 1;
}
