AM_CXXFLAGS = @AM_CXXFLAGS@ -I${top_srcdir}/lib

TESTS = \
	mapper-test \
	reader-test \
	writer-test

check_PROGRAMS = $(TESTS)

noinst_HEADERS =

mapper_test_SOURCES = mapper-test.cc
mapper_test_LDADD = $(top_builddir)/lib/libmarisa2.la

reader_test_SOURCES = reader-test.cc
reader_test_LDADD = $(top_builddir)/lib/libmarisa2.la

writer_test_SOURCES = writer-test.cc
writer_test_LDADD = $(top_builddir)/lib/libmarisa2.la
