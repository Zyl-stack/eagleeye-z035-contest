#ifndef FOVEA_TRANSACTION_H
#define FOVEA_TRANSACTION_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EE_FOVEA_TRANSACTION_INVALID = 0,
    EE_FOVEA_TRANSACTION_REUSED,
    EE_FOVEA_TRANSACTION_NEW
} EE_FoveaTransactionResult;

/*
 * Scheduler target locks are not AXI-Lite transactions.  This small state
 * object owns only the IDs of parameter sets actually committed to Fovea.
 */
typedef struct {
    uint32_t next_request_id;
    EE_FoveaRequest last_transaction;
} EE_FoveaTransactionBuilder;

void ee_fovea_transaction_init(EE_FoveaTransactionBuilder *builder);

/*
 * Builds the hardware request for current_source_frame_id.  A new ID is
 * allocated only when the submitted Fovea parameters differ from the last
 * committed transaction; otherwise the previous transaction is returned.
 */
EE_FoveaTransactionResult ee_fovea_transaction_build(
    EE_FoveaTransactionBuilder *builder,
    const EE_FoveaRequest *parameters,
    uint32_t current_source_frame_id,
    EE_FoveaRequest *out_transaction);

#ifdef __cplusplus
}
#endif

#endif
