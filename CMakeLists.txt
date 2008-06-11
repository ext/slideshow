project(SLIDESHOW)

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )

message( STATUS "Looking for X11" )
include(FindX11)
if( X11_FOUND )
	message( STATUS "Looking for X11 - found" )

	find_library( X11_VIDEO_LIBRARY Xxf86vm
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		DOC "Xxf86vm"
	)

	if( X11_VIDEO_LIBRARY )
		message( STATUS "Looking for Xxf86vm - found" )
	else( X11_VIDEO_LIBRARY )
		message( FATAL_ERROR "Looking for Xxf86vm - not found" )
	endif( X11_VIDEO_LIBRARY )

	set( X11_LIBRARIES ${X11_LIBRARIES} ${X11_VIDEO_LIBRARY} )
else( X11_FOUND )
	if( UNIX )
		message( FATAL_ERROR "Looking for X11 - not found" )
	else( UNIX )
		message( STATUS "Looking for X11 - not found" )
	endif( UNIX )
endif( X11_FOUND )

set( DBUS_FIND_REQUIRED TRUE )
include(FindDBus)

add_subdirectory( vendor/libportable )

include_directories(
	include
	vendor/libportable/include
	${DBUS_INCLUDE_PATH}
)

set( CMAKE_CXX_FLAGS_DEBUG "-Wall -g" )
set( CMAKE_CXX_FLAGS_RELEASE "-Wall -g -O2" )

set( SLIDESHOW_SRCS
	src/main.cpp
	src/Kernel.cpp
	src/Graphics.cpp
	src/Log.cpp
)

if (UNIX)
	set(SLIDESHOW_SRCS ${SLIDESHOW_SRCS} src/OS_Linux.cpp)
else (UNIX)
	message( FATAL_ERROR "Sorry, Slideshow does not yet build on this platform. Please feel free to contribute!" )
endif (UNIX)

set( SLIDESHOW_LIBS libportable GL freeimage mysqlclient )

set( SLIDESHOW_SRCS ${SLIDESHOW_SRCS} src/transitions/dummy.cpp src/transitions/fade.cpp )
set( SLIDESHOW_SRCS ${SLIDESHOW_SRCS} src/browsers/mysqlbrowser.cpp)
set( SLIDESHOW_SRCS ${SLIDESHOW_SRCS} src/IPC/dbus.cpp)

add_executable( slideshow ${SLIDESHOW_SRCS} )
target_link_libraries( slideshow
	${SLIDESHOW_LIBS}
	${X11_LIBRARIES}
	${DBUS_LIBRARY}
)
