#if defined(_MSC_VER)
/* Keep the portable C parser APIs usable under MSVC /W4. */
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "bc_adapter.h"

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define EE_BC_STREAM_LINE_CAPACITY 256u

static void set_error(EE_BCStreamReader *reader, const char *format, ...)
{
    va_list args;

    if (!reader) {
        return;
    }
    va_start(args, format);
    (void)vsnprintf(reader->error_message, sizeof(reader->error_message), format, args);
    va_end(args);
    fprintf(stderr, "B->C STREAM REJECTED: %s\n", reader->error_message);
}

static int read_content_line(EE_BCStreamReader *reader, char *line, size_t capacity)
{
    while (fgets(line, (int)capacity, reader->file) != NULL) {
        size_t length;

        reader->line_number++;
        length = strlen(line);
        while (length > 0u && (line[length - 1u] == '\n' || line[length - 1u] == '\r')) {
            line[--length] = '\0';
        }
        if (length == 0u || line[0] == '#') {
            continue;
        }
        return 1;
    }
    return 0;
}

static int target_type_from_text(const char *text, uint8_t *out_type)
{
    if (strcmp(text, "BALL") == 0) {
        *out_type = EE_SOCCER_TARGET_BALL;
    } else if (strcmp(text, "PLAYER") == 0) {
        *out_type = EE_SOCCER_TARGET_PLAYER;
    } else if (strcmp(text, "REFEREE") == 0) {
        *out_type = EE_SOCCER_TARGET_REFEREE;
    } else if (strcmp(text, "UNKNOWN") == 0) {
        *out_type = EE_SOCCER_TARGET_UNKNOWN;
    } else {
        return 0;
    }
    return 1;
}

static int bbox_is_valid(unsigned int x,
                         unsigned int y,
                         unsigned int w,
                         unsigned int h,
                         uint32_t source_width,
                         uint32_t source_height)
{
    return source_width > 0u && source_height > 0u &&
           x < source_width && y < source_height && w > 0u && h > 0u &&
           x <= USHRT_MAX && y <= USHRT_MAX && w <= USHRT_MAX && h <= USHRT_MAX &&
           (uint64_t)x + w <= source_width && (uint64_t)y + h <= source_height;
}

static int is_stream_info_record(const char *line)
{
    return strncmp(line, "STREAM_INFO,", sizeof("STREAM_INFO,") - 1u) == 0;
}

static int parse_stream_info(EE_BCStreamReader *reader, const char *line)
{
    long source_width;
    long source_height;

    if (sscanf(line, "STREAM_INFO,%ld,%ld", &source_width, &source_height) != 2 ||
        source_width <= 0L || source_height <= 0L ||
        (uint64_t)source_width > UINT32_MAX || (uint64_t)source_height > UINT32_MAX) {
        set_error(reader, "line %lu: invalid STREAM_INFO dimensions", (unsigned long)reader->line_number);
        return 0;
    }
    reader->source_width = (uint32_t)source_width;
    reader->source_height = (uint32_t)source_height;
    reader->stream_info_read = 1u;
    return 1;
}

void ee_bc_stream_reader_init(EE_BCStreamReader *reader, FILE *file)
{
    if (!reader) {
        return;
    }
    memset(reader, 0, sizeof(*reader));
    reader->file = file;
    /* V1 streams without metadata retain the original 640x360 interpretation. */
    reader->source_width = EE_BC_FRAME_WIDTH;
    reader->source_height = EE_BC_FRAME_HEIGHT;
}

const char *ee_bc_stream_error(const EE_BCStreamReader *reader)
{
    return reader ? reader->error_message : "reader is NULL";
}

int ee_bc_stream_read_frame(EE_BCStreamReader *reader, EE_BParsedFrame *out_frame)
{
    char line[EE_BC_STREAM_LINE_CAPACITY];
    unsigned long frame_id;
    unsigned int count;
    unsigned int i;

    if (!reader || !reader->file || !out_frame) {
        return -1;
    }
    reader->error_message[0] = '\0';
    if (!reader->header_read) {
        if (!read_content_line(reader, line, sizeof(line))) {
            return 0;
        }
        if (strcmp(line, EE_BC_STREAM_MAGIC) != 0) {
            set_error(reader, "line %lu: unsupported stream header", (unsigned long)reader->line_number);
            return -1;
        }
        reader->header_read = 1u;
        if (!read_content_line(reader, line, sizeof(line))) {
            return 0;
        }
        if (is_stream_info_record(line)) {
            if (!parse_stream_info(reader, line)) {
                return -1;
            }
            if (!read_content_line(reader, line, sizeof(line))) {
                set_error(reader, "STREAM_INFO is missing its first FRAME record");
                return -1;
            }
        }
    } else if (!read_content_line(reader, line, sizeof(line))) {
        return 0;
    }
    if (is_stream_info_record(line)) {
        set_error(reader, reader->stream_info_read ?
                  "line %lu: duplicate STREAM_INFO record" :
                  "line %lu: STREAM_INFO must precede the first FRAME record",
                  (unsigned long)reader->line_number);
        return -1;
    }
    if (sscanf(line, "FRAME,%lu,%u", &frame_id, &count) != 2 || frame_id > UINT32_MAX ||
        count > EE_MAX_TARGETS) {
        set_error(reader, "line %lu: invalid FRAME record or target count", (unsigned long)reader->line_number);
        return -1;
    }
    if (reader->has_previous_frame && frame_id <= reader->previous_frame_id) {
        set_error(reader, "STALE / NON_MONOTONIC FRAME current=%lu previous=%lu",
                  frame_id, (unsigned long)reader->previous_frame_id);
        return -1;
    }

    memset(out_frame, 0, sizeof(*out_frame));
    out_frame->frame_id = (uint32_t)frame_id;
    out_frame->source_width = reader->source_width;
    out_frame->source_height = reader->source_height;
    out_frame->count = (uint8_t)count;
    for (i = 0u; i < count; ++i) {
        unsigned long target_frame_id;
        unsigned int target_id;
        unsigned int x;
        unsigned int y;
        unsigned int w;
        unsigned int h;
        unsigned int confidence;
        unsigned int valid;
        unsigned int velocity_valid;
        unsigned int age;
        unsigned int miss_count;
        int vx;
        int vy;
        char type_text[16];
        EE_BParsedTarget *target = &out_frame->targets[i];
        unsigned int previous_index;

        if (!read_content_line(reader, line, sizeof(line))) {
            set_error(reader, "unexpected EOF inside frame %lu", frame_id);
            return -1;
        }
        if (sscanf(line,
                   "TARGET,%lu,%u,%15[^,],%u,%u,%u,%u,%u,%u,%d,%d,%u,%u,%u",
                   &target_frame_id, &target_id, type_text, &x, &y, &w, &h,
                   &confidence, &valid, &vx, &vy, &velocity_valid, &age, &miss_count) != 14 ||
            target_frame_id != frame_id || target_id == 0u || target_id >= EE_INVALID_TARGET_ID ||
            !bbox_is_valid(x, y, w, h, reader->source_width, reader->source_height) || confidence > 100u ||
            valid > 1u || vx < SHRT_MIN || vx > SHRT_MAX || vy < SHRT_MIN || vy > SHRT_MAX ||
            velocity_valid > 1u || age > USHRT_MAX || miss_count > USHRT_MAX ||
            !target_type_from_text(type_text, &target->target_type)) {
            set_error(reader, "line %lu: invalid TARGET record", (unsigned long)reader->line_number);
            return -1;
        }
        for (previous_index = 0u; previous_index < i; ++previous_index) {
            if (out_frame->targets[previous_index].target_id == (uint16_t)target_id) {
                set_error(reader, "line %lu: duplicate target_id %u", (unsigned long)reader->line_number, target_id);
                return -1;
            }
        }
        target->frame_id = (uint32_t)target_frame_id;
        target->target_id = (uint16_t)target_id;
        target->x = (uint16_t)x;
        target->y = (uint16_t)y;
        target->w = (uint16_t)w;
        target->h = (uint16_t)h;
        target->confidence = (uint8_t)confidence;
        target->valid = (uint8_t)valid;
        target->vx = (int16_t)vx;
        target->vy = (int16_t)vy;
        target->velocity_valid = (uint8_t)velocity_valid;
        target->age = (uint16_t)age;
        target->miss_count = (uint16_t)miss_count;
    }
    if (!read_content_line(reader, line, sizeof(line)) || strcmp(line, "END_FRAME") != 0) {
        set_error(reader, "frame %lu is missing END_FRAME", frame_id);
        return -1;
    }
    reader->previous_frame_id = (uint32_t)frame_id;
    reader->has_previous_frame = 1u;
    return 1;
}

static uint8_t motion_level_from_velocity(int16_t vx, int16_t vy, uint8_t velocity_valid)
{
    int32_t abs_vx;
    int32_t abs_vy;
    uint32_t average;

    if (!velocity_valid) {
        return 0u;
    }
    abs_vx = vx < 0 ? -(int32_t)vx : (int32_t)vx;
    abs_vy = vy < 0 ? -(int32_t)vy : (int32_t)vy;
    average = ((uint32_t)abs_vx + (uint32_t)abs_vy) / 2u;
    return average > 255u ? 255u : (uint8_t)average;
}

static uint32_t scale_coordinate(uint32_t coordinate, uint32_t source_extent, uint32_t canonical_extent)
{
    return (uint32_t)(((uint64_t)coordinate * canonical_extent) / source_extent);
}

static int16_t scale_velocity(int16_t velocity, uint32_t source_extent, uint32_t canonical_extent)
{
    int64_t scaled;
    int64_t magnitude;

    scaled = (int64_t)velocity * canonical_extent;
    magnitude = scaled < 0 ? -scaled : scaled;
    /* Symmetric nearest-integer rounding keeps small signed motions meaningful. */
    magnitude = (magnitude + (int64_t)(source_extent / 2u)) / source_extent;
    if (scaled < 0) {
        magnitude = -magnitude;
    }
    if (magnitude > INT16_MAX) {
        return INT16_MAX;
    }
    if (magnitude < INT16_MIN) {
        return INT16_MIN;
    }
    return (int16_t)magnitude;
}

static uint8_t normalize_bbox(const EE_BParsedFrame *source,
                              const EE_BParsedTarget *input,
                              EE_GlobalTargetDescriptor *output)
{
    uint32_t x1;
    uint32_t y1;
    uint32_t x2;
    uint32_t y2;

    if (!source || !input || !output || source->source_width == 0u || source->source_height == 0u ||
        !bbox_is_valid(input->x, input->y, input->w, input->h,
                       source->source_width, source->source_height)) {
        return 0u;
    }
    x1 = scale_coordinate(input->x, source->source_width, EE_BC_FRAME_WIDTH);
    y1 = scale_coordinate(input->y, source->source_height, EE_BC_FRAME_HEIGHT);
    x2 = scale_coordinate((uint32_t)input->x + input->w,
                          source->source_width, EE_BC_FRAME_WIDTH);
    y2 = scale_coordinate((uint32_t)input->y + input->h,
                          source->source_height, EE_BC_FRAME_HEIGHT);
    if (x1 >= EE_BC_FRAME_WIDTH || y1 >= EE_BC_FRAME_HEIGHT || x2 <= x1 || y2 <= y1 ||
        x2 > EE_BC_FRAME_WIDTH || y2 > EE_BC_FRAME_HEIGHT) {
        return 0u;
    }
    output->x = (uint16_t)x1;
    output->y = (uint16_t)y1;
    output->w = (uint16_t)(x2 - x1);
    output->h = (uint16_t)(y2 - y1);
    return 1u;
}

uint8_t ee_bc_adapt_frame(const EE_BParsedFrame *source, EE_GlobalTargetList *out_list)
{
    uint8_t i;

    if (!source || !out_list || source->count > EE_MAX_TARGETS) {
        return 0u;
    }
    memset(out_list, 0, sizeof(*out_list));
    out_list->frame_id = source->frame_id;
    out_list->count = source->count;
    for (i = 0u; i < source->count; ++i) {
        const EE_BParsedTarget *input = &source->targets[i];
        EE_GlobalTargetDescriptor *output = &out_list->targets[i];

        output->target_id = input->target_id;
        output->frame_id = input->frame_id;
        if (!normalize_bbox(source, input, output)) {
            memset(out_list, 0, sizeof(*out_list));
            return 0u;
        }
        output->target_type = input->target_type;
        output->coarse_class = input->target_type == EE_SOCCER_TARGET_BALL ?
                               EE_CLASS_UNKNOWN : EE_CLASS_PEDESTRIAN;
        output->coarse_conf = input->confidence;
        output->valid = input->valid;
        output->vx = scale_velocity(input->vx, source->source_width, EE_BC_FRAME_WIDTH);
        output->vy = scale_velocity(input->vy, source->source_height, EE_BC_FRAME_HEIGHT);
        output->motion_level = motion_level_from_velocity(output->vx, output->vy, input->velocity_valid);
        output->uncertainty = (uint8_t)(((100u - input->confidence) * 255u + 50u) / 100u);
        if (output->uncertainty == 0u) {
            output->uncertainty = 1u;
        }
        output->zone_id = EE_ZONE_CENTER;
        /* TODO(B/C): replace FIELD_NORMAL with field-calibration based zones. */
        output->field_zone = EE_FIELD_NORMAL;
        output->is_ball = input->target_type == EE_SOCCER_TARGET_BALL ? 1u : 0u;
    }
    return 1u;
}
