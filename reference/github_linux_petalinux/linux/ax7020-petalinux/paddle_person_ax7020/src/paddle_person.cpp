#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "eagleeye_hw.hpp"
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

uint32_t SnapshotChecksum(const std::vector<uint8_t>& frame) {
  uint32_t hash = 2166136261U;
  for (uint8_t byte : frame) hash = (hash ^ byte) * 16777619U;
  return hash;
}

void SignalHandler(int) { g_stop.store(true); }

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
      << "  --no-display-init       leave dynclk/VTC untouched\n";
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
    else if (arg == "--threads") opt->threads = std::max(1, std::atoi(value.c_str()));
    else if (arg == "--warmup") opt->warmup = std::max(0, std::atoi(value.c_str()));
    else if (arg == "--iterations") opt->iterations = std::max(0, std::atoi(value.c_str()));
    else if (arg == "--threshold") opt->threshold = std::strtof(value.c_str(), nullptr);
    else if (arg == "--pixel-order") {
      if (value != "rgb" && value != "bgr") {
        std::cerr << "--pixel-order must be rgb or bgr\n";
        return false;
      }
      opt->bgr = value == "bgr";
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
               const std::vector<Detection>& persons) {
  std::ofstream out(path.c_str());
  if (!out) throw std::runtime_error("cannot write JSON: " + path);
  out << std::fixed << std::setprecision(4)
      << "{\n  \"sequence\": " << sequence
      << ",\n  \"image_width\": 640,\n  \"image_height\": 360,\n"
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
}

int Run(const Options& opt) {
  std::signal(SIGINT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);

  std::unique_ptr<eagleeye::CameraPipeline> camera;
  std::vector<uint8_t> offline;
  if (opt.raw.empty()) {
    eagleeye::HardwareOptions hardware;
    hardware.devmem = opt.devmem;
    hardware.i2c = opt.i2c;
    hardware.initialize_sensor = opt.initialize_sensor;
    hardware.initialize_display = opt.initialize_display;
    camera.reset(new eagleeye::CameraPipeline(hardware));
    camera->Initialize();
  } else {
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

  std::vector<float> warmup_input(static_cast<size_t>(3) * 320 * 320, 0.f);
  SetInputs(predictor.get(), warmup_input);
  for (int i = 0; i < opt.warmup; ++i) predictor->Run();

  std::vector<uint8_t> snapshot(eagleeye::kActiveFrameBytes);
  std::vector<float> nchw;
  bool dumped = false;
  int sequence = 0;
  while (!g_stop.load() && (opt.iterations == 0 || sequence < opt.iterations)) {
    const auto total_begin = Clock::now();
    const auto capture_begin = total_begin;
    uint64_t capture_sequence = 0;
    if (camera) {
      camera->CaptureLatest(&snapshot, opt.bgr, &capture_sequence);
    } else {
      std::copy(offline.begin(), offline.end(), snapshot.begin());
      if (opt.bgr) {
        for (size_t i = 0; i < snapshot.size(); i += 3)
          std::swap(snapshot[i], snapshot[i + 2]);
      }
    }
    const auto capture_end = Clock::now();
    if (camera) {
      std::cout << "SNAPSHOT frame_sequence=" << capture_sequence
                << " checksum=0x" << std::hex << SnapshotChecksum(snapshot)
                << std::dec << '\n';
    }

    if (!dumped && !opt.dump_first.empty()) {
      WritePpm(opt.dump_first, snapshot);
      std::cout << "HW first_snapshot=" << opt.dump_first << '\n';
      dumped = true;
    }

    const auto preprocess_begin = capture_end;
    ResizeNormalizeNchw(snapshot.data(), &nchw);
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
              << "TIME capture_ms=" << timing.capture_ms
              << " preprocess_ms=" << timing.preprocess_ms
              << " inference_ms=" << timing.inference_ms
              << " postprocess_ms=" << timing.postprocess_ms
              << " total_ms=" << timing.total_ms
              << " FPS=" << timing.fps << '\n' << std::flush;
    if (!opt.json_output.empty()) {
      WriteJson(opt.json_output, sequence, timing, persons);
    }
  }
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  Options options;
  if (!ParseArgs(argc, argv, &options)) return argc > 1 ? 1 : 0;
  try {
    return Run(options);
  } catch (const std::exception& error) {
    std::cerr << "ERROR " << error.what() << '\n';
    return 2;
  }
}
