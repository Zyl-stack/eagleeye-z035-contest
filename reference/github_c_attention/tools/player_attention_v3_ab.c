/* PC-only A/B harness. It reads B's text stream; it is not part of Vitis. */
#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "a_fovea_adapter.h"
#include "attention.h"
#include "attention_scheduler.h"
#include "ball_challenger_gate.h"
#include "bc_adapter.h"
#include "c_config.h"
#include "director_state.h"
#include "enhance_policy.h"
#include "fovea_transaction.h"
#include "motion_predictor.h"
#include "player_attention_features.h"
#include "possession_estimator.h"
#include "roi_motion_controller.h"
#include "roi_smoother.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EE_AB_MAX_FEATURE_SAMPLES 4096u

typedef struct {
    EE_CConfig config;
    EE_AttentionScheduler scheduler;
    EE_PlayerAttentionHistory player_history;
    EE_ROISmoother smoother;
    EE_ROIMotionController motion_controller;
    EE_Director director;
    EE_FoveaTransactionBuilder transactions;
    uint16_t carrier_id;
    uint8_t carrier_score;
} EE_ABRuntime;

typedef struct {
    uint32_t frames;
    uint32_t selected_frames;
    uint32_t no_selected_frames;
    uint32_t switches;
    uint32_t player_selected_frames;
    uint32_t ball_selected_frames;
    uint32_t selected_lifetime_total;
    uint32_t selected_lifetime_count;
    uint32_t current_invalid;
    uint32_t current_lost;
    uint32_t higher_attention;
    uint32_t hold_active;
    uint32_t margin_not_met;
    uint32_t fast_jump_frames;
    uint32_t valid_transactions;
    uint32_t new_transactions;
    uint32_t invalid_transactions;
    uint32_t prediction_attempts;
    uint32_t prediction_valid;
    uint32_t roi_moves;
    uint32_t roi_movement_sum;
    uint32_t roi_movement_max;
    uint32_t ball_invalid_events;
    uint32_t ball_event_player_changes;
    uint16_t last_selected_id;
    uint16_t selected_run_length;
    uint16_t ball_event_reference_player;
    uint8_t ball_event_updates_remaining;
    uint8_t previous_ball_seen;
    uint8_t previous_ball_valid;
    uint8_t previous_roi_valid;
    uint16_t previous_roi_x;
    uint16_t previous_roi_y;
    uint32_t selection_sequence_hash;
    uint8_t interaction_samples[EE_AB_MAX_FEATURE_SAMPLES];
    uint8_t motion_samples[EE_AB_MAX_FEATURE_SAMPLES];
    uint8_t saliency_samples[EE_AB_MAX_FEATURE_SAMPLES];
    uint32_t feature_sample_count;
} EE_ABMetrics;

static uint32_t abs_u16_difference(uint16_t left, uint16_t right)
{
    return left >= right ? (uint32_t)(left - right) : (uint32_t)(right - left);
}

static const EE_GlobalTargetDescriptor *find_valid_target(const EE_GlobalTargetList *list,
                                                           uint16_t target_id)
{
    uint8_t i;

    for (i = 0u; list && i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].valid && list->targets[i].target_id == target_id) {
            return &list->targets[i];
        }
    }
    return NULL;
}

static void mask_ball_records(EE_GlobalTargetList *list)
{
    uint8_t i;

    for (i = 0u; list && i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].is_ball || list->targets[i].target_type == EE_SOCCER_TARGET_BALL) {
            list->targets[i].valid = 0u;
        }
    }
}

static uint8_t raw_ball_valid(const EE_GlobalTargetList *list, uint8_t *out_seen)
{
    uint8_t i;

    *out_seen = 0u;
    for (i = 0u; list && i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].is_ball || list->targets[i].target_type == EE_SOCCER_TARGET_BALL) {
            *out_seen = 1u;
            return list->targets[i].valid;
        }
    }
    return 0u;
}

static void runtime_init(EE_ABRuntime *runtime, EE_AttentionMode mode)
{
    memset(runtime, 0, sizeof(*runtime));
    runtime->config = ee_c_config_default();
    runtime->config.attention_mode = mode;
    /* The supplied real stream has source frame IDs 0, 2, 4, ... . */
    runtime->config.nominal_input_frame_delta = 2u;
    runtime->config.max_prediction_gap_frames = 2u;
    runtime->config.player_attention.history_max_frame_gap_frames = 2u;
    runtime->carrier_id = EE_INVALID_TARGET_ID;
    ee_scheduler_init(&runtime->scheduler, runtime->config.scheduler);
    ee_player_attention_history_init(&runtime->player_history);
    ee_roi_smoother_init(&runtime->smoother, runtime->config.roi_smoother);
    ee_roi_motion_controller_init(&runtime->motion_controller,
                                  runtime->config.roi_smoother,
                                  runtime->config.roi_motion_controller);
    ee_director_init(&runtime->director, runtime->config.director_handover_frames);
    ee_fovea_transaction_init(&runtime->transactions);
    ee_motion_predictor_init();
    ee_motion_predictor_set_nominal_input_frame_delta(runtime->config.nominal_input_frame_delta);
    ee_motion_predictor_set_max_gap_frames(runtime->config.max_prediction_gap_frames);
}

static void record_reason(EE_ABMetrics *metrics, EE_SwitchReason reason)
{
    switch (reason) {
    case EE_SWITCH_REASON_CURRENT_INVALID: ++metrics->current_invalid; break;
    case EE_SWITCH_REASON_CURRENT_LOST: ++metrics->current_lost; break;
    case EE_SWITCH_REASON_HIGHER_ATTENTION: ++metrics->higher_attention; break;
    case EE_SWITCH_REASON_HOLD_ACTIVE: ++metrics->hold_active; break;
    case EE_SWITCH_REASON_MARGIN_NOT_MET: ++metrics->margin_not_met; break;
    default: break;
    }
}

static void record_player_features(EE_ABMetrics *metrics,
                                   const EE_PlayerAttentionFrame *features)
{
    uint8_t i;

    for (i = 0u; features && i < features->count && i < EE_MAX_TARGETS; ++i) {
        const EE_PlayerAttentionFeatures *item = &features->features[i];

        if (!item->valid || metrics->feature_sample_count >= EE_AB_MAX_FEATURE_SAMPLES) {
            continue;
        }
        metrics->interaction_samples[metrics->feature_sample_count] = item->interaction_score;
        metrics->motion_samples[metrics->feature_sample_count] = item->motion_score;
        metrics->saliency_samples[metrics->feature_sample_count] = item->motion_saliency_score;
        ++metrics->feature_sample_count;
    }
}

static void record_selected(EE_ABMetrics *metrics,
                            const EE_GlobalTargetDescriptor *selected)
{
    uint16_t selected_id = selected ? selected->target_id : EE_INVALID_TARGET_ID;

    metrics->selection_sequence_hash =
        (metrics->selection_sequence_hash ^ selected_id) * 16777619u;
    if (!selected) {
        ++metrics->no_selected_frames;
        if (metrics->selected_run_length != 0u) {
            metrics->selected_lifetime_total += metrics->selected_run_length;
            ++metrics->selected_lifetime_count;
            metrics->selected_run_length = 0u;
        }
        metrics->last_selected_id = EE_INVALID_TARGET_ID;
        return;
    }

    ++metrics->selected_frames;
    if (selected->target_type == EE_SOCCER_TARGET_PLAYER) {
        ++metrics->player_selected_frames;
    } else if (selected->target_type == EE_SOCCER_TARGET_BALL) {
        ++metrics->ball_selected_frames;
    }
    if (metrics->last_selected_id != EE_INVALID_TARGET_ID &&
        metrics->last_selected_id != selected->target_id) {
        ++metrics->switches;
        metrics->selected_lifetime_total += metrics->selected_run_length;
        ++metrics->selected_lifetime_count;
        metrics->selected_run_length = 0u;
    }
    ++metrics->selected_run_length;
    metrics->last_selected_id = selected->target_id;

    if (metrics->ball_event_updates_remaining != 0u) {
        if (metrics->ball_event_reference_player != EE_INVALID_TARGET_ID &&
            selected->target_type == EE_SOCCER_TARGET_PLAYER &&
            selected->target_id != metrics->ball_event_reference_player) {
            ++metrics->ball_event_player_changes;
            metrics->ball_event_updates_remaining = 0u;
        } else {
            --metrics->ball_event_updates_remaining;
        }
    }
}

static void record_ball_transition(EE_ABMetrics *metrics, uint8_t seen, uint8_t valid)
{
    if (metrics->previous_ball_seen && metrics->previous_ball_valid && (!seen || !valid)) {
        ++metrics->ball_invalid_events;
        metrics->ball_event_reference_player = metrics->last_selected_id;
        metrics->ball_event_updates_remaining = 3u;
    }
    metrics->previous_ball_seen = seen;
    metrics->previous_ball_valid = valid;
}

static void run_one_frame(EE_ABRuntime *runtime, EE_ABMetrics *metrics,
                          EE_GlobalTargetList *list, uint8_t mask_ball)
{
    EE_GlobalTargetList raw_list = *list;
    EE_GlobalTargetList candidates;
    EE_PlayerAttentionFrame player_features;
    EE_FoveaRequest desired;
    EE_FoveaRequest smoothed;
    EE_FoveaRequest adapted;
    EE_MotionPrediction prediction;
    EE_EnhancePolicyInput policy_input;
    EE_EnhanceDecision enhance;
    EE_AttentionBreakdown enhance_attention;
    EE_ROIMotionInput motion_input;
    EE_ROIMotionDecision motion_decision;
    EE_FoveaTransactionResult transaction;
    const EE_SchedulerDecision *decision;
    const EE_GlobalTargetDescriptor *selected;
    uint8_t ball_seen;
    uint8_t ball_valid;

    ball_valid = raw_ball_valid(&raw_list, &ball_seen);
    record_ball_transition(metrics, ball_seen, ball_valid);
    if (mask_ball) {
        mask_ball_records(list);
    }

    ee_possession_update_list(list, runtime->carrier_id, runtime->carrier_score,
                              &runtime->carrier_id, &runtime->carrier_score);
    memset(&player_features, 0, sizeof(player_features));
    if (runtime->config.attention_mode == EE_ATTENTION_MODE_PLAYER_V3) {
        if (!ee_player_attention_update_scores_for_scheduler_step(
                list, &runtime->config.player_attention, &runtime->player_history,
                runtime->scheduler.wait_frames, &player_features) ||
            !ee_player_attention_candidate_view(list, &candidates)) {
            return;
        }
        record_player_features(metrics, &player_features);
    } else {
        ee_attention_update_scores_for_scheduler_step(list->targets, list->count,
                                                      &runtime->config.attention,
                                                      runtime->scheduler.wait_frames);
        if (!ee_ball_challenger_gate_apply(&runtime->config, &runtime->scheduler,
                                           list, &candidates)) {
            return;
        }
    }

    memset(&desired, 0, sizeof(desired));
    (void)ee_scheduler_step(&runtime->scheduler, &candidates, &desired);
    decision = ee_scheduler_last_decision(&runtime->scheduler);
    selected = find_valid_target(list, decision->selected_target_id);
    if (runtime->config.attention_mode == EE_ATTENTION_MODE_PLAYER_V3 && selected) {
        if (selected->target_type != EE_SOCCER_TARGET_PLAYER) {
            fprintf(stderr, "V3 invariant failed: selected target is not PLAYER\n");
            exit(2);
        }
    }
    record_reason(metrics, decision->switch_reason);
    record_selected(metrics, selected);

    ee_director_update(&runtime->director, list, runtime->carrier_id,
                       decision->selected_target_id);
    memset(&motion_input, 0, sizeof(motion_input));
    motion_input.director_state = runtime->director.state;
    motion_input.director_event = runtime->director.event;
    motion_input.switch_reason = decision->switch_reason;
    motion_input.selected_target_id = decision->selected_target_id;
    motion_input.previous_selected_target_id = decision->previous_target_id;
    (void)ee_roi_motion_controller_step(&runtime->motion_controller,
                                        &motion_input, &motion_decision);
    if (motion_decision.mode == EE_ROI_MOTION_FAST_JUMP) {
        ++metrics->fast_jump_frames;
    }

    memset(&enhance_attention, 0, sizeof(enhance_attention));
    if (selected) {
        enhance_attention.small_target_score = selected->small_target_score;
        enhance_attention.total_attention = selected->attention_score;
    }
    memset(&policy_input, 0, sizeof(policy_input));
    policy_input.selected_target = selected;
    policy_input.attention = selected ? &enhance_attention : NULL;
    policy_input.possession_target_id = runtime->carrier_id;
    enhance = ee_enhance_policy_decide(&policy_input, &runtime->config);

    if (desired.valid && selected) {
        memset(&prediction, 0, sizeof(prediction));
        ++metrics->prediction_attempts;
        if (ee_motion_predict_with_latency(selected, runtime->config.prediction_horizon_frames,
                                           &runtime->config.latency_model, &prediction) &&
            prediction.prediction_valid) {
            desired.roi_x = prediction.predicted_x;
            desired.roi_y = prediction.predicted_y;
            ++metrics->prediction_valid;
        }
        ee_roi_smoother_set_config(&runtime->smoother, motion_decision.smoother_config);
        if (!ee_roi_smoother_step(&runtime->smoother, &desired, &smoothed) ||
            ee_a_fovea_adapt_request(&smoothed, &adapted) != 0) {
            ++metrics->invalid_transactions;
        } else {
            adapted.enhance_mode = (uint8_t)enhance.mode;
            transaction = ee_fovea_transaction_build(&runtime->transactions, &adapted,
                                                      list->frame_id, &adapted);
            if (transaction == EE_FOVEA_TRANSACTION_INVALID) {
                ++metrics->invalid_transactions;
            } else {
                ++metrics->valid_transactions;
                if (transaction == EE_FOVEA_TRANSACTION_NEW) {
                    ++metrics->new_transactions;
                }
            }
            if (metrics->previous_roi_valid) {
                uint32_t movement = abs_u16_difference(metrics->previous_roi_x, adapted.roi_x) +
                                    abs_u16_difference(metrics->previous_roi_y, adapted.roi_y);
                metrics->roi_movement_sum += movement;
                if (movement > metrics->roi_movement_max) {
                    metrics->roi_movement_max = movement;
                }
                ++metrics->roi_moves;
            }
            metrics->previous_roi_x = adapted.roi_x;
            metrics->previous_roi_y = adapted.roi_y;
            metrics->previous_roi_valid = 1u;
        }
    }
    ee_motion_predictor_update(list);
    ++metrics->frames;
}

static int compare_u8(const void *left, const void *right)
{
    uint8_t a = *(const uint8_t *)left;
    uint8_t b = *(const uint8_t *)right;

    return a < b ? -1 : (a > b ? 1 : 0);
}

static void print_distribution(const char *name, uint8_t values[], uint32_t count)
{
    if (count == 0u) {
        printf("  %s: no samples\n", name);
        return;
    }
    qsort(values, count, sizeof(values[0]), compare_u8);
    printf("  %s min/median/p90/max: %u / %u / %u / %u\n", name,
           values[0], values[(count - 1u) / 2u], values[((count - 1u) * 90u) / 100u],
           values[count - 1u]);
}

static int run_group(const char *label, const char *path, EE_AttentionMode mode, uint8_t mask_ball)
{
    EE_BCStreamReader reader;
    EE_BParsedFrame parsed;
    EE_GlobalTargetList list;
    EE_ABRuntime runtime;
    EE_ABMetrics metrics;
    FILE *file;
    int result;

    file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Cannot open stream: %s\n", path);
        return 0;
    }
    memset(&metrics, 0, sizeof(metrics));
    metrics.selection_sequence_hash = 2166136261u;
    metrics.last_selected_id = EE_INVALID_TARGET_ID;
    metrics.ball_event_reference_player = EE_INVALID_TARGET_ID;
    runtime_init(&runtime, mode);
    ee_bc_stream_reader_init(&reader, file);

    while ((result = ee_bc_stream_read_frame(&reader, &parsed)) == 1) {
        if (!ee_bc_adapt_frame(&parsed, &list)) {
            fclose(file);
            return 0;
        }
        run_one_frame(&runtime, &metrics, &list, mask_ball);
    }
    fclose(file);
    if (result != 0) {
        fprintf(stderr, "B stream parse failure: %s\n", ee_bc_stream_error(&reader));
        return 0;
    }
    if (metrics.selected_run_length != 0u) {
        metrics.selected_lifetime_total += metrics.selected_run_length;
        ++metrics.selected_lifetime_count;
    }

    printf("\n[%s] mode=%s ball_records=%s\n", label, ee_attention_mode_to_string(mode),
           mask_ball ? "MASKED" : "ORIGINAL");
    printf("  frames=%u selected=%u no_selected=%u switches=%u\n", metrics.frames,
           metrics.selected_frames, metrics.no_selected_frames, metrics.switches);
    printf("  selected PLAYER/BALL=%u/%u average_lifetime=%.2f updates\n",
           metrics.player_selected_frames, metrics.ball_selected_frames,
           metrics.selected_lifetime_count == 0u ? 0.0 :
           (double)metrics.selected_lifetime_total / metrics.selected_lifetime_count);
    printf("  reasons CURRENT_INVALID=%u CURRENT_LOST=%u HIGHER_ATTENTION=%u HOLD_ACTIVE=%u MARGIN_NOT_MET=%u\n",
           metrics.current_invalid, metrics.current_lost, metrics.higher_attention,
           metrics.hold_active, metrics.margin_not_met);
    printf("  FAST_JUMP frames=%u ratio=%.3f valid/new/invalid_transactions=%u/%u/%u prediction_valid=%u/%u\n",
           metrics.fast_jump_frames, metrics.frames == 0u ? 0.0 :
           (double)metrics.fast_jump_frames / metrics.frames, metrics.valid_transactions,
           metrics.new_transactions, metrics.invalid_transactions, metrics.prediction_valid,
           metrics.prediction_attempts);
    printf("  ROI average/max movement=%.2f/%u px; BALL valid->invalid=%u player changes within next 3 updates=%u\n",
           metrics.roi_moves == 0u ? 0.0 : (double)metrics.roi_movement_sum / metrics.roi_moves,
           metrics.roi_movement_max, metrics.ball_invalid_events, metrics.ball_event_player_changes);
    printf("  selected-sequence hash=0x%08X\n", metrics.selection_sequence_hash);
    if (mode == EE_ATTENTION_MODE_PLAYER_V3) {
        print_distribution("interaction", metrics.interaction_samples, metrics.feature_sample_count);
        print_distribution("motion", metrics.motion_samples, metrics.feature_sample_count);
        print_distribution("motion_saliency", metrics.saliency_samples, metrics.feature_sample_count);
    }
    return 1;
}

int main(int argc, char **argv)
{
    const char *path;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <real_match_b_target_stream.txt>\n", argv[0]);
        return 2;
    }
    path = argv[1];
    printf("EagleEye Player-driven Attention V3 A/B (PC stream harness)\n");
    printf("V2 remains the default production mode; V3 is explicitly enabled only here.\n");
    if (!run_group("A V2 original", path, EE_ATTENTION_MODE_SOCCER_V2, 0u) ||
        !run_group("B V2 no-BALL", path, EE_ATTENTION_MODE_SOCCER_V2, 1u) ||
        !run_group("C V3 original", path, EE_ATTENTION_MODE_PLAYER_V3, 0u) ||
        !run_group("D V3 no-BALL", path, EE_ATTENTION_MODE_PLAYER_V3, 1u)) {
        return 1;
    }
    printf("\n[PASS] V3 completed both BALL-present and BALL-masked runs without selecting BALL.\n");
    return 0;
}
