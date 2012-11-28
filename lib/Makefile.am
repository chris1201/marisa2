AM_CXXFLAGS = -Wall -Weffc++ -Wextra
#AM_CXXFLAGS = -Wall -Weffc++ -Wextra -D_DEBUG

lib_LTLIBRARIES = libmarisa2.la

noinst_LTLIBRARIES = libmarisa2_grimoire.la

libmarisa2_la_SOURCES =
libmarisa2_la_LIBADD = libmarisa2_grimoire.la

libmarisa2_includedir = ${includedir}/marisa2
libmarisa2_include_HEADERS = \
	marisa2.h

libmarisa2_grimoire_la_SOURCES = \
	marisa2/grimoire/time.cc

libmarisa2_grimoire_includedir = ${includedir}/marisa2/grimoire
libmarisa2_grimoire_include_HEADERS = \
	marisa2/grimoire/time.h

noinst_HEADERS =
