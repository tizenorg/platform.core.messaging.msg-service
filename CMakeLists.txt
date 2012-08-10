
##########################################################
# Set Variables
##########################################################
SET(UTILS-LIB msg_utils)
SET(PLUGIN-MANAGER-LIB msg_plugin_manager)
SET(FW-HANDLER-LIB msg_framework_handler)
SET(TRANS-MANAGER-LIB msg_transaction_manager)
SET(TRANS-PROXY-LIB msg_transaction_proxy)
SET(MAPI-LIB msg_mapi)
SET(SMS-PLUGIN-LIB msg_sms_plugin)
SET(MMS-PLUGIN-LIB msg_mms_plugin)
SET(MMS-LANGUAGE-PACK-LIB msg_mms_language_pack)

##########################################################
# Add Subdirectories
##########################################################
ADD_SUBDIRECTORY(utils)
ADD_SUBDIRECTORY(framework)
ADD_SUBDIRECTORY(proxy)
ADD_SUBDIRECTORY(mapi)
ADD_SUBDIRECTORY(test_app)
ADD_SUBDIRECTORY(plugin/sms_plugin)
ADD_SUBDIRECTORY(plugin/mms_plugin)
ADD_SUBDIRECTORY(msg_helper)


##########################################################
# Install Header Files
##########################################################
SET(MAPI-HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/include/mapi/MapiControl.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/mapi/MapiSetting.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/mapi/MapiStorage.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/mapi/MapiTransport.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/mapi/MapiMessage.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/common/MsgTypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/common/MsgMmsTypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/common/MsgSettingTypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/common/MsgStorageTypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/common/MsgTransportTypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/SLP_MessagingFW_PG.h )
INSTALL(FILES ${MAPI-HEADERS} DESTINATION include/msg-service)


##########################################################
# Install etc Files
##########################################################

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/plugin.cfg DESTINATION /usr/share/msg-service)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/alert_on_call.mp3 DESTINATION /opt/etc/msg-service)

SET(MMS-FILES
	${CMAKE_CURRENT_SOURCE_DIR}/test_app/mms_files/A.smi
	${CMAKE_CURRENT_SOURCE_DIR}/test_app/mms_files/audio.amr
	${CMAKE_CURRENT_SOURCE_DIR}/test_app/mms_files/P091120_104633.jpg
	${CMAKE_CURRENT_SOURCE_DIR}/test_app/mms_files/Temp0_2.txt
	${CMAKE_CURRENT_SOURCE_DIR}/test_app/mms_files/Temp1_0.txt
	${CMAKE_CURRENT_SOURCE_DIR}/test_app/mms_files/V091120_104905.3gp
)

INSTALL(FILES ${MMS-FILES} DESTINATION /opt/etc/msg-service)

SET(RINGTONE-FILES
	${CMAKE_CURRENT_SOURCE_DIR}/config/message_tone/Sherbet.wav
)

INSTALL(FILES ${RINGTONE-FILES} DESTINATION /usr/share/media/)

CONFIGURE_FILE(msg-service.pc.in msg-service.pc @ONLY)
INSTALL(FILES ${CMAKE_BINARY_DIR}/msg-service.pc DESTINATION lib/pkgconfig)

SET(RC_LOCAL_SCRIPT msg-server)
INSTALL(PROGRAMS ${RC_LOCAL_SCRIPT} DESTINATION /etc/rc.d/init.d)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/msg-service.conf DESTINATION /etc/dbus-1/system.d)
