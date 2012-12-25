#include "gtest/gtest.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef _WIN32
# include <io.h>
#else  // _WIN32
# include <unistd.h>
#endif  // _WIN32

#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include <marisa2/grimoire/reader.h>

class ReaderTest : public testing::Test {
 protected:
  static constexpr const char *FILENAME = "reader-test.tmp";

  // This function is called before each test.
  virtual void SetUp() {
  }

  // This function is called after each test.
  virtual void TearDown() {
    std::remove(FILENAME);
  }

  class FilePointerCloser {
   public:
    explicit FilePointerCloser(std::FILE *file) : file_(file) {}
    ~FilePointerCloser() {
      if (file_ != nullptr) {
        std::fclose(file_);
      }
    }

   private:
    std::FILE *file_;
  };

  class FileDescriptorCloser {
   public:
    explicit FileDescriptorCloser(int fd) : fd_(fd) {}
    ~FileDescriptorCloser() {
      if (fd_ != -1) {
#ifdef _WIN32
        ::_close(fd_);
#else  // _WIN32
        ::close(fd_);
#endif  // _WIN32
      }
    }

   private:
    int fd_;
  };

  void CreateFile();
  void ReadData(marisa2::grimoire::Reader &reader);

 private:
  static constexpr std::size_t MIN_NUM_OBJS = 1 << 12;
  static constexpr std::size_t MAX_NUM_OBJS = 1 << 16;

  std::vector<std::uint8_t> bytes_;
  std::vector<std::uint16_t> words_;
  std::vector<std::uint32_t> dwords_;

  static std::mt19937 random_;
};

std::mt19937 ReaderTest::random_;

void ReaderTest::CreateFile() {
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

  std::FILE *file = std::fopen(FILENAME, "wb");
  ASSERT_NE(nullptr, file);
  FilePointerCloser closer(file);

  ASSERT_EQ(bytes_.size(),
            std::fwrite(&*bytes_.begin(), sizeof(std::uint8_t),
                        bytes_.size(), file));
  ASSERT_EQ(words_.size(),
            std::fwrite(&*words_.begin(), sizeof(std::uint16_t),
                        words_.size(), file));
  ASSERT_EQ(dwords_.size(),
            std::fwrite(&*dwords_.begin(), sizeof(std::uint32_t),
                        dwords_.size(), file));

  ASSERT_EQ(0, std::fflush(file));
}

void ReaderTest::ReadData(marisa2::grimoire::Reader &reader) {
  marisa2::Error error;

  std::uint8_t byte;
  for (std::size_t i = 0; i < bytes_.size(); ++i) {
    error = reader.read(&byte);
    ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
    ASSERT_EQ(bytes_[i], byte);
  }

  std::uint16_t word;
  for (std::size_t i = 0; i < words_.size(); ++i) {
    error = reader.read(&word, 1);
    ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
    ASSERT_EQ(words_[i], word);
  }

  std::vector<std::uint32_t> buf(dwords_.size());
  error = reader.read(&*buf.begin(), dwords_.size());
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  for (std::size_t i = 0; i < dwords_.size(); ++i) {
    ASSERT_EQ(dwords_[i], buf[i]);
  }

  error = reader.read(&byte);
  ASSERT_EQ(MARISA2_IO_ERROR, error.code()) << error.message();
}

TEST_F(ReaderTest, DefaultConstructor) {
  marisa2::grimoire::Reader reader;
  ASSERT_FALSE(static_cast<bool>(reader));
}

TEST_F(ReaderTest, MoveConstructor) {
  marisa2::Error error;

  marisa2::grimoire::Reader reader;
  error = reader.open(std::cin);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_TRUE(static_cast<bool>(reader));

  marisa2::grimoire::Reader reader2(std::move(reader));
  ASSERT_FALSE(static_cast<bool>(reader));
  ASSERT_TRUE(static_cast<bool>(reader2));
}

TEST_F(ReaderTest, MoveOperator) {
  marisa2::Error error;

  marisa2::grimoire::Reader reader;
  error = reader.open(std::cin);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_TRUE(static_cast<bool>(reader));

  marisa2::grimoire::Reader reader2;
  reader2 = std::move(reader);
  ASSERT_FALSE(static_cast<bool>(reader));
  ASSERT_TRUE(static_cast<bool>(reader2));
}

TEST_F(ReaderTest, Filename) {
  marisa2::Error error;

  marisa2::grimoire::Reader reader;
  error = reader.open(static_cast<const char *>(nullptr));
  ASSERT_EQ(MARISA2_NULL_ERROR, error.code()) << error.message();
  error = reader.open("");
  ASSERT_EQ(MARISA2_IO_ERROR, error.code()) << error.message();

  CreateFile();

  error = reader.open(FILENAME);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ReadData(reader);
}

TEST_F(ReaderTest, FileDescriptor) {
  marisa2::Error error;

  marisa2::grimoire::Reader reader;
  error = reader.open(-1);
  ASSERT_EQ(MARISA2_IO_ERROR, error.code()) << error.message();

  CreateFile();

#ifdef _WIN32
  const int fd = ::_open(FILENAME, _O_RDONLY | _O_BINARY);
#else  // _WIN32
  const int fd = ::open(FILENAME, O_RDONLY);
#endif  // _WIN32
  ASSERT_NE(-1, fd);
  FileDescriptorCloser closer(fd);

  error = reader.open(fd);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ReadData(reader);
}

TEST_F(ReaderTest, FilePointer) {
  marisa2::Error error;

  marisa2::grimoire::Reader reader;
  error = reader.open(static_cast<std::FILE *>(nullptr));
  ASSERT_EQ(MARISA2_NULL_ERROR, error.code()) << error.message();

  CreateFile();

  std::FILE *file = std::fopen(FILENAME, "rb");
  ASSERT_NE(nullptr, file);
  FilePointerCloser closer(file);

  error = reader.open(file);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ReadData(reader);
}

TEST_F(ReaderTest, InputStream) {
  marisa2::Error error;

  std::ifstream invalid_stream;
  invalid_stream.get();
  ASSERT_FALSE(static_cast<bool>(invalid_stream));

  marisa2::grimoire::Reader reader;
  error = reader.open(invalid_stream);
  ASSERT_EQ(MARISA2_IO_ERROR, error.code()) << error.message();

  CreateFile();

  std::ifstream file(FILENAME, std::ios::binary);
  ASSERT_TRUE(static_cast<bool>(file));

  error = reader.open(file);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  ReadData(reader);
}
