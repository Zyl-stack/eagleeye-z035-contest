#ifndef ATTENTION_H
#define ATTENTION_H

#include "attention_config.h"
#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t w_ball;
    uint16_t w_goal;
    uint16_t w_ball_near;
    uint16_t w_possession;
    uint16_t w_aging;
    /* Deprecated road-scene names retained for source compatibility. */
    uint16_t w_zone;
    uint16_t w_motion;
    uint16_t w_approach;
    uint16_t w_small;
    uint16_t w_uncertainty;
} EE_AttentionConfig;

typedef struct {
    uint8_t ball_priority_score;
    uint8_t zone_score;
    uint8_t goal_zone_score;
    uint8_t motion_score;
    uint8_t approach_score;
    uint8_t small_target_score;
    uint8_t uncertainty_score;
    uint8_t ball_proximity_score;
    uint8_t aging_score;
    uint32_t total_attention;
} EE_AttentionDebug;

EE_AttentionConfig ee_attention_default_config(void);

void ee_attention_compute_debug(const EE_GlobalTargetDescriptor *target,
                                const EE_AttentionConfig *config,
                                EE_AttentionDebug *out_debug);
void ee_attention_compute_debug_waiting(const EE_GlobalTargetDescriptor *target,
                                        const EE_AttentionConfig *config,
                                        uint16_t waiting_frames,
                                        EE_AttentionDebug *out_debug);

uint32_t ee_attention_compute_score(const EE_GlobalTargetDescriptor *target,
                                    const EE_AttentionConfig *config);

uint32_t ee_attention_compute_score_waiting(const EE_GlobalTargetDescriptor *target,
                                            const EE_AttentionConfig *config,
                                            uint16_t waiting_frames);

void ee_attention_update_scores(EE_GlobalTargetDescriptor targets[],
                                uint8_t target_count,
                                const EE_AttentionConfig *config);

int16_t ee_attention_select_argmax(const EE_GlobalTargetDescriptor targets[],
                                   uint8_t target_count);

#ifdef __cplusplus
}
#endif

#endif
