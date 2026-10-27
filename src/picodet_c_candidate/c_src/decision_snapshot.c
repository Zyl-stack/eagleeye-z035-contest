#include "decision_snapshot.h"

#include <stdio.h>
#include <string.h>

uint8_t ee_decision_snapshot_build(EE_DecisionSnapshot *snapshot,
                                   const EE_GlobalTargetList *list,
                                   const EE_AttentionScheduler *scheduler,
                                   const EE_FoveaRequest *request,
                                   const EE_AttentionConfig *attention_config,
                                   uint16_t possession_target_id,
                                   const EE_Director *director,
                                   EE_EnhanceDecision enhance)
{
    const EE_SchedulerDecision *decision;
    uint8_t i;

    if (!snapshot || !list || !scheduler || !request || !attention_config || !director) return 0u;
    memset(snapshot, 0, sizeof(*snapshot));
    decision = ee_scheduler_last_decision(scheduler);
    snapshot->frame_id = list->frame_id;
    snapshot->selected_target_id = decision->selected_target_id;
    snapshot->previous_target_id = decision->previous_target_id;
    snapshot->attention_total = decision->selected_attention_score;
    snapshot->possession_target_id = possession_target_id;
    snapshot->switch_reason = decision->switch_reason;
    snapshot->director_state = director->state;
    snapshot->director_event = director->event;
    snapshot->enhance_mode = enhance.mode;
    snapshot->enhance_reason = enhance.reason;
    snapshot->hold_remaining = decision->hold_remaining;
    snapshot->request_valid = request->valid;
    snapshot->request_id = request->valid ? request->request_id : 0u;
    snapshot->roi_x = request->roi_x;
    snapshot->roi_y = request->roi_y;
    snapshot->roi_w = request->roi_w;
    snapshot->roi_h = request->roi_h;

    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].valid &&
            list->targets[i].target_id == snapshot->selected_target_id) {
            snapshot->selected_target_type = list->targets[i].target_type;
            ee_attention_compute_breakdown(&list->targets[i], attention_config,
                                           scheduler->wait_frames[i], &snapshot->attention);
            snapshot->attention_total = snapshot->attention.total_attention;
            snapshot->base_attention = snapshot->attention.base_total;
            snapshot->feedback_adjustment = snapshot->attention.feedback_adjustment;
            snapshot->final_attention = snapshot->attention.final_total;
            snapshot->feedback_attention_reason = snapshot->attention.feedback_reason;
            break;
        }
    }
    return 1u;
}

void ee_decision_snapshot_attach_feedback(EE_DecisionSnapshot *snapshot,
                                          const EE_FoveaFeedbackState *feedback_state)
{
    const EE_FoveaFeedback *record;

    if (!snapshot || !feedback_state) {
        return;
    }

    snapshot->feedback_status = feedback_state->status;
    snapshot->feedback_age_frames = feedback_state->age_frames;
    record = &feedback_state->last_feedback;
    if (feedback_state->status == EE_FOVEA_FEEDBACK_PENDING ||
        feedback_state->status == EE_FOVEA_FEEDBACK_TIMEOUT) {
        snapshot->feedback_request_id = feedback_state->pending_request.request_id;
        snapshot->feedback_source_frame_id = feedback_state->pending_request.source_frame_id;
        snapshot->feedback_target_id = feedback_state->pending_request.target_id;
        snapshot->feedback_quality_flags = 0u;
    } else {
        snapshot->feedback_request_id = record->request_id;
        snapshot->feedback_source_frame_id = record->source_frame_id;
        snapshot->feedback_target_id = record->target_id;
        snapshot->feedback_quality_flags = record->quality_flags;
    }
}

void ee_decision_snapshot_print(const EE_DecisionSnapshot *snapshot)
{
    if (!snapshot) return;
    printf("Frame %u\n", (unsigned)snapshot->frame_id);
    printf("Selected Target: %s #%u\n",
           ee_soccer_target_to_string((EE_SoccerTargetType)snapshot->selected_target_type),
           snapshot->selected_target_id);
    printf("Base Attention: %u\n", (unsigned)snapshot->base_attention);
    printf("Feedback Influence: %+d (%s)\n", snapshot->feedback_adjustment,
           ee_feedback_attention_reason_to_string(snapshot->feedback_attention_reason));
    printf("Final Attention: %u\n", (unsigned)snapshot->final_attention);
    printf("Possession Target: %u\n", snapshot->possession_target_id);
    printf("Switch Reason: %s\n", ee_switch_reason_to_string(snapshot->switch_reason));
    printf("Director State: %s\n", ee_director_state_to_string(snapshot->director_state));
    printf("Director Event: %s\n", ee_director_event_to_string(snapshot->director_event));
    printf("Enhance Mode: %s\n", ee_enhance_mode_to_string(snapshot->enhance_mode));
    printf("Enhance Reason: %s\n", ee_enhance_reason_to_string(snapshot->enhance_reason));
    printf("ROI: (%u,%u,%u,%u)\n", snapshot->roi_x, snapshot->roi_y,
           snapshot->roi_w, snapshot->roi_h);
    printf("Request ID: %u\n", (unsigned)snapshot->request_id);
    printf("Fovea Feedback: %s request=%u frame=%u target=%u quality=0x%08X age=%u\n",
           ee_fovea_feedback_status_to_string(snapshot->feedback_status),
           (unsigned)snapshot->feedback_request_id,
           (unsigned)snapshot->feedback_source_frame_id,
           snapshot->feedback_target_id,
           (unsigned)snapshot->feedback_quality_flags,
           (unsigned)snapshot->feedback_age_frames);
    printf("Attention Breakdown:\n");
    printf("  Ball Priority: %u\n", snapshot->attention.ball_priority_score);
    printf("  Motion: %u\n", snapshot->attention.motion_score);
    printf("  Goal Zone: %u\n", snapshot->attention.goal_zone_score);
    printf("  Small Target: %u\n", snapshot->attention.small_target_score);
    printf("  Uncertainty: %u\n", snapshot->attention.uncertainty_score);
    printf("  Ball Proximity: %u\n", snapshot->attention.ball_proximity_score);
    printf("  Aging: %u\n", snapshot->attention.aging_score);
    printf("  Base Total: %u\n", (unsigned)snapshot->attention.base_total);
    printf("  Feedback: %+d (%s)\n", snapshot->attention.feedback_adjustment,
           ee_feedback_attention_reason_to_string(snapshot->attention.feedback_reason));
    printf("  Final Total: %u\n", (unsigned)snapshot->attention.final_total);
}
