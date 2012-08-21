CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(msg-server CXX)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -O0 -g -Wall")

#########################################################
# Define Plugin Manager
##########################################################

SET(PLUGIN-MANAGER-SRCS
	${CMAKE_SOURCE_DIR}/framework/plugin-manager/MsgPluginManager.cpp
	${CMAKE_SOURCE_DIR}/framework/plugin-manager/MsgPluginConfig.cpp
)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include/mapi
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/framework
	${CMAKE_SOURCE_DIR}/include/utils
)

INCLUDE(FindPkgConfig)
pkg_check_modules(plugin_manager_pkgs REQUIRED glib-2.0 dlog vconf)

FOREACH(flag ${plugin_manager_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${PLUGIN-MANAGER-LIB} SHARED ${PLUGIN-MANAGER-SRCS})
TARGET_LINK_LIBRARIES(${PLUGIN-MANAGER-LIB} ${plugin_manager_pkgs_LDFLAGS} ${UTILS-LIB} dl)

INSTALL(TARGETS ${PLUGIN-MANAGER-LIB} LIBRARY DESTINATION lib)

##########################################################
# Define Messaging Framework Handler
##########################################################

SET(FW-HANDLER-SRCS
	${CMAKE_SOURCE_DIR}/framework/setting-handler/MsgSettingHandler.cpp
	${CMAKE_SOURCE_DIR}/framework/storage-handler/MsgStorageManager.cpp
	${CMAKE_SOURCE_DIR}/framework/storage-handler/MsgStorageMessage.cpp
	${CMAKE_SOURCE_DIR}/framework/storage-handler/MsgStorageMms.cpp
	${CMAKE_SOURCE_DIR}/framework/storage-handler/MsgStorageFolder.cpp
	${CMAKE_SOURCE_DIR}/framework/storage-handler/MsgStorageFilter.cpp
	${CMAKE_SOURCE_DIR}/framework/storage-handler/MsgStorageSim.cpp
	${CMAKE_SOURCE_DIR}/framework/storage-handler/MsgStorageUtil.cpp
	${CMAKE_SOURCE_DIR}/framework/submit-handler/MsgSubmitHandler.cpp
	${CMAKE_SOURCE_DIR}/framework/deliver-handler/MsgDeliverHandler.cpp
)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include/mapi
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/framework
	${CMAKE_SOURCE_DIR}/include/utils
)

INCLUDE(FindPkgConfig)
pkg_check_modules(fw_handler_pkgs REQUIRED glib-2.0 dlog vconf capi-appfw-application)

FOREACH(flag ${fw_handler_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} -std=c++0x")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${FW-HANDLER-LIB} SHARED ${FW-HANDLER-SRCS})
TARGET_LINK_LIBRARIES(${FW-HANDLER-LIB} ${fw_handler_pkgs_LDFLAGS} ${UTILS-LIB} ${PLUGIN-MANAGER-LIB})

INSTALL(TARGETS ${FW-HANDLER-LIB} DESTINATION lib COMPONENT RuntimeLibraries)

##########################################################
# Define Transaction Manager
##########################################################

SET(TRANS-MANAGER-SRCS
	${CMAKE_SOURCE_DIR}/framework/transaction-manager/MsgCmdHandlerSetting.cpp
	${CMAKE_SOURCE_DIR}/framework/transaction-manager/MsgCmdHandlerStorage.cpp
	${CMAKE_SOURCE_DIR}/framework/transaction-manager/MsgCmdHandlerTransport.cpp
	${CMAKE_SOURCE_DIR}/framework/transaction-manager/MsgCmdHandlerFilter.cpp
	${CMAKE_SOURCE_DIR}/framework/transaction-manager/MsgTransManager.cpp
)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include/mapi
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/framework
	${CMAKE_SOURCE_DIR}/include/utils
)

INCLUDE(FindPkgConfig)
pkg_check_modules(trans_manager_pkgs REQUIRED glib-2.0 security-server dlog vconf)

FOREACH(flag ${trans_manager_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${TRANS-MANAGER-LIB} SHARED ${TRANS-MANAGER-SRCS})
TARGET_LINK_LIBRARIES(${TRANS-MANAGER-LIB} ${trans_manager_pkgs_LDFLAGS} ${UTILS-LIB} ${FW-HANDLER-LIB})

INSTALL(TARGETS ${TRANS-MANAGER-LIB} DESTINATION lib COMPONENT RuntimeLibraries)

##########################################################
# Define Execute File
##########################################################

SET(MAIN-SRCS
	${CMAKE_SOURCE_DIR}/framework/main.cpp
)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include/mapi
	${CMAKE_SOURCE_DIR}/include/common
	${CMAKE_SOURCE_DIR}/include/framework
	${CMAKE_SOURCE_DIR}/include/utils
)

INCLUDE(FindPkgConfig)
pkg_check_modules(main_pkgs REQUIRED glib-2.0 dbus-glib-1 dlog vconf)

FOREACH(flag ${main_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${PROJECT_NAME} ${MAIN-SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${main_pkgs_LDFLAGS} ${UTILS-LIB} ${PLUGIN-MANAGER-LIB} ${TRANS-MANAGER-LIB} ${FW-HANDLER-LIB})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin)

