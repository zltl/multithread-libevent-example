#include "dispatcher.h"

#include <cassert>

namespace tl {

extern "C" void dispatcherTimerCB(int, short, void* ptr) {
  auto dispather = (Dispatcher*)ptr;
  dispather->timerCB();
}

Dispatcher::Dispatcher() {}

Dispatcher::~Dispatcher() {
  event_free(ev_timer_);
  event_base_free(ev_base_);
}

void Dispatcher::dispatch() {
  {
    std::unique_lock<std::mutex> lock(mu_);
    assert(stop_);
    ev_base_ = event_base_new();
    ev_timer_ = event_new(ev_base_, -1, EV_PERSIST, dispatcherTimerCB, this);
    stop_ = false;
  }

  event_base_loop(ev_base_, EVLOOP_NO_EXIT_ON_EMPTY);

  // notify join().
  cond_.notify_all();
}

void Dispatcher::stop() {
  post([&] { event_base_loopbreak(ev_base_); });
}

void Dispatcher::join() {
  std::unique_lock<std::mutex> lock(mu_);
  cond_.wait(lock);
}

void Dispatcher::timerCB() {
  std::queue<std::function<void()> > cbs;

  {
    std::unique_lock<std::mutex> lock(mu_);
    cbs = std::move(post_callbacks_);
    assert(post_callbacks_.empty());
  }

  while (!cbs.empty()) {
    cbs.front()();
    cbs.pop();
  }
}

}  // namespace tl
