AM_CXXFLAGS = @AM_CXXFLAGS@ -fno-exceptions

lib_LTLIBRARIES = libmarisa2.la

noinst_LTLIBRARIES = libmarisa2_grimoire.la

libmarisa2_la_LIBADD = libmarisa2_grimoire.la

libmarisa2_la_SOURCES = \
	marisa2/error.cc \
	marisa2/utility.cc

libmarisa2_includedir = ${includedir}/marisa2
libmarisa2_include_HEADERS = \
	marisa2/error.h \
	marisa2/utility.h \
	marisa2/version.h

libmarisa2_grimoire_la_SOURCES = \
	marisa2/grimoire/mapper.cc \
	marisa2/grimoire/reader.cc \
	marisa2/grimoire/writer.cc

libmarisa2_grimoire_includedir = ${includedir}/marisa2/grimoire
libmarisa2_grimoire_include_HEADERS = \
	marisa2/grimoire/mapper.h \
	marisa2/grimoire/reader.h \
	marisa2/grimoire/writer.h

include_HEADERS = marisa2.h

.PHONY: FORCE

FORCE:

marisa2/utility.cc: marisa2/version.h

marisa2/version.h: FORCE
	TOP_SRCDIR="${top_srcdir}" "${srcdir}/version.sh"

noinst_HEADERS =
