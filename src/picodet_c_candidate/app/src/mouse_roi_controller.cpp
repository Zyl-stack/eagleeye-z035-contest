#include "mouse_roi_controller.hpp"
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/input.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace eagleeye {
namespace {
constexpr int kBitsPerWord = static_cast<int>(sizeof(unsigned long) * 8);

bool HasBit(const unsigned long* bits, int bit) {
  return (bits[bit / kBitsPerWord] & (1UL << (bit % kBitsPerWord))) != 0;
}

uint16_t ClampCoordinate(int value, int maximum) {
  return static_cast<uint16_t>(std::max(0, std::min(maximum, value)));
}
}  // namespace

MouseRoiController::MouseRoiController(const std::string& devmem,
                                       const std::string& device,
                                       int sensitivity)
    : control_(devmem), device_(device), sensitivity_(sensitivity) {
  worker_ = std::thread(&MouseRoiController::Run, this);
}

MouseRoiController::~MouseRoiController() {
  stop_.store(true);
  if (worker_.joinable()) worker_.join();
  if (mouse_fd_ >= 0) close(mouse_fd_);
}

bool MouseRoiController::IsRelativeMouse(int fd) {
  unsigned long event_bits[(EV_MAX + kBitsPerWord) / kBitsPerWord] = {};
  unsigned long relative_bits[(REL_MAX + kBitsPerWord) / kBitsPerWord] = {};
  if (ioctl(fd, EVIOCGBIT(0, sizeof(event_bits)), event_bits) < 0) return false;
  if (!HasBit(event_bits, EV_REL)) return false;
  if (ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relative_bits)), relative_bits) < 0) return false;
  return HasBit(relative_bits, REL_X) && HasBit(relative_bits, REL_Y);
}

int MouseRoiController::OpenMouse() const {
  if (device_ != "auto") {
    const int fd = open(device_.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd >= 0 && IsRelativeMouse(fd)) return fd;
    if (fd >= 0) close(fd);
    return -1;
  }
  for (int index = 0; index < 32; ++index) {
    const std::string path = "/dev/input/event" + std::to_string(index);
    const int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) continue;
    if (IsRelativeMouse(fd)) {
      std::cout << "MOUSE_ROI device=" << path << '\n';
      return fd;
    }
    close(fd);
  }
  return -1;
}

void MouseRoiController::CommitCurrent() {
  try {
    control_.CommitManual(current_, ++request_id_, 2);
  } catch (const std::exception& error) {
    std::cerr << "MOUSE_ROI commit_error=" << error.what() << '\n';
  }
}

void MouseRoiController::Run() {
  while (!stop_.load()) {
    const bool paused = control_.Paused();
    if (paused && !was_paused_) {
      current_ = control_.ActiveOrigin();
      pause_origin_ = current_;
      std::cout << "MOUSE_ROI paused origin=" << current_.x << ',' << current_.y
                << " sensitivity=" << sensitivity_ << '\n';
    } else if (!paused && was_paused_) {
      std::cout << "MOUSE_ROI automatic_tracking_resumed\n";
    }
    was_paused_ = paused;

    if (mouse_fd_ < 0) {
      mouse_fd_ = OpenMouse();
      if (mouse_fd_ < 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        continue;
      }
    }

    pollfd descriptor{mouse_fd_, POLLIN, 0};
    const int poll_result = poll(&descriptor, 1, 100);
    if (poll_result < 0 && errno == EINTR) continue;
    if (poll_result < 0 || (descriptor.revents & (POLLERR | POLLHUP | POLLNVAL))) {
      close(mouse_fd_);
      mouse_fd_ = -1;
      std::cout << "MOUSE_ROI disconnected; waiting for USB mouse\n";
      continue;
    }
    if (poll_result == 0 || !(descriptor.revents & POLLIN)) continue;

    input_event events[32];
    const ssize_t bytes = read(mouse_fd_, events, sizeof(events));
    if (bytes < 0 && (errno == EAGAIN || errno == EINTR)) continue;
    if (bytes <= 0) {
      close(mouse_fd_);
      mouse_fd_ = -1;
      continue;
    }

    int dx = 0;
    int dy = 0;
    bool restore_pause_origin = false;
    bool center = false;
    for (size_t i = 0; i < static_cast<size_t>(bytes) / sizeof(input_event); ++i) {
      const input_event& event = events[i];
      if (event.type == EV_REL && event.code == REL_X) dx += event.value;
      if (event.type == EV_REL && event.code == REL_Y) dy += event.value;
      if (event.type == EV_REL && event.code == REL_WHEEL && event.value != 0)
        sensitivity_ = std::max(1, std::min(8, sensitivity_ + (event.value > 0 ? 1 : -1)));
      if (event.type == EV_KEY && event.value == 1 && event.code == BTN_RIGHT)
        restore_pause_origin = true;
      if (event.type == EV_KEY && event.value == 1 && event.code == BTN_MIDDLE)
        center = true;
    }
    if (!paused) continue;

    bool changed = false;
    if (restore_pause_origin) {
      current_ = pause_origin_;
      changed = true;
    } else if (center) {
      current_ = {160, 90};
      changed = true;
    } else if (dx || dy) {
      current_.x = ClampCoordinate(static_cast<int>(current_.x) + dx * sensitivity_, 320);
      current_.y = ClampCoordinate(static_cast<int>(current_.y) + dy * sensitivity_, 180);
      changed = true;
    }
    if (changed) CommitCurrent();
  }
}
}  // namespace eagleeye
