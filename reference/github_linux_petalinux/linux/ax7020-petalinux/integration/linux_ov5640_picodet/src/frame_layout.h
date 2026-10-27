#pragma once

#include <stdint.h>

namespace eagleeye {

constexpr uint32_t kFrameWidth = 640;
constexpr uint32_t kFrameHeight = 360;
constexpr uint32_t kBytesPerPixel = 3;
constexpr uint32_t kFrameStride = kFrameWidth * kBytesPerPixel;
constexpr uint32_t kActiveFrameBytes = kFrameStride * kFrameHeight;

constexpr uint32_t kFrameStoreCount = 3;
constexpr uintptr_t kFrameBufferBase = 0x1e000000U;
constexpr uint32_t kFrameSlotBytes = 0x000b0000U;
constexpr uint32_t kFrameBufferBytes = kFrameStoreCount * kFrameSlotBytes;

constexpr uintptr_t kS2mmVdmaBase = 0x43010000U;
constexpr uintptr_t kMm2sVdmaBase = 0x43000000U;
constexpr uintptr_t kCmosResetGpioBase = 0x41210000U;
constexpr uintptr_t kVtcBase = 0x43c00000U;
constexpr uintptr_t kDynamicClockBase = 0x43c10000U;
constexpr uintptr_t kFoveaAxiBase = 0x40000000U;

static_assert(kActiveFrameBytes <= kFrameSlotBytes,
              "VDMA frame does not fit in the reserved slot");

inline constexpr uintptr_t FrameAddress(uint32_t index) {
  return kFrameBufferBase + (index % kFrameStoreCount) * kFrameSlotBytes;
}

}  // namespace eagleeye
