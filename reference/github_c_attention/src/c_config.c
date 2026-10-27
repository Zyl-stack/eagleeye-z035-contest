#include "c_config.h"

EE_CConfig ee_c_config_default(void)
{
    EE_CConfig config;
    config.attention = ee_attention_default_config();
    config.attention_mode = EE_ATTENTION_MODE_SOCCER_V2;
    config.player_attention = ee_player_attention_default_config();
    config.feedback_attention = ee_feedback_attention_default_policy();
    config.scheduler = ee_scheduler_default_config();
    config.roi_smoother = ee_roi_smoother_default_config();
    config.roi_motion_controller = ee_roi_motion_controller_default_config();
    config.prediction_horizon_frames = 1u;
    config.nominal_input_frame_delta = 1u;
    config.max_prediction_gap_frames = 1u;
    config.latency_model = ee_latency_model_default();
    config.high_attention_threshold = 1500u;
    config.small_target_threshold = 200u;
    config.fast_motion_threshold = 20u;
    config.ball_challenger_gate_enable = 1u;
    config.ball_challenger_min_confidence = 25u;
    config.default_enhance_mode = 1u;
    config.high_quality_enhance_mode = 2u;
    config.director_handover_frames = 1u;
    return config;
}
