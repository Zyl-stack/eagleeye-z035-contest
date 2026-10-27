#ifndef EAGLEEYE_TYPES_H
#define EAGLEEYE_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EE_MAX_TARGETS 16u
#define EE_INVALID_TARGET_ID 0xFFFFu

typedef enum {
    EE_CLASS_UNKNOWN = 0,
    EE_CLASS_CAR,
    EE_CLASS_PEDESTRIAN,
    EE_CLASS_BICYCLE,
    EE_CLASS_CONE,
    EE_CLASS_BACKGROUND
} EE_TargetClass;

typedef enum {
    EE_SOCCER_TARGET_UNKNOWN = 0,
    EE_SOCCER_TARGET_BALL,
    EE_SOCCER_TARGET_PLAYER,
    EE_SOCCER_TARGET_REFEREE
} EE_SoccerTargetType;

typedef enum {
    EE_FIELD_NORMAL = 0,
    EE_FIELD_ATTACK_ZONE,
    EE_FIELD_GOAL_ZONE
} EE_FieldZone;

typedef enum {
    EE_ZONE_NONE = 0,
    EE_ZONE_LEFT,
    EE_ZONE_RIGHT,
    EE_ZONE_CENTER,
    EE_ZONE_EGO_LANE
} EE_Zone;

typedef enum {
    EE_RISK_NORMAL = 0,
    EE_RISK_ATTENTION,
    EE_RISK_WARNING,
    EE_RISK_DANGER
} EE_RiskLevel;

typedef enum {
    EE_FOCUS_BACKGROUND = 0,
    EE_FOCUS_TRACK,
    EE_FOCUS_FOCUS,
    EE_FOCUS_HIGHLIGHT
} EE_FocusState;

typedef enum {
    EE_CONFIRM_NONE = 0,
    EE_CONFIRM_CONFIRMED,
    EE_CONFIRM_UPGRADED,
    EE_CONFIRM_DOWNGRADED,
    EE_CONFIRM_CLEARED,
    EE_CONFIRM_TIMEOUT
} EE_ConfirmState;

typedef struct {
    uint16_t target_id;
    uint32_t frame_id;
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    uint8_t coarse_class;
    uint8_t coarse_conf; /* 0..100 */
    int16_t vx;
    int16_t vy;
    uint8_t zone_id;
    uint8_t approach_level; /* 0..100 */
    uint8_t valid;
    uint32_t attention_score;
    /* Soccer semantics. Legacy coarse_* / zone_id fields remain for compatibility. */
    uint8_t target_type;
    uint8_t motion_level;
    uint8_t small_target_score;
    uint8_t uncertainty;
    uint8_t field_zone;
    uint16_t ball_distance;
    uint8_t is_ball;
    /* Software-derived soccer feature; not an AXI/register wire field. */
    uint8_t possession_score;
} EE_GlobalTargetDescriptor;

typedef struct {
    uint32_t frame_id;
    uint8_t count;
    EE_GlobalTargetDescriptor targets[EE_MAX_TARGETS];
} EE_GlobalTargetList;

typedef struct {
    uint32_t request_id;
    uint16_t target_id;
    uint32_t source_frame_id;
    uint16_t roi_x;
    uint16_t roi_y;
    uint16_t roi_w;
    uint16_t roi_h;
    uint8_t enhance_mode;
    uint16_t deadline_frames;
    uint8_t valid;
} EE_FoveaRequest;

typedef struct {
    uint32_t request_id;
    uint16_t target_id;
    uint32_t source_frame_id;
    uint16_t roi_x;
    uint16_t roi_y;
    uint16_t roi_w;
    uint16_t roi_h;
    uint8_t enhance_mode;
    uint8_t result_valid;
    uint8_t done;
    uint8_t error;
} EE_EnhancedROIResult;

typedef struct {
    uint32_t request_id;
    uint32_t frame_id;
    uint16_t target_id;
    uint8_t valid;
    uint8_t result_valid;
    uint8_t done;
    uint8_t error;
    EE_TargetClass refined_class;
    uint8_t refined_confidence; /* 0..100 */
    EE_RiskLevel confirmed_risk_level;
    float confirmed_risk_score;
    EE_ConfirmState confirm_state;
    EE_FocusState focus_state;
    uint8_t target_type;
    uint8_t tracking_quality;
    uint8_t focus_quality;
} EE_RefinedTargetDescriptor;

const char *ee_class_to_string(EE_TargetClass value);
const char *ee_soccer_target_to_string(EE_SoccerTargetType value);
const char *ee_field_zone_to_string(EE_FieldZone value);
const char *ee_zone_to_string(EE_Zone value);
const char *ee_risk_to_string(EE_RiskLevel value);
const char *ee_focus_to_string(EE_FocusState value);
const char *ee_confirm_to_string(EE_ConfirmState value);

#ifdef __cplusplus
}
#endif

#endif
