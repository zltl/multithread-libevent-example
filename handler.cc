#include "handler.h"

#include "spdlog/spdlog.h"

namespace tl {

extern "C" void handler_event_cb(evutil_socket_t, short what, void* ptr) {
  Handler* h = (Handler*)ptr;
  int r = 0;
  if (what & EV_TIMEOUT) {
    SPDLOG_ERROR("fd={}, timeout", h->fd());
    delete h;
    return;
  }

  if (what & EV_READ) {
    r = h->handleRead();
  }
  if (r != 0) {
    SPDLOG_ERROR("fd={}, read error, errno={} {}", h->fd(), errno,
                 strerror(errno));
    delete h;
    return;
  }
  if (what & EV_WRITE) {
    r = h->handleWrite();
  }
  if (r != 0) {
    SPDLOG_ERROR("fd={}, write error, errno={} {}", h->fd(), errno,
                 strerror(errno));
    delete h;
    return;
  }
}

Handler::Handler(Dispatcher* disp, int fd) {
  disp_ = disp;
  fd_ = fd;
  if (evutil_make_socket_nonblocking(fd_) < 0) {
    SPDLOG_ERROR("evutil_make_socket_nonblocking errno={} {}", errno,
                 strerror(errno));
    close(fd);
    throw std::runtime_error("evutil_make_socket_nonblocking");
  }
  ev_ = event_new(disp->ev_base(), fd_, EV_READ, handler_event_cb, this);
  timeval tv;
  tv.tv_sec = 60;  // 超时
  tv.tv_usec = 0;
  event_add(ev_, &tv);
}

Handler::~Handler() {
  if (ev_) {
    event_del(ev_);
    event_free(ev_);
    ev_ = NULL;
  }
  if (fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
}

int Handler::handleWrite() {
  ssize_t n = 0;

  while (write_buf_.size()) {
    const void* buf;
    std::size_t len;
    write_buf_.dataChunk(buf, len);
    n = write(fd_, buf, len);
    if (n <= 0) {
      if (errno == EAGAIN || errno == EINTR) {
        return 0;
      }
      return -1;
    }
    write_buf_.drain(n);
  }

  struct timeval tv;
  tv.tv_sec = 10;
  tv.tv_usec = 0;
  short what = 0;
  if (write_buf_.size()) {
    what = EV_WRITE;
  } else {
    what = EV_READ;
  }
  event_assign(ev_, disp_->ev_base(), fd_, what, handler_event_cb, this);
  event_add(ev_, &tv);

  return 0;
}

int Handler::handleRead() {
  ssize_t n = 0;

  // read
  while (true) {
    void* buf;
    std::size_t len;
    read_buf_.spaceChunk(buf, len);
    n = recv(fd_, buf, len, 0);
    if (n == 0) {
      return -1;  // closed
    }
    if (n < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
        // continue
        break;
      } else {
        return -1;  // error
      }
    }

    read_buf_.spaceHaveSeted(n);
    if (n < (ssize_t)len) {
      break;
    }    
  }

  // echo back
  while (read_buf_.size()) {
    const void* buf;
    std::size_t len;
    read_buf_.dataChunk(buf, len);
    write_buf_.push(buf, len);
    read_buf_.drain(len);
  }
  struct timeval tv;
  tv.tv_sec = 10;
  tv.tv_usec = 0;
  short what = 0;
  if (write_buf_.size()) {
    what = EV_WRITE;
  } else {
    what = EV_READ;
  }
  event_assign(ev_, disp_->ev_base(), fd_, what, handler_event_cb, this);
  event_add(ev_, &tv);
  return 0;
}

}  // namespace tl
