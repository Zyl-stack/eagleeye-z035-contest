#include "transaction.h"
#include <stdio.h>
#include <string.h>

void ee_transaction_init(EE_TransactionContext *ctx)
{
    if (!ctx) {
        return;
    }
    memset(ctx, 0, sizeof(*ctx));
    ctx->risk_state = EE_RISK_NORMAL;
}

void ee_transaction_start(EE_TransactionContext *ctx, const EE_FoveaRequest *request)
{
    if (!ctx || !request || !request->valid) {
        return;
    }

    ctx->current_request = *request;
    memset(&ctx->refined_result, 0, sizeof(ctx->refined_result));
    ctx->risk_action = EE_CONFIRM_NONE;
    ctx->risk_action_pending = 0u;
    ctx->timeout = 0u;
}

void ee_transaction_release(EE_TransactionContext *ctx)
{
    if (!ctx) {
        return;
    }

    ctx->current_request.valid = 0u;
    memset(&ctx->refined_result, 0, sizeof(ctx->refined_result));
    ctx->risk_action = EE_CONFIRM_NONE;
    ctx->risk_action_pending = 0u;
    ctx->timeout = 0u;
}

uint8_t ee_refined_result_matches_request(const EE_FoveaRequest *request,
                                          const EE_RefinedTargetDescriptor *received)
{
    uint8_t matched = 1u;

    if (!request || !request->valid || !received || !received->valid) {
        return 0u;
    }

    if (received->request_id != request->request_id ||
        received->target_id != request->target_id ||
        received->frame_id != request->source_frame_id) {
        printf("STALE RESULT REJECTED\n");
        if (received->request_id != request->request_id) {
            printf("reason=request_id mismatch\n");
            matched = 0u;
        }
        if (received->target_id != request->target_id) {
            printf("reason=target_id mismatch\n");
            matched = 0u;
        }
        if (received->frame_id != request->source_frame_id) {
            printf("reason=frame_id mismatch\n");
            matched = 0u;
        }
        printf("current_req=%u\n", (unsigned)request->request_id);
        printf("received_req=%u\n", (unsigned)received->request_id);
        printf("current_target=%u received_target=%u\n",
               request->target_id,
               received->target_id);
        printf("current_frame=%u received_frame=%u\n",
               (unsigned)request->source_frame_id,
               (unsigned)received->frame_id);
    }

    return matched;
}

uint8_t ee_transaction_accept_refined(EE_TransactionContext *ctx,
                                      const EE_RefinedTargetDescriptor *received)
{
    if (!ctx || !received || !received->valid || !ctx->current_request.valid) {
        return 0u;
    }

    if (!ee_refined_result_matches_request(&ctx->current_request, received)) {
        return 0u;
    }

    ctx->refined_result = *received;
    ctx->risk_state = received->confirmed_risk_level;
    if (received->focus_state != EE_FOCUS_BACKGROUND) {
        ctx->refined_result.focus_state = received->focus_state;
    } else {
        ctx->refined_result.focus_state = (received->confirmed_risk_level == EE_RISK_DANGER) ?
                                          EE_FOCUS_HIGHLIGHT : EE_FOCUS_FOCUS;
    }
    ctx->risk_action = received->confirm_state;
    ctx->risk_action_pending = (received->confirm_state != EE_CONFIRM_NONE) ? 1u : 0u;
    ctx->timeout = 0u;
    return 1u;
}

uint8_t ee_transaction_consume_action(EE_TransactionContext *ctx,
                                      EE_ConfirmState *out_action)
{
    if (!ctx || !out_action || !ctx->risk_action_pending) {
        return 0u;
    }

    *out_action = ctx->risk_action;
    ctx->risk_action_pending = 0u;
    ctx->risk_action = EE_CONFIRM_NONE;
    return 1u;
}

void ee_transaction_mark_timeout(EE_TransactionContext *ctx)
{
    if (!ctx) {
        return;
    }

    ctx->timeout = 1u;
    ctx->risk_action = EE_CONFIRM_TIMEOUT;
    ctx->risk_action_pending = 1u;
}
