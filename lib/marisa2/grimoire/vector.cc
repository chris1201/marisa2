#include <cstring>
#include <limits>
#include <new>

#include "vector.h"

namespace marisa2 {
namespace grimoire {

VectorImpl::VectorImpl(std::size_t obj_size)
  : address_(nullptr), size_(0), capacity_(0), buf_(), obj_size_(obj_size) {}

Error VectorImpl::map(Mapper &mapper, const VectorHeader &header) {
  if (header.size > std::numeric_limits<std::size_t>::max()) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to map vector: too many objects");
  }

  const std::size_t new_size = static_cast<std::size_t>(header.size);
  if (new_size > (std::numeric_limits<std::size_t>::max() / obj_size_)) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to map vector: too many objects");
  }

  const char *objs;
  Error error = mapper.map(&objs, obj_size_ * new_size);
  if (error) {
    return error;
  }

  address_ = const_cast<char *>(objs);
  size_ = new_size;
  capacity_ = new_size;
  buf_.reset();
  return MARISA2_SUCCESS;
}

Error VectorImpl::read(Reader &reader, const VectorHeader &header) {
  if (header.size > std::numeric_limits<std::size_t>::max()) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to read vector: too many objects");
  }

  const std::size_t new_size = static_cast<std::size_t>(header.size);
  if (new_size > (std::numeric_limits<std::size_t>::max() / obj_size_)) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to read vector: too many objects");
  }

  std::unique_ptr<char[]> new_buf;
  if (new_size != 0) {
    new_buf.reset(new (std::nothrow) char[obj_size_ * new_size]);
    if (!new_buf) {
      return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                           "failed to read vector: new char[] failed");
    }
  }

  Error error = reader.read(new_buf.get(), obj_size_ * new_size);
  if (error) {
    return error;
  }

  address_ = new_buf.get();
  size_ = new_size;
  capacity_ = new_size;
  buf_ = std::move(new_buf);
  return MARISA2_SUCCESS;
}

Error VectorImpl::reserve(std::size_t new_size) {
  if (new_size <= capacity_) {
    return MARISA2_SUCCESS;
  }

  std::size_t new_capacity = new_size;
  if (capacity_ > (new_size / 2)) {
    if (capacity_ > (std::numeric_limits<std::size_t>::max() / 2)) {
      new_capacity = std::numeric_limits<std::size_t>::max();
    } else {
      new_capacity = capacity_ * 2;
    }
  }
  return reallocate(new_capacity);
}

Error VectorImpl::reallocate(std::size_t new_size) {
  if (new_size > (std::numeric_limits<std::size_t>::max() / obj_size_)) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to reallocate vector: too many objects");
  }

  std::unique_ptr<char[]> new_buf;
  if (new_size != 0) {
    new_buf.reset(new (std::nothrow) char[obj_size_ * new_size]);
    if (!new_buf) {
      return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                           "failed to reallocate vector: new char[] failed");
    }
  }

  std::memcpy(new_buf.get(), address_, obj_size_ * size_);
  address_ = new_buf.get();
  capacity_ = new_size;
  buf_ = std::move(new_buf);
  return MARISA2_SUCCESS;
}

}  // namespace grimoire
}  // namespace marisa2
