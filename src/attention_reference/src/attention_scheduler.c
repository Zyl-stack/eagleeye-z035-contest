#include "attention_scheduler.h"
#include "attention.h"
#include <string.h>

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
    req.request_id = scheduler->next_request_id++;
    req.source_frame_id = target->frame_id;
    req.target_id = target->target_id;
    req.valid = 1u;
    req.roi_x = target->x;
    req.roi_y = target->y;
    req.roi_w = target->w;
    req.roi_h = target->h;
    req.enhance_mode = scheduler->cfg.default_scale;
    return req;
}

uint8_t ee_scheduler_step(EE_AttentionScheduler *scheduler,
                          const EE_GlobalTargetList *list,
                          EE_FoveaRequest *out_request)
{
    uint8_t i;
    int best_idx = -1;

    if (!scheduler || !list || !out_request) {
        return 0u;
    }

    memset(out_request, 0, sizeof(*out_request));

    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].valid) {
            if (scheduler->wait_frames[i] < UINT16_MAX) {
                scheduler->wait_frames[i]++;
            }
        } else {
            scheduler->wait_frames[i] = 0u;
        }
    }

    best_idx = ee_attention_select_argmax(list->targets, list->count);
    if (best_idx < 0) {
        ee_scheduler_release(scheduler);
        return 0u;
    }

    if (scheduler->active_request.valid &&
        scheduler->locked_target_id == list->targets[best_idx].target_id) {
        *out_request = scheduler->active_request;
        return 1u;
    }

    scheduler->locked_target_id = list->targets[best_idx].target_id;
    scheduler->hold_counter = 0u;
    scheduler->wait_frames[best_idx] = 0u;
    scheduler->active_request = make_request(scheduler, &list->targets[best_idx]);
    *out_request = scheduler->active_request;
    return 1u;
}
