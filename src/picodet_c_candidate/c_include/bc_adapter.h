#ifndef BC_ADAPTER_H
#define BC_ADAPTER_H

#include <stdio.h>
#include <stdint.h>

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EE_BC_STREAM_MAGIC "EE_B_TARGET_STREAM_V1"
#define EE_BC_FRAME_WIDTH 640u
#define EE_BC_FRAME_HEIGHT 360u

/* B-local fields stay here so the frozen C descriptor does not grow. */
typedef struct {
    uint32_t frame_id;
    uint16_t target_id;
    uint8_t target_type;
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    uint8_t confidence;
    uint8_t valid;
    int16_t vx;
    int16_t vy;
    uint8_t velocity_valid;
    uint16_t age;
    uint16_t miss_count;
} EE_BParsedTarget;

typedef struct {
    uint32_t frame_id;
    /* Source-video coordinate system declared by optional STREAM_INFO. */
    uint32_t source_width;
    uint32_t source_height;
    uint8_t count;
    EE_BParsedTarget targets[EE_MAX_TARGETS];
} EE_BParsedFrame;

typedef struct {
    FILE *file;
    uint32_t line_number;
    uint32_t previous_frame_id;
    uint32_t source_width;
    uint32_t source_height;
    uint8_t header_read;
    uint8_t stream_info_read;
    uint8_t has_previous_frame;
    char error_message[160];
} EE_BCStreamReader;

void ee_bc_stream_reader_init(EE_BCStreamReader *reader, FILE *file);
/* Returns 1 for a parsed frame, 0 for clean EOF, and -1 for rejected input. */
int ee_bc_stream_read_frame(EE_BCStreamReader *reader, EE_BParsedFrame *out_frame);
const char *ee_bc_stream_error(const EE_BCStreamReader *reader);

/* Converts B-local parsed data to the existing frozen C-side descriptor. */
uint8_t ee_bc_adapt_frame(const EE_BParsedFrame *source, EE_GlobalTargetList *out_list);

#ifdef __cplusplus
}
#endif

#endif
