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
PKG_CHECK_MODULES(DBdUS, [dbus-1 >= 0.60, dbus-glib-1 >= 0.60],
	[
		AC_DEFINE(HAVE_DBUS_GLIB, 1, [Define if glib bindings of D-Bus are available])
		enable_dbus_glib=yes
	],
	[
		AC_MSG_ERROR([D-Bus requirements not met. D-Bus support not activated.])
		enable_dbus_glib=no
	])

AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)

AC_OUTPUT
