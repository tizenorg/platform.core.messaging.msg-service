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
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginUDCodec.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginParamCodec.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginTpduCodec.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginSatHandler.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginCbMsgHandler.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginWapPushHandler.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginConcatHandler.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginSetting.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginSimMsg.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginStorage.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginTransport.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginEventHandler.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginUAManager.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginCallback.cpp
	${CMAKE_SOURCE_DIR}/plugin/sms_plugin/SmsPluginMain.cpp
)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include/mapi
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/utils
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

INCLUDE(FindPkgConfig)
pkg_check_modules(sms_plugin_pkgs REQUIRED glib-2.0 tapi libxml-2.0 libwbxml2 drm-client dlog vconf dbus-glib-1)

FOREACH(flag ${sms_plugin_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${SMS-PLUGIN-LIB} SHARED ${SMS-PLUGIN-SRCS})
TARGET_LINK_LIBRARIES(${SMS-PLUGIN-LIB} ${sms_plugin_pkgs_LDFLAGS} ${UTILS-LIB})

INSTALL(TARGETS ${SMS-PLUGIN-LIB} LIBRARY DESTINATION lib)

