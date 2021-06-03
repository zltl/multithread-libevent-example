#include "listener.h"

namespace tl {

extern "C" void listener_event_cb(evutil_socket_t fd, short what, void* ptr) {
  SPDLOG_INFO("get fd {}", fd);
  (void)fd;
  Listener* ls = (Listener*)ptr;
  if (what & EV_READ) {
    ls->doAccept();
  }
}

Listener::~Listener() {
  if (ev_) {
    event_free(ev_);
  }
  if (fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
}

int Listener::open(Dispatcher* disp,
                   std::function<void(Dispatcher* disp, int fd)> handle) {
  struct sockaddr_in sockaddr;
  socklen_t socklen = sizeof(sockaddr);
  int opt = SO_REUSEADDR | SO_REUSEPORT;
  int one = 1;
  disp_ = disp;
  handle_ = handle;

  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ < 0) {
    SPDLOG_ERROR("socket() failed, errno={}, {}", errno, strerror(errno));
    return -1;
  }
  int r = setsockopt(fd_, SOL_SOCKET, opt, (const void*)&one, sizeof(one));
  if (r < 0) {
    SPDLOG_ERROR("setsockopt, errno=%d, %s", errno, strerror(errno));
    return -1;
  }
  if (evutil_make_socket_nonblocking(fd_) < 0) {
    SPDLOG_ERROR("evutil_make_socket_nonblocking errno={}, {}", errno,
                 strerror(errno));
    return -1;
  }
  memset(&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = inet_addr(addr_.c_str());
  sockaddr.sin_port = htons((unsigned short)port_);
  r = bind(fd_, (struct sockaddr*)&sockaddr, socklen);
  if (r < 0) {
    SPDLOG_ERROR("bind() errno={}, {}", errno, strerror(errno));
    return -1;
  }
  r = listen(fd_, 512);
  if (r < 0) {
    SPDLOG_ERROR("listen() errno={}, {}", errno, strerror(errno));
    return -1;
  }
  SPDLOG_INFO("listening {}:{}", addr_, port_);
  ev_ = event_new(disp->ev_base(), fd_, EV_READ | EV_PERSIST, listener_event_cb,
                  this);
  event_add(ev_, nullptr);
  return 0;
}

int Listener::doAccept() {
  int s;
  struct sockaddr_in sa;
  socklen_t slen = sizeof(sa);

  for (;;) {
    s = accept(fd_, (struct sockaddr*)&sa, &slen);
    if (s == -1) {
      return 0;
    }
    SPDLOG_DEBUG("accept %d", s);
    handle_(disp_, s);
  }
  return 0;
}

}  // namespace tl
