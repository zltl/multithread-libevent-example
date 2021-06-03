#pragma once

#include <cassert>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace tl {

class ThreadPool {
 public:
  ThreadPool(size_t num_threads, int priority_count = 1);

  template <class F, class... Args>
  std::future<typename std::result_of<F(Args...)>::type> post(F&& f,
                                                              Args&&... args);

  template <class F, class... Args>
  std::future<typename std::result_of<F(Args...)>::type> postPriority(
      int level, F&& f, Args&&... args);

  ~ThreadPool();

  bool stop();

 private:
  void loop();

  std::vector<std::thread> threads_;
  // 任务根据优先级先执行 0->1->2->3
  std::vector<std::queue<std::function<void()>>> tasks_;
  std::mutex tasks_mutex_;
  std::condition_variable condition_;
  bool stop_;
};

template <class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type> ThreadPool::post(
    F&& f, Args&&... args) {
  return postPriority(0, f, args...);
}

template <class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type> ThreadPool::postPriority(
    int priority, F&& f, Args&&... args) {
  assert(priority < (int)tasks_.size());
  assert(priority >= 0);

  using ReturnT = typename std::result_of<F(Args...)>::type;

  auto task = std::make_shared<std::packaged_task<ReturnT()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  std::future<ReturnT> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(tasks_mutex_);
    tasks_[priority].push([task]() { (*task)(); });
  }

  condition_.notify_one();

  return res;
}

}  // namespace tl
