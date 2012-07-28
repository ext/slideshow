SUBDIRS = transitions browsers

noinst_LIBRARIES = libmodule_loader.a libipc.a libfsm.a
bin_PROGRAMS = slideshow-daemon

warning_flags = -Wall -Wextra -Wno-unused-parameter -Wfloat-equal -Wunsafe-loop-optimizations -Wcast-qual -Wconversion -Wmissing-field-initializers

AM_LDFLAGS = -shared -module -avoid-version
AM_CPPFLAGS = ${libportable_CFLAGS} -pipe $(ARCH_FLAGS) ${warning_flags}

libipc_a_CPPFLAGS = ${AM_CPPFLAGS}
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

slideshow_daemon_CPPFLAGS = ${AM_CPPFLAGS} ${libdaemon_CFLAGS}
slideshow_daemon_CFLAGS   = ${AM_CPPFLAGS}
slideshow_daemon_LDFLAGS  = ${AM_LDFLAGS} -rdynamic

if WITH_DBUS
slideshow_daemon_CPPFLAGS += ${dbus_CFLAGS}
endif

slideshow_daemon_SOURCES = \
	backend/factory.cpp backend/platform.h \
	argument_parser.c argument_parser.h \
	browser_context.rl \
	Browser.cpp Browser.h \
	curl_local.c curl_local.h \
	DaemonApp.cpp DaemonApp.h \
	exception.cpp exception.h \
	ForegroundApp.cpp ForegroundApp.h \
	gl.h \
	graphics.cpp graphics.h \
	IPC.h \
	Kernel.cpp Kernel.h \
	log_base.cpp log_base.h\
	Log.cpp Log.h\
	main.cpp \
	mathutils.h \
	module.h \
	OS.h \
	path.c path.h \
	ptr.h \
	slidelib.h \
	Transition.h \
	vector.h \
	win32.h

slideshow_daemon_LDADD = \
	libmodule_loader.a \
	libipc.a \
	libfsm.a \
	-lltdl \
	-lcurl \
	-ljson \
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
slideshow_daemon_SOURCES += backend/SDLbackend.cpp backend/SDLbackend.h
endif

libmodule_loader_a_SOURCES = module_loader.c module_loader.h assembler.h

CLEANFILES = browser_context.cpp

.rl.cpp:
	$(AM_V_GEN)$(RAGEL) $(RAGELFLAGS) -C $< -o $@
