#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>

#include "eagleeye_hw.hpp"
#include "mouse_roi_controller.hpp"
#include "roi_animator.hpp"
#include "frame_layout.h"
#include "paddle_api.h"

namespace {

using Clock = std::chrono::steady_clock;
std::atomic<bool> g_stop(false);

struct Options {
  std::string model = "model.nb";
  std::string labels = "label_list.txt";
  std::string raw;
  std::string dump_first;
  std::string json_output;
  std::string devmem = "/dev/mem";
  std::string i2c = "/dev/i2c-0";
  int threads = 2;
  int warmup = 1;
  int iterations = 0;
  float threshold = 0.5f;
  bool bgr = false;
  bool initialize_sensor = true;
  bool initialize_display = true;
  bool attach_existing_video = false;
  bool fovea_roi = false;
  bool mouse_roi = false;
  std::string mouse_device = "auto";
  int mouse_sensitivity = 2;
  std::string preprocess = "auto";
};

struct Detection {
  int class_id = -1;
  float confidence = 0.f;
  float x = 0.f;
  float y = 0.f;
  float w = 0.f;
  float h = 0.f;
};

struct Timing {
  double capture_ms = 0;
  double preprocess_ms = 0;
  double inference_ms = 0;
  double postprocess_ms = 0;
  double total_ms = 0;
  double fps = 0;
};

double Ms(Clock::time_point begin, Clock::time_point end) {
  return std::chrono::duration<double, std::milli>(end - begin).count();
}

void SignalHandler(int) { g_stop.store(true); }

class AppLock {
 public:
  AppLock() {
    fd_ = open("/tmp/eagleeye-picodet-v4-application.lock", O_CREAT | O_RDWR, 0600);
    if (fd_ < 0) throw std::runtime_error("cannot create application lock");
    if (flock(fd_, LOCK_EX | LOCK_NB) != 0) {
      close(fd_);
      fd_ = -1;
      throw std::runtime_error("another v4 inference process is already running");
    }
  }
  ~AppLock() { if (fd_ >= 0) close(fd_); }
 private:
  int fd_ = -1;
};

int ParseInt(const std::string& value, int lo, int hi) {
  char* end = nullptr;
  errno = 0;
  long number = std::strtol(value.c_str(), &end, 10);
  if (errno || end == value.c_str() || *end || number < lo || number > hi)
    throw std::runtime_error("numeric option out of range: " + value);
  return static_cast<int>(number);
}

void Usage(const char* program) {
  std::cerr
      << "Usage: " << program << " [options]\n"
      << "  --model FILE            Paddle-Lite model (default model.nb)\n"
      << "  --labels FILE           COCO label list\n"
      << "  --threads N             ARM worker threads (default 2)\n"
      << "  --threshold F           PERSON threshold (default 0.5)\n"
      << "  --warmup N              untimed model warmup runs (default 1)\n"
      << "  --iterations N          0 means run until SIGINT (default 0)\n"
      << "  --pixel-order rgb|bgr   byte order in DDR (default rgb)\n"
      << "  --dump-first FILE.ppm   save first private RGB snapshot\n"
      << "  --json FILE             overwrite latest-result JSON each loop\n"
      << "  --devmem PATH           default /dev/mem\n"
      << "  --i2c PATH              default /dev/i2c-0\n"
      << "  --raw FILE.rgb          offline 640x360 RGB888 test source\n"
      << "  --no-sensor-init        leave OV5640 reset/I2C untouched\n"
      << "  --no-display-init       leave dynclk/VTC untouched\n"
      << "  --attach-existing-video use running VDMA without reinitializing video\n"
      << "  --fovea-roi            commit one smoothed 320x180 Lanczos ROI via AXI-Lite\n"
      << "  --mouse-roi            use a USB relative mouse while the video is paused\n"
      << "  --mouse-device PATH    input event path or auto (default auto)\n"
      << "  --mouse-sensitivity N  ROI pixels per mouse count, 1..8 (default 2)\n";
  std::cerr << "  --preprocess auto|fpga|cpu preprocessing backend (default auto)\n";
}

bool ParseArgs(int argc, char** argv, Options* opt) {
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      Usage(argv[0]);
      return false;
    }
    if (arg == "--no-sensor-init") {
      opt->initialize_sensor = false;
      continue;
    }
    if (arg == "--no-display-init") {
      opt->initialize_display = false;
      continue;
    }
    if (arg == "--attach-existing-video") {
      opt->attach_existing_video = true;
      continue;
    }
    if (arg == "--fovea-roi") {
      opt->fovea_roi = true;
      continue;
    }
    if (arg == "--mouse-roi") {
      opt->mouse_roi = true;
      continue;
    }
    if (i + 1 >= argc) {
      std::cerr << "Missing value after " << arg << '\n';
      return false;
    }
    const std::string value = argv[++i];
    if (arg == "--model") opt->model = value;
    else if (arg == "--labels") opt->labels = value;
    else if (arg == "--raw") opt->raw = value;
    else if (arg == "--dump-first") opt->dump_first = value;
    else if (arg == "--json") opt->json_output = value;
    else if (arg == "--devmem") opt->devmem = value;
    else if (arg == "--i2c") opt->i2c = value;
    else if (arg == "--mouse-device") opt->mouse_device = value;
    else if (arg == "--mouse-sensitivity")
      opt->mouse_sensitivity = ParseInt(value, 1, 8);
    else if (arg == "--threads") opt->threads = ParseInt(value, 1, 2);
    else if (arg == "--warmup") opt->warmup = ParseInt(value, 0, 100);
    else if (arg == "--iterations") opt->iterations = ParseInt(value, 0, 1000000000);
    else if (arg == "--threshold") {
      char* end = nullptr;
      opt->threshold = std::strtof(value.c_str(), &end);
      if (end == value.c_str() || *end || !std::isfinite(opt->threshold) ||
          opt->threshold < 0 || opt->threshold > 1)
        throw std::runtime_error("invalid threshold");
    }
    else if (arg == "--pixel-order") {
      if (value != "rgb" && value != "bgr") {
        std::cerr << "--pixel-order must be rgb or bgr\n";
        return false;
      }
      opt->bgr = value == "bgr";
    } else if (arg == "--preprocess") {
      if (value != "auto" && value != "fpga" && value != "cpu") {
        std::cerr << "--preprocess must be auto, fpga, or cpu\n";
        return false;
      }
      opt->preprocess = value;
    } else {
      std::cerr << "Unknown option: " << arg << '\n';
      return false;
    }
  }
  return true;
}

std::vector<uint8_t> ReadRaw(const std::string& path) {
  std::ifstream in(path.c_str(), std::ios::binary);
  if (!in) throw std::runtime_error("cannot read raw frame: " + path);
  std::vector<uint8_t> data(eagleeye::kActiveFrameBytes);
  in.read(reinterpret_cast<char*>(data.data()), data.size());
  if (in.gcount() != static_cast<std::streamsize>(data.size()) ||
      in.peek() != std::ifstream::traits_type::eof()) {
    throw std::runtime_error("raw frame must be exactly 691200 bytes: " + path);
  }
  return data;
}

void WritePpm(const std::string& path, const std::vector<uint8_t>& rgb) {
  std::ofstream out(path.c_str(), std::ios::binary);
  if (!out) throw std::runtime_error("cannot create frame dump: " + path);
  out << "P6\n" << eagleeye::kFrameWidth << ' ' << eagleeye::kFrameHeight
      << "\n255\n";
  out.write(reinterpret_cast<const char*>(rgb.data()), rgb.size());
  if (!out) throw std::runtime_error("cannot write frame dump: " + path);
}

float CubicWeight(float x) {
  constexpr float a = -0.75f;
  x = std::fabs(x);
  if (x <= 1.f) return (a + 2.f) * x * x * x - (a + 3.f) * x * x + 1.f;
  if (x < 2.f) return a * x * x * x - 5.f * a * x * x + 8.f * a * x - 4.f * a;
  return 0.f;
}

void ResizeNormalizeNchw(const uint8_t* src, std::vector<float>* nchw) {
  constexpr int dst_w = 320;
  constexpr int dst_h = 320;
  constexpr float mean[3] = {0.485f, 0.456f, 0.406f};
  constexpr float stdv[3] = {0.229f, 0.224f, 0.225f};
  const float sx = static_cast<float>(eagleeye::kFrameWidth) / dst_w;
  const float sy = static_cast<float>(eagleeye::kFrameHeight) / dst_h;
  nchw->resize(static_cast<size_t>(3) * dst_h * dst_w);

  for (int dy = 0; dy < dst_h; ++dy) {
    const float fy = (dy + 0.5f) * sy - 0.5f;
    const int iy = static_cast<int>(std::floor(fy));
    for (int dx = 0; dx < dst_w; ++dx) {
      const float fx = (dx + 0.5f) * sx - 0.5f;
      const int ix = static_cast<int>(std::floor(fx));
      for (int c = 0; c < 3; ++c) {
        float sum = 0.f;
        float weights = 0.f;
        for (int ky = -1; ky <= 2; ++ky) {
          const int py = std::min(std::max(iy + ky, 0),
                                  static_cast<int>(eagleeye::kFrameHeight) - 1);
          const float wy = CubicWeight(fy - (iy + ky));
          for (int kx = -1; kx <= 2; ++kx) {
            const int px = std::min(std::max(ix + kx, 0),
                                    static_cast<int>(eagleeye::kFrameWidth) - 1);
            const float weight = wy * CubicWeight(fx - (ix + kx));
            const size_t source =
                (static_cast<size_t>(py) * eagleeye::kFrameWidth + px) * 3 + c;
            sum += weight * src[source];
            weights += weight;
          }
        }
        const float pixel = std::min(255.f, std::max(0.f, sum / weights));
        (*nchw)[(static_cast<size_t>(c) * dst_h + dy) * dst_w + dx] =
            (pixel / 255.f - mean[c]) / stdv[c];
      }
    }
  }
}

class FpgaPreprocessor {
 public:
  explicit FpgaPreprocessor(const std::string& devmem) {
    fd_ = open(devmem.c_str(), O_RDWR | O_SYNC);
    if (fd_ < 0) throw std::runtime_error("cannot open preprocessor " + devmem);
    buffers_ = static_cast<uint8_t*>(mmap(nullptr, kBufferBytes,
        PROT_READ | PROT_WRITE, MAP_SHARED, fd_, kBufferBase));
    if (buffers_ == MAP_FAILED) throw std::runtime_error("cannot map preprocessor buffers");
    ip_ = MapRegs(kIpBase); din_ = MapRegs(kDmaInBase);
    dr_ = MapRegs(kDmaRBase); dg_ = MapRegs(kDmaGBase); db_ = MapRegs(kDmaBBase);
    const uint32_t version = Read(ip_, 0x14);
    if (version != 0x00010000U) {
      std::ostringstream out;
      out << "unexpected preprocessor version 0x" << std::hex << version;
      throw std::runtime_error(out.str());
    }
    std::cout << "PREPROC_FPGA version=0x" << std::hex << version << std::dec
              << " input=0x" << std::hex << kInputAddress
              << " output=0x" << kOutputAddress << std::dec << '\n';
  }

  ~FpgaPreprocessor() {
    for (volatile uint32_t* p : regs_) if (p) munmap(const_cast<uint32_t*>(p), 4096);
    if (buffers_ != MAP_FAILED) munmap(buffers_, kBufferBytes);
    if (fd_ >= 0) close(fd_);
  }

  void Run(const uint8_t* rgb, std::vector<float>* nchw, uint32_t* cycles) {
    std::memcpy(buffers_, rgb, kFrameBytes);
    std::memset(buffers_ + (kOutputAddress - kBufferBase), 0, kTensorBytes);
    __sync_synchronize();
    StartDma(dr_, kOutputAddress, kPlaneBytes, true);
    StartDma(dg_, kOutputAddress + kPlaneBytes, kPlaneBytes, true);
    StartDma(db_, kOutputAddress + 2U * kPlaneBytes, kPlaneBytes, true);
    Write(ip_, 0x04, 0x1ffU);
    Write(ip_, 0x00, 1U);
    StartDma(din_, kInputAddress, kFrameBytes, false);
    const auto deadline = Clock::now() + std::chrono::milliseconds(1000);
    while (Clock::now() < deadline) {
      const uint32_t status = Read(ip_, 0x04);
      const int a = DmaFinished(din_, false), b = DmaFinished(dr_, true);
      const int c = DmaFinished(dg_, true), d = DmaFinished(db_, true);
      if (a < 0 || b < 0 || c < 0 || d < 0) ThrowStatus("DMA error", status);
      if ((status & 2U) && a && b && c && d) {
        if ((status & 0x1fcU) || Read(ip_, 0x0c) != 230400U ||
            Read(ip_, 0x10) != 102400U) ThrowStatus("IP status/count error", status);
        *cycles = Read(ip_, 0x08);
        nchw->resize(3U * 320U * 320U);
        __sync_synchronize();
        std::memcpy(nchw->data(), buffers_ + (kOutputAddress - kBufferBase), kTensorBytes);
        return;
      }
    }
    ThrowStatus("timeout", Read(ip_, 0x04));
  }

 private:
  static constexpr uint32_t kIpBase = 0x40400000U;
  static constexpr uint32_t kDmaInBase = 0x40410000U;
  static constexpr uint32_t kDmaRBase = 0x40420000U;
  static constexpr uint32_t kDmaGBase = 0x40430000U;
  static constexpr uint32_t kDmaBBase = 0x40440000U;
  static constexpr uint32_t kBufferBase = 0x1e300000U;
  static constexpr uint32_t kInputAddress = 0x1e300000U;
  static constexpr uint32_t kOutputAddress = 0x1e400000U;
  static constexpr size_t kBufferBytes = 0x00400000U;
  static constexpr uint32_t kFrameBytes = 691200U;
  static constexpr uint32_t kPlaneBytes = 409600U;
  static constexpr size_t kTensorBytes = 3U * kPlaneBytes;
  static constexpr uint32_t kDmaErrors = 0x00000770U;

  volatile uint32_t* MapRegs(uint32_t address) {
    void* p = mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, address);
    if (p == MAP_FAILED) throw std::runtime_error("cannot map preprocessor registers");
    regs_.push_back(static_cast<volatile uint32_t*>(p));
    return static_cast<volatile uint32_t*>(p);
  }
  static void Write(volatile uint32_t* p, uint32_t off, uint32_t value) {
    __sync_synchronize(); p[off / 4] = value; __sync_synchronize();
  }
  static uint32_t Read(volatile uint32_t* p, uint32_t off) {
    const uint32_t v = p[off / 4]; __sync_synchronize(); return v;
  }
  static uint32_t Cr(bool s2mm) { return s2mm ? 0x30U : 0x00U; }
  static uint32_t Sr(bool s2mm) { return s2mm ? 0x34U : 0x04U; }
  static uint32_t Address(bool s2mm) { return s2mm ? 0x48U : 0x18U; }
  static uint32_t Length(bool s2mm) { return s2mm ? 0x58U : 0x28U; }
  static void ResetDma(volatile uint32_t* p, bool s2mm) {
    Write(p, Cr(s2mm), 4U);
    const auto deadline = Clock::now() + std::chrono::milliseconds(100);
    while ((Read(p, Cr(s2mm)) & 4U) && Clock::now() < deadline) {}
    if (Read(p, Cr(s2mm)) & 4U) throw std::runtime_error("preprocessor DMA reset timeout");
    Write(p, Sr(s2mm), 0xffffffffU);
  }
  static void StartDma(volatile uint32_t* p, uint32_t address,
                       uint32_t length, bool s2mm) {
    ResetDma(p, s2mm);
    Write(p, Cr(s2mm), 1U); Write(p, Address(s2mm), address); Write(p, Length(s2mm), length);
  }
  static int DmaFinished(volatile uint32_t* p, bool s2mm) {
    const uint32_t status = Read(p, Sr(s2mm));
    if (status & kDmaErrors) return -1;
    return (status & 2U) != 0;
  }
  [[noreturn]] void ThrowStatus(const char* what, uint32_t status) const {
    std::ostringstream out;
    out << "preprocessor " << what << " ip=0x" << std::hex << status
        << " in=0x" << Read(din_, Sr(false)) << " r=0x" << Read(dr_, Sr(true))
        << " g=0x" << Read(dg_, Sr(true)) << " b=0x" << Read(db_, Sr(true));
    throw std::runtime_error(out.str());
  }

  int fd_ = -1;
  uint8_t* buffers_ = reinterpret_cast<uint8_t*>(MAP_FAILED);
  std::vector<volatile uint32_t*> regs_;
  volatile uint32_t *ip_=nullptr, *din_=nullptr, *dr_=nullptr, *dg_=nullptr, *db_=nullptr;
};

void SetInputs(paddle::lite_api::PaddlePredictor* predictor,
               const std::vector<float>& nchw) {
  for (const std::string& name : predictor->GetInputNames()) {
    std::unique_ptr<paddle::lite_api::Tensor> tensor = predictor->GetInputByName(name);
    if (name == "image") {
      tensor->Resize({1, 3, 320, 320});
      std::copy(nchw.begin(), nchw.end(), tensor->mutable_data<float>());
    } else if (name == "im_shape") {
      tensor->Resize({1, 2});
      float* data = tensor->mutable_data<float>();
      data[0] = 320.f;
      data[1] = 320.f;
    } else if (name == "scale_factor") {
      tensor->Resize({1, 2});
      float* data = tensor->mutable_data<float>();
      data[0] = 320.f / eagleeye::kFrameHeight;
      data[1] = 320.f / eagleeye::kFrameWidth;
    } else {
      throw std::runtime_error("unsupported model input: " + name);
    }
  }
}

std::vector<Detection> GetPersons(paddle::lite_api::PaddlePredictor* predictor,
                                  float threshold) {
  const std::vector<std::string> names = predictor->GetOutputNames();
  if (names.size() < 2) throw std::runtime_error("model did not return bbox_num");
  std::unique_ptr<const paddle::lite_api::Tensor> bbox = predictor->GetTensor(names[0]);
  std::unique_ptr<const paddle::lite_api::Tensor> bbox_num = predictor->GetTensor(names[1]);
  int64_t values = 1;
  for (int64_t dimension : bbox->shape()) values *= dimension;
  if (values < 0 || values % 6 != 0) {
    throw std::runtime_error("unexpected bbox output shape");
  }
  int count = static_cast<int>(values / 6);
  if (!bbox_num->shape().empty()) count = std::min(count, bbox_num->data<int>()[0]);
  const float* data = bbox->data<float>();
  std::vector<Detection> persons;
  for (int i = 0; i < count; ++i) {
    const int class_id = static_cast<int>(std::lround(data[i * 6]));
    const float score = data[i * 6 + 1];
    if (class_id != 0 || score < threshold) continue;
    const float xmin = std::min<float>(eagleeye::kFrameWidth,
                                       std::max(0.f, data[i * 6 + 2]));
    const float ymin = std::min<float>(eagleeye::kFrameHeight,
                                       std::max(0.f, data[i * 6 + 3]));
    const float xmax = std::min<float>(eagleeye::kFrameWidth,
                                       std::max(0.f, data[i * 6 + 4]));
    const float ymax = std::min<float>(eagleeye::kFrameHeight,
                                       std::max(0.f, data[i * 6 + 5]));
    persons.push_back({class_id, score, xmin, ymin,
                       std::max(0.f, xmax - xmin),
                       std::max(0.f, ymax - ymin)});
  }
  std::sort(persons.begin(), persons.end(),
            [](const Detection& a, const Detection& b) {
              return a.confidence > b.confidence;
            });
  return persons;
}

void WriteJson(const std::string& path, int sequence, const Timing& timing,
               const std::vector<Detection>& persons,
               const eagleeye::SnapshotInfo* snapshot_info) {
  const std::string temp = path + ".tmp";
  std::ofstream out(temp.c_str());
  if (!out) throw std::runtime_error("cannot write JSON: " + path);
  out << std::fixed << std::setprecision(4)
      << "{\n  \"sequence\": " << sequence
      << ",\n  \"image_width\": 640,\n  \"image_height\": 360,\n"
      << "  \"sequence_kind\": \"inference\",\n"
      << "  \"frame_freshness\": \"unverified\",\n";
  if (snapshot_info) {
    out << "  \"snapshot_slot\": " << snapshot_info->slot
        << ",\n  \"content_hash\": " << snapshot_info->hash << ",\n";
  }
  out
      << "  \"timing_ms\": {\"capture\": " << timing.capture_ms
      << ", \"preprocess\": " << timing.preprocess_ms
      << ", \"inference\": " << timing.inference_ms
      << ", \"postprocess\": " << timing.postprocess_ms
      << ", \"total\": " << timing.total_ms
      << ", \"fps\": " << timing.fps << "},\n  \"persons\": [\n";
  for (size_t i = 0; i < persons.size(); ++i) {
    const Detection& d = persons[i];
    out << "    {\"confidence\": " << d.confidence << ", \"x\": " << d.x
        << ", \"y\": " << d.y << ", \"w\": " << d.w
        << ", \"h\": " << d.h << '}';
    if (i + 1 != persons.size()) out << ',';
    out << '\n';
  }
  out << "  ]\n}\n";
  out.close();
  if (!out || std::rename(temp.c_str(), path.c_str()) != 0)
    throw std::runtime_error("cannot publish complete JSON: " + path);
}

int Run(const Options& opt) {
  std::signal(SIGINT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);

  std::unique_ptr<eagleeye::CameraPipeline> camera;
  std::unique_ptr<AppLock> lock;
  std::vector<uint8_t> offline;
  if (opt.raw.empty()) {
    if (!opt.attach_existing_video)
      throw std::runtime_error("use run_board.sh; direct live inference requires --attach-existing-video");
    lock.reset(new AppLock());
    eagleeye::HardwareOptions hardware;
    hardware.devmem = opt.devmem;
    hardware.i2c = opt.i2c;
    hardware.initialize_sensor = opt.initialize_sensor;
    hardware.initialize_display = opt.initialize_display;
    camera.reset(new eagleeye::CameraPipeline(hardware));
    if (opt.attach_existing_video) {
      camera->ValidateAttached();
      const eagleeye::PipelineStatus state = camera->GetStatus();
      std::cout << "HW attached_existing_video s2mm_status=0x" << std::hex
                << state.s2mm_status << " mm2s_status=0x" << state.mm2s_status
                << std::dec << '\n';
    } else {
      camera->Initialize();
    }
  } else {
    if (opt.fovea_roi || opt.mouse_roi)
      throw std::runtime_error("Fovea/mouse ROI requires a live camera");
    offline = ReadRaw(opt.raw);
    std::cout << "HW offline_raw=" << opt.raw << " bytes=" << offline.size() << '\n';
  }

  paddle::lite_api::MobileConfig config;
  config.set_model_from_file(opt.model);
  config.set_threads(opt.threads);
  config.set_power_mode(paddle::lite_api::LITE_POWER_HIGH);
  std::shared_ptr<paddle::lite_api::PaddlePredictor> predictor =
      paddle::lite_api::CreatePaddlePredictor<paddle::lite_api::MobileConfig>(config);
  if (!predictor) throw std::runtime_error("cannot create Paddle-Lite predictor");

  std::unique_ptr<FpgaPreprocessor> fpga_preprocessor;
  if (opt.preprocess != "cpu") {
    try {
      fpga_preprocessor.reset(new FpgaPreprocessor(opt.devmem));
    } catch (const std::exception& error) {
      if (opt.preprocess == "fpga") throw;
      std::cout << "PREPROC_FALLBACK initialization=" << error.what() << '\n';
    }
  }

  std::unique_ptr<eagleeye::RoiAnimator> fovea;
  eagleeye::GridPolicy roi_policy;
  if (opt.fovea_roi) {
    fovea.reset(new eagleeye::RoiAnimator(opt.devmem));
  }
  std::unique_ptr<eagleeye::MouseRoiController> mouse_roi;
  if (opt.mouse_roi) {
    mouse_roi.reset(new eagleeye::MouseRoiController(
        opt.devmem, opt.mouse_device, opt.mouse_sensitivity));
  }

  std::vector<float> warmup_input(static_cast<size_t>(3) * 320 * 320, 0.f);
  SetInputs(predictor.get(), warmup_input);
  for (int i = 0; i < opt.warmup; ++i) predictor->Run();

  std::vector<uint8_t> snapshot(eagleeye::kActiveFrameBytes);
  std::vector<float> nchw;
  bool dumped = false;
  int sequence = 0;
  uint32_t last_hash = 0;
  unsigned unchanged_content = 0;
  while (!g_stop.load() && (opt.iterations == 0 || sequence < opt.iterations)) {
    const auto total_begin = Clock::now();
    const auto capture_begin = total_begin;
    if (camera) {
      camera->CaptureLatest(&snapshot, opt.bgr);
    } else {
      std::copy(offline.begin(), offline.end(), snapshot.begin());
      if (opt.bgr) {
        for (size_t i = 0; i < snapshot.size(); i += 3)
          std::swap(snapshot[i], snapshot[i + 2]);
      }
    }
    const auto capture_end = Clock::now();
    eagleeye::SnapshotInfo snapshot_info;
    if (camera) {
      snapshot_info = camera->GetSnapshotInfo();
      const bool changed = sequence == 0 || snapshot_info.hash != last_hash;
      unchanged_content = changed ? 0 : unchanged_content + 1;
      last_hash = snapshot_info.hash;
      std::cout << "SNAPSHOT inference_sequence=" << sequence + 1
                << " slot=" << snapshot_info.slot << " attempts=" << snapshot_info.attempts
                << " content_hash=0x" << std::hex << snapshot_info.hash << std::dec
                << " content_changed=" << changed << " freshness=unverified\n";
      if (unchanged_content && unchanged_content % 10 == 0)
        std::cout << "WARNING unchanged content for " << unchanged_content
                  << " inferences: static scene OR stale capture; frame sequence not proven\n";
    }

    if (!dumped && !opt.dump_first.empty()) {
      WritePpm(opt.dump_first, snapshot);
      std::cout << "HW first_snapshot=" << opt.dump_first << '\n';
      dumped = true;
    }

    const auto preprocess_begin = Clock::now();
    uint32_t preproc_cycles = 0;
    bool fpga_preprocess_used = false;
    if (fpga_preprocessor) {
      try {
        fpga_preprocessor->Run(snapshot.data(), &nchw, &preproc_cycles);
        fpga_preprocess_used = true;
      } catch (const std::exception& error) {
        if (opt.preprocess == "fpga") throw;
        std::cout << "PREPROC_FALLBACK runtime=" << error.what() << '\n';
        fpga_preprocessor.reset();
      }
    }
    if (!fpga_preprocess_used) ResizeNormalizeNchw(snapshot.data(), &nchw);
    SetInputs(predictor.get(), nchw);
    const auto preprocess_end = Clock::now();
    const auto inference_begin = preprocess_end;
    predictor->Run();
    const auto inference_end = Clock::now();
    const auto postprocess_begin = inference_end;
    const std::vector<Detection> persons = GetPersons(predictor.get(), opt.threshold);
    const auto postprocess_end = Clock::now();

    Timing timing;
    timing.capture_ms = Ms(capture_begin, capture_end);
    timing.preprocess_ms = Ms(preprocess_begin, preprocess_end);
    timing.inference_ms = Ms(inference_begin, inference_end);
    timing.postprocess_ms = Ms(postprocess_begin, postprocess_end);
    timing.total_ms = Ms(total_begin, postprocess_end);
    timing.fps = timing.total_ms > 0 ? 1000.0 / timing.total_ms : 0.0;
    ++sequence;

    if (fovea) {
      fovea->Check();
      std::vector<eagleeye::PersonBox> boxes;
      boxes.reserve(persons.size());
      for (const Detection& d : persons)
        boxes.push_back({d.confidence, d.x, d.y, d.w, d.h});
      const eagleeye::GridSelection grid = roi_policy.Update(boxes);
      if (grid.valid) {
        std::cout << "GRID inference_sequence=" << sequence
                  << " row=" << unsigned(grid.row) << " col=" << unsigned(grid.col)
                  << " players=" << grid.players
                  << " accepted_players=" << grid.accepted_players
                  << " switched=" << grid.switched << " target_roi=" << grid.roi.x << ',' << grid.roi.y << '\n';
        fovea->SetTarget(grid.roi);
      } else {
        std::cout << "GRID inference_sequence=" << sequence
                  << " no_player hold_roi=" << roi_policy.Current().x << ',' << roi_policy.Current().y << '\n';
      }
    }

    std::cout << std::fixed << std::setprecision(4);
    for (const Detection& d : persons) {
      std::cout << "PERSON confidence=" << d.confidence
                << " x=" << d.x << " y=" << d.y
                << " w=" << d.w << " h=" << d.h << '\n';
    }
    if (persons.empty()) {
      std::cout << "PERSON none_above_threshold=" << opt.threshold << '\n';
    }
    std::cout << std::setprecision(3)
              << "PREPROC backend=" << (fpga_preprocess_used ? "fpga" : "cpu")
              << " cycles=" << preproc_cycles << '\n'
              << "TIME capture_ms=" << timing.capture_ms
              << " preprocess_ms=" << timing.preprocess_ms
              << " inference_ms=" << timing.inference_ms
              << " postprocess_ms=" << timing.postprocess_ms
              << " total_ms=" << timing.total_ms
              << " FPS=" << timing.fps << '\n' << std::flush;
    if (!opt.json_output.empty()) {
      WriteJson(opt.json_output, sequence, timing, persons, camera ? &snapshot_info : nullptr);
    }
  }
  if (fovea) fovea->Check();
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  Options options;
  try {
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
      Usage(argv[0]);
      return 0;
    }
    if (!ParseArgs(argc, argv, &options)) return 1;
    return Run(options);
  } catch (const std::exception& error) {
    std::cerr << "ERROR " << error.what() << '\n';
    return 2;
  }
}
