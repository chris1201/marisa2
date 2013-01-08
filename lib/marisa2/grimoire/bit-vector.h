#ifndef MARISA2_GRIMOIRE_BIT_VECTOR_H
#define MARISA2_GRIMOIRE_BIT_VECTOR_H

#include "pop-count.h"
#include "vector.h"

// These flags are used to build indices for select_1/0().
enum {
  MARISA2_ENABLE_RANK     = 1 << 0,
  MARISA2_ENABLE_SELECT_1 = 1 << 1,
  MARISA2_ENABLE_SELECT_0 = 1 << 2
};

namespace marisa2 {
namespace grimoire {

struct BitVectorHeader {
  std::uint64_t size;
  std::uint64_t num_1s;
  std::uint64_t flags;
};

struct BitVectorRank {
  std::uint32_t abs;
  std::uint8_t rels[4];
};

class MARISA2_DLL_EXPORT BitVector {
 public:
  BitVector() noexcept;
  ~BitVector() noexcept;

  BitVector(const BitVector &) = delete;
  BitVector &operator=(const BitVector &) = delete;

  explicit operator bool() const noexcept {
    return size_ != 0;
  }

  Error map(Mapper &mapper, const BitVectorHeader &header) noexcept;
  Error read(Reader &reader, const BitVectorHeader &header) noexcept;
  Error write(Writer &writer) noexcept;

  Error push_back(bool bit) noexcept;

  // MARISA2_ENABLE_SELECT_1/0 are avaiable.
  // MARISA2_ENABLE_RANK is implicitly enabled even if omitted.
  Error build(int flags = 0) noexcept;

  bool operator[](std::size_t i) const noexcept {
    return (reinterpret_cast<const std::uint64_t *>(&*packs_.begin())
        [(i / 64) + (i / 256)] >> (i % 64)) & 1;
  }

  // rank_1/0()s are available after build().
  std::size_t rank_1(std::size_t i) const noexcept {
    const Pack &pack = packs_[i / 256];
    const std::size_t j = (i / 64) % 4;
    return (static_cast<std::size_t>(pack.rank.abs) << 6) + pack.rank.rels[j]
        + PopCount::pop_count((pack.units[j] << 1) << (63 - (i % 64)));
  }
  std::size_t rank_0(std::size_t i) const noexcept {
    return i - rank_1(i);
  }

  // select_1/0()s are available after build() with ENABLE_SELECT_1/0.
  std::size_t select_1(std::size_t i) const noexcept;
  std::size_t select_0(std::size_t i) const noexcept;

  std::size_t size() const noexcept {
    return size_;
  }
  // num_1/0()s are available after build().
  std::size_t num_1s() const noexcept {
    return num_1s_;
  }
  std::size_t num_0s() const noexcept {
    return size_ - num_1s_;
  }
  int flags() const noexcept {
    return flags_;
  }
  BitVectorHeader header() const noexcept {
    return BitVectorHeader{ size_, num_1s_, flags_ };
  }

 private:
  struct Rank {
    std::uint32_t abs;
    std::uint8_t rels[4];
  };

  struct Pack {
    std::uint64_t units[4];
    Rank rank;
  };

  Vector<Pack> packs_;
  std::size_t size_;
  std::size_t num_1s_;
  int flags_;
  Vector<std::uint32_t> select_1s_;
  Vector<std::uint32_t> select_0s_;

  Error build_rank() noexcept;
  Error build_select_1() noexcept;
  Error build_select_0() noexcept;
};

}  // namespace grimoire
}  // namespace marisa2

#endif  // MARISA2_GRIMOIRE_BIT_VECTOR_H
