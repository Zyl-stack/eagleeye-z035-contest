#ifndef ROI_MOTION_CONTROLLER_H
#define ROI_MOTION_CONTROLLER_H

#include "attention_scheduler.h"
#include "director_state.h"
#include "roi_smoother.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EE_ROI_MOTION_TRACK = 0,
    EE_ROI_MOTION_FAST_JUMP
} EE_ROIMotionMode;

typedef struct {
    uint16_t fast_jump_max_step_x;
    uint16_t fast_jump_max_step_y;
    uint8_t fast_jump_frames;
} EE_ROIMotionControllerConfig;

typedef struct {
    EE_DirectorState director_state;
    EE_DirectorEvent director_event;
    EE_SwitchReason switch_reason;
    uint16_t selected_target_id;
    uint16_t previous_selected_target_id;
} EE_ROIMotionInput;

typedef struct {
    EE_ROIMotionMode mode;
    EE_ROISmootherConfig smoother_config;
    uint8_t fast_jump_remaining;
    uint8_t triggered;
} EE_ROIMotionDecision;

typedef struct {
    EE_ROISmootherConfig track_config;
    EE_ROIMotionControllerConfig cfg;
    uint8_t fast_jump_remaining;
    uint8_t trigger_latched;
    uint8_t last_trigger_kind;
    uint16_t last_trigger_selected_target_id;
    uint16_t last_trigger_previous_target_id;
} EE_ROIMotionController;

EE_ROIMotionControllerConfig ee_roi_motion_controller_default_config(void);
void ee_roi_motion_controller_init(EE_ROIMotionController *controller,
                                   EE_ROISmootherConfig track_config,
                                   EE_ROIMotionControllerConfig cfg);
uint8_t ee_roi_motion_controller_step(EE_ROIMotionController *controller,
                                      const EE_ROIMotionInput *input,
                                      EE_ROIMotionDecision *out_decision);
const char *ee_roi_motion_mode_to_string(EE_ROIMotionMode mode);

#ifdef __cplusplus
}
#endif

#endif
