#include "eagleeye_types.h"

const char *ee_class_to_string(EE_TargetClass value)
{
    switch (value) {
    case EE_CLASS_CAR: return "CAR";
    case EE_CLASS_PEDESTRIAN: return "PEDESTRIAN";
    case EE_CLASS_BICYCLE: return "BICYCLE";
    case EE_CLASS_CONE: return "CONE";
    case EE_CLASS_BACKGROUND: return "BACKGROUND";
    default: return "UNKNOWN";
    }
}

const char *ee_soccer_target_to_string(EE_SoccerTargetType value)
{
    switch (value) {
    case EE_SOCCER_TARGET_BALL: return "BALL";
    case EE_SOCCER_TARGET_PLAYER: return "PLAYER";
    case EE_SOCCER_TARGET_REFEREE: return "REFEREE";
    default: return "UNKNOWN";
    }
}

const char *ee_field_zone_to_string(EE_FieldZone value)
{
    switch (value) {
    case EE_FIELD_ATTACK_ZONE: return "ATTACK_ZONE";
    case EE_FIELD_GOAL_ZONE: return "GOAL_ZONE";
    default: return "FIELD_NORMAL";
    }
}

const char *ee_zone_to_string(EE_Zone value)
{
    switch (value) {
    case EE_ZONE_LEFT: return "LEFT";
    case EE_ZONE_RIGHT: return "RIGHT";
    case EE_ZONE_CENTER: return "CENTER";
    case EE_ZONE_EGO_LANE: return "EGO_LANE";
    default: return "NONE";
    }
}

const char *ee_risk_to_string(EE_RiskLevel value)
{
    switch (value) {
    case EE_RISK_ATTENTION: return "ATTENTION";
    case EE_RISK_WARNING: return "WARNING";
    case EE_RISK_DANGER: return "DANGER";
    default: return "NORMAL";
    }
}

const char *ee_focus_to_string(EE_FocusState value)
{
    switch (value) {
    case EE_FOCUS_TRACK: return "TRACK";
    case EE_FOCUS_FOCUS: return "FOCUS";
    case EE_FOCUS_HIGHLIGHT: return "HIGHLIGHT";
    default: return "BACKGROUND";
    }
}

const char *ee_confirm_to_string(EE_ConfirmState value)
{
    switch (value) {
    case EE_CONFIRM_CONFIRMED: return "CONFIRMED";
    case EE_CONFIRM_UPGRADED: return "UPGRADED";
    case EE_CONFIRM_DOWNGRADED: return "DOWNGRADED";
    case EE_CONFIRM_CLEARED: return "CLEARED";
    case EE_CONFIRM_TIMEOUT: return "TIMEOUT";
    default: return "NONE";
    }
}
