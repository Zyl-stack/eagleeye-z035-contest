#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    EE_FoveaRequest current_request;
    EE_RefinedTargetDescriptor refined_result;
    EE_RiskLevel risk_state;
    EE_ConfirmState risk_action;
    uint8_t risk_action_pending;
    uint8_t timeout;
} EE_TransactionContext;

void ee_transaction_init(EE_TransactionContext *ctx);
void ee_transaction_start(EE_TransactionContext *ctx, const EE_FoveaRequest *request);
void ee_transaction_release(EE_TransactionContext *ctx);
uint8_t ee_refined_result_matches_request(const EE_FoveaRequest *request,
                                          const EE_RefinedTargetDescriptor *received);
uint8_t ee_transaction_accept_refined(EE_TransactionContext *ctx,
                                      const EE_RefinedTargetDescriptor *received);
uint8_t ee_transaction_consume_action(EE_TransactionContext *ctx,
                                      EE_ConfirmState *out_action);
void ee_transaction_mark_timeout(EE_TransactionContext *ctx);

#ifdef __cplusplus
}
#endif

#endif
