#include "fovea_transaction.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static EE_FoveaRequest make_parameters(uint16_t target_id,
                                       uint16_t roi_x,
                                       uint16_t roi_y,
                                       uint8_t enhance_mode)
{
    EE_FoveaRequest request;

    memset(&request, 0, sizeof(request));
    request.request_id = 99u; /* Scheduler-local ID: intentionally ignored. */
    request.target_id = target_id;
    request.roi_x = roi_x;
    request.roi_y = roi_y;
    request.roi_w = 320u;
    request.roi_h = 180u;
    request.enhance_mode = enhance_mode;
    request.deadline_frames = 12u;
    request.valid = 1u;
    return request;
}

static void test_parameter_change_lifecycle(void)
{
    EE_FoveaTransactionBuilder builder;
    EE_FoveaRequest parameters = make_parameters(7u, 100u, 50u, 1u);
    EE_FoveaRequest transaction;

    ee_fovea_transaction_init(&builder);
    assert(ee_fovea_transaction_build(&builder, &parameters, 10u, &transaction) ==
           EE_FOVEA_TRANSACTION_NEW);
    assert(transaction.request_id == 1u && transaction.source_frame_id == 10u);

    assert(ee_fovea_transaction_build(&builder, &parameters, 10u, &transaction) ==
           EE_FOVEA_TRANSACTION_REUSED);
    assert(transaction.request_id == 1u && transaction.source_frame_id == 10u);

    parameters.roi_x = 140u;
    assert(ee_fovea_transaction_build(&builder, &parameters, 11u, &transaction) ==
           EE_FOVEA_TRANSACTION_NEW);
    assert(transaction.request_id == 2u && transaction.source_frame_id == 11u);

    parameters.enhance_mode = 2u;
    assert(ee_fovea_transaction_build(&builder, &parameters, 12u, &transaction) ==
           EE_FOVEA_TRANSACTION_NEW);
    assert(transaction.request_id == 3u && transaction.source_frame_id == 12u);

    parameters.target_id = 8u;
    assert(ee_fovea_transaction_build(&builder, &parameters, 13u, &transaction) ==
           EE_FOVEA_TRANSACTION_NEW);
    assert(transaction.request_id == 4u && transaction.target_id == 8u);

    parameters.roi_x = 100u;
    assert(ee_fovea_transaction_build(&builder, &parameters, 14u, &transaction) ==
           EE_FOVEA_TRANSACTION_NEW);
    assert(transaction.request_id == 5u && transaction.source_frame_id == 14u);
    printf("[PASS] target/ROI/mode/frame changes create distinct Fovea transactions.\n");
}

static void test_request_id_wrap_skips_zero(void)
{
    EE_FoveaTransactionBuilder builder;
    EE_FoveaRequest parameters = make_parameters(7u, 100u, 50u, 1u);
    EE_FoveaRequest transaction;

    ee_fovea_transaction_init(&builder);
    builder.next_request_id = UINT32_MAX;
    assert(ee_fovea_transaction_build(&builder, &parameters, 1u, &transaction) ==
           EE_FOVEA_TRANSACTION_NEW);
    assert(transaction.request_id == UINT32_MAX);
    parameters.roi_x++;
    assert(ee_fovea_transaction_build(&builder, &parameters, 2u, &transaction) ==
           EE_FOVEA_TRANSACTION_NEW);
    assert(transaction.request_id == 1u);
    printf("[PASS] transaction request_id wraps UINT32_MAX -> 1 and skips zero.\n");
}

int main(void)
{
    test_parameter_change_lifecycle();
    test_request_id_wrap_skips_zero();
    printf("All Fovea transaction tests passed.\n");
    return 0;
}
