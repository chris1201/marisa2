#include <cstring>
#include <limits>
#include <new>

#include "vector.h"

namespace marisa2 {
namespace grimoire {

Error VectorImpl::map(Mapper &mapper, const VectorHeader &header) {
  if (header.obj_size > std::numeric_limits<std::size_t>::max()) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to map vector: too large objects");
  } else if (header.num_objs > std::numeric_limits<std::size_t>::max()) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to map vector: too many objects");
  }

  const std::size_t obj_size = static_cast<std::size_t>(header.obj_size);
  const std::size_t num_objs = static_cast<std::size_t>(header.num_objs);

  if (obj_size == 0) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to map vector: obj_size == 0");
  } else if (num_objs > (std::numeric_limits<std::size_t>::max() / obj_size)) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to map vector: too many objects");
  }

  const char *objs;
  Error error = mapper.map(&objs, obj_size * num_objs);
  if (error) {
    return error;
  }

  address_ = const_cast<char *>(objs);
  size_ = num_objs;
  capacity_ = num_objs;
  buf_.reset();
  return MARISA2_SUCCESS;
}

Error VectorImpl::read(Reader &reader, const VectorHeader &header) {
  if (header.obj_size > std::numeric_limits<std::size_t>::max()) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to read vector: too large objects");
  } else if (header.num_objs > std::numeric_limits<std::size_t>::max()) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to read vector: too many objects");
  }

  const std::size_t obj_size = static_cast<std::size_t>(header.obj_size);
  const std::size_t num_objs = static_cast<std::size_t>(header.num_objs);

  if (header.obj_size == 0) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to read vector: obj_size == 0");
  } else if (num_objs > (std::numeric_limits<std::size_t>::max() / obj_size)) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to read vector: too many objects");
  }

  std::unique_ptr<char[]> new_buf;
  if (num_objs != 0) {
    new_buf.reset(new (std::nothrow) char[obj_size * num_objs]);
    if (!new_buf) {
      return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                           "failed to read vector: new char[] failed");
    }
  }

  Error error = reader.read(new_buf.get(), obj_size * num_objs);
  if (error) {
    return error;
  }

  address_ = new_buf.get();
  size_ = num_objs;
  capacity_ = num_objs;
  buf_ = std::move(new_buf);
  return MARISA2_SUCCESS;
}

Error VectorImpl::reserve(std::size_t obj_size, std::size_t num_objs) {
  if (num_objs <= capacity_) {
    return MARISA2_SUCCESS;
  }

  std::size_t new_capacity = num_objs;
  if (capacity_ > (num_objs / 2)) {
    if (capacity_ > (std::numeric_limits<std::size_t>::max() / 2)) {
      new_capacity = std::numeric_limits<std::size_t>::max();
    } else {
      new_capacity = capacity_ * 2;
    }
  }
  return reallocate(obj_size, new_capacity);
}

Error VectorImpl::reallocate(std::size_t obj_size, std::size_t num_objs) {
  if (obj_size == 0) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to reallocate vector: obj_size == 0");
  } else if (num_objs > (std::numeric_limits<std::size_t>::max() / obj_size)) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to reallocate vector: too many objects");
  }

  std::unique_ptr<char[]> new_buf;
  if (num_objs != 0) {
    new_buf.reset(new (std::nothrow) char[obj_size * num_objs]);
    if (!new_buf) {
      return MARISA2_ERROR(MARISA2_MEMORY_ERROR,
                           "failed to reallocate vector: new char[] failed");
    }
  }

  std::memcpy(new_buf.get(), address_, obj_size * size_);
  address_ = new_buf.get();
  capacity_ = num_objs;
  buf_ = std::move(new_buf);
  return MARISA2_SUCCESS;
}

}  // namespace grimoire
}  // namespace marisa2
