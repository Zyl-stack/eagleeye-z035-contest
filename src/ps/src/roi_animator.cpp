#include "roi_animator.hpp"
#include <chrono>
#include <stdexcept>

namespace eagleeye {
RoiAnimator::RoiAnimator(const std::string& devmem) : control_(devmem) {
  control_.Commit(trajectory_.Current(), 0);
  worker_ = std::thread(&RoiAnimator::Run, this);
}

RoiAnimator::~RoiAnimator() {
  stop_.store(true);
  if (worker_.joinable()) worker_.join();
}

void RoiAnimator::SetTarget(RoiOrigin target) {
  if (target.x > 320 || target.y > 180)
    throw std::runtime_error("ROI target outside 640x360 frame");
  std::lock_guard<std::mutex> lock(mutex_);
  if (error_) std::rethrow_exception(error_);
  trajectory_.SetTarget(target);
}

void RoiAnimator::Check() const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (error_) std::rethrow_exception(error_);
}

void RoiAnimator::Run() {
  uint32_t request_id = 0;
  try {
    while (!stop_.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      if (stop_.load()) break;
      if (control_.Paused()) continue;
      RoiOrigin next;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        RoiTrajectory proposed = trajectory_;
        if (!proposed.Advance()) continue;
        next = proposed.Current();
      }
      control_.Commit(next, ++request_id);
      {
        std::lock_guard<std::mutex> lock(mutex_);
        trajectory_.SetCurrent(next);
      }
    }
  } catch (...) {
    std::lock_guard<std::mutex> lock(mutex_);
    error_ = std::current_exception();
  }
}
}  // namespace eagleeye
