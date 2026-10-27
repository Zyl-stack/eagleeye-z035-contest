#ifndef FEEDBACK_ATTENTION_H
#define FEEDBACK_ATTENTION_H

#include "fovea_feedback.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EE_FEEDBACK_ATTENTION_NONE = 0,
    EE_FEEDBACK_ATTENTION_DISABLED,
    EE_FEEDBACK_ATTENTION_ACCEPTED,
    EE_FEEDBACK_ATTENTION_TIMEOUT,
    EE_FEEDBACK_ATTENTION_EXPIRED,
    EE_FEEDBACK_ATTENTION_TARGET_MISMATCH,
    EE_FEEDBACK_ATTENTION_REJECTED
} EE_FeedbackAttentionReason;

typedef struct {
    uint8_t enable;
    uint16_t feedback_bonus;
    uint16_t timeout_penalty;
    uint16_t max_abs_adjustment;
    uint16_t max_age_frames;
} EE_FeedbackAttentionPolicy;

typedef struct {
    const EE_FoveaFeedbackState *feedback_state;
    uint32_t current_frame_id;
    EE_FeedbackAttentionPolicy policy;
} EE_FeedbackAttentionContext;

EE_FeedbackAttentionPolicy ee_feedback_attention_default_policy(void);
EE_FeedbackAttentionPolicy ee_feedback_attention_pc_demo_policy(void);
void ee_feedback_attention_bind(const EE_FeedbackAttentionContext *context);
void ee_feedback_attention_clear(void);
int16_t ee_feedback_attention_adjustment(uint16_t target_id,
                                          EE_FeedbackAttentionReason *out_reason);
const char *ee_feedback_attention_reason_to_string(EE_FeedbackAttentionReason reason);

#ifdef __cplusplus
}
#endif

#endif
