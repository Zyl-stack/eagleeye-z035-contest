#include "fovea_feedback_sim.h"

#include "register_map.h"
#include <string.h>

uint8_t ee_fovea_feedback_simulate(const EE_FoveaRequest *request,
                                   EE_FoveaFeedbackSimCase sim_case,
                                   EE_FoveaFeedback *out_feedback)
{
    if (!request || !request->valid || !out_feedback) {
        return 0u;
    }

    memset(out_feedback, 0, sizeof(*out_feedback));
    out_feedback->request_id = request->request_id;
    out_feedback->source_frame_id = request->source_frame_id;
    out_feedback->target_id = request->target_id;
    out_feedback->valid = 1u;
    out_feedback->result_status = EE_FOVEA_FEEDBACK_PENDING;

    switch (sim_case) {
    case EE_FOVEA_FEEDBACK_SIM_STALE:
        if (out_feedback->request_id > 0u) {
            --out_feedback->request_id;
        }
        break;
    case EE_FOVEA_FEEDBACK_SIM_REQUEST_MISMATCH:
        ++out_feedback->request_id;
        break;
    case EE_FOVEA_FEEDBACK_SIM_FRAME_MISMATCH:
        ++out_feedback->source_frame_id;
        break;
    case EE_FOVEA_FEEDBACK_SIM_TARGET_MISMATCH:
        ++out_feedback->target_id;
        break;
    case EE_FOVEA_FEEDBACK_SIM_QUALITY_FLAGS:
        out_feedback->quality_flags = EAGLEEYE_QUALITY_ROI_CLIPPED_MASK |
                                      EAGLEEYE_QUALITY_FRAME_SAFE_LATCHED_MASK |
                                      EAGLEEYE_QUALITY_OUTPUT_BUFFER_VALID_MASK;
        break;
    case EE_FOVEA_FEEDBACK_SIM_INVALID:
        out_feedback->valid = 0u;
        break;
    case EE_FOVEA_FEEDBACK_SIM_EXACT:
    default:
        break;
    }
    return 1u;
}
