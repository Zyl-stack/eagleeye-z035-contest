#include "possession_estimator.h"
#include <string.h>

#define EE_FIELD_DIAGONAL_MANHATTAN 300u

static uint32_t abs_i32(int32_t value)
{
    return (uint32_t)(value < 0 ? -value : value);
}

static uint16_t nearest_axis_distance(uint16_t point, uint16_t origin, uint16_t size)
{
    if (point < origin) return (uint16_t)(origin - point);
    if (point > (uint32_t)origin + size) return (uint16_t)(point - origin - size);
    return 0u;
}

static uint8_t distance_score(const EE_GlobalTargetDescriptor *player,
                              const EE_GlobalTargetDescriptor *ball)
{
    uint16_t ball_cx = (uint16_t)(ball->x + ball->w / 2u);
    uint16_t ball_cy = (uint16_t)(ball->y + ball->h / 2u);
    uint16_t dx = nearest_axis_distance(ball_cx, player->x, player->w);
    uint16_t dy = nearest_axis_distance(ball_cy, player->y, player->h);
    uint32_t distance = (uint32_t)dx + dy;
    if (distance >= EE_FIELD_DIAGONAL_MANHATTAN) return 0u;
    return (uint8_t)(255u - (distance * 255u / EE_FIELD_DIAGONAL_MANHATTAN));
}

static uint8_t motion_score(const EE_GlobalTargetDescriptor *player,
                            const EE_GlobalTargetDescriptor *ball)
{
    int32_t dvx = (int32_t)player->vx - (int32_t)ball->vx;
    int32_t dvy = (int32_t)player->vy - (int32_t)ball->vy;
    uint32_t difference = abs_i32(dvx) + abs_i32(dvy);

    if (difference >= 24u) return 0u;
    return (uint8_t)(255u - difference * 255u / 24u);
}

uint8_t ee_possession_estimate(const EE_GlobalTargetDescriptor *player,
                               const EE_GlobalTargetDescriptor *ball,
                               uint16_t previous_target_id,
                               uint8_t previous_score,
                               EE_PossessionEstimate *out_estimate)
{
    uint16_t distance;
    uint16_t motion;
    uint16_t history;
    uint32_t score;

    if (!out_estimate) return 0u;
    memset(out_estimate, 0, sizeof(*out_estimate));
    if (!player || !ball || !player->valid || !ball->valid || player->is_ball) return 0u;

    distance = distance_score(player, ball);
    /* Speed continuity alone is not possession when the ball is far away. */
    motion = distance < 16u ? 0u : motion_score(player, ball);
    history = (player->target_id == previous_target_id && distance > 32u) ? previous_score : 0u;
    score = 60u * distance + 30u * motion + 10u * history;
    out_estimate->target_id = player->target_id;
    out_estimate->ball_distance_score = (uint8_t)distance;
    out_estimate->motion_continuity_score = (uint8_t)motion;
    out_estimate->history_score = (uint8_t)history;
    out_estimate->possession_score = (uint8_t)(score / 100u);
    out_estimate->possession_valid = 1u;
    return 1u;
}

void ee_possession_update_list(EE_GlobalTargetList *list,
                               uint16_t previous_target_id,
                               uint8_t previous_score,
                               uint16_t *out_best_target_id,
                               uint8_t *out_best_score)
{
    EE_GlobalTargetDescriptor *ball = NULL;
    uint16_t best_id = EE_INVALID_TARGET_ID;
    uint8_t best_score = 0u;
    uint8_t i;

    if (out_best_target_id) *out_best_target_id = EE_INVALID_TARGET_ID;
    if (out_best_score) *out_best_score = 0u;
    if (!list) return;
    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        if (list->targets[i].valid && list->targets[i].is_ball) {
            ball = &list->targets[i];
            break;
        }
    }
    if (!ball) {
        for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
            list->targets[i].possession_score = 0u;
        }
        return;
    }
    for (i = 0u; i < list->count && i < EE_MAX_TARGETS; ++i) {
        EE_PossessionEstimate estimate;
        if (!ee_possession_estimate(&list->targets[i], ball, previous_target_id,
                                    previous_score, &estimate)) {
            list->targets[i].possession_score = 0u;
            continue;
        }
        list->targets[i].possession_score = estimate.possession_score;
        if (estimate.possession_score > best_score) {
            best_score = estimate.possession_score;
            best_id = estimate.target_id;
        }
    }
    if (out_best_target_id) *out_best_target_id = best_id;
    if (out_best_score) *out_best_score = best_score;
}
