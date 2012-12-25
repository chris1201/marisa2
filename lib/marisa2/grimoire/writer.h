#ifndef MARISA2_GRIMOIRE_WRITER_H
#define MARISA2_GRIMOIRE_WRITER_H

#include <cstdio>
#include <iosfwd>
#include <memory>

#include "../error.h"

namespace marisa2 {
namespace grimoire {

class WriterImpl;

class MARISA2_DLL_EXPORT Writer {
 public:
  Writer() noexcept;
  ~Writer() noexcept;

  Writer(Writer &&rhs) noexcept;
  Writer &operator=(Writer &&rhs) noexcept;

  explicit operator bool() const noexcept {
    return static_cast<bool>(impl_);
  }

  Error open(const char *filename) noexcept;
  Error open(std::FILE *file) noexcept;
  Error open(int fd) noexcept;
  Error open(std::ostream &stream) noexcept;

  template <typename T>
  Error write(const T &obj) noexcept {
    return write_objs(&obj, sizeof(T), 1);
  }

  template <typename T>
  Error write(const T *objs, std::size_t num_objs) noexcept {
    return write_objs(objs, sizeof(T), num_objs);
  }

  Error flush() noexcept;

 private:
  std::unique_ptr<WriterImpl> impl_;

  Error write_objs(const void *objs, std::size_t obj_size,
                   std::size_t num_objs) noexcept;
};

}  // namespace grimoire
}  // namespace marisa2

#endif  // MARISA2_GRIMOIRE_WRITER_H
