#ifndef FOVEA_STATS_H
#define FOVEA_STATS_H

#include "decision_snapshot.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EE_STATS_SOURCE_WIDTH  640u
#define EE_STATS_SOURCE_HEIGHT 360u
#define EE_STATS_ROI_WIDTH     320u
#define EE_STATS_ROI_HEIGHT    180u

typedef struct {
    uint32_t full_frame_pixels;
    uint32_t roi_pixels;
    float roi_ratio;
    float theoretical_pixel_reduction;
    uint32_t processed_frames;
    uint32_t bypass_frames;
    uint32_t bilinear_frames;
    uint32_t lanczos_frames;
    uint32_t target_switch_count;
    uint32_t ball_lost_count;
    uint32_t pass_handover_count;
    uint64_t selected_attention_sum;
    uint64_t roi_movement_sum;
    uint16_t previous_roi_x;
    uint16_t previous_roi_y;
    uint8_t has_previous_roi;
} EE_FoveaStats;

void ee_fovea_stats_init(EE_FoveaStats *stats);
void ee_fovea_stats_update(EE_FoveaStats *stats, const EE_DecisionSnapshot *snapshot);
float ee_fovea_stats_average_attention(const EE_FoveaStats *stats);
float ee_fovea_stats_average_roi_movement(const EE_FoveaStats *stats);

#ifdef __cplusplus
}
#endif

#endif
