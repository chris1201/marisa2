#ifdef _WIN32
# include <io.h>
#else  // _WIN32
# include <unistd.h>
#endif  // _WIN32

#include <ostream>
#include <limits>
#include <new>

#include "writer.h"

namespace marisa2 {
namespace grimoire {

class WriterImpl {
 public:
  WriterImpl() noexcept
    : file_(nullptr), fd_(-1), stream_(nullptr), needs_fclose_(false) {}
  ~WriterImpl() noexcept {
    if (needs_fclose_) {
      if (file_ != nullptr) {
        std::fclose(file_);
      }
    }
  }

  WriterImpl(const WriterImpl &) = delete;
  WriterImpl &operator=(const WriterImpl &) = delete;

  explicit operator bool() const noexcept {
    return (file_ != nullptr) || (fd_ != -1) || (stream_ != nullptr);
  }

  Error open(const char *filename) noexcept;
  Error open(std::FILE *file) noexcept;
  Error open(int fd) noexcept;
  Error open(std::ostream &stream) noexcept;

  Error write(const void *bytes, std::size_t num_bytes) noexcept;

  Error flush() noexcept;

 private:
  std::FILE *file_;
  int fd_;
  std::ostream *stream_;
  bool needs_fclose_;
};

Error WriterImpl::open(const char *filename) {
  file_ = std::fopen(filename, "wb");
  if (file_ == nullptr) {
    return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to open file: "
                         "std::fopen() failed");
  }
  needs_fclose_ = true;
  return MARISA2_SUCCESS;
}

Error WriterImpl::open(std::FILE *file) {
  file_ = file;
  return MARISA2_SUCCESS;
}

Error WriterImpl::open(int fd) {
  fd_ = fd;
  return MARISA2_SUCCESS;
}

Error WriterImpl::open(std::ostream &stream) {
  stream_ = &stream;
  return MARISA2_SUCCESS;
}

Error WriterImpl::write(const void *bytes, std::size_t num_bytes) {
  if (fd_ != -1) {
    while (num_bytes != 0) {
#ifdef _WIN32
      const unsigned int max_count = std::numeric_limits<int>::max();
      const unsigned int count = (num_bytes < max_count) ?
          static_cast<unsigned int>(num_bytes) : max_count;
      const int num_bytes_written = ::_write(fd_, bytes, count);
      if (num_bytes_written <= 0) {
        return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to write bytes: "
                             "::_write() failed");
      }
#else  // _WIN32
      const std::size_t max_count = std::numeric_limits< ::ssize_t>::max();
      const std::size_t count = (num_bytes < max_count) ?
          num_bytes : max_count;
      const ::ssize_t num_bytes_written = ::write(fd_, bytes, count);
      if (num_bytes_written <= 0) {
        return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to write bytes: "
                             "::write() failed");
      }
#endif  // _WIN32
      bytes = static_cast<const char *>(bytes) + num_bytes_written;
      num_bytes -= num_bytes_written;
    }
  } else if (file_ != nullptr) {
    if (std::fwrite(bytes, 1, num_bytes, file_) != num_bytes) {
      return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to write bytes: "
                           "std::fwrite() failed");
    }
  } else if (stream_ != nullptr) {
    if (!stream_->write(static_cast<const char *>(bytes), num_bytes)) {
      return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to write bytes: "
                           "std::ostream::write() failed");
    }
  }
  return MARISA2_SUCCESS;
}

Error WriterImpl::flush() {
  if (fd_ != -1) {
#ifdef _WIN32
    if (::_commit(fd_) != 0) {
      return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to flush buffer: "
                           "::_commit() failed");
    }
#else  // _WIN32
    if (::fsync(fd_) != 0) {
      return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to flush buffer: "
                           "::fsync() failed");
    }
#endif  // _WIN32
  } else if (file_ != nullptr) {
    if (std::fflush(file_) != 0) {
      return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to flush buffer: "
                           "std::fflush() failed");
    }
  } else if (stream_ != nullptr) {
    if (!stream_->flush()) {
      return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to flush buffer: "
                           "std::ostream::flush() failed");
    }
  }
  return MARISA2_SUCCESS;
}

Writer::Writer() : impl_(nullptr) {}
Writer::~Writer() {}

Writer::Writer(Writer &&rhs) : impl_(std::move(rhs.impl_)) {}
Writer &Writer::operator=(Writer &&rhs) {
  impl_ = std::move(rhs.impl_);
  return *this;
}


Error Writer::open(const char *filename) {
  if (filename == nullptr) {
    return MARISA2_ERROR(MARISA2_NULL_ERROR,
                         "failed to open file: filename == nullptr");
  }

  std::unique_ptr<WriterImpl> impl(new (std::nothrow) WriterImpl);
  if (!impl) {
    return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                         "failed to open file: new WriterImpl failed");
  }

  Error error = impl->open(filename);
  if (!error) {
    impl_ = std::move(impl);
  }
  return error;
}

Error Writer::open(std::FILE *file) {
  if (file == nullptr) {
    return MARISA2_ERROR(MARISA2_NULL_ERROR,
                         "failed to open file: file == nullptr");
  }

  std::unique_ptr<WriterImpl> impl(new (std::nothrow) WriterImpl);
  if (!impl) {
    return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                         "failed to open file: new WriterImpl failed");
  }

  Error error = impl->open(file);
  if (!error) {
    impl_ = std::move(impl);
  }
  return error;
}

Error Writer::open(int fd) {
  if (fd == -1) {
    return MARISA2_ERROR(MARISA2_CODE_ERROR,
                         "failed to open file: fd == -1");
  }

  std::unique_ptr<WriterImpl> impl(new (std::nothrow) WriterImpl);
  if (!impl) {
    return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                         "failed to open file: new WriterImpl failed");
  }

  Error error = impl->open(fd);
  if (!error) {
    impl_ = std::move(impl);
  }
  return error;
}

Error Writer::open(std::ostream &stream) {
  if (!stream) {
    return MARISA2_ERROR(MARISA2_STATE_ERROR,
                         "failed to open file: invalid stream");
  }

  std::unique_ptr<WriterImpl> impl(new (std::nothrow) WriterImpl);
  if (!impl) {
    return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                         "failed to open file: new WriterImpl failed");
  }

  Error error = impl->open(stream);
  if (!error) {
    impl_ = std::move(impl);
  }
  return error;
}

Error Writer::write_objs(const void *objs, std::size_t obj_size,
                         std::uint32_t num_objs) {
  if (!impl_) {
    return MARISA2_ERROR(MARISA2_STATE_ERROR,
                         "failed to write objects: not ready");
  }

  if (obj_size == 0) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to write objects: obj_size == 0");
  }

  if (num_objs == 0) {
    return MARISA2_SUCCESS;
  } else if (num_objs > (std::numeric_limits<std::size_t>::max() / obj_size)) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to write objects: too many objects");
  }

  return impl_->write(objs, obj_size * num_objs);
}

Error Writer::flush() {
  if (!impl_) {
    return MARISA2_ERROR(MARISA2_STATE_ERROR,
                         "failed to flush buffer: not ready");
  }

  return impl_->flush();
}

}  // namespace grimoire
}  // namespace marisa2
