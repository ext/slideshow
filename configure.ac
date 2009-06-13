AC_INIT([slideshow],[0.2.0], [ext@sidvind.com])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([include/config.h])

AC_PROG_CXX
AC_PROG_LIBTOOL
LT_INIT
AC_LIBTOOL_DLOPEN
AC_PROG_CC_C99

AX_CHECK_GLU

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/IPC/Makefile
	src/state/Makefile
	src/transitions/Makefile
])

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
	
AM_CONDITIONAL([DBUS], [test "x$with_dbus" != xno])
AC_SUBST(dbus_CFLAGS)
AC_SUBST(dbus_LIBS)

dnl #######################################################################
dnl # libportable
dnl #######################################################################
PKG_CHECK_MODULES(libportable, [libportable],
	[
		AC_DEFINE(HAVE_PORTABLE, 1, [Define if libportable is available])
	],
	[
		AC_MSG_ERROR([Required libportable not found])
	])

AC_SUBST(libportable_CFLAGS)
AC_SUBST(libportable_LIBS)

dnl #######################################################################
dnl # libdaemon
dnl #######################################################################
PKG_CHECK_MODULES(libdaemon, [libdaemon],
	[
		AC_DEFINE(HAVE_LIBDAEMON, 1, [Define if libdaemon is available])
	],
	[
		AC_MSG_ERROR([Required libdaemon not found])
	])

AC_SUBST(libdaemon_CFLAGS)
AC_SUBST(libdaemon_LIBS)

dnl #######################################################################
dnl # FreeImage
dnl #######################################################################
AC_CHECK_HEADERS([FreeImage.h], [HAVE_FREEIMAGE=yes], [HAVE_FREEIMAGE=no])
if test x$HAVE_FREEIMAGE = xno; then
   AC_MSG_ERROR(FreeImage development libraries not found);
else
   FREEIMAGE_LIBS="-lfreeimage"
   FREEIMAGE_CFLAGS=""
fi

AC_SUBST(FREEIMAGE_CFLAGS)
AC_SUBST(FREEIMAGE_LIBS)

dnl #######################################################################
dnl # Xlib
dnl #######################################################################
PKG_CHECK_MODULES(X11, [x11],
	[
		AC_DEFINE(HAVE_X11, 1, [Define if X11 development libraries is available])
	],
	[
		AC_MSG_ERROR([Required X11 development libraries not found])
	])

AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

PKG_CHECK_MODULES(xxf86vm, [xxf86vm],
	[
		AC_DEFINE(HAVE_VIDMODE, 1, [Define if xxf86vm development libraries is available])
	],
	[
		PKG_CHECK_MODULES(xxf86vm, [xf86vidmodeproto],
			[
				AC_DEFINE(HAVE_VIDMODE, 1, [Define if xxf86vm development libraries is available])
			],
			[
				AC_MSG_ERROR([Required xxf86vm development libraries not found])
			])
	])
	
AC_SUBST(xxf86vm_CFLAGS)
AC_SUBST(xxf86vm_LIBS)

PKG_CHECK_MODULES(xrandr, [xrandr],
	[
		AC_DEFINE(HAVE_XRANDR, 1, [Define if Xrandr development libraries is available])
	],
	[
		AC_MSG_ERROR([Required Xrandr development libraries not found])
	])

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

AM_CONDITIONAL([MYSQL], [test "x$with_mysql" != xno])
AC_SUBST(MYSQL_LIBS)

dnl #######################################################################
dnl # gccparser
dnl #######################################################################

AC_ARG_ENABLE([gccparser],
	[AS_HELP_STRING([--enable-gccparser], [support for gccparser])],
	[case "${enableval}" in
       yes) gccparser=true ;;
       no)  gccparser=false ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-gccparser]) ;;
     esac],
	[gccparser=false])
AM_CONDITIONAL([GCCPARSER], [test x$gccparser == xtrue])

AC_OUTPUT
