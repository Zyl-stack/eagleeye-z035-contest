#ifndef FOVEA_FEEDBACK_SIM_H
#define FOVEA_FEEDBACK_SIM_H

#include "fovea_feedback.h"

#ifdef __cplusplus
extern "C" {
#endif

/* PC-only test input generator. It does not simulate image processing. */
typedef enum {
    EE_FOVEA_FEEDBACK_SIM_EXACT = 0,
    EE_FOVEA_FEEDBACK_SIM_STALE,
    EE_FOVEA_FEEDBACK_SIM_REQUEST_MISMATCH,
    EE_FOVEA_FEEDBACK_SIM_FRAME_MISMATCH,
    EE_FOVEA_FEEDBACK_SIM_TARGET_MISMATCH,
    EE_FOVEA_FEEDBACK_SIM_QUALITY_FLAGS,
    EE_FOVEA_FEEDBACK_SIM_INVALID
} EE_FoveaFeedbackSimCase;

uint8_t ee_fovea_feedback_simulate(const EE_FoveaRequest *request,
                                   EE_FoveaFeedbackSimCase sim_case,
                                   EE_FoveaFeedback *out_feedback);

#ifdef __cplusplus
}
#endif

#endif
