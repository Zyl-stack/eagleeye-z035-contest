#ifndef MOTION_PREDICTOR_H
#define MOTION_PREDICTOR_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t target_id;
    uint32_t source_frame_id;
    uint16_t predicted_x;
    uint16_t predicted_y;
    uint8_t prediction_valid;
} EE_MotionPrediction;

void ee_motion_predictor_init(void);
void ee_motion_predictor_update(const EE_GlobalTargetList *list);
uint8_t ee_motion_predict(const EE_GlobalTargetDescriptor *target,
                          EE_MotionPrediction *out_prediction);

#ifdef __cplusplus
}
#endif

#endif
