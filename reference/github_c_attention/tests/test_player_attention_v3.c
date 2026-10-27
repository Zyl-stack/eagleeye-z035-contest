#include "attention_scheduler.h"
#include "player_attention_features.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static EE_GlobalTargetDescriptor make_target(uint16_t target_id, uint8_t type,
                                             uint32_t frame_id, uint16_t x, uint16_t y,
                                             uint16_t w, uint16_t h, uint8_t confidence,
                                             int16_t vx, int16_t vy, uint8_t valid)
{
    EE_GlobalTargetDescriptor target;

    memset(&target, 0, sizeof(target));
    target.target_id = target_id;
    target.target_type = type;
    target.is_ball = (uint8_t)(type == EE_SOCCER_TARGET_BALL);
    target.frame_id = frame_id;
    target.x = x;
    target.y = y;
    target.w = w;
    target.h = h;
    target.coarse_conf = confidence;
    target.vx = vx;
    target.vy = vy;
    target.valid = valid;
    return target;
}

static void reset_list(EE_GlobalTargetList *list, uint32_t frame_id)
{
    memset(list, 0, sizeof(*list));
    list->frame_id = frame_id;
}

static const EE_PlayerAttentionFeatures *features_for(const EE_PlayerAttentionFrame *frame,
                                                       uint16_t target_id)
{
    const EE_PlayerAttentionFeatures *features =
        ee_player_attention_find_features(frame, target_id);

    assert(features != NULL);
    return features;
}

static void test_motion_uncertainty_and_saliency(void)
{
    EE_PlayerAttentionConfig config = ee_player_attention_default_config();
    EE_PlayerAttentionHistory history;
    EE_PlayerAttentionFrame frame;
    EE_GlobalTargetList list;
    const EE_PlayerAttentionFeatures *features;

    ee_player_attention_history_init(&history);
    reset_list(&list, 1u);
    list.count = 1u;
    list.targets[0] = make_target(10u, EE_SOCCER_TARGET_PLAYER, 1u,
                                  100u, 100u, 20u, 50u, 100u, 0, 0, 1u);
    assert(ee_player_attention_update_scores_for_scheduler_step(
        &list, &config, &history, NULL, &frame) == 1u);
    features = features_for(&frame, 10u);
    assert(features->motion_score == 0u);
    assert(features->motion_saliency_score == 0u);
    assert(features->uncertainty_score == 0u);

    reset_list(&list, 2u);
    list.count = 1u;
    list.targets[0] = make_target(10u, EE_SOCCER_TARGET_PLAYER, 2u,
                                  100u, 100u, 20u, 50u, 50u, 4, 0, 1u);
    assert(ee_player_attention_update_scores_for_scheduler_step(
        &list, &config, &history, NULL, &frame) == 1u);
    features = features_for(&frame, 10u);
    assert(features->motion_score > 0u);
    assert(features->motion_saliency_score > 0u);
    assert(features->uncertainty_score == 127u);

    reset_list(&list, 3u);
    list.count = 1u;
    list.targets[0] = make_target(10u, EE_SOCCER_TARGET_PLAYER, 3u,
                                  100u, 100u, 20u, 50u, 0u,
                                  INT16_MIN, INT16_MAX, 1u);
    assert(ee_player_attention_update_scores_for_scheduler_step(
        &list, &config, &history, NULL, &frame) == 1u);
    features = features_for(&frame, 10u);
    assert(features->motion_score == 255u);
    assert(features->motion_saliency_score == 255u);
    assert(features->uncertainty_score == 255u);
    printf("[PASS] V3 motion and saliency handle signed velocity limits; uncertainty maps 100/50/0 to 0/127/255.\n");
}

static void test_interaction_history_and_reorder(void)
{
    EE_PlayerAttentionConfig config = ee_player_attention_default_config();
    EE_PlayerAttentionHistory history;
    EE_PlayerAttentionFrame frame;
    EE_GlobalTargetList list;
    const EE_PlayerAttentionFeatures *first;
    const EE_PlayerAttentionFeatures *second;

    ee_player_attention_history_init(&history);
    reset_list(&list, 10u);
    list.count = 2u;
    list.targets[0] = make_target(10u, EE_SOCCER_TARGET_PLAYER, 10u,
                                  10u, 10u, 20u, 50u, 90u, 0, 0, 1u);
    list.targets[1] = make_target(20u, EE_SOCCER_TARGET_PLAYER, 10u,
                                  300u, 10u, 20u, 50u, 90u, 0, 0, 1u);
    assert(ee_player_attention_update_scores_for_scheduler_step(
        &list, &config, &history, NULL, &frame) == 1u);
    assert(features_for(&frame, 10u)->interaction_score == 0u);

    reset_list(&list, 11u);
    list.count = 2u;
    /* Reversed list order proves pair history is keyed by target ID. */
    list.targets[0] = make_target(20u, EE_SOCCER_TARGET_PLAYER, 11u,
                                  65u, 10u, 20u, 50u, 90u, 0, 0, 1u);
    list.targets[1] = make_target(10u, EE_SOCCER_TARGET_PLAYER, 11u,
                                  10u, 10u, 20u, 50u, 90u, 0, 0, 1u);
    assert(ee_player_attention_update_scores_for_scheduler_step(
        &list, &config, &history, NULL, &frame) == 1u);
    first = features_for(&frame, 10u);
    second = features_for(&frame, 20u);
    assert(first->interaction_score > config.interaction_neighbor_points);
    assert(second->interaction_score > config.interaction_neighbor_points);

    reset_list(&list, 100u);
    list.count = 2u;
    list.targets[0] = make_target(10u, EE_SOCCER_TARGET_PLAYER, 100u,
                                  10u, 10u, 20u, 50u, 90u, 0, 0, 1u);
    list.targets[1] = make_target(20u, EE_SOCCER_TARGET_PLAYER, 100u,
                                  30u, 10u, 20u, 50u, 90u, 0, 0, 1u);
    assert(ee_player_attention_update_scores_for_scheduler_step(
        &list, &config, &history, NULL, &frame) == 1u);
    assert(features_for(&frame, 10u)->interaction_score == config.interaction_neighbor_points);
    printf("[PASS] V3 interaction rewards nearby approaching PLAYER pairs, survives target reorder, and rejects stale pair history.\n");
}

static void test_aging_ball_context_and_scheduler_view(void)
{
    EE_PlayerAttentionConfig config = ee_player_attention_default_config();
    EE_PlayerAttentionHistory history;
    EE_PlayerAttentionFrame frame;
    EE_GlobalTargetList list;
    EE_GlobalTargetList candidates;
    EE_AttentionScheduler scheduler;
    EE_FoveaRequest request;
    uint16_t waiting[EE_MAX_TARGETS] = {0u};
    const EE_PlayerAttentionFeatures *near_player;
    uint32_t score_without_context;

    ee_player_attention_history_init(&history);
    reset_list(&list, 1u);
    list.count = 3u;
    list.targets[0] = make_target(1u, EE_SOCCER_TARGET_BALL, 1u,
                                  110u, 110u, 10u, 10u, 90u, 0, 0, 1u);
    list.targets[1] = make_target(10u, EE_SOCCER_TARGET_PLAYER, 1u,
                                  100u, 100u, 20u, 50u, 90u, 5, 0, 1u);
    list.targets[2] = make_target(20u, EE_SOCCER_TARGET_PLAYER, 1u,
                                  300u, 100u, 100u, 150u, 90u, 5, 0, 1u);
    waiting[1] = 29u;
    assert(ee_player_attention_update_scores_for_scheduler_step(
        &list, &config, &history, waiting, &frame) == 1u);
    near_player = features_for(&frame, 10u);
    assert(near_player->aging_score == 255u);
    assert(near_player->optional_ball_bonus == 0u);
    score_without_context = near_player->total_attention;

    config.player_ball_context_enable = 1u;
    assert(ee_player_attention_update_scores_for_scheduler_step(
        &list, &config, &history, waiting, &frame) == 1u);
    near_player = features_for(&frame, 10u);
    assert(near_player->optional_ball_bonus > 0u);
    assert(near_player->total_attention > score_without_context);
    assert(ee_player_attention_candidate_view(&list, &candidates) == 1u);
    assert(candidates.targets[0].valid == 0u);
    assert(candidates.targets[1].valid == 1u && candidates.targets[2].valid == 1u);
    ee_scheduler_init(&scheduler, ee_scheduler_default_config());
    assert(ee_scheduler_step(&scheduler, &candidates, &request) == 1u);
    assert(request.valid && request.target_id != 1u);
    assert(candidates.targets[request.target_id == 10u ? 1u : 2u].target_type ==
           EE_SOCCER_TARGET_PLAYER);
    printf("[PASS] V3 aging keeps Scheduler semantics; BALL context is optional and BALL cannot be selected.\n");
}

static void test_small_target_is_not_a_v3_rank_feature(void)
{
    EE_PlayerAttentionConfig config = ee_player_attention_default_config();
    EE_PlayerAttentionHistory history;
    EE_PlayerAttentionFrame frame;
    EE_GlobalTargetList list;

    ee_player_attention_history_init(&history);
    reset_list(&list, 1u);
    list.count = 2u;
    list.targets[0] = make_target(10u, EE_SOCCER_TARGET_PLAYER, 1u,
                                  100u, 100u, 5u, 5u, 90u, 2, 1, 1u);
    list.targets[1] = make_target(20u, EE_SOCCER_TARGET_PLAYER, 1u,
                                  300u, 100u, 100u, 150u, 90u, 2, 1, 1u);
    assert(ee_player_attention_update_scores_for_scheduler_step(
        &list, &config, &history, NULL, &frame) == 1u);
    assert(features_for(&frame, 10u)->total_attention ==
           features_for(&frame, 20u)->total_attention);
    printf("[PASS] V3 ranking does not use Small Target; Enhance Policy retains its independent input.\n");
}

int main(void)
{
    test_motion_uncertainty_and_saliency();
    test_interaction_history_and_reorder();
    test_aging_ball_context_and_scheduler_view();
    test_small_target_is_not_a_v3_rank_feature();
    printf("Player-driven Attention V3 unit tests passed.\n");
    return 0;
}
