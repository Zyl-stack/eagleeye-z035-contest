#include "attention.h"

static uint8_t abs_i16_to_u8(int16_t value)
{
    int16_t abs_value = (value < 0) ? (int16_t)(-value) : value;
    return (abs_value > 255) ? 255u : (uint8_t)abs_value;
}

static uint8_t small_target_score(uint16_t w, uint16_t h)
{
    uint32_t area = (uint32_t)w * (uint32_t)h;

    if (area >= 12000u) {
        return 0u;
    }
    if (area <= 1000u) {
        return 255u;
    }

    return (uint8_t)((12000u - area) * 255u / 11000u);
}

static uint8_t zone_score(uint8_t zone_id)
{
    switch ((EE_Zone)zone_id) {
    case EE_ZONE_EGO_LANE: return 255u;
    case EE_ZONE_CENTER: return 200u;
    case EE_ZONE_LEFT:
    case EE_ZONE_RIGHT: return 90u;
    default: return 20u;
    }
}

static uint8_t soccer_goal_score(uint8_t field_zone)
{
    switch ((EE_FieldZone)field_zone) {
    case EE_FIELD_GOAL_ZONE: return 255u;
    case EE_FIELD_ATTACK_ZONE: return 180u;
    default: return 40u;
    }
}

void ee_attention_compute_debug_waiting(const EE_GlobalTargetDescriptor *target,
                                        const EE_AttentionConfig *config,
                                        uint16_t waiting_frames,
                                        EE_AttentionDebug *out_debug);

EE_AttentionConfig ee_attention_default_config(void)
{
    EE_AttentionConfig config;
    config.w_zone = EE_ATTENTION_WEIGHT_ZONE;
    config.w_motion = EE_ATTENTION_WEIGHT_MOTION;
    config.w_approach = EE_ATTENTION_WEIGHT_APPROACH;
    config.w_small = EE_ATTENTION_WEIGHT_SMALL;
    config.w_uncertainty = EE_ATTENTION_WEIGHT_UNCERTAINTY;
    config.w_ball = EE_ATTENTION_WEIGHT_BALL;
    config.w_goal = EE_ATTENTION_WEIGHT_GOAL;
    config.w_ball_near = EE_ATTENTION_WEIGHT_BALL_NEAR;
    config.w_possession = EE_ATTENTION_WEIGHT_BALL_NEAR;
    config.w_aging = EE_ATTENTION_WEIGHT_AGING;
    return config;
}

static uint32_t compute_score_waiting(const EE_GlobalTargetDescriptor *target,
                                      const EE_AttentionConfig *config,
                                      uint16_t waiting_frames)
{
    EE_AttentionDebug debug;
    ee_attention_compute_debug_waiting(target, config, waiting_frames, &debug);
    return debug.total_attention;
}

uint32_t ee_attention_compute_score(const EE_GlobalTargetDescriptor *target,
                                    const EE_AttentionConfig *config)
{
    return compute_score_waiting(target, config, 0u);
}

uint32_t ee_attention_compute_score_waiting(const EE_GlobalTargetDescriptor *target,
                                            const EE_AttentionConfig *config,
                                            uint16_t waiting_frames)
{
    return compute_score_waiting(target, config, waiting_frames);
}

void ee_attention_compute_debug_waiting(const EE_GlobalTargetDescriptor *target,
                                        const EE_AttentionConfig *config,
                                        uint16_t waiting_frames,
                                        EE_AttentionDebug *out_debug)
{
    uint8_t zone;
    uint8_t motion;
    uint8_t approach;
    uint8_t small;
    uint8_t uncertainty;
    uint8_t ball_priority;
    uint8_t goal;
    uint8_t ball_near;
    uint8_t aging;

    if (!out_debug) {
        return;
    }

    out_debug->zone_score = 0u;
    out_debug->ball_priority_score = 0u;
    out_debug->goal_zone_score = 0u;
    out_debug->motion_score = 0u;
    out_debug->approach_score = 0u;
    out_debug->small_target_score = 0u;
    out_debug->uncertainty_score = 0u;
    out_debug->ball_proximity_score = 0u;
    out_debug->aging_score = 0u;
    out_debug->total_attention = 0u;

    if (!target || !config || !target->valid) {
        return;
    }

    if (target->target_type != EE_SOCCER_TARGET_UNKNOWN || target->is_ball ||
        target->field_zone != EE_FIELD_NORMAL) {
        zone = target->field_zone == EE_FIELD_GOAL_ZONE ? 255u :
               (target->field_zone == EE_FIELD_ATTACK_ZONE ? 180u : 40u);
    } else {
        zone = zone_score(target->zone_id);
    }
    motion = (uint8_t)((abs_i16_to_u8(target->vx) + abs_i16_to_u8(target->vy)) / 2u);
    approach = target->approach_level;
    small = target->small_target_score != 0u ? target->small_target_score : small_target_score(target->w, target->h);
    uncertainty = target->uncertainty != 0u ? target->uncertainty : (uint8_t)(255u - target->coarse_conf);
    /* Once a carrier estimate exists, the player is the primary focus and
       the ball remains a visible supporting candidate. */
    ball_priority = (target->is_ball || target->target_type == EE_SOCCER_TARGET_BALL) &&
                    target->possession_score == 0u ? 255u : 0u;
    goal = soccer_goal_score(target->field_zone);
    ball_near = target->is_ball ? 0u :
                (target->possession_score != 0u ? target->possession_score :
                 (target->ball_distance >= 200u ? 0u : (uint8_t)(255u - target->ball_distance)));
    aging = (uint8_t)((waiting_frames >= 30u) ? 255u : (waiting_frames * 255u) / 30u);

    out_debug->zone_score = zone;
    out_debug->ball_priority_score = ball_priority;
    out_debug->goal_zone_score = goal;
    out_debug->motion_score = motion;
    out_debug->approach_score = approach;
    out_debug->small_target_score = small;
    out_debug->uncertainty_score = uncertainty;
    out_debug->ball_proximity_score = ball_near;
    out_debug->aging_score = aging;
    out_debug->total_attention =
        (uint32_t)config->w_ball * ball_priority +
        (uint32_t)config->w_motion * motion +
        (uint32_t)config->w_goal * goal +
        (uint32_t)config->w_small * small +
        (uint32_t)config->w_uncertainty * uncertainty +
        (uint32_t)config->w_possession * ball_near +
        (uint32_t)config->w_aging * aging;
}

void ee_attention_compute_debug(const EE_GlobalTargetDescriptor *target,
                                const EE_AttentionConfig *config,
                                EE_AttentionDebug *out_debug)
{
    ee_attention_compute_debug_waiting(target, config, 0u, out_debug);
}

void ee_attention_update_scores(EE_GlobalTargetDescriptor targets[],
                                uint8_t target_count,
                                const EE_AttentionConfig *config)
{
    uint8_t i;
    if (!targets || !config) {
        return;
    }

    for (i = 0u; i < target_count && i < EE_MAX_TARGETS; ++i) {
        targets[i].attention_score = ee_attention_compute_score(&targets[i], config);
    }
}

int16_t ee_attention_select_argmax(const EE_GlobalTargetDescriptor targets[],
                                   uint8_t target_count)
{
    uint8_t i;
    int16_t best_index = -1;
    uint32_t best_score = 0u;

    if (!targets) {
        return -1;
    }

    for (i = 0u; i < target_count && i < EE_MAX_TARGETS; ++i) {
        if (!targets[i].valid) {
            continue;
        }
        if (best_index < 0 || targets[i].attention_score > best_score) {
            best_index = (int16_t)i;
            best_score = targets[i].attention_score;
        }
    }

    return best_index;
}
