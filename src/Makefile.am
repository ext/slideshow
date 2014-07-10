SUBDIRS = transitions browsers IPC

noinst_LIBRARIES = libmodule_loader.a libfsm.a
noinst_LTLIBRARIES = libslideshow_core.la
bin_PROGRAMS = slideshow-daemon

if WITH_SDL
bin_PROGRAMS += slideshow-transition
endif

warning_flags = -Wall -Wextra -Wno-unused-parameter -Wfloat-equal -Wunsafe-loop-optimizations -Wcast-qual -Wconversion -Wmissing-field-initializers

AM_CFLAGS = ${libportable_CFLAGS} ${warning_flags}

libfsm_a_SOURCES = \
	state/state.cpp state/state.hpp \
	state/initial.cpp state/initial.hpp \
	state/switch.cpp state/switch.hpp \
	state/transition.cpp state/transition.hpp \
	state/video.cpp state/video.hpp \
	state/view.cpp state/view.hpp

slideshow_daemon_CFLAGS    = ${AM_CFLAGS} $(ARCH_FLAGS) ${libdaemon_CFLAGS} ${json_CFLAGS} ${CURL_CFLAGS}
slideshow_daemon_CXXFLAGS  = ${slideshow_daemon_CFLAGS}
slideshow_daemon_LDFLAGS   = ${AM_LDFLAGS} -rdynamic
slideshow_daemon_LDADD     = libmodule_loader.a libslideshow_core.la libfsm.a -lltdl ${datapack_LIBS} ${libportable_LIBS} ${libdaemon_LIBS} ${json_LIBS} ${CURL_LIBS}
slideshow_daemon_SOURCES = \
	app/daemon.cpp src/daemon.hpp \
	app/foreground.cpp app/foreground.hpp \
	app/main.cpp \
	backend/factory.cpp backend/platform.h \
	browsers/context.rl browsers/browser.cpp browsers/browser.h \
	core/argument_parser.c core/argument_parser.h \
	core/kernel.cpp core/kernel.hpp \
	core/mathutils.h \
	core/slidelib.h \
	core/vector.h \
	core/win32.h \
	IPC/IPC.cpp IPC/IPC.hpp \
	ptr.h \
	transitions/ransition.h

if WITH_SDL
slideshow_daemon_SOURCES  += backend/SDLbackend.cpp backend/SDLbackend.h
slideshow_daemon_CXXFLAGS += ${SDL_CFLAGS}
slideshow_daemon_LDADD    += ${SDL_LIBS}
endif

libslideshow_core_la_CFLAGS    = ${AM_CFLAGS} ${GL_CFLAGS} ${DevIL_CFLAGS} ${glew_CFLAGS} ${CURL_CFLAGS}
libslideshow_core_la_CXXFLAGS  = ${libslideshow_core_a_CFLAGS}
libslideshow_core_la_LIBADD    = ${GL_LIBS} ${DevIL_LIBS} ${glew_LIBS} ${CURL_LIBS}
libslideshow_core_la_SOURCES   = \
	core/asprintf.c core/asprintf.h \
	core/curl_local.c core/curl_local.h \
	core/exception.cpp core/exception.hpp \
	core/graphics.cpp core/graphics.h \
	core/log.cpp core/log.h core/log.hpp \
	core/opengl.c core/opengl.h \
	core/path.c core/path.h

libmodule_loader_a_SOURCES = core/module_loader.c core/module_loader.h core/assembler.h core/module.h

if WITH_SDL
slideshow_transition_SOURCES = app/slideshow_transition.cpp
slideshow_transition_LDFLAGS = ${AM_LDFLAGS} -rdynamic
slideshow_transition_LDADD = libmodule_loader.a libslideshow_core.la -lm -lltdl -lX11 ${datapack_LIBS} ${SDL_LIBS}
endif

DISTCLEANFILES = src/browser/context.cpp

.rl.cpp:
	$(AM_V_GEN)$(RAGEL) $(RAGELFLAGS) -C $< -o $@
