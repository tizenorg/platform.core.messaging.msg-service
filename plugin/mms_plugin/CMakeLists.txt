CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(mms-plugin CXX)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")


##########################################################
# Define MMS Plugin
##########################################################

SET(MMS-PLUGIN-SRCS
	MmsPluginCodecCommon.cpp
	MmsPluginDecode.cpp
	MmsPluginEncode.cpp
	MmsPluginTextConvert.cpp
	MmsPluginTransport.cpp
	MmsPluginMain.cpp
	MmsPluginEventHandler.cpp
	MmsPluginStorage.cpp
	MmsPluginMessage.cpp
	MmsPluginMIME.cpp
	MmsPluginConnManWrapper.cpp
	MmsPluginUserAgent.cpp
	MmsPluginHttp.cpp
	MmsPluginInternal.cpp
	MmsPluginDrm.cpp
	MmsPluginUtil.cpp
#	MmsPluginTcs.cpp
	MmsPluginComposer.cpp
	MmsPluginAppBase.cpp
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/framework
	${CMAKE_SOURCE_DIR}/include/utils
	${CMAKE_SOURCE_DIR}/include/externals
	${CMAKE_SOURCE_DIR}/include/mapi
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

INCLUDE(FindPkgConfig)
pkg_check_modules(mms_plugin_pkgs REQUIRED glib-2.0 libcurl mm-fileinfo mmutil-imgp mmutil-jpeg vconf dlog media-thumbnail capi-network-connection db-util)

FOREACH(flag ${mms_plugin_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "-O2 -g")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DFEATURE_JAVA_MMS")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")
SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")

ADD_LIBRARY(${MMS-PLUGIN-LIB} SHARED ${MMS-PLUGIN-SRCS})
TARGET_LINK_LIBRARIES(${MMS-PLUGIN-LIB} ${mms_plugin_pkgs_LDFLAGS} ${UTILS-LIB} ${EXTERNALS-LIB})

INSTALL(TARGETS ${MMS-PLUGIN-LIB} LIBRARY DESTINATION ${LIBDIR})

