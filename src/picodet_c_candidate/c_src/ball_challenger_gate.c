#include "ball_challenger_gate.h"

static uint8_t locked_target_is_valid_ball(const EE_AttentionScheduler *scheduler,
                                           const EE_GlobalTargetList *list)
{
    uint8_t i;

    if (!scheduler || !list || !scheduler->active_request.valid) {
        return 0u;
    }

    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        const EE_GlobalTargetDescriptor *target = &list->targets[i];

        if (target->target_id == scheduler->locked_target_id) {
            return (uint8_t)(target->valid && target->is_ball);
        }
    }
    return 0u;
}

uint8_t ee_ball_challenger_gate_apply(const EE_CConfig *config,
                                      const EE_AttentionScheduler *scheduler,
                                      const EE_GlobalTargetList *source_list,
                                      EE_GlobalTargetList *out_candidates)
{
    uint8_t i;
    uint8_t keep_locked_ball;

    if (!config || !scheduler || !source_list || !out_candidates) {
        return 0u;
    }

    *out_candidates = *source_list;
    if (!config->ball_challenger_gate_enable) {
        return 1u;
    }

    keep_locked_ball = locked_target_is_valid_ball(scheduler, source_list);
    for (i = 0u; i < out_candidates->count && i < EE_MAX_TARGETS; ++i) {
        EE_GlobalTargetDescriptor *candidate = &out_candidates->targets[i];

        if (!candidate->valid || !candidate->is_ball) {
            continue;
        }
        if (keep_locked_ball && candidate->target_id == scheduler->locked_target_id) {
            continue;
        }
        if (candidate->coarse_conf < config->ball_challenger_min_confidence) {
            candidate->valid = 0u;
        }
    }
    return 1u;
}
