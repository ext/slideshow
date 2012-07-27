AM_CPPFLAGS = -I${top_srcdir}/include -I${top_srcdir}/src ${libportable_CFLAGS} -D_GNU_SOURCE -Wall
AM_LDFLAGS = -shared -module -avoid-version

plugin_LTLIBRARIES = frontend.la
frontend_la_LIBADD = -lcurl -ljson

if WITH_SQLITE3
plugin_LTLIBRARIES+=sqlite3.la
sqlite3_la_SOURCES = sqlite.c
sqlite3_la_LIBADD = ${sqlite3_LIBS}
endif

if WITH_DIRECTORY
plugin_LTLIBRARIES+=directory.la
endif

if WITH_MYSQL
plugin_LTLIBRARIES += mysql.la
mysql_la_SOURCES = mysql.c
mysql_la_LIBADD = ${MYSQL_LIBS}
endif
