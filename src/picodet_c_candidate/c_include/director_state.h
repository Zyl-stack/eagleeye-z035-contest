#ifndef DIRECTOR_STATE_H
#define DIRECTOR_STATE_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EE_DIRECTOR_NORMAL_PLAY = 0,
    EE_DIRECTOR_BALL_CARRIER_FOCUS,
    EE_DIRECTOR_PASS_HANDOVER,
    EE_DIRECTOR_ATTACK_ZONE,
    EE_DIRECTOR_BALL_LOST
} EE_DirectorState;

typedef enum {
    EE_DIRECTOR_EVENT_NONE = 0,
    EE_DIRECTOR_EVENT_POSSESSION_ACQUIRED,
    EE_DIRECTOR_EVENT_PASS_STARTED,
    EE_DIRECTOR_EVENT_PASS_COMPLETED,
    EE_DIRECTOR_EVENT_BALL_LOST,
    EE_DIRECTOR_EVENT_BALL_RECOVERED,
    EE_DIRECTOR_EVENT_ATTACK_ZONE_ENTER
} EE_DirectorEvent;

typedef struct {
    EE_DirectorState state;
    EE_DirectorEvent event;
    uint16_t possession_target_id;
    uint8_t ball_was_valid;
    uint8_t initialized;
    uint8_t handover_frames;
    uint8_t handover_remaining;
} EE_Director;

void ee_director_init(EE_Director *director, uint8_t handover_frames);
void ee_director_update(EE_Director *director,
                        const EE_GlobalTargetList *list,
                        uint16_t possession_target_id,
                        uint16_t selected_target_id);
const char *ee_director_state_to_string(EE_DirectorState state);
const char *ee_director_event_to_string(EE_DirectorEvent event);

#ifdef __cplusplus
}
#endif

#endif
