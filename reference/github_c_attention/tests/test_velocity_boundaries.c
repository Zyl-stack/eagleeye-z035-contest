#include "attention.h"
#include "enhance_policy.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static EE_GlobalTargetDescriptor make_target(int16_t vx, int16_t vy)
{
    EE_GlobalTargetDescriptor target;

    memset(&target, 0, sizeof(target));
    target.target_id = 1u;
    target.valid = 1u;
    target.w = 40u;
    target.h = 80u;
    target.coarse_conf = 90u;
    target.vx = vx;
    target.vy = vy;
    return target;
}

static uint8_t motion_score(int16_t vx, int16_t vy)
{
    EE_AttentionConfig config = ee_attention_default_config();
    EE_GlobalTargetDescriptor target = make_target(vx, vy);
    EE_AttentionDebug debug;

    ee_attention_compute_debug(&target, &config, &debug);
    return debug.motion_score;
}

static void test_attention_velocity_boundaries(void)
{
    assert(motion_score(0, 0) == 0u);
    assert(motion_score(1, 1) == 1u);
    assert(motion_score(-1, -1) == 1u);
    assert(motion_score(255, 255) == 255u);
    assert(motion_score(-255, -255) == 255u);
    assert(motion_score(256, 256) == 255u);
    assert(motion_score(-256, -256) == 255u);
    assert(motion_score(INT16_MAX, INT16_MAX) == 255u);
    assert(motion_score(INT16_MIN, 0) == 127u);
    assert(motion_score(0, INT16_MIN) == 127u);
    assert(motion_score(INT16_MIN, INT16_MIN) == 255u);
    assert(motion_score(INT16_MAX, INT16_MIN) == 255u);
    assert(motion_score(6, -2) == 4u);
    printf("Attention signed velocity boundary tests passed.\n");
}

static EE_EnhanceDecision enhance_decision(int16_t vx, int16_t vy)
{
    EE_CConfig config;
    EE_GlobalTargetDescriptor target = make_target(vx, vy);
    EE_AttentionBreakdown attention;
    EE_EnhancePolicyInput input;

    memset(&config, 0, sizeof(config));
    memset(&attention, 0, sizeof(attention));
    memset(&input, 0, sizeof(input));
    config.high_attention_threshold = 1500u;
    config.small_target_threshold = 200u;
    config.fast_motion_threshold = 20u;
    config.default_enhance_mode = EE_ENHANCE_MODE_BILINEAR;
    config.high_quality_enhance_mode = EE_ENHANCE_MODE_LANCZOS;
    input.selected_target = &target;
    input.attention = &attention;
    input.possession_target_id = EE_INVALID_TARGET_ID;
    return ee_enhance_policy_decide(&input, &config);
}

static void assert_fast_motion(int16_t vx, int16_t vy)
{
    EE_EnhanceDecision decision = enhance_decision(vx, vy);

    assert(decision.mode == EE_ENHANCE_MODE_BILINEAR);
    assert(decision.reason == EE_ENHANCE_REASON_FAST_MOTION);
}

static void test_enhance_policy_velocity_boundaries(void)
{
    EE_EnhanceDecision decision;

    assert_fast_motion(INT16_MIN, INT16_MIN);
    assert_fast_motion(INT16_MAX, 0);
    assert_fast_motion(INT16_MIN, 0);
    assert_fast_motion(255, 0);
    assert_fast_motion(-255, 0);
    assert_fast_motion(256, 0);
    assert_fast_motion(-256, 0);
    assert_fast_motion(20, 0);

    decision = enhance_decision(0, 0);
    assert(decision.mode == EE_ENHANCE_MODE_BILINEAR);
    assert(decision.reason == EE_ENHANCE_REASON_NORMAL);
    decision = enhance_decision(19, 0);
    assert(decision.mode == EE_ENHANCE_MODE_BILINEAR);
    assert(decision.reason == EE_ENHANCE_REASON_NORMAL);
    printf("Enhance policy signed velocity boundary tests passed.\n");
}

int main(void)
{
    test_attention_velocity_boundaries();
    test_enhance_policy_velocity_boundaries();
    printf("All signed velocity boundary tests passed.\n");
    return 0;
}
