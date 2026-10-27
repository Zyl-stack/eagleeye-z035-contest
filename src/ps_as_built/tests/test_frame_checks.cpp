#include "frame_checks.hpp"
#include <cassert>
#include <iostream>

int main() {
  using namespace eagleeye;
  std::vector<uint8_t> frame(kActiveFrameBytes, 0), output, verify;
  const auto zero = SampleFrame(frame.data());
  frame[(kActiveFrameBytes / 2 / 31) * 31] = 32;
  const auto center = SampleFrame(frame.data());
  assert(!zero.nonzero && center.nonzero && center.hash != zero.hash);
  ReadyTracker tracker;
  assert(!tracker.Observe({{zero, zero, zero}}));
  assert(!tracker.Observe({{center, zero, zero}}));
  assert(!tracker.Observe({{center, zero, zero}}));
  frame[(kActiveFrameBytes / 2 / 31) * 31] = 33;
  const auto second = SampleFrame(frame.data());
  assert(!tracker.Observe({{second, zero, zero}}));
  frame[(kActiveFrameBytes / 2 / 31) * 31] = 34;
  const auto third = SampleFrame(frame.data());
  assert(tracker.Observe({{third, zero, zero}}));
  unsigned reads = 0;
  auto read = [&](uint32_t slot, uint8_t* dst, size_t size) {
    ++reads;
    std::fill(dst, dst + size, slot == 1 ? 42 : 0);
  };
  auto info = CopyStableFrame(read, [] { return false; }, 0, &output, &verify, false);
  assert(info.slot == 1 && info.attempts == 2 && reads == 4);
  assert(output.front() == 42 && output.back() == 42);
  bool failed = false;
  unsigned changing = 0;
  try {
    CopyStableFrame([&](uint32_t, uint8_t* dst, size_t n) {
      std::fill(dst, dst+n, static_cast<uint8_t>(++changing));
    }, [] { return false; }, 0, &output, &verify, false);
  } catch (const std::runtime_error&) { failed = true; }
  assert(failed && changing == 24);
  failed = false;
  try {
    CopyStableFrame([](uint32_t, uint8_t* dst, size_t n) { std::fill(dst, dst+n, 0); },
                    [] { return false; }, 0, &output, &verify, false);
  } catch (const std::runtime_error&) { failed = true; }
  assert(failed);
  failed = false;
  try { CopyStableFrame(read, [] { return true; }, 0, &output, &verify, false); }
  catch (const std::runtime_error&) { failed = true; }
  assert(failed);
  assert(!FatalDma(0x10003, 0x15810));
  assert(FatalDma(0x10003, 0x15811));
  assert(FatalDma(0x10003, 0x10020));
  assert(FatalDma(0x10003, 0x10040));
  assert(FatalDma(0x10002, 0x10000));
  assert(FatalDma(7, 0));
  assert(CompatibleGeometry(360, 1920, 1920, {{0x1e000000, 0x1e0b0000, 0x1e160000}}));
  assert(!CompatibleGeometry(720, 1920, 1920, {{0x1e000000, 0x1e0b0000, 0x1e160000}}));
  assert(!CompatibleGeometry(360, 1920, 2048, {{0x1e000000, 0x1e0b0000, 0x1e160000}}));
  assert(!CompatibleGeometry(360, 1920, 1920, {{0x1e000000, 0x1e0b0000, 0x1e170000}}));
  std::cout << "PASS frame center coverage, readiness, blank/torn/cancel rejection, slot rotation and DMA/geometry policy\n";
}
