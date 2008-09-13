AC_INIT([slideshow],[0.2.0])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([include/config.h])

AC_PROG_CXX
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
dnl # Check for D-Bus support
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
dnl # Check for D-Bus support
dnl #######################################################################
PKG_CHECK_MODULES(libportable, [libportable],
	[
		AC_DEFINE(HAVE_DBUS, 1, [Define if libportable is available])
	],
	[
		AC_MSG_ERROR([Required libportable not found])
	])

AC_SUBST(libportable_CFLAGS)
AC_SUBST(libportable_LIBS)

AC_OUTPUT
