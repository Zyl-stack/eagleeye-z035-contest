#ifndef BALL_CHALLENGER_GATE_H
#define BALL_CHALLENGER_GATE_H

#include "attention_scheduler.h"
#include "c_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Build the Scheduler-only candidate view after Attention has populated the
 * source list. The source list, its scores, and frozen B/C fields are never
 * changed. A currently locked, valid BALL remains eligible even when its
 * confidence is below the challenger threshold.
 */
uint8_t ee_ball_challenger_gate_apply(const EE_CConfig *config,
                                      const EE_AttentionScheduler *scheduler,
                                      const EE_GlobalTargetList *source_list,
                                      EE_GlobalTargetList *out_candidates);

#ifdef __cplusplus
}
#endif

#endif
