#ifndef FOVEA_FEEDBACK_H
#define FOVEA_FEEDBACK_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* C-side transaction outcomes. They are not new AXI-Lite register values. */
typedef enum {
    EE_FOVEA_FEEDBACK_NONE = 0,
    EE_FOVEA_FEEDBACK_PENDING,
    EE_FOVEA_FEEDBACK_ACCEPTED,
    EE_FOVEA_FEEDBACK_TIMEOUT,
    EE_FOVEA_FEEDBACK_STALE,
    EE_FOVEA_FEEDBACK_REQUEST_ID_MISMATCH,
    EE_FOVEA_FEEDBACK_FRAME_ID_MISMATCH,
    EE_FOVEA_FEEDBACK_TARGET_ID_MISMATCH,
    EE_FOVEA_FEEDBACK_INVALID,
    EE_FOVEA_FEEDBACK_DUPLICATE
} EE_FoveaFeedbackStatus;

/* Minimal feedback record: no refined detection, bbox, class, or confidence. */
typedef struct {
    uint32_t request_id;
    uint32_t source_frame_id;
    uint16_t target_id;
    uint8_t valid;
    EE_FoveaFeedbackStatus result_status;
    uint32_t quality_flags;
} EE_FoveaFeedback;

typedef struct {
    EE_FoveaRequest pending_request;
    EE_FoveaFeedback latest_accepted;
    EE_FoveaFeedback last_feedback;
    EE_FoveaFeedbackStatus status;
    uint32_t age_frames;
    uint8_t pending;
    uint8_t timed_out;
} EE_FoveaFeedbackState;

void ee_fovea_feedback_init(EE_FoveaFeedbackState *state);
uint8_t ee_fovea_feedback_begin(EE_FoveaFeedbackState *state,
                                const EE_FoveaRequest *request);
void ee_fovea_feedback_advance(EE_FoveaFeedbackState *state,
                               uint32_t current_frame_id);
uint8_t ee_fovea_feedback_receive(EE_FoveaFeedbackState *state,
                                  const EE_FoveaFeedback *feedback,
                                  uint32_t current_frame_id);
const char *ee_fovea_feedback_status_to_string(EE_FoveaFeedbackStatus status);

#ifdef __cplusplus
}
#endif

#endif
