#include "attention_score.h"

static float clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

EE_AttentionWeights ee_attention_default_weights(void)
{
    EE_AttentionWeights w;
    w.zone = 0.30f;
    w.motion = 0.15f;
    w.approach = 0.25f;
    w.small_target = 0.15f;
    w.uncertainty = 0.25f;
    w.aging = 0.03f;
    return w;
}

float ee_attention_score_compute(const EE_GlobalTargetDescriptor *target,
                                 const EE_AttentionWeights *weights,
                                 uint16_t waiting_frames)
{
    float zone_score;
    float motion_score;
    float approach_score;
    float small_score;
    float uncertainty_score;
    float aging_score;
    float area;

    if (!target || !weights || !target->valid) {
        return 0.0f;
    }

    switch ((EE_Zone)target->zone_id) {
    case EE_ZONE_EGO_LANE: zone_score = 1.00f; break;
    case EE_ZONE_CENTER: zone_score = 0.80f; break;
    case EE_ZONE_LEFT:
    case EE_ZONE_RIGHT: zone_score = 0.45f; break;
    default: zone_score = 0.10f; break;
    }

    motion_score = clampf(((float)(target->vx < 0 ? -target->vx : target->vx) +
                           (float)(target->vy < 0 ? -target->vy : target->vy)) / 12.0f,
                          0.0f, 1.0f);

    approach_score = clampf((float)target->approach_level / 100.0f, 0.0f, 1.0f);

    area = (float)target->w * (float)target->h;
    small_score = clampf((8000.0f - area) / 8000.0f, 0.0f, 1.0f);
    uncertainty_score = clampf((100.0f - (float)target->coarse_conf) / 100.0f, 0.0f, 1.0f);
    aging_score = clampf((float)waiting_frames / 30.0f, 0.0f, 1.0f);

    return weights->zone * zone_score +
           weights->motion * motion_score +
           weights->approach * approach_score +
           weights->small_target * small_score +
           weights->uncertainty * uncertainty_score +
           weights->aging * aging_score;
}

void ee_attention_score_update_list(EE_GlobalTargetList *list,
                                    const EE_AttentionWeights *weights,
                                    const uint16_t waiting_frames[EE_MAX_TARGETS])
{
    uint8_t i;
    if (!list || !weights) {
        return;
    }

    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        uint16_t wait = waiting_frames ? waiting_frames[i] : 0u;
        list->targets[i].attention_score = (uint32_t)(ee_attention_score_compute(&list->targets[i], weights, wait) * 1000.0f);
    }
}
