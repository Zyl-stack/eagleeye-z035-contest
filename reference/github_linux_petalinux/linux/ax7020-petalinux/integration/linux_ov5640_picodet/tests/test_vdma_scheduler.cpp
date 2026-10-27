#include "vdma_frame_scheduler.hpp"

#include <assert.h>

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

using eagleeye::VdmaFrameScheduler;

int main() {
  VdmaFrameScheduler scheduler;
  assert(scheduler.sequence() == 0);
  assert(scheduler.PinLatest(0) == -1);
  assert(scheduler.ChooseNext(2, 2) == 1);
  assert(scheduler.ChooseNext(31, 2) == -1);
  assert(scheduler.BeginHandoff(1));
  assert(!scheduler.BeginHandoff(1));
  assert(!scheduler.PublishAfterWriteMoved(0, 0));
  assert(!scheduler.PublishAfterWriteMoved(1, 2));
  assert(scheduler.PublishAfterWriteMoved(0, 1));
  assert(scheduler.sequence() == 1 && scheduler.ready() == 0);
  assert(scheduler.PinLatest(0) == 0);
  assert(scheduler.ChooseNext(2, 2) == -1);
  assert(!scheduler.PublishAfterWriteMoved(1, 0));
  scheduler.Unpin();
  assert(scheduler.ChooseNext(2, 2) == 0);
  assert(scheduler.BeginHandoff(0));
  assert(!scheduler.PublishAfterWriteMoved(1, 1));
  assert(scheduler.PublishAfterWriteMoved(1, 0));
  assert(scheduler.PinLatest(1) == 1);
  scheduler.Unpin();
  assert(scheduler.PinLatest(2) == -1);

  std::mutex mutex;
  std::condition_variable ready;
  VdmaFrameScheduler concurrent;
  std::vector<uint64_t> delivered;
  bool stopped = false;
  std::thread consumer([&] {
    uint64_t last = 0;
    while (last < 100) {
      std::unique_lock<std::mutex> lock(mutex);
      const bool notified = ready.wait_for(lock, std::chrono::seconds(2), [&] {
        return concurrent.sequence() > last || stopped;
      });
      assert(notified && !stopped);
      const int slot = concurrent.PinLatest(last);
      assert(slot >= 0 && slot != concurrent.writing());
      last = concurrent.sequence();
      delivered.push_back(last);
      concurrent.Unpin();
    }
  });
  for (int frame = 0; frame < 100; ++frame) {
    {
      std::lock_guard<std::mutex> lock(mutex);
      const int complete = concurrent.writing();
      const int next = concurrent.ChooseNext(2, 2);
      assert(next >= 0);
      assert(concurrent.BeginHandoff(next));
      assert(!concurrent.PublishAfterWriteMoved(complete, complete));
      assert(concurrent.PublishAfterWriteMoved(complete, next));
    }
    ready.notify_one();
    while (true) {
      std::lock_guard<std::mutex> lock(mutex);
      if (concurrent.sequence() == delivered.size()) break;
      std::this_thread::yield();
    }
  }
  consumer.join();
  assert(delivered.size() == 100);
  for (uint64_t frame = 1; frame <= 100; ++frame) {
    assert(delivered[frame - 1] == frame);
  }
  return 0;
}
