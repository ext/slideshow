SUBDIRS = browsers IPC state transitions

AM_CPPFLAGS = \
	${dbus_CFLAGS} \
	-I${top_srcdir}/include

bin_PROGRAMS = slideshow

slideshow_SOURCES = \
	Browser.h
	config.h
	ErrorCodes.h
	Exceptions.h
	IPC.h
	OS.h
	Transition.h
	argument_parser.c argument_parser.h \
	Graphics.cpp Graphics.h \
	Kernel.cpp Kernel.h \
	Log.cpp Log.h\
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
