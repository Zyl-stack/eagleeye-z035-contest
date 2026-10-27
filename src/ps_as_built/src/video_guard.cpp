#include "frame_checks.hpp"
#include "reservation.hpp"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace {
volatile sig_atomic_t stopped = 0;
void Stop(int) { stopped = 1; }
class ReadMap {
 public:
  ReadMap(int fd, uintptr_t base, size_t size) : size_(size) {
    ptr_ = mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, base);
    if (ptr_ == MAP_FAILED) throw std::runtime_error("read-only mmap failed");
  }
  ~ReadMap() { munmap(ptr_, size_); }
  uint32_t Read(size_t offset) const {
    uint32_t value = *reinterpret_cast<const volatile uint32_t*>(
        static_cast<const uint8_t*>(ptr_) + offset);
    __sync_synchronize();
    return value;
  }
  const volatile uint8_t* Bytes() const { return static_cast<const volatile uint8_t*>(ptr_); }
 private:
  void* ptr_;
  size_t size_;
};

bool Geometry(const ReadMap& map, bool write) {
  const size_t v = write ? 0xa0 : 0x50;
  const size_t addr = write ? 0xac : 0x5c;
  return eagleeye::CompatibleGeometry(map.Read(v), map.Read(v+4), map.Read(v+8),
                                      {{map.Read(addr), map.Read(addr+4), map.Read(addr+8)}});
}

void Dump(const ReadMap& write, const ReadMap& read) {
  std::printf("VDMA S2MM_CR=0x%08x S2MM_SR=0x%08x MM2S_CR=0x%08x MM2S_SR=0x%08x PARK=0x%08x height=%u hsize=%u stride=%u START=0x%08x,0x%08x,0x%08x\n",
              write.Read(0x30), write.Read(0x34), read.Read(0), read.Read(4),
              write.Read(0x28), write.Read(0xa0), write.Read(0xa4), write.Read(0xa8),
              write.Read(0xac), write.Read(0xb0), write.Read(0xb4));
  std::fflush(stdout);
}
}  // namespace

int main(int argc, char** argv) {
  std::signal(SIGINT, Stop);
  std::signal(SIGTERM, Stop);
  std::string mode, memory = "/dev/mem", dt = "/sys/firmware/devicetree/base";
  int timeout_ms = 45000;
  try {
    if (argc < 2) throw std::runtime_error("usage: video_guard inspect|wait [--timeout-ms N]");
    mode = argv[1];
    if (mode != "inspect" && mode != "wait") throw std::runtime_error("invalid guard mode");
    for (int i = 2; i < argc; ++i) {
      std::string opt = argv[i];
      if (++i >= argc) throw std::runtime_error("missing option value");
      if (opt == "--memory") memory = argv[i];
      else if (opt == "--dt-root") dt = argv[i];
      else if (opt == "--timeout-ms") {
        char* end = nullptr;
        const long value = std::strtol(argv[i], &end, 10);
        if (!end || *end || value < 100 || value > 120000) throw std::runtime_error("invalid timeout");
        timeout_ms = static_cast<int>(value);
      } else throw std::runtime_error("unknown guard option");
    }
    eagleeye::RequireReservation(dt);
    const int fd = open(memory.c_str(), O_RDONLY | O_SYNC);
    if (fd < 0) throw std::runtime_error("cannot open frame memory read-only");
    ReadMap write(fd, eagleeye::kS2mmVdmaBase, 4096);
    ReadMap read(fd, eagleeye::kMm2sVdmaBase, 4096);
    ReadMap frames(fd, eagleeye::kFrameBufferBase, eagleeye::kFrameBufferBytes);
    close(fd);
    Dump(write, read);
    if ((write.Read(0x30) & 1U) == 0 && (read.Read(0) & 1U) == 0 &&
        (write.Read(0x34) & 1U) && (read.Read(4) & 1U)) {
      std::puts("VIDEO_COLD both_channels_stopped");
      return 10;
    }
    if (!Geometry(write, true) || !Geometry(read, false) ||
        eagleeye::FatalDma(write.Read(0x30), write.Read(0x34)) ||
        eagleeye::FatalDma(read.Read(0), read.Read(4)))
      throw std::runtime_error("incompatible or stopped video; refuse automatic live reinitialization");
    if ((write.Read(0x34) | read.Read(4)) & 0xff0U)
      std::puts("WARNING latched VDMA errors preserved; no W1C/reset performed; inspect logs");
    if (mode == "inspect") return 0;
    eagleeye::ReadyTracker tracker;
    const auto begin = std::chrono::steady_clock::now();
    unsigned tick = 0;
    while (!stopped) {
      if (!Geometry(write, true) || !Geometry(read, false) ||
          eagleeye::FatalDma(write.Read(0x30), write.Read(0x34)) ||
          eagleeye::FatalDma(read.Read(0), read.Read(4)))
        throw std::runtime_error("video DMA changed or stopped during readiness wait");
      std::array<eagleeye::Signature, 3> signatures;
      for (uint32_t i = 0; i < 3; ++i)
        signatures[i] = eagleeye::SampleFrame(frames.Bytes() + i * eagleeye::kFrameSlotBytes);
      const bool ready = tracker.Observe(signatures);
      const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - begin).count();
      if (tick++ % 5 == 0 || ready) {
        std::printf("VIDEO_WAIT elapsed_ms=%lld hash=%08x,%08x,%08x nonzero=%d%d%d\n",
                    static_cast<long long>(ms), signatures[0].hash, signatures[1].hash,
                    signatures[2].hash, signatures[0].nonzero, signatures[1].nonzero,
                    signatures[2].nonzero);
        std::fflush(stdout);
      }
      if (ready) {
        Dump(write, read);
        std::puts("VIDEO_READY evidence=sampled_content_changes frame_completion=unverified");
        return 0;
      }
      if (ms >= timeout_ms)
        throw std::runtime_error("no observed nonzero changing frame; move/light camera scene; cold power-cycle if black; no inference started");
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 130;
  } catch (const std::exception& error) {
    std::cerr << "ERROR video_guard: " << error.what() << '\n';
    return 20;
  }
}
