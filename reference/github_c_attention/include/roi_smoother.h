#ifndef ROI_SMOOTHER_H
#define ROI_SMOOTHER_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EE_ROI_FIXED_WIDTH  ((uint16_t)320u)
#define EE_ROI_FIXED_HEIGHT ((uint16_t)180u)
#define EE_ROI_X_MAX        ((uint16_t)320u)
#define EE_ROI_Y_MAX        ((uint16_t)180u)

typedef struct {
    uint16_t max_step_x;
    uint16_t max_step_y;
} EE_ROISmootherConfig;

typedef struct {
    EE_ROISmootherConfig cfg;
    uint16_t roi_x;
    uint16_t roi_y;
    uint8_t initialized;
} EE_ROISmoother;

EE_ROISmootherConfig ee_roi_smoother_default_config(void);
void ee_roi_smoother_init(EE_ROISmoother *smoother, EE_ROISmootherConfig cfg);
void ee_roi_smoother_set_config(EE_ROISmoother *smoother, EE_ROISmootherConfig cfg);
uint8_t ee_roi_smoother_step(EE_ROISmoother *smoother,
                             const EE_FoveaRequest *desired,
                             EE_FoveaRequest *out_smoothed);

#ifdef __cplusplus
}
#endif

#endif
