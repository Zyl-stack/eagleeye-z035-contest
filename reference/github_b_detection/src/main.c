#include "attention_scheduler.h"
#include "attention.h"
#include "fake_data.h"
#include "risk.h"
#include "state_machine.h"
#include "transaction.h"
#include "motion_predictor.h"
#include <stdio.h>
#include <string.h>

static const EE_GlobalTargetDescriptor *find_target(const EE_GlobalTargetList *list, uint16_t target_id)
{
    uint8_t i;
    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].valid && list->targets[i].target_id == target_id) {
            return &list->targets[i];
        }
    }
    return NULL;
}

static void print_targets(const EE_GlobalTargetList *list)
{
    uint8_t i;
    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        const EE_GlobalTargetDescriptor *t = &list->targets[i];
        printf("  target=%u zone=%s coarse=%s conf=%u vx=%d vy=%d approach=%u attn=%u bbox=(%u,%u,%u,%u)\n",
               t->target_id,
               ee_zone_to_string((EE_Zone)t->zone_id),
               ee_class_to_string((EE_TargetClass)t->coarse_class),
               t->coarse_conf,
               t->vx,
               t->vy,
               t->approach_level,
               (unsigned)t->attention_score,
               t->x, t->y, t->w, t->h);
    }
}

static void print_attention_debug(const EE_GlobalTargetList *list,
                                  const uint16_t waiting_frames[EE_MAX_TARGETS])
{
    uint8_t i;
    EE_AttentionConfig cfg = ee_attention_default_config();

    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        EE_AttentionDebug debug;
        const EE_GlobalTargetDescriptor *t = &list->targets[i];
        ee_attention_compute_debug_waiting(t, &cfg, waiting_frames ? waiting_frames[i] : 0u, &debug);
        printf("Target %u\n", t->target_id);
        printf("  ball_priority     = %u\n", debug.ball_priority_score);
        printf("  motion_score      = %u\n", debug.motion_score);
        printf("  goal_zone_score   = %u\n", debug.goal_zone_score);
        printf("  small_target      = %u\n", debug.small_target_score);
        printf("  uncertainty       = %u\n", debug.uncertainty_score);
        printf("  ball_proximity    = %u\n", debug.ball_proximity_score);
        printf("  aging_score       = %u\n", debug.aging_score);
        printf("  total_attention   = %u\n", (unsigned)debug.total_attention);
    }
}

int main(void)
{
    EE_AttentionScheduler scheduler;
    EE_StateMachine sm;
    EE_TransactionContext txn;
    EE_FoveaRequest request;
    EE_RefinedTargetDescriptor refined;
    EE_ConfirmState action;
    EE_SystemState previous_state;
    uint32_t frame;

    ee_scheduler_init(&scheduler, ee_scheduler_default_config());
    ee_state_machine_init(&sm);
    ee_transaction_init(&txn);
    ee_motion_predictor_init();
    memset(&request, 0, sizeof(request));

    printf("EagleEye soccer foveated broadcast software simulation\n");
    printf("Attention selects a ball/player focus target; ROI uses a constant-velocity predictor.\n\n");

    for (frame = 1u; frame <= 18u; ++frame) {
        EE_GlobalTargetList list;
        const EE_GlobalTargetDescriptor *selected_global;

        memset(&refined, 0, sizeof(refined));
        previous_state = sm.state;
        ee_fake_generate_global_targets(frame, &list);
        ee_scheduler_step(&scheduler, &list, &request);

        if (request.valid) {
            const EE_GlobalTargetDescriptor *selected = find_target(&list, request.target_id);
            EE_MotionPrediction prediction;
            if (selected && ee_motion_predict(selected, &prediction) && prediction.prediction_valid) {
                request.roi_x = prediction.predicted_x;
                request.roi_y = prediction.predicted_y;
                printf("  prediction target=%u frame=%u roi_origin=(%u,%u)\n",
                       prediction.target_id, (unsigned)prediction.source_frame_id,
                       prediction.predicted_x, prediction.predicted_y);
            }
        }

        if (sm.state == EE_STATE_REFINE_INFER &&
            ee_fake_generate_refined_result(&request, frame, &refined)) {
            selected_global = find_target(&list, refined.target_id);
            if (selected_global) {
                refined.confirmed_risk_score = ee_confirmed_risk_score(selected_global, &refined);
                refined.confirmed_risk_level = ee_risk_level_from_score(refined.confirmed_risk_score);
            }
            if (!ee_transaction_accept_refined(&txn, &refined)) {
                memset(&refined, 0, sizeof(refined));
            }
        }

        ee_state_machine_step(&sm, &list, &request, &txn.refined_result);

        if (previous_state != EE_STATE_FOVEA_LOCK && sm.state == EE_STATE_FOVEA_LOCK) {
            ee_transaction_start(&txn, &request);
        }
        if (previous_state == EE_STATE_REFINE_INFER && sm.state == EE_STATE_RELEASE && !txn.refined_result.valid) {
            ee_transaction_mark_timeout(&txn);
            printf("  risk_action=%s\n", ee_confirm_to_string(EE_CONFIRM_TIMEOUT));
        }
        if (sm.state == EE_STATE_RELEASE) {
            ee_scheduler_release(&scheduler);
            ee_transaction_release(&txn);
            memset(&request, 0, sizeof(request));
        }

        printf("[frame=%02u] state=%s selected=%u req=%u roi=(%u,%u,%u,%u)\n",
               (unsigned)frame,
               ee_state_to_string(sm.state),
               request.valid ? request.target_id : EE_INVALID_TARGET_ID,
               request.valid ? request.request_id : 0u,
               request.roi_x, request.roi_y, request.roi_w, request.roi_h);
        print_targets(&list);
        print_attention_debug(&list, scheduler.wait_frames);
        printf("Selected Target = %u\n", request.valid ? request.target_id : EE_INVALID_TARGET_ID);

        if (txn.refined_result.valid) {
            printf("  refined req=%u target=%u class=%s conf=%u focus=%s risk=%s score=%.3f\n",
                   txn.refined_result.request_id,
                   txn.refined_result.target_id,
                   ee_class_to_string(txn.refined_result.refined_class),
                   txn.refined_result.refined_confidence,
                   ee_focus_to_string(txn.refined_result.focus_state),
                   ee_risk_to_string(txn.risk_state),
                   txn.refined_result.confirmed_risk_score);
        }
        if (ee_transaction_consume_action(&txn, &action)) {
            printf("  risk_action=%s\n", ee_confirm_to_string(action));
        }
        printf("\n");
        ee_motion_predictor_update(&list);
    }

    return 0;
}
