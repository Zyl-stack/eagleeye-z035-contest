#include "director_state.h"

#include <string.h>

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

static uint8_t ball_is_valid(const EE_GlobalTargetList *list)
{
    uint8_t i;
    if (!list) return 0u;
    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].is_ball && list->targets[i].valid) return 1u;
    }
    return 0u;
}

void ee_director_init(EE_Director *director, uint8_t handover_frames)
{
    if (!director) return;
    memset(director, 0, sizeof(*director));
    director->state = EE_DIRECTOR_NORMAL_PLAY;
    director->possession_target_id = EE_INVALID_TARGET_ID;
    director->handover_frames = handover_frames == 0u ? 1u : handover_frames;
}

void ee_director_update(EE_Director *director,
                        const EE_GlobalTargetList *list,
                        uint16_t possession_target_id,
                        uint16_t selected_target_id)
{
    const EE_GlobalTargetDescriptor *selected;
    uint8_t ball_valid;
    uint8_t attack_zone;

    if (!director || !list) return;
    director->event = EE_DIRECTOR_EVENT_NONE;
    ball_valid = ball_is_valid(list);
    selected = find_target(list, selected_target_id);
    attack_zone = (uint8_t)(selected && selected->valid &&
                  (selected->field_zone == EE_FIELD_ATTACK_ZONE ||
                   selected->field_zone == EE_FIELD_GOAL_ZONE));

    if (!ball_valid) {
        if (!director->initialized || director->ball_was_valid) {
            director->event = EE_DIRECTOR_EVENT_BALL_LOST;
        }
        director->state = EE_DIRECTOR_BALL_LOST;
        director->ball_was_valid = 0u;
        director->initialized = 1u;
        return;
    }

    if (director->initialized && !director->ball_was_valid) {
        director->event = EE_DIRECTOR_EVENT_BALL_RECOVERED;
    }
    director->ball_was_valid = 1u;
    director->initialized = 1u;

    if (possession_target_id != EE_INVALID_TARGET_ID &&
        director->possession_target_id != EE_INVALID_TARGET_ID &&
        possession_target_id != director->possession_target_id) {
        director->possession_target_id = possession_target_id;
        director->handover_remaining = director->handover_frames;
        director->state = EE_DIRECTOR_PASS_HANDOVER;
        director->event = EE_DIRECTOR_EVENT_PASS_STARTED;
        return;
    }

    if (director->state == EE_DIRECTOR_PASS_HANDOVER &&
        possession_target_id == director->possession_target_id) {
        if (director->handover_remaining > 1u) {
            --director->handover_remaining;
            return;
        }
        director->handover_remaining = 0u;
        director->state = EE_DIRECTOR_BALL_CARRIER_FOCUS;
        director->event = EE_DIRECTOR_EVENT_PASS_COMPLETED;
        return;
    }

    if (possession_target_id != EE_INVALID_TARGET_ID &&
        director->possession_target_id == EE_INVALID_TARGET_ID) {
        director->possession_target_id = possession_target_id;
        director->state = EE_DIRECTOR_BALL_CARRIER_FOCUS;
        if (director->event == EE_DIRECTOR_EVENT_NONE) {
            director->event = EE_DIRECTOR_EVENT_POSSESSION_ACQUIRED;
        }
        return;
    }

    if (attack_zone) {
        if (director->state != EE_DIRECTOR_ATTACK_ZONE &&
            director->event == EE_DIRECTOR_EVENT_NONE) {
            director->event = EE_DIRECTOR_EVENT_ATTACK_ZONE_ENTER;
        }
        director->state = EE_DIRECTOR_ATTACK_ZONE;
    } else if (possession_target_id != EE_INVALID_TARGET_ID) {
        director->possession_target_id = possession_target_id;
        director->state = EE_DIRECTOR_BALL_CARRIER_FOCUS;
    } else {
        director->possession_target_id = EE_INVALID_TARGET_ID;
        director->state = EE_DIRECTOR_NORMAL_PLAY;
    }
}

const char *ee_director_state_to_string(EE_DirectorState state)
{
    switch (state) {
    case EE_DIRECTOR_NORMAL_PLAY: return "NORMAL_PLAY";
    case EE_DIRECTOR_BALL_CARRIER_FOCUS: return "BALL_CARRIER_FOCUS";
    case EE_DIRECTOR_PASS_HANDOVER: return "PASS_HANDOVER";
    case EE_DIRECTOR_ATTACK_ZONE: return "ATTACK_ZONE";
    case EE_DIRECTOR_BALL_LOST: return "BALL_LOST";
    default: return "UNKNOWN";
    }
}

const char *ee_director_event_to_string(EE_DirectorEvent event)
{
    switch (event) {
    case EE_DIRECTOR_EVENT_NONE: return "NONE";
    case EE_DIRECTOR_EVENT_POSSESSION_ACQUIRED: return "POSSESSION_ACQUIRED";
    case EE_DIRECTOR_EVENT_PASS_STARTED: return "PASS_STARTED";
    case EE_DIRECTOR_EVENT_PASS_COMPLETED: return "PASS_COMPLETED";
    case EE_DIRECTOR_EVENT_BALL_LOST: return "BALL_LOST";
    case EE_DIRECTOR_EVENT_BALL_RECOVERED: return "BALL_RECOVERED";
    case EE_DIRECTOR_EVENT_ATTACK_ZONE_ENTER: return "ATTACK_ZONE_ENTER";
    default: return "UNKNOWN";
    }
}
