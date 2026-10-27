#ifndef RISK_H
#define RISK_H

#include "eagleeye_types.h"

#ifdef __cplusplus
extern "C" {
#endif

float ee_confirmed_risk_score(const EE_GlobalTargetDescriptor *global,
                              const EE_RefinedTargetDescriptor *refined);
EE_RiskLevel ee_risk_level_from_score(float score);

#ifdef __cplusplus
}
#endif

#endif
