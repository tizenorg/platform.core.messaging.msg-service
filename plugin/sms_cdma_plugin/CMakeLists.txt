CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(sms-plugin CXX)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -O0 -g -Wall")

##########################################################
# Define SMS Plugin
##########################################################

SET(SMS-PLUGIN-SRCS
	${CMAKE_SOURCE_DIR}/plugin/sms_cdma_plugin/SmsCdmaPluginCallback.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_cdma_plugin/SmsCdmaPluginCodec.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_cdma_plugin/SmsCdmaPluginEventHandler.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_cdma_plugin/SmsCdmaPluginMain.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_cdma_plugin/SmsCdmaPluginParamCodec.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_cdma_plugin/SmsCdmaPluginStorage.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_cdma_plugin/SmsCdmaPluginTransport.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_cdma_plugin/SmsCdmaPluginUAManager.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_cdma_plugin/SmsCdmaPluginSetting.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_cdma_plugin/SmsCdmaPluginWapPushHandler.cpp
)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include/mapi
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/utils
	${CMAKE_SOURCE_DIR}/include/externals
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

INCLUDE(FindPkgConfig)
pkg_check_modules(sms_plugin_pkgs REQUIRED db-util glib-2.0 tapi libxml-2.0 libwbxml2 dlog vconf gio-2.0)

FOREACH(flag ${sms_plugin_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${SMS-PLUGIN-LIB} SHARED ${SMS-PLUGIN-SRCS})
TARGET_LINK_LIBRARIES(${SMS-PLUGIN-LIB} ${sms_plugin_pkgs_LDFLAGS} ${UTILS-LIB} ${EXTERNALS-LIB})

IF(_FEATURE_TELEPHONY_ENABLE)
INSTALL(TARGETS ${SMS-PLUGIN-LIB} LIBRARY DESTINATION lib)
ENDIF(_FEATURE_TELEPHONY_ENABLE)

