
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
ADD_SUBDIRECTORY(plugin/sms_plugin)
ADD_SUBDIRECTORY(plugin/mms_plugin)
ADD_SUBDIRECTORY(msg_helper)


##########################################################
# Install Header Files
##########################################################
SET(MAPI-HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/include/mapi/msg.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/mapi/msg_types.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/mapi/msg_storage.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/mapi/msg_storage_types.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/mapi/msg_transport.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/mapi/msg_transport_types.h
	${CMAKE_CURRENT_SOURCE_DIR}/SLP_MessagingFW_PG.h )
INSTALL(FILES ${MAPI-HEADERS} DESTINATION include/msg-service)


##########################################################
# Install etc Files
##########################################################

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/plugin.cfg DESTINATION /usr/share/msg-service)

SET(RINGTONE-FILES
	${CMAKE_CURRENT_SOURCE_DIR}/config/message_tone/Sherbet.wav
)

INSTALL(FILES ${RINGTONE-FILES} DESTINATION /usr/share/media/)

CONFIGURE_FILE(msg-service.pc.in msg-service.pc @ONLY)
INSTALL(FILES ${CMAKE_BINARY_DIR}/msg-service.pc DESTINATION lib/pkgconfig)

SET(RC_LOCAL_SCRIPT msg-server)
INSTALL(PROGRAMS ${RC_LOCAL_SCRIPT} DESTINATION /etc/rc.d/init.d)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/packaging/msg-service.service DESTINATION /usr/lib/systemd/user)

# LICENSE file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION /usr/share/license/msg-service)
