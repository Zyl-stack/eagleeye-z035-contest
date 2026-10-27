#include "picodet_c_attention.hpp"
#include "ball_challenger_gate.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace eagleeye {
PicodetCAttention::PicodetCAttention() : config_(ee_c_config_default()) {
  ee_fovea_transaction_init(&transactions_);
  Reset();
}
void PicodetCAttention::Reset() {
  previous_.clear();
  ee_scheduler_init(&scheduler_, config_.scheduler);
  ee_roi_smoother_init(&smoother_, config_.roi_smoother);
}
CAttentionResult PicodetCAttention::Update(uint32_t sequence, const std::vector<PersonBox>& boxes) {
  CAttentionResult result;
  if (boxes.size() > EE_MAX_TARGETS) { Reset(); return result; }
  EE_BParsedFrame parsed{};
  parsed.frame_id = sequence;
  parsed.source_width = 640;
  parsed.source_height = 360;
  std::vector<Track> current;
  std::vector<bool> used(previous_.size(), false);
  for (const auto& box : boxes) {
    if (!std::isfinite(box.confidence) || !std::isfinite(box.x) || !std::isfinite(box.y) ||
        !std::isfinite(box.w) || !std::isfinite(box.h) || box.confidence < 0 || box.confidence > 1 ||
        box.x < 0 || box.y < 0 || box.w <= 0 || box.h <= 0 || box.x + box.w > 640.001f || box.y + box.h > 360.001f) {
      Reset(); return result;
    }
    const float x = box.x + box.w / 2, y = box.y + box.h / 2;
    int match = -1; float best = 80.f;
    for (size_t i = 0; i < previous_.size(); ++i) {
      const float dist = std::hypot(x - previous_[i].x, y - previous_[i].y);
      if (!used[i] && dist < best) { match = static_cast<int>(i); best = dist; }
    }
    uint16_t id;
    if (match >= 0) { id = previous_[match].id; used[match] = true; }
    else {
      // IDs are provisional and never reused during this process lifetime.
      if (next_id_ == EE_INVALID_TARGET_ID) { Reset(); return result; }
      id = next_id_++;
    }
    current.push_back({id, x, y});
    auto& t = parsed.targets[parsed.count++];
    const auto x1 = static_cast<uint16_t>(std::floor(box.x));
    const auto y1 = static_cast<uint16_t>(std::floor(box.y));
    const auto x2 = static_cast<uint16_t>(std::min(640.f, std::ceil(box.x + box.w)));
    const auto y2 = static_cast<uint16_t>(std::min(360.f, std::ceil(box.y + box.h)));
    if (x2 <= x1 || y2 <= y1) { Reset(); return result; }
    t.frame_id = sequence; t.target_id = id; t.target_type = EE_SOCCER_TARGET_PLAYER;
    t.x = x1; t.y = y1; t.w = x2 - x1; t.h = y2 - y1;
    t.confidence = static_cast<uint8_t>(std::lround(box.confidence * 100)); t.valid = 1;
    // No fabricated camera timestamp or velocity: inference freshness is unverified.
  }
  if (current.empty()) { Reset(); return result; }
  EE_GlobalTargetList list{}, candidates{};
  if (!ee_bc_adapt_frame(&parsed, &list)) { Reset(); return result; }
  ee_attention_update_scores_for_scheduler_step(list.targets, list.count, &config_.attention, scheduler_.wait_frames);
  EE_FoveaRequest request{}, smoothed{};
  if (!ee_ball_challenger_gate_apply(&config_, &scheduler_, &list, &candidates) ||
      !ee_scheduler_step(&scheduler_, &candidates, &request) ||
      !ee_roi_smoother_step(&smoother_, &request, &smoothed) ||
      ee_fovea_transaction_build(&transactions_, &smoothed, sequence, &smoothed) == EE_FOVEA_TRANSACTION_INVALID) {
    Reset(); return result;
  }
  previous_ = current;
  result.valid = true; result.target_id = smoothed.target_id; result.request_id = smoothed.request_id;
  result.roi = {smoothed.roi_x, smoothed.roi_y};
  return result;
}
}