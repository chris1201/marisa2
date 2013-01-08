#include <limits>

#include "bit-vector.h"

namespace marisa2 {
namespace grimoire {

BitVector::BitVector()
  : packs_(), size_(0), num_1s_(0), flags_(0), select_1s_(), select_0s_() {}

BitVector::~BitVector() {}

Error BitVector::map(Mapper &mapper, const BitVectorHeader &header) {
  if (header.size > std::numeric_limits<std::size_t>::max()) {
    return MARISA2_ERROR(MARISA2_RANGE_ERROR,
                         "failed to map bit vector: too large");
  }
  const std::size_t new_size = static_cast<std::size_t>(header.size);

  if (header.num_1s > header.size) {
    return MARISA2_ERROR(MARISA2_FORMAT_ERROR,
                         "failed to map bit vector: invalid num_1s");
  }
  const std::size_t new_num_1s = static_cast<std::size_t>(header.num_1s);
  const std::size_t new_num_0s = new_size - new_num_1s;

  if (header.flags == 0) {
    return MARISA2_ERROR(MARISA2_FORMAT_ERROR,
                         "failed to map bit vector: invalid flags");
  }

  Vector<Pack> new_packs;
  Error error = new_packs.map(mapper,
      VectorHeader{ (new_size / 256) + ((new_size % 256 != 0)) + 1 });
  if (error) {
    return error;
  }

  Vector<std::uint32_t> new_select_1s;
  if (header.flags & MARISA2_ENABLE_SELECT_1) {
    Error error = new_select_1s.map(mapper,
        VectorHeader{ (new_num_1s / 256) + ((new_num_1s % 256) != 0) + 1 });
    if (error) {
      return error;
    }
  }

  Vector<std::uint32_t> new_select_0s;
  if (header.flags & MARISA2_ENABLE_SELECT_0) {
    Error error = new_select_0s.map(mapper,
        VectorHeader{ (new_num_0s / 256) + ((new_num_0s % 256) != 0) + 1 });
    if (error) {
      return error;
    }
  }

  // TODO

  size_ = new_size;
  num_1s_ = new_num_1s;
  flags_ = static_cast<int>(header.flags);
  return MARISA2_SUCCESS;
}

Error BitVector::read(Reader &reader, const BitVectorHeader &header) {
  // TODO
  return MARISA2_SUCCESS;
}

Error BitVector::write(Writer &writer) {
  if (flags_ == 0) {
    return MARISA2_ERROR(MARISA2_STATE_ERROR,
                         "failed to write bit vector: not fixed");
  }

  Error error = packs_.write(writer);
  if (error) {
    return error;
  }

  error = select_1s_.write(writer);
  if (error) {
    return error;
  }

  error = select_0s_.write(writer);
  if (error) {
    return error;
  }

  return MARISA2_SUCCESS;
}

Error BitVector::push_back(bool bit) noexcept {
  if (flags_ != 0) {
    return MARISA2_ERROR(MARISA2_STATE_ERROR,
                         "failed to push bit: already fixed");
  }

  if (size_ == std::numeric_limits<std::size_t>::max()) {
    return MARISA2_ERROR(MARISA2_SIZE_ERROR, "failed to push bit: full");
  }

  if (size_ == (packs_.size() * 256)) {
    Error error = packs_.push_back(
        Pack{ { 0, 0, 0, 0 }, { 0, { 0, 0, 0, 0 } } });
    if (error) {
      return error;
    }
  }

  packs_[size_ / 256].units[(size_ / 64) % 4] |=
      std::uint64_t(bit) << (size_ % 64);
  ++size_;
  return MARISA2_SUCCESS;
}

Error BitVector::build(int flags) {
  if (flags_ != 0) {
    return MARISA2_ERROR(MARISA2_STATE_ERROR,
                         "failed to push bit: already fixed");
  }

  Error error = build_rank();
  if (error) {
    return error;
  }
  flags_ |= MARISA2_ENABLE_RANK;

  if (flags & MARISA2_ENABLE_SELECT_1) {
    Error error = build_select_1();
    if (error) {
      return error;
    }
    flags_ |= MARISA2_ENABLE_SELECT_1;
  }

  if (flags & MARISA2_ENABLE_SELECT_0) {
    Error error = build_select_0();
    if (error) {
      return error;
    }
    flags_ |= MARISA2_ENABLE_SELECT_0;
  }

  return MARISA2_SUCCESS;
}

std::size_t BitVector::select_1(std::size_t i) const {
  // TODO
  return i;
}

std::size_t BitVector::select_0(std::size_t i) const {
  // TODO
  return i;
}

Error BitVector::build_rank() noexcept {
  Error error = packs_.push_back(
      Pack{ { 0, 0, 0, 0 }, { 0, { 0, 0, 0, 0 } } });
  if (error) {
    return error;
  }

  packs_.shrink();

  for (std::size_t i = 0; i < packs_.size(); ++i) {
    Pack &pack = packs_[i];
    pack.rank.abs = static_cast<std::uint32_t>(num_1s_ >> 6);
    for (std::size_t j = 0; j < 4; ++j) {
      pack.rank.rels[j] = static_cast<std::uint8_t>(
          num_1s_ - (static_cast<std::size_t>(pack.rank.abs) << 6));
      num_1s_ += PopCount::pop_count(pack.units[j]);
    }
  }

  return MARISA2_SUCCESS;
}

Error BitVector::build_select_1() noexcept {
  Error error = select_1s_.resize(
      (num_1s_ / 256) + ((num_1s_ % 256) != 0) + 1);
  if (error) {
    return error;
  }

  std::size_t count = 0;
  for (std::size_t i = 0; i < size_; ++i) {
    if ((*this)[i]) {
      if ((count++ % 256) == 0) {
        select_1s_[count / 256] = static_cast<std::uint32_t>(i >> 6);
      }
    }
  }
  select_1s_.back() = size_ >> 6;
  return MARISA2_SUCCESS;
}

Error BitVector::build_select_0() noexcept {
  Error error = select_0s_.resize(
      (num_0s() / 256) + ((num_0s() % 256) != 0) + 1);
  if (error) {
    return error;
  }

  std::size_t count = 0;
  for (std::size_t i = 0; i < size_; ++i) {
    if (!(*this)[i]) {
      if ((count++ % 256) == 0) {
        select_0s_[count / 256] = static_cast<std::uint32_t>(i >> 6);
      }
    }
  }
  select_0s_.back() = size_ >> 6;
  return MARISA2_SUCCESS;
}

}  // namespace grimoire
}  // namespace marisa2
