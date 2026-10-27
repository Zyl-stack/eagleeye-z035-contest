#include "attention.h"
#include "attention_scheduler.h"
#include "eagleeye_types.h"
#include "fake_data.h"
#include "motion_predictor.h"
#include "possession_estimator.h"
#include "possession_estimator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t triangle_offset(uint32_t frame, uint16_t phase, uint16_t amplitude)
{
    uint32_t period = (uint32_t)amplitude * 2u;
    uint32_t position;
    if (amplitude == 0u) return 0u;
    position = (frame * 2u + phase) % period;
    return (uint16_t)(position <= amplitude ? position : period - position);
}

static uint16_t lerp_u16(uint16_t a, uint16_t b, uint32_t frame,
                         uint32_t start, uint32_t end)
{
    int32_t difference;
    int32_t value;
    if (frame <= start) return a;
    if (frame >= end) return b;
    difference = (int32_t)b - (int32_t)a;
    value = (int32_t)a + difference * (int32_t)(frame - start) /
            (int32_t)(end - start);
    return (uint16_t)value;
}

static EE_GlobalTargetDescriptor make_target(uint16_t id, uint8_t type,
                                             uint32_t frame, uint16_t x,
                                             uint16_t y, uint16_t w, uint16_t h,
                                             uint8_t confidence, int16_t vx,
                                             int16_t vy, uint8_t zone,
                                             uint8_t field_zone, uint16_t distance,
                                             uint8_t is_ball)
{
    EE_GlobalTargetDescriptor target;
    memset(&target, 0, sizeof(target));
    target.target_id = id;
    target.frame_id = frame;
    target.x = x;
    target.y = y;
    target.w = w;
    target.h = h;
    target.coarse_class = EE_CLASS_UNKNOWN;
    target.coarse_conf = confidence;
    target.vx = vx;
    target.vy = vy;
    target.zone_id = zone;
    target.approach_level = (uint8_t)(vx > 0 ? (vx > 8 ? 90u : 50u) : 10u);
    target.valid = 1u;
    target.target_type = type;
    target.field_zone = field_zone;
    target.ball_distance = distance;
    target.is_ball = is_ball;
    return target;
}

static void make_mini_match(uint32_t frame, EE_GlobalTargetList *list,
                            EE_GlobalTargetDescriptor *last_ball,
                            uint8_t *ball_occluded)
{
    static const uint16_t ids[8] = {7u, 10u, 17u, 21u, 4u, 6u, 8u, 22u};
    static const uint16_t base_x[8] = {300u, 430u, 100u, 100u, 200u, 100u, 500u, 100u};
    static const uint16_t base_y[8] = {140u, 190u, 250u, 300u, 180u, 80u, 260u, 320u};
    uint16_t ball_x;
    uint16_t ball_y;
    int16_t ball_vx;
    int16_t ball_vy;
    uint8_t i;

    memset(list, 0, sizeof(*list));
    list->frame_id = frame;
    list->count = 9u;

    if (frame <= 50u) {
        ball_x = 305u;
        ball_y = 145u;
        ball_vx = 2;
        ball_vy = 1;
    } else if (frame <= 90u) {
        ball_x = lerp_u16(305u, 365u, frame, 51u, 90u);
        ball_y = lerp_u16(145u, 170u, frame, 51u, 90u);
        ball_vx = 9;
        ball_vy = 3;
    } else if (frame <= 130u) {
        ball_x = lerp_u16(365u, 435u, frame, 91u, 130u);
        ball_y = lerp_u16(170u, 195u, frame, 91u, 130u);
        ball_vx = 7;
        ball_vy = 3;
    } else if (frame <= 180u) {
        ball_x = lerp_u16(435u, 520u, frame, 131u, 180u);
        ball_y = lerp_u16(195u, 185u, frame, 131u, 180u);
        ball_vx = 10;
        ball_vy = -1;
    } else if (frame <= 210u) {
        ball_x = 540u;
        ball_y = 180u;
        ball_vx = 8;
        ball_vy = 0;
    } else if (frame <= 225u) {
        ball_x = 540u;
        ball_y = 180u;
        ball_vx = 5;
        ball_vy = 0;
    } else if (frame <= 245u) {
        ball_x = lerp_u16(540u, 500u, frame, 226u, 245u);
        ball_y = lerp_u16(180u, 190u, frame, 226u, 245u);
        ball_vx = -3;
        ball_vy = 1;
    } else {
        ball_x = lerp_u16(500u, 600u, frame, 246u, 280u);
        ball_y = lerp_u16(190u, 145u, frame, 246u, 280u);
        ball_vx = 8;
        ball_vy = -3;
    }

    *ball_occluded = (uint8_t)(frame >= 211u && frame <= 225u);
    for (i = 0u; i < 8u; ++i) {
        uint16_t x = (uint16_t)(base_x[i] + triangle_offset(frame, (uint16_t)(i * 7u), 10u));
        uint16_t y = (uint16_t)(base_y[i] + triangle_offset(frame, (uint16_t)(i * 11u), 7u));
        uint8_t field_zone = (i == 1u && frame >= 181u) ? EE_FIELD_GOAL_ZONE :
                             (i == 1u && frame >= 131u ? EE_FIELD_ATTACK_ZONE : EE_FIELD_NORMAL);
        uint16_t distance = (uint16_t)(abs((int)x - (int)ball_x) + abs((int)y - (int)ball_y));
        uint8_t confidence = (i == 2u || i == 5u) ? 68u : 90u;
        list->targets[i] = make_target(ids[i], EE_SOCCER_TARGET_PLAYER, frame,
                                        x, y, 24u, 54u, confidence,
                                        (int16_t)((i & 1u) ? 2 : -2), 1,
                                        (uint8_t)(i < 4u ? EE_ZONE_LEFT : EE_ZONE_RIGHT),
                                        field_zone, distance, 0u);
    }

    list->targets[8] = make_target(1u, EE_SOCCER_TARGET_BALL, frame,
                                   ball_x, ball_y, 14u, 14u, 96u,
                                   ball_vx, ball_vy, EE_ZONE_CENTER,
                                   ball_x >= 520u ? EE_FIELD_GOAL_ZONE :
                                   (ball_x >= 400u ? EE_FIELD_ATTACK_ZONE : EE_FIELD_NORMAL),
                                   0u, 1u);
    if (*ball_occluded) {
        *last_ball = list->targets[8];
        list->targets[8].valid = 0u;
    } else {
        *last_ball = list->targets[8];
    }
}

static void print_target_json(const EE_GlobalTargetDescriptor *target,
                              const EE_AttentionDebug *debug, uint8_t selected)
{
    const char *team = target->is_ball ? "BALL" :
                       ((target->target_id == 7u || target->target_id == 10u ||
                         target->target_id == 17u || target->target_id == 21u) ? "A" : "B");
    uint8_t visible = (target->target_id == 10u && target->frame_id >= 246u &&
                       target->frame_id <= 265u) ? 0u : 1u;
    printf("{\"target_id\":%u,\"team\":\"%s\",\"target_type\":\"%s\",\"valid\":%u,\"visible\":%u,\"x\":%u,\"y\":%u,\"w\":%u,\"h\":%u,"
           "\"confidence\":%u,\"vx\":%d,\"vy\":%d,\"field_zone\":\"%s\",\"ball_distance\":%u,"
           "\"ball_priority\":%u,\"motion\":%u,\"goal_zone\":%u,\"small_target\":%u,\"uncertainty\":%u,"
           "\"ball_proximity\":%u,\"possession_score\":%u,\"aging\":%u,\"attention\":%u,\"selected\":%u}",
           target->target_id, team, ee_soccer_target_to_string((EE_SoccerTargetType)target->target_type), target->valid, visible,
           target->x, target->y, target->w, target->h, target->coarse_conf, target->vx, target->vy,
           ee_field_zone_to_string((EE_FieldZone)target->field_zone), target->ball_distance,
           debug->ball_priority_score, debug->motion_score, debug->goal_zone_score, debug->small_target_score,
           debug->uncertainty_score, debug->ball_proximity_score, target->possession_score, debug->aging_score,
           (unsigned)debug->total_attention, selected);
}

int main(int argc, char **argv)
{
    uint8_t scenario = 1u;
    uint32_t frame;
    uint32_t total_frames;
    EE_AttentionScheduler scheduler;
    EE_AttentionConfig attention_config;
    EE_GlobalTargetDescriptor last_ball;
    uint8_t ball_occluded;
    uint16_t previous_carrier = EE_INVALID_TARGET_ID;
    uint8_t previous_possession = 0u;

    if (argc > 1) {
        scenario = (uint8_t)atoi(argv[1]);
        if (scenario < 1u || scenario > 6u) scenario = 1u;
    }
    total_frames = scenario == 6u ? 280u : 40u;
    memset(&last_ball, 0, sizeof(last_ball));
    ee_scheduler_init(&scheduler, ee_scheduler_default_config());
    attention_config = ee_attention_default_config();
    ee_motion_predictor_init();

    for (frame = 1u; frame <= total_frames; ++frame) {
        EE_GlobalTargetList list;
        EE_FoveaRequest request;
        EE_MotionPrediction prediction;
        uint16_t predicted_x = 0u;
        uint16_t predicted_y = 0u;
        uint8_t prediction_valid = 0u;
        uint8_t prediction_source_core = 0u;
        uint16_t selected_id;
        uint16_t challenger_id = EE_INVALID_TARGET_ID;
        uint32_t challenger_score = 0u;
        uint32_t selected_score = 0u;
        uint16_t estimated_carrier_id = EE_INVALID_TARGET_ID;
        uint8_t estimated_carrier_score = 0u;
        uint16_t ball_x = 0u;
        uint16_t ball_y = 0u;
        uint8_t ball_valid = 0u;
        uint16_t hold_remaining = 0u;
        uint8_t i;

        if (scenario == 6u) {
            make_mini_match(frame, &list, &last_ball, &ball_occluded);
        } else {
            ball_occluded = (uint8_t)(scenario == 4u && (frame == 6u || frame == 7u));
            ee_fake_generate_soccer_scenario((scenario == 5u) ? 6u : scenario, frame, &list);
        }

        if (scenario == 6u) {
            ee_possession_update_list(&list, previous_carrier, previous_possession,
                                      &estimated_carrier_id, &estimated_carrier_score);
        }
        if (estimated_carrier_id != EE_INVALID_TARGET_ID) {
            previous_carrier = estimated_carrier_id;
            previous_possession = estimated_carrier_score;
        }
        /* Mark carrier-oriented soccer context without changing the wire
           interface. The ball remains in the candidate list, but does not
           win solely because it is a small BALL target. */
        for (i = 0u; i < list.count && i < EE_MAX_TARGETS; ++i) {
            if (list.targets[i].is_ball && estimated_carrier_id != EE_INVALID_TARGET_ID) {
                list.targets[i].possession_score = 1u;
            }
        }
        for (i = 0u; i < list.count && i < EE_MAX_TARGETS; ++i) {
            if (list.targets[i].is_ball) {
                ball_x = (uint16_t)(list.targets[i].x + list.targets[i].w / 2u);
                ball_y = (uint16_t)(list.targets[i].y + list.targets[i].h / 2u);
                ball_valid = list.targets[i].valid;
            }
        }
        ee_scheduler_step(&scheduler, &list, &request);
        selected_id = request.valid ? request.target_id : EE_INVALID_TARGET_ID;
        if (scheduler.active_request.valid && scheduler.hold_counter < scheduler.cfg.min_hold_frames) {
            hold_remaining = (uint16_t)(scheduler.cfg.min_hold_frames - scheduler.hold_counter);
        }
        for (i = 0u; i < list.count && i < EE_MAX_TARGETS; ++i) {
            EE_AttentionDebug debug;
            ee_attention_compute_debug_waiting(&list.targets[i], &attention_config,
                                               scheduler.wait_frames[i], &debug);
            if (list.targets[i].valid && list.targets[i].target_id == selected_id) {
                selected_score = debug.total_attention;
            } else if (list.targets[i].valid && debug.total_attention > challenger_score) {
                challenger_score = debug.total_attention;
                challenger_id = list.targets[i].target_id;
            }
        }
        if (request.valid && scenario == 6u) {
            if (ball_occluded && last_ball.valid) {
                EE_GlobalTargetDescriptor probe = last_ball;
                probe.frame_id = frame;
                probe.x = (uint16_t)((int32_t)last_ball.x + last_ball.vx);
                probe.y = (uint16_t)((int32_t)last_ball.y + last_ball.vy);
                if (ee_motion_predict(&probe, &prediction)) {
                    predicted_x = prediction.predicted_x;
                    predicted_y = prediction.predicted_y;
                    prediction_valid = prediction.prediction_valid;
                    prediction_source_core = 1u;
                }
            } else {
                for (i = 0u; i < list.count && i < EE_MAX_TARGETS; ++i) {
                    if (list.targets[i].valid && list.targets[i].target_id == selected_id &&
                        ee_motion_predict(&list.targets[i], &prediction)) {
                        predicted_x = prediction.predicted_x;
                        predicted_y = prediction.predicted_y;
                        prediction_valid = prediction.prediction_valid;
                        prediction_source_core = 1u;
                        break;
                    }
                }
            }
            if (prediction_valid) {
                request.roi_x = predicted_x;
                request.roi_y = predicted_y;
            }
        } else if (request.valid) {
            for (i = 0u; i < list.count && i < EE_MAX_TARGETS; ++i) {
                if (list.targets[i].valid && list.targets[i].target_id == selected_id &&
                    ee_motion_predict(&list.targets[i], &prediction)) {
                    predicted_x = prediction.predicted_x;
                    predicted_y = prediction.predicted_y;
                    prediction_valid = prediction.prediction_valid;
                    prediction_source_core = 1u;
                    break;
                }
            }
        }

        if (request.valid && scenario == 6u && selected_id != 1u) {
            EE_GlobalTargetDescriptor *focus = NULL;
            for (i = 0u; i < list.count && i < EE_MAX_TARGETS; ++i) {
                if (list.targets[i].valid && list.targets[i].target_id == selected_id) {
                    focus = &list.targets[i];
                    break;
                }
            }
            if (focus) {
                uint16_t left = focus->x < ball_x ? focus->x : ball_x;
                uint16_t top = focus->y < ball_y ? focus->y : ball_y;
                uint16_t right = (uint16_t)(((uint32_t)focus->x + focus->w) > (uint32_t)ball_x + 14u ?
                                             focus->x + focus->w : ball_x + 14u);
                uint16_t bottom = (uint16_t)(((uint32_t)focus->y + focus->h) > (uint32_t)ball_y + 14u ?
                                              focus->y + focus->h : ball_y + 14u);
                left = left > 20u ? (uint16_t)(left - 20u) : 0u;
                top = top > 20u ? (uint16_t)(top - 20u) : 0u;
                right = (uint16_t)(right + 20u > 640u ? 640u : right + 20u);
                bottom = (uint16_t)(bottom + 20u > 360u ? 360u : bottom + 20u);
                request.roi_x = left;
                request.roi_y = top;
                request.roi_w = (uint16_t)(right - left);
                request.roi_h = (uint16_t)(bottom - top);
            }
        }

        printf("{\"frame_id\":%u,\"scenario\":%u,\"selected\":%u,\"fovea_view_target_id\":%u,"
               "\"request_id\":%u,\"roi_x\":%u,\"roi_y\":%u,\"roi_w\":%u,\"roi_h\":%u,"
               "\"prediction_valid\":%u,\"predicted_x\":%u,\"predicted_y\":%u,\"prediction_source\":\"%s\","
               "\"focus_state\":\"%s\",\"algorithm_state\":\"scheduler_output\",\"scenario_event\":\"%s\","
               "\"event\":\"%s\",\"ball_occluded\":%u,"
               "\"ball_x\":%u,\"ball_y\":%u,\"ball_valid\":%u,"
               "\"estimated_ball_carrier_id\":%u,\"estimated_ball_carrier_score\":%u,"
               "\"switch_margin\":%.3f,\"hold_remaining\":%u,"
               "\"challenger_id\":%u,\"score_difference\":%u,\"targets\":[",
               frame, scenario, selected_id, selected_id, request.valid ? request.request_id : 0u,
               request.roi_x, request.roi_y, request.roi_w, request.roi_h, prediction_valid,
               predicted_x, predicted_y, prediction_source_core ? "core" : "none",
               (selected_score >= 900u ? "HIGHLIGHT" : "FOCUS"),
               scenario == 6u ? (ball_occluded ? "BALL TEMP LOST" :
               (frame <= 40u ? "KICK-OFF / BALL TRACKING" : frame <= 80u ? "PASS IN PROGRESS" :
               frame <= 130u ? "PASS / POSSESSION CHANGE" : frame <= 180u ? "CARRIER ATTACK" :
               frame <= 225u ? "BALL TEMP LOST" : frame <= 245u ? "BALL REACQUIRED" :
               frame <= 265u ? "PLAYER TEMP LOST" : "FINAL ATTACK")) : "STATIC TEST SCENE",
               scenario == 6u ? (ball_occluded ? "BALL TEMP LOST" :
               (frame <= 50u ? "NORMAL PLAY" : frame <= 90u ? "FAST BALL-CARRIER MOTION" :
               frame <= 130u ? "PASS #7 -> #10" : frame <= 180u ? "POSSESSION #10" :
               frame <= 210u ? "APPROACHING GOAL" : frame <= 225u ? "BALL OCCLUDED" :
               frame <= 245u ? "BALL REACQUIRED" : frame <= 265u ? "PLAYER #10 OCCLUDED / PREDICT HOLD" :
               "REACQUIRED")) : "STATIC TEST SCENE",
               ball_occluded, ball_x, ball_y, ball_valid, estimated_carrier_id,
               estimated_carrier_score, scheduler.cfg.switch_margin, hold_remaining, challenger_id,
               selected_score >= challenger_score ?
               selected_score - challenger_score : challenger_score - selected_score);
        for (i = 0u; i < list.count && i < EE_MAX_TARGETS; ++i) {
            EE_AttentionDebug debug;
            ee_attention_compute_debug_waiting(&list.targets[i], &attention_config,
                                               scheduler.wait_frames[i], &debug);
            if (i != 0u) printf(",");
            print_target_json(&list.targets[i], &debug,
                              list.targets[i].valid && list.targets[i].target_id == selected_id);
        }
        printf("]}\n");
        ee_motion_predictor_update(&list);
    }
    return 0;
}
