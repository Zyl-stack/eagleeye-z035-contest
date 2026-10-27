#ifndef ENHANCE_POLICY_H
#define ENHANCE_POLICY_H

#include "attention.h"
#include "c_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EE_ENHANCE_MODE_BYPASS = 0,
    EE_ENHANCE_MODE_BILINEAR = 1,
    EE_ENHANCE_MODE_LANCZOS = 2
} EE_EnhanceMode;

typedef enum {
    EE_ENHANCE_REASON_NONE = 0,
    EE_ENHANCE_REASON_NO_TARGET,
    EE_ENHANCE_REASON_NORMAL,
    EE_ENHANCE_REASON_HIGH_ATTENTION,
    EE_ENHANCE_REASON_SMALL_TARGET,
    EE_ENHANCE_REASON_POSSESSION,
    EE_ENHANCE_REASON_ATTACK_ZONE,
    EE_ENHANCE_REASON_FAST_MOTION
} EE_EnhanceReason;

typedef struct {
    const EE_GlobalTargetDescriptor *selected_target;
    const EE_AttentionBreakdown *attention;
    uint16_t possession_target_id;
} EE_EnhancePolicyInput;

typedef struct {
    EE_EnhanceMode mode;
    EE_EnhanceReason reason;
} EE_EnhanceDecision;

EE_EnhanceDecision ee_enhance_policy_decide(const EE_EnhancePolicyInput *input,
                                             const EE_CConfig *config);
const char *ee_enhance_mode_to_string(EE_EnhanceMode mode);
const char *ee_enhance_reason_to_string(EE_EnhanceReason reason);

#ifdef __cplusplus
}
#endif

#endif
