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

SET(UTILS-SRCS
	${CMAKE_SOURCE_DIR}/utils/MsgCallStatusManager.cpp
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
	${CMAKE_SOURCE_DIR}/utils/MsgUtilMime.cpp
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include/mapi
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/framework
	${CMAKE_SOURCE_DIR}/include/utils
	${CMAKE_SOURCE_DIR}/vobject-engine/include
)

INCLUDE(FindPkgConfig)

IF(_MSG_WEARABLE_PROFILE)
SET(PKG_MODULES glib-2.0 vconf dlog libxml-2.0 storage json-glib-1.0 capi-system-info capi-media-thumbnail-util capi-media-image-util capi-content-media-content aul sqlite3 capi-media-metadata-extractor icu-uc)
ELSE()
SET(PKG_MODULES glib-2.0 vconf contacts-service2 dlog libxml-2.0 storage json-glib-1.0 capi-system-info capi-media-thumbnail-util capi-media-image-util capi-content-media-content aul sqlite3 capi-media-metadata-extractor icu-uc)
ENDIF()
pkg_check_modules(utils_pkgs REQUIRED ${PKG_MODULES})

FOREACH(flag ${utils_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DFEATURE_JAVA_MMS")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${UTILS-LIB} SHARED ${UTILS-SRCS})
TARGET_LINK_LIBRARIES(${UTILS-LIB} ${utils_pkgs_LDFLAGS} rt ${VOBJECT-LIB} acl)

INSTALL(TARGETS ${UTILS-LIB} DESTINATION ${LIBDIR} COMPONENT RuntimeLibraries)

