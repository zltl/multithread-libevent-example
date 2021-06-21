#include "thread_pool.h"

#include <cassert>
#include <functional>

#include "spdlog/spdlog.h"

namespace tl {

ThreadPool::ThreadPool(size_t num_threads, int priority_count) : stop_(false) {
  assert(priority_count > 0);
  for (size_t i = 0; i < num_threads; i++) {
    threads_.emplace_back([this] { this->loop(); });
  }
  tasks_.reserve(priority_count);
  for (int i = 0; i < priority_count; i++) {
    tasks_.push_back(std::queue<std::function<void()>>());
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
    bool got_task = false;
    std::function<void()> task;
    {
      // wait for new task or stop signal
      std::unique_lock<std::mutex> lock(tasks_mutex_);
      condition_.wait(lock, [this] {
        bool allempty = true;
        for (auto tp : this->tasks_) {
          if (!tp.empty()) {
            allempty = false;
            break;
          }
        }
        return this->stop_ || !allempty;
      });

      // exit loop when stop and no more tasks
      if (stop_ && tasks_.empty()) return;

      for (auto& task_list : tasks_) {
        if (task_list.empty()) {
          continue;
        }
        task = std::move(task_list.front());
        task_list.pop();
        got_task = true;
        break;
      }
    }
    if (got_task) task();
  }
}

bool ThreadPool::stop() {
  std::unique_lock<std::mutex> lock(tasks_mutex_);
  return stop_;
}

}  // namespace tl
