#pragma once

#include "common.hpp"
#include "safe_queue.hpp"
#include <atomic>
#include <cstdint>
#include <functional>
#include <thread>

namespace tp {
class ThreadPool {
  // https://medium.com/@bhushanrane1992/getting-started-with-c-thread-pool-b6d1102da99a
public:
  explicit ThreadPool(int32_t numThreads);

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ~ThreadPool();

  template <typename TCallback> void addTask(TCallback &&callback) {
    tasks_.addTask(std::function<void()>(std::forward<TCallback>(callback)));
  }

  void waitIdle();

  uint32_t threadCount() const noexcept { return numThreads; }

  void stop();

private:
  void startWorkers(uint32_t n);
  std::vector<std::thread> workers_;
  SafeQueue tasks_;
  std::atomic<bool> running_ = false;
  uint32_t numThreads;
};

}; // namespace tp
