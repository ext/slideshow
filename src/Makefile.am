SUBDIRS = IPC state transitions

lib_LTLIBRARIES = libslide.la
bin_PROGRAMS = slideshow xquery slidetool

if GCCPARSER
.cpp.o:
	$(CXXCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $< 2>&1 | gccparser
	mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po
endif

slideshow_CPPFLAGS = \
	${dbus_CFLAGS} \
	${libdaemon_CFLAGS} \
	-I${top_srcdir}/include \
	-pipe -Wall -Wextra -Wno-unused-parameter -Wfloat-equal -Wunsafe-loop-optimizations -Wcast-qual -Wconversion -Wmissing-field-initializers -Winline

slideshow_SOURCES = \
	${top_srcdir}/include/Browser.h \
	${top_srcdir}/include/config.h \
	${top_srcdir}/include/ErrorCodes.h \
	${top_srcdir}/include/Exceptions.h \
	${top_srcdir}/include/IPC.h \
	${top_srcdir}/include/OS.h \
	${top_srcdir}/include/Transition.h \
	${top_srcdir}/include/argument_parser.h \
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
	module_loader.cpp \
	OS_Linux.cpp \
	browsers/dummybrowser.cpp browsers/dummybrowser.h

slideshow_LDADD = \
	-lslide \
	IPC/libipc.a \
	state/libfsm.a \
	-lltdl \
	${libportable_LIBS} \
	${libdaemon_LIBS} \
	${dbus_LIBS} \
	${GL_LIBS} \
	${FREEIMAGE_LIBS} \
	${X11_LIBS} \
	${xxf86vm_LIBS} \
	${xrandr_LIBS}

if MYSQL
slideshow_SOURCES += browsers/mysqlbrowser.cpp browsers/mysqlbrowser.h
slideshow_LDADD += ${MYSQL_LIBS}
endif

libslide_la_CFLAGS = -D_GNU_SOURCE -pipe -Wall -Wextra ${wand_CFLAGS}
libslide_la_SOURCES = slidelib.c
libslide_la_LIBADD = ${json_LIBS} ${wand_LIBS}
libslide_la_LDFLAGS = -shared -avoid-version

xquery_CFLAGS = -pipe -Wall -Wextra
xquery_SOURCES = xquery.c
xquery_LDADD = ${X11_LIBS} ${xrandr_LIBS}

slidetool_CFLAGS = -pipe -Wall -Wextra
slidetool_SOURCES = slidetool.c
slidetool_LDADD = -lslide
