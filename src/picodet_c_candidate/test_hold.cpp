#include "roi_animator.hpp"
#include <atomic>
#include <cassert>
#include <chrono>
#include <thread>
static std::atomic<int> commits{0};
static std::atomic<bool> in_commit{false};
namespace eagleeye {
FoveaControl::FoveaControl(const std::string&) : base_(nullptr), fd_(-1) {}
FoveaControl::~FoveaControl() {}
bool FoveaControl::Paused() const { return false; }
void FoveaControl::Commit(RoiOrigin, uint32_t id, uint32_t) {
  if (id != 0) {
    in_commit.store(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    commits.fetch_add(1);
    in_commit.store(false);
  } else commits.fetch_add(1);
}
}
int main() {
  eagleeye::RoiAnimator animator("fake");
  animator.SetTarget({300,170});
  for (int i=0; i<100 && !in_commit.load(); ++i) std::this_thread::sleep_for(std::chrono::milliseconds(5));
  assert(in_commit.load());
  animator.Hold();
  assert(!in_commit.load());
  const int held = commits.load();
  std::this_thread::sleep_for(std::chrono::milliseconds(180));
  assert(commits.load() == held);
}