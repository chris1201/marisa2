#ifndef MARISA2_UTILITY_H
#define MARISA2_UTILITY_H

#include "features.h"

namespace marisa2 {

class MARISA2_DLL_EXPORT Utility {
 public:
  Utility() = delete;
  ~Utility() = delete;

  // This function returns the library version.
  // For example, "pre-alpha-64-gc3aead7".
  static const char *version() noexcept;
};

}  // namespace marisa2

#endif  // MARISA2_UTILITY_H
