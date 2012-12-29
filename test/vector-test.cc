#include "gtest/gtest.h"

#include <limits>
#include <random>
#include <sstream>
#include <vector>

#include <marisa2/grimoire/vector.h>

class VectorTest : public testing::Test {
 protected:
  // This function is called before each test.
  virtual void SetUp() {
  }

  // This function is called after each test.
  virtual void TearDown() {
  }
};

TEST_F(VectorTest, DefaultConstructor) {
  marisa2::grimoire::Vector<int> vector;
  ASSERT_FALSE(static_cast<bool>(vector));
  ASSERT_EQ(0U, vector.size());
  ASSERT_EQ(0U, vector.capacity());

  marisa2::grimoire::VectorHeader header = vector.header();
  ASSERT_EQ(sizeof(int), header.obj_size);
  ASSERT_EQ(0U, header.num_objs);
}

TEST_F(VectorTest, Map) {
  marisa2::Error error;
  marisa2::grimoire::Vector<int> vector;

  marisa2::grimoire::Mapper mapper;
  error = vector.map(mapper, marisa2::grimoire::VectorHeader{ sizeof(int), 0 });
  ASSERT_EQ(MARISA2_STATE_ERROR, error.code()) << error.message();

  int values[3] = { 123, 456, 789 };
  error = mapper.open(values, sizeof(values));
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  error = vector.map(mapper, marisa2::grimoire::VectorHeader{ sizeof(int), 1 });
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(1U, vector.size());
  ASSERT_EQ(123, vector[0]);

  error = vector.map(mapper, marisa2::grimoire::VectorHeader{ sizeof(int), 2 });
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(2U, vector.size());
  ASSERT_EQ(456, vector[0]);
  ASSERT_EQ(789, vector[1]);

  error = vector.map(mapper, marisa2::grimoire::VectorHeader{ sizeof(int), 1 });
  ASSERT_EQ(MARISA2_BOUND_ERROR, error.code()) << error.message();
}

TEST_F(VectorTest, Read) {
  marisa2::Error error;
  marisa2::grimoire::Vector<int> vector;

  marisa2::grimoire::Reader reader;
  error = vector.read(reader, marisa2::grimoire::VectorHeader{ sizeof(int), 0 });
  ASSERT_EQ(MARISA2_STATE_ERROR, error.code()) << error.message();

  int values[3] = { 123, 456, 789 };
  std::stringstream stream;
  stream.write(reinterpret_cast<const char *>(values), sizeof(values));
  ASSERT_TRUE(static_cast<bool>(stream));

  error = reader.open(stream);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  error = vector.read(reader, marisa2::grimoire::VectorHeader{ sizeof(int), 1 });
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(1U, vector.size());
  ASSERT_EQ(123, vector[0]);

  error = vector.read(reader, marisa2::grimoire::VectorHeader{ sizeof(int), 2 });
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(2U, vector.size());
  ASSERT_EQ(456, vector[0]);
  ASSERT_EQ(789, vector[1]);

  error = vector.read(reader, marisa2::grimoire::VectorHeader{ sizeof(int), 1 });
  ASSERT_EQ(MARISA2_IO_ERROR, error.code()) << error.message();
}

TEST_F(VectorTest, Write) {
  marisa2::Error error;
  marisa2::grimoire::Vector<int> vector;

  error = vector.push_back(234);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  error = vector.push_back(567);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  error = vector.push_back(890);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  std::stringstream stream;
  marisa2::grimoire::Writer writer;
  error = writer.open(stream);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  error = vector.write(writer);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_TRUE(static_cast<bool>(stream));

  int values[3];
  stream.read(reinterpret_cast<char *>(values), sizeof(values));
  ASSERT_TRUE(static_cast<bool>(stream));

  ASSERT_EQ(234, values[0]);
  ASSERT_EQ(567, values[1]);
  ASSERT_EQ(890, values[2]);
}

TEST_F(VectorTest, IO) {
  marisa2::Error error;
  marisa2::grimoire::Vector<int> vector;

  error = vector.push_back(987);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  error = vector.push_back(654);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  error = vector.push_back(321);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  std::stringstream stream;

  marisa2::grimoire::Writer writer;
  error = writer.open(stream);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  error = writer.write(vector.header());
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  error = vector.write(writer);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  marisa2::grimoire::Reader reader;
  error = reader.open(stream);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  marisa2::grimoire::VectorHeader header;
  error = reader.read(&header);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  error = vector.read(reader, header);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(3U, vector.size());
  ASSERT_EQ(987, vector[0]);
  ASSERT_EQ(654, vector[1]);
  ASSERT_EQ(321, vector[2]);
}

TEST_F(VectorTest, PushBack) {
  marisa2::Error error;
  marisa2::grimoire::Vector<int> vector;

  error = vector.push_back(1);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  error = vector.push_back(10);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  error = vector.push_back(100);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(3U, vector.size());
  ASSERT_EQ(4U, vector.capacity());

  ASSERT_EQ(1, vector[0]);
  ASSERT_EQ(10, vector[1]);
  ASSERT_EQ(100, vector[2]);
}

//TEST_F(VectorTest, PopBack) {
//  marisa2::Error error;
//  marisa2::grimoire::Vector<int> vector;

//  error = vector.pop_back();
//  ASSERT_EQ(MARISA2_STATE_ERROR, error.code()) << error.message();

//  error = vector.push_back(123);
//  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

//  error = vector.pop_back();
//  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
//  ASSERT_EQ(0U, vector.size());

//  error = vector.pop_back();
//  ASSERT_EQ(MARISA2_STATE_ERROR, error.code()) << error.message();
//}

TEST_F(VectorTest, Resize) {
  marisa2::Error error;
  marisa2::grimoire::Vector<int> vector;

  error = vector.resize(10);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(10U, vector.size());
  ASSERT_EQ(10U, vector.capacity());

  error = vector.resize(15);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(15U, vector.size());
  ASSERT_EQ(20U, vector.capacity());

  error = vector.resize(5);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(5U, vector.size());
  ASSERT_EQ(20U, vector.capacity());

  error = vector.resize(21);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(21U, vector.size());
  ASSERT_EQ(40U, vector.capacity());

  error = vector.resize(100);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(100U, vector.size());
  ASSERT_EQ(100U, vector.capacity());

  error = vector.resize(std::numeric_limits<std::size_t>::max());
  ASSERT_EQ(MARISA2_RANGE_ERROR, error.code()) << error.message();
}

TEST_F(VectorTest, ResizeAndFill) {
  marisa2::Error error;
  marisa2::grimoire::Vector<int> vector;

  error = vector.resize(3, 123);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(3U, vector.size());
  ASSERT_EQ(3U, vector.capacity());

  ASSERT_EQ(123, vector[0]);
  ASSERT_EQ(123, vector[1]);
  ASSERT_EQ(123, vector[2]);

  error = vector.resize(5, 456);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(5U, vector.size());
  ASSERT_EQ(6U, vector.capacity());

  ASSERT_EQ(456, vector[3]);
  ASSERT_EQ(456, vector[4]);

  error = vector.resize(1 << 10, 789);
  ASSERT_EQ(1U << 10, vector.size());
  ASSERT_EQ(1U << 10, vector.capacity());
  for (std::size_t i = 5; i < vector.size(); ++i) {
    ASSERT_EQ(789, vector[i]);
  }

  error = vector.resize(std::numeric_limits<std::size_t>::max(), 123456789);
  ASSERT_EQ(MARISA2_RANGE_ERROR, error.code()) << error.message();
}


TEST_F(VectorTest, Reserve) {
  marisa2::Error error;
  marisa2::grimoire::Vector<int> vector;

  error = vector.reserve(5);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(0U, vector.size());
  ASSERT_EQ(5U, vector.capacity());

  error = vector.resize(7);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(7U, vector.size());
  ASSERT_EQ(10U, vector.capacity());

  error = vector.reserve(3);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(7U, vector.size());
  ASSERT_EQ(10U, vector.capacity());

  error = vector.reserve(100);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_EQ(7U, vector.size());
  ASSERT_EQ(100U, vector.capacity());

  error = vector.reserve(std::numeric_limits<std::size_t>::max());
  ASSERT_EQ(MARISA2_RANGE_ERROR, error.code()) << error.message();
}

TEST_F(VectorTest, Begin) {
  marisa2::Error error;
  marisa2::grimoire::Vector<int> vector;

  error = vector.resize(10);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(&vector[0], vector.begin());

  error = vector.reserve(30);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(&vector[0], vector.begin());

  error = vector.resize(100);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(&vector[0], vector.begin());
}

TEST_F(VectorTest, End) {
  marisa2::Error error;
  marisa2::grimoire::Vector<int> vector;

  error = vector.resize(10);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(vector.begin() + 10, vector.end());

  error = vector.reserve(30);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(vector.begin() + 10, vector.end());

  error = vector.resize(100);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(vector.begin() + 100, vector.end());
}

TEST_F(VectorTest, Front) {
  marisa2::Error error;
  marisa2::grimoire::Vector<int> vector;

  error = vector.resize(10);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(&vector[0], &vector.front());

  error = vector.reserve(30);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(&vector[0], &vector.front());

  error = vector.resize(100);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(&vector[0], &vector.front());
}

TEST_F(VectorTest, Back) {
  marisa2::Error error;
  marisa2::grimoire::Vector<int> vector;

  error = vector.resize(10);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(vector.begin() + 9, &vector.back());

  error = vector.reserve(30);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(vector.begin() + 9, &vector.back());

  error = vector.resize(100);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ASSERT_EQ(vector.begin() + 99, &vector.back());
}
