#include "attention_scheduler.h"
#include "c_config.h"
#include "decision_snapshot.h"
#include "director_state.h"
#include "enhance_policy.h"
#include "fovea_stats.h"
#include "possession_estimator.h"
#include "roi_smoother.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    EE_AttentionScheduler scheduler;
    EE_ROISmoother smoother;
    EE_CConfig config;
    EE_Director director;
    EE_FoveaStats stats;
    EE_DecisionSnapshot snapshot;
    EE_EnhanceDecision enhance;
    uint16_t carrier;
    uint8_t carrier_score;
} SceneRuntime;

static EE_GlobalTargetDescriptor target(uint16_t id, uint32_t frame, uint8_t ball,
                                        uint8_t valid, uint16_t x, uint16_t y,
                                        int16_t vx, int16_t vy)
{
    EE_GlobalTargetDescriptor value;
    memset(&value, 0, sizeof(value));
    value.target_id = id;
    value.frame_id = frame;
    value.target_type = ball ? EE_SOCCER_TARGET_BALL : EE_SOCCER_TARGET_PLAYER;
    value.is_ball = ball;
    value.valid = valid;
    value.x = x;
    value.y = y;
    value.w = ball ? 12u : 42u;
    value.h = ball ? 12u : 88u;
    value.coarse_conf = 88u;
    value.uncertainty = 31u;
    value.vx = vx;
    value.vy = vy;
    value.field_zone = EE_FIELD_NORMAL;
    return value;
}

static void runtime_init(SceneRuntime *runtime)
{
    memset(runtime, 0, sizeof(*runtime));
    runtime->config = ee_c_config_default();
    ee_scheduler_init(&runtime->scheduler, runtime->config.scheduler);
    ee_roi_smoother_init(&runtime->smoother, runtime->config.roi_smoother);
    ee_director_init(&runtime->director, runtime->config.director_handover_frames);
    ee_fovea_stats_init(&runtime->stats);
    runtime->carrier = EE_INVALID_TARGET_ID;
}

static EE_FoveaRequest run_frame(const char *scene, SceneRuntime *runtime,
                                 EE_GlobalTargetList *list)
{
    EE_FoveaRequest desired;
    EE_FoveaRequest smooth;
    const EE_SchedulerDecision *decision;
    const EE_GlobalTargetDescriptor *selected = NULL;
    EE_AttentionBreakdown breakdown;
    EE_EnhancePolicyInput policy_input;
    uint8_t i;

    ee_possession_update_list(list, runtime->carrier, runtime->carrier_score,
                              &runtime->carrier, &runtime->carrier_score);
    ee_attention_update_scores_for_scheduler_step(
        list->targets, list->count, &runtime->config.attention,
        runtime->scheduler.wait_frames);
    if (!ee_scheduler_step(&runtime->scheduler, list, &desired)) memset(&desired, 0, sizeof(desired));
    if (desired.valid) {
        assert(ee_roi_smoother_step(&runtime->smoother, &desired, &smooth) == 1u);
    } else {
        memset(&smooth, 0, sizeof(smooth));
    }
    decision = ee_scheduler_last_decision(&runtime->scheduler);
    memset(&breakdown, 0, sizeof(breakdown));
    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].valid && list->targets[i].target_id == decision->selected_target_id) {
            selected = &list->targets[i];
            ee_attention_compute_breakdown(selected, &runtime->config.attention,
                                           runtime->scheduler.wait_frames[i], &breakdown);
            break;
        }
    }
    ee_director_update(&runtime->director, list, runtime->carrier,
                       decision->selected_target_id);
    policy_input.selected_target = selected;
    policy_input.attention = selected ? &breakdown : NULL;
    policy_input.possession_target_id = runtime->carrier;
    runtime->enhance = ee_enhance_policy_decide(&policy_input, &runtime->config);
    smooth.enhance_mode = (uint8_t)runtime->enhance.mode;
    assert(ee_decision_snapshot_build(&runtime->snapshot, list, &runtime->scheduler,
                                      &smooth, &runtime->config.attention, runtime->carrier,
                                      &runtime->director, runtime->enhance) == 1u);
    ee_fovea_stats_update(&runtime->stats, &runtime->snapshot);
    printf("[SCENE %s] frame=%u selected=%u reason=%s possession=%u attention=%u "
           "director=%s event=%s enhance=%s/%s roi=(%u,%u,%u,%u) request=%u\n",
           scene, (unsigned)list->frame_id, decision->selected_target_id,
           ee_switch_reason_to_string(decision->switch_reason), runtime->carrier,
           (unsigned)decision->selected_attention_score,
           ee_director_state_to_string(runtime->director.state),
           ee_director_event_to_string(runtime->director.event),
           ee_enhance_mode_to_string(runtime->enhance.mode),
           ee_enhance_reason_to_string(runtime->enhance.reason), smooth.roi_x, smooth.roi_y,
           smooth.roi_w, smooth.roi_h, (unsigned)smooth.request_id);
    if (smooth.valid) {
        assert(smooth.roi_x <= 320u && smooth.roi_y <= 180u);
        assert(smooth.roi_w == 320u && smooth.roi_h == 180u);
    }
    assert(runtime->enhance.mode <= EE_ENHANCE_MODE_LANCZOS);
    return smooth;
}

static void normal_tracking(void)
{
    SceneRuntime runtime;
    EE_GlobalTargetList list;
    EE_FoveaRequest request;
    runtime_init(&runtime);
    memset(&list, 0, sizeof(list));
    list.frame_id = 1u; list.count = 3u;
    list.targets[0] = target(11u, 1u, 0u, 1u, 180u, 130u, 2, 0);
    list.targets[1] = target(12u, 1u, 0u, 1u, 400u, 130u, 1, 0);
    list.targets[2] = target(3u, 1u, 1u, 1u, 210u, 205u, 3, 0);
    request = run_frame("1 NORMAL_TRACKING", &runtime, &list);
    assert(request.valid && request.target_id != EE_INVALID_TARGET_ID);
}

static void fast_pass(void)
{
    SceneRuntime runtime;
    EE_GlobalTargetList list;
    uint16_t first_carrier;
    runtime_init(&runtime);
    memset(&list, 0, sizeof(list)); list.count = 3u;
    list.frame_id = 1u;
    list.targets[0] = target(11u, 1u, 0u, 1u, 100u, 130u, 4, 0);
    list.targets[1] = target(12u, 1u, 0u, 1u, 420u, 130u, 4, 0);
    list.targets[2] = target(3u, 1u, 1u, 1u, 110u, 205u, 4, 0);
    (void)run_frame("2 FAST_PASS_A", &runtime, &list);
    first_carrier = runtime.carrier;
    list.frame_id = 2u;
    list.targets[0].frame_id = 2u; list.targets[1].frame_id = 2u; list.targets[2].frame_id = 2u;
    list.targets[2].x = 430u;
    (void)run_frame("2 FAST_PASS_B", &runtime, &list);
    assert(first_carrier == 11u && runtime.carrier == 12u);
    assert(runtime.director.state == EE_DIRECTOR_PASS_HANDOVER);
    assert(runtime.director.event == EE_DIRECTOR_EVENT_PASS_STARTED);
    assert(runtime.scheduler.last_decision.switch_reason == EE_SWITCH_REASON_KEEP_CURRENT ||
           runtime.scheduler.last_decision.switch_reason == EE_SWITCH_REASON_HOLD_ACTIVE);
}

static void ball_occlusion(void)
{
    SceneRuntime runtime;
    EE_GlobalTargetList list;
    runtime_init(&runtime);
    memset(&list, 0, sizeof(list)); list.count = 2u; list.frame_id = 1u;
    list.targets[0] = target(11u, 1u, 0u, 1u, 180u, 130u, 2, 0);
    list.targets[1] = target(3u, 1u, 1u, 1u, 210u, 205u, 3, 0);
    (void)run_frame("3 BALL_VISIBLE", &runtime, &list);
    list.frame_id = 2u; list.targets[0].frame_id = 2u; list.targets[1].frame_id = 2u;
    list.targets[1].valid = 0u;
    (void)run_frame("3 BALL_OCCLUDED", &runtime, &list);
    assert(runtime.director.state == EE_DIRECTOR_BALL_LOST);
    assert(runtime.scheduler.active_request.target_id != 3u);
    list.frame_id = 3u; list.targets[0].frame_id = 3u; list.targets[1].frame_id = 3u;
    list.targets[1].valid = 1u;
    (void)run_frame("3 BALL_RECOVERED", &runtime, &list);
    assert(list.targets[1].target_id == 3u);
    assert(runtime.director.event == EE_DIRECTOR_EVENT_BALL_RECOVERED);
}

static void player_occlusion(void)
{
    SceneRuntime runtime;
    EE_GlobalTargetList list;
    runtime_init(&runtime);
    memset(&list, 0, sizeof(list)); list.count = 2u; list.frame_id = 1u;
    list.targets[0] = target(21u, 1u, 0u, 1u, 100u, 100u, 0, 0);
    list.targets[1] = target(22u, 1u, 0u, 1u, 400u, 100u, 0, 0);
    list.targets[0].uncertainty = 200u;
    (void)run_frame("4 PLAYER_VISIBLE", &runtime, &list);
    assert(runtime.scheduler.active_request.target_id == 21u);
    list.frame_id = 2u; list.targets[0].frame_id = 2u; list.targets[1].frame_id = 2u;
    list.targets[0].valid = 0u;
    (void)run_frame("4 PLAYER_OCCLUDED", &runtime, &list);
    assert(runtime.scheduler.active_request.target_id == 22u);
    assert(runtime.scheduler.last_decision.switch_reason == EE_SWITCH_REASON_CURRENT_INVALID);
}

static void competing_players(void)
{
    SceneRuntime runtime;
    EE_GlobalTargetList list;
    runtime_init(&runtime);
    memset(&list, 0, sizeof(list)); list.count = 2u; list.frame_id = 1u;
    list.targets[0] = target(31u, 1u, 0u, 1u, 150u, 120u, 1, 0);
    list.targets[1] = target(32u, 1u, 0u, 1u, 350u, 120u, 1, 0);
    list.targets[0].uncertainty = 80u; list.targets[1].uncertainty = 79u;
    (void)run_frame("5 COMPETING_FIRST", &runtime, &list);
    list.frame_id = 2u; list.targets[0].frame_id = 2u; list.targets[1].frame_id = 2u;
    list.targets[1].uncertainty = 82u;
    (void)run_frame("5 COMPETING_CLOSE", &runtime, &list);
    assert(runtime.scheduler.last_decision.switched == 0u);
}

static void sudden_attention_spike(void)
{
    SceneRuntime runtime;
    EE_GlobalTargetList list;
    uint8_t i;
    runtime_init(&runtime);
    memset(&list, 0, sizeof(list)); list.count = 1u; list.frame_id = 1u;
    list.targets[0] = target(41u, 1u, 0u, 1u, 120u, 120u, 0, 0);
    (void)run_frame("6 SPIKE_BASE", &runtime, &list);
    list.count = 2u; list.targets[1] = target(3u, 2u, 1u, 1u, 400u, 200u, 40, 0);
    for (i = 0u; i < 6u; ++i) {
        list.frame_id = (uint32_t)(2u + i);
        list.targets[0].frame_id = list.frame_id; list.targets[1].frame_id = list.frame_id;
        (void)run_frame("6 SPIKE", &runtime, &list);
    }
    assert(runtime.scheduler.active_request.target_id == 3u);
}

static void ball_lost_too_long(void)
{
    SceneRuntime runtime;
    EE_GlobalTargetList list;
    uint8_t i;
    runtime_init(&runtime);
    memset(&list, 0, sizeof(list)); list.count = 2u;
    list.targets[0] = target(51u, 1u, 0u, 1u, 200u, 120u, 1, 0);
    list.targets[1] = target(3u, 1u, 1u, 1u, 220u, 205u, 2, 0);
    list.frame_id = 1u; (void)run_frame("7 BALL_START", &runtime, &list);
    list.targets[1].valid = 0u;
    for (i = 0u; i < 14u; ++i) {
        list.frame_id = (uint32_t)(2u + i);
        list.targets[0].frame_id = list.frame_id; list.targets[1].frame_id = list.frame_id;
        (void)run_frame("7 BALL_LOST", &runtime, &list);
    }
    assert(runtime.scheduler.active_request.valid == 1u);
    assert(runtime.scheduler.active_request.target_id == 51u);
    assert(runtime.scheduler.wait_frames[0] >= runtime.scheduler.cfg.timeout_frames);
}

static void boundary_motion(void)
{
    SceneRuntime runtime;
    EE_GlobalTargetList list;
    static const uint16_t x[4] = {0u, 630u, 320u, 320u};
    static const uint16_t y[4] = {180u, 180u, 0u, 350u};
    uint8_t i;
    runtime_init(&runtime);
    memset(&list, 0, sizeof(list)); list.count = 1u;
    for (i = 0u; i < 4u; ++i) {
        list.frame_id = (uint32_t)(i + 1u);
        list.targets[0] = target(3u, list.frame_id, 1u, 1u, x[i], y[i], 0, 0);
        (void)run_frame("8 BOUNDARY", &runtime, &list);
    }
    assert(runtime.stats.processed_frames == 4u);
}

int main(void)
{
    normal_tracking();
    fast_pass();
    ball_occlusion();
    player_occlusion();
    competing_players();
    sudden_attention_spike();
    ball_lost_too_long();
    boundary_motion();
    printf("All eight match regression scenes passed.\n");
    return 0;
}
