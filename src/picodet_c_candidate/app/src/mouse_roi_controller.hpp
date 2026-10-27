#pragma once

#include "fovea_control.hpp"
#include <atomic>
#include <string>
#include <thread>

namespace eagleeye {
class MouseRoiController {
 public:
  MouseRoiController(const std::string& devmem, const std::string& device,
                     int sensitivity);
  ~MouseRoiController();
  MouseRoiController(const MouseRoiController&) = delete;
  MouseRoiController& operator=(const MouseRoiController&) = delete;

 private:
  void Run();
  int OpenMouse() const;
  static bool IsRelativeMouse(int fd);
  void CommitCurrent();

  FoveaControl control_;
  std::string device_;
  int sensitivity_;
  int mouse_fd_ = -1;
  bool was_paused_ = false;
  RoiOrigin current_{160, 90};
  RoiOrigin pause_origin_{160, 90};
  uint32_t request_id_ = 0x80000000U;
  std::atomic<bool> stop_{false};
  std::thread worker_;
};
}  // namespace eagleeye
