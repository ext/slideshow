AM_CPPFLAGS = -I${top_srcdir}/include  -Wall -Wextra  -Wno-unused-parameter -Wfloat-equal -Wunsafe-loop-optimizations -Wcast-qual -Wconversion -Wmissing-field-initializers
AM_LDFLAGS = -shared -module -avoid-version

plugin_LTLIBRARIES = sqlite3.la

sqlite3_la_SOURCES = sqlite.c
sqlite3_la_LIBADD = ${sqlite3_LIBS}
