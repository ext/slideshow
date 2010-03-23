SUBDIRS = IPC state transitions

noinst_LIBRARIES = libmodule_loader.a
bin_PROGRAMS = slideshow xquery

common_flags = 
	${dbus_CFLAGS} \
	${libdaemon_CFLAGS} \
	-pipe -Wall -Wextra -Wno-unused-parameter -Wfloat-equal -Wunsafe-loop-optimizations -Wcast-qual -Wconversion -Wmissing-field-initializers -Winline

slideshow_CPPFLAGS = -I${top_srcdir}/include ${common_flags} -DBUILD_DAEMON
slideshow_CFLAGS   = -I${top_srcdir}/include ${common_flags}

slideshow_SOURCES = \
	${top_srcdir}/include/Browser.h \
	${top_srcdir}/include/config.h \
	${top_srcdir}/include/ErrorCodes.h \
	${top_srcdir}/include/Exceptions.h \
	${top_srcdir}/include/IPC.h \
	${top_srcdir}/include/OS.h \
	${top_srcdir}/include/Transition.h \
	${top_srcdir}/include/argument_parser.h \
	backend/factory.cpp \
	backend/SDLbackend.cpp \
	argument_parser.c \
	browser_context.cpp \
	exception.cpp \
	Browser.cpp ${top_srcdir}/include/Browser.h \
	DaemonApp.cpp ${top_srcdir}/include/DaemonApp.h \
	ForegroundApp.cpp ${top_srcdir}/include/ForegroundApp.h \
	Graphics.cpp ${top_srcdir}/include/Graphics.h \
	Kernel.cpp ${top_srcdir}/include/Kernel.h \
	Log.cpp ${top_srcdir}/include/Log.h\
	main.cpp \
	path.c ${top_srcdir}/include/path.h \
	OS_Linux.cpp \
	browsers/dummybrowser.cpp browsers/dummybrowser.h

slideshow_LDADD = \
	libmodule_loader.a \
	IPC/libipc.a \
	state/libfsm.a \
	-lltdl \
	${libportable_LIBS} \
	${libdaemon_LIBS} \
	${dbus_LIBS} \
	${GL_LIBS} \
	${DEVIL_LIBS} \
	${SDL_LIBS} \
	${X11_LIBS} \
	${xxf86vm_LIBS} \
	${xrandr_LIBS}

if MYSQL
	slideshow_SOURCES += browsers/mysqlbrowser.cpp browsers/mysqlbrowser.h
	slideshow_LDADD += ${MYSQL_LIBS}
endif

libmodule_loader_a_CFLAGS = -D_GNU_SOURCE -pipe -Wall -Wextra -I${top_srcdir}/include
libmodule_loader_a_SOURCES = module_loader.c ${top_srcdir}/include/module_loader.h

xquery_CFLAGS = -pipe -Wall -Wextra
xquery_SOURCES = xquery.c
xquery_LDADD = ${X11_LIBS} ${xrandr_LIBS}
