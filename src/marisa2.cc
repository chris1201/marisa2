#include <getopt.h>

#include <iostream>

#include <marisa2/utility.h>

namespace {

int print_version() {
  std::cout << "marisa2 " << marisa2::Utility::version()
            << std::endl;
  return 0;
}

int print_help() {
  std::cout << "Usage: marisa2 [OPTION]... [FILE]...\n"
               "\n"
               "Options:\n"
               "  -h, --help     print help\n"
               "  -v, --version  print version\n"
            << std::endl;
  return 0;
}

}  // namespace

int main(int argc, char *argv[]) {
  std::ios::sync_with_stdio(false);

  constexpr struct option options[] = {
    { "help", 0, nullptr, 'h' },
    { "version", 0, nullptr, 'v' },
    { nullptr, 0, nullptr, '\0' }
  };

  int label;
  while ((label = ::getopt_long(argc, argv, "vh", options, nullptr)) != -1) {
    switch (label) {
      case 'h': {
        return print_help();
      }
      case 'v': {
        return print_version();
      }
      default: {
        return 1;
      }
    }
  }

  return 0;
}
