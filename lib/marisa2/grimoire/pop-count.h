#ifndef MARISA2_GRIMOIRE_POP_COUNT_H
#define MARISA2_GRIMOIRE_POP_COUNT_H

#include <cstdint>

namespace marisa2 {
namespace grimoire {

class PopCount {
 public:
  PopCount() = default;
  explicit constexpr PopCount(std::uint32_t x) noexcept
    : value_(pop_count_1st(x)) {}

  explicit constexpr operator bool() noexcept {
    return value_ != 0;
  }

  constexpr std::uint8_t low_8() noexcept {
    return static_cast<std::uint8_t>(value_);
  }
  constexpr std::uint8_t low_16() noexcept {
    return static_cast<std::uint8_t>(value_ >> 8);
  }
  constexpr std::uint8_t low_24() noexcept {
    return static_cast<std::uint8_t>(value_ >> 16);
  }
  constexpr std::uint8_t low_32() noexcept {
    return static_cast<std::uint8_t>(value_ >> 24);
  }

  static constexpr std::uint8_t pop_count(std::uint32_t x) noexcept {
#ifdef MARISA2_USE_POPCNT
    return static_cast<std::uint8_t>(::__builtin_popcount(x));
#else  // MARISA2_USE_POPCNT
    return static_cast<std::uint8_t>(pop_count_1st(x) >> 24);
#endif  // MARISA2_USE_POPCNT
  }

 private:
  std::uint32_t value_;

  static constexpr std::uint32_t pop_count_1st(std::uint32_t x) noexcept {
    return pop_count_2nd((x & 0x55555555U) + ((x & 0xAAAAAAAAU) >> 1));
  }
  static constexpr std::uint32_t pop_count_2nd(std::uint32_t x) noexcept {
    return pop_count_3rd((x & 0x33333333U) + ((x & 0xCCCCCCCCU) >> 2));
  }
  static constexpr std::uint32_t pop_count_3rd(std::uint32_t x) noexcept {
    return pop_count_last((x & 0x0F0F0F0FU) + ((x & 0xF0F0F0F0U) >> 4));
  }
  static constexpr std::uint32_t pop_count_last(std::uint32_t x) noexcept {
    return x * 0x01010101U;
  }
};

}  // namespace grimoire
}  // namespace marisa2

#endif  // MARISA2_GRIMOIRE_POP_COUNT_H
