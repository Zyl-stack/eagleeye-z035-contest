#include "fovea_feedback.h"

#include <string.h>

static uint8_t feedback_matches_request(const EE_FoveaFeedback *feedback,
                                        const EE_FoveaRequest *request)
{
    return feedback->request_id == request->request_id &&
           feedback->source_frame_id == request->source_frame_id &&
           feedback->target_id == request->target_id;
}

static void set_status(EE_FoveaFeedbackState *state,
                       const EE_FoveaFeedback *feedback,
                       EE_FoveaFeedbackStatus status)
{
    if (feedback) {
        state->last_feedback = *feedback;
    } else {
        memset(&state->last_feedback, 0, sizeof(state->last_feedback));
    }
    state->last_feedback.result_status = status;
    state->status = status;
}

void ee_fovea_feedback_init(EE_FoveaFeedbackState *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->status = EE_FOVEA_FEEDBACK_NONE;
}

uint8_t ee_fovea_feedback_begin(EE_FoveaFeedbackState *state,
                                const EE_FoveaRequest *request)
{
    if (!state || !request || !request->valid) {
        return 0u;
    }

    state->pending_request = *request;
    memset(&state->last_feedback, 0, sizeof(state->last_feedback));
    state->status = EE_FOVEA_FEEDBACK_PENDING;
    state->age_frames = 0u;
    state->pending = 1u;
    state->timed_out = 0u;
    return 1u;
}

void ee_fovea_feedback_advance(EE_FoveaFeedbackState *state,
                               uint32_t current_frame_id)
{
    uint32_t deadline_frame;

    if (!state || !state->pending_request.valid ||
        current_frame_id < state->pending_request.source_frame_id) {
        return;
    }

    state->age_frames = current_frame_id - state->pending_request.source_frame_id;
    if (!state->pending || state->pending_request.deadline_frames == 0u) {
        return;
    }

    deadline_frame = state->pending_request.source_frame_id +
                     (uint32_t)state->pending_request.deadline_frames;
    if (current_frame_id > deadline_frame) {
        state->pending = 0u;
        state->timed_out = 1u;
        set_status(state, NULL, EE_FOVEA_FEEDBACK_TIMEOUT);
        state->last_feedback.request_id = state->pending_request.request_id;
        state->last_feedback.source_frame_id = state->pending_request.source_frame_id;
        state->last_feedback.target_id = state->pending_request.target_id;
    }
}

uint8_t ee_fovea_feedback_receive(EE_FoveaFeedbackState *state,
                                  const EE_FoveaFeedback *feedback,
                                  uint32_t current_frame_id)
{
    if (!state || !feedback || !feedback->valid || !state->pending_request.valid) {
        if (state) {
            set_status(state, feedback, EE_FOVEA_FEEDBACK_INVALID);
        }
        return 0u;
    }

    ee_fovea_feedback_advance(state, current_frame_id);
    if (state->timed_out && feedback_matches_request(feedback, &state->pending_request)) {
        set_status(state, feedback, EE_FOVEA_FEEDBACK_TIMEOUT);
        return 0u;
    }

    if (feedback->request_id != state->pending_request.request_id) {
        if (feedback->request_id < state->pending_request.request_id) {
            set_status(state, feedback, EE_FOVEA_FEEDBACK_STALE);
        } else {
            set_status(state, feedback, EE_FOVEA_FEEDBACK_REQUEST_ID_MISMATCH);
        }
        return 0u;
    }
    if (feedback->source_frame_id != state->pending_request.source_frame_id) {
        set_status(state, feedback, EE_FOVEA_FEEDBACK_FRAME_ID_MISMATCH);
        return 0u;
    }
    if (feedback->target_id != state->pending_request.target_id) {
        set_status(state, feedback, EE_FOVEA_FEEDBACK_TARGET_ID_MISMATCH);
        return 0u;
    }
    if (!state->pending) {
        set_status(state, feedback, EE_FOVEA_FEEDBACK_DUPLICATE);
        return 0u;
    }

    state->latest_accepted = *feedback;
    state->latest_accepted.result_status = EE_FOVEA_FEEDBACK_ACCEPTED;
    set_status(state, &state->latest_accepted, EE_FOVEA_FEEDBACK_ACCEPTED);
    state->pending = 0u;
    state->timed_out = 0u;
    return 1u;
}

const char *ee_fovea_feedback_status_to_string(EE_FoveaFeedbackStatus status)
{
    switch (status) {
    case EE_FOVEA_FEEDBACK_NONE: return "NONE";
    case EE_FOVEA_FEEDBACK_PENDING: return "PENDING";
    case EE_FOVEA_FEEDBACK_ACCEPTED: return "ACCEPTED";
    case EE_FOVEA_FEEDBACK_TIMEOUT: return "TIMEOUT";
    case EE_FOVEA_FEEDBACK_STALE: return "STALE_FEEDBACK";
    case EE_FOVEA_FEEDBACK_REQUEST_ID_MISMATCH: return "REQUEST_ID_MISMATCH";
    case EE_FOVEA_FEEDBACK_FRAME_ID_MISMATCH: return "FRAME_ID_MISMATCH";
    case EE_FOVEA_FEEDBACK_TARGET_ID_MISMATCH: return "TARGET_ID_MISMATCH";
    case EE_FOVEA_FEEDBACK_INVALID: return "INVALID";
    case EE_FOVEA_FEEDBACK_DUPLICATE: return "DUPLICATE";
    default: return "UNKNOWN";
    }
}
