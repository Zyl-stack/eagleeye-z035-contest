#include "enhance_policy.h"

static uint32_t absolute_i16(int16_t value)
{
    int32_t abs_value = (int32_t)value;

    if (abs_value < 0) {
        abs_value = -abs_value;
    }
    return (uint32_t)abs_value;
}

EE_EnhanceDecision ee_enhance_policy_decide(const EE_EnhancePolicyInput *input,
                                             const EE_CConfig *config)
{
    EE_EnhanceDecision decision = {EE_ENHANCE_MODE_BYPASS, EE_ENHANCE_REASON_NONE};
    const EE_GlobalTargetDescriptor *target;
    uint32_t speed;

    if (!input || !config || !input->selected_target || !input->selected_target->valid) {
        decision.reason = EE_ENHANCE_REASON_NO_TARGET;
        return decision;
    }
    target = input->selected_target;
    speed = absolute_i16(target->vx) + absolute_i16(target->vy);

    /* Fast motion prefers the lower-cost interpolation mode in this policy. */
    if (speed >= config->fast_motion_threshold) {
        decision.mode = EE_ENHANCE_MODE_BILINEAR;
        decision.reason = EE_ENHANCE_REASON_FAST_MOTION;
    } else if (input->possession_target_id == target->target_id) {
        decision.mode = (EE_EnhanceMode)config->high_quality_enhance_mode;
        decision.reason = EE_ENHANCE_REASON_POSSESSION;
    } else if (target->field_zone == EE_FIELD_ATTACK_ZONE ||
               target->field_zone == EE_FIELD_GOAL_ZONE) {
        decision.mode = (EE_EnhanceMode)config->high_quality_enhance_mode;
        decision.reason = EE_ENHANCE_REASON_ATTACK_ZONE;
    } else if (input->attention &&
               input->attention->small_target_score >= config->small_target_threshold) {
        decision.mode = (EE_EnhanceMode)config->high_quality_enhance_mode;
        decision.reason = EE_ENHANCE_REASON_SMALL_TARGET;
    } else if (input->attention &&
               input->attention->total_attention >= config->high_attention_threshold) {
        decision.mode = (EE_EnhanceMode)config->high_quality_enhance_mode;
        decision.reason = EE_ENHANCE_REASON_HIGH_ATTENTION;
    } else {
        decision.mode = (EE_EnhanceMode)config->default_enhance_mode;
        decision.reason = EE_ENHANCE_REASON_NORMAL;
    }
    if (decision.mode > EE_ENHANCE_MODE_LANCZOS) {
        decision.mode = EE_ENHANCE_MODE_BYPASS;
        decision.reason = EE_ENHANCE_REASON_NONE;
    }
    return decision;
}

const char *ee_enhance_mode_to_string(EE_EnhanceMode mode)
{
    switch (mode) {
    case EE_ENHANCE_MODE_BYPASS: return "BYPASS";
    case EE_ENHANCE_MODE_BILINEAR: return "BILINEAR";
    case EE_ENHANCE_MODE_LANCZOS: return "LANCZOS";
    default: return "UNKNOWN";
    }
}

const char *ee_enhance_reason_to_string(EE_EnhanceReason reason)
{
    switch (reason) {
    case EE_ENHANCE_REASON_NONE: return "NONE";
    case EE_ENHANCE_REASON_NO_TARGET: return "NO_TARGET";
    case EE_ENHANCE_REASON_NORMAL: return "NORMAL";
    case EE_ENHANCE_REASON_HIGH_ATTENTION: return "HIGH_ATTENTION";
    case EE_ENHANCE_REASON_SMALL_TARGET: return "SMALL_TARGET";
    case EE_ENHANCE_REASON_POSSESSION: return "POSSESSION";
    case EE_ENHANCE_REASON_ATTACK_ZONE: return "ATTACK_ZONE";
    case EE_ENHANCE_REASON_FAST_MOTION: return "FAST_MOTION";
    default: return "UNKNOWN";
    }
}
