SUBDIRS = transitions

noinst_LIBRARIES = libmodule_loader.a libipc.a libfsm.a
bin_PROGRAMS = slideshow xquery

warning_flags = -Wall -Wextra -Wno-unused-parameter -Wfloat-equal -Wunsafe-loop-optimizations -Wcast-qual -Wconversion -Wmissing-field-initializers -Winline

AM_LDFLAGS = -shared -module -avoid-version
AM_CPPFLAGS = -I${top_srcdir}/include -pipe ${warning_flags}

libipc_a_CPPFLAGS = -I${top_srcdir}/include
libipc_a_SOURCES = 

if WITH_DBUS
	libipc_a_CPPFLAGS += ${dbus_CFLAGS}
	libipc_a_SOURCES += IPC/dbus.cpp IPC/dbus.h
endif

libfsm_a_SOURCES = \
	state/State.h \
	state/InitialState.cpp state/InitialState.h \
	state/SwitchState.cpp state/SwitchState.h \
	state/TransitionState.cpp state/TransitionState.h \
	state/ViewState.cpp state/ViewState.h

slideshow_CPPFLAGS = ${AM_CPPFLAGS} ${libdaemon_CFLAGS} -DBUILD_DAEMON
slideshow_CFLAGS   = ${AM_CPPFLAGS}

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
	Browser.cpp ${top_srcdir}/include/Browser.h \
	DaemonApp.cpp ${top_srcdir}/include/DaemonApp.h \
	ForegroundApp.cpp ${top_srcdir}/include/ForegroundApp.h \
	Graphics.cpp ${top_srcdir}/include/Graphics.h \
	Kernel.cpp ${top_srcdir}/include/Kernel.h \
	Log.cpp ${top_srcdir}/include/Log.h\
	main.cpp \
	path.c ${top_srcdir}/include/path.h \
	browsers/dummybrowser.cpp browsers/dummybrowser.h

slideshow_LDADD = \
	libmodule_loader.a \
	libipc.a \
	libfsm.a \
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

libmodule_loader_a_SOURCES = module_loader.c ${top_srcdir}/include/module_loader.h

xquery_SOURCES = xquery.c
xquery_LDADD = ${X11_LIBS} ${xrandr_LIBS}
