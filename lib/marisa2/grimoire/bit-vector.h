#ifndef MARISA2_GRIMOIRE_BIT_VECTOR_H
#define MARISA2_GRIMOIRE_BIT_VECTOR_H

#include <limits>

#include "pop-count.h"
#include "vector.h"

namespace marisa2 {
namespace grimoire {

struct BitVectorHeader {
  std::uint64_t size;
  std::uint64_t num_1s;
  std::uint8_t rank;
  std::uint8_t select_1;
  std::uint8_t select_0;
  std::uint8_t reserved[5];
};

struct BitVectorRank {
  std::uint32_t abs;
  std::uint8_t rels[4];
};

// These flags are used to build indices for select_1/0().
enum {
  ENABLE_SELECT_1 = 1,
  ENABLE_SELECT_0 = 2
};

class MARISA2_DLL_EXPORT BitVector {
 public:
  BitVector() noexcept;
  ~BitVector() noexcept;

  BitVector(const BitVector &) = delete;
  BitVector &operator=(const BitVector &) = delete;

  explicit operator bool() const noexcept {
    return static_cast<bool>(units_);
  }

  Error map(Mapper &mapper, const BitVectorHeader &header) noexcept;
  Error read(Reader &reader, const BitVectorHeader &header) noexcept;
  Error write(Writer &writer) noexcept;

  Error push_back(bool bit) noexcept;

  // ENABLE_SELECT_1 and ENABLE_SELECT_0 are avaiable.
  Error build(int flags = 0) noexcept;

  bool operator[](std::size_t i) const noexcept {
    return (units_[i / 64] >> (i % 64)) & 1;
  }

  std::size_t rank_1(std::size_t i) const noexcept;
  std::size_t rank_0(std::size_t i) const noexcept {
    return i - rank_1(i);
  }

  std::size_t select_1(std::size_t i) const noexcept;
  std::size_t select_0(std::size_t i) const noexcept;

  std::size_t size() const noexcept {
    return size_;
  }
  std::size_t num_1s() const noexcept {
    return num_1s_;
  }
  std::size_t num_0s() const noexcept {
    return size_ - num_1s_;
  }
  BitVectorHeader header() const noexcept {
    return BitVectorHeader{ size_, num_1s_,
                            static_cast<bool>(ranks_),
                            static_cast<bool>(select_1s_),
                            static_cast<bool>(select_0s_), {} };
  }

 private:
  Vector<std::uint64_t> units_;
  std::size_t size_;
  std::size_t num_1s_;
  Vector<BitVectorRank> ranks_;
  Vector<std::uint32_t> select_1s_;
  Vector<std::uint32_t> select_0s_;

  Error build_rank() noexcept;
  Error build_select(int flags) noexcept;
};

}  // namespace grimoire
}  // namespace marisa2

#endif  // MARISA2_GRIMOIRE_BIT_VECTOR_H
