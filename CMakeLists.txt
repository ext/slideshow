project(SLIDESHOW)

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )

message( STATUS "Looking for X11 dev" )
include(FindX11)
if( X11_FOUND )
	message( STATUS "Looking for X11 dev - found" )

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
		message( FATAL_ERROR "Looking for X11 dev - not found" )
	else( UNIX )
		message( STATUS "Looking for X11 dev - not found" )
	endif( UNIX )
endif( X11_FOUND )

# OpenGL
message( STATUS "Looking for OpenGL" )
INCLUDE( FindOpenGL )
IF( OPENGL_FOUND )
	MESSAGE( STATUS "Looking for OpenGL - found" )
ELSE( OPENGL_FOUND )
	MESSAGE( FATAL_ERROR "Looking for OpenGL - not found" )
ENDIF( OPENGL_FOUND )

set( FREEIMAGE_FIND_REQUIRED TRUE )
include(FindFreeImage)

set( DBUS_FIND_REQUIRED TRUE )
include(FindDBus)

set( MYSQL_FIND_REQUIRED TRUE )
include(FindMySQL)

set( PORTABLE_FIND_REQUIRED TRUE )
include(FindPortable)

include_directories(
	include
	${DBUS_INCLUDE_PATH}
	${OPENGL_INCLUDE_PATH}
	${FREEIMAGE_INCLUDE_PATH}
	${MYSQL_INCLUDE_PATH}
	${PORTABLE_INCLUDE_PATH}
)

set( CMAKE_CXX_FLAGS_DEBUG "-Wall -g" )
set( CMAKE_CXX_FLAGS_RELEASE "-Wall -g -O2" )
set( CMAKE_C_FLAGS_DEBUG "-Wall -g" )
set( CMAKE_C_FLAGS_RELEASE "-Wall -g -O2" )

add_subdirectory( src/browsers )
add_subdirectory( src/IPC )
add_subdirectory( src/transitions )

set( SLIDESHOW_SRCS
	src/state/InitialState.cpp
	src/state/SwitchState.cpp
	src/state/TransitionState.cpp
	src/state/ViewState.cpp
	src/argument_parser.c
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

add_executable( slideshow ${SLIDESHOW_SRCS} )
target_link_libraries( slideshow
	Browsers
	IPC
	Transitions
	${SLIDESHOW_LIBS}
	${X11_LIBRARIES}
	${DBUS_LIBRARY}
	${OPENGL_LIBRARIES}
	${FREEIMAGE_LIBRARY}
	${MYSQL_LIBRARY}
	${PORTABLE_LIBRARY}
)

