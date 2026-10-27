#ifndef PLAYER_ATTENTION_FEATURES_H
#define PLAYER_ATTENTION_FEATURES_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EE_PLAYER_ATTENTION_MAX_PAIRS ((EE_MAX_TARGETS * (EE_MAX_TARGETS - 1u)) / 2u)

typedef enum {
    EE_ATTENTION_MODE_SOCCER_V2 = 0,
    EE_ATTENTION_MODE_PLAYER_V3
} EE_AttentionMode;

/* Tunable baseline values. These are not claimed to be ground-truth optimal. */
typedef struct {
    uint8_t w_motion;
    uint8_t w_interaction;
    uint8_t w_goal_zone;
    uint8_t w_motion_saliency;
    uint8_t w_uncertainty;
    uint8_t w_aging;
    uint16_t player_motion_full_scale;
    uint16_t interaction_radius;
    uint16_t interaction_approach_margin;
    uint8_t interaction_neighbor_points;
    uint8_t interaction_approach_points;
    uint16_t motion_saliency_full_scale;
    uint32_t history_max_frame_gap_frames;
    uint8_t goal_zone_enable;
    uint8_t attack_zone_score;
    uint8_t goal_zone_score;
    uint8_t player_ball_context_enable;
    uint8_t player_ball_context_weight;
    uint8_t player_ball_context_min_confidence;
    uint16_t player_ball_context_radius;
    uint8_t player_ball_context_max_bonus;
} EE_PlayerAttentionConfig;

typedef struct {
    uint16_t target_id;
    uint8_t valid;
    uint8_t motion_score;
    uint8_t interaction_score;
    uint8_t goal_zone_score;
    uint8_t motion_saliency_score;
    uint8_t uncertainty_score;
    uint8_t aging_score;
    uint8_t optional_ball_bonus;
    uint32_t base_attention;
    uint32_t total_attention;
} EE_PlayerAttentionFeatures;

typedef struct {
    uint8_t count;
    EE_PlayerAttentionFeatures features[EE_MAX_TARGETS];
} EE_PlayerAttentionFrame;

typedef struct {
    uint16_t target_id;
    uint32_t frame_id;
    int32_t center_x;
    int32_t center_y;
    int16_t vx;
    int16_t vy;
    uint8_t valid;
} EE_PlayerAttentionObservation;

typedef struct {
    uint16_t target_id_low;
    uint16_t target_id_high;
    uint32_t frame_id;
    uint32_t distance;
    uint8_t valid;
} EE_PlayerAttentionPairHistory;

typedef struct {
    EE_PlayerAttentionObservation observations[EE_MAX_TARGETS];
    EE_PlayerAttentionPairHistory pairs[EE_PLAYER_ATTENTION_MAX_PAIRS];
} EE_PlayerAttentionHistory;

EE_PlayerAttentionConfig ee_player_attention_default_config(void);
const char *ee_attention_mode_to_string(EE_AttentionMode mode);
void ee_player_attention_history_init(EE_PlayerAttentionHistory *history);

/*
 * Compute V3 features and update only PLAYER attention scores. The original
 * B->C descriptors remain structurally unchanged; the frame is diagnostic
 * data for the current list order.
 */
uint8_t ee_player_attention_update_scores_for_scheduler_step(
    EE_GlobalTargetList *list,
    const EE_PlayerAttentionConfig *config,
    EE_PlayerAttentionHistory *history,
    const uint16_t current_wait_frames[EE_MAX_TARGETS],
    EE_PlayerAttentionFrame *out_frame);

/* Build the Scheduler-only V3 candidate view: valid PLAYER targets only. */
uint8_t ee_player_attention_candidate_view(const EE_GlobalTargetList *source_list,
                                           EE_GlobalTargetList *out_candidates);

const EE_PlayerAttentionFeatures *ee_player_attention_find_features(
    const EE_PlayerAttentionFrame *frame, uint16_t target_id);

#ifdef __cplusplus
}
#endif

#endif
