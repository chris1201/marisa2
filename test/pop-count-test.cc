#include "gtest/gtest.h"

#include <random>

#include <marisa2/grimoire/pop-count.h>

class PopCountTest : public testing::Test {
 protected:
  // This function is called before each test.
  virtual void SetUp() {
  }

  // This function is called after each test.
  virtual void TearDown() {
  }

  static constexpr int NUM_VALUES = 1 << 16;

  static std::mt19937 random_;
};

std::mt19937 PopCountTest::random_;

TEST_F(PopCountTest, DefaultConstructor) {
  marisa2::grimoire::PopCount pop_count;
  pop_count = marisa2::grimoire::PopCount(0);
  ASSERT_EQ(0U, pop_count.low_32());
}

TEST_F(PopCountTest, Constructor) {
  for (int i = 0; i < NUM_VALUES; ++i) {
    std::uint32_t src = random_();
    while (src == 0) {
      src = random_();
    }

    marisa2::grimoire::PopCount pop_count(src);
    ASSERT_EQ(static_cast<std::uint8_t>(::__builtin_popcount(src << 24)),
              pop_count.low_8());
    ASSERT_EQ(static_cast<std::uint8_t>(::__builtin_popcount(src << 16)),
              pop_count.low_16());
    ASSERT_EQ(static_cast<std::uint8_t>(::__builtin_popcount(src << 8)),
              pop_count.low_24());
    ASSERT_EQ(static_cast<std::uint8_t>(::__builtin_popcount(src)),
              pop_count.low_32());
  }
}

TEST_F(PopCountTest, CopyConstructor) {
  marisa2::grimoire::PopCount pop_count(255);
  ASSERT_EQ(8U, pop_count.low_32());

  marisa2::grimoire::PopCount pop_count2(pop_count);
  ASSERT_EQ(8U, pop_count2.low_32());
}

TEST_F(PopCountTest, CopyOperator) {
  marisa2::grimoire::PopCount pop_count(65535);
  ASSERT_EQ(16U, pop_count.low_32());

  marisa2::grimoire::PopCount pop_count2;
  pop_count2 = pop_count;
  ASSERT_EQ(16U, pop_count2.low_32());
}

TEST_F(PopCountTest, PopCount) {
  for (int i = 0; i < NUM_VALUES; ++i) {
    std::uint32_t src = random_();
    while (src == 0) {
      src = random_();
    }

    ASSERT_EQ(static_cast<std::uint8_t>(::__builtin_popcount(src)),
              marisa2::grimoire::PopCount::pop_count(src));
  }
}
