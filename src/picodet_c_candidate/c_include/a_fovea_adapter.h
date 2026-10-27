#ifndef A_FOVEA_ADAPTER_H
#define A_FOVEA_ADAPTER_H

#include "eagleeye_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define A_FOVEA_INPUT_WIDTH       ((uint16_t)640u)
#define A_FOVEA_INPUT_HEIGHT      ((uint16_t)360u)
#define A_FOVEA_ROI_WIDTH         ((uint16_t)320u)
#define A_FOVEA_ROI_HEIGHT        ((uint16_t)180u)
#define A_FOVEA_ROI_X_MAX         ((uint16_t)320u)
#define A_FOVEA_ROI_Y_MAX         ((uint16_t)180u)

#define A_FOVEA_MODE_BYPASS       ((uint8_t)0u)
#define A_FOVEA_MODE_BILINEAR     ((uint8_t)1u)
#define A_FOVEA_MODE_LANCZOS      ((uint8_t)2u)

#define A_FOVEA_REGISTER_WRITE_COUNT ((uint8_t)9u)

typedef struct {
    uint32_t offset;
    uint32_t value;
} EE_AFoveaRegisterWrite;

void ee_a_fovea_fixed_roi_from_center(uint16_t center_x,
                                      uint16_t center_y,
                                      uint16_t *out_roi_x,
                                      uint16_t *out_roi_y);

uint8_t ee_a_fovea_mode_from_use_lanczos(uint8_t use_lanczos);

int ee_a_fovea_adapt_request(const EE_FoveaRequest *source,
                             EE_FoveaRequest *adapted);

int ee_a_fovea_build_write_sequence(
    const EE_FoveaRequest *source,
    uint32_t output_buffer_id,
    EE_AFoveaRegisterWrite writes[A_FOVEA_REGISTER_WRITE_COUNT]);

#ifdef __cplusplus
}
#endif

#endif
