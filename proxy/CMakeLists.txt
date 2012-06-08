CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(msg-server CXX)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -O0 -g -Wall")

##########################################################
# Define Transaction Proxy
##########################################################

SET(TRANS-PROXY-SRCS
	${CMAKE_SOURCE_DIR}/proxy/MsgProxyListener.cpp
	${CMAKE_SOURCE_DIR}/proxy/MsgHandleControl.cpp
	${CMAKE_SOURCE_DIR}/proxy/MsgHandleSetting.cpp
	${CMAKE_SOURCE_DIR}/proxy/MsgHandleStorage.cpp
	${CMAKE_SOURCE_DIR}/proxy/MsgHandleTransport.cpp
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/framework
	${CMAKE_SOURCE_DIR}/include/utils
	${CMAKE_SOURCE_DIR}/include/proxy
)

INCLUDE(FindPkgConfig)
pkg_check_modules(trans_proxy_pkgs REQUIRED glib-2.0 dlog vconf security-server)

FOREACH(flag ${trans_proxy_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${TRANS-PROXY-LIB} SHARED ${TRANS-PROXY-SRCS})
TARGET_LINK_LIBRARIES(${TRANS-PROXY-LIB} ${trans_proxy_pkgs_LDFLAGS} ${UTILS-LIB} ${FW-HANDLER-LIB})

INSTALL(TARGETS ${TRANS-PROXY-LIB} DESTINATION lib COMPONENT RuntimeLibraries)

