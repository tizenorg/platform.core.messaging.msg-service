CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

##########################################################
# Set Variables
##########################################################
SET(VOBJECT-LIB msg_vobject)
SET(UTILS-LIB msg_utils)
SET(EXTERNALS-LIB msg_externals)
SET(PLUGIN-MANAGER-LIB msg_plugin_manager)
SET(FW-HANDLER-LIB msg_framework_handler)
SET(TRANS-MANAGER-LIB msg_transaction_manager)
SET(TRANS-PROXY-LIB msg_transaction_proxy)
SET(MAPI-LIB msg_mapi)
SET(SMS-PLUGIN-LIB msg_sms_plugin)
SET(MMS-PLUGIN-LIB msg_mms_plugin)

IF(_FEATURE_SMS_CDMA)
	ADD_DEFINITIONS("-DFEATURE_SMS_CDMA")
ENDIF(_FEATURE_SMS_CDMA)

#IF(_MSG_WEARABLE_PROFILE)
	ADD_DEFINITIONS("-DMSG_WEARABLE_PROFILE")
#ENDIF(_MSG_WEARABLE_PROFILE)

ADD_DEFINITIONS("-D_FILE_OFFSET_BITS=64")
ADD_DEFINITIONS("-DMSG_PENDING_PUSH_MESSAGE")
ADD_DEFINITIONS("-DMSG_CONTACTS_SERVICE_NOT_SUPPORTED")
##########################################################
# Add Subdirectories
##########################################################
ADD_SUBDIRECTORY(vobject-engine)
ADD_SUBDIRECTORY(utils)
ADD_SUBDIRECTORY(externals)
ADD_SUBDIRECTORY(framework)
ADD_SUBDIRECTORY(proxy)
ADD_SUBDIRECTORY(mapi)
IF(_FEATURE_SMS_CDMA)
ADD_SUBDIRECTORY(plugin/sms_cdma_plugin)
ELSE(_FEATURE_SMS_CDMA)
ADD_SUBDIRECTORY(plugin/sms_plugin)
ENDIF(_FEATURE_SMS_CDMA)
ADD_SUBDIRECTORY(plugin/mms_plugin)

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
	)
INSTALL(FILES ${MAPI-HEADERS} DESTINATION include/msg-service)


##########################################################
# Install etc Files
##########################################################

#INSTALL(FILES ${MMS-FILES} DESTINATION /opt/etc/msg-service)

CONFIGURE_FILE(msg-service.pc.in msg-service.pc @ONLY)
INSTALL(FILES ${CMAKE_BINARY_DIR}/msg-service.pc DESTINATION lib/pkgconfig)

# LICENSE file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.APLv2 DESTINATION /usr/share/license/msg-service)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/msg-service.rule DESTINATION /etc/smack/accesses.d/)

#Install sysinfo file to provide which features are supported.
IF(_FEATURE_SMS_CDMA)
	INSTALL(FILES ${CMAKE_SOURCE_DIR}/config/msg-service-db.CDMA.sql DESTINATION /usr/share/msg-service RENAME msg-service-db.sql)
	INSTALL(FILES ${CMAKE_SOURCE_DIR}/config/sysinfo-message.xml DESTINATION /etc/config)
ELSE(_FEATURE_SMS_CDMA)
	INSTALL(FILES ${CMAKE_SOURCE_DIR}/config/msg-service-db.sql DESTINATION /usr/share/msg-service RENAME msg-service-db.sql)
	INSTALL(FILES ${CMAKE_SOURCE_DIR}/config/sysinfo-message.xml DESTINATION /etc/config)
ENDIF(_FEATURE_SMS_CDMA)
