#ifndef ATTENTION_CONFIG_H
#define ATTENTION_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Soccer: base priority for the ball itself. */
#define EE_ATTENTION_WEIGHT_BALL        ((uint16_t)3u)
#define EE_ATTENTION_WEIGHT_GOAL        ((uint16_t)3u)
#define EE_ATTENTION_WEIGHT_BALL_NEAR   ((uint16_t)2u)
#define EE_ATTENTION_WEIGHT_AGING       ((uint16_t)1u)

/* Legacy alias retained for code that still names the old zone term. */
#define EE_ATTENTION_WEIGHT_ZONE        ((uint16_t)3u)

/* Gives priority to moving targets. Kept low in V1 to avoid motion noise dominance. */
#define EE_ATTENTION_WEIGHT_MOTION      ((uint16_t)1u)

/* Gives priority to targets with approaching tendency. */
#define EE_ATTENTION_WEIGHT_APPROACH    ((uint16_t)3u)

/* Gives priority to small targets that need fovea confirmation. */
#define EE_ATTENTION_WEIGHT_SMALL       ((uint16_t)2u)

/* Gives priority to uncertain coarse detections. Uncertainty = 255 - coarse_conf. */
#define EE_ATTENTION_WEIGHT_UNCERTAINTY ((uint16_t)3u)

#ifdef __cplusplus
}
#endif

#endif
