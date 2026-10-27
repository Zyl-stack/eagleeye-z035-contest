#include "attention.h"
#include "attention_scheduler.h"
#include "bc_adapter.h"
#include "feedback_attention.h"
#include "fovea_feedback.h"
#include "fovea_feedback_sim.h"

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

static EE_GlobalTargetDescriptor make_target(uint16_t target_id, uint8_t is_ball,
                                             uint8_t confidence, uint16_t w,
                                             uint16_t h)
{
    EE_GlobalTargetDescriptor target;

    memset(&target, 0, sizeof(target));
    target.target_id = target_id;
    target.frame_id = 100u;
    target.w = w;
    target.h = h;
    target.coarse_conf = confidence;
    target.valid = 1u;
    target.is_ball = is_ball;
    target.target_type = is_ball ? EE_SOCCER_TARGET_BALL : EE_SOCCER_TARGET_PLAYER;
    target.field_zone = EE_FIELD_NORMAL;
    target.ball_distance = 200u;
    return target;
}

static EE_FoveaFeedbackState accepted_state(uint32_t request_id, uint32_t frame_id,
                                             uint16_t target_id)
{
    EE_FoveaFeedbackState state;
    EE_FoveaRequest request = make_request(request_id, frame_id, target_id, 4u);
    EE_FoveaFeedback feedback;

    ee_fovea_feedback_init(&state);
    assert(ee_fovea_feedback_begin(&state, &request) == 1u);
    assert(ee_fovea_feedback_simulate(&request, EE_FOVEA_FEEDBACK_SIM_EXACT,
                                      &feedback) == 1u);
    assert(ee_fovea_feedback_receive(&state, &feedback, frame_id + 1u) == 1u);
    return state;
}

static void bind_feedback(const EE_FoveaFeedbackState *state, uint32_t frame_id,
                          EE_FeedbackAttentionPolicy policy)
{
    EE_FeedbackAttentionContext context;

    context.feedback_state = state;
    context.current_frame_id = frame_id;
    context.policy = policy;
    ee_feedback_attention_bind(&context);
}

static void test_default_disabled_preserves_base(void)
{
    EE_GlobalTargetDescriptor target = make_target(3u, 1u, 90u, 14u, 14u);
    EE_FoveaFeedbackState state = accepted_state(7u, 100u, 3u);
    EE_AttentionConfig attention = ee_attention_default_config();
    EE_AttentionDebug disabled;
    EE_AttentionDebug baseline;

    ee_feedback_attention_clear();
    ee_attention_compute_debug(&target, &attention, &baseline);
    bind_feedback(&state, 101u, ee_feedback_attention_default_policy());
    ee_attention_compute_debug(&target, &attention, &disabled);
    assert(disabled.feedback_adjustment == 0);
    assert(disabled.feedback_reason == EE_FEEDBACK_ATTENTION_DISABLED);
    assert(disabled.base_total == baseline.base_total);
    assert(disabled.final_total == baseline.final_total);
    printf("[PASS] default disabled: base=%u final=%u unchanged\n",
           (unsigned)disabled.base_total, (unsigned)disabled.final_total);
    ee_feedback_attention_clear();
}

static void test_accepted_target_only_and_expiry(void)
{
    EE_GlobalTargetDescriptor target_three = make_target(3u, 1u, 90u, 14u, 14u);
    EE_GlobalTargetDescriptor other_target = make_target(4u, 0u, 90u, 40u, 70u);
    EE_FoveaFeedbackState state = accepted_state(7u, 100u, 3u);
    EE_FeedbackAttentionPolicy policy = ee_feedback_attention_pc_demo_policy();
    EE_AttentionConfig attention = ee_attention_default_config();
    EE_AttentionDebug selected;
    EE_AttentionDebug other;

    policy.feedback_bonus = 12u;
    policy.max_abs_adjustment = 8u;
    policy.max_age_frames = 2u;
    bind_feedback(&state, 101u, policy);
    ee_attention_compute_debug(&target_three, &attention, &selected);
    ee_attention_compute_debug(&other_target, &attention, &other);
    assert(selected.feedback_adjustment == 8);
    assert(selected.feedback_reason == EE_FEEDBACK_ATTENTION_ACCEPTED);
    assert(selected.final_total == selected.base_total + 8u);
    assert(other.feedback_adjustment == 0);
    assert(other.feedback_reason == EE_FEEDBACK_ATTENTION_TARGET_MISMATCH);
    printf("[PASS] ACCEPTED target=3 adjustment=+8; target=4 adjustment=0\n");

    bind_feedback(&state, 103u, policy);
    ee_attention_compute_debug(&target_three, &attention, &selected);
    assert(selected.feedback_adjustment == 0);
    assert(selected.feedback_reason == EE_FEEDBACK_ATTENTION_EXPIRED);
    printf("[PASS] accepted feedback expires after max_age_frames=2\n");
    ee_feedback_attention_clear();
}

static void test_rejected_statuses_and_timeout(void)
{
    static const EE_FoveaFeedbackSimCase rejected_cases[] = {
        EE_FOVEA_FEEDBACK_SIM_STALE,
        EE_FOVEA_FEEDBACK_SIM_REQUEST_MISMATCH,
        EE_FOVEA_FEEDBACK_SIM_FRAME_MISMATCH,
        EE_FOVEA_FEEDBACK_SIM_TARGET_MISMATCH,
        EE_FOVEA_FEEDBACK_SIM_INVALID
    };
    EE_GlobalTargetDescriptor target = make_target(3u, 1u, 90u, 14u, 14u);
    EE_AttentionConfig attention = ee_attention_default_config();
    EE_FeedbackAttentionPolicy policy = ee_feedback_attention_pc_demo_policy();
    uint8_t i;

    for (i = 0u; i < (uint8_t)(sizeof(rejected_cases) / sizeof(rejected_cases[0])); ++i) {
        EE_FoveaFeedbackState state;
        EE_FoveaRequest request = make_request(10u, 100u, 3u, 4u);
        EE_FoveaFeedback feedback;
        EE_AttentionDebug debug;

        ee_fovea_feedback_init(&state);
        assert(ee_fovea_feedback_begin(&state, &request) == 1u);
        assert(ee_fovea_feedback_simulate(&request, rejected_cases[i], &feedback) == 1u);
        assert(ee_fovea_feedback_receive(&state, &feedback, 101u) == 0u);
        bind_feedback(&state, 101u, policy);
        ee_attention_compute_debug(&target, &attention, &debug);
        assert(debug.feedback_adjustment == 0);
        assert(debug.feedback_reason == EE_FEEDBACK_ATTENTION_REJECTED);
    }

    {
        EE_FoveaFeedbackState state;
        EE_FoveaRequest request = make_request(11u, 100u, 3u, 2u);
        EE_AttentionDebug debug;

        ee_fovea_feedback_init(&state);
        assert(ee_fovea_feedback_begin(&state, &request) == 1u);
        ee_fovea_feedback_advance(&state, 103u);
        assert(state.status == EE_FOVEA_FEEDBACK_TIMEOUT);
        policy.timeout_penalty = 10u;
        policy.max_abs_adjustment = 6u;
        bind_feedback(&state, 103u, policy);
        ee_attention_compute_debug(&target, &attention, &debug);
        assert(debug.feedback_adjustment == -6);
        assert(debug.feedback_reason == EE_FEEDBACK_ATTENTION_TIMEOUT);
        printf("[PASS] rejected statuses adjustment=0; TIMEOUT target=3 adjustment=-6\n");
    }
    ee_feedback_attention_clear();
}

static void test_bounded_competition_and_hold(void)
{
    EE_GlobalTargetList list;
    EE_GlobalTargetList held_list;
    EE_FoveaFeedbackState state = accepted_state(21u, 100u, 3u);
    EE_FeedbackAttentionPolicy policy = ee_feedback_attention_pc_demo_policy();
    EE_AttentionScheduler scheduler;
    EE_FoveaRequest request;
    EE_AttentionConfig attention = ee_attention_default_config();
    EE_AttentionDebug strong;
    EE_AttentionDebug weak;

    memset(&list, 0, sizeof(list));
    list.frame_id = 101u;
    list.count = 2u;
    list.targets[0] = make_target(1u, 1u, 100u, 14u, 14u);
    list.targets[0].field_zone = EE_FIELD_GOAL_ZONE;
    list.targets[1] = make_target(3u, 0u, 100u, 70u, 100u);
    policy.feedback_bonus = 16u;
    policy.max_abs_adjustment = 16u;
    bind_feedback(&state, 101u, policy);
    ee_attention_compute_debug(&list.targets[0], &attention, &strong);
    ee_attention_compute_debug(&list.targets[1], &attention, &weak);
    assert(strong.base_total > weak.base_total + policy.max_abs_adjustment);
    ee_scheduler_init(&scheduler, ee_scheduler_default_config());
    ee_attention_update_scores_for_scheduler_step(
        list.targets, list.count, &attention, scheduler.wait_frames);
    assert(ee_scheduler_step(&scheduler, &list, &request) == 1u);
    assert(request.target_id == 1u);
    printf("[PASS] strong target base=%u remains selected over weak+feedback=%u\n",
           (unsigned)strong.base_total, (unsigned)weak.final_total);

    /* A close score may be resolved by legal feedback, while the existing
       scheduler's hold logic still guards a subsequent frame. */
    memset(&held_list, 0, sizeof(held_list));
    held_list.frame_id = 101u;
    held_list.count = 2u;
    held_list.targets[0] = make_target(3u, 0u, 155u, 50u, 50u);
    held_list.targets[1] = make_target(4u, 0u, 153u, 50u, 50u);
    ee_scheduler_init(&scheduler, ee_scheduler_default_config());
    ee_attention_update_scores_for_scheduler_step(
        held_list.targets, held_list.count, &attention, scheduler.wait_frames);
    assert(ee_scheduler_step(&scheduler, &held_list, &request) == 1u);
    assert(request.target_id == 3u);
    held_list.frame_id = 102u;
    held_list.targets[1].coarse_conf = 145u;
    ee_attention_update_scores_for_scheduler_step(
        held_list.targets, held_list.count, &attention, scheduler.wait_frames);
    assert(ee_scheduler_step(&scheduler, &held_list, &request) == 1u);
    assert(request.target_id == 3u);
    assert(scheduler.last_decision.switch_reason == EE_SWITCH_REASON_HOLD_ACTIVE);
    printf("[PASS] feedback-aware close competition keeps existing Scheduler hold behavior\n");
    ee_feedback_attention_clear();
}

static void test_b_sample_default_disabled(void)
{
    FILE *file = fopen("examples/b_target_stream_v1_sample.txt", "r");
    EE_BCStreamReader reader;
    EE_BParsedFrame parsed;
    EE_GlobalTargetList baseline_list;
    EE_GlobalTargetList disabled_list;
    EE_AttentionScheduler baseline_scheduler;
    EE_AttentionScheduler disabled_scheduler;
    EE_FoveaFeedbackState state = accepted_state(30u, 1u, 3u);
    EE_FoveaRequest baseline_request;
    EE_FoveaRequest disabled_request;
    EE_AttentionConfig attention = ee_attention_default_config();
    uint8_t frame_count = 0u;

    assert(file != NULL);
    ee_bc_stream_reader_init(&reader, file);
    ee_scheduler_init(&baseline_scheduler, ee_scheduler_default_config());
    ee_scheduler_init(&disabled_scheduler, ee_scheduler_default_config());
    while (ee_bc_stream_read_frame(&reader, &parsed) == 1) {
        assert(ee_bc_adapt_frame(&parsed, &baseline_list) == 1u);
        disabled_list = baseline_list;
        ee_feedback_attention_clear();
        ee_attention_update_scores_for_scheduler_step(
            baseline_list.targets, baseline_list.count, &attention,
            baseline_scheduler.wait_frames);
        assert(ee_scheduler_step(&baseline_scheduler, &baseline_list, &baseline_request) == 1u);
        bind_feedback(&state, disabled_list.frame_id, ee_feedback_attention_default_policy());
        ee_attention_update_scores_for_scheduler_step(
            disabled_list.targets, disabled_list.count, &attention,
            disabled_scheduler.wait_frames);
        assert(ee_scheduler_step(&disabled_scheduler, &disabled_list, &disabled_request) == 1u);
        assert(baseline_request.target_id == disabled_request.target_id);
        assert(baseline_list.count == disabled_list.count);
        for (uint8_t i = 0u; i < baseline_list.count; ++i) {
            assert(baseline_list.targets[i].attention_score ==
                   disabled_list.targets[i].attention_score);
        }
        ++frame_count;
    }
    fclose(file);
    assert(frame_count == 12u);
    printf("[PASS] B 12-frame sample: default disabled scores and selections unchanged\n");
    ee_feedback_attention_clear();
}

int main(void)
{
    test_default_disabled_preserves_base();
    test_accepted_target_only_and_expiry();
    test_rejected_statuses_and_timeout();
    test_bounded_competition_and_hold();
    test_b_sample_default_disabled();
    printf("All feedback-aware Attention tests passed.\n");
    return 0;
}
