#ifndef FAKE_DATA_H
#define FAKE_DATA_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void ee_fake_generate_global_targets(uint32_t frame_id, EE_GlobalTargetList *out_list);
void ee_fake_generate_soccer_scenario(uint8_t scenario,
                                      uint32_t frame_id,
                                      EE_GlobalTargetList *out_list);
uint8_t ee_fake_generate_refined_result(const EE_FoveaRequest *request,
                                        uint32_t current_frame_id,
                                        EE_RefinedTargetDescriptor *out_refined);

#ifdef __cplusplus
}
#endif

#endif
