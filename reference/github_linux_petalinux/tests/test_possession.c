#include "possession_estimator.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static EE_GlobalTargetDescriptor make_target(uint16_t id, uint16_t x, uint16_t y,
                                             int16_t vx, int16_t vy, uint8_t ball)
{
    EE_GlobalTargetDescriptor t;
    memset(&t, 0, sizeof(t));
    t.target_id = id; t.x = x; t.y = y; t.w = ball ? 14u : 24u; t.h = ball ? 14u : 54u;
    t.vx = vx; t.vy = vy; t.valid = 1u; t.is_ball = ball;
    t.target_type = ball ? EE_SOCCER_TARGET_BALL : EE_SOCCER_TARGET_PLAYER;
    return t;
}

int main(void)
{
    EE_GlobalTargetList list;
    uint16_t carrier;
    uint8_t score;
    uint8_t previous_score;
    memset(&list, 0, sizeof(list));
    list.count = 3u;
    list.targets[0] = make_target(7u, 100u, 100u, 4, 0, 0);
    list.targets[1] = make_target(10u, 400u, 100u, 0, 0, 0);
    list.targets[2] = make_target(1u, 105u, 105u, 4, 0, 1);
    ee_possession_update_list(&list, EE_INVALID_TARGET_ID, 0u, &carrier, &score);
    assert(carrier == 7u && list.targets[0].possession_score > list.targets[1].possession_score);
    previous_score = score;
    printf("[PASS] carrier near BALL: #%u possession=%u\n", carrier, score);

    list.targets[2].x = 405u; list.targets[2].y = 105u;
    list.targets[0].x = 0u; list.targets[0].y = 0u;
    list.targets[1].vx = 4; list.targets[1].vy = 0;
    ee_possession_update_list(&list, carrier, previous_score, &carrier, &score);
    assert(carrier == 10u);
    printf("[PASS] pass handover candidate: #%u possession=%u\n", carrier, score);

    list.targets[2].x = 105u; list.targets[2].y = 105u;
    list.targets[2].vx = 4;
    ee_possession_update_list(&list, 7u, previous_score, &carrier, &score);
    assert(carrier == 7u);
    printf("[PASS] history stabilizes nearby carrier: #%u possession=%u\n", carrier, score);

    list.targets[2].x = 600u; list.targets[2].y = 300u;
    ee_possession_update_list(&list, 7u, previous_score, &carrier, &score);
    assert(carrier != 7u || score == 0u);
    printf("[PASS] history does not permanently lock old carrier\n");
    return 0;
}
