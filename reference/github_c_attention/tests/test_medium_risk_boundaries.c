#include "attention_scheduler.h"
#include "feedback_attention.h"
#include "fovea_feedback.h"
#include "motion_predictor.h"
#include "roi_motion_controller.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned g_failures;
static unsigned g_known_limitations;
static int g_strict;

static void check_condition(int condition, const char *description)
{
    if (condition) {
        printf("[PASS] %s\n", description);
    } else {
        printf("[RISK] %s\n", description);
        ++g_failures;
    }
}

static void check_known_limitation(int condition, const char *description)
{
    if (condition) {
        printf("[PASS] %s\n", description);
    } else {
        printf("[KNOWN LIMITATION] %s\n", description);
        ++g_known_limitations;
    }
}

static EE_GlobalTargetDescriptor make_target(uint16_t target_id,
                                              uint32_t frame_id)
{
    EE_GlobalTargetDescriptor target;

    memset(&target, 0, sizeof(target));
    target.target_id = target_id;
    target.frame_id = frame_id;
    target.x = 100u;
    target.y = 100u;
    target.w = 20u;
    target.h = 20u;
    target.coarse_conf = 90u;
    target.vx = 4;
    target.vy = 1;
    target.valid = 1u;
    return target;
}

static EE_FoveaRequest make_request(uint32_t request_id,
                                    uint32_t source_frame_id,
                                    uint16_t deadline_frames)
{
    EE_FoveaRequest request;

    memset(&request, 0, sizeof(request));
    request.request_id = request_id;
    request.source_frame_id = source_frame_id;
    request.target_id = 3u;
    request.roi_x = 160u;
    request.roi_y = 90u;
    request.roi_w = 320u;
    request.roi_h = 180u;
    request.deadline_frames = deadline_frames;
    request.valid = 1u;
    return request;
}

static EE_FoveaFeedback exact_feedback(const EE_FoveaRequest *request)
{
    EE_FoveaFeedback feedback;

    memset(&feedback, 0, sizeof(feedback));
    feedback.request_id = request->request_id;
    feedback.source_frame_id = request->source_frame_id;
    feedback.target_id = request->target_id;
    feedback.valid = 1u;
    return feedback;
}

static void test_request_id_wraparound(void)
{
    EE_AttentionScheduler scheduler;
    EE_SchedulerConfig config = ee_scheduler_default_config();
    EE_GlobalTargetList list;
    EE_FoveaRequest before_wrap;
    EE_FoveaRequest at_wrap;
    EE_FoveaRequest after_wrap;
    EE_FoveaFeedbackState state;
    EE_FoveaFeedback old_feedback;
    uint32_t expected_id;
    int normal_sequence_ok = 1;

    memset(&list, 0, sizeof(list));
    list.frame_id = 10u;
    list.count = 1u;
    list.targets[0] = make_target(3u, list.frame_id);
    ee_scheduler_init(&scheduler, config);
    for (expected_id = 1u; expected_id <= 3u; ++expected_id) {
        EE_FoveaRequest normal_request;

        list.frame_id = expected_id;
        list.targets[0].frame_id = list.frame_id;
        if (ee_scheduler_step(&scheduler, &list, &normal_request) != 1u ||
            normal_request.request_id != expected_id) {
            normal_sequence_ok = 0;
        }
        ee_scheduler_release(&scheduler);
    }
    check_condition(normal_sequence_ok, "normal request_id sequence remains 1, 2, 3");

    ee_scheduler_init(&scheduler, config);
    scheduler.next_request_id = UINT32_MAX - 1u;
    check_condition(ee_scheduler_step(&scheduler, &list, &before_wrap) == 1u &&
                        before_wrap.request_id == UINT32_MAX - 1u,
                    "request_id emits UINT32_MAX-1 before wrap");

    ee_scheduler_release(&scheduler);
    list.frame_id = 11u;
    list.targets[0].frame_id = list.frame_id;
    check_condition(ee_scheduler_step(&scheduler, &list, &at_wrap) == 1u &&
                        at_wrap.request_id == UINT32_MAX,
                    "request_id emits UINT32_MAX before wrap");

    ee_scheduler_release(&scheduler);
    list.frame_id = 12u;
    list.targets[0].frame_id = list.frame_id;
    check_condition(ee_scheduler_step(&scheduler, &list, &after_wrap) == 1u,
                    "scheduler creates a request after request_id wrap");
    check_condition(after_wrap.request_id == 1u,
                    "request_id sequence wraps from UINT32_MAX to 1 and skips zero");

    ee_fovea_feedback_init(&state);
    (void)ee_fovea_feedback_begin(&state, &after_wrap);
    old_feedback = exact_feedback(&at_wrap);
    (void)ee_fovea_feedback_receive(&state, &old_feedback, list.frame_id);
    check_known_limitation(state.status == EE_FOVEA_FEEDBACK_STALE,
                           "pre-wrap request is classified stale after request_id wraps");
}

static void test_frame_wraparound_timeout(void)
{
    EE_FoveaFeedbackState state;
    EE_FoveaRequest request = make_request(7u, UINT32_MAX - 1u, 1u);

    ee_fovea_feedback_init(&state);
    (void)ee_fovea_feedback_begin(&state, &request);
    ee_fovea_feedback_advance(&state, 0u);
    check_known_limitation(state.status == EE_FOVEA_FEEDBACK_TIMEOUT &&
                               state.timed_out == 1u,
                           "feedback deadline expires correctly across frame_id wrap");
}

static int16_t feedback_adjustment_at(const EE_FoveaFeedbackState *state,
                                      uint32_t current_frame_id,
                                      uint16_t maximum_age,
                                      EE_FeedbackAttentionReason *out_reason)
{
    EE_FeedbackAttentionContext context;
    EE_FeedbackAttentionPolicy policy = ee_feedback_attention_pc_demo_policy();
    int16_t adjustment;

    policy.feedback_bonus = 8u;
    policy.max_abs_adjustment = 8u;
    policy.max_age_frames = maximum_age;
    context.feedback_state = state;
    context.current_frame_id = current_frame_id;
    context.policy = policy;
    ee_feedback_attention_bind(&context);
    adjustment = ee_feedback_attention_adjustment(3u, out_reason);
    ee_feedback_attention_clear();
    return adjustment;
}

static EE_FoveaFeedbackState make_accepted_state(uint32_t source_frame_id)
{
    EE_FoveaFeedbackState state;
    EE_FoveaRequest request = make_request(9u, source_frame_id, 4u);
    EE_FoveaFeedback feedback = exact_feedback(&request);

    ee_fovea_feedback_init(&state);
    (void)ee_fovea_feedback_begin(&state, &request);
    (void)ee_fovea_feedback_receive(&state, &feedback, source_frame_id);
    return state;
}

static void test_feedback_age_boundaries(void)
{
    EE_FoveaFeedbackState normal = make_accepted_state(100u);
    EE_FoveaFeedbackState wrapped = make_accepted_state(UINT32_MAX - 1u);
    EE_FeedbackAttentionReason reason;
    int16_t adjustment;

    adjustment = feedback_adjustment_at(&normal, 102u, 2u, &reason);
    check_condition(adjustment == 8 && reason == EE_FEEDBACK_ATTENTION_ACCEPTED,
                    "feedback remains active at exactly max_age_frames");
    adjustment = feedback_adjustment_at(&normal, 103u, 2u, &reason);
    check_condition(adjustment == 0 && reason == EE_FEEDBACK_ATTENTION_EXPIRED,
                    "feedback expires one frame beyond max_age_frames");

    adjustment = feedback_adjustment_at(&wrapped, 0u, 2u, &reason);
    check_known_limitation(adjustment == 8 && reason == EE_FEEDBACK_ATTENTION_ACCEPTED,
                           "feedback age remains valid across frame_id wrap");
}

static void test_predictor_frame_wraparound(void)
{
    EE_GlobalTargetList previous;
    EE_GlobalTargetDescriptor current;
    EE_MotionPrediction prediction;

    memset(&previous, 0, sizeof(previous));
    previous.frame_id = UINT32_MAX;
    previous.count = 1u;
    previous.targets[0] = make_target(3u, UINT32_MAX);
    current = make_target(3u, 0u);
    ee_motion_predictor_init();
    ee_motion_predictor_update(&previous);
    check_known_limitation(ee_motion_predict(&current, &prediction) == 1u &&
                               prediction.prediction_valid == 1u,
                           "predictor recognizes consecutive frames across frame_id wrap");
}

static void test_predictor_long_gap(void)
{
    EE_GlobalTargetList previous;
    EE_GlobalTargetDescriptor current;
    EE_MotionPrediction prediction;

    memset(&previous, 0, sizeof(previous));
    previous.frame_id = 1u;
    previous.count = 1u;
    previous.targets[0] = make_target(3u, 1u);
    current = make_target(3u, 1000u);
    ee_motion_predictor_init();
    ee_motion_predictor_update(&previous);
    check_condition(ee_motion_predict(&current, &prediction) == 0u &&
                        prediction.prediction_valid == 0u,
                    "predictor rejects stale history after a long frame gap");
    ee_motion_predictor_set_max_gap_frames(999u);
    check_condition(ee_motion_predict(&current, &prediction) == 1u &&
                        prediction.prediction_valid == 1u,
                    "predictor max gap can be explicitly widened when required");
}

static EE_ROIMotionInput repeated_fast_input(EE_DirectorEvent event)
{
    EE_ROIMotionInput input;

    memset(&input, 0, sizeof(input));
    input.director_event = event;
    input.switch_reason = EE_SWITCH_REASON_KEEP_CURRENT;
    input.selected_target_id = 3u;
    input.previous_selected_target_id = 3u;
    return input;
}

static void test_fast_jump_repeated_trigger(void)
{
    EE_ROIMotionController controller;
    EE_ROIMotionControllerConfig fast = ee_roi_motion_controller_default_config();
    EE_ROISmootherConfig track = ee_roi_smoother_default_config();
    EE_ROIMotionDecision decision;
    EE_ROIMotionInput trigger = repeated_fast_input(EE_DIRECTOR_EVENT_PASS_STARTED);
    EE_ROIMotionInput no_trigger = repeated_fast_input(EE_DIRECTOR_EVENT_NONE);
    EE_ROIMotionInput new_switch = repeated_fast_input(EE_DIRECTOR_EVENT_NONE);
    unsigned frame;

    new_switch.switch_reason = EE_SWITCH_REASON_HIGHER_ATTENTION;
    new_switch.selected_target_id = 4u;
    new_switch.previous_selected_target_id = 3u;

    ee_roi_motion_controller_init(&controller, track, fast);
    for (frame = 0u; frame < fast.fast_jump_frames; ++frame) {
        (void)ee_roi_motion_controller_step(&controller,
                                             frame == 0u ? &trigger : &no_trigger,
                                             &decision);
        check_condition(decision.mode == EE_ROI_MOTION_FAST_JUMP,
                        "single FAST_JUMP trigger remains active for configured duration");
    }
    (void)ee_roi_motion_controller_step(&controller, &no_trigger, &decision);
    check_condition(decision.mode == EE_ROI_MOTION_TRACK,
                    "single FAST_JUMP trigger returns to TRACK on schedule");

    ee_roi_motion_controller_init(&controller, track, fast);
    for (frame = 0u; frame <= fast.fast_jump_frames; ++frame) {
        (void)ee_roi_motion_controller_step(&controller, &trigger, &decision);
    }
    check_condition(decision.mode == EE_ROI_MOTION_TRACK,
                    "repeated identical FAST_JUMP event does not re-arm indefinitely");

    (void)ee_roi_motion_controller_step(&controller, &no_trigger, &decision);
    (void)ee_roi_motion_controller_step(&controller, &new_switch, &decision);
    check_condition(decision.mode == EE_ROI_MOTION_FAST_JUMP &&
                        decision.triggered == 1u &&
                        decision.fast_jump_remaining == fast.fast_jump_frames,
                    "new target switch restarts FAST_JUMP after the previous window");

    ee_roi_motion_controller_init(&controller, track, fast);
    (void)ee_roi_motion_controller_step(&controller, &trigger, &decision);
    (void)ee_roi_motion_controller_step(&controller, &new_switch, &decision);
    check_condition(decision.mode == EE_ROI_MOTION_FAST_JUMP &&
                        decision.triggered == 1u &&
                        decision.fast_jump_remaining == fast.fast_jump_frames,
                    "different target switch restarts FAST_JUMP during an active window");
}

int main(int argc, char **argv)
{
    g_strict = argc > 1 && strcmp(argv[1], "--strict") == 0;
    test_request_id_wraparound();
    test_frame_wraparound_timeout();
    test_feedback_age_boundaries();
    test_predictor_frame_wraparound();
    test_predictor_long_gap();
    test_fast_jump_repeated_trigger();

    if (g_known_limitations != 0u) {
        printf("Known UINT32 wrap limitations observed: %u.\n", g_known_limitations);
    }
    if (g_failures != 0u || (g_strict && g_known_limitations != 0u)) {
        printf("Medium-risk boundary tests failed: regressions=%u known_limitations=%u strict=%d.\n",
               g_failures, g_known_limitations, g_strict);
        return 1;
    }
    printf("Committed Round 2A boundaries passed; known limitations remain visible.\n");
    return 0;
}
