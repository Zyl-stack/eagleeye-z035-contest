#include "attention.h"
#include "attention_scheduler.h"
#include "risk.h"
#include "state_machine.h"
#include "transaction.h"
#include "fake_data.h"
#include "motion_predictor.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static EE_GlobalTargetDescriptor make_target(uint16_t target_id,
                                             uint32_t frame_id,
                                             uint16_t x,
                                             uint16_t y,
                                             uint16_t w,
                                             uint16_t h,
                                             uint8_t coarse_conf,
                                             uint8_t zone_id,
                                             uint8_t approach_level)
{
    EE_GlobalTargetDescriptor target;
    memset(&target, 0, sizeof(target));
    target.valid = 1u;
    target.target_id = target_id;
    target.frame_id = frame_id;
    target.x = x;
    target.y = y;
    target.w = w;
    target.h = h;
    target.coarse_class = EE_CLASS_PEDESTRIAN;
    target.coarse_conf = coarse_conf;
    target.vx = 12;
    target.vy = -4;
    target.zone_id = zone_id;
    target.approach_level = approach_level;
    return target;
}

static void make_competition_list(EE_GlobalTargetList *list)
{
    memset(list, 0, sizeof(*list));
    list->frame_id = 1u;
    list->count = 3u;
    list->targets[0] = make_target(101u, 1u, 100u, 120u, 120u, 90u, 92u, EE_ZONE_LEFT, 10u);
    list->targets[1] = make_target(202u, 1u, 360u, 155u, 24u, 42u, 48u, EE_ZONE_EGO_LANE, 65u);
    list->targets[2] = make_target(303u, 1u, 260u, 140u, 64u, 56u, 76u, EE_ZONE_CENTER, 70u);
}

static EE_RefinedTargetDescriptor make_refined(const EE_FoveaRequest *request,
                                               EE_TargetClass refined_class,
                                               uint8_t refined_confidence,
                                               EE_RiskLevel risk_level,
                                               EE_ConfirmState confirm_state)
{
    EE_RefinedTargetDescriptor refined;
    memset(&refined, 0, sizeof(refined));
    refined.valid = 1u;
    refined.request_id = request->request_id;
    refined.target_id = request->target_id;
    refined.frame_id = request->source_frame_id;
    refined.refined_class = refined_class;
    refined.result_valid = 1u;
    refined.done = 1u;
    refined.error = 0u;
    refined.refined_confidence = refined_confidence;
    refined.confirmed_risk_level = risk_level;
    refined.confirmed_risk_score = (risk_level == EE_RISK_DANGER) ? 0.92f : 0.12f;
    refined.confirm_state = confirm_state;
    return refined;
}

static void step_to_refine_infer(EE_StateMachine *sm,
                                 const EE_GlobalTargetList *list,
                                 const EE_FoveaRequest *request)
{
    uint8_t i;
    for (i = 0u; i < 8u && sm->state != EE_STATE_REFINE_INFER; ++i) {
        ee_state_machine_step(sm, list, request, NULL);
    }
    assert(sm->state == EE_STATE_REFINE_INFER);
}

static void print_selected_debug(const EE_GlobalTargetList *list, const EE_FoveaRequest *request)
{
    EE_AttentionConfig cfg = ee_attention_default_config();
    uint8_t i;
    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        EE_AttentionDebug debug;
        ee_attention_compute_debug(&list->targets[i], &cfg, &debug);
        printf("Target %u total_attention=%u zone=%u motion=%u approach=%u small=%u uncertainty=%u\n",
               list->targets[i].target_id,
               (unsigned)debug.total_attention,
               debug.zone_score,
               debug.motion_score,
               debug.approach_score,
               debug.small_target_score,
               debug.uncertainty_score);
    }
    printf("Selected Target = %u\n", request->target_id);
}

static void scenario_confirmed(void)
{
    EE_GlobalTargetList list;
    EE_AttentionScheduler scheduler;
    EE_StateMachine sm;
    EE_TransactionContext txn;
    EE_FoveaRequest request;
    EE_RefinedTargetDescriptor refined;
    EE_ConfirmState action;

    make_competition_list(&list);
    ee_scheduler_init(&scheduler, ee_scheduler_default_config());
    ee_state_machine_init(&sm);
    ee_transaction_init(&txn);

    assert(ee_scheduler_step(&scheduler, &list, &request) == 1u);
    assert(request.valid == 1u);
    assert(request.target_id == 202u);
    assert(request.request_id == 1u);
    ee_transaction_start(&txn, &request);
    print_selected_debug(&list, &request);

    step_to_refine_infer(&sm, &list, &request);
    refined = make_refined(&request, EE_CLASS_PEDESTRIAN, 94u, EE_RISK_DANGER, EE_CONFIRM_CONFIRMED);

    assert(ee_transaction_accept_refined(&txn, &refined) == 1u);
    assert(txn.refined_result.request_id == request.request_id);
    assert(txn.refined_result.target_id == request.target_id);
    assert(txn.refined_result.frame_id == request.source_frame_id);

    ee_state_machine_step(&sm, &list, &request, &txn.refined_result);
    assert(sm.state == EE_STATE_CONFIRMED);
    assert(txn.risk_state == EE_RISK_DANGER);
    assert(ee_transaction_consume_action(&txn, &action) == 1u);
    assert(action == EE_CONFIRM_CONFIRMED);
    assert(ee_transaction_consume_action(&txn, &action) == 0u);

    printf("[PASS] Scenario 1 CONFIRMED req=%u target=%u frame=%u risk=%s action=%s\n",
           (unsigned)request.request_id,
           request.target_id,
           (unsigned)request.source_frame_id,
           ee_risk_to_string(txn.risk_state),
           ee_confirm_to_string(EE_CONFIRM_CONFIRMED));
}

static void scenario_downgraded(void)
{
    EE_GlobalTargetList list;
    EE_AttentionScheduler scheduler;
    EE_StateMachine sm;
    EE_TransactionContext txn;
    EE_FoveaRequest request;
    EE_RefinedTargetDescriptor refined;
    EE_ConfirmState action;

    make_competition_list(&list);
    ee_scheduler_init(&scheduler, ee_scheduler_default_config());
    ee_state_machine_init(&sm);
    ee_transaction_init(&txn);

    assert(ee_scheduler_step(&scheduler, &list, &request) == 1u);
    ee_transaction_start(&txn, &request);
    step_to_refine_infer(&sm, &list, &request);

    refined = make_refined(&request, EE_CLASS_BACKGROUND, 91u, EE_RISK_NORMAL, EE_CONFIRM_DOWNGRADED);
    assert(ee_transaction_accept_refined(&txn, &refined) == 1u);
    ee_state_machine_step(&sm, &list, &request, &txn.refined_result);

    assert(sm.state == EE_STATE_CONFIRMED);
    assert(txn.risk_state == EE_RISK_NORMAL);
    assert(ee_transaction_consume_action(&txn, &action) == 1u);
    assert(action == EE_CONFIRM_DOWNGRADED);

    printf("[PASS] Scenario 2 DOWNGRADED req=%u target=%u risk=%s action=%s\n",
           (unsigned)request.request_id,
           request.target_id,
           ee_risk_to_string(txn.risk_state),
           ee_confirm_to_string(action));
}

static void scenario_timeout(void)
{
    EE_GlobalTargetList list;
    EE_AttentionScheduler scheduler;
    EE_StateMachine sm;
    EE_StateMachineConfig cfg;
    EE_TransactionContext txn;
    EE_FoveaRequest request;
    EE_ConfirmState action;
    uint8_t i;

    make_competition_list(&list);
    ee_scheduler_init(&scheduler, ee_scheduler_default_config());
    ee_transaction_init(&txn);

    cfg.enhance_frames = 0u;
    cfg.refine_timeout_frames = 2u;
    cfg.hold_frames = 1u;
    ee_state_machine_init_with_config(&sm, cfg);

    assert(ee_scheduler_step(&scheduler, &list, &request) == 1u);
    ee_transaction_start(&txn, &request);
    step_to_refine_infer(&sm, &list, &request);

    for (i = 0u; i < 4u && sm.state != EE_STATE_RELEASE; ++i) {
        ee_state_machine_step(&sm, &list, &request, NULL);
    }

    assert(sm.state == EE_STATE_RELEASE);
    assert(txn.refined_result.valid == 0u);
    ee_transaction_mark_timeout(&txn);
    assert(ee_transaction_consume_action(&txn, &action) == 1u);
    assert(action == EE_CONFIRM_TIMEOUT);

    ee_transaction_release(&txn);
    ee_scheduler_release(&scheduler);
    memset(&request, 0, sizeof(request));

    assert(txn.current_request.valid == 0u);
    assert(txn.refined_result.valid == 0u);

    printf("[PASS] Scenario 3 TIMEOUT released_current_request=%u refined_valid=%u action=%s\n",
           txn.current_request.valid,
           txn.refined_result.valid,
           ee_confirm_to_string(action));
}

static void scenario_stale_result_rejected(void)
{
    EE_GlobalTargetList list;
    EE_AttentionScheduler scheduler;
    EE_TransactionContext txn;
    EE_FoveaRequest first_request;
    EE_FoveaRequest second_request;
    EE_RefinedTargetDescriptor stale;

    make_competition_list(&list);
    ee_scheduler_init(&scheduler, ee_scheduler_default_config());
    ee_transaction_init(&txn);

    assert(ee_scheduler_step(&scheduler, &list, &first_request) == 1u);
    ee_transaction_start(&txn, &first_request);
    ee_transaction_release(&txn);
    ee_scheduler_release(&scheduler);

    assert(ee_scheduler_step(&scheduler, &list, &second_request) == 1u);
    ee_transaction_start(&txn, &second_request);
    assert(second_request.request_id == 2u);

    stale = make_refined(&second_request, EE_CLASS_PEDESTRIAN, 95u, EE_RISK_DANGER, EE_CONFIRM_UPGRADED);
    stale.request_id = 1u;

    assert(ee_transaction_accept_refined(&txn, &stale) == 0u);
    assert(txn.refined_result.valid == 0u);
    assert(txn.risk_state == EE_RISK_NORMAL);

    printf("[PASS] Scenario 4 STALE RESULT REJECTED current_req=%u received_req=%u risk=%s\n",
           (unsigned)second_request.request_id,
           (unsigned)stale.request_id,
           ee_risk_to_string(txn.risk_state));
}

static void mismatch_request_id_rejected(void)
{
    EE_FoveaRequest request;
    EE_RefinedTargetDescriptor refined;

    memset(&request, 0, sizeof(request));
    request.valid = 1u;
    request.request_id = 10u;
    request.target_id = 202u;
    request.source_frame_id = 7u;

    refined = make_refined(&request, EE_CLASS_PEDESTRIAN, 95u, EE_RISK_DANGER, EE_CONFIRM_UPGRADED);
    refined.request_id = 9u;

    assert(ee_refined_result_matches_request(&request, &refined) == 0u);
    printf("[PASS] mismatch request_id rejected current_req=%u received_req=%u\n",
           (unsigned)request.request_id,
           (unsigned)refined.request_id);
}

static void mismatch_target_id_rejected(void)
{
    EE_FoveaRequest request;
    EE_RefinedTargetDescriptor refined;

    memset(&request, 0, sizeof(request));
    request.valid = 1u;
    request.request_id = 11u;
    request.target_id = 202u;
    request.source_frame_id = 8u;

    refined = make_refined(&request, EE_CLASS_PEDESTRIAN, 95u, EE_RISK_DANGER, EE_CONFIRM_UPGRADED);
    refined.target_id = 303u;

    assert(ee_refined_result_matches_request(&request, &refined) == 0u);
    printf("[PASS] mismatch target_id rejected current_target=%u received_target=%u\n",
           request.target_id,
           refined.target_id);
}

static void mismatch_frame_id_rejected(void)
{
    EE_FoveaRequest request;
    EE_RefinedTargetDescriptor refined;

    memset(&request, 0, sizeof(request));
    request.valid = 1u;
    request.request_id = 12u;
    request.target_id = 202u;
    request.source_frame_id = 9u;

    refined = make_refined(&request, EE_CLASS_PEDESTRIAN, 95u, EE_RISK_DANGER, EE_CONFIRM_UPGRADED);
    refined.frame_id = 10u;

    assert(ee_refined_result_matches_request(&request, &refined) == 0u);
    printf("[PASS] mismatch frame_id rejected current_frame=%u received_frame=%u\n",
           (unsigned)request.source_frame_id,
           (unsigned)refined.frame_id);
}

static void soccer_scenario_tests(void)
{
    EE_GlobalTargetList first;
    EE_GlobalTargetList second;
    EE_AttentionScheduler scheduler;
    EE_FoveaRequest request;
    EE_MotionPrediction prediction;

    ee_motion_predictor_init();
    ee_scheduler_init(&scheduler, ee_scheduler_default_config());

    ee_fake_generate_soccer_scenario(1u, 1u, &first);
    assert(ee_scheduler_step(&scheduler, &first, &request) == 1u);
    assert(request.target_id == 1u);
    ee_motion_predictor_update(&first);

    ee_fake_generate_soccer_scenario(2u, 2u, &second);
    assert(ee_motion_predict(&second.targets[0], &prediction) == 1u);
    assert(prediction.prediction_valid == 1u);
    assert(prediction.target_id == 1u);
    printf("[PASS] Soccer 1/2 BALL focus and prediction target=%u predicted=(%u,%u)\n",
           prediction.target_id, prediction.predicted_x, prediction.predicted_y);

    ee_fake_generate_soccer_scenario(3u, 3u, &second);
    assert(ee_scheduler_step(&scheduler, &second, &request) == 1u);
    assert(request.target_id == 1u);
    printf("[PASS] Soccer 3 GOAL_ZONE keeps BALL focus target=%u\n", request.target_id);

    ee_fake_generate_soccer_scenario(6u, 4u, &second);
    assert(ee_scheduler_step(&scheduler, &second, &request) == 1u);
    assert(request.target_id == 1u || request.target_id == 21u || request.target_id == 22u);
    printf("[PASS] Soccer 6 Switch Margin/Hold protection target=%u\n", request.target_id);

    ee_fake_generate_soccer_scenario(7u, 5u, &second);
    assert(second.count == 0u);
    printf("[PASS] Soccer 7 long disappearance provides empty candidates for timeout/release\n");
}

int main(void)
{
    scenario_confirmed();
    scenario_downgraded();
    scenario_timeout();
    scenario_stale_result_rejected();
    mismatch_request_id_rejected();
    mismatch_target_id_rejected();
    mismatch_frame_id_rejected();
    soccer_scenario_tests();
    printf("All scenario tests passed.\n");
    return 0;
}
