#ifndef MARISA2_GRIMOIRE_MAPPER_H
#define MARISA2_GRIMOIRE_MAPPER_H

#include <cstddef>
#include <memory>

#include "../error.h"

namespace marisa2 {
namespace grimoire {

class MapperImpl;

class MARISA2_DLL_EXPORT Mapper {
 public:
  Mapper() noexcept;
  ~Mapper() noexcept;

  Mapper(Mapper &&rhs) noexcept;
  Mapper &operator=(Mapper &&rhs) noexcept;

  explicit operator bool() const noexcept {
    return static_cast<bool>(impl_);
  }

  Error open(const char *filename) noexcept;
  Error open(const void *address, std::size_t num_bytes) noexcept;

  template <typename T>
  Error map(const T **objs, std::size_t num_objs = 1) noexcept {
    return map_objs(reinterpret_cast<const void **>(objs),
                    sizeof(T), num_objs);
  }

 private:
  std::unique_ptr<MapperImpl> impl_;

  Error map_objs(const void **objs, std::size_t obj_size,
                 std::size_t num_objs) noexcept;
};

}  // namespace grimoire
}  // namespace marisa2

#endif  // MARISA2_GRIMOIRE_MAPPER_H
