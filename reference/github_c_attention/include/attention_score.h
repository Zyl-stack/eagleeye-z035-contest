#ifndef ATTENTION_SCORE_H
#define ATTENTION_SCORE_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float zone;
    float motion;
    float approach;
    float small_target;
    float uncertainty;
    float aging;
} EE_AttentionWeights;

float ee_attention_score_compute(const EE_GlobalTargetDescriptor *target,
                                 const EE_AttentionWeights *weights,
                                 uint16_t waiting_frames);

void ee_attention_score_update_list(EE_GlobalTargetList *list,
                                    const EE_AttentionWeights *weights,
                                    const uint16_t waiting_frames[EE_MAX_TARGETS]);

EE_AttentionWeights ee_attention_default_weights(void);

#ifdef __cplusplus
}
#endif

#endif
