#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace eagleeye {
struct PersonBox { float confidence, x, y, w, h; };
struct RoiOrigin { uint16_t x, y; };
struct GridSelection {
  bool valid = false;
  bool switched = false;
  uint8_t row = 1, col = 1;
  unsigned players = 0, accepted_players = 0;
  RoiOrigin roi{160, 90};
};

class GridPolicy {
 public:
  RoiOrigin Current() const { return {static_cast<uint16_t>(col_ * 160), static_cast<uint16_t>(row_ * 90)}; }

  GridSelection Update(const std::vector<PersonBox>& detections) {
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

    unsigned counts[3][3] = {};
    for (const auto& p : unique) {
      const unsigned col = Cell(p.x + p.w * 0.5f, 640.f);
      const unsigned row = Cell(p.y + p.h * 0.5f, 360.f);
      ++counts[row][col];
    }
    unsigned best_count = 0, best_row = 0, best_col = 0;
    for (unsigned row = 0; row < 3; ++row)
      for (unsigned col = 0; col < 3; ++col)
        if (counts[row][col] > best_count) {
          best_count = counts[row][col]; best_row = row; best_col = col;
        }

    GridSelection result;
    result.accepted_players = static_cast<unsigned>(unique.size());
    if (!best_count) return result;
    if (!active_) {
      active_ = true; row_ = best_row; col_ = best_col; hold_ = 0;
      result.switched = true;
    } else {
      const unsigned current_count = counts[row_][col_];
      if (hold_ < kMinHoldInferences) ++hold_;
      else if ((best_row != row_ || best_col != col_) &&
               (current_count == 0 || best_count >= current_count + 2)) {
        row_ = best_row; col_ = best_col; hold_ = 0;
        result.switched = true;
      }
    }
    // The grid chooses a destination; the display loop moves toward it separately.
    result.valid = true;
    result.row = static_cast<uint8_t>(row_);
    result.col = static_cast<uint8_t>(col_);
    result.players = counts[row_][col_];
    result.roi = Current();
    return result;
  }

 private:
  static unsigned Cell(float center, float size) {
    return static_cast<unsigned>(std::min(2.f, std::max(0.f, std::floor(center * 3.f / size))));
  }
  static float IoU(const PersonBox& a, const PersonBox& b) {
    const float left = std::max(a.x, b.x), top = std::max(a.y, b.y);
    const float right = std::min(a.x + a.w, b.x + b.w);
    const float bottom = std::min(a.y + a.h, b.y + b.h);
    const float intersection = std::max(0.f, right-left) * std::max(0.f, bottom-top);
    const float sum = a.w*a.h + b.w*b.h - intersection;
    return sum > 0.f ? intersection / sum : 0.f;
  }
  static constexpr unsigned kMinHoldInferences = 3;
  bool active_ = false;
  unsigned row_ = 1, col_ = 1, hold_ = 0;
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
