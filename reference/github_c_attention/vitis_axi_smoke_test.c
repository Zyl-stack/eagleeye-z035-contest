#include "platform_io_vitis.h"
#include "a_fovea_adapter.h"
#include "xil_printf.h"

#define FOVEA_AXI_SMOKE_TIMEOUT_POLLS 1000000u
#define SOCCER_BALL_TEST_ID 1u

static const char *active_mode_to_string(uint8_t active_mode)
{
    switch (active_mode) {
    case 0u: return "BYPASS";
    case 1u: return "BILINEAR";
    case 2u: return "LANCZOS";
    default: return "RESERVED";
    }
}

int main(void)
{
    EE_FoveaRequest request;
    EE_FoveaAxiStatus status;
    EE_FoveaQualityDecode quality;
    int submit_result;
    int status_result;
    uint32_t poll_count;

    request.request_id = 1u;
    request.target_id = SOCCER_BALL_TEST_ID;
    request.source_frame_id = 1u;
    request.roi_x = 160u;
    request.roi_y = 90u;
    request.roi_w = A_FOVEA_ROI_WIDTH;
    request.roi_h = A_FOVEA_ROI_HEIGHT;
    request.enhance_mode = A_FOVEA_MODE_BILINEAR;
    request.deadline_frames = 12u;
    request.valid = 1u;

    xil_printf("EagleEye Soccer FOVEA REQUEST Smoke Test\r\n\r\n");
    ee_platform_io_vitis_init();
    xil_printf("base = 0x%08lx\r\n", (unsigned long)ee_fovea_get_baseaddr());
    xil_printf("request_id = %lu\r\n", (unsigned long)request.request_id);
    xil_printf("FOCUS target = BALL\r\n");
    xil_printf("target_id = %u (BALL)\r\n", request.target_id);
    xil_printf("source_frame_id = %lu\r\n", (unsigned long)request.source_frame_id);
    xil_printf("ROI=(%u,%u,%u,%u)\r\n",
               request.roi_x,
               request.roi_y,
               request.roi_w,
               request.roi_h);
    xil_printf("MODE=BILINEAR\r\n");

    submit_result = fovea_submit_request(&request);
    if (submit_result == 0) {
        xil_printf("FOVEA REQUEST SENT\r\n");
        xil_printf("WAITING FOR FRAME LATCH\r\n");
    } else {
        xil_printf("request submit skipped/failed, code=%d\r\n", submit_result);
    }

    for (poll_count = 0u; poll_count < FOVEA_AXI_SMOKE_TIMEOUT_POLLS; ++poll_count) {
        status_result = fovea_read_status(&status);
        if (status_result != 0) {
            xil_printf("STATUS read skipped/failed, code=%d\r\n", status_result);
            break;
        }
        if (status.done || status.error) {
            break;
        }
    }

    if (poll_count >= FOVEA_AXI_SMOKE_TIMEOUT_POLLS) {
        xil_printf("software timeout while polling STATUS\r\n");
    }

    if (status_result == 0 && status.done) {
        xil_printf("REQUEST LATCHED\r\n");
    }

    fovea_decode_quality_flags(status.quality_flags, &quality);
    xil_printf("STATUS = 0x%08lx\r\n", (unsigned long)status.raw_status);
    xil_printf("active_mode=%u (%s)\r\n",
               status.active_mode,
               active_mode_to_string(status.active_mode));
    xil_printf("busy=%u done=%u error=%u\r\n",
               status.busy,
               status.done,
               status.error);
    xil_printf("CYCLE_COUNT = 0x%08lx\r\n", (unsigned long)status.cycle_count);
    xil_printf("QUALITY_FLAGS = 0x%08lx\r\n", (unsigned long)status.quality_flags);
    xil_printf("ROI_CLIPPED=%u MODE_FALLBACK=%u TARGET_SWITCH=%u\r\n",
               quality.roi_clipped,
               quality.mode_fallback,
               quality.target_switch);
    xil_printf("REQUEST_CANCELLED=%u FRAME_SAFE_LATCHED=%u OUTPUT_BUFFER_VALID=%u\r\n",
               quality.request_cancelled,
               quality.frame_safe_latched,
               quality.output_buffer_valid);

#if EAGLEEYE_FOVEA_BASEADDR_PLACEHOLDER
    xil_printf("FOVEA AXI-LITE BSP BASEADDR MACRO NOT CONFIGURED\r\n");
    xil_printf("Refresh the Vitis platform from the A-side XSA and map EAGLEEYE_FOVEA_BASEADDR.\r\n");
#endif

    return 0;
}
