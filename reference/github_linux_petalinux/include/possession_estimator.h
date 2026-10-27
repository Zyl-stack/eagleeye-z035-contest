#ifndef POSSESSION_ESTIMATOR_H
#define POSSESSION_ESTIMATOR_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t target_id;
    uint8_t ball_distance_score;
    uint8_t motion_continuity_score;
    uint8_t history_score;
    uint8_t possession_score;
    uint8_t possession_valid;
} EE_PossessionEstimate;

uint8_t ee_possession_estimate(const EE_GlobalTargetDescriptor *player,
                               const EE_GlobalTargetDescriptor *ball,
                               uint16_t previous_target_id,
                               uint8_t previous_score,
                               EE_PossessionEstimate *out_estimate);

void ee_possession_update_list(EE_GlobalTargetList *list,
                               uint16_t previous_target_id,
                               uint8_t previous_score,
                               uint16_t *out_best_target_id,
                               uint8_t *out_best_score);

#ifdef __cplusplus
}
#endif

#endif
