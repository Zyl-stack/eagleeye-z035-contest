#include "a_fovea_adapter.h"
#include "motion_predictor.h"
#include "roi_motion_controller.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static EE_FoveaRequest make_request(uint16_t target_id, uint16_t x, uint16_t y)
{
    EE_FoveaRequest request;

    memset(&request, 0, sizeof(request));
    request.valid = 1u;
    request.target_id = target_id;
    request.source_frame_id = 2u;
    request.roi_x = x;
    request.roi_y = y;
    request.roi_w = 20u;
    request.roi_h = 20u;
    return request;
}

static EE_ROIMotionInput make_input(EE_DirectorEvent event,
                                    EE_SwitchReason reason,
                                    uint16_t selected,
                                    uint16_t previous)
{
    EE_ROIMotionInput input;

    memset(&input, 0, sizeof(input));
    input.director_event = event;
    input.switch_reason = reason;
    input.selected_target_id = selected;
    input.previous_selected_target_id = previous;
    return input;
}

static EE_FoveaRequest apply_step(EE_ROIMotionController *controller,
                                  EE_ROISmoother *smoother,
                                  EE_ROIMotionInput input,
                                  const EE_FoveaRequest *desired,
                                  EE_ROIMotionDecision *out_decision)
{
    EE_FoveaRequest smoothed;
    EE_FoveaRequest adapted;

    assert(ee_roi_motion_controller_step(controller, &input, out_decision) == 1u);
    ee_roi_smoother_set_config(smoother, out_decision->smoother_config);
    assert(ee_roi_smoother_step(smoother, desired, &smoothed) == 1u);
    assert(ee_a_fovea_adapt_request(&smoothed, &adapted) == 0);
    return adapted;
}

static uint16_t frames_to_reach(uint8_t fast_event)
{
    EE_ROISmootherConfig track = ee_roi_smoother_default_config();
    EE_ROIMotionControllerConfig fast = ee_roi_motion_controller_default_config();
    EE_ROIMotionController controller;
    EE_ROISmoother smoother;
    EE_FoveaRequest start = make_request(11u, 0u, 0u);
    EE_FoveaRequest target = make_request(12u, 620u, 0u);
    EE_ROIMotionDecision decision;
    EE_FoveaRequest output;
    uint16_t frame;

    ee_roi_motion_controller_init(&controller, track, fast);
    ee_roi_smoother_init(&smoother, track);
    output = apply_step(&controller, &smoother,
                        make_input(EE_DIRECTOR_EVENT_NONE, EE_SWITCH_REASON_FIRST_TARGET,
                                   11u, EE_INVALID_TARGET_ID),
                        &start, &decision);
    assert(output.roi_x == 0u);

    for (frame = 1u; frame <= 16u; ++frame) {
        EE_ROIMotionInput input = make_input(
            frame == 1u && fast_event ? EE_DIRECTOR_EVENT_PASS_STARTED : EE_DIRECTOR_EVENT_NONE,
            frame == 1u && fast_event ? EE_SWITCH_REASON_HIGHER_ATTENTION : EE_SWITCH_REASON_KEEP_CURRENT,
            12u, frame == 1u && fast_event ? 11u : 12u);
        output = apply_step(&controller, &smoother, input, &target, &decision);
        if (output.roi_x == A_FOVEA_ROI_X_MAX) {
            return frame;
        }
    }
    return 0u;
}

static void test_track_and_pass(void)
{
    EE_ROISmootherConfig track = ee_roi_smoother_default_config();
    EE_ROIMotionControllerConfig fast = ee_roi_motion_controller_default_config();
    EE_ROIMotionController controller;
    EE_ROISmoother smoother;
    EE_ROIMotionDecision decision;
    EE_FoveaRequest start = make_request(11u, 0u, 0u);
    EE_FoveaRequest target = make_request(11u, 620u, 0u);
    EE_FoveaRequest output;
    uint16_t track_frames;
    uint16_t fast_frames;

    assert(track.max_step_x == 32u && track.max_step_y == 18u);
    assert(fast.fast_jump_max_step_x == 128u && fast.fast_jump_max_step_y == 72u);
    assert(fast.fast_jump_frames == 3u);

    ee_roi_motion_controller_init(&controller, track, fast);
    ee_roi_smoother_init(&smoother, track);
    (void)apply_step(&controller, &smoother,
                     make_input(EE_DIRECTOR_EVENT_NONE, EE_SWITCH_REASON_FIRST_TARGET,
                                11u, EE_INVALID_TARGET_ID),
                     &start, &decision);
    output = apply_step(&controller, &smoother,
                        make_input(EE_DIRECTOR_EVENT_NONE, EE_SWITCH_REASON_KEEP_CURRENT, 11u, 11u),
                        &target, &decision);
    assert(decision.mode == EE_ROI_MOTION_TRACK);
    assert(output.roi_x == 32u);

    track_frames = frames_to_reach(0u);
    fast_frames = frames_to_reach(1u);
    assert(track_frames == 10u);
    assert(fast_frames == 3u);
    assert(fast_frames < track_frames);
    printf("[PASS] track step=(%u,%u), pass fast_jump=(%u,%u) frames track=%u fast=%u\n",
           track.max_step_x, track.max_step_y, fast.fast_jump_max_step_x,
           fast.fast_jump_max_step_y, track_frames, fast_frames);
}

static void test_duration_recovery_and_non_triggers(void)
{
    EE_ROISmootherConfig track = ee_roi_smoother_default_config();
    EE_ROIMotionControllerConfig fast = ee_roi_motion_controller_default_config();
    EE_ROIMotionController controller;
    EE_ROIMotionDecision decision;
    EE_ROIMotionInput input;

    ee_roi_motion_controller_init(&controller, track, fast);
    input = make_input(EE_DIRECTOR_EVENT_BALL_RECOVERED, EE_SWITCH_REASON_KEEP_CURRENT, 1u, 1u);
    assert(ee_roi_motion_controller_step(&controller, &input, &decision) == 1u);
    assert(decision.mode == EE_ROI_MOTION_FAST_JUMP && decision.fast_jump_remaining == 3u);
    input = make_input(EE_DIRECTOR_EVENT_NONE, EE_SWITCH_REASON_KEEP_CURRENT, 1u, 1u);
    assert(ee_roi_motion_controller_step(&controller, &input, &decision) == 1u);
    assert(decision.mode == EE_ROI_MOTION_FAST_JUMP && decision.fast_jump_remaining == 2u);
    assert(ee_roi_motion_controller_step(&controller, &input, &decision) == 1u);
    assert(decision.mode == EE_ROI_MOTION_FAST_JUMP && decision.fast_jump_remaining == 1u);
    assert(ee_roi_motion_controller_step(&controller, &input, &decision) == 1u);
    assert(decision.mode == EE_ROI_MOTION_TRACK);

    ee_roi_motion_controller_init(&controller, track, fast);
    input = make_input(EE_DIRECTOR_EVENT_NONE, EE_SWITCH_REASON_HOLD_ACTIVE, 11u, 11u);
    assert(ee_roi_motion_controller_step(&controller, &input, &decision) == 1u);
    assert(decision.mode == EE_ROI_MOTION_TRACK);
    input.switch_reason = EE_SWITCH_REASON_MARGIN_NOT_MET;
    assert(ee_roi_motion_controller_step(&controller, &input, &decision) == 1u);
    assert(decision.mode == EE_ROI_MOTION_TRACK);
    input.director_event = EE_DIRECTOR_EVENT_ATTACK_ZONE_ENTER;
    assert(ee_roi_motion_controller_step(&controller, &input, &decision) == 1u);
    assert(decision.mode == EE_ROI_MOTION_TRACK);
    input.director_event = EE_DIRECTOR_EVENT_BALL_LOST;
    input.selected_target_id = EE_INVALID_TARGET_ID;
    assert(ee_roi_motion_controller_step(&controller, &input, &decision) == 1u);
    assert(decision.mode == EE_ROI_MOTION_TRACK && decision.fast_jump_remaining == 0u);
    printf("[PASS] recovery fast duration=3; HOLD/MARGIN/ATTACK/BALL_LOST stay TRACK\n");
}

static void test_true_switch_bounds_and_latency(void)
{
    EE_ROISmootherConfig track = ee_roi_smoother_default_config();
    EE_ROIMotionControllerConfig fast = ee_roi_motion_controller_default_config();
    EE_ROIMotionController controller;
    EE_ROISmoother smoother;
    EE_ROIMotionDecision decision;
    EE_FoveaRequest output;
    EE_FoveaRequest initial_request = make_request(1u, 0u, 0u);
    EE_FoveaRequest corner = make_request(2u, 639u, 359u);
    EE_GlobalTargetDescriptor ball;
    EE_GlobalTargetList previous;
    EE_MotionPrediction prediction;
    EE_LatencyModel latency = {1u, 1u, 1u};
    EE_FoveaRequest initial_ball_request;
    EE_FoveaRequest predicted_request;

    ee_roi_motion_controller_init(&controller, track, fast);
    ee_roi_smoother_init(&smoother, track);
    output = apply_step(&controller, &smoother,
                        make_input(EE_DIRECTOR_EVENT_NONE, EE_SWITCH_REASON_FIRST_TARGET,
                                   1u, EE_INVALID_TARGET_ID),
                        &initial_request, &decision);
    (void)output;
    output = apply_step(&controller, &smoother,
                        make_input(EE_DIRECTOR_EVENT_NONE, EE_SWITCH_REASON_HIGHER_ATTENTION, 2u, 1u),
                        &corner, &decision);
    assert(decision.mode == EE_ROI_MOTION_FAST_JUMP && decision.triggered == 1u);
    assert(output.roi_x <= A_FOVEA_ROI_X_MAX && output.roi_y <= A_FOVEA_ROI_Y_MAX);
    assert(output.roi_w == A_FOVEA_ROI_WIDTH && output.roi_h == A_FOVEA_ROI_HEIGHT);

    memset(&ball, 0, sizeof(ball));
    ball.target_id = 1u;
    ball.frame_id = 2u;
    ball.x = 250u;
    ball.y = 150u;
    ball.w = 14u;
    ball.h = 14u;
    ball.vx = 20;
    ball.vy = 5;
    ball.valid = 1u;
    memset(&previous, 0, sizeof(previous));
    previous.frame_id = 1u;
    previous.count = 1u;
    previous.targets[0] = ball;
    previous.targets[0].frame_id = 1u;
    ee_motion_predictor_init();
    ee_motion_predictor_update(&previous);
    assert(ee_motion_predict_with_latency(&ball, 1u, &latency, &prediction) == 1u);
    assert(prediction.predicted_x == 330u && prediction.predicted_y == 170u);
    predicted_request = make_request(1u, prediction.predicted_x, prediction.predicted_y);
    predicted_request.roi_w = ball.w;
    predicted_request.roi_h = ball.h;
    initial_ball_request = make_request(1u, ball.x, ball.y);
    initial_ball_request.roi_w = ball.w;
    initial_ball_request.roi_h = ball.h;

    ee_roi_motion_controller_init(&controller, track, fast);
    ee_roi_smoother_init(&smoother, track);
    (void)apply_step(&controller, &smoother,
                     make_input(EE_DIRECTOR_EVENT_NONE, EE_SWITCH_REASON_FIRST_TARGET,
                                1u, EE_INVALID_TARGET_ID),
                     &initial_ball_request, &decision);
    output = apply_step(&controller, &smoother,
                        make_input(EE_DIRECTOR_EVENT_BALL_RECOVERED, EE_SWITCH_REASON_KEEP_CURRENT,
                                   1u, 1u),
                        &predicted_request, &decision);
    assert(decision.mode == EE_ROI_MOTION_FAST_JUMP);
    assert(output.roi_x == 177u && output.roi_y == 87u);
    printf("[PASS] switch/bounds valid; latency prediction=(%u,%u) fast ROI=(%u,%u)\n",
           prediction.predicted_x, prediction.predicted_y, output.roi_x, output.roi_y);
}

int main(void)
{
    test_track_and_pass();
    test_duration_recovery_and_non_triggers();
    test_true_switch_bounds_and_latency();
    printf("All dual-mode ROI controller tests passed.\n");
    return 0;
}
