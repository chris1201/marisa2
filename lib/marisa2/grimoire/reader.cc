#ifdef _WIN32
# include <io.h>
#else  // _WIN32
# include <unistd.h>
#endif  // _WIN32

#include <istream>
#include <limits>
#include <new>

#include "reader.h"

namespace marisa2 {
namespace grimoire {

class ReaderImpl {
 public:
  ReaderImpl() noexcept
    : file_(nullptr), fd_(-1), stream_(nullptr), needs_fclose_(false) {}
  ~ReaderImpl() noexcept {
    // file_ is closed if the reader is opened with a filename.
    if (needs_fclose_) {
      if (file_ != nullptr) {
        std::fclose(file_);
      }
    }
  }

  ReaderImpl(const ReaderImpl &) = delete;
  ReaderImpl &operator=(const ReaderImpl &) = delete;

  Error open(const char *filename) noexcept;
  Error open(std::FILE *file) noexcept;
  Error open(int fd) noexcept;
  Error open(std::istream &stream) noexcept;

  Error read(void *bytes, std::size_t num_bytes) noexcept;

 private:
  std::FILE *file_;
  int fd_;
  std::istream *stream_;
  bool needs_fclose_;
};

Error ReaderImpl::open(const char *filename) {
  file_ = std::fopen(filename, "rb");
  if (file_ == nullptr) {
    return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to open file: "
                         "std::fopen() failed");
  }

  // The file will be closed in destructor.
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

Error ReaderImpl::read(void *buf, std::size_t num_bytes) {
  if (fd_ != -1) {
    while (num_bytes != 0) {
#ifdef _WIN32
      const unsigned int max_count = std::numeric_limits<int>::max();
      const unsigned int count = (num_bytes < max_count) ?
          static_cast<unsigned int>(num_bytes) : max_count;
      const int num_bytes_read = ::_read(fd_, buf, count);
      if (num_bytes_read <= 0) {
        return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to read bytes: "
                             "::_read() failed");
      }
#else  // _WIN32
      const std::size_t max_count = std::numeric_limits< ::ssize_t>::max();
      const std::size_t count = (num_bytes < max_count) ?
          num_bytes : max_count;
      const ::ssize_t num_bytes_read = ::read(fd_, buf, count);
      if (num_bytes_read <= 0) {
        return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to read bytes: "
                             "::read() failed");
      }
#endif  // _WIN32
      buf = static_cast<char *>(buf) + num_bytes_read;
      num_bytes -= num_bytes_read;
    }
  } else if (file_ != nullptr) {
    if (std::fread(buf, 1, num_bytes, file_) != num_bytes) {
      return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to read bytes: "
                           "std::fread() failed");
    }
  } else if (stream_ != nullptr) {
    if (!stream_->read(static_cast<char *>(buf), num_bytes)) {
      return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to read bytes: "
                           "std::istream::read() failed");
    }
  }
  return MARISA2_SUCCESS;
}

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
                         "failed to open file: new ReaderImpl failed");
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
                         "failed to open file: new ReaderImpl failed");
  }

  Error error = impl->open(file);
  if (!error) {
    impl_ = std::move(impl);
  }
  return error;
}

Error Reader::open(int fd) {
  if (fd == -1) {
    return MARISA2_ERROR(MARISA2_IO_ERROR,
                         "failed to open file: fd == -1");
  }

  std::unique_ptr<ReaderImpl> impl(new (std::nothrow) ReaderImpl);
  if (!impl) {
    return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                         "failed to open file: new ReaderImpl failed");
  }

  Error error = impl->open(fd);
  if (!error) {
    impl_ = std::move(impl);
  }
  return error;
}

Error Reader::open(std::istream &stream) {
  if (!stream) {
    return MARISA2_ERROR(MARISA2_IO_ERROR,
                         "failed to open file: invalid stream");
  }

  std::unique_ptr<ReaderImpl> impl(new (std::nothrow) ReaderImpl);
  if (!impl) {
    return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                         "failed to open file: new ReaderImpl failed");
  }

  Error error = impl->open(stream);
  if (!error) {
    impl_ = std::move(impl);
  }
  return error;
}

Error Reader::read_objs(void *objs, std::size_t obj_size,
                        std::size_t num_objs) {
  if (!impl_) {
    return MARISA2_ERROR(MARISA2_STATE_ERROR,
                         "failed to read objects: not ready");
  }

  if (obj_size == 0) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to read objects: obj_size == 0");
  }

  if (num_objs == 0) {
    return MARISA2_SUCCESS;
  } else if (num_objs > (std::numeric_limits<std::size_t>::max() / obj_size)) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to read objects: too many objects");
  }

  return impl_->read(objs, obj_size * num_objs);
}

}  // namespace grimoire
}  // namespace marisa2
