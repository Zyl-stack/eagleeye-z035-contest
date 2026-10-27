#include "risk.h"

static float clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

float ee_confirmed_risk_score(const EE_GlobalTargetDescriptor *global,
                              const EE_RefinedTargetDescriptor *refined)
{
    float class_score = 0.0f;
    float zone_score = 0.0f;
    float approach_score = 0.0f;
    float confidence_score;

    if (!global || !refined || !global->valid || !refined->valid) {
        return 0.0f;
    }

    switch (refined->refined_class) {
    case EE_CLASS_PEDESTRIAN: class_score = 1.00f; break;
    case EE_CLASS_BICYCLE: class_score = 0.85f; break;
    case EE_CLASS_CAR: class_score = 0.65f; break;
    case EE_CLASS_CONE: class_score = 0.35f; break;
    case EE_CLASS_BACKGROUND: class_score = 0.05f; break;
    default: class_score = 0.20f; break;
    }

    switch ((EE_Zone)global->zone_id) {
    case EE_ZONE_EGO_LANE: zone_score = 1.00f; break;
    case EE_ZONE_CENTER: zone_score = 0.75f; break;
    case EE_ZONE_LEFT:
    case EE_ZONE_RIGHT: zone_score = 0.45f; break;
    default: zone_score = 0.10f; break;
    }

    approach_score = clampf((float)global->approach_level / 100.0f, 0.0f, 1.0f);

    confidence_score = (float)refined->refined_confidence / 100.0f;
    return 0.40f * class_score + 0.25f * confidence_score +
           0.20f * zone_score + 0.15f * approach_score;
}

EE_RiskLevel ee_risk_level_from_score(float score)
{
    if (score >= 0.78f) return EE_RISK_DANGER;
    if (score >= 0.58f) return EE_RISK_WARNING;
    if (score >= 0.35f) return EE_RISK_ATTENTION;
    return EE_RISK_NORMAL;
}
