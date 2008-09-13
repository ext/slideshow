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
slideshow_LDADD = state/libfsm.a
slideshow_LDFLAGS = \
	${libportable_LIBS} \
	${GL_LIBS} \
	${FREEIMAGE_LIBS} \
	${X11_LIBS} \
	${xxf86vm_LIBS}
