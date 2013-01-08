#ifndef MARISA2_GRIMOIRE_POP_COUNT_H
#define MARISA2_GRIMOIRE_POP_COUNT_H

#include <cstdint>

namespace marisa2 {
namespace grimoire {

class PopCount {
 public:
  PopCount() = default;
  explicit constexpr PopCount(std::uint64_t x) noexcept
    : value_(pop_count_1st(x)) {}

  explicit constexpr operator bool() noexcept {
    return value_ != 0;
  }

  // pop_count[i] returns the number of 1s in the least significant
  // ((i + 1) * 8) bits. If i > 7, the result is undefined.
  constexpr std::uint8_t operator[](std::size_t i) noexcept {
    return static_cast<std::uint8_t>(value_ >> (i << 3));
  }

  // ::__builtin_popcountll() uses popcnt if -msse4.2 is specified.
  // Note: ::__builtin_popcountll() is not constexpr on Mac OSX.
//  static constexpr std::uint8_t pop_count(std::uint64_t x) noexcept {
  static std::uint8_t pop_count(std::uint64_t x) noexcept {
#ifdef MARISA2_USE_POPCNT
    return static_cast<std::uint8_t>(::__builtin_popcountll(x));
#else  // MARISA2_USE_POPCNT
    return static_cast<std::uint8_t>(pop_count_1st(x) >> 56);
#endif  // MARISA2_USE_POPCNT
  }

 private:
  std::uint64_t value_;

  // See http://en.wikipedia.org/wiki/Hamming_weight for details.
  static constexpr std::uint64_t MASK_55 = 0x5555555555555555ULL;
  static constexpr std::uint64_t MASK_33 = 0x3333333333333333ULL;
  static constexpr std::uint64_t MASK_0F = 0x0F0F0F0F0F0F0F0FULL;
  static constexpr std::uint64_t MASK_01 = 0x0101010101010101ULL;

  static constexpr std::uint64_t pop_count_1st(std::uint64_t x) noexcept {
    return pop_count_2nd(x - ((x >> 1) & MASK_55));
  }
  static constexpr std::uint64_t pop_count_2nd(std::uint64_t x) noexcept {
    return pop_count_3rd((x & MASK_33) + ((x >> 2) & MASK_33));
  }
  static constexpr std::uint64_t pop_count_3rd(std::uint64_t x) noexcept {
    return pop_count_4th((x + (x >> 4)) & MASK_0F);
  }
  static constexpr std::uint64_t pop_count_4th(std::uint64_t x) noexcept {
    return x * MASK_01;
  }
};

}  // namespace grimoire
}  // namespace marisa2

#endif  // MARISA2_GRIMOIRE_POP_COUNT_H
