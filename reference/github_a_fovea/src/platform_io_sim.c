#include "platform_io.h"
#include "fake_data.h"
#include <string.h>

static EE_FoveaRequest g_active_request;
static EE_RefinedTargetDescriptor g_last_refined;
static uint8_t g_submitted;

void ee_platform_io_init(void)
{
    memset(&g_active_request, 0, sizeof(g_active_request));
    memset(&g_last_refined, 0, sizeof(g_last_refined));
    g_submitted = 0u;
}

uint8_t fovea_submit_request(const EE_FoveaRequest *request)
{
    if (!request || !request->valid) {
        return 0u;
    }

    g_active_request = *request;
    memset(&g_last_refined, 0, sizeof(g_last_refined));
    g_submitted = 1u;
    return 1u;
}

EE_FoveaStatus fovea_read_status(uint32_t current_frame_id)
{
    if (!g_submitted || !g_active_request.valid) {
        return EE_FOVEA_STATUS_IDLE;
    }

    if (g_active_request.deadline_frames > 0u &&
        current_frame_id > g_active_request.source_frame_id + g_active_request.deadline_frames) {
        return EE_FOVEA_STATUS_ERROR;
    }

    if (ee_fake_generate_refined_result(&g_active_request, current_frame_id, &g_last_refined)) {
        return EE_FOVEA_STATUS_DONE;
    }

    return EE_FOVEA_STATUS_BUSY;
}

uint8_t refined_result_available(uint32_t current_frame_id,
                                 EE_RefinedTargetDescriptor *out_refined)
{
    if (!out_refined) {
        return 0u;
    }

    if (fovea_read_status(current_frame_id) != EE_FOVEA_STATUS_DONE) {
        memset(out_refined, 0, sizeof(*out_refined));
        return 0u;
    }

    *out_refined = g_last_refined;
    return 1u;
}
