#pragma once

#include <event2/event.h>
#include <event2/thread.h>

#include <condition_variable>
#include <functional>
#include <queue>

namespace tl {

// event_base_dispatch wrapper
// accept callbacks to run inside the dispatch loop.
class Dispatcher {
 public:
  Dispatcher();
  ~Dispatcher();

  // start dispatch loop.
  void dispatch();
  // stop dispatch loop.
  void stop();
  // wait dispatch loop to exit after call stop().
  void join();

  // Commit a function to be called inside the dispatch loop.
  // It will be called in the callback of "ev_timer_".
  template <class F, class... Args>
  void post(F&& f, Args&&... args);

  void timerCB();

  event_base* ev_base() { return ev_base_; }

 private:
  struct event_base* ev_base_ = nullptr;
  struct event* ev_timer_ = nullptr;
  std::queue<std::function<void()> > post_callbacks_;
  bool stop_ = false;
  std::mutex mu_;
  std::condition_variable cond_;
};

template <class F, class... Args>
void Dispatcher::post(F&& f, Args&&... args) {
  bool do_post = false;
  {
    std::unique_lock<std::mutex> lock(mu_);
    do_post = post_callbacks_.empty();
    post_callbacks_.push(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  }

  if (do_post) {
    event_active(ev_timer_, 0, 0);
  }
}

}  // namespace tl
