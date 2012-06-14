CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(mms-plugin CXX)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

##########################################################
# Define MMS Language Pack
##########################################################

SET(MMS-LANGUAGE-PACK-SRCS
	LanguagePack/MmsPluginWmLngLatinUni.cpp
	LanguagePack/MmsPluginWmLngString.cpp
	LanguagePack/MmsPluginWmLngUTF8Uni.cpp
	LanguagePack/MmsPluginWmLngUniUTF8.cpp
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/utils
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

INCLUDE(FindPkgConfig)
pkg_check_modules(mms_language_pkgs REQUIRED glib-2.0 dlog)

FOREACH(flag ${mms_language_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "-O2 -g")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${MMS-LANGUAGE-PACK-LIB} SHARED ${MMS-LANGUAGE-PACK-SRCS})
TARGET_LINK_LIBRARIES(${MMS-LANGUAGE-PACK-LIB} ${mms_language_pkgs_LDFLAGS} ${UTILS-LIB})

INSTALL(TARGETS ${MMS-LANGUAGE-PACK-LIB} LIBRARY DESTINATION lib)

##########################################################
# Define MMS Plugin
##########################################################

SET(MMS-PLUGIN-SRCS
	MmsPluginTransport.cpp
	MmsPluginMain.cpp
	MmsPluginEventHandler.cpp
	MmsPluginStorage.cpp
	MmsPluginMessage.cpp
	MmsPluginMIME.cpp
	MmsPluginDecode.cpp
	MmsPluginAvCodec.cpp
	MmsPluginDebug.cpp
	MmsPluginConnManWrapper.cpp
	MmsPluginUserAgent.cpp
	MmsPluginHttp.cpp
	MmsPluginEncode.cpp
	MmsPluginCodec.cpp
	MmsPluginInternal.cpp
	MmsPluginContentCodec.cpp
	MmsPluginSmil.cpp
	MmsPluginSMILValidate.cpp
	MmsPluginDrm.cpp
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/framework
	${CMAKE_SOURCE_DIR}/include/msg_helper
	${CMAKE_SOURCE_DIR}/include/utils
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

INCLUDE(FindPkgConfig)
pkg_check_modules(mms_plugin_pkgs REQUIRED glib-2.0 network libxml-2.0 drm-client libcurl mm-fileinfo mmutil-jpeg vconf dlog media-thumbnail)

FOREACH(flag ${mms_plugin_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "-O2 -g")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
#ADD_DEFINITIONS("-DWno-write-strings")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${MMS-PLUGIN-LIB} SHARED ${MMS-PLUGIN-SRCS})
TARGET_LINK_LIBRARIES(${MMS-PLUGIN-LIB} ${mms_plugin_pkgs_LDFLAGS} ${MMS-LANGUAGE-PACK-LIB} ${UTILS-LIB})

INSTALL(TARGETS ${MMS-PLUGIN-LIB} LIBRARY DESTINATION lib)

