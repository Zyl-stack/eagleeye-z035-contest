#include "fovea_roi.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  if (argc != 2) return 2;
  std::ifstream in(argv[1]);
  if (!in) return 2;
  eagleeye::GridPolicy policy;
  std::vector<eagleeye::PersonBox> boxes;
  std::string line;
  unsigned frames = 0, switches = 0, roi_commits = 0;
  auto displayed = policy.Current();
  while (std::getline(in, line)) {
    eagleeye::PersonBox box{};
    if (std::sscanf(line.c_str(), "PERSON confidence=%f x=%f y=%f w=%f h=%f",
                    &box.confidence, &box.x, &box.y, &box.w, &box.h) == 5) {
      boxes.push_back(box);
    } else if (line.compare(0, 5, "TIME ") == 0) {
      auto result = policy.Update(boxes);
      if (result.switched) ++switches;
      if (result.valid &&
          (result.roi.x != displayed.x || result.roi.y != displayed.y)) {
        displayed = result.roi;
        ++roi_commits;
      }
      ++frames;
      boxes.clear();
    }
  }
  if (frames != 20) return 3;
  std::cout << "replayed_frames=" << frames << " grid_switches=" << switches
            << " moving_roi_commits=" << roi_commits << '\n';
}
