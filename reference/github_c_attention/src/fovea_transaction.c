#include "fovea_transaction.h"

#include <string.h>

static uint8_t parameters_match(const EE_FoveaRequest *left,
                                const EE_FoveaRequest *right)
{
    return left->valid == right->valid &&
           left->target_id == right->target_id &&
           left->source_frame_id == right->source_frame_id &&
           left->roi_x == right->roi_x &&
           left->roi_y == right->roi_y &&
           left->roi_w == right->roi_w &&
           left->roi_h == right->roi_h &&
           left->enhance_mode == right->enhance_mode &&
           left->deadline_frames == right->deadline_frames;
}

static uint32_t allocate_request_id(EE_FoveaTransactionBuilder *builder)
{
    uint32_t request_id;

    if (builder->next_request_id == 0u) {
        builder->next_request_id = 1u;
    }
    request_id = builder->next_request_id;
    ++builder->next_request_id;
    if (builder->next_request_id == 0u) {
        builder->next_request_id = 1u;
    }
    return request_id;
}

void ee_fovea_transaction_init(EE_FoveaTransactionBuilder *builder)
{
    if (!builder) {
        return;
    }
    memset(builder, 0, sizeof(*builder));
    builder->next_request_id = 1u;
}

EE_FoveaTransactionResult ee_fovea_transaction_build(
    EE_FoveaTransactionBuilder *builder,
    const EE_FoveaRequest *parameters,
    uint32_t current_source_frame_id,
    EE_FoveaRequest *out_transaction)
{
    EE_FoveaRequest candidate;

    if (!builder || !parameters || !parameters->valid || !out_transaction) {
        return EE_FOVEA_TRANSACTION_INVALID;
    }

    candidate = *parameters;
    candidate.source_frame_id = current_source_frame_id;
    if (builder->last_transaction.valid &&
        parameters_match(&candidate, &builder->last_transaction)) {
        *out_transaction = builder->last_transaction;
        return EE_FOVEA_TRANSACTION_REUSED;
    }

    candidate.request_id = allocate_request_id(builder);
    builder->last_transaction = candidate;
    *out_transaction = candidate;
    return EE_FOVEA_TRANSACTION_NEW;
}
