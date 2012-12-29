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

  constexpr std::uint8_t operator[](std::size_t i) noexcept {
    return static_cast<std::uint8_t>(value_ >> (i * 8));
  }

  static constexpr std::uint8_t pop_count(std::uint64_t x) noexcept {
#ifdef MARISA2_USE_POPCNT
    return static_cast<std::uint8_t>(::__builtin_popcountll(x));
#else  // MARISA2_USE_POPCNT
    return static_cast<std::uint8_t>(pop_count_1st(x) >> 56);
#endif  // MARISA2_USE_POPCNT
  }

 private:
  std::uint64_t value_;

  static constexpr std::uint64_t MULTIPLIER =
      static_cast<std::uint64_t>(0x0101010101010101ULL);

  static constexpr std::uint64_t pop_count_1st(std::uint64_t x) noexcept {
    return pop_count_2nd(
        (x & (0x55 * MULTIPLIER)) + ((x & (0xAA * MULTIPLIER)) >> 1));
  }
  static constexpr std::uint64_t pop_count_2nd(std::uint64_t x) noexcept {
    return pop_count_3rd(
        (x & (0x33 * MULTIPLIER)) + ((x & (0xCC * MULTIPLIER)) >> 2));
  }
  static constexpr std::uint64_t pop_count_3rd(std::uint64_t x) noexcept {
    return pop_count_4th(
        (x & (0x0F * MULTIPLIER)) + ((x & (0xF0 * MULTIPLIER)) >> 4));
  }
  static constexpr std::uint64_t pop_count_4th(std::uint64_t x) noexcept {
    return x * MULTIPLIER;
  }
};

}  // namespace grimoire
}  // namespace marisa2

#endif  // MARISA2_GRIMOIRE_POP_COUNT_H
