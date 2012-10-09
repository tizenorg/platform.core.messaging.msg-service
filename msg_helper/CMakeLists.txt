CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(msg-server CXX)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -O0 -g -Wall")

##########################################################
# Define Test App
##########################################################

SET(MSG-HELPER-SRCS
	${CMAKE_SOURCE_DIR}/msg_helper/MsgSensorWrapper.cpp
	${CMAKE_SOURCE_DIR}/msg_helper/MsgSoundPlayer.cpp
	${CMAKE_SOURCE_DIR}/msg_helper/main.cpp
)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include/mapi
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/framework
	${CMAKE_SOURCE_DIR}/include/msg_helper
	${CMAKE_SOURCE_DIR}/include/utils
)

INCLUDE(FindPkgConfig)
pkg_check_modules(msg_helper_pkgs REQUIRED glib-2.0 dlog vconf devman_haptic mm-player mm-fileinfo mm-player sensor)

FOREACH(flag ${msg_helper_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(msg-helper ${MSG-HELPER-SRCS})
TARGET_LINK_LIBRARIES(msg-helper ${msg_helper_pkgs_LDFLAGS} ${UTILS-LIB})

INSTALL(TARGETS msg-helper DESTINATION bin)

