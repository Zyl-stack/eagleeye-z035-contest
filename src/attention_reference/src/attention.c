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

EE_AttentionConfig ee_attention_default_config(void)
{
    EE_AttentionConfig config;
    config.w_zone = 3u;
    config.w_motion = 1u;
    config.w_approach = 3u;
    config.w_small = 2u;
    config.w_uncertainty = 3u;
    return config;
}

uint32_t ee_attention_compute_score(const EE_GlobalTargetDescriptor *target,
                                    const EE_AttentionConfig *config)
{
    uint8_t zone;
    uint8_t motion;
    uint8_t approach;
    uint8_t small;
    uint8_t uncertainty;

    if (!target || !config || !target->valid) {
        return 0u;
    }

    zone = zone_score(target->zone_id);
    motion = (uint8_t)((abs_i16_to_u8(target->vx) + abs_i16_to_u8(target->vy)) / 2u);
    approach = target->approach_level;
    small = small_target_score(target->w, target->h);
    uncertainty = (uint8_t)(255u - target->coarse_conf);

    return (uint32_t)config->w_zone * zone +
           (uint32_t)config->w_motion * motion +
           (uint32_t)config->w_approach * approach +
           (uint32_t)config->w_small * small +
           (uint32_t)config->w_uncertainty * uncertainty;
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
