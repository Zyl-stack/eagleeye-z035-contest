#include "c_config.h"
#include "decision_snapshot.h"
#include "director_state.h"
#include "enhance_policy.h"
#include "fovea_stats.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static EE_GlobalTargetDescriptor make_target(uint16_t id, uint8_t type,
                                              uint8_t valid, uint16_t x,
                                              uint8_t field_zone)
{
    EE_GlobalTargetDescriptor target;
    memset(&target, 0, sizeof(target));
    target.target_id = id;
    target.frame_id = 1u;
    target.target_type = type;
    target.is_ball = (uint8_t)(type == EE_SOCCER_TARGET_BALL);
    target.valid = valid;
    target.x = x;
    target.y = 100u;
    target.w = target.is_ball ? 12u : 40u;
    target.h = target.is_ball ? 12u : 80u;
    target.coarse_conf = 90u;
    target.field_zone = field_zone;
    return target;
}

static void test_enhance_policy(void)
{
    EE_CConfig config = ee_c_config_default();
    EE_GlobalTargetDescriptor player = make_target(11u, EE_SOCCER_TARGET_PLAYER, 1u, 100u, EE_FIELD_NORMAL);
    EE_AttentionBreakdown breakdown;
    EE_EnhancePolicyInput input;
    EE_EnhanceDecision decision;
    memset(&breakdown, 0, sizeof(breakdown));
    memset(&input, 0, sizeof(input));
    input.possession_target_id = EE_INVALID_TARGET_ID;

    decision = ee_enhance_policy_decide(&input, &config);
    assert(decision.mode == EE_ENHANCE_MODE_BYPASS);
    assert(decision.reason == EE_ENHANCE_REASON_NO_TARGET);

    input.selected_target = &player;
    input.attention = &breakdown;
    decision = ee_enhance_policy_decide(&input, &config);
    assert(decision.mode == EE_ENHANCE_MODE_BILINEAR);
    assert(decision.reason == EE_ENHANCE_REASON_NORMAL);

    breakdown.total_attention = config.high_attention_threshold;
    decision = ee_enhance_policy_decide(&input, &config);
    assert(decision.mode == EE_ENHANCE_MODE_LANCZOS);
    assert(decision.reason == EE_ENHANCE_REASON_HIGH_ATTENTION);

    breakdown.small_target_score = config.small_target_threshold;
    decision = ee_enhance_policy_decide(&input, &config);
    assert(decision.mode == EE_ENHANCE_MODE_LANCZOS);
    assert(decision.reason == EE_ENHANCE_REASON_SMALL_TARGET);

    input.possession_target_id = player.target_id;
    decision = ee_enhance_policy_decide(&input, &config);
    assert(decision.mode == EE_ENHANCE_MODE_LANCZOS);
    assert(decision.reason == EE_ENHANCE_REASON_POSSESSION);

    input.possession_target_id = EE_INVALID_TARGET_ID;
    player.vx = (int16_t)config.fast_motion_threshold;
    decision = ee_enhance_policy_decide(&input, &config);
    assert(decision.mode == EE_ENHANCE_MODE_BILINEAR);
    assert(decision.reason == EE_ENHANCE_REASON_FAST_MOTION);
    assert(decision.mode >= EE_ENHANCE_MODE_BYPASS && decision.mode <= EE_ENHANCE_MODE_LANCZOS);
    printf("Enhance policy cases passed: BYPASS/BILINEAR/LANCZOS and explicit reasons.\n");
}

static void test_director_state(void)
{
    EE_Director director;
    EE_GlobalTargetList list;
    memset(&list, 0, sizeof(list));
    list.frame_id = 1u;
    list.count = 3u;
    list.targets[0] = make_target(1u, EE_SOCCER_TARGET_BALL, 1u, 200u, EE_FIELD_NORMAL);
    list.targets[1] = make_target(11u, EE_SOCCER_TARGET_PLAYER, 1u, 180u, EE_FIELD_NORMAL);
    list.targets[2] = make_target(12u, EE_SOCCER_TARGET_PLAYER, 1u, 300u, EE_FIELD_NORMAL);
    ee_director_init(&director, 1u);

    ee_director_update(&director, &list, EE_INVALID_TARGET_ID, 1u);
    assert(director.state == EE_DIRECTOR_NORMAL_PLAY);

    ee_director_update(&director, &list, 11u, 11u);
    assert(director.state == EE_DIRECTOR_BALL_CARRIER_FOCUS);
    assert(director.event == EE_DIRECTOR_EVENT_POSSESSION_ACQUIRED);

    ee_director_update(&director, &list, 12u, 12u);
    assert(director.state == EE_DIRECTOR_PASS_HANDOVER);
    assert(director.event == EE_DIRECTOR_EVENT_PASS_STARTED);
    ee_director_update(&director, &list, 12u, 12u);
    assert(director.state == EE_DIRECTOR_BALL_CARRIER_FOCUS);
    assert(director.event == EE_DIRECTOR_EVENT_PASS_COMPLETED);

    list.targets[0].valid = 0u;
    ee_director_update(&director, &list, 12u, 12u);
    assert(director.state == EE_DIRECTOR_BALL_LOST);
    assert(director.event == EE_DIRECTOR_EVENT_BALL_LOST);

    list.targets[0].valid = 1u;
    ee_director_update(&director, &list, EE_INVALID_TARGET_ID, 1u);
    assert(director.state == EE_DIRECTOR_NORMAL_PLAY);
    assert(director.event == EE_DIRECTOR_EVENT_BALL_RECOVERED);

    list.targets[1].field_zone = EE_FIELD_GOAL_ZONE;
    ee_director_update(&director, &list, EE_INVALID_TARGET_ID, 11u);
    assert(director.state == EE_DIRECTOR_ATTACK_ZONE);
    assert(director.event == EE_DIRECTOR_EVENT_ATTACK_ZONE_ENTER);
    printf("Director states/events passed, without changing Scheduler selection.\n");
}

static void test_snapshot_and_stats(void)
{
    EE_CConfig config = ee_c_config_default();
    EE_AttentionScheduler scheduler;
    EE_Director director;
    EE_GlobalTargetList list;
    EE_FoveaRequest request;
    EE_DecisionSnapshot snapshot;
    EE_FoveaStats stats;
    EE_AttentionBreakdown breakdown;
    EE_EnhancePolicyInput policy_input;
    EE_EnhanceDecision enhance;

    memset(&list, 0, sizeof(list));
    list.frame_id = 1u;
    list.count = 2u;
    list.targets[0] = make_target(1u, EE_SOCCER_TARGET_BALL, 1u, 210u, EE_FIELD_NORMAL);
    list.targets[1] = make_target(11u, EE_SOCCER_TARGET_PLAYER, 1u, 190u, EE_FIELD_NORMAL);
    ee_scheduler_init(&scheduler, config.scheduler);
    ee_attention_update_scores_for_scheduler_step(
        list.targets, list.count, &config.attention, scheduler.wait_frames);
    assert(ee_scheduler_step(&scheduler, &list, &request) == 1u);
    ee_director_init(&director, config.director_handover_frames);
    ee_director_update(&director, &list, EE_INVALID_TARGET_ID, request.target_id);
    ee_attention_compute_breakdown(&list.targets[0], &config.attention,
                                   scheduler.wait_frames[0], &breakdown);
    policy_input.selected_target = &list.targets[0];
    policy_input.attention = &breakdown;
    policy_input.possession_target_id = EE_INVALID_TARGET_ID;
    enhance = ee_enhance_policy_decide(&policy_input, &config);
    request.enhance_mode = (uint8_t)enhance.mode;
    assert(ee_decision_snapshot_build(&snapshot, &list, &scheduler, &request,
                                      &config.attention, EE_INVALID_TARGET_ID,
                                      &director, enhance) == 1u);
    assert(snapshot.selected_target_id == request.target_id);
    assert(snapshot.attention_total == snapshot.attention.total_attention);
    ee_decision_snapshot_print(&snapshot);

    ee_fovea_stats_init(&stats);
    ee_fovea_stats_update(&stats, &snapshot);
    assert(stats.full_frame_pixels == 230400u);
    assert(stats.roi_pixels == 57600u);
    assert(fabsf(stats.roi_ratio - 0.25f) < 0.0001f);
    assert(fabsf(stats.theoretical_pixel_reduction - 0.75f) < 0.0001f);
    assert(stats.processed_frames == 1u);
    printf("Foveated statistics: ROI=25%%, theoretical pixel workload reduction=75%%.\n");
}

int main(void)
{
    test_enhance_policy();
    test_director_state();
    test_snapshot_and_stats();
    printf("Competition features V2 tests passed.\n");
    return 0;
}
