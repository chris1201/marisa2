#include "bit-vector.h"

namespace marisa2 {
namespace grimoire {

BitVector::BitVector()
  : units_(), size_(0), num_1s_(0), ranks_(), select_1s_(), select_0s_() {}

BitVector::~BitVector() {}

//Error BitVector::map(Mapper &mapper, const BitVectorHeader &header);
//Error BitVector::read(Reader &reader, const BitVectorHeader &header);
//Error BitVector::write(Writer &writer);

Error BitVector::push_back(bool bit) noexcept {
  if (size_ == std::numeric_limits<std::size_t>::max()) {
    return MARISA2_ERROR(MARISA2_SIZE_ERROR, "failed push bit: full");
  }
  if (size_ == (units_.size() * 64)) {
    Error error = units_.push_back(0);
    if (error) {
      return error;
    }
  }
  if (bit) {
    units_[size_ / 64] |= std::uint64_t(1) << (size_ % 64);
    ++num_1s_;
  }
  ++size_;
  return MARISA2_SUCCESS;
}

Error BitVector::build(int flags) {
  ranks_.clear();
  select_1s_.clear();
  select_0s_.clear();

  Error error = build_rank();
  if (error) {
    return error;
  }

  if (flags & (ENABLE_SELECT_1 | ENABLE_SELECT_0)) {
    return build_select(flags);
  }
  return MARISA2_SUCCESS;
}

std::size_t BitVector::rank_1(std::size_t i) const {
  return (static_cast<std::size_t>(ranks_[i / 256].abs) << 6)
      + ranks_[i / 256].rels[(i / 64) % 4]
      + PopCount::pop_count((units_[i / 64] << 1) << (~i % 64));
}

//std::size_t BitVector::select_1(std::size_t i) const;
//std::size_t BitVector::select_0(std::size_t i) const;

Error BitVector::build_rank() noexcept {
  Error error = ranks_.resize((size_ / 256) + ((size_ % 256) != 0) + 1);
  if (error) {
    return error;
  }

  std::size_t num_1s = 0;
  for (std::size_t i = 0; i < size_; i += 64) {
    if ((i % 256) == 0) {
      ranks_[i / 256].abs = static_cast<std::uint32_t>(num_1s >> 6);
    }
    ranks_[i / 256].rels[i / 64] =
        static_cast<std::uint8_t>(num_1s - ranks_[i / 256].abs);
  }

  if ((size_ % 256) != 0) {
    std::size_t i = size_ / 256;
    std::size_t abs = (ranks_[i].abs << 6) + ranks_[i].rels[0];
    switch (((size_ - 1) / 64) % 4) {
      case 0: {
        ranks_[i].rels[1] = static_cast<std::uint8_t>(num_1s - abs);
      }
      case 1: {
        ranks_[i].rels[2] = static_cast<std::uint8_t>(num_1s - abs);
      }
      case 2: {
        ranks_[i].rels[3] = static_cast<std::uint8_t>(num_1s - abs);
        break;
      }
    }
  }

  ranks_.back().abs = static_cast<std::uint32_t>(num_1s >> 6);
  ranks_.back().rels[0] = static_cast<std::uint8_t>(num_1s & 0x3F);
  ranks_.back().rels[1] = static_cast<std::uint8_t>(num_1s & 0x3F);
  ranks_.back().rels[2] = static_cast<std::uint8_t>(num_1s & 0x3F);
  ranks_.back().rels[3] = static_cast<std::uint8_t>(num_1s & 0x3F);

  return MARISA2_SUCCESS;
}

Error BitVector::build_select(int flags) noexcept {
  return MARISA2_SUCCESS;
}

}  // namespace grimoire
}  // namespace marisa2
