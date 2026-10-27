#include "motion_predictor.h"
#include <string.h>

static EE_GlobalTargetDescriptor g_previous[EE_MAX_TARGETS];
static uint32_t g_nominal_input_frame_delta = 1u;
static uint32_t g_max_prediction_gap_frames = 1u;

static uint16_t clamp_coordinate(int32_t value)
{
    if (value < 0) return 0u;
    if (value > 65535) return 65535u;
    return (uint16_t)value;
}

EE_LatencyModel ee_latency_model_default(void)
{
    EE_LatencyModel model;

    /* Hardware delay is not measured yet; preserve the existing one-frame path. */
    model.processing_delay_frames = 0u;
    model.request_delay_frames = 0u;
    model.frame_latch_delay_frames = 0u;
    return model;
}

uint16_t ee_latency_model_total_frames(const EE_LatencyModel *model)
{
    uint32_t total;

    if (!model) {
        return 0u;
    }
    total = (uint32_t)model->processing_delay_frames +
            (uint32_t)model->request_delay_frames +
            (uint32_t)model->frame_latch_delay_frames;
    return total > UINT16_MAX ? UINT16_MAX : (uint16_t)total;
}

void ee_motion_predictor_init(void)
{
    memset(g_previous, 0, sizeof(g_previous));
    g_nominal_input_frame_delta = 1u;
    g_max_prediction_gap_frames = 1u;
}

void ee_motion_predictor_set_nominal_input_frame_delta(uint32_t nominal_input_frame_delta)
{
    g_nominal_input_frame_delta = nominal_input_frame_delta == 0u ? 1u : nominal_input_frame_delta;
}

void ee_motion_predictor_set_max_gap_frames(uint32_t max_gap_frames)
{
    g_max_prediction_gap_frames = max_gap_frames;
}

static uint32_t allowed_source_frame_delta(void)
{
    return g_nominal_input_frame_delta > g_max_prediction_gap_frames
               ? g_nominal_input_frame_delta
               : g_max_prediction_gap_frames;
}

void ee_motion_predictor_update(const EE_GlobalTargetList *list)
{
    uint8_t i;
    if (!list) return;
    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].valid) g_previous[i] = list->targets[i];
    }
}

uint8_t ee_motion_predict(const EE_GlobalTargetDescriptor *target,
                          EE_MotionPrediction *out_prediction)
{
    return ee_motion_predict_with_latency(target, 1u, NULL, out_prediction);
}

uint8_t ee_motion_predict_with_latency(const EE_GlobalTargetDescriptor *target,
                                       uint16_t prediction_horizon_frames,
                                       const EE_LatencyModel *latency_model,
                                       EE_MotionPrediction *out_prediction)
{
    uint8_t i;
    uint16_t latency_frames;
    uint32_t total_horizon;

    if (!target || !out_prediction) return 0u;
    memset(out_prediction, 0, sizeof(*out_prediction));
    out_prediction->target_id = target->target_id;
    out_prediction->source_frame_id = target->frame_id;
    latency_frames = ee_latency_model_total_frames(latency_model);
    total_horizon = (uint32_t)prediction_horizon_frames + latency_frames;
    if (total_horizon > UINT16_MAX) {
        total_horizon = UINT16_MAX;
    }
    out_prediction->prediction_horizon_frames = (uint16_t)total_horizon;
    out_prediction->latency_compensation_frames = latency_frames;

    for (i = 0u; i < EE_MAX_TARGETS; ++i) {
        if (g_previous[i].valid && g_previous[i].target_id == target->target_id &&
            g_previous[i].frame_id < target->frame_id &&
            target->frame_id - g_previous[i].frame_id <= allowed_source_frame_delta()) {
            out_prediction->predicted_x = clamp_coordinate(
                (int32_t)target->x + (int32_t)target->vx * (int32_t)total_horizon);
            out_prediction->predicted_y = clamp_coordinate(
                (int32_t)target->y + (int32_t)target->vy * (int32_t)total_horizon);
            out_prediction->prediction_valid = 1u;
            return 1u;
        }
    }
    return 0u;
}
