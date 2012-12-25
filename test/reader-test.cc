#include "gtest/gtest.h"

#ifdef _WIN32
# include <io.h>
#else  // _WIN32
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
#endif  // _WIN32

#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>

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
  static constexpr std::size_t NUM_BYTES = 3;
  static constexpr std::size_t NUM_WORDS = 2;
  static constexpr std::size_t NUM_DWORDS = 5;

  std::uint8_t bytes_[NUM_BYTES];
  std::uint16_t words_[NUM_WORDS];
  std::uint32_t dwords_[NUM_DWORDS];
};

void ReaderTest::CreateFile() {
  std::mt19937 random;
  for (std::size_t i = 0; i < NUM_BYTES; ++i) {
    bytes_[i] = static_cast<std::uint8_t>(random());
  }
  for (std::size_t i = 0; i < NUM_WORDS; ++i) {
    words_[i] = static_cast<std::uint16_t>(random());
  }
  for (std::size_t i = 0; i < NUM_DWORDS; ++i) {
    dwords_[i] = static_cast<std::uint32_t>(random());
  }

  std::FILE *file = std::fopen(FILENAME, "wb");
  ASSERT_NE(nullptr, file);
  FilePointerCloser closer(file);

  const std::size_t num_bytes = NUM_BYTES;
  const std::size_t num_words = NUM_WORDS;
  const std::size_t num_dwords = NUM_DWORDS;
  ASSERT_EQ(num_bytes, std::fwrite(bytes_, sizeof(std::uint8_t),
                                   NUM_BYTES, file));
  ASSERT_EQ(num_words, std::fwrite(words_, sizeof(std::uint16_t),
                                   NUM_WORDS, file));
  ASSERT_EQ(num_dwords, std::fwrite(dwords_, sizeof(std::uint32_t),
                                    NUM_DWORDS, file));
  ASSERT_EQ(0, std::fflush(file));
}

void ReaderTest::ReadData(marisa2::grimoire::Reader &reader) {
  marisa2::Error error;

  std::uint8_t byte;
  for (std::size_t i = 0; i < NUM_BYTES; ++i) {
    error = reader.read(&byte);
    ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
    ASSERT_EQ(bytes_[i], byte);
  }

  std::uint16_t word;
  for (std::size_t i = 0; i < NUM_WORDS; ++i) {
    error = reader.read(&word, 1);
    ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
    ASSERT_EQ(words_[i], word);
  }

  std::uint32_t buf[NUM_DWORDS];
  error = reader.read(buf, NUM_DWORDS);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  for (std::size_t i = 0; i < NUM_DWORDS; ++i) {
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
