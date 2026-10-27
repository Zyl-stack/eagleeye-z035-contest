#pragma once

#include "fovea_roi.hpp"
#include <cstdint>
#include <string>

namespace eagleeye {
class FoveaControl {
 public:
  explicit FoveaControl(const std::string& devmem = "/dev/mem");
  ~FoveaControl();
  FoveaControl(const FoveaControl&) = delete;
  FoveaControl& operator=(const FoveaControl&) = delete;
  uint32_t Status() const;
  bool Paused() const;
  RoiOrigin ActiveOrigin() const;
  void Commit(RoiOrigin origin, uint32_t request_id, uint32_t mode = 2);
  void CommitManual(RoiOrigin origin, uint32_t request_id, uint32_t mode = 2);

 private:
  void CommitImpl(RoiOrigin origin, uint32_t request_id, uint32_t mode, bool manual);
  uint32_t Read(uint32_t offset) const;
  void Write(uint32_t offset, uint32_t value) const;
  void* base_ = nullptr;
  int fd_ = -1;
};
}  // namespace eagleeye
