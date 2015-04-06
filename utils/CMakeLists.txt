CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(msg-server CXX)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -O0 -g -Wall")

##########################################################
# Define Framework Utils
##########################################################

SET(UTILS-SRCS
	${CMAKE_SOURCE_DIR}/utils/MsgDebug.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgException.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgUtilFunction.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgTextConvert.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgIpcSocket.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgUtilFile.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgSqliteWrapper.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgUtilStorage.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgGconfWrapper.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgContact.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgVMessage.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgMemory.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgMmsMessage.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgDrmWrapper.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgJsonParser.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgSerialize.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgSmil.cpp
	${CMAKE_SOURCE_DIR}/utils/MsgZoneManager.cpp
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include/mapi
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/framework
	${CMAKE_SOURCE_DIR}/include/utils
	${CMAKE_SOURCE_DIR}/vobject-engine/include
)

INCLUDE(FindPkgConfig)

SET(PKG_MODULES glib-2.0 vconf db-util contacts-service2 dlog libxml-2.0 storage json-glib-1.0 capi-system-info)
IF(_FEATURE_CONTAINER_ENABLE)
ADD_DEFINITIONS("-DFEATURE_CONTAINER_ENABLE")
SET(PKG_MODULES ${PKG_MODULES} vasum)
ENDIF(_FEATURE_CONTAINER_ENABLE)
pkg_check_modules(utils_pkgs REQUIRED ${PKG_MODULES})

FOREACH(flag ${utils_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DFEATURE_JAVA_MMS")

SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${UTILS-LIB} SHARED ${UTILS-SRCS})
TARGET_LINK_LIBRARIES(${UTILS-LIB} ${utils_pkgs_LDFLAGS} rt ${VOBJECT-LIB})

INSTALL(TARGETS ${UTILS-LIB} DESTINATION ${LIBDIR} COMPONENT RuntimeLibraries)

