#include "fovea_stats.h"

#include <string.h>

static uint16_t distance_u16(uint16_t a, uint16_t b)
{
    return a >= b ? (uint16_t)(a - b) : (uint16_t)(b - a);
}

void ee_fovea_stats_init(EE_FoveaStats *stats)
{
    if (!stats) return;
    memset(stats, 0, sizeof(*stats));
    stats->full_frame_pixels = EE_STATS_SOURCE_WIDTH * EE_STATS_SOURCE_HEIGHT;
    stats->roi_pixels = EE_STATS_ROI_WIDTH * EE_STATS_ROI_HEIGHT;
    stats->roi_ratio = (float)stats->roi_pixels / (float)stats->full_frame_pixels;
    stats->theoretical_pixel_reduction = 1.0f - stats->roi_ratio;
}

void ee_fovea_stats_update(EE_FoveaStats *stats, const EE_DecisionSnapshot *snapshot)
{
    if (!stats || !snapshot) return;
    ++stats->processed_frames;
    if (snapshot->enhance_mode == EE_ENHANCE_MODE_BYPASS) ++stats->bypass_frames;
    else if (snapshot->enhance_mode == EE_ENHANCE_MODE_BILINEAR) ++stats->bilinear_frames;
    else if (snapshot->enhance_mode == EE_ENHANCE_MODE_LANCZOS) ++stats->lanczos_frames;
    if (snapshot->switch_reason == EE_SWITCH_REASON_HIGHER_ATTENTION ||
        snapshot->switch_reason == EE_SWITCH_REASON_CURRENT_INVALID ||
        snapshot->switch_reason == EE_SWITCH_REASON_CURRENT_LOST) {
        ++stats->target_switch_count;
    }
    if (snapshot->director_event == EE_DIRECTOR_EVENT_BALL_LOST) ++stats->ball_lost_count;
    if (snapshot->director_event == EE_DIRECTOR_EVENT_PASS_STARTED) ++stats->pass_handover_count;
    stats->selected_attention_sum += snapshot->attention_total;
    if (snapshot->request_valid) {
        if (stats->has_previous_roi) {
            stats->roi_movement_sum += distance_u16(stats->previous_roi_x, snapshot->roi_x);
            stats->roi_movement_sum += distance_u16(stats->previous_roi_y, snapshot->roi_y);
        }
        stats->previous_roi_x = snapshot->roi_x;
        stats->previous_roi_y = snapshot->roi_y;
        stats->has_previous_roi = 1u;
    }
}

float ee_fovea_stats_average_attention(const EE_FoveaStats *stats)
{
    if (!stats || stats->processed_frames == 0u) return 0.0f;
    return (float)stats->selected_attention_sum / (float)stats->processed_frames;
}

float ee_fovea_stats_average_roi_movement(const EE_FoveaStats *stats)
{
    if (!stats || stats->processed_frames < 2u) return 0.0f;
    return (float)stats->roi_movement_sum / (float)(stats->processed_frames - 1u);
}
