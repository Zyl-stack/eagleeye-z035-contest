#include "eagleeye_hw.hpp"

#include "frame_layout.h"
#include "vdma_frame_scheduler.hpp"

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <chrono>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <thread>

#ifndef I2C_SLAVE
#define I2C_SLAVE 0x0703
#endif

// Reuse the exact register table from the delivered, verified bare-metal
// application. EAGLEEYE_LINUX_TABLE_ONLY excludes all Xilinx BSP functions.
#define EAGLEEYE_LINUX_TABLE_ONLY
#include "../reference_baremetal/ov5640.c"
#undef EAGLEEYE_LINUX_TABLE_ONLY

namespace eagleeye {
namespace {

constexpr size_t kMmioSpan = 0x1000;
constexpr uint32_t kVdmaControlRun = 0x00000001;
constexpr uint32_t kVdmaControlReset = 0x00000004;
constexpr uint32_t kVdmaControlFrameIrq = 0x00001000;
constexpr uint32_t kVdmaControlErrorIrq = 0x00004000;
constexpr uint32_t kVdmaControlOneFrame = 0x00010000;
constexpr uint32_t kVdmaFrameIrq = 0x00001000;
constexpr uint32_t kVdmaStatusErrors = 0x0000c9f0;
constexpr uint32_t kVdmaW1c = 0x0000d990;
constexpr uint32_t kVdmaParkPointer = 0x28;
constexpr uint32_t kVdmaVersion = 0x2c;

constexpr uint32_t kMm2sControl = 0x00;
constexpr uint32_t kMm2sStatus = 0x04;
constexpr uint32_t kMm2sVsize = 0x50;
constexpr uint32_t kMm2sHsize = 0x54;
constexpr uint32_t kMm2sStride = 0x58;
constexpr uint32_t kMm2sStart0 = 0x5c;

constexpr uint32_t kS2mmControl = 0x30;
constexpr uint32_t kS2mmStatus = 0x34;
constexpr uint32_t kS2mmVsize = 0xa0;
constexpr uint32_t kS2mmHsize = 0xa4;
constexpr uint32_t kS2mmStride = 0xa8;
constexpr uint32_t kS2mmStart0 = 0xac;
constexpr uint32_t kS2mmObservedHsize = 0xf0;
constexpr uint32_t kS2mmObservedVsize = 0xf4;

std::runtime_error Error(const std::string& what) {
  return std::runtime_error(what + ": " + std::strerror(errno));
}

class Mapping {
 public:
  Mapping() = default;
  Mapping(int fd, uintptr_t physical, size_t length) { Map(fd, physical, length); }
  ~Mapping() {
    if (mapping_ != MAP_FAILED) munmap(mapping_, mapping_length_);
  }

  Mapping(const Mapping&) = delete;
  Mapping& operator=(const Mapping&) = delete;

  void Map(int fd, uintptr_t physical, size_t length) {
    const long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) throw Error("sysconf(_SC_PAGESIZE)");
    const uintptr_t mask = static_cast<uintptr_t>(page_size - 1);
    const uintptr_t page = physical & ~mask;
    offset_ = physical - page;
    mapping_length_ = (offset_ + length + mask) & ~mask;
    mapping_ = mmap(nullptr, mapping_length_, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, static_cast<off_t>(page));
    if (mapping_ == MAP_FAILED) throw Error("mmap physical memory");
    data_ = static_cast<uint8_t*>(mapping_) + offset_;
  }

  volatile uint32_t* Reg(uint32_t offset) const {
    return reinterpret_cast<volatile uint32_t*>(data_ + offset);
  }
  uint8_t* bytes() const { return data_; }

 private:
  void* mapping_ = MAP_FAILED;
  size_t mapping_length_ = 0;
  size_t offset_ = 0;
  uint8_t* data_ = nullptr;
};

uint32_t Read32(const Mapping& map, uint32_t offset) {
  const uint32_t value = *map.Reg(offset);
  __sync_synchronize();
  return value;
}

void Write32(const Mapping& map, uint32_t offset, uint32_t value) {
  __sync_synchronize();
  *map.Reg(offset) = value;
  __sync_synchronize();
}

void WaitBit(const Mapping& map, uint32_t offset, uint32_t mask,
             bool set, int timeout_ms, const char* description) {
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(timeout_ms);
  while (((Read32(map, offset) & mask) != 0) != set) {
    if (std::chrono::steady_clock::now() >= deadline) {
      std::ostringstream out;
      out << "timeout waiting for " << description << " (reg=0x"
          << std::hex << Read32(map, offset) << ')';
      throw std::runtime_error(out.str());
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void WriteAll(int fd, const uint8_t* data, size_t size) {
  size_t done = 0;
  while (done < size) {
    const ssize_t written = write(fd, data + done, size - done);
    if (written < 0 && errno == EINTR) continue;
    if (written <= 0) throw Error("OV5640 I2C write");
    done += static_cast<size_t>(written);
  }
}

uint8_t ReadSensorRegister(int fd, uint16_t reg) {
  uint8_t address[2] = {static_cast<uint8_t>(reg >> 8),
                        static_cast<uint8_t>(reg)};
  WriteAll(fd, address, sizeof(address));
  uint8_t value = 0;
  ssize_t result;
  do {
    result = read(fd, &value, 1);
  } while (result < 0 && errno == EINTR);
  if (result != 1) throw Error("OV5640 I2C read");
  return value;
}

void WriteSensorRegister(int fd, uint16_t reg, uint8_t value) {
  const uint8_t message[3] = {static_cast<uint8_t>(reg >> 8),
                              static_cast<uint8_t>(reg), value};
  WriteAll(fd, message, sizeof(message));
  if (reg == 0x3008 && value == 0x82) usleep(5000);
}

void ResetVdmaChannel(const Mapping& vdma, uint32_t control) {
  Write32(vdma, control, Read32(vdma, control) | kVdmaControlReset);
  WaitBit(vdma, control, kVdmaControlReset, false, 100,
          "AXI VDMA reset completion");
}

void ConfigureVdmaChannel(const Mapping& vdma, bool s2mm,
                          uint32_t initial_store) {
  const uint32_t control = s2mm ? kS2mmControl : kMm2sControl;
  const uint32_t status = s2mm ? kS2mmStatus : kMm2sStatus;
  const uint32_t vsize = s2mm ? kS2mmVsize : kMm2sVsize;
  const uint32_t hsize = s2mm ? kS2mmHsize : kMm2sHsize;
  const uint32_t stride = s2mm ? kS2mmStride : kMm2sStride;
  const uint32_t start = s2mm ? kS2mmStart0 : kMm2sStart0;

  ResetVdmaChannel(vdma, control);
  // Only documented W1C status bits are acknowledged after reset.
  Write32(vdma, status, kVdmaW1c);
  for (uint32_t i = 0; i < kFrameStoreCount; ++i) {
    Write32(vdma, start + i * 4, static_cast<uint32_t>(FrameAddress(i)));
  }
  Write32(vdma, stride, kFrameStride);
  Write32(vdma, hsize, kFrameStride);
  const uint32_t park = Read32(vdma, kVdmaParkPointer);
  const uint32_t reference_mask = s2mm ? 0x1f00U : 0x1fU;
  const uint32_t reference = s2mm ? initial_store << 8 : initial_store;
  Write32(vdma, kVdmaParkPointer,
          (park & ~reference_mask) | reference);
  Write32(vdma, control, kVdmaControlRun | kVdmaControlFrameIrq |
                              kVdmaControlErrorIrq | kVdmaControlOneFrame);
  // VSIZE is the transfer trigger and must be written last.
  Write32(vdma, vsize, kFrameHeight);
}

}  // namespace

class CameraPipeline::Impl {
 public:
  explicit Impl(const HardwareOptions& options) : options_(options) {
    memory_fd_ = open(options_.devmem.c_str(), O_RDWR | O_SYNC);
    if (memory_fd_ < 0) throw Error("open " + options_.devmem);
    frame_buffer_.Map(memory_fd_, kFrameBufferBase, kFrameBufferBytes);
    s2mm_.Map(memory_fd_, kS2mmVdmaBase, kMmioSpan);
    mm2s_.Map(memory_fd_, kMm2sVdmaBase, kMmioSpan);
    gpio_.Map(memory_fd_, kCmosResetGpioBase, kMmioSpan);
    vtc_.Map(memory_fd_, kVtcBase, kMmioSpan);
    dynclk_.Map(memory_fd_, kDynamicClockBase, kMmioSpan);
  }

  ~Impl() {
    stop_.store(true);
    frame_ready_.notify_all();
    if (poll_thread_.joinable()) poll_thread_.join();
    // Stop both channels before the memory mappings and descriptors disappear.
    if (started_) {
      Write32(s2mm_, kS2mmControl,
              Read32(s2mm_, kS2mmControl) & ~kVdmaControlRun);
      Write32(mm2s_, kMm2sControl,
              Read32(mm2s_, kMm2sControl) & ~kVdmaControlRun);
    }
    if (i2c_fd_ >= 0) close(i2c_fd_);
    if (memory_fd_ >= 0) close(memory_fd_);
  }

  void ResetSensor() {
    // AXI GPIO channel 1: DATA=0x0, TRI=0x4. The board reset is active low.
    Write32(gpio_, 0x4, 0x0);
    Write32(gpio_, 0x0, 0x1);
    usleep(500000);
    Write32(gpio_, 0x0, 0x0);
    usleep(500000);
    Write32(gpio_, 0x0, 0x1);
    usleep(500000);
  }

  void InitializeSensor() {
    i2c_fd_ = open(options_.i2c.c_str(), O_RDWR);
    if (i2c_fd_ < 0) throw Error("open " + options_.i2c);
    if (ioctl(i2c_fd_, I2C_SLAVE, 0x3c) < 0) {
      throw Error("select OV5640 I2C address 0x3c");
    }
    const uint16_t chip_id =
        (static_cast<uint16_t>(ReadSensorRegister(i2c_fd_, 0x300a)) << 8) |
        ReadSensorRegister(i2c_fd_, 0x300b);
    if (chip_id != 0x5640) {
      std::ostringstream out;
      out << "unexpected OV5640 chip id 0x" << std::hex << chip_id;
      throw std::runtime_error(out.str());
    }
    for (size_t i = 0; sensor_init_data[i].reg != SEQUENCE_END; ++i) {
      WriteSensorRegister(i2c_fd_, sensor_init_data[i].reg,
                          sensor_init_data[i].val);
    }
    // The retained table came from the older 1280x720 demo. The current XSA
    // and VDMA contract are 640x360; keep the same 16:9 crop/timing and apply
    // the final scaler output size expected by the new PL pipeline.
    WriteSensorRegister(i2c_fd_, 0x3808, 0x02);
    WriteSensorRegister(i2c_fd_, 0x3809, 0x80);
    WriteSensorRegister(i2c_fd_, 0x380a, 0x01);
    WriteSensorRegister(i2c_fd_, 0x380b, 0x68);
    std::cout << "HW ov5640_id=0x" << std::hex << chip_id << std::dec
              << " i2c=" << options_.i2c << " initialized=1\n";
  }

  void ConfigureDisplay() {
    // 100 MHz reference -> 74.285714 MHz pixel clock. These values are the
    // Digilent axi_dynclk integer-MMCM solution for DIVCLK=7, FBMULT=52,
    // CLKOUT_DIVIDE=2. The core internally generates 5x pixel clock.
    Write32(dynclk_, 0x00, 0x00000000);
    WaitBit(dynclk_, 0x04, 0x1, false, 100, "dynamic clock stop");
    Write32(dynclk_, 0x08, 0x00000041);
    Write32(dynclk_, 0x0c, 0x0000069a);
    Write32(dynclk_, 0x10, 0x00000000);
    Write32(dynclk_, 0x14, 0x000020c4);
    Write32(dynclk_, 0x18, 0xcfafa401);
    Write32(dynclk_, 0x1c, 0x00a300ff);
    Write32(dynclk_, 0x00, 0x00000001);
    WaitBit(dynclk_, 0x04, 0x1, true, 500, "720p pixel clock lock");

    // Explicit 1280x720p60 VTC generator values from the supplied XSA.
    Write32(vtc_, 0x060, (720U << 16) | 1280U);
    Write32(vtc_, 0x068, 0x00000040);  // progressive
    Write32(vtc_, 0x06c, 0x0000001f);  // positive AV/blank/sync
    Write32(vtc_, 0x070, 1650U);
    Write32(vtc_, 0x074, (750U << 16) | 750U);
    Write32(vtc_, 0x078, (1430U << 16) | 1390U);
    Write32(vtc_, 0x07c, (1280U << 16) | 1280U);
    Write32(vtc_, 0x080, (729U << 16) | 724U);
    Write32(vtc_, 0x084, (1280U << 16) | 1280U);
    Write32(vtc_, 0x088, (1280U << 16) | 1280U);
    Write32(vtc_, 0x08c, (729U << 16) | 724U);
    Write32(vtc_, 0x090, (1280U << 16) | 1280U);
    Write32(vtc_, 0x000, 0x03fdef06);  // register sources + update + generator
  }

  void Initialize() {
    if (options_.initialize_sensor) {
      ResetSensor();
      InitializeSensor();
    }
    std::memset(frame_buffer_.bytes(), 0, kFrameBufferBytes);
    __sync_synchronize();
    if (options_.initialize_display) ConfigureDisplay();

    // VDMA0 displays a blank third slot until a complete capture is available.
    // VDMA1 writes slot zero. The XSA has three stores in each VDMA.
    started_ = true;
    ConfigureVdmaChannel(mm2s_, false, 2);
    ConfigureVdmaChannel(s2mm_, true, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const PipelineStatus status = GetStatus();
    if (status.s2mm_vsize != kFrameHeight ||
        status.s2mm_hsize != kFrameStride ||
        (status.s2mm_stride & 0xffffU) != kFrameStride) {
      throw std::runtime_error("S2MM configured frame layout does not match the snapshot layout: " +
                               RegisterDump());
    }
    for (uint32_t i = 0; i < kFrameStoreCount; ++i) {
      if (status.start_address[i] != FrameAddress(i)) {
        throw std::runtime_error("S2MM frame address readback mismatch: " +
                                 RegisterDump());
      }
    }
    if ((status.s2mm_status & kVdmaStatusErrors) != 0 ||
        (status.mm2s_status & kVdmaStatusErrors) != 0) {
      throw std::runtime_error("VDMA start error: " + RegisterDump());
    }
    std::cout << "HW frame=640x360 stride=1920 buffers=3 base=0x1e000000"
              << " hdmi=1280x720p60 s2mm_status=0x" << std::hex
              << status.s2mm_status << " mm2s_status=0x"
              << status.mm2s_status << std::dec << '\n';
    poll_thread_ = std::thread(&Impl::PollFrames, this);
  }

  PipelineStatus GetStatus() const {
    PipelineStatus status;
    status.s2mm_control = Read32(s2mm_, kS2mmControl);
    status.s2mm_status = Read32(s2mm_, kS2mmStatus);
    status.mm2s_control = Read32(mm2s_, kMm2sControl);
    status.mm2s_status = Read32(mm2s_, kMm2sStatus);
    status.park_pointer = Read32(s2mm_, kVdmaParkPointer);
    status.current_write_store = (status.park_pointer >> 24) & 0x1f;
    status.current_read_store =
        (Read32(mm2s_, kVdmaParkPointer) >> 16) & 0x1f;
    status.s2mm_vsize = Read32(s2mm_, kS2mmVsize);
    status.s2mm_hsize = Read32(s2mm_, kS2mmHsize);
    status.s2mm_stride = Read32(s2mm_, kS2mmStride);
    for (uint32_t i = 0; i < kFrameStoreCount; ++i) {
      status.start_address[i] = Read32(s2mm_, kS2mmStart0 + i * 4);
    }
    status.vdma_version = Read32(s2mm_, kVdmaVersion);
    std::lock_guard<std::mutex> lock(mutex_);
    status.completed_frames = scheduler_.sequence();
    status.latest_completed_store = scheduler_.ready();
    status.timeout_count = timeout_count_;
    status.error_count = error_count_;
    return status;
  }

  void CaptureLatest(std::vector<uint8_t>* snapshot, bool bgr_to_rgb,
                     uint64_t* sequence) {
    if (!snapshot) throw std::invalid_argument("null snapshot buffer");
    snapshot->resize(kActiveFrameBytes);
    double copy_ms = 0;
    int copied_slot = -1;
    uint64_t copied_sequence = 0;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (!frame_ready_.wait_for(lock, std::chrono::milliseconds(1500), [&] {
            return scheduler_.sequence() > delivered_sequence_ ||
                   poll_error_ || stop_.load();
          })) {
        ++timeout_count_;
        throw std::runtime_error("timeout waiting for a new completed VDMA frame: " +
                                 RegisterDump());
      }
      if (poll_error_) std::rethrow_exception(poll_error_);
      if (stop_.load()) throw std::runtime_error("camera capture stopped");
      const int slot = scheduler_.PinLatest(delivered_sequence_);
      if (slot < 0) throw std::runtime_error("no safe completed VDMA slot");
      copied_slot = slot;
      // The reservation is held until the copy completes. The poller cannot
      // program this slot for DMA while the snapshot is being copied.
      const uint8_t* source = frame_buffer_.bytes() + slot * kFrameSlotBytes;
      __sync_synchronize();
      const auto copy_begin = std::chrono::steady_clock::now();
      std::memcpy(snapshot->data(), source, kActiveFrameBytes);
      __sync_synchronize();
      copy_ms = std::chrono::duration<double, std::milli>(
                    std::chrono::steady_clock::now() - copy_begin).count();
      delivered_sequence_ = scheduler_.sequence();
      copied_sequence = delivered_sequence_;
      if (sequence) *sequence = delivered_sequence_;
      scheduler_.Unpin();
    }
    std::cout << "SNAPSHOT_COPY frame_sequence=" << copied_sequence
              << " source_slot=" << copied_slot
              << " copy_ms=" << copy_ms << '\n' << std::flush;
    if (bgr_to_rgb) {
      for (size_t i = 0; i < snapshot->size(); i += 3) {
        std::swap((*snapshot)[i], (*snapshot)[i + 2]);
      }
    }
  }

  std::string RegisterDump() const {
    std::ostringstream out;
    out << std::hex << std::setfill('0')
        << "S2MM CR=0x" << std::setw(8) << Read32(s2mm_, kS2mmControl)
        << " DMASR=0x" << std::setw(8) << Read32(s2mm_, kS2mmStatus)
        << " PARK=0x" << std::setw(8) << Read32(s2mm_, kVdmaParkPointer)
        << " VERSION=0x" << std::setw(8) << Read32(s2mm_, kVdmaVersion)
        << " VSIZE=" << std::dec << Read32(s2mm_, kS2mmVsize)
        << " HSIZE=" << Read32(s2mm_, kS2mmHsize)
        << " STRIDE=" << Read32(s2mm_, kS2mmStride)
        << " OBS_HSIZE=0x" << std::hex << std::setw(8)
        << Read32(s2mm_, kS2mmObservedHsize)
        << " OBS_VSIZE=0x" << std::setw(8)
        << Read32(s2mm_, kS2mmObservedVsize)
        << " MM2S_CR=0x" << std::hex << std::setw(8)
        << Read32(mm2s_, kMm2sControl)
        << " MM2S_DMASR=0x" << std::setw(8)
        << Read32(mm2s_, kMm2sStatus);
    for (uint32_t i = 0; i < kFrameStoreCount; ++i) {
      out << " START" << i << "=0x" << std::setw(8)
          << Read32(s2mm_, kS2mmStart0 + i * 4);
    }
    return out.str();
  }

  void PollFrames() {
    try {
      auto last_frame = std::chrono::steady_clock::now();
      while (!stop_.load()) {
        const uint32_t read_status = Read32(mm2s_, kMm2sStatus);
        const uint32_t write_status = Read32(s2mm_, kS2mmStatus);
        if ((read_status | write_status) & kVdmaStatusErrors) {
          throw std::runtime_error("VDMA status error: " + RegisterDump());
        }

        if (read_status & kVdmaFrameIrq) {
          Write32(mm2s_, kMm2sStatus, kVdmaFrameIrq);
          std::lock_guard<std::mutex> lock(mutex_);
          const int ready = scheduler_.ready();
          if (ready >= 0 && ready != scheduler_.writing()) {
            const uint32_t park = Read32(mm2s_, kVdmaParkPointer);
            if (static_cast<int>(park & 0x1f) != ready) {
              Write32(mm2s_, kVdmaParkPointer,
                      (park & ~0x1fU) | static_cast<uint32_t>(ready));
            }
          }
        }

        if (write_status & kVdmaFrameIrq) {
          std::unique_lock<std::mutex> lock(mutex_);
          const int completed = scheduler_.writing();
          const uint32_t park = Read32(s2mm_, kVdmaParkPointer);
          if (static_cast<int>((park >> 24) & 0x1f) != completed) {
            throw std::runtime_error("S2MM frame store disagrees with software: " +
                                     RegisterDump());
          }
          const uint32_t display = Read32(mm2s_, kVdmaParkPointer);
          int next = scheduler_.ChooseNext(
              static_cast<int>((display >> 16) & 0x1f),
              static_cast<int>(display & 0x1f));
          const auto free_deadline = std::chrono::steady_clock::now() +
                                     std::chrono::milliseconds(20);
          while (next < 0 && !stop_.load() &&
                 std::chrono::steady_clock::now() < free_deadline) {
            // A pending MM2S reference may become current after a display
            // frame. Until then neither display store is safe for writing.
            if ((Read32(s2mm_, kS2mmStatus) | Read32(mm2s_, kMm2sStatus)) &
                kVdmaStatusErrors) {
              throw std::runtime_error("VDMA error while waiting for a free store: " +
                                       RegisterDump());
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            const uint32_t updated_display = Read32(mm2s_, kVdmaParkPointer);
            next = scheduler_.ChooseNext(
                static_cast<int>((updated_display >> 16) & 0x1f),
                static_cast<int>(updated_display & 0x1f));
          }
          if (next < 0) {
            ++timeout_count_;
            throw std::runtime_error("no free VDMA slot after frame completion: " +
                                     RegisterDump());
          }

          // Ack only the frame-complete W1C bit. Program the next parked
          // frame, then wait for hardware to leave the completed store.
          if (!scheduler_.BeginHandoff(next)) {
            throw std::runtime_error("invalid VDMA handoff request");
          }
          Write32(s2mm_, kS2mmStatus, kVdmaFrameIrq);
          Write32(s2mm_, kVdmaParkPointer,
                  (park & ~0x1f00U) | (static_cast<uint32_t>(next) << 8));
          const auto deadline = std::chrono::steady_clock::now() +
                                std::chrono::milliseconds(150);
          while (!stop_.load()) {
            const uint32_t status = Read32(s2mm_, kS2mmStatus);
            if (status & kVdmaStatusErrors) {
              throw std::runtime_error("S2MM error before next frame: " +
                                       RegisterDump());
            }
            if (status & kVdmaFrameIrq) {
              throw std::runtime_error("another S2MM frame completed before store transition: " +
                                       RegisterDump());
            }
            if (((Read32(s2mm_, kVdmaParkPointer) >> 24) & 0x1f) ==
                static_cast<uint32_t>(next)) {
              break;
            }
            if (std::chrono::steady_clock::now() >= deadline) {
              ++timeout_count_;
              throw std::runtime_error("S2MM did not enter next write store: " +
                                       RegisterDump());
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
          }
          if (stop_.load()) break;
          if (!scheduler_.PublishAfterWriteMoved(
                  completed,
                  static_cast<int>((Read32(s2mm_, kVdmaParkPointer) >> 24) & 0x1f))) {
            throw std::runtime_error("invalid VDMA buffer ownership transition");
          }
          last_frame = std::chrono::steady_clock::now();
          std::cout << "FRAME completed_sequence=" << scheduler_.sequence()
                    << " completed_slot=" << completed
                    << " dma_writing_slot=" << next << '\n' << std::flush;
          lock.unlock();
          frame_ready_.notify_all();
        }

        if (std::chrono::steady_clock::now() - last_frame >
            std::chrono::milliseconds(1500)) {
          std::lock_guard<std::mutex> lock(mutex_);
          ++timeout_count_;
          throw std::runtime_error("S2MM frame completion timeout: " +
                                   RegisterDump());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    } catch (...) {
      uint64_t failures = 0;
      uint64_t timeouts = 0;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        ++error_count_;
        failures = error_count_;
        timeouts = timeout_count_;
        poll_error_ = std::current_exception();
      }
      try {
        std::rethrow_exception(poll_error_);
      } catch (const std::exception& error) {
        std::cerr << "VDMA ERROR error_count=" << failures
                  << " timeout_count=" << timeouts << ' '
                  << error.what() << '\n' << std::flush;
      }
      frame_ready_.notify_all();
    }
  }

 private:
  HardwareOptions options_;
  int memory_fd_ = -1;
  int i2c_fd_ = -1;
  Mapping frame_buffer_;
  Mapping s2mm_;
  Mapping mm2s_;
  Mapping gpio_;
  Mapping vtc_;
  Mapping dynclk_;
  bool started_ = false;
  std::atomic<bool> stop_{false};
  std::thread poll_thread_;
  mutable std::mutex mutex_;
  std::condition_variable frame_ready_;
  VdmaFrameScheduler scheduler_;
  uint64_t delivered_sequence_ = 0;
  uint64_t timeout_count_ = 0;
  uint64_t error_count_ = 0;
  std::exception_ptr poll_error_;
};

CameraPipeline::CameraPipeline(const HardwareOptions& options)
    : impl_(new Impl(options)) {}
CameraPipeline::~CameraPipeline() = default;
void CameraPipeline::Initialize() { impl_->Initialize(); }
void CameraPipeline::CaptureLatest(std::vector<uint8_t>* snapshot,
                                   bool bgr_to_rgb, uint64_t* sequence) {
  impl_->CaptureLatest(snapshot, bgr_to_rgb, sequence);
}
PipelineStatus CameraPipeline::GetStatus() const { return impl_->GetStatus(); }

}  // namespace eagleeye
