#ifndef MARISA2_UTILITY_H
#define MARISA2_UTILITY_H

namespace marisa2 {

class Utility {
 public:
  Utility() = delete;
  ~Utility() = delete;

  static const char *version() noexcept;
};

}  // namespace marisa2

#endif  // MARISA2_UTILITY_H
