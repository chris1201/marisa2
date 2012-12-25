#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([marisa2], [pre-alpha], [syata@acm.org])
AC_CONFIG_SRCDIR([lib/marisa2.h])
AM_INIT_AUTOMAKE

LT_INIT([win32-dll])
AC_PROG_CXX
AC_PROG_INSTALL

AC_CONFIG_MACRO_DIR([m4])

AM_CXXFLAGS="-Wall -Wextra -std=c++0x"

AC_CANONICAL_HOST

case "${host}" in
  *-*-mingw*|*-*-cygwin*)
    AM_CXXFLAGS="${AM_CXXFLAGS} -DMARISA2_BUILD_DLL"
    AM_LDFLAGS="-no-undefined"
    ;;
  *)
    AM_LDFLAGS=""
    ;;
esac

AC_MSG_CHECKING([whether to use popcnt (SSE4.2)])
AC_ARG_ENABLE([popcnt],
              [AS_HELP_STRING([--enable-popcnt],
                              [use popcnt (SSE4.2) [default=no]])],
              [],
              [enable_popcnt="no"])
if test "x${enable_popcnt}" != "xno"
then
  AM_CXXFLAGS="$AM_CXXFLAGS -DMARISA2_USE_POPCNT -msse4.2"
  enable_popcnt="yes"
fi
AC_MSG_RESULT([${enable_popcnt}])

AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

AC_CONFIG_FILES([Makefile
                 marisa2.pc
                 lib/Makefile
                 src/Makefile
                 test/Makefile])
AC_OUTPUT

echo
echo "${PACKAGE_NAME} ${PACKAGE_VERSION} configuration:"
echo "-------------------------------"
echo "  HOST:      ${host}"
echo "  CXX:       ${CXX}"
echo "  CXXFLAGS:  ${AM_CXXFLAGS} ${CXXFLAGS}"
echo "  LDFLAGS:   ${AM_LDFLAGS} ${LDFLAGS}"
echo "  PREFIX:    ${prefix}"
echo
