#include "roi_smoother.h"

#include <string.h>

static uint16_t clamp_u32(uint32_t value, uint16_t maximum)
{
    return value > maximum ? maximum : (uint16_t)value;
}

static uint16_t move_toward(uint16_t current, uint16_t target, uint16_t max_step)
{
    if (target > current) {
        uint16_t distance = (uint16_t)(target - current);
        return (uint16_t)(current + (distance > max_step ? max_step : distance));
    }
    {
        uint16_t distance = (uint16_t)(current - target);
        return (uint16_t)(current - (distance > max_step ? max_step : distance));
    }
}

EE_ROISmootherConfig ee_roi_smoother_default_config(void)
{
    EE_ROISmootherConfig cfg;
    cfg.max_step_x = 32u;
    cfg.max_step_y = 18u;
    return cfg;
}

void ee_roi_smoother_init(EE_ROISmoother *smoother, EE_ROISmootherConfig cfg)
{
    if (!smoother) return;
    memset(smoother, 0, sizeof(*smoother));
    smoother->cfg = cfg;
}

void ee_roi_smoother_set_config(EE_ROISmoother *smoother, EE_ROISmootherConfig cfg)
{
    if (!smoother) return;
    smoother->cfg = cfg;
}

uint8_t ee_roi_smoother_step(EE_ROISmoother *smoother,
                             const EE_FoveaRequest *desired,
                             EE_FoveaRequest *out_smoothed)
{
    uint32_t center_x;
    uint32_t center_y;
    uint16_t desired_x;
    uint16_t desired_y;

    if (!smoother || !desired || !out_smoothed || !desired->valid) return 0u;
    center_x = (uint32_t)desired->roi_x + (uint32_t)desired->roi_w / 2u;
    center_y = (uint32_t)desired->roi_y + (uint32_t)desired->roi_h / 2u;
    desired_x = center_x > EE_ROI_FIXED_WIDTH / 2u
                    ? clamp_u32(center_x - EE_ROI_FIXED_WIDTH / 2u, EE_ROI_X_MAX)
                    : 0u;
    desired_y = center_y > EE_ROI_FIXED_HEIGHT / 2u
                    ? clamp_u32(center_y - EE_ROI_FIXED_HEIGHT / 2u, EE_ROI_Y_MAX)
                    : 0u;

    if (!smoother->initialized) {
        smoother->roi_x = desired_x;
        smoother->roi_y = desired_y;
        smoother->initialized = 1u;
    } else {
        smoother->roi_x = move_toward(smoother->roi_x, desired_x, smoother->cfg.max_step_x);
        smoother->roi_y = move_toward(smoother->roi_y, desired_y, smoother->cfg.max_step_y);
    }
    *out_smoothed = *desired;
    out_smoothed->roi_x = clamp_u32(smoother->roi_x, EE_ROI_X_MAX);
    out_smoothed->roi_y = clamp_u32(smoother->roi_y, EE_ROI_Y_MAX);
    out_smoothed->roi_w = EE_ROI_FIXED_WIDTH;
    out_smoothed->roi_h = EE_ROI_FIXED_HEIGHT;
    return 1u;
}
