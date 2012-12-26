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

#include <marisa2/grimoire/writer.h>

class WriterTest : public testing::Test {
 protected:
  static constexpr const char *FILENAME = "writer-test.tmp";

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

  void CreateFile(marisa2::grimoire::Writer &writer);
  void ReadData();

 private:
  static constexpr std::size_t MIN_NUM_OBJS = 1 << 12;
  static constexpr std::size_t MAX_NUM_OBJS = 1 << 16;

  std::vector<std::uint8_t> bytes_;
  std::vector<std::uint16_t> words_;
  std::vector<std::uint32_t> dwords_;

  static std::mt19937 random_;
};

std::mt19937 WriterTest::random_;

void WriterTest::CreateFile(marisa2::grimoire::Writer &writer) {
  marisa2::Error error;

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

  for (std::size_t i = 0; i < bytes_.size(); ++i) {
    error = writer.write(bytes_[i]);
    ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  }
  for (std::size_t i = 0; i < bytes_.size(); ++i) {
    error = writer.write(&words_[i], 1);
    ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  }
  error = writer.write(&*dwords_.begin(), dwords_.size());
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  error = writer.flush();
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
}

void WriterTest::ReadData() {
  std::ifstream file(FILENAME, std::ios::binary);
  ASSERT_TRUE(static_cast<bool>(file));

  std::uint8_t byte;
  for (std::size_t i = 0; i < bytes_.size(); ++i) {
    file.read(reinterpret_cast<char *>(&byte), sizeof(byte));
    ASSERT_TRUE(static_cast<bool>(file));
    ASSERT_EQ(bytes_[i], byte);
  }

  std::uint16_t word;
  for (std::size_t i = 0; i < words_.size(); ++i) {
    file.read(reinterpret_cast<char *>(&word), sizeof(word));
    ASSERT_TRUE(static_cast<bool>(file));
    ASSERT_EQ(words_[i], word);
  }

  std::uint32_t dword;
  for (std::size_t i = 0; i < dwords_.size(); ++i) {
    file.read(reinterpret_cast<char *>(&dword), sizeof(dword));
    ASSERT_TRUE(static_cast<bool>(file));
    ASSERT_EQ(dwords_[i], dword);
  }

  file.get();
  ASSERT_FALSE(static_cast<bool>(file));
}

TEST_F(WriterTest, DefaultConstructor) {
  marisa2::grimoire::Writer writer;
  ASSERT_TRUE(!writer);
}

TEST_F(WriterTest, MoveConstructor) {
  marisa2::Error error;

  marisa2::grimoire::Writer writer;
  error = writer.open(std::cout);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_TRUE(static_cast<bool>(writer));

  marisa2::grimoire::Writer writer2(std::move(writer));
  ASSERT_FALSE(static_cast<bool>(writer));
  ASSERT_TRUE(static_cast<bool>(writer2));
}

TEST_F(WriterTest, MoveOperator) {
  marisa2::Error error;

  marisa2::grimoire::Writer writer;
  error = writer.open(std::cout);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();
  ASSERT_TRUE(static_cast<bool>(writer));

  marisa2::grimoire::Writer writer2;
  writer2 = std::move(writer);
  ASSERT_FALSE(static_cast<bool>(writer));
  ASSERT_TRUE(static_cast<bool>(writer2));
}

TEST_F(WriterTest, Filename) {
  marisa2::Error error;

  marisa2::grimoire::Writer writer;
  error = writer.open(static_cast<const char *>(nullptr));
  ASSERT_EQ(MARISA2_NULL_ERROR, error.code()) << error.message();
  error = writer.open("");
  ASSERT_EQ(MARISA2_IO_ERROR, error.code()) << error.message();

  error = writer.open(FILENAME);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  CreateFile(writer);
  ReadData();
}

TEST_F(WriterTest, FileDescriptor) {
  marisa2::Error error;

  marisa2::grimoire::Writer writer;
  error = writer.open(-1);
  ASSERT_EQ(MARISA2_IO_ERROR, error.code()) << error.message();

#ifdef _WIN32
  const int fd = ::_open(FILENAME, _O_CREAT | _O_WRONLY | _O_BINARY, 0666);
#else  // _WIN32
  const int fd = ::open(FILENAME, O_CREAT | O_WRONLY, 0666);
#endif  // _WIN32
  ASSERT_NE(-1, fd);
  FileDescriptorCloser closer(fd);

  error = writer.open(fd);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  CreateFile(writer);
  ReadData();
}

TEST_F(WriterTest, FilePointer) {
  marisa2::Error error;

  marisa2::grimoire::Writer writer;
  error = writer.open(static_cast<std::FILE *>(nullptr));
  ASSERT_EQ(MARISA2_NULL_ERROR, error.code()) << error.message();

  std::FILE *file = std::fopen(FILENAME, "wb");
  ASSERT_NE(nullptr, file);
  FilePointerCloser closer(file);

  error = writer.open(file);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  CreateFile(writer);
  ReadData();
}

TEST_F(WriterTest, OutputStream) {
  marisa2::Error error;

  std::ofstream invalid_stream;
  invalid_stream.put('\0');
  ASSERT_FALSE(static_cast<bool>(invalid_stream));

  marisa2::grimoire::Writer writer;
  error = writer.open(invalid_stream);
  ASSERT_EQ(MARISA2_IO_ERROR, error.code()) << error.message();

  std::ofstream file(FILENAME, std::ios::binary);
  ASSERT_TRUE(static_cast<bool>(file));

  error = writer.open(file);
  ASSERT_EQ(MARISA2_NO_ERROR, error.code()) << error.message();

  CreateFile(writer);
  ReadData();
}
