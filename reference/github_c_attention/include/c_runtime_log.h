#ifndef C_RUNTIME_LOG_H
#define C_RUNTIME_LOG_H

#include "attention_scheduler.h"
#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EE_LOG_LEVEL_NONE = 0,
    EE_LOG_LEVEL_SUMMARY,
    EE_LOG_LEVEL_VERBOSE
} EE_LogLevel;

typedef struct {
    EE_LogLevel level;
} EE_RuntimeLogger;

typedef struct {
    const EE_GlobalTargetList *targets;
    const EE_FoveaRequest *request;
    const EE_SchedulerDecision *decision;
    uint16_t possession_target_id;
    uint8_t possession_score;
    uint8_t timeout;
    uint8_t stale_result_rejected;
} EE_RuntimeFrameLog;

void ee_runtime_logger_init(EE_RuntimeLogger *logger, EE_LogLevel level);
void ee_runtime_log_frame(const EE_RuntimeLogger *logger, const EE_RuntimeFrameLog *frame);

#ifdef __cplusplus
}
#endif

#endif
