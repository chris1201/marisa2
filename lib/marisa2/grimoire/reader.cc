#ifdef _WIN32
# include <io.h>
#else  // _WIN32
# include <unistd.h>
#endif  // _WIN32

#include <algorithm>
#include <iostream>
#include <limits>
#include <new>

#include "reader.h"

namespace marisa2 {
namespace grimoire {
namespace {

class ReaderImpl {
 public:
  ReaderImpl() noexcept
    : file_(nullptr), fd_(-1), stream_(nullptr), needs_fclose_(false) {}
  ~ReaderImpl() noexcept {
    if (needs_fclose_) {
      if (file_ != nullptr) {
        std::fclose(file_);
      }
    }
  }

  ReaderImpl(const ReaderImpl &) = delete;
  ReaderImpl &operator=(const ReaderImpl &) = delete;

  explicit operator bool() const noexcept {
    return (file_ != nullptr) || (fd_ != -1) || (stream_ != nullptr);
  }

  Error open(const char *filename) noexcept;
  Error open(std::FILE *file) noexcept;
  Error open(int fd) noexcept;
  Error open(std::istream &stream) noexcept;

  Error read(void *buf, std::size_t size) noexcept;

 private:
  std::FILE *file_;
  int fd_;
  std::istream *stream_;
  bool needs_fclose_;
};

Error ReaderImpl::open(const char *filename) {
  file_ = std::fopen(filename, "rb");
  if (file_ == nullptr) {
    return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to open file");
  }
  needs_fclose_ = true;
  return MARISA2_SUCCESS;
}

Error ReaderImpl::open(std::FILE *file) {
  file_ = file;
  return MARISA2_SUCCESS;
}

Error ReaderImpl::open(int fd) {
  fd_ = fd;
  return MARISA2_SUCCESS;
}

Error ReaderImpl::open(std::istream &stream) {
  stream_ = &stream;
  return MARISA2_SUCCESS;
}

Error ReaderImpl::read(void *buf, std::size_t size) {
  if (fd_ != -1) {
    while (size != 0) {
#ifdef _WIN32
      // TODO: constexpr is better.
      const unsigned int count = std::min(size,
          static_cast<std::size_t>(std::numeric_limits<int>::max()));
      const int size_read = ::_read(fd_, buf, count);
#else  // _WIN32
      // TODO: constexpr is better.
      const ::size_t count = std::min(size,
          static_cast<std::size_t>(std::numeric_limits< ::ssize_t>::max()));
      const ::ssize_t size_read = ::read(fd_, buf, count);
#endif  // _WIN32
      if (size_read <= 0) {
        return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to read bytes");
      }
      buf = static_cast<char *>(buf) + size_read;
      size -= size_read;
    }
  } else if (file_ != nullptr) {
    if (std::fread(buf, 1, size, file_) != size) {
      return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to read bytes");
    }
  } else if (stream_ != nullptr) {
    if (!stream_->read(static_cast<char *>(buf), size)) {
      return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to read bytes");
    }
  }
  return MARISA2_SUCCESS;
}

}  // namespace

Reader::Reader() : impl_(nullptr) {}
Reader::~Reader() {}

Reader::Reader(Reader &&rhs) : impl_(std::move(rhs.impl_)) {}
Reader &Reader::operator=(Reader &&rhs) {
  impl_ = std::move(rhs.impl_);
  return *this;
}

Error Reader::open(const char *filename) {
  if (filename == nullptr) {
    return MARISA2_ERROR(MARISA2_NULL_ERROR,
                         "failed to open file: filename == nullptr");
  }

  std::unique_ptr<ReaderImpl> impl(new (std::nothrow) ReaderImpl);
  if (!impl) {
    return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                         "failed to open file: memory allocation failed");
  }

  Error error = impl->open(filename);
  if (!error) {
    impl_ = std::move(impl);
  }
  return error;
}

Error Reader::open(std::FILE *file) {
  if (file == nullptr) {
    return MARISA2_ERROR(MARISA2_NULL_ERROR,
                         "failed to open file: file == nullptr");
  }

  std::unique_ptr<ReaderImpl> impl(new (std::nothrow) ReaderImpl);
  if (!impl) {
    return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                         "failed to open file: memory allocation failed");
  }

  Error error = impl->open(file);
  if (!error) {
    impl_ = std::move(impl);
  }
  return error;
}

Error Reader::open(int fd) {
  if (fd == -1) {
    return MARISA2_ERROR(MARISA2_CODE_ERROR,
                         "failed to open file: fd == -1");
  }

  std::unique_ptr<ReaderImpl> impl(new (std::nothrow) ReaderImpl);
  if (!impl) {
    return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                         "failed to open file: memory allocation failed");
  }

  Error error = impl->open(fd);
  if (!error) {
    impl_ = std::move(impl);
  }
  return error;
}

Error Reader::open(std::istream &stream) {
  if (!stream) {
    return MARISA2_ERROR(MARISA2_STATE_ERROR,
                         "failed to open file: invalid stream");
  }

  std::unique_ptr<ReaderImpl> impl(new (std::nothrow) ReaderImpl);
  if (!impl) {
    return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                         "failed to open file: memory allocation failed");
  }

  Error error = impl->open(stream);
  if (!error) {
    impl_ = std::move(impl);
  }
  return error;
}

Error Reader::read_objs(void *buf, std::size_t obj_size,
                        std::uint32_t num_objs) {
  if (!impl_) {
    return MARISA2_ERROR(MARISA2_STATE_ERROR,
                         "failed to read objects: not ready");
  }

  if (obj_size == 0) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to read objects: invalid object size");
  }

  if (num_objs == 0) {
    return MARISA2_SUCCESS;
  } else if (num_objs > (std::numeric_limits<std::size_t>::max() / obj_size)) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to read objects: too many objects");
  }

  return read_bytes(buf, obj_size * num_objs);
}

Error Reader::read_bytes(void *buf, std::size_t num_bytes) {
  return impl_->read(buf, num_bytes);
}

}  // namespace grimoire
}  // namespace marisa2
