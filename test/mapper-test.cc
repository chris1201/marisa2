#include "gtest/gtest.h"

#include <cstdint>
#include <fstream>
#include <random>
#include <sstream>
#include <vector>

#include <marisa2/grimoire/mapper.h>

class MapperTest : public testing::Test {
 protected:
  static constexpr const char *FILENAME = "mapper-test.tmp";

  // This function is called before each test.
  virtual void SetUp() {
  }

  // This function is called after each test.
  virtual void TearDown() {
    std::remove(FILENAME);
  }

  void WriteData(std::ostream &stream);
  void ReadData(marisa2::grimoire::Mapper &mapper);

 private:
  static constexpr std::size_t MIN_NUM_OBJS = 1 << 12;
  static constexpr std::size_t MAX_NUM_OBJS = 1 << 16;

  std::vector<std::uint8_t> bytes_;
  std::vector<std::uint16_t> words_;
  std::vector<std::uint32_t> dwords_;

  static std::mt19937 random_;
};

std::mt19937 MapperTest::random_;

void MapperTest::WriteData(std::ostream &stream) {
  bytes_.resize(MIN_NUM_OBJS
      + (random_() % (MIN_NUM_OBJS - MIN_NUM_OBJS + 1)));
  words_.resize(MIN_NUM_OBJS
      + (random_() % (MIN_NUM_OBJS - MIN_NUM_OBJS + 1)));
  dwords_.resize(MIN_NUM_OBJS
      + (random_() % (MIN_NUM_OBJS - MIN_NUM_OBJS + 1)));

  for (std::size_t i = 0; i < bytes_.size(); ++i) {
    bytes_[i] = static_cast<std::uint8_t>(random_());
  }
  for (std::size_t i = 0; i < words_.size(); ++i) {
    words_[i] = static_cast<std::uint16_t>(random_());
  }
  for (std::size_t i = 0; i < dwords_.size(); ++i) {
    dwords_[i] = static_cast<std::uint32_t>(random_());
  }

  stream.write(reinterpret_cast<const char *>(&*bytes_.begin()),
               sizeof(std::uint8_t) * bytes_.size());
  stream.write(reinterpret_cast<const char *>(&*words_.begin()),
               sizeof(std::uint16_t) * words_.size());
  stream.write(reinterpret_cast<const char *>(&*dwords_.begin()),
               sizeof(std::uint32_t) * dwords_.size());

  stream.flush();
  ASSERT_TRUE(static_cast<bool>(stream));
}

void MapperTest::ReadData(marisa2::grimoire::Mapper &mapper) {
  marisa2::Error error;

  const std::uint8_t *byte;
  for (std::size_t i = 0; i < bytes_.size(); ++i) {
    error = mapper.map(&byte);
    ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
    ASSERT_EQ(bytes_[i], *byte);
  }

  const std::uint16_t *word;
  for (std::size_t i = 0; i < words_.size(); ++i) {
    error = mapper.map(&word, 1);
    ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
    ASSERT_EQ(words_[i], *word);
  }

  const std::uint32_t *dwords;

  error = mapper.map(&dwords, dwords_.size());
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  for (std::size_t i = 0; i < dwords_.size(); ++i) {
    ASSERT_EQ(dwords_[i], dwords[i]);
  }
}

TEST_F(MapperTest, DefaultConstructor) {
  marisa2::grimoire::Mapper mapper;
  ASSERT_TRUE(!mapper);
}

TEST_F(MapperTest, MoveConstructor) {
  marisa2::Error error;

  char buf[1];

  marisa2::grimoire::Mapper mapper;
  error = mapper.open(buf, sizeof(buf));
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_TRUE(static_cast<bool>(mapper));

  marisa2::grimoire::Mapper mapper2(std::move(mapper));
  ASSERT_FALSE(static_cast<bool>(mapper));
  ASSERT_TRUE(static_cast<bool>(mapper2));
}

TEST_F(MapperTest, MoveOperator) {
  marisa2::Error error;

  char buf[1];

  marisa2::grimoire::Mapper mapper;
  error = mapper.open(buf, sizeof(buf));
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_TRUE(static_cast<bool>(mapper));

  marisa2::grimoire::Mapper mapper2;
  mapper2 = std::move(mapper);
  ASSERT_FALSE(static_cast<bool>(mapper));
  ASSERT_TRUE(static_cast<bool>(mapper2));
}

TEST_F(MapperTest, Filename) {
  marisa2::Error error;

  marisa2::grimoire::Mapper mapper;
  error = mapper.open(static_cast<const char *>(nullptr));
  ASSERT_EQ(MARISA2_NULL_ERROR, error.code()) << error.message();
  error = mapper.open("");
  ASSERT_EQ(MARISA2_IO_ERROR, error.code()) << error.message();

  std::ofstream file(FILENAME, std::ios::binary);
  ASSERT_TRUE(static_cast<bool>(file));

  WriteData(file);
  file.close();

  error = mapper.open(FILENAME);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ReadData(mapper);
}

TEST_F(MapperTest, Address) {
  marisa2::Error error;

  marisa2::grimoire::Mapper mapper;
  error = mapper.open(nullptr, 1);
  ASSERT_EQ(MARISA2_NULL_ERROR, error.code()) << error.message();
  error = mapper.open("", 0);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  std::stringstream stream;
  WriteData(stream);

  std::string buf = stream.str();

  error = mapper.open(buf.data(), buf.size());
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ReadData(mapper);
}
