#include "fake_data.h"
#include <string.h>

static EE_GlobalTargetDescriptor make_soccer_target(uint32_t frame_id, uint16_t target_id,
                                                    uint8_t type, uint16_t x, uint16_t y,
                                                    uint16_t w, uint16_t h, int16_t vx, int16_t vy,
                                                    uint8_t confidence, uint8_t field_zone,
                                                    uint16_t ball_distance, uint8_t valid)
{
    EE_GlobalTargetDescriptor t;
    memset(&t, 0, sizeof(t));
    t.target_id = target_id; t.frame_id = frame_id;
    t.x = x; t.y = y; t.w = w; t.h = h; t.vx = vx; t.vy = vy;
    t.coarse_conf = confidence; t.valid = valid; t.target_type = type;
    t.coarse_class = (type == EE_SOCCER_TARGET_BALL) ? EE_CLASS_UNKNOWN : EE_CLASS_PEDESTRIAN;
    t.field_zone = field_zone; t.zone_id = EE_ZONE_CENTER;
    t.ball_distance = ball_distance; t.is_ball = (type == EE_SOCCER_TARGET_BALL) ? 1u : 0u;
    t.motion_level = (uint8_t)((vx < 0 ? -vx : vx) + (vy < 0 ? -vy : vy));
    t.uncertainty = (uint8_t)(255u - confidence);
    return t;
}

void ee_fake_generate_soccer_scenario(uint8_t scenario, uint32_t frame_id, EE_GlobalTargetList *out_list)
{
    if (!out_list) return;
    memset(out_list, 0, sizeof(*out_list));
    out_list->frame_id = frame_id;
    switch (scenario) {
    case 2u:
        out_list->count = 2u;
        out_list->targets[0] = make_soccer_target(frame_id, 1u, EE_SOCCER_TARGET_BALL,
            (uint16_t)(220u + frame_id * 6u), 200u, 16u, 16u, 6, 2, 94u, EE_FIELD_ATTACK_ZONE, 0u, 1u);
        out_list->targets[1] = make_soccer_target(frame_id, 11u, EE_SOCCER_TARGET_PLAYER,
            340u, 180u, 46u, 92u, 1, 0, 91u, EE_FIELD_NORMAL, 55u, 1u);
        break;
    case 3u:
        out_list->count = 3u;
        out_list->targets[0] = make_soccer_target(frame_id, 1u, EE_SOCCER_TARGET_BALL,
            (uint16_t)(300u + frame_id), 210u, 18u, 18u, 6, 1, 95u, EE_FIELD_GOAL_ZONE, 0u, 1u);
        out_list->targets[1] = make_soccer_target(frame_id, 11u, EE_SOCCER_TARGET_PLAYER,
            320u, 180u, 44u, 90u, 2, 0, 90u, EE_FIELD_GOAL_ZONE, 25u, 1u);
        out_list->targets[2] = make_soccer_target(frame_id, 12u, EE_SOCCER_TARGET_PLAYER,
            120u, 180u, 50u, 94u, 1, 0, 92u, EE_FIELD_NORMAL, 170u, 1u);
        break;
    case 4u:
        out_list->count = 2u;
        out_list->targets[0] = make_soccer_target(frame_id, 1u, EE_SOCCER_TARGET_BALL,
            300u, 210u, 18u, 18u, 6, 1, 95u, EE_FIELD_NORMAL, 0u, 0u);
        out_list->targets[1] = make_soccer_target(frame_id, 11u, EE_SOCCER_TARGET_PLAYER,
            320u, 180u, 44u, 90u, 1, 0, 90u, EE_FIELD_ATTACK_ZONE, 25u, 1u);
        break;
    case 6u:
        out_list->count = 2u;
        out_list->targets[0] = make_soccer_target(frame_id, 21u, EE_SOCCER_TARGET_PLAYER,
            220u, 170u, 42u, 86u, 2, 0, 88u, EE_FIELD_ATTACK_ZONE, 60u, 1u);
        out_list->targets[1] = make_soccer_target(frame_id, 22u, EE_SOCCER_TARGET_PLAYER,
            360u, 175u, 42u, 86u, 2, 0, 87u, EE_FIELD_ATTACK_ZONE, 61u, 1u);
        break;
    case 7u:
        out_list->count = 0u;
        break;
    default:
        out_list->count = 3u;
        out_list->targets[0] = make_soccer_target(frame_id, 1u, EE_SOCCER_TARGET_BALL,
            (uint16_t)(250u + frame_id * 2u), 210u, 18u, 18u, 2, 1, 96u, EE_FIELD_NORMAL, 0u, 1u);
        out_list->targets[1] = make_soccer_target(frame_id, 11u, EE_SOCCER_TARGET_PLAYER,
            310u, 175u, 46u, 92u, 1, 0, 90u, EE_FIELD_ATTACK_ZONE, 38u, 1u);
        out_list->targets[2] = make_soccer_target(frame_id, 12u, EE_SOCCER_TARGET_PLAYER,
            120u, 180u, 50u, 94u, 1, 0, 91u, EE_FIELD_NORMAL, 160u, 1u);
        break;
    }
}

void ee_fake_generate_global_targets(uint32_t frame_id, EE_GlobalTargetList *out_list)
{
    ee_fake_generate_soccer_scenario(1u, frame_id, out_list);
}

uint8_t ee_fake_generate_refined_result(const EE_FoveaRequest *request, uint32_t current_frame_id,
                                        EE_RefinedTargetDescriptor *out_refined)
{
    if (!request || !request->valid || !out_refined || current_frame_id <= request->source_frame_id + 2u) return 0u;
    memset(out_refined, 0, sizeof(*out_refined));
    out_refined->request_id = request->request_id; out_refined->frame_id = request->source_frame_id;
    out_refined->target_id = request->target_id; out_refined->valid = 1u;
    out_refined->result_valid = 1u; out_refined->done = 1u;
    out_refined->target_type = (request->target_id == 1u) ? EE_SOCCER_TARGET_BALL : EE_SOCCER_TARGET_PLAYER;
    out_refined->tracking_quality = 92u; out_refined->focus_quality = 90u;
    out_refined->refined_class = (request->target_id == 1u) ? EE_CLASS_UNKNOWN : EE_CLASS_PEDESTRIAN;
    out_refined->refined_confidence = (request->target_id == 1u) ? 94u : 91u;
    out_refined->confirmed_risk_level = (request->target_id == 1u) ? EE_RISK_WARNING : EE_RISK_ATTENTION;
    out_refined->focus_state = (request->target_id == 1u) ? EE_FOCUS_HIGHLIGHT : EE_FOCUS_FOCUS;
    out_refined->confirm_state = EE_CONFIRM_CONFIRMED;
    return 1u;
}
