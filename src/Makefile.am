SUBDIRS = browsers IPC state transitions

AM_CPPFLAGS = \
	${dbus_CFLAGS} \
	-I${top_srcdir}/include

bin_PROGRAMS = slideshow

slideshow_SOURCES = \
	${top_srcdir}/include/Browser.h \
	${top_srcdir}/include/config.h \
	${top_srcdir}/include/ErrorCodes.h \
	${top_srcdir}/include/Exceptions.h \
	${top_srcdir}/include/IPC.h \
	${top_srcdir}/include/OS.h \
	${top_srcdir}/include/Transition.h \
	argument_parser.c ${top_srcdir}/include/argument_parser.h \
	Graphics.cpp ${top_srcdir}/include/Graphics.h \
	Kernel.cpp ${top_srcdir}/include/Kernel.h \
	Log.cpp ${top_srcdir}/include/Log.h\
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
