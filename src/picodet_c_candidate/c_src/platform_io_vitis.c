#include "platform_io_vitis.h"
#include "a_fovea_adapter.h"
#include "register_map.h"
#if !EAGLEEYE_FOVEA_BASEADDR_PLACEHOLDER
#include "xil_io.h"
#include "xil_printf.h"
#endif

#if !EAGLEEYE_FOVEA_BASEADDR_PLACEHOLDER
static uint32_t ee_reg_read(uint32_t offset)
{
    return Xil_In32((UINTPTR)(EAGLEEYE_FOVEA_BASEADDR + offset));
}
#endif

#if !EAGLEEYE_FOVEA_BASEADDR_PLACEHOLDER
static void ee_reg_write(uint32_t offset, uint32_t value)
{
    Xil_Out32((UINTPTR)(EAGLEEYE_FOVEA_BASEADDR + offset), value);
}
#endif

void ee_platform_io_vitis_init(void)
{
#if EAGLEEYE_FOVEA_BASEADDR_PLACEHOLDER
    return;
#else
    xil_printf("EagleEye Vitis platform IO init\r\n");
    ee_reg_write(EAGLEEYE_REG_CONTROL, EAGLEEYE_CTRL_ENABLE_MASK);
#endif
}

uintptr_t ee_fovea_get_baseaddr(void)
{
    return EAGLEEYE_FOVEA_BASEADDR;
}

int fovea_submit_request(const EE_FoveaRequest *req)
{
    EE_AFoveaRegisterWrite writes[A_FOVEA_REGISTER_WRITE_COUNT];
    int write_count;

    if (!req || !req->valid) {
        return -1;
    }

    write_count = ee_a_fovea_build_write_sequence(
        req, EAGLEEYE_FOVEA_DEFAULT_OUTPUT_BUFFER_ID, writes);
    if (write_count != (int)A_FOVEA_REGISTER_WRITE_COUNT) {
        return -3;
    }

#if EAGLEEYE_FOVEA_BASEADDR_PLACEHOLDER
    return -2;
#else
    uint8_t i;
    for (i = 0u; i < (uint8_t)write_count; ++i) {
        ee_reg_write(writes[i].offset, writes[i].value);
    }
    return 0;
#endif
}

int fovea_read_status(EE_FoveaAxiStatus *out_status)
{
    if (!out_status) {
        return -1;
    }

    out_status->raw_status = 0u;
    out_status->cycle_count = 0u;
    out_status->quality_flags = 0u;
    fovea_decode_status(0u, out_status);

#if EAGLEEYE_FOVEA_BASEADDR_PLACEHOLDER
    return -2;
#else
    out_status->raw_status = ee_reg_read(EAGLEEYE_REG_STATUS);
    out_status->cycle_count = ee_reg_read(EAGLEEYE_REG_CYCLE_COUNT);
    out_status->quality_flags = ee_reg_read(EAGLEEYE_REG_QUALITY_FLAGS);
    fovea_decode_status(out_status->raw_status, out_status);
    return 0;
#endif
}

void fovea_decode_status(uint32_t raw_status, EE_FoveaAxiStatus *out_status)
{
    if (!out_status) {
        return;
    }

    out_status->raw_status = raw_status;
    out_status->busy = (raw_status & EAGLEEYE_STATUS_BUSY_MASK) ? 1u : 0u;
    out_status->done = (raw_status & EAGLEEYE_STATUS_DONE_MASK) ? 1u : 0u;
    out_status->error = (raw_status & EAGLEEYE_STATUS_ERROR_MASK) ? 1u : 0u;
    out_status->active_mode =
        (uint8_t)((raw_status & EAGLEEYE_STATUS_ACTIVE_MODE_MASK) >>
                  EAGLEEYE_STATUS_ACTIVE_MODE_SHIFT);
}

void fovea_decode_quality_flags(uint32_t raw_quality_flags,
                                EE_FoveaQualityDecode *out_quality)
{
    if (!out_quality) {
        return;
    }

    out_quality->roi_clipped =
        (raw_quality_flags & EAGLEEYE_QUALITY_ROI_CLIPPED_MASK) ? 1u : 0u;
    out_quality->mode_fallback =
        (raw_quality_flags & EAGLEEYE_QUALITY_MODE_FALLBACK_MASK) ? 1u : 0u;
    out_quality->target_switch =
        (raw_quality_flags & EAGLEEYE_QUALITY_TARGET_SWITCH_MASK) ? 1u : 0u;
    out_quality->request_cancelled =
        (raw_quality_flags & EAGLEEYE_QUALITY_REQUEST_CANCELLED_MASK) ? 1u : 0u;
    out_quality->frame_safe_latched =
        (raw_quality_flags & EAGLEEYE_QUALITY_FRAME_SAFE_LATCHED_MASK) ? 1u : 0u;
    out_quality->output_buffer_valid =
        (raw_quality_flags & EAGLEEYE_QUALITY_OUTPUT_BUFFER_VALID_MASK) ? 1u : 0u;
}
