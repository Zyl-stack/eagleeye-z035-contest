#ifndef MOTION_PREDICTOR_H
#define MOTION_PREDICTOR_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* CONFIGURED / ESTIMATED frame delays. They are not hardware measurements. */
    uint16_t processing_delay_frames;
    uint16_t request_delay_frames;
    uint16_t frame_latch_delay_frames;
} EE_LatencyModel;

typedef struct {
    uint16_t target_id;
    uint32_t source_frame_id;
    uint16_t predicted_x;
    uint16_t predicted_y;
    uint16_t prediction_horizon_frames;
    uint16_t latency_compensation_frames;
    uint8_t prediction_valid;
} EE_MotionPrediction;

EE_LatencyModel ee_latency_model_default(void);
uint16_t ee_latency_model_total_frames(const EE_LatencyModel *model);
void ee_motion_predictor_init(void);
/* Source-frame distance between adjacent detection updates; legacy default is 1. */
void ee_motion_predictor_set_nominal_input_frame_delta(uint32_t nominal_input_frame_delta);
/* Explicit upper source-frame delta override for sparse-input/stale-history tests. */
void ee_motion_predictor_set_max_gap_frames(uint32_t max_gap_frames);
void ee_motion_predictor_update(const EE_GlobalTargetList *list);
uint8_t ee_motion_predict(const EE_GlobalTargetDescriptor *target,
                          EE_MotionPrediction *out_prediction);
uint8_t ee_motion_predict_with_latency(const EE_GlobalTargetDescriptor *target,
                                       uint16_t prediction_horizon_frames,
                                       const EE_LatencyModel *latency_model,
                                       EE_MotionPrediction *out_prediction);

#ifdef __cplusplus
}
#endif

#endif
