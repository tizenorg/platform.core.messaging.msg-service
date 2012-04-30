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

SET(TEST-APP-SRCS
	${CMAKE_SOURCE_DIR}/test_app/MsgTestSetting.cpp
	${CMAKE_SOURCE_DIR}/test_app/MsgTestStorage.cpp
	${CMAKE_SOURCE_DIR}/test_app/MsgTestTransport.cpp
#	${CMAKE_SOURCE_DIR}/test_app/MsgTestConvert.cpp
	${CMAKE_SOURCE_DIR}/test_app/MsgTestThreadView.cpp
	${CMAKE_SOURCE_DIR}/test_app/main.cpp
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/mapi
)

INCLUDE(FindPkgConfig)
pkg_check_modules(test_app_pkgs REQUIRED glib-2.0)

FOREACH(flag ${test_app_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(msg-test-app ${TEST-APP-SRCS})
TARGET_LINK_LIBRARIES(msg-test-app ${test_app_pkgs_LDFLAGS} ${MAPI-LIB})

INSTALL(TARGETS msg-test-app DESTINATION bin)

