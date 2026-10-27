#include "attention.h"
#include "attention_scheduler.h"
#include "bc_adapter.h"
#include "possession_estimator.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static FILE *open_text_stream(const char *text)
{
    FILE *file = tmpfile();
    assert(file != NULL);
    assert(fputs(text, file) >= 0);
    rewind(file);
    return file;
}

static void assert_rejected(const char *text, const char *reason_fragment)
{
    EE_BCStreamReader reader;
    EE_BParsedFrame parsed;
    FILE *file = open_text_stream(text);
    ee_bc_stream_reader_init(&reader, file);
    assert(ee_bc_stream_read_frame(&reader, &parsed) == -1);
    if (reason_fragment) assert(strstr(ee_bc_stream_error(&reader), reason_fragment) != NULL);
    fclose(file);
}

static void test_sample_stream(void)
{
    EE_BCStreamReader reader;
    EE_BParsedFrame parsed;
    EE_GlobalTargetList list;
    EE_AttentionScheduler scheduler;
    EE_AttentionConfig attention = ee_attention_default_config();
    EE_FoveaRequest request;
    FILE *file = fopen("examples/b_target_stream_v1_sample.txt", "r");
    uint16_t previous_carrier = EE_INVALID_TARGET_ID;
    uint8_t previous_score = 0u;
    uint16_t carrier;
    uint8_t score;
    uint32_t expected_frame = 0u;
    int result;

    assert(file != NULL);
    ee_bc_stream_reader_init(&reader, file);
    ee_scheduler_init(&scheduler, ee_scheduler_default_config());

    while ((result = ee_bc_stream_read_frame(&reader, &parsed)) == 1) {
        assert(parsed.frame_id == expected_frame);
        assert(parsed.source_width == EE_BC_FRAME_WIDTH);
        assert(parsed.source_height == EE_BC_FRAME_HEIGHT);
        assert(parsed.count == 3u);
        assert(parsed.targets[0].target_id == 1u);
        assert(parsed.targets[1].target_id == 2u);
        assert(parsed.targets[2].target_id == 3u);
        assert(ee_bc_adapt_frame(&parsed, &list) == 1u);
        assert(list.frame_id == expected_frame);
        assert(list.targets[2].is_ball == 1u);

        ee_possession_update_list(&list, previous_carrier, previous_score, &carrier, &score);
        previous_carrier = carrier;
        previous_score = score;
        ee_attention_update_scores_for_scheduler_step(
            list.targets, list.count, &attention, scheduler.wait_frames);
        assert(ee_scheduler_step(&scheduler, &list, &request) == 1u);
        assert(request.valid == 1u);

        if (expected_frame == 8u) {
            assert(list.targets[2].valid == 0u);
            assert(request.target_id != 3u);
        }
        if (expected_frame == 9u) {
            assert(list.targets[2].valid == 1u);
            assert(list.targets[2].target_id == 3u);
        }
        expected_frame++;
    }
    assert(result == 0);
    assert(expected_frame == 12u);
    fclose(file);
    printf("[PASS] B sample: 12 frames; PLAYER #1/#2 and BALL #3 remain stable; invalid BALL is skipped and recovers.\n");
}

static void assert_normalized_target(const char *text,
                                     uint16_t expected_x,
                                     uint16_t expected_y,
                                     uint16_t expected_w,
                                     uint16_t expected_h,
                                     int16_t expected_vx,
                                     int16_t expected_vy)
{
    EE_BCStreamReader reader;
    EE_BParsedFrame parsed;
    EE_GlobalTargetList list;
    FILE *file = open_text_stream(text);

    ee_bc_stream_reader_init(&reader, file);
    assert(ee_bc_stream_read_frame(&reader, &parsed) == 1);
    assert(parsed.count == 1u);
    assert(ee_bc_adapt_frame(&parsed, &list) == 1u);
    assert(list.count == 1u);
    assert(list.targets[0].x == expected_x && list.targets[0].y == expected_y);
    assert(list.targets[0].w == expected_w && list.targets[0].h == expected_h);
    assert(list.targets[0].vx == expected_vx && list.targets[0].vy == expected_vy);
    assert(ee_bc_stream_read_frame(&reader, &parsed) == 0);
    fclose(file);
}

static void test_source_resolution_normalization(void)
{
    static const char canonical[] =
        "EE_B_TARGET_STREAM_V1\nSTREAM_INFO,640,360\nFRAME,1,1\n"
        "TARGET,1,7,BALL,100,50,20,10,80,1,5,2,1,1,0\nEND_FRAME\n";
    static const char hd[] =
        "EE_B_TARGET_STREAM_V1\nSTREAM_INFO,1280,720\nFRAME,1,1\n"
        "TARGET,1,7,BALL,200,100,40,20,80,1,10,4,1,1,0\nEND_FRAME\n";
    static const char full_hd[] =
        "EE_B_TARGET_STREAM_V1\nSTREAM_INFO,1920,1080\nFRAME,1,1\n"
        "TARGET,1,7,BALL,300,150,60,30,80,1,15,6,1,1,0\nEND_FRAME\n";
    static const char signed_velocity[] =
        "EE_B_TARGET_STREAM_V1\nSTREAM_INFO,1280,720\nFRAME,1,1\n"
        "TARGET,1,7,BALL,200,100,40,20,80,1,-10,-4,1,1,0\nEND_FRAME\n";
    static const char saturated_velocity[] =
        "EE_B_TARGET_STREAM_V1\nSTREAM_INFO,1,1\nFRAME,1,1\n"
        "TARGET,1,7,BALL,0,0,1,1,80,1,-32768,32767,1,1,0\nEND_FRAME\n";

    assert_normalized_target(canonical, 100u, 50u, 20u, 10u, 5, 2);
    assert_normalized_target(hd, 100u, 50u, 20u, 10u, 5, 2);
    assert_normalized_target(full_hd, 100u, 50u, 20u, 10u, 5, 2);
    assert_normalized_target(signed_velocity, 100u, 50u, 20u, 10u, -5, -2);
    assert_normalized_target(saturated_velocity, 0u, 0u, 640u, 360u, INT16_MIN, INT16_MAX);
    printf("[PASS] STREAM_INFO normalizes 640x360, 1280x720, and 1920x1080 boxes and signed velocities to C 640x360.\n");
}

static void test_rejected_streams(void)
{
    static const char nonmonotonic[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,2,0\nEND_FRAME\nFRAME,1,0\nEND_FRAME\n";
    static const char bad_count[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,0,17\n";
    static const char bad_bbox[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,0,1\nTARGET,0,1,BALL,630,350,20,20,80,1,0,0,1,1,0\nEND_FRAME\n";
    static const char zero_id[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,0,1\nTARGET,0,0,BALL,10,10,12,12,80,1,0,0,1,1,0\nEND_FRAME\n";
    static const char duplicate_frame[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,2,0\nEND_FRAME\nFRAME,2,0\nEND_FRAME\n";
    static const char count_mismatch[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,0,2\nTARGET,0,1,BALL,10,10,12,12,80,1,0,0,1,1,0\nEND_FRAME\n";
    static const char missing_end[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,0,1\nTARGET,0,1,BALL,10,10,12,12,80,1,0,0,1,1,0\n";
    static const char bad_type[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,0,1\nTARGET,0,1,GOAL,10,10,12,12,80,1,0,0,1,1,0\nEND_FRAME\n";
    static const char negative_confidence[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,0,1\nTARGET,0,1,BALL,10,10,12,12,-1,1,0,0,1,1,0\nEND_FRAME\n";
    static const char high_confidence[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,0,1\nTARGET,0,1,BALL,10,10,12,12,101,1,0,0,1,1,0\nEND_FRAME\n";
    static const char zero_width[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,0,1\nTARGET,0,1,BALL,10,10,0,12,80,1,0,0,1,1,0\nEND_FRAME\n";
    static const char zero_height[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,0,1\nTARGET,0,1,BALL,10,10,12,0,80,1,0,0,1,1,0\nEND_FRAME\n";
    static const char duplicate_target[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,0,2\n"
        "TARGET,0,1,PLAYER,10,10,20,40,80,1,0,0,1,1,0\n"
        "TARGET,0,1,BALL,40,40,12,12,80,1,0,0,1,1,0\nEND_FRAME\n";
    static const char target_frame_mismatch[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,0,1\nTARGET,1,1,BALL,10,10,12,12,80,1,0,0,1,1,0\nEND_FRAME\n";
    static const char zero_source_width[] =
        "EE_B_TARGET_STREAM_V1\nSTREAM_INFO,0,360\nFRAME,0,0\nEND_FRAME\n";
    static const char zero_source_height[] =
        "EE_B_TARGET_STREAM_V1\nSTREAM_INFO,640,0\nFRAME,0,0\nEND_FRAME\n";
    static const char negative_source_width[] =
        "EE_B_TARGET_STREAM_V1\nSTREAM_INFO,-640,360\nFRAME,0,0\nEND_FRAME\n";
    static const char negative_source_height[] =
        "EE_B_TARGET_STREAM_V1\nSTREAM_INFO,640,-360\nFRAME,0,0\nEND_FRAME\n";
    static const char repeated_stream_info[] =
        "EE_B_TARGET_STREAM_V1\nSTREAM_INFO,640,360\nSTREAM_INFO,1280,720\n";
    static const char late_stream_info[] =
        "EE_B_TARGET_STREAM_V1\nFRAME,0,0\nEND_FRAME\nSTREAM_INFO,640,360\n";
    static const char source_bbox_out_of_range[] =
        "EE_B_TARGET_STREAM_V1\nSTREAM_INFO,1280,720\nFRAME,0,1\n"
        "TARGET,0,1,BALL,1270,100,20,20,80,1,0,0,1,1,0\nEND_FRAME\n";
    static const char scaled_empty_bbox[] =
        "EE_B_TARGET_STREAM_V1\nSTREAM_INFO,1280,720\nFRAME,0,1\n"
        "TARGET,0,1,BALL,0,0,1,2,80,1,0,0,1,1,0\nEND_FRAME\n";
    EE_BCStreamReader reader;
    EE_BParsedFrame parsed;
    FILE *file;

    file = open_text_stream(nonmonotonic);
    ee_bc_stream_reader_init(&reader, file);
    assert(ee_bc_stream_read_frame(&reader, &parsed) == 1);
    assert(ee_bc_stream_read_frame(&reader, &parsed) == -1);
    assert(strstr(ee_bc_stream_error(&reader), "STALE / NON_MONOTONIC FRAME") != NULL);
    fclose(file);

    file = open_text_stream(duplicate_frame);
    ee_bc_stream_reader_init(&reader, file);
    assert(ee_bc_stream_read_frame(&reader, &parsed) == 1);
    assert(ee_bc_stream_read_frame(&reader, &parsed) == -1);
    assert(strstr(ee_bc_stream_error(&reader), "STALE / NON_MONOTONIC FRAME") != NULL);
    fclose(file);

    file = open_text_stream(bad_count);
    ee_bc_stream_reader_init(&reader, file);
    assert(ee_bc_stream_read_frame(&reader, &parsed) == -1);
    fclose(file);

    file = open_text_stream(bad_bbox);
    ee_bc_stream_reader_init(&reader, file);
    assert(ee_bc_stream_read_frame(&reader, &parsed) == -1);
    fclose(file);

    file = open_text_stream(zero_id);
    ee_bc_stream_reader_init(&reader, file);
    assert(ee_bc_stream_read_frame(&reader, &parsed) == -1);
    fclose(file);

    assert_rejected(count_mismatch, "invalid TARGET record");
    assert_rejected(missing_end, "missing END_FRAME");
    assert_rejected(bad_type, "invalid TARGET record");
    assert_rejected(negative_confidence, "invalid TARGET record");
    assert_rejected(high_confidence, "invalid TARGET record");
    assert_rejected(zero_width, "invalid TARGET record");
    assert_rejected(zero_height, "invalid TARGET record");
    assert_rejected(duplicate_target, "duplicate target_id");
    assert_rejected(target_frame_mismatch, "invalid TARGET record");
    assert_rejected(zero_source_width, "invalid STREAM_INFO dimensions");
    assert_rejected(zero_source_height, "invalid STREAM_INFO dimensions");
    assert_rejected(negative_source_width, "invalid STREAM_INFO dimensions");
    assert_rejected(negative_source_height, "invalid STREAM_INFO dimensions");
    assert_rejected(repeated_stream_info, "duplicate STREAM_INFO record");

    file = open_text_stream(late_stream_info);
    ee_bc_stream_reader_init(&reader, file);
    assert(ee_bc_stream_read_frame(&reader, &parsed) == 1);
    assert(ee_bc_stream_read_frame(&reader, &parsed) == -1);
    assert(strstr(ee_bc_stream_error(&reader), "STREAM_INFO must precede") != NULL);
    fclose(file);

    assert_rejected(source_bbox_out_of_range, "invalid TARGET record");
    file = open_text_stream(scaled_empty_bbox);
    ee_bc_stream_reader_init(&reader, file);
    assert(ee_bc_stream_read_frame(&reader, &parsed) == 1);
    {
        EE_GlobalTargetList list;
        assert(ee_bc_adapt_frame(&parsed, &list) == 0u);
    }
    fclose(file);
    printf("[PASS] B parser rejects malformed STREAM_INFO, late/duplicate metadata, and invalid source boxes.\n");
}

int main(void)
{
    test_sample_stream();
    test_source_resolution_normalization();
    test_rejected_streams();
    printf("All B->C adapter tests passed.\n");
    return 0;
}
