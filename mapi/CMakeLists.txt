CMAKE_MINIMUM_REQUIRED(VERSION 2.6) 
PROJECT(msg-server CXX)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)
SET(VERSION_MAJOR 0)
SET(VERSION "${VERSION_MAJOR}.1.0")

IF("${CMAKE_BUILD_TYPE}" STREQUAL "") 
	SET(CMAKE_BUILD_TYPE "Debug") 
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "") 
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -O0 -g -Wall")

##########################################################
# Define MAPI   
##########################################################

SET(MAPI-SRCS 
	${CMAKE_SOURCE_DIR}/mapi/msg_svc.cpp
	${CMAKE_SOURCE_DIR}/mapi/msg_control.cpp
	${CMAKE_SOURCE_DIR}/mapi/msg_storage.cpp
	${CMAKE_SOURCE_DIR}/mapi/msg_transport.cpp
	${CMAKE_SOURCE_DIR}/mapi/msg_filter.cpp
	${CMAKE_SOURCE_DIR}/mapi/msg_message.cpp
	${CMAKE_SOURCE_DIR}/mapi/msg_mms.cpp
	${CMAKE_SOURCE_DIR}/mapi/msg_setting.cpp
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/proxy
	${CMAKE_SOURCE_DIR}/include/utils
	${CMAKE_SOURCE_DIR}/include/mapi
)

INCLUDE(FindPkgConfig)
pkg_check_modules(mapi_pkgs REQUIRED glib-2.0 dlog privacy-manager-client vconf)

FOREACH(flag ${mapi_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${MAPI-LIB} SHARED ${MAPI-SRCS})
TARGET_LINK_LIBRARIES(${MAPI-LIB} ${mapi_pkgs_LDFLAGS} ${UTILS-LIB} ${TRANS-PROXY-LIB})
SET_TARGET_PROPERTIES(${MAPI-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${MAPI-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${MAPI-LIB} DESTINATION ${LIBDIR} COMPONENT RuntimeLibraries)

