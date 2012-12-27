#ifdef _WIN32
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <windows.h>
#else  // _WIN32
 #include <sys/mman.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <fcntl.h>
 #include <unistd.h>
#endif  // _WIN32

#include <limits>

#include "mapper.h"

namespace marisa2 {
namespace grimoire {

class MapperImpl {
 public:
  MapperImpl();
  ~MapperImpl();

  MapperImpl(const MapperImpl &) = delete;
  MapperImpl &operator=(const MapperImpl &) = delete;

  Error open(const char *filename);
  Error open(const void *address, std::size_t num_bytes);

  Error map(const void **objs, std::size_t num_bytes);

 private:
  const void *ptr_;
  std::size_t avail_;
  void *origin_;
  std::size_t size_;
#ifdef _WIN32
  HANDLE file_;
  HANDLE map_;
#else  // _WIN32
  int fd_;
#endif  // _WIN32
};

#ifdef _WIN32
MapperImpl::MapperImpl()
  : ptr_(nullptr), avail_(0), origin_(nullptr), size_(0),
    file_(INVALID_HANDLE_VALUE), map_(nullptr) {}
#else  // _WIN32
MapperImpl::MapperImpl()
  : ptr_(nullptr), avail_(0), origin_(nullptr), size_(0), fd_(-1) {}
#endif  // _WIN32

#ifdef _WIN32
MapperImpl::~MapperImpl() {
  if (origin_ != nullptr) {
    ::UnmapViewOfFile(origin_);
  }

  if (map_ != nullptr) {
    ::CloseHandle(map_);
  }

  if (file_ != nullptr) {
    ::CloseHandle(file_);
  }
}
#else  // _WIN32
MapperImpl::~MapperImpl() {
  if (fd_ != -1) {
    if ((origin_ != nullptr) && (origin_ != MAP_FAILED)) {
      ::munmap(origin_, size_);
    }
    ::close(fd_);
  }
}
#endif  // _WIN32

#ifdef _WIN32
Error MapperImpl::open(const char *filename) {
  struct _stat st;
  if (::_stat(filename, &st) != 0) {
    return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to map file: "
                         "::_stat() failed");
  }
  size_ = static_cast<std::size_t>(st.st_size);

  file_ = ::CreateFileA(filename, GENERIC_READ, FILE_SHARE_DELETE |
                        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file_ == INVALID_HANDLE_VALUE) {
    return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to map file: "
                         "::CreateFileA() failed");
  }

  map_ = ::CreateFileMapping(file_, nullptr, PAGE_READONLY, 0, 0, nullptr);
  if (map_ == nullptr) {
    return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to map file: "
                         "::CreateFileMapping() failed");
  }

  origin_ = ::MapViewOfFile(map_, FILE_MAP_READ, 0, 0, 0);
  if (origin_ == nullptr) {
    return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to map file: "
                         "::MapViewOfFile() failed");
  }

  ptr_ = static_cast<const char *>(origin_);
  avail_ = size_;
  return MARISA2_SUCCESS;
}
#else  // _WIN32
Error MapperImpl::open(const char *filename) {
  struct stat st;
  if (::stat(filename, &st) != 0) {
    return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to map file: "
                         "::stat() failed");
  }
  size_ = static_cast<std::size_t>(st.st_size);

  fd_ = ::open(filename, O_RDONLY);
  if (fd_ == -1) {
    return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to map file: "
                         "::open() failed");
  }

  origin_ = ::mmap(nullptr, size_, PROT_READ, MAP_SHARED, fd_, 0);
  if (origin_ == MAP_FAILED) {
    return MARISA2_ERROR(MARISA2_IO_ERROR, "failed to map file: "
                         "::mmap() failed");
  }

  ptr_ = static_cast<const char *>(origin_);
  avail_ = size_;
  return MARISA2_SUCCESS;
}
#endif  // _WIN32

Error MapperImpl::open(const void *address, std::size_t num_bytes) {
  ptr_ = address;
  avail_ = num_bytes;
  origin_ = const_cast<void *>(address);
  size_ = num_bytes;
  return MARISA2_SUCCESS;
}

Error MapperImpl::map(const void **bytes, std::size_t num_bytes) {
  if (num_bytes > avail_) {
    return MARISA2_ERROR(MARISA2_BOUND_ERROR, "failed to map bytes: "
                         "mapped bytes are exhausted");
  }

  *bytes = ptr_;
  ptr_ = static_cast<const char *>(ptr_) + num_bytes;
  avail_ -= num_bytes;
  return MARISA2_SUCCESS;
}

Mapper::Mapper() : impl_(nullptr) {}
Mapper::~Mapper() {}

Mapper::Mapper(const Mapper &rhs) : impl_(rhs.impl_) {}
Mapper &Mapper::operator=(const Mapper &rhs) {
  impl_ = rhs.impl_;
  return *this;
}

Mapper::Mapper(Mapper &&rhs) : impl_(std::move(rhs.impl_)) {}
Mapper &Mapper::operator=(Mapper &&rhs) {
  impl_ = std::move(rhs.impl_);
  return *this;
}

Error Mapper::open(const char *filename) {
  if (filename == nullptr) {
    return MARISA2_ERROR(MARISA2_NULL_ERROR,
                         "failed to map file: filename == nullptr");
  }

  std::unique_ptr<MapperImpl> impl(new (std::nothrow) MapperImpl);
  if (!impl) {
    return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                         "failed to map file: new MapperImpl failed");
  }

  Error error = impl->open(filename);
  if (!error) {
    impl_ = std::move(impl);
  }
  return error;
}

Error Mapper::open(const void *address, std::size_t num_bytes) {
  if (address == nullptr) {
    return MARISA2_ERROR(MARISA2_NULL_ERROR,
                         "failed to map bytes: address == nullptr");
  }

  if (num_bytes == 0) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to map bytes: num_bytes == 0");
  }

  std::unique_ptr<MapperImpl> impl(new (std::nothrow) MapperImpl);
  if (!impl) {
    return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                         "failed to map file: new MapperImpl failed");
  }

  Error error = impl->open(address, num_bytes);
  if (!error) {
    impl_ = std::move(impl);
  }
  return error;
}

Error Mapper::map_objs(const void **objs, std::size_t obj_size,
                       std::size_t num_objs) {
  if (!impl_) {
    return MARISA2_ERROR(MARISA2_STATE_ERROR,
                         "failed to map objects: not ready");
  }

  if (objs == nullptr) {
    return MARISA2_ERROR(MARISA2_NULL_ERROR,
                         "failed to map objects: objs == nullptr");
  }

  if (obj_size == 0) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to map objects: obj_size == 0");
  }

  if (num_objs == 0) {
    return MARISA2_SUCCESS;
  } else if (num_objs > (std::numeric_limits<std::size_t>::max() / obj_size)) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to map objects: too many objects");
  }

  return impl_->map(objs, obj_size * num_objs);
}

}  // namespace grimoire
}  // namespace marisa2
