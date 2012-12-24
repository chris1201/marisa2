AM_CXXFLAGS = @AM_CXXFLAGS@ -I${top_srcdir}/lib

TESTS = \
	test

check_PROGRAMS = $(TESTS)

noinst_HEADERS =

test_SOURCES = test.cc
test_LDADD = ../lib/libmarisa2.la
