AC_INIT([slideshow],[0.2.0])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([include/config.h])

AC_PROG_CXX
AC_PROG_RANLIB
LT_INIT

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/browsers/Makefile
	src/IPC/Makefile
	src/state/Makefile
	src/transitions/Makefile
])

AC_FUNC_FORK

dnl #######################################################################
dnl # D-Bus
dnl #######################################################################
PKG_CHECK_MODULES(dbus, [dbus-1, dbus-glib-1],
	[
		AC_DEFINE(HAVE_DBUS, 1, [Define if D-Bus is available])
		enable_dbus=yes
	],
	[
		AC_MSG_ERROR([Required D-Bus not found.])
	])

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
dnl # OpenGL/GLX
dnl #######################################################################
AC_CHECK_HEADERS([GL/gl.h GL/glx.h], [HAVE_GL=yes], [HAVE_GL=no])
if test x$HAVE_GL = xno; then
   AC_MSG_ERROR(OpenGL development libraries not found);
else
   GL_LIBS="-lGL"
   GL_CFLAGS=""
fi

AC_SUBST(GL_CFLAGS)
AC_SUBST(GL_LIBS)

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

dnl #######################################################################
dnl # MySQL
dnl #######################################################################
AC_CHECK_HEADERS([mysql/mysql.h],
	[
		AC_DEFINE(HAVE_MYSQL, 1, [Define if X11 development libraries is available])
		MYSQL_LIBS="-lmysqlclient"
	],
	[
		AC_MSG_ERROR([Required mysql development libraries not found])
	])

AC_SUBST(MYSQL_LIBS)

AC_OUTPUT
