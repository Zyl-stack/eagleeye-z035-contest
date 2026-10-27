#pragma once

#include "frame_layout.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <thread>
#include <vector>

namespace eagleeye {
struct Signature {
  uint32_t hash = 2166136261U;
  bool nonzero = false;
};

inline Signature SampleFrame(const volatile uint8_t* bytes) {
  Signature s;
  // Coprime to the line stride: cover the full image rather than its edges.
  for (size_t i = 0; i < kActiveFrameBytes; i += 31) {
    const uint8_t b = bytes[i];
    s.hash = (s.hash ^ b) * 16777619U;
    s.nonzero = s.nonzero || b != 0;
  }
  return s;
}

inline uint32_t ContentHash(const std::vector<uint8_t>& bytes) {
  uint32_t h = 2166136261U;
  for (uint8_t b : bytes) h = (h ^ b) * 16777619U;
  return h;
}

class ReadyTracker {
 public:
  bool Observe(const std::array<Signature, kFrameStoreCount>& now) {
    bool change = false;
    for (size_t i = 0; i < now.size(); ++i) {
      if (seen_ && now[i].nonzero && last_[i].nonzero &&
          now[i].hash != last_[i].hash) change = true;
    }
    changes_ += change ? 1 : 0;
    last_ = now;
    seen_ = true;
    return changes_ >= 2;
  }
 private:
  std::array<Signature, kFrameStoreCount> last_{};
  bool seen_ = false;
  unsigned changes_ = 0;
};

struct SnapshotInfo {
  uint32_t slot = 0;
  unsigned attempts = 0;
  uint32_t hash = 0;
};

template <class Reader, class Cancelled>
SnapshotInfo CopyStableFrame(Reader read, Cancelled cancelled,
                            uint32_t start_slot, std::vector<uint8_t>* output,
                            std::vector<uint8_t>* verify, bool sleep_retry = true) {
  output->resize(kActiveFrameBytes);
  verify->resize(kActiveFrameBytes);
  for (unsigned attempt = 0; attempt < 12; ++attempt) {
    if (cancelled()) throw std::runtime_error("capture cancelled");
    const uint32_t slot = (start_slot + attempt) % kFrameStoreCount;
    read(slot, output->data(), output->size());
    read(slot, verify->data(), verify->size());
    if (*output == *verify &&
        std::any_of(output->begin(), output->end(), [](uint8_t b) { return b != 0; })) {
      return {slot, attempt + 1, ContentHash(*output)};
    }
    if (sleep_retry) std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  throw std::runtime_error("no nonzero matching snapshot: blank or changing DDR; video left running");
}

inline bool FatalDma(uint32_t control, uint32_t status) {
  return (control & 1U) == 0 || (control & 4U) != 0 ||
         (status & 1U) != 0 || (status & 0x60U) != 0;
}

inline bool CompatibleGeometry(uint32_t height, uint32_t width_bytes,
                               uint32_t stride, const std::array<uint32_t, 3>& addresses) {
  if (height != kFrameHeight || width_bytes != kFrameStride || stride != kFrameStride)
    return false;
  for (uint32_t i = 0; i < 3; ++i)
    if (addresses[i] != FrameAddress(i)) return false;
  return true;
}
}  // namespace eagleeye
