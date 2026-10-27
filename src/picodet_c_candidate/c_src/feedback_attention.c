#include "feedback_attention.h"

#include <string.h>

static EE_FeedbackAttentionContext g_context;
static uint8_t g_context_bound;

static uint16_t bounded_value(uint16_t value, uint16_t maximum)
{
    return value > maximum ? maximum : value;
}

static uint8_t is_expired(uint32_t source_frame_id, uint32_t current_frame_id,
                          uint16_t maximum_age)
{
    return current_frame_id < source_frame_id ||
           current_frame_id - source_frame_id > (uint32_t)maximum_age;
}

static uint8_t accepted_record_matches_pending(const EE_FoveaFeedbackState *state)
{
    const EE_FoveaFeedback *feedback = &state->latest_accepted;
    const EE_FoveaRequest *request = &state->pending_request;

    return feedback->valid && request->valid &&
           feedback->request_id == request->request_id &&
           feedback->source_frame_id == request->source_frame_id &&
           feedback->target_id == request->target_id;
}

EE_FeedbackAttentionPolicy ee_feedback_attention_default_policy(void)
{
    EE_FeedbackAttentionPolicy policy;

    /* Disabled by default: stable V2 behavior remains bit-for-bit compatible. */
    policy.enable = 0u;
    policy.feedback_bonus = 16u;
    policy.timeout_penalty = 8u;
    policy.max_abs_adjustment = 32u;
    policy.max_age_frames = 4u;
    return policy;
}

EE_FeedbackAttentionPolicy ee_feedback_attention_pc_demo_policy(void)
{
    EE_FeedbackAttentionPolicy policy = ee_feedback_attention_default_policy();
    policy.enable = 1u;
    return policy;
}

void ee_feedback_attention_bind(const EE_FeedbackAttentionContext *context)
{
    if (!context) {
        ee_feedback_attention_clear();
        return;
    }
    g_context = *context;
    g_context_bound = 1u;
}

void ee_feedback_attention_clear(void)
{
    memset(&g_context, 0, sizeof(g_context));
    g_context_bound = 0u;
}

int16_t ee_feedback_attention_adjustment(uint16_t target_id,
                                          EE_FeedbackAttentionReason *out_reason)
{
    const EE_FoveaFeedbackState *state;
    const EE_FeedbackAttentionPolicy *policy;
    uint16_t bounded;

    if (out_reason) {
        *out_reason = EE_FEEDBACK_ATTENTION_NONE;
    }
    if (!g_context_bound || !g_context.feedback_state) {
        return 0;
    }

    policy = &g_context.policy;
    state = g_context.feedback_state;
    if (!policy->enable) {
        if (out_reason) *out_reason = EE_FEEDBACK_ATTENTION_DISABLED;
        return 0;
    }

    if (state->status == EE_FOVEA_FEEDBACK_ACCEPTED) {
        if (!accepted_record_matches_pending(state)) {
            if (out_reason) *out_reason = EE_FEEDBACK_ATTENTION_REJECTED;
            return 0;
        }
        if (target_id != state->latest_accepted.target_id) {
            if (out_reason) *out_reason = EE_FEEDBACK_ATTENTION_TARGET_MISMATCH;
            return 0;
        }
        if (is_expired(state->latest_accepted.source_frame_id,
                       g_context.current_frame_id, policy->max_age_frames)) {
            if (out_reason) *out_reason = EE_FEEDBACK_ATTENTION_EXPIRED;
            return 0;
        }
        bounded = bounded_value(policy->feedback_bonus, policy->max_abs_adjustment);
        if (out_reason) *out_reason = EE_FEEDBACK_ATTENTION_ACCEPTED;
        return bounded > INT16_MAX ? INT16_MAX : (int16_t)bounded;
    }

    if (state->status == EE_FOVEA_FEEDBACK_TIMEOUT) {
        if (!state->pending_request.valid || target_id != state->pending_request.target_id) {
            if (out_reason) *out_reason = EE_FEEDBACK_ATTENTION_TARGET_MISMATCH;
            return 0;
        }
        if (is_expired(state->pending_request.source_frame_id,
                       g_context.current_frame_id, policy->max_age_frames)) {
            if (out_reason) *out_reason = EE_FEEDBACK_ATTENTION_EXPIRED;
            return 0;
        }
        bounded = bounded_value(policy->timeout_penalty, policy->max_abs_adjustment);
        if (out_reason) *out_reason = EE_FEEDBACK_ATTENTION_TIMEOUT;
        /* Keep the signed representation safe even if a future configuration
           supplies a uint16_t limit larger than an int16_t can represent. */
        return (int16_t)-(int16_t)(bounded > INT16_MAX ? INT16_MAX : bounded);
    }

    if (state->status == EE_FOVEA_FEEDBACK_NONE ||
        state->status == EE_FOVEA_FEEDBACK_PENDING) {
        return 0;
    }
    if (out_reason) *out_reason = EE_FEEDBACK_ATTENTION_REJECTED;
    return 0;
}

const char *ee_feedback_attention_reason_to_string(EE_FeedbackAttentionReason reason)
{
    switch (reason) {
    case EE_FEEDBACK_ATTENTION_DISABLED: return "DISABLED";
    case EE_FEEDBACK_ATTENTION_ACCEPTED: return "ACCEPTED";
    case EE_FEEDBACK_ATTENTION_TIMEOUT: return "TIMEOUT";
    case EE_FEEDBACK_ATTENTION_EXPIRED: return "EXPIRED";
    case EE_FEEDBACK_ATTENTION_TARGET_MISMATCH: return "TARGET_MISMATCH";
    case EE_FEEDBACK_ATTENTION_REJECTED: return "REJECTED";
    default: return "NO_FEEDBACK";
    }
}
