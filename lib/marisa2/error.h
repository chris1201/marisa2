#ifndef MARISA2_ERROR_H
#define MARISA2_ERROR_H

// TODO: Refine error codes and comments.

enum Marisa2ErrorCode {
  // MARISA2_NO_ERROR means that a requested operation has succeeded.
  MARISA2_NO_ERROR      = 0,

  // MARISA2_STATE_ERROR means that an object was not ready for a requested
  // operation.
  MARISA2_STATE_ERROR   = 1,

  // MARISA2_NULL_ERROR means that an invalid NULL pointer has been given.
  MARISA2_NULL_ERROR    = 2,

  // MARISA2_BOUND_ERROR means that an operation has tried to access an out of
  // range address.
  MARISA2_BOUND_ERROR   = 3,

  // MARISA2_RANGE_ERROR means that an out of range value has appeared in
  // operation.
  MARISA2_RANGE_ERROR   = 4,

  // MARISA2_CODE_ERROR means that an undefined code has appeared in operation.
  MARISA2_CODE_ERROR    = 5,

  // MARISA2_SIZE_ERROR means that a size has exceeded a library limitation.
  MARISA2_SIZE_ERROR    = 6,

  // MARISA2_MEMORY_ERROR means that a memory allocation has failed.
  MARISA2_MEMORY_ERROR  = 7,

  // MARISA2_IO_ERROR means that an I/O operation has failed.
  MARISA2_IO_ERROR      = 8,

  // MARISA2_FORMAT_ERROR means that input was in invalid format.
  MARISA2_FORMAT_ERROR  = 9,

  MARISA2_UNKNOWN_ERROR = -1
};

namespace marisa2 {

typedef Marisa2ErrorCode ErrorCode;

class Error {
 public:
  constexpr Error() noexcept : message_(nullptr) {}
  constexpr Error(ErrorCode, const char *message) noexcept
    : message_(message) {}

  explicit constexpr operator bool() noexcept {
    return message_ != nullptr;
  }

  constexpr const char *message() noexcept {
    return message_;
  }

  // This function parses an error message and returns its error code.
  ErrorCode code() const noexcept;

 private:
  const char *message_;
};

}  // namespace marisa2

#define MARISA2_SUCCESS ::marisa2::Error()

// These macros are used to convert __LINE__ to a string constant.
#define MARISA2_LINE_INTEGER_TO_STRING(line) #line
#define MARISA2_LINE_TO_STRING(line) MARISA2_LINE_INTEGER_TO_STRING(line)
#define MARISA2_LINE_STRING MARISA2_LINE_TO_STRING(__LINE__)

// This macro generates an instance of Error with filename, line number,
// error code, and error message. The message format is as follows:
// "__FILE__:__LINE__: code: message"
#define MARISA2_ERROR(error_code, error_message) \
  ::marisa2::Error(error_code, __FILE__ ":" MARISA2_LINE_STRING ": " \
                   #error_code ": " error_message)

#endif  // MARISA2_ERROR_H
