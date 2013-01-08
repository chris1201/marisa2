#include "gtest/gtest.h"

#include <random>

#include <marisa2/grimoire/bit-vector.h>

class BitVectorTest : public testing::Test {
 protected:
  // This function is called before each test.
  virtual void SetUp() {
  }

  // This function is called after each test.
  virtual void TearDown() {
  }
};

TEST_F(BitVectorTest, DefaultConstructor) {
  marisa2::grimoire::BitVector bit_vector;
  ASSERT_FALSE(static_cast<bool>(bit_vector));
  ASSERT_EQ(0U, bit_vector.size());
  ASSERT_EQ(0U, bit_vector.num_1s());
  ASSERT_EQ(0U, bit_vector.num_0s());
}

TEST_F(BitVectorTest, Map) {
  // TODO
}

TEST_F(BitVectorTest, Read) {
  // TODO
}

TEST_F(BitVectorTest, Write) {
  // TODO
}

TEST_F(BitVectorTest, PushBack) {
  marisa2::Error error;
  marisa2::grimoire::BitVector bit_vector;

  error = bit_vector.push_back(true);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(1U, bit_vector.size());

  error = bit_vector.push_back(false);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(2U, bit_vector.size());

  error = bit_vector.push_back(true);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(3U, bit_vector.size());

  ASSERT_TRUE(bit_vector[0]);
  ASSERT_FALSE(bit_vector[1]);
  ASSERT_TRUE(bit_vector[2]);
}

TEST_F(BitVectorTest, Build) {
  marisa2::Error error;
  marisa2::grimoire::BitVector bit_vector;

  error = bit_vector.push_back(true);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  error = bit_vector.push_back(false);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  error = bit_vector.push_back(true);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  error = bit_vector.build();
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(MARISA2_ENABLE_RANK, bit_vector.flags());

  ASSERT_EQ(2U, bit_vector.num_1s());
  ASSERT_EQ(1U, bit_vector.num_0s());

  ASSERT_EQ(bit_vector.num_1s(), bit_vector.rank_1(bit_vector.size()));
  ASSERT_EQ(bit_vector.num_0s(), bit_vector.rank_0(bit_vector.size()));
}

TEST_F(BitVectorTest, Rank) {
  marisa2::Error error;
  marisa2::grimoire::BitVector bit_vector;

  error = bit_vector.push_back(true);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  error = bit_vector.push_back(false);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  error = bit_vector.push_back(true);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  error = bit_vector.build();
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(MARISA2_ENABLE_RANK, bit_vector.flags());

  ASSERT_EQ(0U, bit_vector.rank_1(0));
  ASSERT_EQ(1U, bit_vector.rank_1(1));
  ASSERT_EQ(1U, bit_vector.rank_1(2));
  ASSERT_EQ(2U, bit_vector.rank_1(3));

  ASSERT_EQ(0U, bit_vector.rank_0(0));
  ASSERT_EQ(0U, bit_vector.rank_0(1));
  ASSERT_EQ(1U, bit_vector.rank_0(2));
  ASSERT_EQ(1U, bit_vector.rank_0(3));
}

TEST_F(BitVectorTest, Select) {
  marisa2::Error error;
  marisa2::grimoire::BitVector bit_vector;

  error = bit_vector.push_back(true);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  error = bit_vector.push_back(false);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  error = bit_vector.push_back(true);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  error = bit_vector.build(MARISA2_ENABLE_SELECT_1 | MARISA2_ENABLE_SELECT_0);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(MARISA2_ENABLE_RANK | MARISA2_ENABLE_SELECT_1 |
            MARISA2_ENABLE_SELECT_0, bit_vector.flags());

  // TODO
}
