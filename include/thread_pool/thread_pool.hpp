#pragma once

#include "common.hpp"
#include "safe_queue.hpp"

namespace tp {
class ThreadPool {
  // https://medium.com/@bhushanrane1992/getting-started-with-c-thread-pool-b6d1102da99a
public:
  ThreadPool(uint32_t numThreads) {}

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ~ThreadPool();

  template <typename TCallback> void addTask(TCallback &&callback) {
    tasks.addTask(std::function<void()>(std::forward<TCallback>(cb)));
  }

  void waitIdle();

  uint32_t threadCount() const noexcept { return numThreads; }

  void stop();

private:
  void startWorkers(uint32_t n);
  vec<std::thread> workers;
  SafeQueue tasks;
  std::atomic<bool> running = false;
  uint32_t numThreads;
};

}; // namespace tp
