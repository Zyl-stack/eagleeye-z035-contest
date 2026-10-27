#include "a_fovea_adapter.h"
#include "register_map.h"

static uint16_t clamp_u32(uint32_t value, uint16_t maximum)
{
    return value > (uint32_t)maximum ? maximum : (uint16_t)value;
}

void ee_a_fovea_fixed_roi_from_center(uint16_t center_x,
                                      uint16_t center_y,
                                      uint16_t *out_roi_x,
                                      uint16_t *out_roi_y)
{
    uint32_t roi_x = center_x > A_FOVEA_ROI_WIDTH / 2u
                         ? (uint32_t)center_x - A_FOVEA_ROI_WIDTH / 2u
                         : 0u;
    uint32_t roi_y = center_y > A_FOVEA_ROI_HEIGHT / 2u
                         ? (uint32_t)center_y - A_FOVEA_ROI_HEIGHT / 2u
                         : 0u;

    if (out_roi_x) {
        *out_roi_x = clamp_u32(roi_x, A_FOVEA_ROI_X_MAX);
    }
    if (out_roi_y) {
        *out_roi_y = clamp_u32(roi_y, A_FOVEA_ROI_Y_MAX);
    }
}

uint8_t ee_a_fovea_mode_from_use_lanczos(uint8_t use_lanczos)
{
    return use_lanczos ? A_FOVEA_MODE_LANCZOS : A_FOVEA_MODE_BILINEAR;
}

int ee_a_fovea_adapt_request(const EE_FoveaRequest *source,
                             EE_FoveaRequest *adapted)
{
    uint32_t center_x;
    uint32_t center_y;

    if (!source || !adapted || !source->valid) {
        return -1;
    }
    if (source->enhance_mode > A_FOVEA_MODE_LANCZOS) {
        return -2;
    }

    center_x = (uint32_t)source->roi_x + (uint32_t)source->roi_w / 2u;
    center_y = (uint32_t)source->roi_y + (uint32_t)source->roi_h / 2u;

    *adapted = *source;
    ee_a_fovea_fixed_roi_from_center(clamp_u32(center_x, UINT16_MAX),
                                     clamp_u32(center_y, UINT16_MAX),
                                     &adapted->roi_x,
                                     &adapted->roi_y);
    adapted->roi_w = A_FOVEA_ROI_WIDTH;
    adapted->roi_h = A_FOVEA_ROI_HEIGHT;
    return 0;
}

int ee_a_fovea_build_write_sequence(
    const EE_FoveaRequest *source,
    uint32_t output_buffer_id,
    EE_AFoveaRegisterWrite writes[A_FOVEA_REGISTER_WRITE_COUNT])
{
    EE_FoveaRequest adapted;

    if (!writes) {
        return -1;
    }
    if (ee_a_fovea_adapt_request(source, &adapted) != 0) {
        return -2;
    }

    writes[0].offset = EAGLEEYE_REG_REQUEST_ID;
    writes[0].value = adapted.request_id;
    writes[1].offset = EAGLEEYE_REG_SOURCE_FRAME_ID;
    writes[1].value = adapted.source_frame_id;
    writes[2].offset = EAGLEEYE_REG_TARGET_ID;
    writes[2].value = (uint32_t)adapted.target_id;
    writes[3].offset = EAGLEEYE_REG_ROI_XY;
    writes[3].value = ee_pack_u16_pair(adapted.roi_x, adapted.roi_y);
    writes[4].offset = EAGLEEYE_REG_ROI_WH;
    writes[4].value = ee_pack_u16_pair(adapted.roi_w, adapted.roi_h);
    writes[5].offset = EAGLEEYE_REG_ENHANCE_MODE;
    writes[5].value = (uint32_t)adapted.enhance_mode;
    writes[6].offset = EAGLEEYE_REG_OUTPUT_BUFFER;
    writes[6].value = output_buffer_id;
    writes[7].offset = EAGLEEYE_REG_DEADLINE;
    writes[7].value = (uint32_t)adapted.deadline_frames;

    /* COMMIT is always the final write after every shadow parameter. */
    writes[8].offset = EAGLEEYE_REG_CONTROL;
    writes[8].value = EAGLEEYE_CTRL_ENABLE_MASK | EAGLEEYE_CTRL_COMMIT_MASK;
    return (int)A_FOVEA_REGISTER_WRITE_COUNT;
}
