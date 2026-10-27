#include "fovea_control.hpp"
#include "frame_layout.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace eagleeye {
namespace {
std::mutex g_fovea_commit_mutex;
}

FoveaControl::FoveaControl(const std::string& devmem) {
  fd_ = open(devmem.c_str(), O_RDWR | O_SYNC);
  if (fd_ < 0) throw std::runtime_error("cannot open /dev/mem for Fovea");
  base_ = mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, kFoveaAxiBase);
  if (base_ == MAP_FAILED) {
    base_ = nullptr;
    close(fd_);
    fd_ = -1;
    throw std::runtime_error("cannot map Fovea AXI-Lite");
  }
  // Mouse ROI hardware contract: "EEM2" at 0x64.
  if (Read(0x64) != 0x45454d32U) {
    munmap(base_, 4096);
    close(fd_);
    base_ = nullptr;
    fd_ = -1;
    throw std::runtime_error("Fovea register signature mismatch; no ROI writes performed");
  }
}

FoveaControl::~FoveaControl() {
  if (base_) munmap(base_, 4096);
  if (fd_ >= 0) close(fd_);
}

uint32_t FoveaControl::Read(uint32_t offset) const {
  __sync_synchronize();
  return *reinterpret_cast<volatile uint32_t*>(static_cast<char*>(base_) + offset);
}

void FoveaControl::Write(uint32_t offset, uint32_t value) const {
  __sync_synchronize();
  *reinterpret_cast<volatile uint32_t*>(static_cast<char*>(base_) + offset) = value;
  __sync_synchronize();
}

uint32_t FoveaControl::Status() const { return Read(0x04); }

bool FoveaControl::Paused() const { return (Status() & (1U << 3)) != 0; }

RoiOrigin FoveaControl::ActiveOrigin() const {
  const uint32_t packed = Read(0x30);
  return {static_cast<uint16_t>(packed & 0x3ffU),
          static_cast<uint16_t>((packed >> 16) & 0x1ffU)};
}

void FoveaControl::Commit(RoiOrigin origin, uint32_t request_id, uint32_t mode) {
  CommitImpl(origin, request_id, mode, false);
}

void FoveaControl::CommitManual(RoiOrigin origin, uint32_t request_id, uint32_t mode) {
  CommitImpl(origin, request_id, mode, true);
}

void FoveaControl::CommitImpl(RoiOrigin origin, uint32_t request_id, uint32_t mode,
                              bool manual) {
  std::lock_guard<std::mutex> lock(g_fovea_commit_mutex);
  if (origin.x > 320 || origin.y > 180 || mode > 2)
    throw std::runtime_error("ROI origin is outside the 640x360 frame");
  if (Status() & 1U)
    throw std::runtime_error("previous Fovea request is still pending");
  Write(0x00, 1U);               // enable, preserve other hardware state
  Write(0x08, request_id);
  Write(0x0c, 0U);               // unknown camera frame id, never fabricate it
  Write(0x10, 0U);               // no tracker id in the current PicoDet output
  Write(0x14, (uint32_t(origin.y) << 16) | origin.x);
  Write(0x18, (180U << 16) | 320U);
  Write(0x1c, mode);             // 0 direct, 1 bilinear, 2 Lanczos
  Write(0x20, 0U);
  Write(0x24, 0U);
  Write(0x00, 3U | (manual ? (1U << 5) : 0U));
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
  while (std::chrono::steady_clock::now() < deadline) {
    const uint32_t status = Status();
    if (status & 4U) throw std::runtime_error("Fovea rejected ROI request");
    if ((status & 2U) && !(status & 1U)) {
      std::cout << (manual ? "FOVEA_MOUSE" : "FOVEA")
                << " request=" << request_id << " roi=" << origin.x << ',' << origin.y
                << ",320,180 requested_mode=" << mode << " display_mode="
                << ((status >> 4) & 3U) << " status=0x" << std::hex << status
                << " quality=0x" << Read(0x2c) << std::dec << '\n';
      return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  throw std::runtime_error("Fovea mode/ROI request was not accepted within 500 ms");
}
}  // namespace eagleeye
