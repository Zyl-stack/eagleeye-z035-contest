#pragma once

#include "fovea_control.hpp"
#include <atomic>
#include <exception>
#include <mutex>
#include <string>
#include <thread>

namespace eagleeye {
class RoiAnimator {
 public:
  explicit RoiAnimator(const std::string& devmem);
  ~RoiAnimator();
  RoiAnimator(const RoiAnimator&) = delete;
  RoiAnimator& operator=(const RoiAnimator&) = delete;
  void SetTarget(RoiOrigin target);
  void Hold();
  void Check() const;

 private:
  void Run();
  FoveaControl control_;
  mutable std::mutex mutex_;
  RoiTrajectory trajectory_;
  std::exception_ptr error_;
  std::atomic<bool> stop_{false};
  std::thread worker_;
};
}  // namespace eagleeye
