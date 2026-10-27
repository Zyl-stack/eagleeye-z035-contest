#pragma once

#include <stdint.h>

namespace eagleeye {

// Software ownership for the three physical frame stores. The caller only
// publishes a frame after hardware has acknowledged a different write store.
class VdmaFrameScheduler {
 public:
  static constexpr int kSlots = 3;

  int writing() const { return writing_; }
  int ready() const { return ready_; }
  int pinned() const { return pinned_; }
  int pending_next() const { return pending_next_; }
  uint64_t sequence() const { return sequence_; }

  int ChooseNext(int display_current, int display_requested) const {
    if (display_current < 0 || display_current >= kSlots ||
        display_requested < 0 || display_requested >= kSlots) {
      return -1;
    }
    for (int step = 1; step < kSlots; ++step) {
      const int candidate = (writing_ + step) % kSlots;
      if (candidate != display_current && candidate != display_requested &&
          candidate != pinned_) {
        return candidate;
      }
    }
    return -1;
  }

  bool BeginHandoff(int next) {
    if (pending_next_ >= 0 || next < 0 || next >= kSlots ||
        next == writing_ || next == pinned_) {
      return false;
    }
    pending_next_ = next;
    return true;
  }

  bool PublishAfterWriteMoved(int completed, int observed_write) {
    if (completed != writing_ || pending_next_ < 0 ||
        observed_write != pending_next_ || observed_write == pinned_) {
      return false;
    }
    writing_ = pending_next_;
    ready_ = completed;
    ++sequence_;
    pending_next_ = -1;
    return true;
  }

  int PinLatest(uint64_t after_sequence) {
    if (sequence_ <= after_sequence || ready_ < 0 ||
        ready_ == writing_ || pinned_ >= 0) {
      return -1;
    }
    pinned_ = ready_;
    return pinned_;
  }

  void Unpin() { pinned_ = -1; }

 private:
  int writing_ = 0;
  int ready_ = -1;
  int pinned_ = -1;
  int pending_next_ = -1;
  uint64_t sequence_ = 0;
};

}  // namespace eagleeye
