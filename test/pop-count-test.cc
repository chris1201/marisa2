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

  static std::mt19937_64 random_;
};

std::mt19937_64 PopCountTest::random_;

TEST_F(PopCountTest, DefaultConstructor) {
  marisa2::grimoire::PopCount pop_count;
  pop_count = marisa2::grimoire::PopCount(0);
  ASSERT_EQ(0U, pop_count[7]);
}

TEST_F(PopCountTest, Constructor) {
  for (int i = 0; i < NUM_VALUES; ++i) {
    std::uint64_t src = random_();
    marisa2::grimoire::PopCount pop_count(src);
    for (std::uint8_t j = 0; j < 8; ++j) {
      std::uint64_t masked_src = src << (56 - (j * 8));
      ASSERT_EQ(static_cast<std::uint8_t>(::__builtin_popcountll(masked_src)),
                pop_count[j]);
    }
  }
}

TEST_F(PopCountTest, CopyConstructor) {
  marisa2::grimoire::PopCount pop_count(255);
  ASSERT_EQ(8U, pop_count[7]);

  marisa2::grimoire::PopCount pop_count2(pop_count);
  ASSERT_EQ(8U, pop_count2[7]);
}

TEST_F(PopCountTest, CopyOperator) {
  marisa2::grimoire::PopCount pop_count(65535);
  ASSERT_EQ(16U, pop_count[7]);

  marisa2::grimoire::PopCount pop_count2;
  pop_count2 = pop_count;
  ASSERT_EQ(16U, pop_count2[7]);
}

TEST_F(PopCountTest, PopCount) {
  for (int i = 0; i < NUM_VALUES; ++i) {
    std::uint64_t src = random_();
    ASSERT_EQ(static_cast<std::uint8_t>(::__builtin_popcountll(src)),
              marisa2::grimoire::PopCount::pop_count(src));
  }
}
