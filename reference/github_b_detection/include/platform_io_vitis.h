#ifndef PLATFORM_IO_VITIS_H
#define PLATFORM_IO_VITIS_H

#include "eagleeye_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(EAGLEEYE_VITIS)
#include "xparameters.h"
#if defined(EAGLEEYE_FOVEA_BASEADDR)
#define EAGLEEYE_FOVEA_BASEADDR_PLACEHOLDER 0
#else
/*
 * TODO: After Vivado generates the real xparameters.h macro, define
 * EAGLEEYE_FOVEA_BASEADDR explicitly, for example:
 * #define EAGLEEYE_FOVEA_BASEADDR XPAR_FOVEA_AXIL_CTRL_0_S00_AXI_BASEADDR
 *
 * A suggested, non-final Vivado instance name is fovea_axil_ctrl_0.
 * A suggested, non-final address is 0x43C00000.
 * Do not hard-code that address here; wait for Address Editor/xparameters.h.
 */
#define EAGLEEYE_FOVEA_BASEADDR ((uintptr_t)0u)
#define EAGLEEYE_FOVEA_BASEADDR_PLACEHOLDER 1
#endif
#else
#define EAGLEEYE_FOVEA_BASEADDR ((uintptr_t)0u)
#define EAGLEEYE_FOVEA_BASEADDR_PLACEHOLDER 1
#endif

/*
 * Frozen FoveaRequest currently has no output_buffer_id field.
 * TODO: A/C must decide whether this becomes a request field or a fixed system config.
 */
#ifndef EAGLEEYE_FOVEA_DEFAULT_OUTPUT_BUFFER_ID
#define EAGLEEYE_FOVEA_DEFAULT_OUTPUT_BUFFER_ID ((uint32_t)0u)
#endif

typedef struct {
    uint32_t raw_status;
    uint32_t cycle_count;
    uint32_t quality_flags;
    uint8_t busy;
    uint8_t done;
    uint8_t error;
    uint8_t active_mode;
} EE_FoveaAxiStatus;

typedef struct {
    uint8_t roi_clipped;
    uint8_t mode_fallback;
    uint8_t target_switch;
    uint8_t request_cancelled;
    uint8_t frame_safe_latched;
    uint8_t output_buffer_valid;
} EE_FoveaQualityDecode;

void ee_platform_io_vitis_init(void);
uintptr_t ee_fovea_get_baseaddr(void);
int fovea_submit_request(const EE_FoveaRequest *req);
int fovea_read_status(EE_FoveaAxiStatus *out_status);
void fovea_decode_status(uint32_t raw_status, EE_FoveaAxiStatus *out_status);
void fovea_decode_quality_flags(uint32_t raw_quality_flags,
                                EE_FoveaQualityDecode *out_quality);

#ifdef __cplusplus
}
#endif

#endif
