#include "buffer.h"
#include <algorithm>

namespace tl {

buffer::buffer() {}
buffer::~buffer() {}

std::size_t buffer::push(const void *data, const std::size_t len) {
  auto src_data = static_cast<const ElemType_ *>(data);
  auto it = chunk_list_.begin();
  auto left_len = len;
  // fill the first chunk.
  if (!chunk_list_.empty() && it->cap > it->end) {
    auto to_copy = std::min(it->cap - it->end, len);
    std::copy(src_data, src_data + to_copy, it->p + it->end);
    left_len -= to_copy;
    it->end += to_copy;
    src_data += to_copy;
    size_ += to_copy;
  }

  if (left_len == 0) {
    return len;
  }

  // create a new chunk and copy src_data[0..left_len].
  // If default_chunk_size < left_len, allocate a buffer containing enough free
  // space, else allocate a default size of chunk.
  auto chunk_size = default_chunk_size_;
  if (left_len > default_chunk_size_) {
    chunk_size = left_len;
  }
  Chunk_ new_chunk(chunk_size);
  if (new_chunk.p == nullptr) {
    return len - left_len;
  }
  std::copy(src_data, src_data + left_len, new_chunk.p);
  new_chunk.end = left_len;
  chunk_list_.push_front(new_chunk);
  new_chunk.p = nullptr;
  size_ += left_len;
  return len;
}

std::size_t buffer::drain(const std::size_t len) {
  std::size_t drain_len = 0;

  while (drain_len < len && !chunk_list_.empty()) {
    auto &ch = chunk_list_.back();
    if (len - drain_len >= ch.end - ch.offset) {
      drain_len += ch.end - ch.offset;
      chunk_list_.pop_back();
    } else {
      ch.offset += len - drain_len;
      size_ -= len;
      return len;
    }
  }

  size_ -= drain_len;
  return drain_len;
}

void buffer::data(const void *&data, const std::size_t len) {
  if (size_ < len) {
    data = nullptr;
    return;
  }

  auto &ch = chunk_list_.back();
  if (len <= ch.end - ch.offset) {
    data = static_cast<const void *>(ch.p + ch.offset);
    return;
  }

  Chunk_ new_chunk(len);
  if (new_chunk.p == nullptr) {
    data = nullptr;
    return;
  }

  data = new_chunk.p;
  std::size_t copied = 0;
  while (copied < len) {
    auto &ch = chunk_list_.back();
    if (ch.end > ch.offset) {
      auto to_copy = std::min(ch.end - ch.offset, len - copied);
      std::copy(ch.p + ch.offset, ch.p + ch.offset + to_copy,
                new_chunk.p + copied);
      copied += to_copy;
      if (to_copy >= ch.end - ch.offset) {
        chunk_list_.pop_back();
      } else {
        ch.offset += to_copy;
      }
    }
  }
  new_chunk.end = len;
  chunk_list_.push_back(new_chunk);
  new_chunk.p = nullptr;
}

void buffer::dataChunk(const void *&data, std::size_t &len) {
  if (size_ == 0) {
    data = nullptr;
    len = 0;
    return;
  }
  auto &ch = chunk_list_.back();
  data = ch.p + ch.offset;
  len = ch.end - ch.offset;
}

void buffer::space(void *&buf, const std::size_t len) {
  if (!chunk_list_.empty()) {
    auto &ch = chunk_list_.front();
    if (ch.cap - ch.end >= len) {
      buf = ch.p + ch.end;
      return;
    }
  }

  while (!chunk_list_.empty() &&
         chunk_list_.front().offset == chunk_list_.front().end) {
    chunk_list_.pop_front();
  }

  auto newlen = std::max(len, default_chunk_size_);
  Chunk_ new_chunk(newlen);
  buf = new_chunk.p;
  new_chunk.cap = newlen;
  chunk_list_.push_front(new_chunk);
  new_chunk.p = nullptr;
}

void buffer::spaceChunk(void *&buf, std::size_t &len) {
  if (!chunk_list_.empty()) {
    auto &ch = chunk_list_.front();
    if (ch.cap - ch.end > 0) {
      buf = ch.p + ch.end;
      len = ch.cap - ch.end;
      return;
    }
  }
  Chunk_ new_chunk(default_chunk_size_);
  new_chunk.cap = default_chunk_size_;
  buf = new_chunk.p;
  if (buf != nullptr) {
    len = default_chunk_size_;
  } else {
    len = 0;
  }
  chunk_list_.push_front(new_chunk);
  new_chunk.p = nullptr;
}

void buffer::spaceHaveSeted(const std::size_t len) {
  auto &ch = chunk_list_.front();
  ch.end += len;
  size_ += len;
}

std::size_t buffer::size() { return size_; }

std::size_t buffer::default_chunk_size() { return default_chunk_size_; }

void buffer::default_chunk_size(const std::size_t size) {
  default_chunk_size_ = size;
}

unsigned char &buffer::operator[](const std::size_t i) {
  std::size_t cur = 0;

  for (auto it = chunk_list_.rbegin(); it != chunk_list_.rend(); ++it) {
    if (cur + it->end - it->offset > i) {
      return it->p[i - cur + it->offset];
    }
    cur += it->end - it->offset;
  }
  // NOTE: throw an exception may be better, but we do not use exceptions.
  static unsigned char todo = '\0';
  return todo;
}

void buffer::swap(buffer &x) {
  std::swap(x.size_, size_);
  std::swap(x.default_chunk_size_, default_chunk_size_);
  std::swap(x.chunk_list_, chunk_list_);
}

} // namespace tl
