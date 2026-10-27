#ifndef ATTENTION_H
#define ATTENTION_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t w_zone;
    uint16_t w_motion;
    uint16_t w_approach;
    uint16_t w_small;
    uint16_t w_uncertainty;
} EE_AttentionConfig;

EE_AttentionConfig ee_attention_default_config(void);

uint32_t ee_attention_compute_score(const EE_GlobalTargetDescriptor *target,
                                    const EE_AttentionConfig *config);

void ee_attention_update_scores(EE_GlobalTargetDescriptor targets[],
                                uint8_t target_count,
                                const EE_AttentionConfig *config);

int16_t ee_attention_select_argmax(const EE_GlobalTargetDescriptor targets[],
                                   uint8_t target_count);

#ifdef __cplusplus
}
#endif

#endif
