#include "error.h"

#include <cstring>

namespace marisa2 {

ErrorCode Error::code() const {
  if (!message_) {
    return MARISA2_NO_ERROR;
  }

  const char *code_string = std::strstr(message_, ": ");
  if (!code_string) {
    return MARISA2_UNKNOWN_ERROR;
  }
  code_string += 2;

#define MARISA2_TEST_ERROR_CODE(error_code) \
  if (std::memcmp(code_string, #error_code, sizeof(#error_code) - 1) == 0) { \
    return error_code; \
  }

  MARISA2_TEST_ERROR_CODE(MARISA2_NO_ERROR);
  MARISA2_TEST_ERROR_CODE(MARISA2_STATE_ERROR);
  MARISA2_TEST_ERROR_CODE(MARISA2_NULL_ERROR);
  MARISA2_TEST_ERROR_CODE(MARISA2_BOUND_ERROR);
  MARISA2_TEST_ERROR_CODE(MARISA2_RANGE_ERROR);
  MARISA2_TEST_ERROR_CODE(MARISA2_CODE_ERROR);
  MARISA2_TEST_ERROR_CODE(MARISA2_SIZE_ERROR);
  MARISA2_TEST_ERROR_CODE(MARISA2_MEMORY_ERROR);
  MARISA2_TEST_ERROR_CODE(MARISA2_IO_ERROR);
  MARISA2_TEST_ERROR_CODE(MARISA2_FORMAT_ERROR);

#undef MARISA2_TEST_ERROR_CODE

  return MARISA2_UNKNOWN_ERROR;
}

}  // namespace marisa2
