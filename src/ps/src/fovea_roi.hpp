#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace eagleeye {
struct PersonBox { float confidence, x, y, w, h; };
struct RoiOrigin { uint16_t x, y; };

struct AttentionSelection {
  bool valid = false;
  bool switched = false;
  uint32_t target_id = 0;  // Local, provisional ID; PicoDet does not supply IDs.
  uint32_t score = 0;
  uint8_t zone = 0, motion = 0, approach = 0, small = 0, uncertainty = 0;
  unsigned accepted_players = 0;
  RoiOrigin roi{160, 90};
};

struct AttentionConfig {
  // Image-space proxy for a key region. Calibrate for the actual camera/field.
  float zone_left = 160.f, zone_top = 90.f;
  float zone_right = 480.f, zone_bottom = 270.f;
  unsigned min_hold_inferences = 3;
  uint32_t switch_margin = 367;  // 12% of the maximum integer score (3060).
};

class AttentionPolicy {
 public:
  explicit AttentionPolicy(AttentionConfig config = {}) : config_(config) {}
  RoiOrigin Current() const { return current_; }

  AttentionSelection Update(const std::vector<PersonBox>& detections) {
    std::vector<PersonBox> people;
    people.reserve(detections.size());
    for (const auto& p : detections) {
      if (!std::isfinite(p.confidence) || !std::isfinite(p.x) || !std::isfinite(p.y) ||
          !std::isfinite(p.w) || !std::isfinite(p.h) || p.confidence < 0.50f ||
          p.x < 0.f || p.y < 0.f || p.x >= 640.f || p.y >= 360.f ||
          p.w < 10.f || p.h < 20.f || p.w > 240.f || p.h > 270.f ||
          p.h < p.w * 0.9f) continue;
      people.push_back(p);
    }
    std::stable_sort(people.begin(), people.end(), [](const PersonBox& a, const PersonBox& b) {
      return a.confidence > b.confidence;
    });
    std::vector<PersonBox> unique;
    unique.reserve(std::min<size_t>(people.size(), 16));
    for (const auto& p : people) {
      bool duplicate = false;
      for (const auto& kept : unique)
        if (IoU(p, kept) > 0.55f) { duplicate = true; break; }
      if (!duplicate) unique.push_back(p);
      if (unique.size() == 16) break;
    }

    AttentionSelection result;
    result.accepted_players = static_cast<unsigned>(unique.size());
    if (unique.empty()) {
      previous_.clear();
      return result;  // Keep the last safe ROI when detection is empty.
    }

    std::vector<Candidate> candidates;
    candidates.reserve(unique.size());
    std::vector<bool> used(previous_.size(), false);
    for (const auto& box : unique) {
      int match = -1;
      float nearest = 80.f;
      for (size_t i = 0; i < previous_.size(); ++i) {
        if (used[i]) continue;
        const float distance = CenterDistance(box, previous_[i].box);
        if (distance < nearest) { nearest = distance; match = static_cast<int>(i); }
      }
      Candidate candidate;
      candidate.box = box;
      candidate.id = match < 0 ? next_id_++ : previous_[match].id;
      candidate.zone = Zone(box);
      candidate.small = Small(box);
      candidate.uncertainty = ClampByte(255.f * (1.f - box.confidence));
      if (match >= 0) {
        used[match] = true;
        const auto& old = previous_[match].box;
        const float dx = CenterX(box) - CenterX(old);
        const float dy = CenterY(box) - CenterY(old);
        candidate.motion = ClampByte((std::fabs(dx) + std::fabs(dy)) * 255.f / 80.f);
        const float before = std::hypot(CenterX(old) - 320.f, CenterY(old) - 180.f);
        const float after = std::hypot(CenterX(box) - 320.f, CenterY(box) - 180.f);
        candidate.approach = ClampByte((before - after) * 255.f / 80.f);
      }
      // C-branch integer attention weights from commit e0b2e11.
      candidate.score = 3u * candidate.zone + candidate.motion +
                        3u * candidate.approach + 2u * candidate.small +
                        3u * candidate.uncertainty;
      candidates.push_back(candidate);
    }

    size_t best = 0;
    int current_index = -1;
    for (size_t i = 0; i < candidates.size(); ++i) {
      if (candidates[i].score > candidates[best].score) best = i;
      if (active_ && candidates[i].id == selected_id_) current_index = static_cast<int>(i);
    }
    size_t chosen = best;
    if (current_index >= 0) {
      chosen = static_cast<size_t>(current_index);
      if (hold_ < config_.min_hold_inferences) ++hold_;
      else if (best != chosen &&
               candidates[best].score > candidates[chosen].score + config_.switch_margin)
        chosen = best;
    }
    if (!active_ || candidates[chosen].id != selected_id_) {
      active_ = true;
      selected_id_ = candidates[chosen].id;
      hold_ = 0;
      result.switched = true;
    }

    const auto& selected = candidates[chosen];
    current_ = {ClampOrigin(CenterX(selected.box) - 160.f, 320),
                ClampOrigin(CenterY(selected.box) - 90.f, 180)};
    result.valid = true;
    result.target_id = selected.id;
    result.score = selected.score;
    result.zone = selected.zone;
    result.motion = selected.motion;
    result.approach = selected.approach;
    result.small = selected.small;
    result.uncertainty = selected.uncertainty;
    result.roi = current_;
    previous_ = candidates;
    return result;
  }

 private:
  struct Candidate {
    uint32_t id = 0, score = 0;
    uint8_t zone = 0, motion = 0, approach = 0, small = 0, uncertainty = 0;
    PersonBox box{};
  };
  static float CenterX(const PersonBox& p) { return p.x + p.w * 0.5f; }
  static float CenterY(const PersonBox& p) { return p.y + p.h * 0.5f; }
  static float CenterDistance(const PersonBox& a, const PersonBox& b) {
    return std::hypot(CenterX(a) - CenterX(b), CenterY(a) - CenterY(b));
  }
  static uint8_t ClampByte(float value) {
    return static_cast<uint8_t>(std::max(0.f, std::min(255.f, std::round(value))));
  }
  static uint16_t ClampOrigin(float value, int limit) {
    return static_cast<uint16_t>(std::max(0.f, std::min(static_cast<float>(limit), std::round(value))));
  }
  static float IoU(const PersonBox& a, const PersonBox& b) {
    const float left = std::max(a.x, b.x), top = std::max(a.y, b.y);
    const float right = std::min(a.x + a.w, b.x + b.w);
    const float bottom = std::min(a.y + a.h, b.y + b.h);
    const float intersection = std::max(0.f, right - left) * std::max(0.f, bottom - top);
    const float sum = a.w * a.h + b.w * b.h - intersection;
    return sum > 0.f ? intersection / sum : 0.f;
  }
  uint8_t Zone(const PersonBox& p) const {
    const float x = CenterX(p), y = CenterY(p);
    return x >= config_.zone_left && x <= config_.zone_right &&
           y >= config_.zone_top && y <= config_.zone_bottom ? 200u : 90u;
  }
  static uint8_t Small(const PersonBox& p) {
    const float area = p.w * p.h;
    if (area >= 12000.f) return 0;
    if (area <= 1000.f) return 255;
    return ClampByte((12000.f - area) * 255.f / 11000.f);
  }

  AttentionConfig config_;
  std::vector<Candidate> previous_;
  uint32_t next_id_ = 1, selected_id_ = 0;
  unsigned hold_ = 0;
  bool active_ = false;
  RoiOrigin current_{160, 90};
};

class RoiTrajectory {
 public:
  RoiOrigin Current() const { return current_; }
  RoiOrigin Target() const { return target_; }
  void SetTarget(RoiOrigin target) { target_ = target; }
  void SetCurrent(RoiOrigin current) { current_ = current; }
  bool Advance() {
    const RoiOrigin next{Step(current_.x, target_.x), Step(current_.y, target_.y)};
    if (next.x == current_.x && next.y == current_.y) return false;
    current_ = next;
    return true;
  }

 private:
  static uint16_t Step(uint16_t current, uint16_t target) {
    const int delta = static_cast<int>(target) - current;
    return static_cast<uint16_t>(static_cast<int>(current) +
        std::max(-12, std::min(12, delta)));
  }
  RoiOrigin current_{160, 90};
  RoiOrigin target_{160, 90};
};
}  // namespace eagleeye
