#pragma once

#include "common.hpp"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <utility>

class SafeQueue {
public:
  SafeQueue() = default;

  template <typename TCallback> bool addTask(TCallback &&callback) {
    auto func = std::function<void()>(std::forward<TCallback>(callback));
    {
      std::lock_guard<std::mutex> lock_guard(mtx_);
      if (closed_)
        return false;
      tasks.push(std::move(func));
      ++remaining_;
    }
    cv_tasks_.notify_one();
    return true;
  }

  // worker-facing blocking pop.
  bool getTask(std::function<void()> &target_callback) {
    std::unique_lock<std::mutex> unique_lock(mtx_);
    cv_tasks_.wait(unique_lock, [&] { return closed_ || !tasks.empty(); });
    if (tasks.empty())
      return false;
    target_callback = std::move(tasks.front());
    tasks.pop();
    return true;
  }

  // non-blocking pop
  bool tryGetTask(std::function<void()> &out) {
    std::lock_guard<std::mutex> lk(mtx_);
    if (tasks.empty())
      return false;
    out = std::move(tasks.front());
    tasks.pop();
    return true;
  }

  void waitUntilComplete() {
    std::unique_lock<std::mutex> unique_lock(mtx_);
    cv_done_.wait(unique_lock, [&] { return remaining_ == 0; });
  }

  // wait for either new available tasks OR all work done
  void waitUntilWorkOrComplete() {
    std::unique_lock<std::mutex> lk(mtx_);
    cv_tasks_.wait(
        lk, [&] { return remaining_ == 0 || !tasks.empty() || closed_; });
  }

  void close() {
    {
      std::lock_guard<std::mutex> lock_guard(mtx_);
      closed_ = true;
    }
    cv_tasks_.notify_all();
  }

  bool isComplete() {
    std::lock_guard<std::mutex> lk(mtx_);
    return remaining_ == 0;
  }

  // wake waiters in both paths when work hits zero
  void taskDone() {
    bool notify_done = false;
    {
      std::lock_guard<std::mutex> lk(mtx_);
      notify_done = (--remaining_ == 0);
    }
    if (notify_done) {
      cv_done_.notify_all();
      cv_tasks_.notify_all();
    }
  }

private:
  std::queue<std::function<void()>> tasks;
  std::mutex mtx_;
  std::condition_variable cv_tasks_;
  std::condition_variable cv_done_;
  // guarded by mutex
  bool closed_ = false;
  size_t remaining_ = 0;
};
