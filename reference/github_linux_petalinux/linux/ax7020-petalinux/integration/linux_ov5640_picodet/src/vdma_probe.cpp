#include "frame_layout.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#ifndef I2C_SLAVE
#define I2C_SLAVE 0x0703
#endif

// Keep the probe's optional sensor setup identical to the delivered Linux
// application's register table; it does not include Xilinx BSP code.
#define EAGLEEYE_LINUX_TABLE_ONLY
#include "../reference_baremetal/ov5640.c"
#undef EAGLEEYE_LINUX_TABLE_ONLY

namespace {

using Clock = std::chrono::steady_clock;

constexpr size_t kMmioSpan = 0x1000;
constexpr uint32_t kS2mmControl = 0x30;
constexpr uint32_t kS2mmStatus = 0x34;
constexpr uint32_t kS2mmFramePointerStatus = 0x24;
constexpr uint32_t kParkPointer = 0x28;
constexpr uint32_t kVersion = 0x2c;
constexpr uint32_t kS2mmVsize = 0xa0;
constexpr uint32_t kS2mmHsize = 0xa4;
constexpr uint32_t kS2mmStride = 0xa8;
constexpr uint32_t kS2mmStart0 = 0xac;
constexpr uint32_t kS2mmObservedHsize = 0xf0;
constexpr uint32_t kS2mmObservedVsize = 0xf4;
constexpr uint32_t kMm2sControl = 0x00;
constexpr uint32_t kMm2sStatus = 0x04;
constexpr uint32_t kVdmaRun = 0x00000001;
constexpr uint32_t kVdmaReset = 0x00000004;
constexpr uint32_t kVdmaFrameIrqEnable = 0x00001000;
constexpr uint32_t kVdmaErrorIrqEnable = 0x00004000;
constexpr uint32_t kVdmaW1c = 0x0000d990;

struct Options {
  std::string devmem = "/dev/mem";
  std::string i2c = "/dev/i2c-0";
  int cycles = 100;
  int period_ms = 40;
  bool initialize_vdma = false;
  bool initialize_sensor = false;
  uint32_t w1c_mask = 0;
};

struct Mapping {
  Mapping(int fd, uintptr_t physical, size_t length) { Map(fd, physical, length); }
  ~Mapping() {
    if (mapping_ != MAP_FAILED) munmap(mapping_, mapping_length_);
  }
  Mapping(const Mapping&) = delete;
  Mapping& operator=(const Mapping&) = delete;

  volatile uint32_t* reg(uint32_t offset) const {
    return reinterpret_cast<volatile uint32_t*>(data_ + offset);
  }
  const uint8_t* bytes() const { return data_; }

 private:
  void Map(int fd, uintptr_t physical, size_t length) {
    const long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) throw std::runtime_error("sysconf(_SC_PAGESIZE) failed");
    const uintptr_t mask = static_cast<uintptr_t>(page_size - 1);
    const uintptr_t page = physical & ~mask;
    const size_t offset = static_cast<size_t>(physical - page);
    mapping_length_ = (offset + length + mask) & ~static_cast<size_t>(mask);
    mapping_ = mmap(nullptr, mapping_length_, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, static_cast<off_t>(page));
    if (mapping_ == MAP_FAILED) {
      throw std::runtime_error("mmap failed: " + std::string(std::strerror(errno)));
    }
    data_ = static_cast<uint8_t*>(mapping_) + offset;
  }

  void* mapping_ = MAP_FAILED;
  size_t mapping_length_ = 0;
  uint8_t* data_ = nullptr;
};

uint32_t Read32(const Mapping& map, uint32_t offset) {
  const uint32_t value = *map.reg(offset);
  __sync_synchronize();
  return value;
}

void Write32(const Mapping& map, uint32_t offset, uint32_t value) {
  __sync_synchronize();
  *map.reg(offset) = value;
  __sync_synchronize();
}

uint32_t Crc32Update(uint32_t crc, const uint8_t* data, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    crc ^= data[i];
    for (int bit = 0; bit < 8; ++bit) {
      crc = (crc >> 1) ^ ((crc & 1U) ? 0xedb88320U : 0U);
    }
  }
  return crc;
}

uint32_t FrameSampleCrc(const uint8_t* frame) {
  constexpr size_t kBlockBytes = 1024;
  constexpr size_t kOffsets[] = {
      0,
      eagleeye::kActiveFrameBytes / 3,
      2 * eagleeye::kActiveFrameBytes / 3,
      eagleeye::kActiveFrameBytes - kBlockBytes,
  };
  uint32_t crc = 0xffffffffU;
  for (size_t offset : kOffsets) crc = Crc32Update(crc, frame + offset, kBlockBytes);
  return ~crc;
}

uint32_t ParseU32(const std::string& value) {
  size_t parsed = 0;
  const unsigned long result = std::stoul(value, &parsed, 0);
  if (parsed != value.size() || result > 0xffffffffUL) {
    throw std::invalid_argument("invalid 32-bit value: " + value);
  }
  return static_cast<uint32_t>(result);
}

void Usage(const char* program) {
  std::cerr
      << "Usage: " << program << " [options]\n"
      << "  --cycles N                 samples to record, default 100\n"
      << "  --period-ms N              sampling interval, default 40\n"
      << "  --devmem PATH              default /dev/mem\n"
      << "  --i2c PATH                 default /dev/i2c-0\n"
      << "  --initialize-vdma          reset/program S2MM parking mode, no W1C\n"
      << "  --initialize-sensor        reset/program OV5640 before observing\n"
      << "  --w1c-mask MASK            explicitly W1C S2MM DMASR after recording\n"
      << "  --help\n";
}

Options ParseArgs(int argc, char** argv) {
  Options options;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      Usage(argv[0]);
      std::exit(0);
    }
    if (arg == "--initialize-vdma") {
      options.initialize_vdma = true;
      continue;
    }
    if (arg == "--initialize-sensor") {
      options.initialize_sensor = true;
      continue;
    }
    if (i + 1 >= argc) throw std::invalid_argument("missing value after " + arg);
    const std::string value = argv[++i];
    if (arg == "--cycles") options.cycles = std::stoi(value);
    else if (arg == "--period-ms") options.period_ms = std::stoi(value);
    else if (arg == "--devmem") options.devmem = value;
    else if (arg == "--i2c") options.i2c = value;
    else if (arg == "--w1c-mask") options.w1c_mask = ParseU32(value);
    else throw std::invalid_argument("unknown option: " + arg);
  }
  if (options.cycles <= 0 || options.period_ms <= 0) {
    throw std::invalid_argument("--cycles and --period-ms must be positive");
  }
  if (options.w1c_mask & ~kVdmaW1c) {
    throw std::invalid_argument("--w1c-mask contains unsupported bits");
  }
  return options;
}

void WaitReset(const Mapping& s2mm) {
  const auto deadline = Clock::now() + std::chrono::milliseconds(100);
  while (Read32(s2mm, kS2mmControl) & kVdmaReset) {
    if (Clock::now() >= deadline) throw std::runtime_error("S2MM reset timeout");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void ConfigureS2mmParking(const Mapping& s2mm) {
  Write32(s2mm, kS2mmControl, Read32(s2mm, kS2mmControl) | kVdmaReset);
  WaitReset(s2mm);
  for (uint32_t slot = 0; slot < eagleeye::kFrameStoreCount; ++slot) {
    Write32(s2mm, kS2mmStart0 + 4 * slot,
            static_cast<uint32_t>(eagleeye::FrameAddress(slot)));
  }
  Write32(s2mm, kS2mmStride, eagleeye::kFrameStride);
  Write32(s2mm, kS2mmHsize, eagleeye::kFrameStride);
  const uint32_t park = Read32(s2mm, kParkPointer);
  Write32(s2mm, kParkPointer, (park & ~0x1f00U));  // WRTREF = slot 0.
  // Circular/Park bit remains clear. No status bit is cleared here.
  Write32(s2mm, kS2mmControl,
          kVdmaRun | kVdmaFrameIrqEnable | kVdmaErrorIrqEnable);
  Write32(s2mm, kS2mmVsize, eagleeye::kFrameHeight);
}

void WriteAll(int fd, const uint8_t* data, size_t size) {
  size_t done = 0;
  while (done < size) {
    const ssize_t written = write(fd, data + done, size - done);
    if (written < 0 && errno == EINTR) continue;
    if (written <= 0) throw std::runtime_error("OV5640 I2C write failed");
    done += static_cast<size_t>(written);
  }
}

void SensorInit(const Options& options) {
  const int fd = open(options.i2c.c_str(), O_RDWR);
  if (fd < 0) throw std::runtime_error("open I2C failed: " + std::string(std::strerror(errno)));
  try {
    if (ioctl(fd, I2C_SLAVE, 0x3c) < 0) throw std::runtime_error("OV5640 I2C select failed");
    for (size_t i = 0; sensor_init_data[i].reg != SEQUENCE_END; ++i) {
      const uint8_t message[3] = {
          static_cast<uint8_t>(sensor_init_data[i].reg >> 8),
          static_cast<uint8_t>(sensor_init_data[i].reg), sensor_init_data[i].val};
      WriteAll(fd, message, sizeof(message));
      if (sensor_init_data[i].reg == 0x3008 && sensor_init_data[i].val == 0x82) usleep(5000);
    }
  } catch (...) {
    close(fd);
    throw;
  }
  close(fd);
}

std::string Hex(uint32_t value) {
  std::ostringstream out;
  out << "0x" << std::hex << std::setw(8) << std::setfill('0') << value;
  return out.str();
}

}  // namespace

int main(int argc, char** argv) {
  try {
    const Options options = ParseArgs(argc, argv);
    const int fd = open(options.devmem.c_str(), O_RDWR | O_SYNC);
    if (fd < 0) throw std::runtime_error("open /dev/mem failed: " + std::string(std::strerror(errno)));
    try {
      Mapping s2mm(fd, eagleeye::kS2mmVdmaBase, kMmioSpan);
      Mapping mm2s(fd, eagleeye::kMm2sVdmaBase, kMmioSpan);
      Mapping frame_buffers(fd, eagleeye::kFrameBufferBase, eagleeye::kFrameBufferBytes);
      if (options.initialize_sensor) SensorInit(options);
      if (options.initialize_vdma) ConfigureS2mmParking(s2mm);

      std::cout << "VDMA_PROBE begin cycles=" << options.cycles
                << " period_ms=" << options.period_ms
                << " initialize_sensor=" << options.initialize_sensor
                << " initialize_vdma=" << options.initialize_vdma
                << " w1c_mask=" << Hex(options.w1c_mask) << '\n';
      uint32_t previous_s2mm_status = 0;
      uint32_t previous_park = 0;
      uint32_t previous_crc[eagleeye::kFrameStoreCount] = {};
      bool have_previous = false;
      for (int cycle = 0; cycle < options.cycles; ++cycle) {
        const auto now = Clock::now().time_since_epoch();
        const uint64_t timestamp_us = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(now).count());
        const uint32_t s2mm_cr = Read32(s2mm, kS2mmControl);
        const uint32_t s2mm_dmasr = Read32(s2mm, kS2mmStatus);
        const uint32_t s2mm_park = Read32(s2mm, kParkPointer);
        const uint32_t s2mm_frmptr = Read32(s2mm, kS2mmFramePointerStatus);
        const uint32_t mm2s_cr = Read32(mm2s, kMm2sControl);
        const uint32_t mm2s_dmasr = Read32(mm2s, kMm2sStatus);
        const uint32_t mm2s_park = Read32(mm2s, kParkPointer);
        uint32_t crc[eagleeye::kFrameStoreCount] = {};
        for (uint32_t slot = 0; slot < eagleeye::kFrameStoreCount; ++slot) {
          const uint8_t* sample = frame_buffers.bytes() + slot * eagleeye::kFrameSlotBytes;
          // Four fixed 1 KiB spans across the active image make a 4 KiB
          // sample without claiming to validate an entire DMA frame.
          crc[slot] = FrameSampleCrc(sample);
        }
        const bool changed = have_previous &&
            (s2mm_dmasr != previous_s2mm_status || s2mm_park != previous_park ||
             crc[0] != previous_crc[0] || crc[1] != previous_crc[1] || crc[2] != previous_crc[2]);
        std::cout << "PROBE cycle=" << cycle << " timestamp_us=" << timestamp_us
                  << " changed=" << changed
                  << " S2MM_CR=" << Hex(s2mm_cr)
                  << " S2MM_DMASR=" << Hex(s2mm_dmasr)
                  << " S2MM_PARK=" << Hex(s2mm_park)
                  << " S2MM_FRMPTR_STS=" << Hex(s2mm_frmptr)
                  << " S2MM_WRTSTR=" << ((s2mm_park >> 24) & 0x1f)
                  << " S2MM_WRTREF=" << ((s2mm_park >> 8) & 0x1f)
                  << " S2MM_IRQ_ENABLE=" << Hex(s2mm_cr & 0x00007000U)
                  << " S2MM_IRQ_STATUS=" << Hex(s2mm_dmasr & 0x00007000U)
                  << " S2MM_VSIZE=" << Read32(s2mm, kS2mmVsize)
                  << " S2MM_HSIZE=" << Read32(s2mm, kS2mmHsize)
                  << " S2MM_STRIDE=" << (Read32(s2mm, kS2mmStride) & 0xffffU)
                  << " S2MM_START0=" << Hex(Read32(s2mm, kS2mmStart0))
                  << " S2MM_START1=" << Hex(Read32(s2mm, kS2mmStart0 + 4))
                  << " S2MM_START2=" << Hex(Read32(s2mm, kS2mmStart0 + 8))
                  << " S2MM_OBS_HSIZE=" << Hex(Read32(s2mm, kS2mmObservedHsize))
                  << " S2MM_OBS_VSIZE=" << Hex(Read32(s2mm, kS2mmObservedVsize))
                  << " MM2S_CR=" << Hex(mm2s_cr)
                  << " MM2S_DMASR=" << Hex(mm2s_dmasr)
                  << " MM2S_PARK=" << Hex(mm2s_park)
                  << " MM2S_READSTR=" << ((mm2s_park >> 16) & 0x1f)
                  << " MM2S_READREF=" << (mm2s_park & 0x1f)
                  << " frame_buffer_0_crc=" << Hex(crc[0])
                  << " frame_buffer_1_crc=" << Hex(crc[1])
                  << " frame_buffer_2_crc=" << Hex(crc[2]) << '\n' << std::flush;

        if (options.w1c_mask != 0) {
          std::cout << "W1C cycle=" << cycle << " before=" << Hex(s2mm_dmasr)
                    << " mask=" << Hex(options.w1c_mask);
          Write32(s2mm, kS2mmStatus, options.w1c_mask);
          std::cout << " after=" << Hex(Read32(s2mm, kS2mmStatus)) << '\n' << std::flush;
        }
        previous_s2mm_status = s2mm_dmasr;
        previous_park = s2mm_park;
        std::memcpy(previous_crc, crc, sizeof(crc));
        have_previous = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(options.period_ms));
      }
      std::cout << "VDMA_PROBE complete w1c_performed=" << (options.w1c_mask != 0) << '\n';
    } catch (...) {
      close(fd);
      throw;
    }
    close(fd);
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "VDMA_PROBE ERROR " << error.what() << '\n';
    return 2;
  }
}
