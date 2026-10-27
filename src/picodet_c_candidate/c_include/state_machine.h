#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EE_STATE_GLOBAL_SCAN = 0,
    EE_STATE_CANDIDATE,
    EE_STATE_FOVEA_LOCK,
    EE_STATE_ENHANCING,
    EE_STATE_REFINE_INFER,
    EE_STATE_CONFIRMED,
    EE_STATE_HOLD,
    EE_STATE_RELEASE
} EE_SystemState;

typedef struct {
    uint16_t enhance_frames;
    uint16_t refine_timeout_frames;
    uint16_t hold_frames;
} EE_StateMachineConfig;

typedef struct {
    EE_SystemState state;
    uint16_t state_frames;
    uint16_t selected_target_id;
    uint32_t selected_request_id;
    EE_RiskLevel current_risk;
    EE_StateMachineConfig cfg;
} EE_StateMachine;

void ee_state_machine_init(EE_StateMachine *sm);
void ee_state_machine_init_with_config(EE_StateMachine *sm, EE_StateMachineConfig cfg);
EE_SystemState ee_state_machine_step(EE_StateMachine *sm,
                                     const EE_GlobalTargetList *global_list,
                                     const EE_FoveaRequest *request,
                                     const EE_RefinedTargetDescriptor *refined);
EE_StateMachineConfig ee_state_machine_default_config(void);
const char *ee_state_to_string(EE_SystemState state);

#ifdef __cplusplus
}
#endif

#endif
