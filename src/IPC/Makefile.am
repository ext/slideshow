AM_CFLAGS = -I${top_srcdir}/src -Wall -Wextra -Wno-unused-parameter
AM_LDFLAGS = -shared -module -avoid-version

plugin_LTLIBRARIES = signal.la

if WITH_DBUS
plugin_LTLIBRARIES += dbus.la
dbus_la_CFLAGS = ${AM_CFLAGS} ${dbus_CFLAGS}
dbus_la_LIBADD = ${dbus_LIBS}
endif
