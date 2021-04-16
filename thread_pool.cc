#include "thread_pool.h"

#include <functional>

namespace tl {

ThreadPool::ThreadPool(size_t num_threads): stop_(false) {
  for (size_t i = 0; i < num_threads; i++) {
    threads_.emplace_back([this] { this->loop(); });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(tasks_mutex_);
    stop_ = true;
  }
  condition_.notify_all();

  for (auto& t : threads_) t.join();
}

void ThreadPool::loop() {
  for (;;) {
    std::function<void()> task;
    {
      // wait for new task or stop signal
      std::unique_lock<std::mutex> lock(tasks_mutex_);
      condition_.wait(lock,
                      [this] { return this->stop_ || !this->tasks_.empty(); });

      // exit loop when stop and no more tasks
      if (stop_ && tasks_.empty()) return;

      task = std::move(tasks_.front());
      tasks_.pop();
    }

    task();
  }
}

}  // namespace tl
