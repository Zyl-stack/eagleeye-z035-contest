#ifndef C_CONFIG_H
#define C_CONFIG_H

#include "attention.h"
#include "attention_scheduler.h"
#include "motion_predictor.h"
#include "player_attention_features.h"
#include "roi_motion_controller.h"
#include "roi_smoother.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    EE_AttentionConfig attention;
    /* V2 remains the default so existing regressions retain their semantics. */
    EE_AttentionMode attention_mode;
    EE_PlayerAttentionConfig player_attention;
    EE_FeedbackAttentionPolicy feedback_attention;
    EE_SchedulerConfig scheduler;
    EE_ROISmootherConfig roi_smoother;
    /* Existing ROI smoother values remain the TRACK step configuration. */
    EE_ROIMotionControllerConfig roi_motion_controller;
    uint16_t prediction_horizon_frames;
    /* Source-frame delta between consecutive detector updates; legacy default is 1. */
    uint32_t nominal_input_frame_delta;
    /* Explicit stale-history upper bound in source-frame IDs. */
    uint32_t max_prediction_gap_frames;
    EE_LatencyModel latency_model;
    uint32_t high_attention_threshold;
    uint8_t small_target_threshold;
    uint8_t fast_motion_threshold;
    /* Scheduler-only eligibility threshold for new BALL challengers. */
    uint8_t ball_challenger_gate_enable;
    uint8_t ball_challenger_min_confidence;
    uint8_t default_enhance_mode;
    uint8_t high_quality_enhance_mode;
    uint8_t director_handover_frames;
} EE_CConfig;

EE_CConfig ee_c_config_default(void);

#ifdef __cplusplus
}
#endif

#endif
