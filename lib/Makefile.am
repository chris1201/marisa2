lib_LTLIBRARIES = libmarisa2.la

noinst_LTLIBRARIES = libmarisa2_grimoire.la

libmarisa2_la_LIBADD = libmarisa2_grimoire.la

libmarisa2_la_SOURCES = \
	marisa2/error.cc

libmarisa2_includedir = ${includedir}/marisa2
libmarisa2_include_HEADERS = \
	marisa2/error.h \
	marisa2/version.h

libmarisa2_grimoire_la_SOURCES = \
	marisa2/grimoire/time.cc

libmarisa2_grimoire_includedir = ${includedir}/marisa2/grimoire
libmarisa2_grimoire_include_HEADERS = \
	marisa2/grimoire/time.h

include_HEADERS = marisa2.h

.PHONY: FORCE

FORCE:

marisa2/version.h: FORCE
	TOP_SRCDIR="$(top_srcdir)" "$(srcdir)/version.sh"

noinst_HEADERS =
