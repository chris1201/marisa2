AM_CXXFLAGS = @AM_CXXFLAGS@ -I${top_srcdir}/lib -DGTEST_HAS_PTHREAD=0

TESTS = test-all

check_PROGRAMS = ${TESTS}

test_all_SOURCES = \
	gtest/gtest-all.cc \
	gtest/gtest_main.cc \
	mapper-test.cc \
	reader-test.cc \
	writer-test.cc

test_all_LDADD = ${top_builddir}/lib/libmarisa2.la

noinst_HEADERS = \
	gtest/gtest.h
