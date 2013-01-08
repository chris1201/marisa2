#ifndef MARISA2_GRIMOIRE_VECTOR_H
#define MARISA2_GRIMOIRE_VECTOR_H

#include <cstdint>
#include <type_traits>

#include "mapper.h"
#include "reader.h"
#include "writer.h"

namespace marisa2 {
namespace grimoire {

struct VectorHeader {
  std::uint64_t size;
};

class MARISA2_DLL_EXPORT VectorImpl {
 public:
  explicit VectorImpl(std::size_t obj_size) noexcept;
  ~VectorImpl() = default;

  VectorImpl(const VectorImpl &) = delete;
  VectorImpl &operator=(const VectorImpl &) = delete;

  Error map(Mapper &mapper, const VectorHeader &header) noexcept;
  Error read(Reader &reader, const VectorHeader &header) noexcept;

  Error reserve(std::size_t new_size) noexcept;
  Error reallocate(std::size_t new_size) noexcept;

  const void *address() const noexcept {
    return address_;
  }
  void *address() noexcept {
    return address_;
  }

  std::size_t size() const noexcept {
    return size_;
  }
  std::size_t capacity() const noexcept {
    return capacity_;
  }

  void set_size(std::size_t new_size) noexcept {
    size_ = new_size;
  }

 private:
  void *address_;
  std::size_t size_;
  std::size_t capacity_;
  std::unique_ptr<char[]> buf_;
  const std::size_t obj_size_;
};

template <typename T>
class Vector {
  static_assert(std::is_pod<T>::value, "T is not a POD type.");

 public:
  Vector() noexcept : impl_(sizeof(T)) {}
  ~Vector() = default;

  Vector(const Vector &) = delete;
  Vector &operator=(const Vector &) = delete;

  explicit operator bool() const noexcept {
    return impl_.size() != 0;
  }

  Error map(Mapper &mapper, const VectorHeader &header) noexcept {
    return impl_.map(mapper, header);
  }
  Error read(Reader &reader, const VectorHeader &header) noexcept {
    return impl_.read(reader, header);
  }
  Error write(Writer &writer) const noexcept {
    return writer.write(static_cast<const T *>(impl_.address()), impl_.size());
  }

  Error push_back(const T &obj) noexcept {
    if (impl_.size() == impl_.capacity()) {
      Error error = impl_.reserve(impl_.size() + 1);
      if (error) {
        return error;
      }
    }
    static_cast<T *>(impl_.address())[impl_.size()] = obj;
    impl_.set_size(impl_.size() + 1);
    return MARISA2_SUCCESS;
  }

  // TODO: Remove this function if unused.
//  Error pop_back() noexcept {
//    if (impl_.size() == 0) {
//      return MARISA2_ERROR(MARISA2_STATE_ERROR,
//                           "failed to pop object: empty");
//    }
//    impl_.set_size(impl_.size() - 1);
//    return MARISA2_SUCCESS;
//  }

  Error resize(std::size_t new_size) noexcept {
    if (new_size > impl_.capacity()) {
      Error error = impl_.reserve(new_size);
      if (error) {
        return error;
      }
    }
    impl_.set_size(new_size);
    return MARISA2_SUCCESS;
  }

  Error resize(std::size_t new_size, const T &obj) noexcept {
    if (new_size > impl_.capacity()) {
      Error error = impl_.reserve(new_size);
      if (error) {
        return error;
      }
    }
    for (std::size_t i = impl_.size(); i < new_size; ++i) {
      static_cast<T *>(impl_.address())[i] = obj;
    }
    impl_.set_size(new_size);
    return MARISA2_SUCCESS;
  }

  Error reserve(std::size_t required_capacity) noexcept {
    return impl_.reserve(required_capacity);
  }

  void clear() noexcept {
    impl_.reallocate(0);
  }

  Error shrink() noexcept {
    if (impl_.size() != impl_.capacity()) {
      return impl_.reallocate(impl_.size());
    }
    return MARISA2_SUCCESS;
  }

  const T &operator[](std::size_t i) const noexcept {
    return static_cast<const T *>(impl_.address())[i];
  }
  T &operator[](std::size_t i) noexcept {
    return static_cast<T *>(impl_.address())[i];
  }

  const T *begin() const noexcept {
    return static_cast<const T *>(impl_.address());
  }
  const T *end() const noexcept {
    return static_cast<const T *>(impl_.address()) + impl_.size();
  }
  T *begin() noexcept {
    return static_cast<T *>(impl_.address());
  }
  T *end() noexcept {
    return static_cast<T *>(impl_.address()) + impl_.size();
  }

  const T &front() const noexcept {
    return *static_cast<const T *>(impl_.address());
  }
  const T &back() const noexcept {
    return static_cast<const T *>(impl_.address())[impl_.size() - 1];
  }
  T &front() noexcept {
    return *static_cast<T *>(impl_.address());
  }
  T &back() noexcept {
    return static_cast<T *>(impl_.address())[impl_.size() - 1];
  }

  std::size_t size() const noexcept {
    return impl_.size();
  }
  std::size_t capacity() const noexcept {
    return impl_.capacity();
  }
  VectorHeader header() const noexcept {
    return VectorHeader{ impl_.size() };
  }

 private:
  VectorImpl impl_;
};

}  // namespace grimoire
}  // namespace marisa2

#endif  // MARISA2_GRIMOIRE_VECTOR_H
