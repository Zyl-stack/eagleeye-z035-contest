#include "fovea_control.hpp"
#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
  if (argc != 3) return 3;
  try {
    eagleeye::FoveaControl hardware(argv[1]);
    hardware.Commit({100, 50}, 7, std::atoi(argv[2]));
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 2;
  }
}
