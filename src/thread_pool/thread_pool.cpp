#include "thread_pool/thread_pool.hpp"
#include <atomic>

namespace tp {

ThreadPool::ThreadPool(int32_t threadCount) : running(true) {
  threadCount = max(threadCount, 1);
  startWorkers(m_threadCount);
}

ThreadPool::~ThreadPool() {
  // if object is deleted, finish tasks and signal threads to stop
  waitIdle();
  stop();
}

void ThreadPool::startWorkers(uint32_t n) {
  workers.reserve(n);
  for (uint32_t i = 0; i < n; ++i) {
    // pass lambda callable to thread
    workers.emplace_back([this] {
      while (running.load(std::memory_order_relaxed)) {
        std::function<void()> task;
        tasks.getTask(task);
        if (task) {
          task();
          tasks.taskDone();
        } else {
          SafeQueue::wait();
        }
      }
    });
  }
}

void ThreadPool::waitIdle() { tasks.waitUntilComplete() }

void ThreadPool::stop() {
  bool stopped = true;
  if (running.compare_exchange_strong(stopped, false,
                                      std::memory_order_relaxed)) {
    for (auto &th : m_workers) {
      // join threads
      if (th.joinable())
        th.join();
    }
    m_workers.clear();
  }
}

} // namespace tp
