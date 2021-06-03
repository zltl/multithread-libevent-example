#pragma once

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/tcp.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "spdlog/spdlog.h"
#include <string>

#include "buffer.h"
#include "dispatcher.h"
#include "event2/event.h"
#include "event2/util.h"

namespace tl {

class Handler {
 public:
  Handler(Dispatcher* disp, int fd);
  ~Handler();

  int handleRead();
  int handleWrite();

  int fd() { return fd_; }

 private:
  int fd_ = -1;
  event* ev_ = nullptr;
  Dispatcher* disp_;
  buffer read_buf_;
  buffer write_buf_;
};

}  // namespace tl
