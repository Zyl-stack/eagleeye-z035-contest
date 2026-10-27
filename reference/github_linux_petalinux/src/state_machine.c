#include "state_machine.h"
#include <stdio.h>
#include <string.h>

void ee_state_machine_init(EE_StateMachine *sm)
{
    ee_state_machine_init_with_config(sm, ee_state_machine_default_config());
}

EE_StateMachineConfig ee_state_machine_default_config(void)
{
    EE_StateMachineConfig cfg;
    cfg.enhance_frames = 1u;
    cfg.refine_timeout_frames = 8u;
    cfg.hold_frames = 3u;
    return cfg;
}

void ee_state_machine_init_with_config(EE_StateMachine *sm, EE_StateMachineConfig cfg)
{
    if (!sm) {
        return;
    }
    memset(sm, 0, sizeof(*sm));
    sm->state = EE_STATE_GLOBAL_SCAN;
    sm->selected_target_id = EE_INVALID_TARGET_ID;
    sm->cfg = cfg;
}

static uint8_t has_candidate(const EE_GlobalTargetList *global_list)
{
    uint8_t i;
    if (!global_list) {
        return 0u;
    }
    for (i = 0u; i < global_list->count && i < EE_MAX_TARGETS; ++i) {
        if (global_list->targets[i].valid) {
            return 1u;
        }
    }
    return 0u;
}

EE_SystemState ee_state_machine_step(EE_StateMachine *sm,
                                     const EE_GlobalTargetList *global_list,
                                     const EE_FoveaRequest *request,
                                     const EE_RefinedTargetDescriptor *refined)
{
    EE_SystemState next;

    if (!sm) {
        return EE_STATE_GLOBAL_SCAN;
    }

    if (request && request->valid) {
        sm->selected_target_id = request->target_id;
        sm->selected_request_id = request->request_id;
    }
    if (refined && refined->valid) {
        sm->current_risk = refined->confirmed_risk_level;
    }

    next = sm->state;

    switch (sm->state) {
    case EE_STATE_GLOBAL_SCAN:
        /* Enter: system reset or no valid target. Exit: B reports at least one valid candidate. */
        next = has_candidate(global_list) ? EE_STATE_CANDIDATE : EE_STATE_GLOBAL_SCAN;
        break;
    case EE_STATE_CANDIDATE:
        /* Enter: valid candidates exist. Exit: C scheduler emits a valid Fovea Request. */
        next = (request && request->valid) ? EE_STATE_FOVEA_LOCK : EE_STATE_CANDIDATE;
        break;
    case EE_STATE_FOVEA_LOCK:
        /* Enter: selected target/request is locked at frame boundary. Exit: lock is complete. */
        next = EE_STATE_ENHANCING;
        break;
    case EE_STATE_ENHANCING:
        /* Enter: A starts ROI/Fovea enhancement. Exit: configured enhancement latency expires. */
        next = (sm->state_frames >= sm->cfg.enhance_frames) ? EE_STATE_REFINE_INFER : EE_STATE_ENHANCING;
        break;
    case EE_STATE_REFINE_INFER:
        /* Enter: enhanced ROI is ready for refined inference. Exit: refined result returns or timeout. */
        if (refined && refined->valid) {
            next = EE_STATE_CONFIRMED;
        } else if (sm->state_frames >= sm->cfg.refine_timeout_frames) {
            next = EE_STATE_RELEASE;
        }
        break;
    case EE_STATE_CONFIRMED:
        /* Enter: refined descriptor is available. Exit: risk result has been latched. */
        next = EE_STATE_HOLD;
        break;
    case EE_STATE_HOLD:
        /* Enter: confirmed risk is displayed/held. Exit: minimum hold time expires. */
        next = (sm->state_frames >= sm->cfg.hold_frames) ? EE_STATE_RELEASE : EE_STATE_HOLD;
        break;
    case EE_STATE_RELEASE:
    default:
        /* Enter: Fovea ownership is released. Exit: return to candidate handling or global scan. */
        next = has_candidate(global_list) ? EE_STATE_CANDIDATE : EE_STATE_GLOBAL_SCAN;
        break;
    }

    if (next != sm->state) {
        printf("  state_change: %s -> %s target=%u request=%u\n",
               ee_state_to_string(sm->state),
               ee_state_to_string(next),
               sm->selected_target_id,
               (unsigned)sm->selected_request_id);
        sm->state = next;
        sm->state_frames = 0u;
    } else if (sm->state_frames < UINT16_MAX) {
        sm->state_frames++;
    }

    return sm->state;
}

const char *ee_state_to_string(EE_SystemState state)
{
    switch (state) {
    case EE_STATE_GLOBAL_SCAN: return "GLOBAL_SCAN";
    case EE_STATE_CANDIDATE: return "CANDIDATE";
    case EE_STATE_FOVEA_LOCK: return "FOVEA_LOCK";
    case EE_STATE_ENHANCING: return "ENHANCING";
    case EE_STATE_REFINE_INFER: return "REFINE_INFER";
    case EE_STATE_CONFIRMED: return "CONFIRMED";
    case EE_STATE_HOLD: return "HOLD";
    case EE_STATE_RELEASE: return "RELEASE";
    default: return "UNKNOWN";
    }
}
