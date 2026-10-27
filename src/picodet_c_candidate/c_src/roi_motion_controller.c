#include "roi_motion_controller.h"

#include <string.h>

static uint8_t selected_target_changed(const EE_ROIMotionInput *input)
{
    return input->selected_target_id != EE_INVALID_TARGET_ID &&
           input->previous_selected_target_id != EE_INVALID_TARGET_ID &&
           input->selected_target_id != input->previous_selected_target_id;
}

static uint8_t fast_jump_trigger_kind(const EE_ROIMotionInput *input)
{
    if (input->selected_target_id == EE_INVALID_TARGET_ID) {
        return 0u;
    }
    if (input->director_event == EE_DIRECTOR_EVENT_PASS_STARTED) {
        return 1u;
    }
    if (input->director_event == EE_DIRECTOR_EVENT_BALL_RECOVERED) {
        return 2u;
    }
    if (input->switch_reason == EE_SWITCH_REASON_CURRENT_INVALID) {
        return 3u;
    }
    if (input->switch_reason == EE_SWITCH_REASON_CURRENT_LOST) {
        return 4u;
    }
    return selected_target_changed(input) ? 5u : 0u;
}

static uint8_t accept_fast_jump_trigger(EE_ROIMotionController *controller,
                                        const EE_ROIMotionInput *input,
                                        uint8_t trigger_kind)
{
    uint8_t is_new;

    if (trigger_kind == 0u) {
        controller->trigger_latched = 0u;
        return 0u;
    }
    is_new = !controller->trigger_latched ||
             controller->last_trigger_kind != trigger_kind ||
             controller->last_trigger_selected_target_id != input->selected_target_id ||
             controller->last_trigger_previous_target_id != input->previous_selected_target_id;
    if (is_new) {
        controller->trigger_latched = 1u;
        controller->last_trigger_kind = trigger_kind;
        controller->last_trigger_selected_target_id = input->selected_target_id;
        controller->last_trigger_previous_target_id = input->previous_selected_target_id;
    }
    return is_new;
}

EE_ROIMotionControllerConfig ee_roi_motion_controller_default_config(void)
{
    EE_ROIMotionControllerConfig cfg;

    cfg.fast_jump_max_step_x = 128u;
    cfg.fast_jump_max_step_y = 72u;
    cfg.fast_jump_frames = 3u;
    return cfg;
}

void ee_roi_motion_controller_init(EE_ROIMotionController *controller,
                                   EE_ROISmootherConfig track_config,
                                   EE_ROIMotionControllerConfig cfg)
{
    if (!controller) {
        return;
    }
    memset(controller, 0, sizeof(*controller));
    controller->track_config = track_config;
    controller->cfg = cfg;
}

uint8_t ee_roi_motion_controller_step(EE_ROIMotionController *controller,
                                      const EE_ROIMotionInput *input,
                                      EE_ROIMotionDecision *out_decision)
{
    uint8_t triggered;
    uint8_t trigger_kind;

    if (!controller || !input || !out_decision) {
        return 0u;
    }

    memset(out_decision, 0, sizeof(*out_decision));
    if (input->selected_target_id == EE_INVALID_TARGET_ID) {
        controller->fast_jump_remaining = 0u;
        controller->trigger_latched = 0u;
        out_decision->mode = EE_ROI_MOTION_TRACK;
        out_decision->smoother_config = controller->track_config;
        return 1u;
    }

    trigger_kind = fast_jump_trigger_kind(input);
    triggered = accept_fast_jump_trigger(controller, input, trigger_kind);
    if (triggered) {
        controller->fast_jump_remaining = controller->cfg.fast_jump_frames;
    }

    out_decision->triggered = triggered;
    out_decision->fast_jump_remaining = controller->fast_jump_remaining;
    if (controller->fast_jump_remaining > 0u) {
        out_decision->mode = EE_ROI_MOTION_FAST_JUMP;
        out_decision->smoother_config.max_step_x = controller->cfg.fast_jump_max_step_x;
        out_decision->smoother_config.max_step_y = controller->cfg.fast_jump_max_step_y;
        --controller->fast_jump_remaining;
    } else {
        out_decision->mode = EE_ROI_MOTION_TRACK;
        out_decision->smoother_config = controller->track_config;
    }
    return 1u;
}

const char *ee_roi_motion_mode_to_string(EE_ROIMotionMode mode)
{
    return mode == EE_ROI_MOTION_FAST_JUMP ? "FAST_JUMP" : "TRACK";
}
