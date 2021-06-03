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

#include "dispatcher.h"
#include "event2/event.h"
#include "event2/util.h"
#include "handler.h"

namespace tl {

class Listener {
 public:
  Listener(const std::string& addr, int port) : addr_(addr), port_(port) {}
  ~Listener();

  int open(Dispatcher* disp,
           std::function<void(Dispatcher* disp, int fd)> handle);

  int doAccept();

 private:
  std::string addr_;
  int port_;
  int fd_ = -1;
  event* ev_ = NULL;
  Dispatcher* disp_;
  std::function<void(Dispatcher* disp, int fd)> handle_;
};

}  // namespace tl
