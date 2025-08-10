#pragma once

#include "common.hpp"
#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class SafeQueue {
public:
  SafeQueue() = default;

  template <typename TCallback> void addTask(TCallback &&callback) {
    std::lock_guard<std::mutex> lock_guard(mtx);
    tasks.push(std::forward<TCallback>(callback));
    ++remaining_tasks;
  }

  void getTask(std::function<void()> &target_callback) {
    std::lock_guard<std::mutex> lock_guard(mutex);
    if (tasks.empty())
      return;
    target_callback = std::move(tasks.front());
    tasks.pop();
  }

  static void wait() { std::this_thread::yield(); }

  void waitUntilComplete() const {
    while (remaining_tasks > 0)
      wait();
  }

  void taskDone() { --remaining_tasks; }

private:
  std::queue<std::function<void()>> tasks;
  std::mutex mtx;
  std::atomic<uint32_t> remaining_tasks = 0;
};
