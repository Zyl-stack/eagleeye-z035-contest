#include "attention.h"
#include "attention_scheduler.h"
#include "c_config.h"
#include "c_runtime_log.h"
#include "roi_smoother.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static EE_GlobalTargetDescriptor make_target(uint16_t id, uint32_t frame, uint8_t ball,
                                             uint16_t x, uint16_t y)
{
    EE_GlobalTargetDescriptor target;
    memset(&target, 0, sizeof(target));
    target.target_id = id;
    target.frame_id = frame;
    target.target_type = ball ? EE_SOCCER_TARGET_BALL : EE_SOCCER_TARGET_PLAYER;
    target.is_ball = ball;
    target.x = x;
    target.y = y;
    target.w = ball ? 12u : 42u;
    target.h = ball ? 12u : 88u;
    target.coarse_conf = 85u;
    target.uncertainty = 38u;
    target.field_zone = EE_FIELD_NORMAL;
    target.valid = 1u;
    return target;
}

static EE_FoveaRequest make_desired(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    EE_FoveaRequest request;
    memset(&request, 0, sizeof(request));
    request.valid = 1u;
    request.request_id = 1u;
    request.target_id = 1u;
    request.source_frame_id = 1u;
    request.roi_x = x;
    request.roi_y = y;
    request.roi_w = w;
    request.roi_h = h;
    return request;
}

static uint8_t scheduler_step_with_attention(EE_AttentionScheduler *scheduler,
                                             EE_GlobalTargetList *list,
                                             const EE_AttentionConfig *attention,
                                             EE_FoveaRequest *request)
{
    ee_attention_update_scores_for_scheduler_step(
        list->targets, list->count, attention, scheduler->wait_frames);
    return ee_scheduler_step(scheduler, list, request);
}

static void test_attention_breakdown(void)
{
    EE_GlobalTargetDescriptor target = make_target(3u, 1u, 1u, 200u, 160u);
    EE_AttentionConfig config = ee_attention_default_config();
    EE_AttentionBreakdown breakdown;
    uint32_t score;
    target.vx = 18;
    target.vy = -2;
    score = ee_attention_compute_score_waiting(&target, &config, 7u);
    ee_attention_compute_breakdown(&target, &config, 7u, &breakdown);
    assert(breakdown.total_attention == score);
    printf("[PASS] Attention breakdown total=%u matches production score.\n", (unsigned)score);
}

static void test_scheduler_reasons(void)
{
    EE_GlobalTargetList list;
    EE_AttentionScheduler scheduler;
    EE_SchedulerConfig cfg = ee_scheduler_default_config();
    EE_FoveaRequest request;
    const EE_SchedulerDecision *decision;
    EE_AttentionConfig attention = ee_attention_default_config();
    uint8_t i;

    cfg.min_hold_frames = 2u;
    cfg.switch_margin = 2.0f;
    ee_scheduler_init(&scheduler, cfg);
    memset(&list, 0, sizeof(list));
    list.count = 1u;
    list.targets[0] = make_target(10u, 1u, 0u, 100u, 100u);
    assert(scheduler_step_with_attention(&scheduler, &list, &attention, &request) == 1u);
    decision = ee_scheduler_last_decision(&scheduler);
    assert(decision->switch_reason == EE_SWITCH_REASON_FIRST_TARGET && decision->switched == 1u);

    list.targets[0].frame_id++;
    assert(scheduler_step_with_attention(&scheduler, &list, &attention, &request) == 1u);
    assert(scheduler.last_decision.switch_reason == EE_SWITCH_REASON_KEEP_CURRENT);

    list.count = 2u;
    list.targets[1] = make_target(20u, 2u, 1u, 300u, 180u);
    assert(scheduler_step_with_attention(&scheduler, &list, &attention, &request) == 1u);
    assert(scheduler.last_decision.switch_reason == EE_SWITCH_REASON_HOLD_ACTIVE);
    assert(scheduler_step_with_attention(&scheduler, &list, &attention, &request) == 1u);
    assert(scheduler.last_decision.switch_reason == EE_SWITCH_REASON_MARGIN_NOT_MET);

    scheduler.cfg.switch_margin = 0.01f;
    for (i = 0u; i < 3u && request.target_id != 20u; ++i) {
        assert(scheduler_step_with_attention(&scheduler, &list, &attention, &request) == 1u);
    }
    assert(request.target_id == 20u);
    assert(scheduler.last_decision.switch_reason == EE_SWITCH_REASON_HIGHER_ATTENTION);

    list.targets[1].valid = 0u;
    assert(scheduler_step_with_attention(&scheduler, &list, &attention, &request) == 1u);
    assert(request.target_id == 10u);
    assert(scheduler.last_decision.switch_reason == EE_SWITCH_REASON_CURRENT_INVALID);
    printf("[PASS] Scheduler reasons expose first/keep/hold/margin/higher/current-invalid decisions.\n");
}

static void test_custom_attention_config_controls_scheduler(void)
{
    EE_CConfig config = ee_c_config_default();
    EE_AttentionScheduler scheduler;
    EE_GlobalTargetList list;
    EE_FoveaRequest request;

    memset(&list, 0, sizeof(list));
    list.count = 2u;
    list.targets[0] = make_target(1u, 1u, 1u, 200u, 150u);
    list.targets[1] = make_target(2u, 1u, 0u, 300u, 150u);
    list.targets[1].possession_score = 255u;

    ee_scheduler_init(&scheduler, config.scheduler);
    assert(scheduler_step_with_attention(&scheduler, &list, &config.attention,
                                         &request) == 1u);
    assert(request.target_id == 1u);

    config.attention.w_possession = 10u;
    ee_scheduler_init(&scheduler, config.scheduler);
    assert(scheduler_step_with_attention(&scheduler, &list, &config.attention,
                                         &request) == 1u);
    assert(request.target_id == 2u);
    printf("[PASS] custom EE_CConfig.attention changes the final Scheduler selection.\n");
}

static void assert_fixed_roi(const EE_FoveaRequest *request)
{
    assert(request->roi_x <= EE_ROI_X_MAX);
    assert(request->roi_y <= EE_ROI_Y_MAX);
    assert(request->roi_w == EE_ROI_FIXED_WIDTH);
    assert(request->roi_h == EE_ROI_FIXED_HEIGHT);
}

static void test_roi_smoother(void)
{
    EE_ROISmoother smoother;
    EE_ROISmootherConfig cfg = ee_roi_smoother_default_config();
    EE_FoveaRequest desired;
    EE_FoveaRequest output;
    uint16_t previous_x;
    uint16_t previous_y;

    cfg.max_step_x = 20u;
    cfg.max_step_y = 10u;
    ee_roi_smoother_init(&smoother, cfg);

    desired = make_desired(160u, 90u, 20u, 20u);
    assert(ee_roi_smoother_step(&smoother, &desired, &output) == 1u);
    assert_fixed_roi(&output);
    previous_x = output.roi_x;
    previous_y = output.roi_y;
    assert(ee_roi_smoother_step(&smoother, &desired, &output) == 1u);
    assert(output.roi_x == previous_x && output.roi_y == previous_y);

    desired.roi_x = 170u;
    desired.roi_y = 95u;
    assert(ee_roi_smoother_step(&smoother, &desired, &output) == 1u);
    assert(output.roi_x >= previous_x && output.roi_x - previous_x <= cfg.max_step_x);
    assert(output.roi_y >= previous_y && output.roi_y - previous_y <= cfg.max_step_y);

    previous_x = output.roi_x;
    previous_y = output.roi_y;
    desired = make_desired(620u, 340u, 20u, 20u);
    desired.target_id = 2u;
    assert(ee_roi_smoother_step(&smoother, &desired, &output) == 1u);
    assert(output.roi_x - previous_x <= cfg.max_step_x);
    assert(output.roi_y - previous_y <= cfg.max_step_y);
    assert_fixed_roi(&output);

    desired = make_desired(0u, 0u, 10u, 10u);
    assert(ee_roi_smoother_step(&smoother, &desired, &output) == 1u);
    assert_fixed_roi(&output);
    desired = make_desired(630u, 0u, 10u, 10u);
    assert(ee_roi_smoother_step(&smoother, &desired, &output) == 1u);
    assert_fixed_roi(&output);
    desired = make_desired(0u, 350u, 10u, 10u);
    assert(ee_roi_smoother_step(&smoother, &desired, &output) == 1u);
    assert_fixed_roi(&output);
    printf("[PASS] ROI smoother covers stationary, small/large moves, target switch, and four boundaries.\n");
}

static void test_unified_config_and_disabled_log(void)
{
    EE_CConfig config = ee_c_config_default();
    EE_RuntimeLogger logger;
    EE_RuntimeFrameLog frame;
    memset(&frame, 0, sizeof(frame));
    ee_runtime_logger_init(&logger, EE_LOG_LEVEL_NONE);
    ee_runtime_log_frame(&logger, &frame);
    assert(config.scheduler.min_hold_frames == ee_scheduler_default_config().min_hold_frames);
    assert(config.attention.w_ball == ee_attention_default_config().w_ball);
    assert(config.roi_smoother.max_step_x == ee_roi_smoother_default_config().max_step_x);
    assert(config.prediction_horizon_frames == 1u);
    assert(config.max_prediction_gap_frames == 1u);
    assert(config.nominal_input_frame_delta == 1u);
    printf("[PASS] EE_CConfig preserves current defaults; runtime logging is disable-able.\n");
}

int main(void)
{
    test_attention_breakdown();
    test_scheduler_reasons();
    test_custom_attention_config_controls_scheduler();
    test_roi_smoother();
    test_unified_config_and_disabled_log();
    printf("All C stability tests passed.\n");
    return 0;
}
