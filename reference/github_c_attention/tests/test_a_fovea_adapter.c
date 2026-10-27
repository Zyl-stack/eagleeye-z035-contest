#include "a_fovea_adapter.h"
#include "register_map.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static EE_FoveaRequest make_request(uint16_t x, uint16_t y,
                                    uint16_t w, uint16_t h,
                                    uint8_t mode)
{
    EE_FoveaRequest request;
    memset(&request, 0, sizeof(request));
    request.request_id = 7u;
    request.source_frame_id = 42u;
    request.target_id = 202u;
    request.roi_x = x;
    request.roi_y = y;
    request.roi_w = w;
    request.roi_h = h;
    request.enhance_mode = mode;
    request.deadline_frames = 12u;
    request.valid = 1u;
    return request;
}

static void expect_center(uint16_t center_x, uint16_t center_y,
                          uint16_t expected_x, uint16_t expected_y)
{
    uint16_t x = UINT16_MAX;
    uint16_t y = UINT16_MAX;
    ee_a_fovea_fixed_roi_from_center(center_x, center_y, &x, &y);
    assert(x == expected_x);
    assert(y == expected_y);
}

int main(void)
{
    EE_FoveaRequest source;
    EE_FoveaRequest adapted;
    EE_AFoveaRegisterWrite writes[A_FOVEA_REGISTER_WRITE_COUNT];
    int write_count;

    expect_center(0u, 0u, 0u, 0u);
    expect_center(639u, 359u, 320u, 180u);
    expect_center(320u, 180u, 160u, 90u);
    printf("[PASS] A Fovea top-left, bottom-right and center clamp\n");

    source = make_request(300u, 170u, 40u, 20u, A_FOVEA_MODE_LANCZOS);
    assert(ee_a_fovea_adapt_request(&source, &adapted) == 0);
    assert(adapted.roi_x == 160u && adapted.roi_y == 90u);
    assert(adapted.roi_w == 320u && adapted.roi_h == 180u);
    assert(adapted.request_id == source.request_id);
    assert(adapted.target_id == source.target_id);
    assert(adapted.source_frame_id == source.source_frame_id);
    printf("[PASS] dynamic ROI -> fixed ROI=(%u,%u,%u,%u)\n",
           adapted.roi_x, adapted.roi_y, adapted.roi_w, adapted.roi_h);

    assert(ee_a_fovea_mode_from_use_lanczos(0u) == A_FOVEA_MODE_BILINEAR);
    assert(ee_a_fovea_mode_from_use_lanczos(1u) == A_FOVEA_MODE_LANCZOS);
    source.enhance_mode = A_FOVEA_MODE_BYPASS;
    assert(ee_a_fovea_adapt_request(&source, &adapted) == 0);
    assert(adapted.enhance_mode == A_FOVEA_MODE_BYPASS);
    printf("[PASS] mode mapping 0=BYPASS 1=BILINEAR 2=LANCZOS\n");

    source.enhance_mode = A_FOVEA_MODE_BILINEAR;
    write_count = ee_a_fovea_build_write_sequence(&source, 0u, writes);
    assert(write_count == (int)A_FOVEA_REGISTER_WRITE_COUNT);
    assert(EAGLEEYE_CTRL_ENABLE_MASK == (1u << 0));
    assert(EAGLEEYE_CTRL_COMMIT_MASK == (1u << 1));
    assert(writes[4].offset == EAGLEEYE_REG_ROI_WH);
    assert(writes[4].value == ee_pack_u16_pair(320u, 180u));
    assert(writes[8].offset == EAGLEEYE_REG_CONTROL);
    assert(writes[8].value ==
           (EAGLEEYE_CTRL_ENABLE_MASK | EAGLEEYE_CTRL_COMMIT_MASK));
    {
        uint8_t i;
        for (i = 0u; i + 1u < A_FOVEA_REGISTER_WRITE_COUNT; ++i) {
            assert(writes[i].offset != EAGLEEYE_REG_CONTROL);
        }
    }
    printf("[PASS] ROI_WH fixed and CONTROL ENABLE|COMMIT is final write\n");

    return 0;
}
