SUBDIRS = IPC state transitions

AM_CPPFLAGS = \
	${dbus_CFLAGS} \
	-I${top_srcdir}/include \
	-pipe -Wall -Wextra -Wno-unused-parameter -Wnon-virtual-dtor -Wfloat-equal -Wunsafe-loop-optimizations -Wcast-qual -Wconversion -Wmissing-field-initializers -Winline

if GCCPARSER
.cpp.o:
	$(CXXCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $< 2>&1 | gccparser
	mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po
endif

bin_PROGRAMS = slideshow

slideshow_SOURCES = \
	${top_srcdir}/include/Browser.h \
	${top_srcdir}/include/config.h \
	${top_srcdir}/include/ErrorCodes.h \
	${top_srcdir}/include/Exceptions.h \
	${top_srcdir}/include/IPC.h \
	${top_srcdir}/include/OS.h \
	${top_srcdir}/include/Transition.h \
	argument_parser.c ${top_srcdir}/include/argument_parser.h \
	browser_context.cpp \
	Browser.cpp ${top_srcdir}/include/Browser.h \
	Graphics.cpp ${top_srcdir}/include/Graphics.h \
	Kernel.cpp ${top_srcdir}/include/Kernel.h \
	Log.cpp ${top_srcdir}/include/Log.h\
	main.cpp \
	module_loader.cpp \
	OS_Linux.cpp \
	browsers/dummybrowser.cpp browsers/dummybrowser.h

slideshow_LDADD = \
	IPC/libipc.a \
	state/libfsm.a \
	transitions/libtransition.a

slideshow_LDFLAGS = \
	-lltdl \
	${libportable_LIBS} \
	${dbus_LIBS} \
	${GL_LIBS} \
	${FREEIMAGE_LIBS} \
	${X11_LIBS} \
	${xxf86vm_LIBS}

if MYSQL
slideshow_SOURCES += browsers/mysqlbrowser.cpp browsers/mysqlbrowser.h
slideshow_LDFLAGS += ${MYSQL_LIBS}
endif
