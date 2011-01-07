SUBDIRS = transitions

noinst_LIBRARIES = libmodule_loader.a libipc.a libfsm.a
bin_PROGRAMS = slideshow
lib_LTLIBRARIES = xquery.la 

warning_flags = -Wall -Wextra -Wno-unused-parameter -Wfloat-equal -Wunsafe-loop-optimizations -Wcast-qual -Wconversion -Wmissing-field-initializers -Winline

AM_LDFLAGS = -shared -module -avoid-version
AM_CPPFLAGS = -I${top_srcdir}/include -pipe $(ARCH_FLAGS) ${warning_flags}

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
	state/VideoState.cpp state/VideoState.h \
	state/ViewState.cpp state/ViewState.h

slideshow_CPPFLAGS = ${AM_CPPFLAGS} ${libdaemon_CFLAGS} -DBUILD_DAEMON
slideshow_CFLAGS   = ${AM_CPPFLAGS}

if WITH_DBUS
slideshow_CPPFLAGS += ${dbus_CFLAGS}
endif

slideshow_SOURCES = \
	${top_srcdir}/include/backend/platform.h \
	${top_srcdir}/include/argument_parser.h \
	${top_srcdir}/include/assembler.h \
	${top_srcdir}/include/gl.h \
	${top_srcdir}/include/IPC.h \
	${top_srcdir}/include/mathutils.h \
	${top_srcdir}/include/module.h \
	${top_srcdir}/include/OS.h \
	${top_srcdir}/include/ptr.h \
	${top_srcdir}/include/slidelib.h \
	${top_srcdir}/include/Transition.h \
	${top_srcdir}/include/vector.h \
	${top_srcdir}/include/win32.h \
	backend/factory.cpp \
	browsers/dummybrowser.cpp browsers/dummybrowser.h \
	argument_parser.c \
	browser_context.rl \
	Browser.cpp ${top_srcdir}/include/Browser.h \
	DaemonApp.cpp ${top_srcdir}/include/DaemonApp.h \
	exception.cpp ${top_srcdir}/include/exception.h \
	ForegroundApp.cpp ${top_srcdir}/include/ForegroundApp.h \
	Graphics.cpp ${top_srcdir}/include/Graphics.h \
	Kernel.cpp ${top_srcdir}/include/Kernel.h \
	Log.cpp ${top_srcdir}/include/Log.h\
	main.cpp \
	path.c ${top_srcdir}/include/path.h

slideshow_LDADD = \
	libmodule_loader.a \
	libipc.a \
	libfsm.a \
	-lltdl \
	-lGLEW \
	${libportable_LIBS} \
	${libdaemon_LIBS} \
	${dbus_LIBS} \
	${GL_LIBS} \
	${DEVIL_LIBS} \
	${SDL_LIBS} \
	${X11_LIBS} \
	${xxf86vm_LIBS} \
	${xrandr_LIBS}

if WITH_SDL
slideshow_SOURCES += backend/SDLbackend.cpp ${top_srcdir}/include/backend/SDLbackend.h
endif

if WITH_MYSQL
slideshow_SOURCES += browsers/mysqlbrowser.cpp
slideshow_LDADD += ${MYSQL_LIBS}
endif

if WITH_SQLITE3
slideshow_SOURCES += browsers/sqlite.cpp
slideshow_LDADD += ${sqlite3_LIBS}
endif

libmodule_loader_a_SOURCES = module_loader.c ${top_srcdir}/include/module_loader.h

xquery_la_SOURCES = xquery.c
xquery_la_CFLAGS = -Wall -Wstrict-prototypes -I/usr/include/python2.6
xquery_la_LDFLAGS = -module -export-dynamic ${LDFLAGS}
xquery_la_LIBADD = ${X11_LIBS} ${xrandr_LIBS}

SUFFIXES = .rl

.rl.cpp:
	$(AM_V_GEN)$(RAGEL) $(RAGELFLAGS) -C $< -o $@
