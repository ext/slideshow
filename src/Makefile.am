SUBDIRS = transitions browsers IPC

noinst_LIBRARIES = libmodule_loader.a libfsm.a
bin_PROGRAMS = slideshow-daemon

if WITH_SDL
bin_PROGRAMS += slideshow-transition
endif

warning_flags = -Wall -Wextra -Wno-unused-parameter -Wfloat-equal -Wunsafe-loop-optimizations -Wcast-qual -Wconversion -Wmissing-field-initializers

AM_LDFLAGS = -shared -module -avoid-version
AM_CFLAGS = ${libportable_CFLAGS} ${warning_flags}

libfsm_a_SOURCES = \
	state/State.h \
	state/InitialState.cpp state/InitialState.h \
	state/SwitchState.cpp state/SwitchState.h \
	state/TransitionState.cpp state/TransitionState.h \
	state/VideoState.cpp state/VideoState.h \
	state/ViewState.cpp state/ViewState.h

slideshow_daemon_CFLAGS    = ${AM_CFLAGS} $(ARCH_FLAGS) ${libdaemon_CFLAGS} ${DevIL_CFLAGS} ${glew_CFLAGS} ${json_CFLAGS} ${CURL_CFLAGS}
slideshow_daemon_CXXFLAGS  = ${slideshow_daemon_CFLAGS}
slideshow_daemon_LDFLAGS   = ${AM_LDFLAGS} -rdynamic
slideshow_daemon_LDADD     = libmodule_loader.a libfsm.a -lltdl ${libportable_LIBS} ${libdaemon_LIBS} ${GL_LIBS} ${DevIL_LIBS} ${glew_LIBS} ${json_LIBS} ${CURL_LIBS}
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
	IPC.cpp IPC.hpp \
	Kernel.cpp Kernel.h \
	log.cpp log.h log.hpp \
	main.cpp \
	mathutils.h \
	module.h \
	opengl.c opengl.h \
	path.c path.h \
	ptr.h \
	slidelib.h \
	Transition.h \
	vector.h \
	win32.h

if WITH_SDL
slideshow_daemon_SOURCES  += backend/SDLbackend.cpp backend/SDLbackend.h
slideshow_daemon_CXXFLAGS += ${SDL_CFLAGS}
slideshow_daemon_LDADD    += ${SDL_LIBS}
endif

libmodule_loader_a_SOURCES = module_loader.c module_loader.h assembler.h

if WITH_SDL
slideshow_transition_SOURCES = slideshow_transition.cpp path.c log.cpp opengl.c
slideshow_transition_LDADD = libmodule_loader.a -lltdl ${SDL_LIBS} ${GL_LIBS} ${DevIL_LIBS} ${glew_LIBS}
endif

CLEANFILES = browser_context.cpp

.rl.cpp:
	$(AM_V_GEN)$(RAGEL) $(RAGELFLAGS) -C $< -o $@
