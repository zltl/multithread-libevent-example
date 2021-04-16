#pragma once

#include <cstddef>
#include <list>

namespace tl {

// buffer queue of bytes
class buffer final {
public:
  buffer();
  ~buffer();

  // Add data to front, return the actual added data size.
  // If buffer cannot allocate enough memory, the return value
  // will smaller than "len".
  std::size_t push(const void *data, const std::size_t len);
  // Delete "len"'s bytes data from tail of the buffer queue. Return
  // actual deleted bytes. If size of buffer smaller than "len", return
  // the size of buffer befor deleted.
  std::size_t drain(const std::size_t len);
  // Get pointer to a continuous buffer containing at less "len" bytes of
  // data from the last chunk. set "data" to nullptr if not enough data. buffer
  // may alocate a new chunk to make datas continuous.
  void data(const void *&data, const std::size_t len);
  // Get pointer to the first chunk's first data location. The output variable
  // "len" depends on the size of data of the last chunk.
  void dataChunk(const void *&data, std::size_t &len);
  // Get a pointer to a "len" bytes' size of free buffer chunk to set data.
  // "buf" be nullptr if EOM.
  void space(void *&buf, const std::size_t len);
  // Get a pointer to the first free chunk. Allocate a new chunk if no more
  // free space.
  void spaceChunk(void *&buf, std::size_t &len);
  // Call this function when set data to free space return by space() and
  // spaceChunnk(), increase "end" with "len".
  void spaceHaveSeted(const std::size_t len);

  // Return total data size of buffer.
  std::size_t size();

  std::size_t default_chunk_size();
  void default_chunk_size(const std::size_t size);

  // Reference to the ”i“'th element of data from tail.
  unsigned char &operator[](const std::size_t i);

  // Exchanges the contents of the container by the content of x.
  void swap(buffer&x);


private:
  using ElemType_ = unsigned char;
  struct Chunk_ {
    Chunk_(std::size_t len) {
      p = new ElemType_[len];
      cap = len;
    }
    // !!! IMPORTANT: ch.p = nullptr after copy.
    Chunk_(const Chunk_ &ch) {
      p = ch.p;
      offset = ch.offset;
      end = ch.end;
      cap = ch.cap;
    }
    ~Chunk_() {
      if (p)
        delete[] p;
    }
    ElemType_ *p = nullptr;
    std::size_t offset = 0;
    std::size_t end = 0;
    std::size_t cap = 0;
  };

  // total data size
  std::size_t size_ = 0;
  // default chunk size
  std::size_t default_chunk_size_ = 4096;
  // a list containing all chunks.
  // add data --> front .. chunk .. chunk .. end --> drain data
  std::list<Chunk_> chunk_list_;
};

} // namespace tl
