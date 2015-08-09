noinst_LIBRARIES = libmodule_loader.a libfsm.a
noinst_LTLIBRARIES = libslideshow_core.la
bin_PROGRAMS = slideshow-daemon
plugin_LTLIBRARIES =

DATAFILES =
EXTRA_DIST =
BUILT_SOURCES =
CLEANFILES =

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
	app/daemon.cpp app/daemon.hpp \
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
	transitions/transition.h

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

#
# Transitions
#

TRANSITION_CFLAGS = -I${top_srcdir}/src -Itransitions ${warning_flags}
TRANSITION_LDFLAGS = -shared -module -avoid-version

plugin_LTLIBRARIES += fade.la spin.la vfade.la noise.la

fade_la_SOURCES   = transitions/fade.c transitions/fade_files.dpl transitions/fade.frag
fade_la_DATAFILES = transitions/fade_files.dpl
fade_la_CFLAGS    = ${TRANSITION_CFLAGS}
fade_la_LDFLAGS   = ${TRANSITION_LDFLAGS}

vfade_la_SOURCES   = transitions/vfade.c transitions/vfade_files.dpl transitions/vfade.frag
vfade_la_DATAFILES = transitions/vfade_files.dpl
vfade_la_CFLAGS    = ${TRANSITION_CFLAGS}
vfade_la_LDFLAGS   = ${TRANSITION_LDFLAGS}

noise_la_SOURCES   = transitions/noise.c transitions/noise_files.dpl transitions/noise.frag
noise_la_DATAFILES = transitions/noise_files.dpl
noise_la_CFLAGS    = ${TRANSITION_CFLAGS}
noise_la_LDFLAGS   = ${TRANSITION_LDFLAGS}

spin_la_SOURCES   = transitions/spin.c
spin_la_CFLAGS    = ${TRANSITION_CFLAGS}
spin_la_LDFLAGS   = ${TRANSITION_LDFLAGS}

DATAFILES += ${fade_la_DATAFILES} ${vfade_la_DATAFILES} ${noise_la_DATAFILES}
BUILT_SOURCES += $(DATAFILES:.dpl=.c)
CLEANFILES += $(DATAFILES:.dpl=.c) $(DATAFILES:.dpl=.h) $(addprefix $(DEPDIR)/, $(notdir $(DATAFILES:=.d)))

EXTRA_DIST += transitions/default.c transitions/fsquad.vert

#
# Browsers
#

BROWSER_CFLAGS = -I${top_srcdir}/src ${warning_flags}
BROWSER_LDFLAGS = -shared -module -avoid-version

plugin_LTLIBRARIES += frontend.la
frontend_la_SOURCES = browsers/frontend.c
frontend_la_CFLAGS  = ${BROWSER_CFLAGS} ${json_CFLAGS}
frontend_la_LDFLAGS = ${BROWSER_LDFLAGS}
frontend_la_LIBADD  = -lcurl ${json_LIBS}

if WITH_SQLITE3
plugin_LTLIBRARIES += sqlite3.la
sqlite3_la_SOURCES  = browsers/sqlite.c
sqlite3_la_CFLAGS   = ${BROWSER_CFLAGS}
sqlite3_la_LDFLAGS  = ${BROWSER_LDFLAGS}
sqlite3_la_LIBADD   = ${sqlite3_LIBS}
endif

if WITH_DIRECTORY
plugin_LTLIBRARIES   += directory.la
directory_la_SOURCES  = browsers/directory.c
directory_la_CFLAGS   = ${BROWSER_CFLAGS}
directory_la_LDFLAGS  = ${BROWSER_LDFLAGS}
endif

if WITH_MYSQL
plugin_LTLIBRARIES += mysql.la
mysql_la_SOURCES    = browsers/mysql.c
mysql_la_CFLAGS     = ${BROWSER_CFLAGS} ${MYSQL_CLIENT_CFLAGS}
mysql_la_LDFLAGS    = ${BROWSER_LDFLAGS}
mysql_la_LIBADD     = ${MYSQL_CLIENT_LIBS}
endif

#
# IPC
#

IPC_CFLAGS = -I${top_srcdir}/src ${warning_flags}
IPC_LDFLAGS = -shared -module -avoid-version

plugin_LTLIBRARIES += signal.la
signal_la_SOURCES = IPC/signal.c
signal_la_CFLAGS = ${IPC_CFLAGS}
signal_la_LDFLAGS = ${IPC_LDFLAGS}
signal_la_LIBADD = ${dbus_LIBS}

if WITH_DBUS
plugin_LTLIBRARIES += dbus.la
dbus_la_SOURCES = IPC/dbus.c
dbus_la_CFLAGS = ${IPC_CFLAGS} ${dbus_CFLAGS}
dbus_la_LDFLAGS = ${IPC_LDFLAGS}
dbus_la_LIBADD = ${dbus_LIBS}
endif

# Misc

DISTCLEANFILES = browsers/context.cpp

.rl.cpp:
	$(AM_V_GEN)$(RAGEL) $(RAGELFLAGS) -C $< -o $@

.dpl.c:
	@test x"$(@D)" = x || $(MKDIR_P) "$(@D)"
	@$(MKDIR_P) "$(DEPDIR)"
	$(AM_V_GEN)${DATAPACKER} -f $< -s $(dir $<) -d $(DEPDIR)/$(notdir $<).d -e $(basename $@).h -o $@

-include $(addprefix $(DEPDIR)/, $(notdir $(DATAFILES:=.d)))
