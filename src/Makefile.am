SUBDIRS = browsers IPC state transitions

AM_CPPFLAGS = \
	${dbus_CFLAGS} \
	-I${top_srcdir}/include

bin_PROGRAMS = slideshow

slideshow_SOURCES = \
	argument_parser.c \
	Graphics.cpp \
	Kernel.cpp \
	Log.cpp \
	main.cpp \
	OS_Linux.cpp
slideshow_LDADD = \
	browsers/libbrowser.a \
	IPC/libipc.a \
	state/libfsm.a \
	transitions/libtransition.a
slideshow_LDFLAGS = \
	${libportable_LIBS} \
	${dbus_LIBS} \
	${GL_LIBS} \
	${FREEIMAGE_LIBS} \
	${MYSQL_LIBS} \
	${X11_LIBS} \
	${xxf86vm_LIBS}
