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

typedef struct {
    EE_SchedulerConfig cfg;
    uint32_t next_request_id;
    uint16_t locked_target_id;
    uint16_t hold_counter;
    uint16_t wait_frames[EE_MAX_TARGETS];
    EE_FoveaRequest active_request;
} EE_AttentionScheduler;

EE_SchedulerConfig ee_scheduler_default_config(void);
void ee_scheduler_init(EE_AttentionScheduler *scheduler, EE_SchedulerConfig cfg);
uint8_t ee_scheduler_step(EE_AttentionScheduler *scheduler,
                          EE_GlobalTargetList *list,
                          EE_FoveaRequest *out_request);
uint16_t ee_scheduler_select_target(EE_AttentionScheduler *scheduler,
                                    EE_GlobalTargetDescriptor targets[],
                                    uint8_t target_count);
void ee_scheduler_release(EE_AttentionScheduler *scheduler);

#ifdef __cplusplus
}
#endif

#endif
