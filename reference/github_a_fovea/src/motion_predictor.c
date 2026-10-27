#include "motion_predictor.h"
#include <string.h>

static EE_GlobalTargetDescriptor g_previous[EE_MAX_TARGETS];

static uint16_t clamp_coordinate(int32_t value)
{
    if (value < 0) return 0u;
    if (value > 65535) return 65535u;
    return (uint16_t)value;
}

void ee_motion_predictor_init(void)
{
    memset(g_previous, 0, sizeof(g_previous));
}

void ee_motion_predictor_update(const EE_GlobalTargetList *list)
{
    uint8_t i;
    if (!list) return;
    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].valid) g_previous[i] = list->targets[i];
    }
}

uint8_t ee_motion_predict(const EE_GlobalTargetDescriptor *target,
                          EE_MotionPrediction *out_prediction)
{
    uint8_t i;
    if (!target || !out_prediction) return 0u;
    memset(out_prediction, 0, sizeof(*out_prediction));
    out_prediction->target_id = target->target_id;
    out_prediction->source_frame_id = target->frame_id;
    for (i = 0u; i < EE_MAX_TARGETS; ++i) {
        if (g_previous[i].valid && g_previous[i].target_id == target->target_id &&
            g_previous[i].frame_id < target->frame_id) {
            out_prediction->predicted_x = clamp_coordinate((int32_t)target->x + target->vx);
            out_prediction->predicted_y = clamp_coordinate((int32_t)target->y + target->vy);
            out_prediction->prediction_valid = 1u;
            return 1u;
        }
    }
    return 0u;
}
