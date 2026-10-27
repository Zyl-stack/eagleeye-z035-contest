#pragma once
#include "fovea_roi.hpp"
#include "bc_adapter.h"
#include "c_config.h"
#include "fovea_transaction.h"
#include <cstdint>
#include <vector>

namespace eagleeye {
struct CAttentionResult {
  bool valid = false;
  uint16_t target_id = 0;
  uint32_t request_id = 0;
  RoiOrigin roi{0, 0};
};
class PicodetCAttention {
 public:
  PicodetCAttention();
  CAttentionResult Update(uint32_t sequence, const std::vector<PersonBox>& boxes);
 private:
  struct Track { uint16_t id; float x, y; };
  uint16_t next_id_ = 1;
  std::vector<Track> previous_;
  EE_CConfig config_;
  EE_AttentionScheduler scheduler_;
  EE_ROISmoother smoother_;
  EE_FoveaTransactionBuilder transactions_;
  void Reset();
};
}