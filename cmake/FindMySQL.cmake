#
# Try to find MySQL library and include path.
# Once done this will define
#
# MYSQL_FOUND
# MYSQL_INCLUDE_PATH
# MYSQL_LIBRARY
#

IF ( NOT MYSQL_FIND_QUIETLY )
	MESSAGE ( STATUS "Looking for MySQL" )
ENDIF ( NOT MYSQL_FIND_QUIETLY )

IF (WIN32)
	MESSAGE(FATAL_ERROR "bajs")
ELSE (WIN32)
	FIND_PATH( MYSQL_INCLUDE_PATH mysql/mysql.h
		/usr/local/include
		/usr/include
		/sw/include
		/opt/local/include
		/opt/csw/include
		/opt/include
		PATH_SUFFIXES MySQL-1.0
	)

	FIND_LIBRARY( MYSQL_LIBRARY mysqlclient
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
	)
ENDIF (WIN32)

IF ( MYSQL_INCLUDE_PATH AND MYSQL_LIBRARY )
	SET( MYSQL_FOUND TRUE )
ELSE ( MYSQL_INCLUDE_PATH AND MYSQL_LIBRARY )
	SET( MYSQL_FOUND FALSE )
ENDIF ( MYSQL_INCLUDE_PATH AND MYSQL_LIBRARY )

IF ( MYSQL_FOUND )
   IF ( NOT MYSQL_FIND_QUIETLY )
	  MESSAGE ( STATUS "Looking for MySQL - found" )
   ENDIF ( NOT MYSQL_FIND_QUIETLY )
ELSE ( MYSQL_FOUND )
   IF ( MYSQL_FIND_REQUIRED )
	  MESSAGE ( FATAL_ERROR "Looking for MySQL - not found" )
   ENDIF ( MYSQL_FIND_REQUIRED )
ENDIF ( MYSQL_FOUND )

MARK_AS_ADVANCED( MYSQL_FOUND MYSQL_INCLUDE_PATH MYSQL_LIBRARY )
