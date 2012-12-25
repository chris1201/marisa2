#ifndef MARISA2_FEATURES_H
#define MARISA2_FEATURES_H

#ifdef MARISA2_BUILD_DLL
# define MARISA2_DLL_EXPORT __declspec(dllexport)
#else  // MARISA2_BUILD_DLL
# ifdef _WIN32
#  define MARISA2_DLL_EXPORT __declspec(dllimport)
# else  // _WIN32
#  define MARISA2_DLL_EXPORT
# endif  // _WIN32
#endif  // MARISA2_BUILD_DLL

#endif  // MARISA2_FEATURES_H
