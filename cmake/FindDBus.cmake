#
# Try to find DBus library and include path.
# Once done this will define
#
# DBUS_FOUND
# DBUS_INCLUDE_PATH
# DBUS_LIBRARY
#

IF ( NOT DBUS_FIND_QUIETLY )
	MESSAGE ( STATUS "Looking for DBus" )
ENDIF ( NOT DBUS_FIND_QUIETLY )

IF (WIN32)
	MESSAGE(FATAL_ERROR "bajs")
ELSE (WIN32)
	FIND_PATH( DBUS_INCLUDE_PATH dbus/dbus.h
		/usr/local/include
		/usr/include
		/sw/include
		/opt/local/include
		/opt/csw/include
		/opt/include
		PATH_SUFFIXES dbus-1.0
	)

	FIND_LIBRARY( DBUS_LIBRARY dbus-1
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
	)

	get_filename_component(_dbusLibPath ${DBUS_LIBRARY} PATH)

	SET( DBUS_INCLUDE_PATH ${DBUS_INCLUDE_PATH} ${_dbusLibPath}/dbus-1.0/include )
	message( ${DBUS_INCLUDE_PATH} )
ENDIF (WIN32)

IF ( DBUS_INCLUDE_PATH AND DBUS_LIBRARY )
	SET( DBUS_FOUND TRUE )
ELSE ( DBUS_INCLUDE_PATH AND DBUS_LIBRARY )
	SET( DBUS_FOUND FALSE )
ENDIF ( DBUS_INCLUDE_PATH AND DBUS_LIBRARY )

IF ( DBUS_FOUND )
   IF ( NOT DBUS_FIND_QUIETLY )
	  MESSAGE ( STATUS "Looking for DBus - found" )
   ENDIF ( NOT DBUS_FIND_QUIETLY )
ELSE ( DBUS_FOUND )
   IF ( DBUS_FIND_REQUIRED )
	  MESSAGE ( FATAL_ERROR "Looking for DBus - not found" )
   ENDIF ( DBUS_FIND_REQUIRED )
ENDIF ( DBUS_FOUND )

MARK_AS_ADVANCED( DBUS_FOUND DBUS_INCLUDE_PATH DBUS_LIBRARY )
