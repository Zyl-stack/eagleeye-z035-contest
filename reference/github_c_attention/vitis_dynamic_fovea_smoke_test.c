#include "a_fovea_adapter.h"
#include "attention.h"
#include "c_config.h"
#include "enhance_policy.h"
#include "platform_io_vitis.h"
#include "roi_smoother.h"
#include "xil_printf.h"

#define DYNAMIC_FOVEA_TIMEOUT_POLLS 1000000u

/* This smoke test needs policy settings, not a live Scheduler instance. */
static EE_CConfig dynamic_test_config(void)
{
    EE_CConfig config = {0};

    config.attention = ee_attention_default_config();
    config.roi_smoother = ee_roi_smoother_default_config();
    config.scheduler.timeout_frames = 12u;
    config.high_attention_threshold = 1500u;
    config.small_target_threshold = 255u;
    config.fast_motion_threshold = 255u;
    config.default_enhance_mode = EE_ENHANCE_MODE_BILINEAR;
    config.high_quality_enhance_mode = EE_ENHANCE_MODE_LANCZOS;
    return config;
}

static EE_GlobalTargetDescriptor make_test_target(uint32_t frame_id,
                                                   uint16_t target_id,
                                                   uint8_t is_ball,
                                                   uint16_t x,
                                                   uint16_t y,
                                                   uint16_t w,
                                                   uint16_t h)
{
    EE_GlobalTargetDescriptor target = {0};
    target.target_id = target_id;
    target.frame_id = frame_id;
    target.x = x;
    target.y = y;
    target.w = w;
    target.h = h;
    target.target_type = is_ball ? EE_SOCCER_TARGET_BALL : EE_SOCCER_TARGET_PLAYER;
    target.is_ball = is_ball;
    target.coarse_conf = is_ball ? 96u : 90u;
    target.valid = 1u;
    target.field_zone = EE_FIELD_NORMAL;
    return target;
}

static int wait_for_status(EE_FoveaAxiStatus *status)
{
    uint32_t poll_count;
    int read_result;

    for (poll_count = 0u; poll_count < DYNAMIC_FOVEA_TIMEOUT_POLLS; ++poll_count) {
        read_result = fovea_read_status(status);
        if (read_result != 0) return read_result;
        if (status->done || status->error) return status->error ? -3 : 0;
    }
    return -4;
}

static void run_dynamic_frame(uint32_t frame_id,
                              EE_GlobalTargetDescriptor *target,
                              EE_ROISmoother *smoother,
                              EE_CConfig *config)
{
    EE_FoveaRequest desired = {0};
    EE_FoveaRequest smoothed = {0};
    EE_FoveaRequest adapted = {0};
    EE_AttentionBreakdown breakdown = {0};
    EE_EnhancePolicyInput policy_input = {0};
    EE_EnhanceDecision decision;
    EE_FoveaAxiStatus status = {0};
    uint8_t has_target = target && target->valid;
    int submit_result;
    int wait_result;

    if (has_target) {
        desired.request_id = frame_id;
        desired.target_id = target->target_id;
        desired.source_frame_id = frame_id;
        desired.roi_x = target->x;
        desired.roi_y = target->y;
        desired.roi_w = target->w;
        desired.roi_h = target->h;
        desired.deadline_frames = config->scheduler.timeout_frames;
        desired.valid = 1u;
        ee_attention_compute_breakdown(target, &config->attention, 0u, &breakdown);
        policy_input.selected_target = target;
        policy_input.attention = &breakdown;
        policy_input.possession_target_id = EE_INVALID_TARGET_ID;
    }

    decision = ee_enhance_policy_decide(&policy_input, config);
    desired.enhance_mode = (uint8_t)decision.mode;

    if (has_target && ee_roi_smoother_step(smoother, &desired, &smoothed)) {
        if (ee_a_fovea_adapt_request(&smoothed, &adapted) != 0) {
            xil_printf("DYNAMIC FRAME %lu ERROR: adapter rejected request\r\n",
                       (unsigned long)frame_id);
            return;
        }
        submit_result = fovea_submit_request(&adapted);
    } else {
        smoothed = desired;
        submit_result = -1;
    }

    xil_printf("frame_id=%lu request_id=%lu target_id=%u roi=(%u,%u,%u,%u) "
               "enhance_mode=%u (%s) reason=%s\r\n",
               (unsigned long)frame_id,
               (unsigned long)smoothed.request_id,
               smoothed.target_id,
               adapted.roi_x, adapted.roi_y, adapted.roi_w, adapted.roi_h,
               decision.mode, ee_enhance_mode_to_string(decision.mode),
               ee_enhance_reason_to_string(decision.reason));

    if (!has_target || decision.mode == EE_ENHANCE_MODE_BYPASS) {
        xil_printf("status=BYPASS_NO_VALID_TARGET\r\n\r\n");
        return;
    }
    if (submit_result != 0) {
        xil_printf("status=REQUEST_NOT_SENT code=%d\r\n\r\n", submit_result);
        return;
    }

    xil_printf("REQUEST SENT\r\n");
    xil_printf("WAITING FOR FRAME LATCH\r\n");
    wait_result = wait_for_status(&status);
    if (wait_result == 0) {
        xil_printf("REQUEST LATCHED\r\n");
        xil_printf("status=done busy=%u error=%u active_mode=%u\r\n\r\n",
                   status.busy, status.error, status.active_mode);
    } else if (wait_result == -4) {
        xil_printf("ERROR: software timeout waiting for STATUS.done\r\n\r\n");
    } else if (wait_result == -3) {
        xil_printf("ERROR: Fovea STATUS.error asserted\r\n\r\n");
    } else {
        xil_printf("ERROR: STATUS read failed code=%d\r\n\r\n", wait_result);
    }
}

int main(void)
{
    EE_CConfig config = dynamic_test_config();
    EE_ROISmoother smoother;
    EE_GlobalTargetDescriptor target;

    /* Frames 1-3 exercise normal tracking; frame 4 demonstrates LANCZOS. */
    ee_roi_smoother_init(&smoother, config.roi_smoother);

    xil_printf("EagleEye Dynamic Fovea Smoke Test\r\n");
    xil_printf("base = 0x%08lx\r\n", (unsigned long)ee_fovea_get_baseaddr());
    xil_printf("NOT HARDWARE VERIFIED until REQUEST LATCHED is observed on UART.\r\n\r\n");
    ee_platform_io_vitis_init();

    target = make_test_target(1u, 11u, 0u, 300u, 150u, 40u, 60u);
    run_dynamic_frame(1u, &target, &smoother, &config);
    target.frame_id = 2u; target.x = 340u;
    run_dynamic_frame(2u, &target, &smoother, &config);
    target.frame_id = 3u; target.x = 380u;
    run_dynamic_frame(3u, &target, &smoother, &config);
    target = make_test_target(4u, 1u, 1u, 400u, 150u, 12u, 12u);
    run_dynamic_frame(4u, &target, &smoother, &config);
    target.frame_id = 5u; target.valid = 0u;
    run_dynamic_frame(5u, &target, &smoother, &config);
    return 0;
}
