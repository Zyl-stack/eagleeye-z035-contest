#include "c_runtime_log.h"

#include <stdio.h>

static const EE_GlobalTargetDescriptor *find_target(const EE_GlobalTargetList *list,
                                                     uint16_t target_id)
{
    uint8_t i;
    if (!list) return NULL;
    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].target_id == target_id) return &list->targets[i];
    }
    return NULL;
}

void ee_runtime_logger_init(EE_RuntimeLogger *logger, EE_LogLevel level)
{
    if (logger) logger->level = level;
}

void ee_runtime_log_frame(const EE_RuntimeLogger *logger, const EE_RuntimeFrameLog *frame)
{
    const EE_GlobalTargetDescriptor *selected;
    const EE_FoveaRequest *request;
    const EE_SchedulerDecision *decision;

    if (!logger || logger->level == EE_LOG_LEVEL_NONE || !frame || !frame->targets ||
        !frame->request || !frame->decision) return;
    request = frame->request;
    decision = frame->decision;
    selected = find_target(frame->targets, decision->selected_target_id);
    printf("[C_FRAME] frame=%u targets=%u selected=%u type=%s previous=%u switched=%u "
           "reason=%s attention=%u possession=%u/%u request=%u source=%u "
           "roi=(%u,%u,%u,%u) mode=%u hold_remaining=%u timeout=%u stale=%u\n",
           (unsigned)frame->targets->frame_id, frame->targets->count,
           decision->selected_target_id,
           selected ? ee_soccer_target_to_string((EE_SoccerTargetType)selected->target_type) : "NONE",
           decision->previous_target_id, decision->switched,
           ee_switch_reason_to_string(decision->switch_reason),
           (unsigned)decision->selected_attention_score,
           frame->possession_target_id, frame->possession_score,
           (unsigned)request->request_id, (unsigned)request->source_frame_id,
           request->roi_x, request->roi_y, request->roi_w, request->roi_h,
           request->enhance_mode, decision->hold_remaining, frame->timeout,
           frame->stale_result_rejected);

    if (logger->level == EE_LOG_LEVEL_VERBOSE) {
        uint8_t i;
        for (i = 0u; i < frame->targets->count && i < EE_MAX_TARGETS; ++i) {
            const EE_GlobalTargetDescriptor *target = &frame->targets->targets[i];
            printf("  [C_TARGET] id=%u type=%s valid=%u conf=%u attention=%u "
                   "bbox=(%u,%u,%u,%u) velocity=(%d,%d) possession=%u\n",
                   target->target_id,
                   ee_soccer_target_to_string((EE_SoccerTargetType)target->target_type),
                   target->valid, target->coarse_conf, (unsigned)target->attention_score,
                   target->x, target->y, target->w, target->h,
                   target->vx, target->vy, target->possession_score);
        }
    }
}
