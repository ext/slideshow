SUBDIRS = transitions browsers IPC

noinst_LIBRARIES = libmodule_loader.a libfsm.a
noinst_LTLIBRARIES = libslideshow_base.la
bin_PROGRAMS = slideshow-daemon

if WITH_SDL
bin_PROGRAMS += slideshow-transition
endif

warning_flags = -Wall -Wextra -Wno-unused-parameter -Wfloat-equal -Wunsafe-loop-optimizations -Wcast-qual -Wconversion -Wmissing-field-initializers

AM_CFLAGS = ${libportable_CFLAGS} ${warning_flags}

libfsm_a_SOURCES = \
	state/state.hpp \
	state/initial.cpp state/initial.hpp \
	state/switch.cpp state/switch.hpp \
	state/transition.cpp state/transition.hpp \
	state/video.cpp state/video.hpp \
	state/view.cpp state/view.hpp

slideshow_daemon_CFLAGS    = ${AM_CFLAGS} $(ARCH_FLAGS) ${libdaemon_CFLAGS} ${json_CFLAGS} ${CURL_CFLAGS}
slideshow_daemon_CXXFLAGS  = ${slideshow_daemon_CFLAGS}
slideshow_daemon_LDFLAGS   = ${AM_LDFLAGS} -rdynamic
slideshow_daemon_LDADD     = libmodule_loader.a libslideshow_base.la libfsm.a -lltdl ${datapack_LIBS} ${libportable_LIBS} ${libdaemon_LIBS} ${json_LIBS} ${CURL_LIBS}
slideshow_daemon_SOURCES = \
	backend/factory.cpp backend/platform.h \
	argument_parser.c argument_parser.h \
	browser_context.rl \
	Browser.cpp Browser.h \
	DaemonApp.cpp DaemonApp.h \
	ForegroundApp.cpp ForegroundApp.h \
	gl.h \
	graphics.cpp graphics.h \
	IPC.cpp IPC.hpp \
	kernel.cpp kernel.hpp \
	main.cpp \
	mathutils.h \
	module.h \
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

libslideshow_base_la_CFLAGS    = ${AM_CFLAGS} ${GL_CFLAGS} ${DevIL_CFLAGS} ${glew_CFLAGS} ${CURL_CFLAGS}
libslideshow_base_la_CXXFLAGS  = ${libslideshow_base_a_CFLAGS}
libslideshow_base_la_LIBADD    = ${GL_LIBS} ${DevIL_LIBS} ${glew_LIBS} ${CURL_LIBS}
libslideshow_base_la_SOURCES   = \
	curl_local.c curl_local.h \
	exception.cpp exception.h \
	graphics.cpp graphics.h \
	log.cpp log.h log.hpp \
	opengl.c opengl.h \
	path.c path.h

libmodule_loader_a_SOURCES = module_loader.c module_loader.h assembler.h

if WITH_SDL
slideshow_transition_SOURCES = slideshow_transition.cpp
slideshow_transition_LDFLAGS = ${AM_LDFLAGS} -rdynamic
slideshow_transition_LDADD = libmodule_loader.a libslideshow_base.la -lm -lltdl -lX11 ${datapack_LIBS} ${SDL_LIBS}
endif

CLEANFILES = browser_context.cpp

.rl.cpp:
	$(AM_V_GEN)$(RAGEL) $(RAGELFLAGS) -C $< -o $@
