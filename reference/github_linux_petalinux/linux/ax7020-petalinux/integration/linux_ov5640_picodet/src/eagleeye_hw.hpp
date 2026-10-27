#pragma once

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

namespace eagleeye {

struct HardwareOptions {
  std::string devmem = "/dev/mem";
  std::string i2c = "/dev/i2c-0";
  bool initialize_sensor = true;
  bool initialize_display = true;
};

struct PipelineStatus {
  uint32_t s2mm_control = 0;
  uint32_t s2mm_status = 0;
  uint32_t mm2s_control = 0;
  uint32_t mm2s_status = 0;
  uint32_t park_pointer = 0;
  uint32_t current_write_store = 0;
  uint32_t current_read_store = 0;
  uint32_t s2mm_vsize = 0;
  uint32_t s2mm_hsize = 0;
  uint32_t s2mm_stride = 0;
  uint32_t start_address[3] = {};
  uint32_t vdma_version = 0;
  uint64_t completed_frames = 0;
  int latest_completed_store = -1;
  uint64_t timeout_count = 0;
  uint64_t error_count = 0;
};

class CameraPipeline {
 public:
  explicit CameraPipeline(const HardwareOptions& options);
  ~CameraPipeline();

  CameraPipeline(const CameraPipeline&) = delete;
  CameraPipeline& operator=(const CameraPipeline&) = delete;

  void Initialize();
  void CaptureLatest(std::vector<uint8_t>* snapshot, bool bgr_to_rgb,
                     uint64_t* sequence = nullptr);
  PipelineStatus GetStatus() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace eagleeye
