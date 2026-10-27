#include "attention_score.h"
#include <assert.h>

int main(void)
{
    EE_AttentionWeights weights = ee_attention_default_weights();
    EE_GlobalTargetDescriptor low = {0};
    EE_GlobalTargetDescriptor high = {0};

    low.valid = 1u;
    low.zone_id = EE_ZONE_LEFT;
    low.w = 90u;
    low.h = 80u;
    low.coarse_conf = 95u;

    high.valid = 1u;
    high.zone_id = EE_ZONE_EGO_LANE;
    high.w = 24u;
    high.h = 40u;
    high.coarse_conf = 48u;
    high.approach_level = 70u;

    assert(ee_attention_score_compute(&high, &weights, 0u) >
           ee_attention_score_compute(&low, &weights, 0u));
    return 0;
}
