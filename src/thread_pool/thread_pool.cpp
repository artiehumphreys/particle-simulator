#include "common.hpp"
#include "thread_pool/thread_pool.hpp"
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

namespace tp {

static inline uint32_t clamp_threads(uint32_t n) {
  if (n <= 0) {
    const uint32_t hc = std::thread::hardware_concurrency();
    n = hc ? hc : 1u;
  }
  return std::max(1u, n);
}

ThreadPool::ThreadPool(int32_t threadCount) {
  startWorkers(clamp_threads(threadCount));
}

ThreadPool::~ThreadPool() {
  // if object is deleted, finish tasks and signal threads to stop
  waitIdle();
  stop();
}

void ThreadPool::startWorkers(uint32_t n) {
  workers_.reserve(n);
  running_.store(true, std::memory_order_release);
  for (uint32_t i = 0; i < n; ++i) {
    // pass lambda callable to thread
    workers_.emplace_back([this] {
      std::function<void()> task;
      for (;;) {
        // use getTask to park workers until work is available
        if (!tasks_.getTask(task))
          break;
        try {
          task();
        } catch (...) {
          std::cout << "Cannot process task. Continuing." << '\n';
          continue;
        }
        tasks_.taskDone();
      }
    });
  }
}

void ThreadPool::waitIdle() {
  std::function<void()> task;
  for (;;) {
    // use tryGetTask to not block new tasks from entering the queue
    while (tasks_.tryGetTask(task)) {
      // ensuring taskDone runs on scope exit
      struct Done {
        SafeQueue *q;
        ~Done() noexcept { q->taskDone(); }
      } done{&tasks_};
      try {
        task();
      } catch (...) {
      }
    }
    if (tasks_.isComplete())
      return;
    tasks_.waitUntilWorkOrComplete();
  }
}

void ThreadPool::stop() {
  if (!running_.exchange(false, std::memory_order_acq_rel))
    return;
  tasks_.close();
  for (auto &th : workers_) {
    // join threads
    if (th.joinable())
      th.join();
  }
  workers_.clear();
}

} // namespace tp
