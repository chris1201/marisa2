#ifndef MARISA2_GRIMOIRE_READER_H
#define MARISA2_GRIMOIRE_READER_H

#include <cstdint>
#include <cstdio>
#include <iosfwd>
#include <memory>

#include "../error.h"

namespace marisa2 {
namespace grimoire {

class ReaderImpl;

class Reader {
 public:
  Reader() noexcept;
  ~Reader() noexcept;

  Reader(Reader &&rhs) noexcept;
  Reader &operator=(Reader &&rhs) noexcept;

  explicit operator bool() const noexcept {
    return static_cast<bool>(impl_);
  }

  Error open(const char *filename) noexcept;
  Error open(std::FILE *file) noexcept;
  Error open(int fd) noexcept;
  Error open(std::istream &stream) noexcept;

  template <typename T>
  Error read(T *objs, std::size_t num_objs = 1) noexcept {
    return read_objs(objs, sizeof(T), num_objs);
  }

 private:
  std::unique_ptr<ReaderImpl> impl_;

  Error read_objs(void *objs, std::size_t obj_size,
                  std::uint32_t num_objs) noexcept;
};

}  // namespace grimoire
}  // namespace marisa2

#endif  // MARISA2_GRIMOIRE_READER_H
