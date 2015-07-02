CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(msg-server CXX)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -O2 -g -Wall")

##########################################################
# Define Framework Utils
##########################################################

SET(EXTERNALS-SRCS
	${CMAKE_SOURCE_DIR}/externals/MsgSoundPlayer.cpp
	${CMAKE_SOURCE_DIR}/externals/MsgNotificationWrapper.cpp
	${CMAKE_SOURCE_DIR}/externals/MsgSpamFilter.cpp
	${CMAKE_SOURCE_DIR}/externals/MsgLbs.cpp
	${CMAKE_SOURCE_DIR}/externals/MsgSensorWrapper.cpp
	${CMAKE_SOURCE_DIR}/externals/MsgAlarm.cpp
	${CMAKE_SOURCE_DIR}/externals/MsgDevicedWrapper.cpp
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include/mapi
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/utils
	${CMAKE_SOURCE_DIR}/include/externals
)

INCLUDE(FindPkgConfig)
pkg_check_modules(externals_pkgs REQUIRED glib-2.0 deviced mm-session alarm-service notification badge iniparser capi-appfw-application lbs-dbus mm-fileinfo mm-player mm-sound sensor feedback db-util)

FOREACH(flag ${externals_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DFEATURE_JAVA_MMS")
IF(_TIZEN_I586_ENABLED)
ADD_DEFINITIONS("-DTIZEN_I586_ENABLED")
ENDIF(_TIZEN_I586_ENABLED)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${EXTERNALS-LIB} SHARED ${EXTERNALS-SRCS})
TARGET_LINK_LIBRARIES(${EXTERNALS-LIB} ${externals_pkgs_LDFLAGS} rt ${UTILS-LIB} ${VOBJECT-LIB})

INSTALL(TARGETS ${EXTERNALS-LIB} DESTINATION ${LIBDIR} COMPONENT RuntimeLibraries)

