dnl Remember to bump 'setup.py' and 'slideshow/templates/index.html' as well
AC_INIT([slideshow],[0.4.0], [https://github.com/ext/slideshow/issues/new])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall 1.11 subdir-objects foreign])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/transitions/Makefile
	src/browsers/Makefile
	src/IPC/Makefile
])
AC_CONFIG_FILES([slideshow-wrapper], [
  chmod +x slideshow-wrapper
  test -e slideshow-daemon     || ln -s slideshow-wrapper slideshow-daemon
  test -e slideshow-valgrind   || ln -s slideshow-wrapper slideshow-valgrind
  test -e slideshow-gdb        || ln -s slideshow-wrapper slideshow-gdb
  test -e slideshow-transition || ln -s slideshow-wrapper slideshow-transition
])

AC_GNU_SOURCE

# automake 1.12 needs AM_PROG_AR but automake 1.11 doesn't have it
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_CC_C99
AX_CXX_COMPILE_STDCXX_11
AM_DISABLE_STATIC
AC_LIBTOOL_DLOPEN
LT_INIT
CHECK_RAGEL([src/browser_context.cpp])
AX_CHECK_GL
AC_FUNC_FORK

dnl Windows does not support daemon mode yet, but any platform where autotools
dnl is used is assumed to.
AC_DEFINE([BUILD_DAEMON], [1], [Define to 1 to build daemon mode support])

dnl Only needed because of an ugly hack in argument parser, breaks cross-compiling =/
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

dnl Require datapack for packing shaders and other resources into transition
dnl plugins.
dnl https://github.com/ext/datapack
AC_PATH_PROG([DATAPACKER], [datapacker])
AS_IF([test -z "${DATAPACKER}"], [
  AC_MSG_ERROR([datapacker (datapack) not found in PATH])
])
PKG_CHECK_MODULES([datapack], [datapack >= 0.3])

dnl #######################################################################
dnl # Paths
dnl #######################################################################

pkgdatadir="${datadir}/${PACKAGE}"
resourcedir="${pkgdatadir}/resources"
plugindir="${pkgdatadir}/plugin"

AC_SUBST(pkgdatadir)
AC_SUBST(resourcedir)
AC_SUBST(plugindir)

AC_DEFINE_DIR([DATA_DIR], [pkgdatadir], [Data directory])
AC_DEFINE_DIR([RESOURCE_DIR], [resourcedir], [Resource directory])
AC_DEFINE_DIR([PLUGIN_DIR], [plugindir], [Plugin directory])

dnl #######################################################################
dnl # Required libraries
dnl #######################################################################

PKG_CHECK_MODULES([libdaemon], [libdaemon >= 0.13])
PKG_CHECK_MODULES([DevIL], [IL ILU])
PKG_CHECK_MODULES([glew], [glew])
PKG_CHECK_MODULES([json_c], [json-c],[
  json_CFLAGS=$json_c_CFLAGS
  json_LIBS=$json_c_LIBS
], [PKG_CHECK_MODULES([json], [json])])
AX_LIB_CURL([],,AC_MSG_ERROR([Required library libcurl not found]))

dnl #######################################################################
dnl # Browsers
dnl #######################################################################

dnl Directory
AC_ARG_WITH([directory], [AS_HELP_STRING([--with-directory], [support for directory browser @<:@default=enabled@:>@])],, [with_directory=yes])
AS_IF([test "x$with_directory" != xno], [
  AC_DEFINE(HAVE_DIRECTORY, 1, [Define if directory browser is available])
])
AM_CONDITIONAL([WITH_DIRECTORY], [test "x$with_directory" != xno])

dnl MySQL
AC_ARG_WITH([mysql], [AS_HELP_STRING([--with-mysql], [support for MySQL @<:@default=no@:>@])], [], [with_mysql=no])
AS_IF([test "x$with_mysql" != xno], [
  MYSQL_CLIENT
])
AM_CONDITIONAL([WITH_MYSQL], [test "x$with_mysql" != xno])

dnl SQLite3
AC_ARG_WITH([sqlite3], [AS_HELP_STRING([--with-sqlite3], [support for SQLite3 @<:@default=no@:>@])], [], [with_sqlite3=no])
AS_IF([test "x$with_sqlite3" != xno], [
  PKG_CHECK_MODULES(sqlite3, [sqlite3])
])
AM_CONDITIONAL([WITH_SQLITE3], [test "x$with_sqlite3" != xno])


dnl #######################################################################
dnl # IPC
dnl #######################################################################

AC_ARG_WITH([dbus], [AS_HELP_STRING([--with-dbus], [support for D-Bus])], [],	[with_dbus=no])
AS_IF([test "x$with_dbus" != xno], [
  PKG_CHECK_MODULES(dbus, [dbus-1, dbus-glib-1])
  AC_DEFINE([HAVE_DBUS], [1], [Define to 1 if you have D-Bus])
])	
AM_CONDITIONAL([WITH_DBUS], [test "x$with_dbus" != xno])

dnl #######################################################################
dnl # Backends
dnl #######################################################################

AC_ARG_WITH([sdl], [AS_HELP_STRING([--with-sdl], [support for SDL backend @<:@default=no@:>@])], [],	[with_sdl=no])
AS_IF([test "x$with_sdl" != xno], [
  AM_PATH_SDL
  AC_DEFINE(HAVE_SDL, 1, [Define if libsdl is available])
])
AM_CONDITIONAL([WITH_SDL], [test "x$with_sdl" != xno])

AC_OUTPUT
