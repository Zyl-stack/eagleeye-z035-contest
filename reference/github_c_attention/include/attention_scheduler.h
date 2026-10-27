#ifndef ATTENTION_SCHEDULER_H
#define ATTENTION_SCHEDULER_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float switch_margin;
    uint16_t min_hold_frames;
    uint16_t timeout_frames;
    uint8_t default_scale;
    uint8_t safety_margin_px;
} EE_SchedulerConfig;

typedef enum {
    EE_SWITCH_REASON_NONE = 0,
    EE_SWITCH_REASON_FIRST_TARGET,
    EE_SWITCH_REASON_KEEP_CURRENT,
    EE_SWITCH_REASON_HOLD_ACTIVE,
    EE_SWITCH_REASON_MARGIN_NOT_MET,
    EE_SWITCH_REASON_HIGHER_ATTENTION,
    EE_SWITCH_REASON_CURRENT_INVALID,
    EE_SWITCH_REASON_CURRENT_LOST
} EE_SwitchReason;

typedef struct {
    uint16_t selected_target_id;
    uint16_t previous_target_id;
    uint32_t selected_attention_score;
    uint32_t previous_attention_score;
    uint16_t hold_remaining;
    uint8_t switched;
    EE_SwitchReason switch_reason;
} EE_SchedulerDecision;

typedef struct {
    EE_SchedulerConfig cfg;
    uint32_t next_request_id;
    uint16_t locked_target_id;
    uint16_t hold_counter;
    uint16_t wait_frames[EE_MAX_TARGETS];
    EE_FoveaRequest active_request;
    EE_SchedulerDecision last_decision;
} EE_AttentionScheduler;

EE_SchedulerConfig ee_scheduler_default_config(void);
void ee_scheduler_init(EE_AttentionScheduler *scheduler, EE_SchedulerConfig cfg);
/* targets[].attention_score must be prepared by Attention before this call. */
uint8_t ee_scheduler_step(EE_AttentionScheduler *scheduler,
                          EE_GlobalTargetList *list,
                          EE_FoveaRequest *out_request);
uint16_t ee_scheduler_select_target(EE_AttentionScheduler *scheduler,
                                    EE_GlobalTargetDescriptor targets[],
                                    uint8_t target_count);
void ee_scheduler_release(EE_AttentionScheduler *scheduler);
const EE_SchedulerDecision *ee_scheduler_last_decision(const EE_AttentionScheduler *scheduler);
const char *ee_switch_reason_to_string(EE_SwitchReason reason);

#ifdef __cplusplus
}
#endif

#endif
