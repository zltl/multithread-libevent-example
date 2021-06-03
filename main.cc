

#include <signal.h>

#include <memory>

#include "dispatcher.h"
#include "event2/event.h"
#include "event2/thread.h"
#include "handler.h"
#include "listener.h"
#include "spdlog/spdlog.h"
#include "thread_pool.h"

#define MAX_IO_THREAD_COUNT 4

int main() {
  signal(SIGPIPE, SIG_IGN);
  evthread_use_pthreads();

  spdlog::set_pattern("[%Y-%m-%dT%H:%M:%S.%e%z] [%l] [%!(%s#%#)] %v");
  spdlog::set_level(spdlog::level::trace);

  SPDLOG_INFO("starting...");

  tl::Dispatcher* disps = new tl::Dispatcher[MAX_IO_THREAD_COUNT];

  tl::ThreadPool* thread_pool(new tl::ThreadPool(MAX_IO_THREAD_COUNT));

  std::unique_ptr<tl::Listener> ls[4];

  for (int i = 0; i < MAX_IO_THREAD_COUNT; i++) {
    SPDLOG_INFO("staring thread {}", i);
    ls[i].reset(new tl::Listener("0.0.0.0", 2200));
    ls[i]->open(&disps[i], [](tl::Dispatcher* d, int fd) {
      tl::Handler* h = nullptr;
      try {
        h = new tl::Handler(d, fd);
      } catch (...) {
        if (h) delete h;
      }
    });
    thread_pool->post(
        [](tl::Dispatcher* disp) {
          SPDLOG_TRACE("dispatch()");
          disp->dispatch();
        },
        &disps[i]);
  }

  // wait join
  delete thread_pool;

  delete[] disps;
  return 0;
}
