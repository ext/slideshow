AM_CPPFLAGS = \
	${dbus_CFLAGS} \
	-I${top_srcdir}/include

noinst_LIBRARIES = libipc.a
libipc_a_SOURCES = 

if WITH_DBUS
	libipc_a_SOURCES += dbus.cpp dbus.h
endif
