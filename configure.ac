dnl Remember to bump 'setup.py' and 'slideshow/templates/index.html' as well
AC_INIT([slideshow],[0.3.4], [ext@sidvind.com])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall 1.11 subdir-objects])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([include/config.h])

AC_GNU_SOURCE
AC_PROG_CXX
AM_PROG_CC_C_O
AM_DISABLE_STATIC
AC_LIBTOOL_DLOPEN
LT_INIT
AC_PROG_CC_C99
CHECK_RAGEL([src/browser_context.cpp])
AX_LIB_CURL([],,AC_MSG_ERROR([Required library libcurl not found]))
AX_CHECK_GLU
PKG_CHECK_MODULES(glew, [glew])
PKG_CHECK_MODULES(json, [json])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/transitions/Makefile
	src/browsers/Makefile
])
AC_CONFIG_FILES([slideshow-daemon], [chmod +x slideshow-daemon])

AC_MSG_CHECKING([host architecture])
case "$host" in
	x86_64-*)
		AC_MSG_RESULT([amd64])
		AC_SUBST(ARCH_FLAGS, [-DARCH_AMD64=1]) ;;
	i[3456]86-*)
		AC_MSG_RESULT([x86])
		AC_SUBST(ARCH_FLAGS, [-DARCH_X86=1]) ;;
	*)
		AC_MSG_RESULT([unknown]) ;;
esac

dnl #######################################################################
dnl # Paths
dnl #######################################################################

pkgdatadir="${datadir}/${PACKAGE}"
AC_SUBST(pkgdatadir)
resourcedir="${pkgdatadir}/resources"
AC_SUBST(resourcedir)
plugindir="${pkgdatadir}/plugin"
AC_SUBST(plugindir)

AC_DEFINE_DIR([DATA_DIR], [pkgdatadir], [Data directory])
AC_DEFINE_DIR([RESOURCE_DIR], [resourcedir], [Resource directory])
AC_DEFINE_DIR([PLUGIN_DIR], [plugindir], [Plugin directory])

dnl #######################################################################
dnl # Check presence of functions
dnl #######################################################################

AC_FUNC_FORK

dnl #######################################################################
dnl # D-Bus
dnl #######################################################################
AC_ARG_WITH([dbus],
	[AS_HELP_STRING([--with-dbus], [support for D-Bus])],
	[],
	[with_dbus=no])

AS_IF([test "x$with_dbus" != xno],
	[
		PKG_CHECK_MODULES(dbus, [dbus-1, dbus-glib-1],
		[
			AC_DEFINE(HAVE_DBUS, 1, [Define if D-Bus is available])
			enable_dbus=yes
		],
		[
			AC_MSG_ERROR([Required D-Bus not found.])
		])
	],
	[])
	
AM_CONDITIONAL([WITH_DBUS], [test "x$with_dbus" != xno])
AC_SUBST(dbus_CFLAGS)
AC_SUBST(dbus_LIBS)

dnl #######################################################################
dnl # libportable
dnl #######################################################################
PKG_CHECK_MODULES(libportable, [libportable >= 0.2.2])
AC_SUBST(libportable_CFLAGS)
AC_SUBST(libportable_LIBS)

dnl #######################################################################
dnl # libdaemon
dnl #######################################################################
PKG_CHECK_MODULES(libdaemon, [libdaemon >= 0.13],
	[
		AC_DEFINE(HAVE_LIBDAEMON, 1, [Define if libdaemon is available])
	])

AC_SUBST(libdaemon_CFLAGS)
AC_SUBST(libdaemon_LIBS)

dnl #######################################################################
dnl # DevIL
dnl #######################################################################
AC_CHECK_HEADERS([IL/il.h], [HAVE_DEVIL=yes], [HAVE_DEVIL=no])
if test x$HAVE_DEVIL = xno; then
   AC_MSG_ERROR(DevIL development libraries not found);
else
   DEVIL_LIBS="-lIL -lILU -lILUT"
   DEVIL_CFLAGS=""
fi

AC_SUBST(DEVIL_LIBS)
AC_SUBST(DEVIL_CFLAGS)

dnl #######################################################################
dnl # Xlib
dnl #######################################################################
AC_ARG_WITH([xlib],
	[AS_HELP_STRING([--with-xlib], [support for XLib backend])],
	[],
	[with_xlib=no])

AS_IF([test "x$with_sdl" != xno],
	[
		PKG_CHECK_MODULES(X11, [x11],
		[
			AC_DEFINE(HAVE_X11, 1, [Define if X11 development libraries is available])
		],
		[
			AC_MSG_ERROR([Required X11 development libraries not found])
		])
		PKG_CHECK_MODULES(xxf86vm, [xxf86vm],
			[
				AC_DEFINE(HAVE_XF86VIDMODE, 1, [Define if xxf86vm development libraries is available])
			],
			[
				PKG_CHECK_MODULES(xxf86vm, [xf86vidmodeproto],
					[
						AC_DEFINE(HAVE_XF86VIDMODE, 1, [Define if xxf86vm development libraries is available])
					],
					[
						AC_MSG_ERROR([Required xxf86vm development libraries not found])
					])
			])
		PKG_CHECK_MODULES(xrandr, [xrandr],
			[
				AC_DEFINE(HAVE_XRANDR, 1, [Define if Xrandr development libraries is available])
			],
			[
				AC_MSG_ERROR([Required Xrandr development libraries not found])
			])
	],
	[])

AM_CONDITIONAL([WITH_XLIB], [test "x$with_xlib" != xno])

AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)
	
AC_SUBST(xxf86vm_CFLAGS)
AC_SUBST(xxf86vm_LIBS)

AC_SUBST(xrandr_CFLAGS)
AC_SUBST(xrandr_LIBS)

dnl #######################################################################
dnl # MySQL
dnl #######################################################################

AC_ARG_WITH([mysql],
	[AS_HELP_STRING([--with-mysql], [support for MySQL])],
	[],
	[with_mysql=no])

AS_IF([test "x$with_mysql" != xno],
	[AC_CHECK_HEADERS([mysql/mysql.h],
		[
			MYSQL_LIBS="-lmysqlclient"
		],
		[
			AC_MSG_ERROR([Required MySQL development libraries not found])
		])],
	[])

AM_CONDITIONAL([WITH_MYSQL], [test "x$with_mysql" != xno])
AC_SUBST(MYSQL_LIBS)

dnl #######################################################################
dnl # SQLite3
dnl #######################################################################

AC_ARG_WITH([sqlite3], [AS_HELP_STRING([--with-sqlite3], [support for SQLite3 @<:@default=no@:>@])])
AS_IF([test "x$with_sqlite3" != xno], [
    PKG_CHECK_MODULES(sqlite3, [sqlite3])
    AC_DEFINE(HAVE_SQLITE3, 1, [Define if sqlite3 is available])
])
AM_CONDITIONAL([WITH_SQLITE3], [test "x$with_sqlite3" != xno])

dnl #######################################################################
dnl # SDL
dnl #######################################################################
AC_ARG_WITH([sdl],
	[AS_HELP_STRING([--with-sdl], [support for SDL backend])],
	[],
	[with_sdl=no])

AS_IF([test "x$with_sdl" != xno],
	[
		AM_PATH_SDL
		AC_DEFINE(HAVE_SDL, 1, [Define if libsdl is available])
	],
	[])
	
AM_CONDITIONAL([WITH_SDL], [test "x$with_sdl" != xno])
AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LIBS)

AC_OUTPUT
