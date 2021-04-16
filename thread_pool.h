#pragma once

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
  ThreadPool(size_t num_threads);

  template <class F, class... Args>
  std::future<typename std::result_of<F(Args...)>::type> post(F&& f,
                                                              Args&&... args);

  ~ThreadPool();

 private:
  void loop();

  std::vector<std::thread> threads_;
  std::queue<std::function<void()> > tasks_;
  std::mutex tasks_mutex_;
  std::condition_variable condition_;
  bool stop_;
};

template <class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type> ThreadPool::post(
    F&& f, Args&&... args) {
  using ReturnT = typename std::result_of<F(Args...)>::type;

  auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  std::future<ReturnT> res = task->get_future();

  {
    std::unique_lock<std::mutex> lock(tasks_mutex_);
    tasks_.emplace([task] { task(); });
  }

  condition_.notify_one();

  return res;
}

}  // namespace tl
