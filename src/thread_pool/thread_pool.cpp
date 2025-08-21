#include "common.hpp"
#include "thread_pool/thread_pool.hpp"

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

void ThreadPool::waitIdle() { tasks_.waitUntilComplete(); }

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
