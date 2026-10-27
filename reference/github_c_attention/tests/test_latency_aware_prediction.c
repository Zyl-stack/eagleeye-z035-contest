#include "a_fovea_adapter.h"
#include "c_config.h"
#include "motion_predictor.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static EE_GlobalTargetDescriptor make_target(uint16_t id, uint32_t frame_id,
                                             uint16_t x, uint16_t y,
                                             uint16_t w, uint16_t h,
                                             int16_t vx, int16_t vy)
{
    EE_GlobalTargetDescriptor target;

    memset(&target, 0, sizeof(target));
    target.target_id = id;
    target.frame_id = frame_id;
    target.x = x;
    target.y = y;
    target.w = w;
    target.h = h;
    target.vx = vx;
    target.vy = vy;
    target.valid = 1u;
    return target;
}

static void prime_predictor(const EE_GlobalTargetDescriptor *current)
{
    EE_GlobalTargetList previous_list;
    EE_GlobalTargetDescriptor previous = *current;

    assert(current->frame_id > 0u);
    previous.frame_id = current->frame_id - 1u;
    memset(&previous_list, 0, sizeof(previous_list));
    previous_list.frame_id = previous.frame_id;
    previous_list.count = 1u;
    previous_list.targets[0] = previous;
    ee_motion_predictor_init();
    ee_motion_predictor_update(&previous_list);
}

static void prime_predictor_with_delta(const EE_GlobalTargetDescriptor *current, uint32_t source_frame_delta)
{
    EE_GlobalTargetList previous_list;
    EE_GlobalTargetDescriptor previous = *current;

    assert(current->frame_id >= source_frame_delta);
    previous.frame_id = current->frame_id - source_frame_delta;
    memset(&previous_list, 0, sizeof(previous_list));
    previous_list.frame_id = previous.frame_id;
    previous_list.count = 1u;
    previous_list.targets[0] = previous;
    ee_motion_predictor_init();
    ee_motion_predictor_update(&previous_list);
}

static EE_MotionPrediction predict(const EE_GlobalTargetDescriptor *target,
                                   uint16_t base_horizon,
                                   EE_LatencyModel latency)
{
    EE_MotionPrediction prediction;

    prime_predictor(target);
    assert(ee_motion_predict_with_latency(target, base_horizon, &latency, &prediction) == 1u);
    assert(prediction.prediction_valid == 1u);
    return prediction;
}

static EE_FoveaRequest request_from_position(const EE_GlobalTargetDescriptor *target,
                                             uint16_t x, uint16_t y)
{
    EE_FoveaRequest request;

    memset(&request, 0, sizeof(request));
    request.valid = 1u;
    request.target_id = target->target_id;
    request.source_frame_id = target->frame_id;
    request.roi_x = x;
    request.roi_y = y;
    request.roi_w = target->w;
    request.roi_h = target->h;
    return request;
}

static uint32_t roi_center_error(const EE_GlobalTargetDescriptor *target,
                                 const EE_FoveaRequest *desired,
                                 uint16_t actual_x, uint16_t actual_y)
{
    EE_FoveaRequest adapted;
    int32_t expected_center_x;
    int32_t expected_center_y;
    int32_t actual_center_x;
    int32_t actual_center_y;
    int32_t dx;
    int32_t dy;

    assert(ee_a_fovea_adapt_request(desired, &adapted) == 0);
    expected_center_x = (int32_t)actual_x + (int32_t)target->w / 2;
    expected_center_y = (int32_t)actual_y + (int32_t)target->h / 2;
    actual_center_x = (int32_t)adapted.roi_x + (int32_t)adapted.roi_w / 2;
    actual_center_y = (int32_t)adapted.roi_y + (int32_t)adapted.roi_h / 2;
    dx = expected_center_x - actual_center_x;
    dy = expected_center_y - actual_center_y;
    return (uint32_t)(dx < 0 ? -dx : dx) + (uint32_t)(dy < 0 ? -dy : dy);
}

static void test_stationary_target(void)
{
    EE_GlobalTargetDescriptor target = make_target(1u, 2u, 250u, 130u, 20u, 20u, 0, 0);
    EE_LatencyModel latency = {3u, 2u, 1u};
    EE_MotionPrediction prediction = predict(&target, 1u, latency);

    assert(prediction.predicted_x == 250u);
    assert(prediction.predicted_y == 130u);
    assert(prediction.latency_compensation_frames == 6u);
    assert(prediction.prediction_horizon_frames == 7u);
    printf("[PASS] latency stationary target=(%u,%u) horizon=%u latency=%u\n",
           prediction.predicted_x, prediction.predicted_y,
           prediction.prediction_horizon_frames, prediction.latency_compensation_frames);
}

static void test_positive_and_negative_velocity(void)
{
    EE_GlobalTargetDescriptor target = make_target(2u, 2u, 100u, 140u, 20u, 20u, 10, 0);
    EE_LatencyModel delay0 = {0u, 0u, 0u};
    EE_LatencyModel delay1 = {1u, 0u, 0u};
    EE_LatencyModel delay3 = {1u, 1u, 1u};
    EE_MotionPrediction base = predict(&target, 1u, delay0);
    EE_MotionPrediction one = predict(&target, 1u, delay1);
    EE_MotionPrediction three = predict(&target, 1u, delay3);

    assert(base.predicted_x == 110u);
    assert(one.predicted_x == 120u);
    assert(three.predicted_x == 140u);
    printf("[PASS] latency horizontal base=%u delay1=%u delay3=%u\n",
           base.predicted_x, one.predicted_x, three.predicted_x);

    target.target_id = 3u;
    target.vx = -8;
    target.vy = -5;
    base = predict(&target, 1u, delay0);
    three = predict(&target, 1u, delay3);
    assert(base.predicted_x == 92u && base.predicted_y == 135u);
    assert(three.predicted_x == 68u && three.predicted_y == 120u);
    printf("[PASS] latency negative velocity base=(%u,%u) delay3=(%u,%u)\n",
           base.predicted_x, base.predicted_y, three.predicted_x, three.predicted_y);
}

static void test_fast_ball_error_comparison(void)
{
    EE_GlobalTargetDescriptor ball = make_target(1000u, 2u, 250u, 150u, 14u, 14u, 20, 5);
    EE_LatencyModel latency = {1u, 1u, 1u};
    EE_MotionPrediction existing = predict(&ball, 1u, ee_latency_model_default());
    EE_MotionPrediction compensated = predict(&ball, 1u, latency);
    EE_FoveaRequest current_request = request_from_position(&ball, ball.x, ball.y);
    EE_FoveaRequest existing_request = request_from_position(&ball, existing.predicted_x, existing.predicted_y);
    EE_FoveaRequest compensated_request = request_from_position(&ball, compensated.predicted_x,
                                                                 compensated.predicted_y);
    uint16_t future_x = 330u;
    uint16_t future_y = 170u;
    uint32_t current_error = roi_center_error(&ball, &current_request, future_x, future_y);
    uint32_t existing_error = roi_center_error(&ball, &existing_request, future_x, future_y);
    uint32_t compensated_error = roi_center_error(&ball, &compensated_request, future_x, future_y);

    assert(current_error == 100u);
    assert(existing_error == 75u);
    assert(compensated_error == 0u);
    printf("[PASS] fast ball center_error current=%u existing=%u latency_aware=%u\n",
           (unsigned)current_error, (unsigned)existing_error, (unsigned)compensated_error);
}

static void test_sudden_direction_change_limit(void)
{
    EE_GlobalTargetDescriptor ball = make_target(1001u, 2u, 300u, 150u, 14u, 14u, 20, 0);
    EE_LatencyModel latency = {1u, 1u, 1u};
    EE_MotionPrediction existing = predict(&ball, 1u, ee_latency_model_default());
    EE_MotionPrediction compensated = predict(&ball, 1u, latency);
    EE_FoveaRequest current_request = request_from_position(&ball, ball.x, ball.y);
    EE_FoveaRequest existing_request = request_from_position(&ball, existing.predicted_x, existing.predicted_y);
    EE_FoveaRequest compensated_request = request_from_position(&ball, compensated.predicted_x,
                                                                 compensated.predicted_y);
    uint16_t unexpected_future_x = 220u;
    uint32_t current_error = roi_center_error(&ball, &current_request, unexpected_future_x, ball.y);
    uint32_t existing_error = roi_center_error(&ball, &existing_request, unexpected_future_x, ball.y);
    uint32_t compensated_error = roi_center_error(&ball, &compensated_request, unexpected_future_x, ball.y);

    assert(current_error == 80u);
    assert(existing_error == 100u);
    assert(compensated_error == 160u);
    printf("[PASS] sudden turn limitation center_error current=%u existing=%u latency_aware=%u\n",
           (unsigned)current_error, (unsigned)existing_error, (unsigned)compensated_error);
}

static void assert_adapted_roi_in_bounds(const EE_GlobalTargetDescriptor *target,
                                         uint16_t predicted_x, uint16_t predicted_y)
{
    EE_FoveaRequest desired = request_from_position(target, predicted_x, predicted_y);
    EE_FoveaRequest adapted;

    assert(ee_a_fovea_adapt_request(&desired, &adapted) == 0);
    assert(adapted.roi_x <= A_FOVEA_ROI_X_MAX);
    assert(adapted.roi_y <= A_FOVEA_ROI_Y_MAX);
    assert(adapted.roi_w == A_FOVEA_ROI_WIDTH);
    assert(adapted.roi_h == A_FOVEA_ROI_HEIGHT);
}

static void test_boundaries(void)
{
    EE_LatencyModel latency = {1u, 1u, 1u};
    EE_GlobalTargetDescriptor left = make_target(4u, 2u, 2u, 180u, 14u, 14u, -100, 0);
    EE_GlobalTargetDescriptor right = make_target(5u, 2u, 638u, 180u, 14u, 14u, 100, 0);
    EE_GlobalTargetDescriptor top = make_target(6u, 2u, 320u, 2u, 14u, 14u, 0, -100);
    EE_GlobalTargetDescriptor bottom = make_target(7u, 2u, 320u, 358u, 14u, 14u, 0, 100);
    EE_MotionPrediction prediction;

    prediction = predict(&left, 1u, latency);
    assert(prediction.predicted_x == 0u);
    assert_adapted_roi_in_bounds(&left, prediction.predicted_x, prediction.predicted_y);
    prediction = predict(&right, 1u, latency);
    assert_adapted_roi_in_bounds(&right, prediction.predicted_x, prediction.predicted_y);
    prediction = predict(&top, 1u, latency);
    assert(prediction.predicted_y == 0u);
    assert_adapted_roi_in_bounds(&top, prediction.predicted_x, prediction.predicted_y);
    prediction = predict(&bottom, 1u, latency);
    assert_adapted_roi_in_bounds(&bottom, prediction.predicted_x, prediction.predicted_y);
    printf("[PASS] latency boundary ROI x=0..320 y=0..180 w=320 h=180\n");
}

static void test_default_config_preserves_existing_horizon(void)
{
    EE_CConfig config = ee_c_config_default();

    assert(config.prediction_horizon_frames == 1u);
    assert(config.nominal_input_frame_delta == 1u);
    assert(ee_latency_model_total_frames(&config.latency_model) == 0u);
    printf("[PASS] default latency model is configured/estimated at 0 frames\n");
}

static void test_strided_input_cadence(void)
{
    EE_GlobalTargetDescriptor target = make_target(77u, 2u, 100u, 140u, 20u, 20u, 10, -4);
    EE_MotionPrediction prediction;

    prime_predictor_with_delta(&target, 2u);
    assert(ee_motion_predict_with_latency(&target, 1u, NULL, &prediction) == 0u);
    assert(prediction.prediction_valid == 0u);

    ee_motion_predictor_set_nominal_input_frame_delta(2u);
    assert(ee_motion_predict_with_latency(&target, 1u, NULL, &prediction) == 1u);
    assert(prediction.prediction_valid == 1u);
    /* vx/vy are already source-frame velocities: horizon remains one source frame. */
    assert(prediction.predicted_x == 110u && prediction.predicted_y == 136u);

    target.frame_id = 20u;
    prime_predictor_with_delta(&target, 20u);
    ee_motion_predictor_set_nominal_input_frame_delta(2u);
    assert(ee_motion_predict_with_latency(&target, 1u, NULL, &prediction) == 0u);
    assert(prediction.prediction_valid == 0u);
    printf("[PASS] stride-2 cadence accepts delta=2, rejects delta=20, and does not double-scale velocity.\n");
}

int main(void)
{
    test_default_config_preserves_existing_horizon();
    test_stationary_target();
    test_positive_and_negative_velocity();
    test_fast_ball_error_comparison();
    test_sudden_direction_change_limit();
    test_boundaries();
    test_strided_input_cadence();
    printf("All latency-aware prediction tests passed.\n");
    return 0;
}
