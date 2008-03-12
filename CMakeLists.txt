project(SLIDESHOW)

INCLUDE(UsePkgConfig)

add_subdirectory( vendor/libportable )

include_directories( ${DBUS_CFLAGS} include vendor/libportable/include )

set( CMAKE_CXX_FLAGS_DEBUG "-Wall -g" )
set( CMAKE_CXX_FLAGS_RELEASE "-Wall -g -O2" )

set( SLIDESHOW_SRCS src/main.cpp src/Kernel.cpp src/Graphics.cpp src/Log.cpp )
set( SLIDESHOW_LIBS libportable GL freeimage mysqlclient )

if (UNIX)
	set(SLIDESHOW_SRCS ${SLIDESHOW_SRCS} src/OS_Linux.cpp)
	set(SLIDESHOW_LIBS ${SLIDESHOW_LIBS} Xxf86vm)
endif (UNIX)

set( SLIDESHOW_SRCS ${SLIDESHOW_SRCS} src/transitions/dummy.cpp)
set( SLIDESHOW_SRCS ${SLIDESHOW_SRCS} src/transitions/fade.cpp)

set( SLIDESHOW_SRCS ${SLIDESHOW_SRCS} src/browsers/mysqlbrowser.cpp)

set( SLIDESHOW_SRCS ${SLIDESHOW_SRCS} src/IPC/dbus.cpp)

# external libraries
PKGCONFIG(dbus-1 DBUS_INCLUDE_DIR DBUS_LINK_DIR DBUS_LINK_FLAGS DBUS_CFLAGS)
SET(DBUS_DEFINITIONS ${DBUS_CFLAGS})
FIND_LIBRARY(DBUS_LIBS NAMES dbus-1 PATHS ${DBUS_LINK_DIR} /usr/lib /usr/local/lib)

set( CMAKE_CXX_FLAGS ${DBUS_CFLAGS} )
set(SLIDESHOW_LIBS ${SLIDESHOW_LIBS} ${DBUS_LINK_FLAGS} )

add_executable( slideshow ${SLIDESHOW_SRCS} )
target_link_libraries( slideshow ${SLIDESHOW_LIBS} )
