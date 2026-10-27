#include "attention_scheduler.h"
#include <string.h>

static int16_t select_argmax(const EE_GlobalTargetDescriptor targets[],
                             uint8_t target_count)
{
    uint8_t i;
    int16_t best_index = -1;
    uint32_t best_score = 0u;

    if (!targets) {
        return -1;
    }
    for (i = 0u; i < target_count && i < EE_MAX_TARGETS; ++i) {
        if (!targets[i].valid) {
            continue;
        }
        if (best_index < 0 || targets[i].attention_score > best_score) {
            best_index = (int16_t)i;
            best_score = targets[i].attention_score;
        }
    }
    return best_index;
}

static void set_decision(EE_AttentionScheduler *scheduler,
                         uint16_t previous_target_id,
                         uint16_t selected_target_id,
                         uint32_t previous_score,
                         uint32_t selected_score,
                         uint8_t switched,
                         EE_SwitchReason reason)
{
    scheduler->last_decision.previous_target_id = previous_target_id;
    scheduler->last_decision.selected_target_id = selected_target_id;
    scheduler->last_decision.previous_attention_score = previous_score;
    scheduler->last_decision.selected_attention_score = selected_score;
    scheduler->last_decision.switched = switched;
    scheduler->last_decision.switch_reason = reason;
    scheduler->last_decision.hold_remaining =
        scheduler->hold_counter < scheduler->cfg.min_hold_frames
            ? (uint16_t)(scheduler->cfg.min_hold_frames - scheduler->hold_counter)
            : 0u;
}

EE_SchedulerConfig ee_scheduler_default_config(void)
{
    EE_SchedulerConfig cfg;
    cfg.switch_margin = 0.12f;
    cfg.min_hold_frames = 4u;
    cfg.timeout_frames = 12u;
    cfg.default_scale = 2u;
    cfg.safety_margin_px = 8u;
    return cfg;
}

void ee_scheduler_init(EE_AttentionScheduler *scheduler, EE_SchedulerConfig cfg)
{
    if (!scheduler) {
        return;
    }
    memset(scheduler, 0, sizeof(*scheduler));
    scheduler->cfg = cfg;
    scheduler->next_request_id = 1u;
    scheduler->locked_target_id = EE_INVALID_TARGET_ID;
    scheduler->last_decision.selected_target_id = EE_INVALID_TARGET_ID;
    scheduler->last_decision.previous_target_id = EE_INVALID_TARGET_ID;
}

void ee_scheduler_release(EE_AttentionScheduler *scheduler)
{
    if (!scheduler) {
        return;
    }
    scheduler->locked_target_id = EE_INVALID_TARGET_ID;
    scheduler->hold_counter = 0u;
    scheduler->active_request.valid = 0u;
}

static EE_FoveaRequest make_request(EE_AttentionScheduler *scheduler,
                                    const EE_GlobalTargetDescriptor *target)
{
    EE_FoveaRequest req;
    memset(&req, 0, sizeof(req));
    if (scheduler->next_request_id == 0u) {
        scheduler->next_request_id = 1u;
    }
    req.request_id = scheduler->next_request_id;
    ++scheduler->next_request_id;
    if (scheduler->next_request_id == 0u) {
        scheduler->next_request_id = 1u;
    }
    req.source_frame_id = target->frame_id;
    req.target_id = target->target_id;
    req.valid = 1u;
    req.roi_x = target->x;
    req.roi_y = target->y;
    req.roi_w = target->w;
    req.roi_h = target->h;
    req.enhance_mode = scheduler->cfg.default_scale;
    req.deadline_frames = scheduler->cfg.timeout_frames;
    return req;
}

uint8_t ee_scheduler_step(EE_AttentionScheduler *scheduler,
                          EE_GlobalTargetList *list,
                          EE_FoveaRequest *out_request)
{
    uint8_t i;
    int best_idx = -1;
    uint16_t selected_target_id;
    uint16_t previous_target_id;
    EE_SwitchReason change_reason = EE_SWITCH_REASON_HIGHER_ATTENTION;

    if (!scheduler || !list || !out_request) {
        return 0u;
    }

    memset(out_request, 0, sizeof(*out_request));
    previous_target_id = scheduler->active_request.valid
                             ? scheduler->locked_target_id
                             : EE_INVALID_TARGET_ID;

    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].valid) {
            if (scheduler->wait_frames[i] < UINT16_MAX) {
                scheduler->wait_frames[i]++;
            }
        } else {
            scheduler->wait_frames[i] = 0u;
        }
    }

    selected_target_id = ee_scheduler_select_target(scheduler, list->targets, list->count);
    if (selected_target_id == EE_INVALID_TARGET_ID) {
        set_decision(scheduler, previous_target_id, EE_INVALID_TARGET_ID, 0u, 0u, 0u,
                     scheduler->active_request.valid ? EE_SWITCH_REASON_CURRENT_LOST
                                                     : EE_SWITCH_REASON_NONE);
        ee_scheduler_release(scheduler);
        return 0u;
    }

    best_idx = select_argmax(list->targets, list->count);
    if (best_idx < 0) {
        set_decision(scheduler, previous_target_id, EE_INVALID_TARGET_ID, 0u, 0u, 0u,
                     EE_SWITCH_REASON_NONE);
        ee_scheduler_release(scheduler);
        return 0u;
    }

    if (scheduler->active_request.valid) {
        int current_idx = -1;
        for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
            if (list->targets[i].valid && list->targets[i].target_id == scheduler->locked_target_id) {
                current_idx = (int)i;
                break;
            }
        }
        if (current_idx >= 0) {
            scheduler->hold_counter++;
            if (scheduler->locked_target_id == list->targets[best_idx].target_id) {
                set_decision(scheduler, previous_target_id, scheduler->locked_target_id,
                             list->targets[current_idx].attention_score,
                             list->targets[best_idx].attention_score, 0u,
                             EE_SWITCH_REASON_KEEP_CURRENT);
                *out_request = scheduler->active_request;
                return 1u;
            }
            if (scheduler->hold_counter <= scheduler->cfg.min_hold_frames) {
                set_decision(scheduler, previous_target_id, scheduler->locked_target_id,
                             list->targets[current_idx].attention_score,
                             list->targets[current_idx].attention_score, 0u,
                             EE_SWITCH_REASON_HOLD_ACTIVE);
                *out_request = scheduler->active_request;
                return 1u;
            }
            if ((float)list->targets[best_idx].attention_score <=
                (float)list->targets[current_idx].attention_score * (1.0f + scheduler->cfg.switch_margin)) {
                set_decision(scheduler, previous_target_id, scheduler->locked_target_id,
                             list->targets[current_idx].attention_score,
                             list->targets[current_idx].attention_score, 0u,
                             EE_SWITCH_REASON_MARGIN_NOT_MET);
                *out_request = scheduler->active_request;
                return 1u;
            }
        } else {
            change_reason = EE_SWITCH_REASON_CURRENT_INVALID;
        }
    }

    scheduler->locked_target_id = list->targets[best_idx].target_id;
    scheduler->hold_counter = 0u;
    scheduler->wait_frames[best_idx] = 0u;
    scheduler->active_request = make_request(scheduler, &list->targets[best_idx]);
    *out_request = scheduler->active_request;
    set_decision(scheduler, previous_target_id, scheduler->locked_target_id, 0u,
                 list->targets[best_idx].attention_score, 1u,
                 previous_target_id == EE_INVALID_TARGET_ID
                     ? EE_SWITCH_REASON_FIRST_TARGET
                     : change_reason);
    return 1u;
}

const EE_SchedulerDecision *ee_scheduler_last_decision(const EE_AttentionScheduler *scheduler)
{
    return scheduler ? &scheduler->last_decision : NULL;
}

const char *ee_switch_reason_to_string(EE_SwitchReason reason)
{
    switch (reason) {
    case EE_SWITCH_REASON_FIRST_TARGET: return "FIRST_TARGET";
    case EE_SWITCH_REASON_KEEP_CURRENT: return "KEEP_CURRENT";
    case EE_SWITCH_REASON_HOLD_ACTIVE: return "HOLD_ACTIVE";
    case EE_SWITCH_REASON_MARGIN_NOT_MET: return "MARGIN_NOT_MET";
    case EE_SWITCH_REASON_HIGHER_ATTENTION: return "HIGHER_ATTENTION";
    case EE_SWITCH_REASON_CURRENT_INVALID: return "CURRENT_INVALID";
    case EE_SWITCH_REASON_CURRENT_LOST: return "CURRENT_LOST";
    default: return "NONE";
    }
}

uint16_t ee_scheduler_select_target(EE_AttentionScheduler *scheduler,
                                    EE_GlobalTargetDescriptor targets[],
                                    uint8_t target_count)
{
    int16_t best_idx;

    if (!scheduler || !targets) {
        return EE_INVALID_TARGET_ID;
    }

    best_idx = select_argmax(targets, target_count);
    if (best_idx < 0) {
        return EE_INVALID_TARGET_ID;
    }

    return targets[best_idx].target_id;
}
