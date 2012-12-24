#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([marisa2], [0.0.1], [syata@acm.org])
AC_CONFIG_SRCDIR([lib/marisa2.h])
AM_INIT_AUTOMAKE

# Checks for programs.
LT_INIT
AC_PROG_CXX
AC_PROG_INSTALL

AC_CONFIG_MACRO_DIR([m4])

AM_CXXFLAGS="-Wall -Wextra -std=c++0x"
AM_LDFLAGS="-no-undefined"

AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_MSG_CHECKING([whether to use popcnt (SSE4.2)])
AC_ARG_ENABLE([popcnt],
              [AS_HELP_STRING([--enable-popcnt],
                              [use popcnt (SSE4.2) [default=no]])],
              [],
              [enable_popcnt="no"])
if test "x${enable_popcnt}" != "xno"
then
  CPPFLAGS="$CPPFLAGS -DMARISA_USE_POPCNT -msse4.2"
  enable_popcnt="yes"
fi
AC_MSG_RESULT([${enable_popcnt}])

AC_CONFIG_FILES([Makefile
                 marisa2.pc
                 lib/Makefile
                 src/Makefile
                 test/Makefile])
AC_OUTPUT

echo
echo "${PACKAGE_NAME} ${PACKAGE_VERSION} configuration:"
echo "-----------------------"
echo "  CXX:       ${CXX}"
echo "  CXXFLAGS:  ${AM_CXXFLAGS} ${CXXFLAGS}"
echo "  LDFLAGS:   ${AM_LDFLAGS} ${LDFLAGS}"
echo "  LIBS:      ${LIBS}"
echo "  PREFIX:    ${prefix}"
echo
