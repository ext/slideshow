AM_CPPFLAGS = \
	-I${top_srcdir}/include

AM_LDFLAGS = -shared -module -avoid-version

plugin_LTLIBRARIES = sqlite3.la

sqlite3_la_SOURCES = sqlite.c
sqlite3_la_LIBADD = -lsqlite3
