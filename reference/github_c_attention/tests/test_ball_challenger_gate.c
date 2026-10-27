#include "attention_scheduler.h"
#include "ball_challenger_gate.h"
#include "c_config.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static EE_GlobalTargetDescriptor make_target(uint16_t target_id,
                                             uint8_t is_ball,
                                             uint8_t confidence)
{
    EE_GlobalTargetDescriptor target;

    memset(&target, 0, sizeof(target));
    target.target_id = target_id;
    target.frame_id = 1u;
    target.x = (uint16_t)(target_id * 10u);
    target.y = 20u;
    target.w = is_ball ? 8u : 32u;
    target.h = is_ball ? 8u : 64u;
    target.coarse_conf = confidence;
    target.valid = 1u;
    target.is_ball = is_ball;
    target.target_type = is_ball ? EE_SOCCER_TARGET_BALL : EE_SOCCER_TARGET_PLAYER;
    return target;
}

static void init_locked_ball_scheduler(EE_AttentionScheduler *scheduler, uint16_t target_id)
{
    EE_SchedulerConfig scheduler_config = ee_scheduler_default_config();

    ee_scheduler_init(scheduler, scheduler_config);
    scheduler->locked_target_id = target_id;
    scheduler->active_request.valid = 1u;
    scheduler->active_request.target_id = target_id;
}

static void test_threshold_and_target_type(void)
{
    EE_CConfig config = ee_c_config_default();
    EE_AttentionScheduler scheduler;
    EE_GlobalTargetList source;
    EE_GlobalTargetList candidates;

    ee_scheduler_init(&scheduler, config.scheduler);
    memset(&source, 0, sizeof(source));
    source.count = 1u;
    source.targets[0] = make_target(20u, 1u, 24u);
    source.targets[0].attention_score = 1234u;
    assert(ee_ball_challenger_gate_apply(&config, &scheduler, &source, &candidates) == 1u);
    assert(candidates.targets[0].valid == 0u);
    assert(source.targets[0].valid == 1u);
    assert(source.targets[0].attention_score == 1234u);

    source.targets[0].coarse_conf = 25u;
    assert(ee_ball_challenger_gate_apply(&config, &scheduler, &source, &candidates) == 1u);
    assert(candidates.targets[0].valid == 1u);

    source.targets[0].coarse_conf = 40u;
    assert(ee_ball_challenger_gate_apply(&config, &scheduler, &source, &candidates) == 1u);
    assert(candidates.targets[0].valid == 1u);

    source.targets[0] = make_target(30u, 0u, 1u);
    assert(ee_ball_challenger_gate_apply(&config, &scheduler, &source, &candidates) == 1u);
    assert(candidates.targets[0].valid == 1u);

    config.ball_challenger_gate_enable = 0u;
    source.targets[0] = make_target(40u, 1u, 1u);
    assert(ee_ball_challenger_gate_apply(&config, &scheduler, &source, &candidates) == 1u);
    assert(candidates.targets[0].valid == 1u);
    assert(source.targets[0].valid == 1u);
}

static void test_incumbent_and_challenger_behavior(void)
{
    EE_CConfig config = ee_c_config_default();
    EE_AttentionScheduler scheduler;
    EE_GlobalTargetList source;
    EE_GlobalTargetList candidates;
    EE_FoveaRequest request;

    init_locked_ball_scheduler(&scheduler, 10u);
    memset(&source, 0, sizeof(source));
    source.count = 1u;
    source.targets[0] = make_target(10u, 1u, 15u);
    assert(ee_ball_challenger_gate_apply(&config, &scheduler, &source, &candidates) == 1u);
    assert(candidates.targets[0].valid == 1u);

    source.count = 2u;
    source.targets[0] = make_target(10u, 1u, 15u);
    source.targets[1] = make_target(11u, 1u, 20u);
    source.targets[1].attention_score = UINT32_MAX;
    assert(ee_ball_challenger_gate_apply(&config, &scheduler, &source, &candidates) == 1u);
    assert(candidates.targets[0].valid == 1u);
    assert(candidates.targets[1].valid == 0u);
    assert(source.targets[1].valid == 1u);
    assert(ee_scheduler_step(&scheduler, &candidates, &request) == 1u);
    assert(ee_scheduler_last_decision(&scheduler)->selected_target_id == 10u);

    source.targets[1].coarse_conf = 25u;
    assert(ee_ball_challenger_gate_apply(&config, &scheduler, &source, &candidates) == 1u);
    assert(candidates.targets[1].valid == 1u);
    assert(ee_scheduler_step(&scheduler, &candidates, &request) == 1u);
    assert(ee_scheduler_last_decision(&scheduler)->selected_target_id == 10u);
    assert(ee_scheduler_last_decision(&scheduler)->switch_reason == EE_SWITCH_REASON_HOLD_ACTIVE ||
           ee_scheduler_last_decision(&scheduler)->switch_reason == EE_SWITCH_REASON_KEEP_CURRENT ||
           ee_scheduler_last_decision(&scheduler)->switch_reason == EE_SWITCH_REASON_MARGIN_NOT_MET);
}

int main(void)
{
    test_threshold_and_target_type();
    test_incumbent_and_challenger_behavior();
    printf("[PASS] BALL challenger gate: threshold, incumbent, player, and Scheduler handoff.\n");
    return 0;
}
