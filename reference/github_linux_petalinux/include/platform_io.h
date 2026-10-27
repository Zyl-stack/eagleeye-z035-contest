#ifndef PLATFORM_IO_H
#define PLATFORM_IO_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EE_FOVEA_STATUS_IDLE = 0,
    EE_FOVEA_STATUS_BUSY,
    EE_FOVEA_STATUS_DONE,
    EE_FOVEA_STATUS_ERROR
} EE_FoveaStatus;

void ee_platform_io_init(void);
uint8_t fovea_submit_request(const EE_FoveaRequest *request);
EE_FoveaStatus fovea_read_status(uint32_t current_frame_id);
uint8_t refined_result_available(uint32_t current_frame_id,
                                 EE_RefinedTargetDescriptor *out_refined);

#ifdef __cplusplus
}
#endif

#endif
