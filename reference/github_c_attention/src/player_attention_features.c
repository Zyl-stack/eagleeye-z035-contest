#include "player_attention_features.h"

#include <string.h>

static uint32_t abs_i16(int16_t value)
{
    int32_t wide_value = (int32_t)value;

    if (wide_value < 0) {
        wide_value = -wide_value;
    }
    return (uint32_t)wide_value;
}

static uint32_t abs_i32_difference(int32_t left, int32_t right)
{
    int32_t difference = left - right;

    if (difference < 0) {
        difference = -difference;
    }
    return (uint32_t)difference;
}

static uint8_t saturate_u8(uint32_t value)
{
    return value > 255u ? 255u : (uint8_t)value;
}

static uint8_t scale_to_u8(uint32_t value, uint32_t full_scale)
{
    if (full_scale == 0u) {
        return value == 0u ? 0u : 255u;
    }
    if (value >= full_scale) {
        return 255u;
    }
    return (uint8_t)((value * 255u) / full_scale);
}

static uint8_t is_player(const EE_GlobalTargetDescriptor *target)
{
    return target && target->valid &&
           target->target_type == EE_SOCCER_TARGET_PLAYER;
}

static uint8_t is_reliable_ball(const EE_GlobalTargetDescriptor *target,
                                const EE_PlayerAttentionConfig *config)
{
    return target && config && target->valid &&
           (target->is_ball || target->target_type == EE_SOCCER_TARGET_BALL) &&
           target->coarse_conf >= config->player_ball_context_min_confidence;
}

static int32_t center_x(const EE_GlobalTargetDescriptor *target)
{
    return (int32_t)target->x + (int32_t)target->w / 2;
}

static int32_t center_y(const EE_GlobalTargetDescriptor *target)
{
    return (int32_t)target->y + (int32_t)target->h / 2;
}

static uint32_t manhattan_distance(int32_t left_x, int32_t left_y,
                                   int32_t right_x, int32_t right_y)
{
    return abs_i32_difference(left_x, right_x) + abs_i32_difference(left_y, right_y);
}

static uint8_t frame_gap_is_continuous(uint32_t previous_frame,
                                       uint32_t current_frame,
                                       uint32_t max_gap)
{
    return current_frame > previous_frame &&
           current_frame - previous_frame <= max_gap;
}

static EE_PlayerAttentionObservation *find_observation(EE_PlayerAttentionHistory *history,
                                                        uint16_t target_id)
{
    uint8_t i;

    for (i = 0u; i < EE_MAX_TARGETS; ++i) {
        if (history->observations[i].valid &&
            history->observations[i].target_id == target_id) {
            return &history->observations[i];
        }
    }
    return NULL;
}

static EE_PlayerAttentionObservation *reserve_observation(EE_PlayerAttentionHistory *history,
                                                           uint16_t target_id)
{
    EE_PlayerAttentionObservation *oldest = &history->observations[0];
    uint8_t i;

    for (i = 0u; i < EE_MAX_TARGETS; ++i) {
        EE_PlayerAttentionObservation *entry = &history->observations[i];

        if (entry->valid && entry->target_id == target_id) {
            return entry;
        }
        if (!entry->valid) {
            return entry;
        }
        if (entry->frame_id < oldest->frame_id) {
            oldest = entry;
        }
    }
    return oldest;
}

static void ordered_pair_ids(uint16_t first, uint16_t second,
                             uint16_t *out_low, uint16_t *out_high)
{
    if (first < second) {
        *out_low = first;
        *out_high = second;
    } else {
        *out_low = second;
        *out_high = first;
    }
}

static EE_PlayerAttentionPairHistory *find_pair(EE_PlayerAttentionHistory *history,
                                                uint16_t first, uint16_t second)
{
    uint16_t low;
    uint16_t high;
    uint16_t i;

    ordered_pair_ids(first, second, &low, &high);
    for (i = 0u; i < EE_PLAYER_ATTENTION_MAX_PAIRS; ++i) {
        EE_PlayerAttentionPairHistory *entry = &history->pairs[i];

        if (entry->valid && entry->target_id_low == low && entry->target_id_high == high) {
            return entry;
        }
    }
    return NULL;
}

static EE_PlayerAttentionPairHistory *reserve_pair(EE_PlayerAttentionHistory *history,
                                                   uint16_t first, uint16_t second)
{
    EE_PlayerAttentionPairHistory *oldest = &history->pairs[0];
    uint16_t low;
    uint16_t high;
    uint16_t i;

    ordered_pair_ids(first, second, &low, &high);
    for (i = 0u; i < EE_PLAYER_ATTENTION_MAX_PAIRS; ++i) {
        EE_PlayerAttentionPairHistory *entry = &history->pairs[i];

        if (entry->valid && entry->target_id_low == low && entry->target_id_high == high) {
            return entry;
        }
        if (!entry->valid) {
            return entry;
        }
        if (entry->frame_id < oldest->frame_id) {
            oldest = entry;
        }
    }
    return oldest;
}

static void invalidate_history_for_target(EE_PlayerAttentionHistory *history, uint16_t target_id)
{
    uint8_t i;
    uint16_t pair_index;

    for (i = 0u; i < EE_MAX_TARGETS; ++i) {
        if (history->observations[i].valid && history->observations[i].target_id == target_id) {
            history->observations[i].valid = 0u;
        }
    }
    for (pair_index = 0u; pair_index < EE_PLAYER_ATTENTION_MAX_PAIRS; ++pair_index) {
        EE_PlayerAttentionPairHistory *pair = &history->pairs[pair_index];

        if (pair->valid && (pair->target_id_low == target_id || pair->target_id_high == target_id)) {
            pair->valid = 0u;
        }
    }
}

static uint8_t goal_zone_score(const EE_GlobalTargetDescriptor *target,
                               const EE_PlayerAttentionConfig *config)
{
    if (!config->goal_zone_enable) {
        return 0u;
    }
    switch ((EE_FieldZone)target->field_zone) {
    case EE_FIELD_GOAL_ZONE: return config->goal_zone_score;
    case EE_FIELD_ATTACK_ZONE: return config->attack_zone_score;
    default: return 0u;
    }
}

static uint8_t uncertainty_score(const EE_GlobalTargetDescriptor *target)
{
    uint8_t confidence = target->coarse_conf > 100u ? 100u : target->coarse_conf;

    return (uint8_t)(((uint32_t)(100u - confidence) * 255u) / 100u);
}

static uint8_t aging_score(uint16_t current_wait)
{
    uint16_t next_wait = current_wait;

    if (next_wait < UINT16_MAX) {
        ++next_wait;
    }
    return next_wait >= 30u ? 255u : (uint8_t)(((uint32_t)next_wait * 255u) / 30u);
}

EE_PlayerAttentionConfig ee_player_attention_default_config(void)
{
    EE_PlayerAttentionConfig config;

    config.w_motion = 3u;
    config.w_interaction = 3u;
    config.w_goal_zone = 3u;
    config.w_motion_saliency = 2u;
    config.w_uncertainty = 2u;
    config.w_aging = 2u;
    config.player_motion_full_scale = 16u;
    config.interaction_radius = 80u;
    config.interaction_approach_margin = 2u;
    config.interaction_neighbor_points = 48u;
    config.interaction_approach_points = 80u;
    config.motion_saliency_full_scale = 12u;
    config.history_max_frame_gap_frames = 2u;
    config.goal_zone_enable = 0u;
    config.attack_zone_score = 180u;
    config.goal_zone_score = 255u;
    config.player_ball_context_enable = 0u;
    config.player_ball_context_weight = 64u;
    config.player_ball_context_min_confidence = 50u;
    config.player_ball_context_radius = 100u;
    config.player_ball_context_max_bonus = 64u;
    return config;
}

const char *ee_attention_mode_to_string(EE_AttentionMode mode)
{
    return mode == EE_ATTENTION_MODE_PLAYER_V3 ? "PLAYER_DRIVEN_V3" : "SOCCER_V2";
}

void ee_player_attention_history_init(EE_PlayerAttentionHistory *history)
{
    if (history) {
        memset(history, 0, sizeof(*history));
    }
}

uint8_t ee_player_attention_update_scores_for_scheduler_step(
    EE_GlobalTargetList *list,
    const EE_PlayerAttentionConfig *config,
    EE_PlayerAttentionHistory *history,
    const uint16_t current_wait_frames[EE_MAX_TARGETS],
    EE_PlayerAttentionFrame *out_frame)
{
    int32_t centers_x[EE_MAX_TARGETS];
    int32_t centers_y[EE_MAX_TARGETS];
    const EE_GlobalTargetDescriptor *reliable_ball = NULL;
    uint8_t i;
    uint8_t j;

    if (!list || !config || !history || !out_frame) {
        return 0u;
    }
    memset(out_frame, 0, sizeof(*out_frame));
    out_frame->count = list->count > EE_MAX_TARGETS ? EE_MAX_TARGETS : list->count;
    memset(centers_x, 0, sizeof(centers_x));
    memset(centers_y, 0, sizeof(centers_y));

    for (i = 0u; i < out_frame->count; ++i) {
        EE_GlobalTargetDescriptor *target = &list->targets[i];
        EE_PlayerAttentionFeatures *features = &out_frame->features[i];

        features->target_id = target->target_id;
        if (!is_player(target)) {
            target->attention_score = 0u;
            if (target->target_type == EE_SOCCER_TARGET_PLAYER && !target->valid) {
                invalidate_history_for_target(history, target->target_id);
            }
            if (!reliable_ball && is_reliable_ball(target, config)) {
                reliable_ball = target;
            }
            continue;
        }

        centers_x[i] = center_x(target);
        centers_y[i] = center_y(target);
        features->valid = 1u;
        features->motion_score = scale_to_u8(abs_i16(target->vx) + abs_i16(target->vy),
                                             config->player_motion_full_scale);
        features->goal_zone_score = goal_zone_score(target, config);
        features->uncertainty_score = uncertainty_score(target);
        features->aging_score = aging_score(current_wait_frames ? current_wait_frames[i] : 0u);

        {
            EE_PlayerAttentionObservation *previous = find_observation(history, target->target_id);

            if (previous && frame_gap_is_continuous(previous->frame_id, target->frame_id,
                                                    config->history_max_frame_gap_frames)) {
                uint32_t delta_velocity = abs_i32_difference((int32_t)target->vx, (int32_t)previous->vx) +
                                          abs_i32_difference((int32_t)target->vy, (int32_t)previous->vy);
                features->motion_saliency_score = scale_to_u8(
                    delta_velocity, config->motion_saliency_full_scale);
            }
        }
    }

    for (i = 0u; i < out_frame->count; ++i) {
        if (!out_frame->features[i].valid) {
            continue;
        }
        for (j = (uint8_t)(i + 1u); j < out_frame->count; ++j) {
            EE_PlayerAttentionPairHistory *previous_pair;
            uint32_t distance;
            uint8_t approaching = 0u;

            if (!out_frame->features[j].valid) {
                continue;
            }
            distance = manhattan_distance(centers_x[i], centers_y[i], centers_x[j], centers_y[j]);
            if (distance <= config->interaction_radius) {
                out_frame->features[i].interaction_score = saturate_u8(
                    (uint32_t)out_frame->features[i].interaction_score +
                    config->interaction_neighbor_points);
                out_frame->features[j].interaction_score = saturate_u8(
                    (uint32_t)out_frame->features[j].interaction_score +
                    config->interaction_neighbor_points);
            }
            previous_pair = find_pair(history, list->targets[i].target_id, list->targets[j].target_id);
            if (previous_pair &&
                frame_gap_is_continuous(previous_pair->frame_id, list->frame_id,
                                        config->history_max_frame_gap_frames) &&
                distance + config->interaction_approach_margin < previous_pair->distance) {
                approaching = 1u;
            }
            if (approaching) {
                out_frame->features[i].interaction_score = saturate_u8(
                    (uint32_t)out_frame->features[i].interaction_score +
                    config->interaction_approach_points);
                out_frame->features[j].interaction_score = saturate_u8(
                    (uint32_t)out_frame->features[j].interaction_score +
                    config->interaction_approach_points);
            }
        }
    }

    if (config->player_ball_context_enable && reliable_ball) {
        int32_t ball_center_x = center_x(reliable_ball);
        int32_t ball_center_y = center_y(reliable_ball);

        for (i = 0u; i < out_frame->count; ++i) {
            uint32_t distance;
            uint32_t proximity;
            uint32_t bonus;

            if (!out_frame->features[i].valid) {
                continue;
            }
            distance = manhattan_distance(centers_x[i], centers_y[i], ball_center_x, ball_center_y);
            if (distance >= config->player_ball_context_radius ||
                config->player_ball_context_radius == 0u) {
                continue;
            }
            proximity = ((uint32_t)(config->player_ball_context_radius - distance) * 255u) /
                        config->player_ball_context_radius;
            bonus = (proximity * config->player_ball_context_weight) / 255u;
            out_frame->features[i].optional_ball_bonus = saturate_u8(
                bonus > config->player_ball_context_max_bonus ?
                    config->player_ball_context_max_bonus : bonus);
        }
    }

    for (i = 0u; i < out_frame->count; ++i) {
        EE_PlayerAttentionFeatures *features = &out_frame->features[i];
        EE_GlobalTargetDescriptor *target = &list->targets[i];

        if (!features->valid) {
            continue;
        }
        features->base_attention =
            (uint32_t)config->w_motion * features->motion_score +
            (uint32_t)config->w_interaction * features->interaction_score +
            (uint32_t)config->w_goal_zone * features->goal_zone_score +
            (uint32_t)config->w_motion_saliency * features->motion_saliency_score +
            (uint32_t)config->w_uncertainty * features->uncertainty_score +
            (uint32_t)config->w_aging * features->aging_score;
        features->total_attention = features->base_attention + features->optional_ball_bonus;
        target->attention_score = features->total_attention;
    }

    for (i = 0u; i < out_frame->count; ++i) {
        if (out_frame->features[i].valid) {
            EE_PlayerAttentionObservation *observation = reserve_observation(
                history, list->targets[i].target_id);

            observation->target_id = list->targets[i].target_id;
            observation->frame_id = list->targets[i].frame_id;
            observation->center_x = centers_x[i];
            observation->center_y = centers_y[i];
            observation->vx = list->targets[i].vx;
            observation->vy = list->targets[i].vy;
            observation->valid = 1u;
        }
    }
    for (i = 0u; i < out_frame->count; ++i) {
        if (!out_frame->features[i].valid) {
            continue;
        }
        for (j = (uint8_t)(i + 1u); j < out_frame->count; ++j) {
            EE_PlayerAttentionPairHistory *pair;

            if (!out_frame->features[j].valid) {
                continue;
            }
            pair = reserve_pair(history, list->targets[i].target_id, list->targets[j].target_id);
            ordered_pair_ids(list->targets[i].target_id, list->targets[j].target_id,
                             &pair->target_id_low, &pair->target_id_high);
            pair->frame_id = list->frame_id;
            pair->distance = manhattan_distance(centers_x[i], centers_y[i], centers_x[j], centers_y[j]);
            pair->valid = 1u;
        }
    }
    return 1u;
}

uint8_t ee_player_attention_candidate_view(const EE_GlobalTargetList *source_list,
                                           EE_GlobalTargetList *out_candidates)
{
    uint8_t i;

    if (!source_list || !out_candidates) {
        return 0u;
    }
    *out_candidates = *source_list;
    for (i = 0u; i < out_candidates->count && i < EE_MAX_TARGETS; ++i) {
        EE_GlobalTargetDescriptor *candidate = &out_candidates->targets[i];

        if (candidate->target_type != EE_SOCCER_TARGET_PLAYER) {
            candidate->valid = 0u;
        }
    }
    return 1u;
}

const EE_PlayerAttentionFeatures *ee_player_attention_find_features(
    const EE_PlayerAttentionFrame *frame, uint16_t target_id)
{
    uint8_t i;

    if (!frame) {
        return NULL;
    }
    for (i = 0u; i < frame->count && i < EE_MAX_TARGETS; ++i) {
        if (frame->features[i].valid && frame->features[i].target_id == target_id) {
            return &frame->features[i];
        }
    }
    return NULL;
}
