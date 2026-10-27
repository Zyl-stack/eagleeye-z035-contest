#pragma once

#include "frame_layout.h"
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace eagleeye {
inline std::vector<uint8_t> ReadProperty(const std::string& path) {
  std::ifstream f(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

inline uint64_t BeValue(const std::vector<uint8_t>& bytes, size_t offset, size_t length) {
  if (length > 8 || offset > bytes.size() || length > bytes.size() - offset)
    throw std::runtime_error("invalid device-tree cells");
  uint64_t v = 0;
  for (size_t i = 0; i < length; ++i) v = (v << 8) | bytes[offset + i];
  return v;
}

inline void RequireReservation(const std::string& root = "/sys/firmware/devicetree/base") {
  const std::string reserved = root + "/reserved-memory";
  auto ac = ReadProperty(reserved + "/#address-cells");
  auto sc = ReadProperty(reserved + "/#size-cells");
  if (ac.empty()) ac = ReadProperty(root + "/#address-cells");
  if (sc.empty()) sc = ReadProperty(root + "/#size-cells");
  const uint64_t a = BeValue(ac, 0, 4), s = BeValue(sc, 0, 4);
  if (a < 1 || a > 2 || s < 1 || s > 2) throw std::runtime_error("unsupported reserved-memory cells");
  // The delivered AX7020 device tree names this node explicitly.
  const std::string path = reserved + "/framebuf@1e000000";
  bool found = false;
  const int no_map = open((path + "/no-map").c_str(), O_RDONLY);
  if (no_map >= 0) {
    close(no_map);
    const auto status = ReadProperty(path + "/status");
    if (status.empty() || std::string(status.begin(), status.end()).find("disabled") != 0) {
      const auto reg = ReadProperty(path + "/reg");
      const size_t tuple = static_cast<size_t>((a + s) * 4);
      if (!reg.empty() && reg.size() % tuple == 0) {
        for (size_t offset = 0; offset < reg.size(); offset += tuple) {
          const uint64_t base = BeValue(reg, offset, a * 4);
          const uint64_t size = BeValue(reg, offset + a * 4, s * 4);
          if (base <= kFrameBufferBase && size >= kFrameBufferBytes &&
              kFrameBufferBase - base <= size - kFrameBufferBytes) found = true;
        }
      }
    }
  }
  if (!found) throw std::runtime_error("0x1e000000..0x1e20ffff no-map reservation not found");
}
}  // namespace eagleeye
