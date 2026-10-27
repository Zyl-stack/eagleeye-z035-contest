#include "fovea_control.hpp"
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: fovea_ctl status|direct|lanczos\n";
    return 2;
  }
  try {
    eagleeye::FoveaControl fovea;
    const std::string action = argv[1];
    if (action == "status") {
      const uint32_t status = fovea.Status();
      const eagleeye::RoiOrigin active = fovea.ActiveOrigin();
      std::cout << "FOVEA status=0x" << std::hex << status << std::dec
                << " paused=" << ((status >> 3) & 1U)
                << " active_roi=" << active.x << ',' << active.y << '\n';
    } else if (action == "direct") {
      fovea.Commit({160, 90}, 0, 0);
    } else if (action == "lanczos") {
      fovea.Commit({160, 90}, 0, 2);
    } else {
      std::cerr << "Unknown action\n";
      return 2;
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "ERROR " << error.what() << '\n';
    return 1;
  }
}
