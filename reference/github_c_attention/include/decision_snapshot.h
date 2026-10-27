#ifndef DECISION_SNAPSHOT_H
#define DECISION_SNAPSHOT_H

#include "attention.h"
#include "attention_scheduler.h"
#include "director_state.h"
#include "enhance_policy.h"
#include "fovea_feedback.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t frame_id;
    uint16_t selected_target_id;
    uint8_t selected_target_type;
    uint16_t previous_target_id;
    uint32_t attention_total;
    uint32_t base_attention;
    int16_t feedback_adjustment;
    uint32_t final_attention;
    EE_FeedbackAttentionReason feedback_attention_reason;
    EE_AttentionBreakdown attention;
    uint16_t possession_target_id;
    EE_SwitchReason switch_reason;
    EE_DirectorState director_state;
    EE_DirectorEvent director_event;
    EE_EnhanceMode enhance_mode;
    EE_EnhanceReason enhance_reason;
    uint16_t roi_x;
    uint16_t roi_y;
    uint16_t roi_w;
    uint16_t roi_h;
    uint32_t request_id;
    uint16_t hold_remaining;
    uint8_t request_valid;
    EE_FoveaFeedbackStatus feedback_status;
    uint32_t feedback_request_id;
    uint32_t feedback_source_frame_id;
    uint16_t feedback_target_id;
    uint32_t feedback_quality_flags;
    uint32_t feedback_age_frames;
} EE_DecisionSnapshot;

uint8_t ee_decision_snapshot_build(EE_DecisionSnapshot *snapshot,
                                   const EE_GlobalTargetList *list,
                                   const EE_AttentionScheduler *scheduler,
                                   const EE_FoveaRequest *request,
                                   const EE_AttentionConfig *attention_config,
                                   uint16_t possession_target_id,
                                   const EE_Director *director,
                                   EE_EnhanceDecision enhance);
void ee_decision_snapshot_attach_feedback(EE_DecisionSnapshot *snapshot,
                                          const EE_FoveaFeedbackState *feedback_state);
void ee_decision_snapshot_print(const EE_DecisionSnapshot *snapshot);

#ifdef __cplusplus
}
#endif

#endif
