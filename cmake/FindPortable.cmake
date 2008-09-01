#
# Try to find libportable.
# Once done this will define
#
# PORTABLE_FOUND
# PORTABLE_INCLUDE_PATH
# PORTABLE_LIBRARY
# 

IF ( NOT PORTABLE_FIND_QUIETLY )
	MESSAGE ( STATUS "Looking for libportable" )
ENDIF ( NOT PORTABLE_FIND_QUIETLY )

IF (WIN32)
	MESSAGE(FATAL_ERROR "bajs")
ELSE (WIN32)
	FIND_PATH( PORTABLE_INCLUDE_PATH portable/Time.h
	    /usr/local/include
	    /usr/include
	    /sw/include
	    /opt/local/include
	    /opt/csw/include
	    /opt/include
    )
    	
	FIND_LIBRARY( PORTABLE_LIBRARY portable
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		DOC "libportable")
ENDIF (WIN32)

IF (PORTABLE_INCLUDE_PATH)
	SET( PORTABLE_FOUND 1 CACHE STRING "Set to 1 if libportable is found, 0 otherwise")
ELSE (PORTABLE_INCLUDE_PATH)
	SET( PORTABLE_FOUND 0 CACHE STRING "Set to 1 if libportable is found, 0 otherwise")
ENDIF (PORTABLE_INCLUDE_PATH)

IF ( PORTABLE_FOUND )
   IF ( NOT PORTABLE_FIND_QUIETLY )
	  MESSAGE ( STATUS "Looking for libportable - found" )
   ENDIF ( NOT PORTABLE_FIND_QUIETLY )
ELSE ( PORTABLE_FOUND )
   IF ( PORTABLE_FIND_REQUIRED )
	  MESSAGE ( FATAL_ERROR "Looking for libportable - not found" )
   ENDIF ( PORTABLE_FIND_REQUIRED )
ENDIF ( PORTABLE_FOUND )

MARK_AS_ADVANCED( PORTABLE_FOUND PORTABLE_INCLUDE_PATH PORTABLE_LIBRARY )
