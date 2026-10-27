#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "a_fovea_adapter.h"
#include "attention.h"
#include "attention_scheduler.h"
#include "ball_challenger_gate.h"
#include "bc_adapter.h"
#include "c_config.h"
#include "director_state.h"
#include "enhance_policy.h"
#include "feedback_attention.h"
#include "fovea_transaction.h"
#include "motion_predictor.h"
#include "possession_estimator.h"
#include "roi_motion_controller.h"
#include "roi_smoother.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    EE_CConfig config;
    EE_AttentionScheduler scheduler;
    EE_ROISmoother smoother;
    EE_ROIMotionController motion_controller;
    EE_Director director;
    EE_FoveaTransactionBuilder fovea_transactions;
    uint16_t carrier_id;
    uint8_t carrier_score;
} HandoffRuntime;

static const EE_GlobalTargetDescriptor *find_target(const EE_GlobalTargetList *list,
                                                     uint16_t target_id,
                                                     uint8_t *out_index)
{
    uint8_t i;

    if (!list) {
        return NULL;
    }
    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].valid && list->targets[i].target_id == target_id) {
            if (out_index) {
                *out_index = i;
            }
            return &list->targets[i];
        }
    }
    return NULL;
}

static void runtime_init(HandoffRuntime *runtime)
{
    memset(runtime, 0, sizeof(*runtime));
    runtime->config = ee_c_config_default();
    runtime->carrier_id = EE_INVALID_TARGET_ID;
    ee_scheduler_init(&runtime->scheduler, runtime->config.scheduler);
    ee_roi_smoother_init(&runtime->smoother, runtime->config.roi_smoother);
    ee_roi_motion_controller_init(&runtime->motion_controller,
                                  runtime->config.roi_smoother,
                                  runtime->config.roi_motion_controller);
    ee_director_init(&runtime->director, runtime->config.director_handover_frames);
    ee_motion_predictor_init();
    ee_motion_predictor_set_nominal_input_frame_delta(runtime->config.nominal_input_frame_delta);
    ee_motion_predictor_set_max_gap_frames(runtime->config.max_prediction_gap_frames);
    ee_fovea_transaction_init(&runtime->fovea_transactions);
    ee_feedback_attention_clear();
}

static void assert_parsed_contract(const EE_BParsedFrame *frame)
{
    uint8_t i;

    assert(frame != NULL);
    assert(frame->count <= EE_MAX_TARGETS);
    for (i = 0u; i < frame->count; ++i) {
        const EE_BParsedTarget *target = &frame->targets[i];

        assert(target->frame_id == frame->frame_id);
        assert(target->target_id != 0u);
        assert(target->target_id != EE_INVALID_TARGET_ID);
        assert(target->target_type == EE_SOCCER_TARGET_BALL ||
               target->target_type == EE_SOCCER_TARGET_PLAYER);
        assert(target->confidence <= 100u);
        assert(target->valid <= 1u);
        assert(target->velocity_valid <= 1u);
        assert(target->x < EE_BC_FRAME_WIDTH && target->y < EE_BC_FRAME_HEIGHT);
        assert(target->w > 0u && target->h > 0u);
        assert((uint32_t)target->x + target->w <= EE_BC_FRAME_WIDTH);
        assert((uint32_t)target->y + target->h <= EE_BC_FRAME_HEIGHT);
    }
}

static void run_frame(HandoffRuntime *runtime,
                      EE_GlobalTargetList *list,
                      const EE_BParsedFrame *parsed,
                      uint32_t *request_count,
                      uint32_t *transaction_count,
                      uint32_t *prediction_count)
{
    EE_FoveaRequest desired;
    EE_GlobalTargetList scheduler_candidates;
    EE_FoveaRequest smoothed;
    EE_FoveaRequest adapted;
    EE_MotionPrediction prediction;
    EE_AttentionBreakdown attention;
    EE_EnhancePolicyInput policy_input;
    EE_EnhanceDecision enhance;
    EE_ROIMotionInput motion_input;
    EE_ROIMotionDecision motion_decision;
    EE_FoveaTransactionResult transaction_result;
    const EE_SchedulerDecision *decision;
    const EE_GlobalTargetDescriptor *selected;
    uint8_t selected_index = 0u;
    uint16_t ball_id = EE_INVALID_TARGET_ID;
    uint8_t ball_valid = 0u;
    uint8_t i;

    assert(ee_bc_adapt_frame(parsed, list) == 1u);
    memset(&prediction, 0, sizeof(prediction));
    ee_possession_update_list(list, runtime->carrier_id, runtime->carrier_score,
                              &runtime->carrier_id, &runtime->carrier_score);
    ee_attention_update_scores_for_scheduler_step(
        list->targets, list->count, &runtime->config.attention,
        runtime->scheduler.wait_frames);
    assert(ee_ball_challenger_gate_apply(&runtime->config, &runtime->scheduler, list,
                                         &scheduler_candidates) == 1u);
    if (!ee_scheduler_step(&runtime->scheduler, &scheduler_candidates, &desired)) {
        memset(&desired, 0, sizeof(desired));
    }
    decision = ee_scheduler_last_decision(&runtime->scheduler);
    selected = find_target(list, decision->selected_target_id, &selected_index);
    memset(&attention, 0, sizeof(attention));
    if (selected) {
        ee_attention_compute_breakdown(selected, &runtime->config.attention,
                                       runtime->scheduler.wait_frames[selected_index], &attention);
        assert(attention.feedback_adjustment == 0);
        assert(attention.base_total == attention.final_total);
    }

    ee_director_update(&runtime->director, list, runtime->carrier_id,
                       decision->selected_target_id);
    memset(&motion_input, 0, sizeof(motion_input));
    motion_input.director_state = runtime->director.state;
    motion_input.director_event = runtime->director.event;
    motion_input.switch_reason = decision->switch_reason;
    motion_input.selected_target_id = decision->selected_target_id;
    motion_input.previous_selected_target_id = decision->previous_target_id;
    assert(ee_roi_motion_controller_step(&runtime->motion_controller, &motion_input,
                                         &motion_decision) == 1u);

    memset(&policy_input, 0, sizeof(policy_input));
    policy_input.selected_target = selected;
    policy_input.attention = selected ? &attention : NULL;
    policy_input.possession_target_id = runtime->carrier_id;
    enhance = ee_enhance_policy_decide(&policy_input, &runtime->config);

    if (desired.valid) {
        assert(selected != NULL);
        assert(desired.request_id != 0u);
        assert(desired.target_id == selected->target_id);
        if (ee_motion_predict_with_latency(selected, runtime->config.prediction_horizon_frames,
                                           &runtime->config.latency_model, &prediction) &&
            prediction.prediction_valid) {
            desired.roi_x = prediction.predicted_x;
            desired.roi_y = prediction.predicted_y;
            ++*prediction_count;
        }
        ee_roi_smoother_set_config(&runtime->smoother, motion_decision.smoother_config);
        assert(ee_roi_smoother_step(&runtime->smoother, &desired, &smoothed) == 1u);
        smoothed.enhance_mode = (uint8_t)enhance.mode;
        assert(ee_a_fovea_adapt_request(&smoothed, &adapted) == 0);
        transaction_result = ee_fovea_transaction_build(&runtime->fovea_transactions,
                                                         &adapted, list->frame_id, &adapted);
        assert(transaction_result != EE_FOVEA_TRANSACTION_INVALID);
        if (transaction_result == EE_FOVEA_TRANSACTION_NEW) {
            ++*transaction_count;
        }
        assert(adapted.request_id != 0u);
        assert(adapted.target_id == selected->target_id);
        assert(adapted.source_frame_id == list->frame_id);
        assert(adapted.roi_w == A_FOVEA_ROI_WIDTH && adapted.roi_h == A_FOVEA_ROI_HEIGHT);
        assert(adapted.roi_x <= A_FOVEA_ROI_X_MAX && adapted.roi_y <= A_FOVEA_ROI_Y_MAX);
        assert(adapted.enhance_mode <= EE_ENHANCE_MODE_LANCZOS);
        ++*request_count;
    } else {
        memset(&adapted, 0, sizeof(adapted));
    }

    for (i = 0u; i < list->count; ++i) {
        if (list->targets[i].is_ball) {
            ball_id = list->targets[i].target_id;
            ball_valid = list->targets[i].valid;
            break;
        }
    }
    printf("frame=%u count=%u ball=%u valid=%u selected=%u type=%s base=%u final=%u "
           "switch=%s predicted=%u roi_mode=%s roi=(%u,%u,%u,%u) enhance=%s request=%u\n",
           (unsigned)list->frame_id, list->count, ball_id, ball_valid,
           decision->selected_target_id,
           selected ? ee_soccer_target_to_string((EE_SoccerTargetType)selected->target_type) : "NONE",
           (unsigned)attention.base_total, (unsigned)attention.final_total,
           ee_switch_reason_to_string(decision->switch_reason), prediction.prediction_valid,
           ee_roi_motion_mode_to_string(motion_decision.mode), adapted.roi_x, adapted.roi_y,
           adapted.roi_w, adapted.roi_h, ee_enhance_mode_to_string(enhance.mode),
           (unsigned)adapted.request_id);
    ee_motion_predictor_update(list);
}

int main(void)
{
    const char *path = "tests/data/ball_static_b_target_stream.txt";
    EE_BCStreamReader reader;
    EE_BParsedFrame parsed;
    EE_GlobalTargetList list;
    HandoffRuntime runtime;
    FILE *file;
    uint32_t frame_count = 0u;
    uint32_t request_count = 0u;
    uint32_t transaction_count = 0u;
    uint32_t prediction_count = 0u;
    uint32_t first_ball_frame = UINT32_MAX;
    uint32_t last_ball_frame = 0u;
    uint16_t ball_ids[12];
    uint8_t ball_valid[12];
    uint8_t ball_count = 0u;
    int result;
    uint8_t i;

    file = fopen(path, "r");
    assert(file != NULL);
    ee_bc_stream_reader_init(&reader, file);
    runtime_init(&runtime);
    printf("B YOLO handoff C pipeline regression\n");
    printf("frame count ball valid selected type base final switch predicted roi_mode ROI enhance request\n");

    while ((result = ee_bc_stream_read_frame(&reader, &parsed)) == 1) {
        assert(parsed.frame_id == frame_count);
        assert(parsed.source_width == EE_BC_FRAME_WIDTH);
        assert(parsed.source_height == EE_BC_FRAME_HEIGHT);
        assert_parsed_contract(&parsed);
        if (parsed.count == 1u && parsed.targets[0].target_type == EE_SOCCER_TARGET_BALL) {
            assert(parsed.targets[0].x == 195u && parsed.targets[0].y == 18u);
            assert(parsed.targets[0].w == 6u && parsed.targets[0].h == 11u);
            assert(ball_count < (uint8_t)(sizeof(ball_ids) / sizeof(ball_ids[0])));
            if (first_ball_frame == UINT32_MAX) {
                first_ball_frame = parsed.frame_id;
            }
            last_ball_frame = parsed.frame_id;
            ball_ids[ball_count] = parsed.targets[0].target_id;
            ball_valid[ball_count] = parsed.targets[0].valid;
            ++ball_count;
        }
        run_frame(&runtime, &list, &parsed, &request_count, &transaction_count, &prediction_count);
        ++frame_count;
    }
    fclose(file);

    assert(result == 0);
    assert(frame_count == 24u);
    assert(first_ball_frame == 12u && last_ball_frame == 23u && ball_count == 12u);
    for (i = 0u; i < ball_count; ++i) {
        assert(ball_ids[i] == 1u);
        assert(ball_valid[i] == 1u);
    }
    assert(request_count == 12u);
    assert(transaction_count == 12u);
    assert(prediction_count == 11u);
    assert(runtime.scheduler.active_request.request_id == 1u);
    assert(runtime.scheduler.active_request.source_frame_id == 12u);
    assert(runtime.fovea_transactions.last_transaction.request_id == 12u);
    assert(runtime.fovea_transactions.last_transaction.source_frame_id == 23u);
    printf("[PASS] stream header=%s frames=24 first_ball=12 last_ball=23 ball_ids=",
           EE_BC_STREAM_MAGIC);
    for (i = 0u; i < ball_count; ++i) {
        printf("%u%s", ball_ids[i], i + 1u == ball_count ? "" : ",");
    }
    printf(" valid=1x12 requests=%u transactions=%u predictions=%u\n",
           (unsigned)request_count, (unsigned)transaction_count, (unsigned)prediction_count);
    printf("[PASS] all valid Fovea Requests preserve nonzero IDs and A ROI=(320x180), x=0..320 y=0..180.\n");
    return 0;
}
