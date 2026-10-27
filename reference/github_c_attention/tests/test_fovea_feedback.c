#include "a_fovea_adapter.h"
#include "attention.h"
#include "attention_scheduler.h"
#include "fovea_feedback.h"
#include "fovea_feedback_sim.h"
#include "feedback_attention.h"
#include "fovea_transaction.h"
#include "motion_predictor.h"
#include "roi_motion_controller.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static EE_FoveaRequest make_request(uint32_t request_id, uint32_t frame_id,
                                    uint16_t target_id, uint16_t deadline)
{
    EE_FoveaRequest request;

    memset(&request, 0, sizeof(request));
    request.request_id = request_id;
    request.source_frame_id = frame_id;
    request.target_id = target_id;
    request.roi_x = 160u;
    request.roi_y = 90u;
    request.roi_w = 320u;
    request.roi_h = 180u;
    request.deadline_frames = deadline;
    request.valid = 1u;
    return request;
}

static EE_FoveaFeedback simulated(const EE_FoveaRequest *request,
                                  EE_FoveaFeedbackSimCase sim_case)
{
    EE_FoveaFeedback feedback;

    assert(ee_fovea_feedback_simulate(request, sim_case, &feedback) == 1u);
    return feedback;
}

static void test_exact_match_and_quality(void)
{
    EE_FoveaFeedbackState state;
    EE_FoveaRequest request = make_request(10u, 100u, 3u, 4u);
    EE_FoveaFeedback feedback = simulated(&request, EE_FOVEA_FEEDBACK_SIM_QUALITY_FLAGS);

    ee_fovea_feedback_init(&state);
    assert(ee_fovea_feedback_begin(&state, &request) == 1u);
    assert(state.status == EE_FOVEA_FEEDBACK_PENDING);
    assert(ee_fovea_feedback_receive(&state, &feedback, 101u) == 1u);
    assert(state.status == EE_FOVEA_FEEDBACK_ACCEPTED);
    assert(state.latest_accepted.request_id == 10u);
    assert(state.latest_accepted.source_frame_id == 100u);
    assert(state.latest_accepted.target_id == 3u);
    assert(state.latest_accepted.quality_flags == feedback.quality_flags);
    assert(state.age_frames == 1u);
    printf("[PASS] FEEDBACK ACCEPTED req=10 frame=100 target=3 quality=0x%08X\n",
           (unsigned)feedback.quality_flags);
}

static void test_stale_and_request_mismatch(void)
{
    EE_FoveaFeedbackState state;
    EE_FoveaRequest previous = make_request(29u, 99u, 3u, 4u);
    EE_FoveaRequest current = make_request(30u, 100u, 3u, 4u);
    EE_FoveaFeedback feedback;

    ee_fovea_feedback_init(&state);
    assert(ee_fovea_feedback_begin(&state, &previous) == 1u);
    feedback = simulated(&previous, EE_FOVEA_FEEDBACK_SIM_EXACT);
    assert(ee_fovea_feedback_receive(&state, &feedback, 100u) == 1u);
    assert(ee_fovea_feedback_begin(&state, &current) == 1u);

    feedback = simulated(&current, EE_FOVEA_FEEDBACK_SIM_STALE);
    assert(feedback.request_id == 29u);
    assert(ee_fovea_feedback_receive(&state, &feedback, 101u) == 0u);
    assert(state.status == EE_FOVEA_FEEDBACK_STALE);
    assert(state.pending == 1u && state.pending_request.request_id == 30u);
    assert(state.latest_accepted.request_id == 29u);
    printf("[PASS] STALE_FEEDBACK current_req=30 received_req=29 preserves current pending request\n");

    feedback = simulated(&current, EE_FOVEA_FEEDBACK_SIM_REQUEST_MISMATCH);
    assert(ee_fovea_feedback_receive(&state, &feedback, 101u) == 0u);
    assert(state.status == EE_FOVEA_FEEDBACK_REQUEST_ID_MISMATCH);
    assert(state.pending_request.request_id == 30u);
    printf("[PASS] REQUEST_ID_MISMATCH current_req=30 received_req=31\n");
}

static void test_frame_target_invalid_and_duplicate(void)
{
    EE_FoveaFeedbackState state;
    EE_FoveaRequest request = make_request(27u, 100u, 3u, 4u);
    EE_FoveaFeedback feedback;

    ee_fovea_feedback_init(&state);
    assert(ee_fovea_feedback_begin(&state, &request) == 1u);
    feedback = simulated(&request, EE_FOVEA_FEEDBACK_SIM_FRAME_MISMATCH);
    assert(ee_fovea_feedback_receive(&state, &feedback, 101u) == 0u);
    assert(state.status == EE_FOVEA_FEEDBACK_FRAME_ID_MISMATCH);
    printf("[PASS] FRAME_ID_MISMATCH current_frame=100 received_frame=101\n");

    feedback = simulated(&request, EE_FOVEA_FEEDBACK_SIM_TARGET_MISMATCH);
    assert(ee_fovea_feedback_receive(&state, &feedback, 101u) == 0u);
    assert(state.status == EE_FOVEA_FEEDBACK_TARGET_ID_MISMATCH);
    printf("[PASS] TARGET_ID_MISMATCH current_target=3 received_target=4\n");

    feedback = simulated(&request, EE_FOVEA_FEEDBACK_SIM_INVALID);
    assert(ee_fovea_feedback_receive(&state, &feedback, 101u) == 0u);
    assert(state.status == EE_FOVEA_FEEDBACK_INVALID);
    printf("[PASS] INVALID feedback rejected\n");

    feedback = simulated(&request, EE_FOVEA_FEEDBACK_SIM_EXACT);
    assert(ee_fovea_feedback_receive(&state, &feedback, 101u) == 1u);
    assert(state.status == EE_FOVEA_FEEDBACK_ACCEPTED);
    assert(ee_fovea_feedback_receive(&state, &feedback, 102u) == 0u);
    assert(state.status == EE_FOVEA_FEEDBACK_DUPLICATE);
    assert(state.latest_accepted.request_id == 27u);
    printf("[PASS] DUPLICATE feedback rejected without replacing accepted req=27\n");
}

static void test_timeout_late_and_out_of_order(void)
{
    EE_FoveaFeedbackState state;
    EE_FoveaRequest timeout_request = make_request(40u, 100u, 3u, 2u);
    EE_FoveaRequest later_request = make_request(42u, 110u, 5u, 3u);
    EE_FoveaFeedback feedback;

    ee_fovea_feedback_init(&state);
    assert(ee_fovea_feedback_begin(&state, &timeout_request) == 1u);
    ee_fovea_feedback_advance(&state, 103u);
    assert(state.status == EE_FOVEA_FEEDBACK_TIMEOUT && state.timed_out == 1u);
    feedback = simulated(&timeout_request, EE_FOVEA_FEEDBACK_SIM_EXACT);
    assert(ee_fovea_feedback_receive(&state, &feedback, 104u) == 0u);
    assert(state.status == EE_FOVEA_FEEDBACK_TIMEOUT);
    printf("[PASS] TIMEOUT then late feedback remains rejected req=40\n");

    assert(ee_fovea_feedback_begin(&state, &later_request) == 1u);
    feedback = simulated(&later_request, EE_FOVEA_FEEDBACK_SIM_STALE);
    feedback.request_id = 40u;
    assert(ee_fovea_feedback_receive(&state, &feedback, 111u) == 0u);
    assert(state.status == EE_FOVEA_FEEDBACK_STALE);
    assert(state.pending_request.request_id == 42u && state.pending == 1u);
    printf("[PASS] out-of-order old req=40 cannot overwrite pending req=42\n");
}

static void test_request_to_feedback_pipeline(void)
{
    EE_GlobalTargetList previous_list;
    EE_GlobalTargetList list;
    EE_GlobalTargetDescriptor target;
    EE_AttentionScheduler scheduler;
    EE_FoveaRequest request;
    EE_FoveaRequest smoothed;
    EE_FoveaRequest adapted;
    EE_MotionPrediction prediction;
    EE_ROISmoother smoother;
    EE_ROIMotionController controller;
    EE_ROIMotionInput motion_input;
    EE_ROIMotionDecision motion_decision;
    EE_FoveaFeedbackState feedback_state;
    EE_FoveaTransactionBuilder transaction_builder;
    EE_FoveaFeedback feedback;
    EE_FeedbackAttentionContext feedback_context;
    EE_FeedbackAttentionPolicy feedback_policy;
    EE_AttentionConfig attention_config;
    EE_AttentionDebug next_frame_attention;
    uint16_t selected_id;
    uint32_t attention_before;

    memset(&target, 0, sizeof(target));
    target.target_id = 3u;
    target.frame_id = 2u;
    target.x = 300u;
    target.y = 140u;
    target.w = 14u;
    target.h = 14u;
    target.coarse_conf = 90u;
    target.vx = 12;
    target.vy = 2;
    target.valid = 1u;
    target.is_ball = 1u;
    target.target_type = EE_SOCCER_TARGET_BALL;

    memset(&previous_list, 0, sizeof(previous_list));
    previous_list.frame_id = 1u;
    previous_list.count = 1u;
    previous_list.targets[0] = target;
    previous_list.targets[0].frame_id = 1u;
    ee_motion_predictor_init();
    ee_motion_predictor_update(&previous_list);

    memset(&list, 0, sizeof(list));
    list.frame_id = 2u;
    list.count = 1u;
    list.targets[0] = target;
    ee_scheduler_init(&scheduler, ee_scheduler_default_config());
    attention_config = ee_attention_default_config();
    ee_attention_update_scores_for_scheduler_step(
        list.targets, list.count, &attention_config, scheduler.wait_frames);
    assert(ee_scheduler_step(&scheduler, &list, &request) == 1u);
    selected_id = request.target_id;
    attention_before = list.targets[0].attention_score;
    assert(ee_motion_predict(&target, &prediction) == 1u);
    request.roi_x = prediction.predicted_x;
    request.roi_y = prediction.predicted_y;

    ee_roi_smoother_init(&smoother, ee_roi_smoother_default_config());
    ee_roi_motion_controller_init(&controller, ee_roi_smoother_default_config(),
                                  ee_roi_motion_controller_default_config());
    memset(&motion_input, 0, sizeof(motion_input));
    motion_input.selected_target_id = selected_id;
    motion_input.previous_selected_target_id = EE_INVALID_TARGET_ID;
    motion_input.switch_reason = EE_SWITCH_REASON_FIRST_TARGET;
    assert(ee_roi_motion_controller_step(&controller, &motion_input, &motion_decision) == 1u);
    ee_roi_smoother_set_config(&smoother, motion_decision.smoother_config);
    assert(ee_roi_smoother_step(&smoother, &request, &smoothed) == 1u);
    assert(ee_a_fovea_adapt_request(&smoothed, &adapted) == 0);
    ee_fovea_transaction_init(&transaction_builder);
    transaction_builder.next_request_id = 77u;
    assert(ee_fovea_transaction_build(&transaction_builder, &adapted, list.frame_id, &adapted) ==
           EE_FOVEA_TRANSACTION_NEW);
    assert(adapted.request_id == 77u);

    ee_fovea_feedback_init(&feedback_state);
    assert(ee_fovea_feedback_begin(&feedback_state, &adapted) == 1u);
    feedback = simulated(&adapted, EE_FOVEA_FEEDBACK_SIM_EXACT);
    assert(ee_fovea_feedback_receive(&feedback_state, &feedback, 3u) == 1u);
    assert(feedback_state.status == EE_FOVEA_FEEDBACK_ACCEPTED);
    assert(selected_id == request.target_id);
    assert(attention_before == list.targets[0].attention_score);
    assert(adapted.request_id == feedback_state.latest_accepted.request_id);
    assert(adapted.source_frame_id == feedback_state.latest_accepted.source_frame_id);
    assert(adapted.target_id == feedback_state.latest_accepted.target_id);
    /* The exact result above is accepted after the frame-2 scheduling work.
       Bind it only for the next frame to verify transaction-to-Attention
       causality without changing the Scheduler implementation. */
    feedback_policy = ee_feedback_attention_pc_demo_policy();
    feedback_context.feedback_state = &feedback_state;
    feedback_context.current_frame_id = 3u;
    feedback_context.policy = feedback_policy;
    ee_feedback_attention_bind(&feedback_context);
    target.frame_id = 3u;
    ee_attention_compute_debug(&target, &attention_config, &next_frame_attention);
    assert(next_frame_attention.feedback_adjustment > 0);
    assert(next_frame_attention.feedback_reason == EE_FEEDBACK_ATTENTION_ACCEPTED);
    ee_feedback_attention_clear();
    printf("[PASS] pipeline selected=%u hardware_request=%u frame=%u prediction->TRACK->smooth->adapter->ACCEPTED\n",
           selected_id, (unsigned)adapted.request_id, (unsigned)adapted.source_frame_id);
}

int main(void)
{
    test_exact_match_and_quality();
    test_stale_and_request_mismatch();
    test_frame_target_invalid_and_duplicate();
    test_timeout_late_and_out_of_order();
    test_request_to_feedback_pipeline();
    printf("All Fovea feedback transaction tests passed.\n");
    return 0;
}
