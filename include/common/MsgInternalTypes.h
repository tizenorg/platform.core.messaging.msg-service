/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.tizenopensource.org/license
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef MSG_INTERNAL_TYPES_H
#define MSG_INTERNAL_TYPES_H

/**
 *	@file 		MsgInternalTypes.h
 *	@brief 		Defines types of messaging framework
 *	@version 	1.0
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgMmsTypes.h"


/*==================================================================================================
                                    DEFINES
==================================================================================================*/
#define MSG_DATA_ROOT_PATH		"/opt/usr/data/msg-service/"
#define MSG_DATA_PATH				MSG_DATA_ROOT_PATH"msgdata/"
#define MSG_SMIL_FILE_PATH		MSG_DATA_ROOT_PATH"smildata/"
#define MSG_IPC_DATA_PATH			MSG_DATA_ROOT_PATH"ipcdata/"
#define MSG_THUMBNAIL_PATH		MSG_DATA_PATH"thumbnails/"
// temporary
#define TPDU_LOG_FILE 				MSG_DATA_ROOT_PATH"tpduLog.txt"
#define MSG_NATIONAL_SIM	"memory/private/msg-service/national_sim"
#define MSG_SIM_MSISDN	"memory/private/msg-service/msisdn"
#define MAX_FULL_PATH_SIZE		320	// max length for internal file path
#define MAX_PRECONFIG_NUM		8
#define MAX_THREAD_ADDR_LEN	40
#define MAX_THREAD_NAME_LEN	195
#define MAX_THREAD_DATA_LEN	128
#define MAX_CB_MSG_TEXT_LEN	4200	// 1page max char(93)*max page(15)*max bytes of UTF8 1 char(3)
#define MAX_CB_MSG_LANGUAGE_TYPE_LEN	3
#define MAX_ETWS_WARNING_SECURITY_INFO_LEN	50

#define SMS_MINIMUM_SPACE	(3 * 1024)
#define MMS_MINIMUM_SPACE	(600 * 1024)

/*vconf keys*/
#define MSG_SIM_IMSI			"memory/private/msg-service/sim_imsi"
#define MSG_SIM_CHANGED	"memory/private/msg-service/sim_changed"

#define DEFAULT_SETTING_PATH				"db/private/msg-service"

#define DEFAULT_GENERAL_OPT_PATH		DEFAULT_SETTING_PATH"/general"
#define DEFAULT_SMS_SEND_OPT_PATH	DEFAULT_SETTING_PATH"/sms_send"
#define DEFAULT_SMSC_INFO_PATH			DEFAULT_SETTING_PATH"/smsc"
#define DEFAULT_MMS_SEND_OPT_PATH	DEFAULT_SETTING_PATH"/mms_send"
#define DEFAULT_MMS_RECV_OPT_PATH	DEFAULT_SETTING_PATH"/mms_recv"
#define DEFAULT_MMS_STYLE_OPT_PATH	DEFAULT_SETTING_PATH"/mms_style"
#define DEFAULT_PUSH_MSG_OPT_PATH	DEFAULT_SETTING_PATH"/push_msg"
#define DEFAULT_CB_MSG_OPT_PATH		DEFAULT_SETTING_PATH"/cb_msg"
#define DEFAULT_VOICE_MAIL_OPT_PATH	DEFAULT_SETTING_PATH"/voice_mail"
#define DEFAULT_MSGSIZE_OPT_PATH		DEFAULT_SETTING_PATH"/size_opt"
#define DEFAULT_SIM_COUNT_PATH			DEFAULT_SETTING_PATH"/sim_count"

#define MSG_KEEP_COPY				DEFAULT_GENERAL_OPT_PATH"/keep_copy"
#define MSG_ALERT_TONE				VCONFKEY_SETAPPL_NOTI_MSG_ALERT_REP_TYPE_INT
#define MSG_AUTO_ERASE				DEFAULT_GENERAL_OPT_PATH"/auto_erase"
#define MSG_BLOCK_MESSAGE		DEFAULT_GENERAL_OPT_PATH"/block_msg"
#define CONTACT_SYNC_TIME		DEFAULT_GENERAL_OPT_PATH"/contact_sync_time"

#define SMS_SEND_DCS							DEFAULT_SMS_SEND_OPT_PATH"/dcs"
#define SMS_SEND_NETWORK_MODE	VCONFKEY_MESSAGE_NETWORK_MODE
#define SMS_SEND_REPLY_PATH			DEFAULT_SMS_SEND_OPT_PATH"/reply_path"
#define SMS_SEND_DELIVERY_REPORT	DEFAULT_SMS_SEND_OPT_PATH"/delivery_report"
#define SMS_SEND_SAVE_STORAGE		DEFAULT_SMS_SEND_OPT_PATH"/save_storage"

#define SMSC_TOTAL_COUNT		DEFAULT_SMSC_INFO_PATH"/total_count"
#define SMSC_SELECTED			DEFAULT_SMSC_INFO_PATH"/selected"
#define SMSC_PID						DEFAULT_SMSC_INFO_PATH"/pid"
#define SMSC_VAL_PERIOD		DEFAULT_SMSC_INFO_PATH"/val_period"
#define SMSC_NAME					DEFAULT_SMSC_INFO_PATH"/name"
#define SMSC_TON						DEFAULT_SMSC_INFO_PATH"/ton"
#define SMSC_NPI						DEFAULT_SMSC_INFO_PATH"/npi"
#define SMSC_ADDRESS				DEFAULT_SMSC_INFO_PATH"/address"

#define MMS_SEND_MSG_CLASS							DEFAULT_MMS_SEND_OPT_PATH"/msg_class"
#define MMS_SEND_PRIORITY								DEFAULT_MMS_SEND_OPT_PATH"/priority"
#define MMS_SEND_EXPIRY_TIME							DEFAULT_MMS_SEND_OPT_PATH"/expiry_time"
#define MMS_SEND_CUSTOM_DELIVERY				DEFAULT_MMS_SEND_OPT_PATH"/custom_delivery"
#define MMS_SEND_DELIVERY_TIME						DEFAULT_MMS_SEND_OPT_PATH"/delivery_time"
#define MMS_SEND_SENDER_VISIBILITY				DEFAULT_MMS_SEND_OPT_PATH"/sender_visibility"
#define MMS_SEND_DELIVERY_REPORT					DEFAULT_MMS_SEND_OPT_PATH"/delivery_report"
#define MMS_SEND_READ_REPLY							DEFAULT_MMS_SEND_OPT_PATH"/read_reply"
#define MMS_SEND_KEEP_COPY							DEFAULT_MMS_SEND_OPT_PATH"/keep_copy"
#define MMS_SEND_BODY_REPLYING						DEFAULT_MMS_SEND_OPT_PATH"/body_replying"
#define MMS_SEND_HIDE_RECIPIENTS					DEFAULT_MMS_SEND_OPT_PATH"/hide_recipients"
#define MMS_SEND_REPORT_ALLOWED				DEFAULT_MMS_SEND_OPT_PATH"/report_allowed"
#define MMS_SEND_REPLY_CHARGING					DEFAULT_MMS_SEND_OPT_PATH"/reply_charging"
#define MMS_SEND_REPLY_CHARGING_DEADLINE	DEFAULT_MMS_SEND_OPT_PATH"/reply_charging_deadline"
#define MMS_SEND_REPLY_CHARGING_SIZE			DEFAULT_MMS_SEND_OPT_PATH"/reply_charging_size"
#define MMS_SEND_CREATION_MODE					DEFAULT_MMS_SEND_OPT_PATH"/creation_mode"

#define MMS_RECV_HOME_NETWORK		DEFAULT_MMS_RECV_OPT_PATH"/home_network"
#define MMS_RECV_ABROAD_NETWORK	DEFAULT_MMS_RECV_OPT_PATH"/abroad_network"
#define MMS_RECV_READ_RECEIPT			DEFAULT_MMS_RECV_OPT_PATH"/read_receipt"
#define MMS_RECV_DELIVERY_RECEIPT	DEFAULT_MMS_RECV_OPT_PATH"/delivery_receipt"
#define MMS_RECV_REJECT_UNKNOWN		DEFAULT_MMS_RECV_OPT_PATH"/reject_unknown"
#define MMS_RECV_REJECT_ADVERTISE	DEFAULT_MMS_RECV_OPT_PATH"/reject_advertisement"

#define MMS_STYLE_FONT_SIZE							DEFAULT_MMS_STYLE_OPT_PATH"/font_size"
#define MMS_STYLE_FONT_STYLE_BOLD			DEFAULT_MMS_STYLE_OPT_PATH"/font_style/bold"
#define MMS_STYLE_FONT_STYLE_ITALIC			DEFAULT_MMS_STYLE_OPT_PATH"/font_style/italic"
#define MMS_STYLE_FONT_STYLE_UNDERLINE	DEFAULT_MMS_STYLE_OPT_PATH"/font_style/underline"
#define MMS_STYLE_FONT_COLOR_RED			DEFAULT_MMS_STYLE_OPT_PATH"/font_color/red"
#define MMS_STYLE_FONT_COLOR_GREEN		DEFAULT_MMS_STYLE_OPT_PATH"/font_color/green"
#define MMS_STYLE_FONT_COLOR_BLUE			DEFAULT_MMS_STYLE_OPT_PATH"/font_color/blue"
#define MMS_STYLE_FONT_COLOR_HUE			DEFAULT_MMS_STYLE_OPT_PATH"/font_color/hue"
#define MMS_STYLE_BG_COLOR_RED				DEFAULT_MMS_STYLE_OPT_PATH"/bg_color/red"
#define MMS_STYLE_BG_COLOR_GREEN			DEFAULT_MMS_STYLE_OPT_PATH"/bg_color/green"
#define MMS_STYLE_BG_COLOR_BLUE				DEFAULT_MMS_STYLE_OPT_PATH"/bg_color/blue"
#define MMS_STYLE_BG_COLOR_HUE				DEFAULT_MMS_STYLE_OPT_PATH"/bg_color/hue"
#define MMS_STYLE_PAGE_DUR							DEFAULT_MMS_STYLE_OPT_PATH"/page_dur"
#define MMS_STYLE_PAGE_CUSTOM_DUR			DEFAULT_MMS_STYLE_OPT_PATH"/page_custom_dur"
#define MMS_STYLE_PAGE_DUR_MANUAL			DEFAULT_MMS_STYLE_OPT_PATH"/page_dur_manual"

#define PUSH_RECV_OPTION				DEFAULT_PUSH_MSG_OPT_PATH"/recv_option"
#define PUSH_SERVICE_TYPE				DEFAULT_PUSH_MSG_OPT_PATH"/service_load"

#define CB_RECEIVE						DEFAULT_CB_MSG_OPT_PATH"/receive"
#define CB_SAVE						DEFAULT_CB_MSG_OPT_PATH"/save"
#define CB_MAX_SIM_COUNT			DEFAULT_CB_MSG_OPT_PATH"/max_sim_count"
#define CB_CHANNEL_COUNT			DEFAULT_CB_MSG_OPT_PATH"/channel_count"
#define CB_CHANNEL_ACTIVATE		DEFAULT_CB_MSG_OPT_PATH"/channel_activate"
#define CB_CHANNEL_ID_FROM		DEFAULT_CB_MSG_OPT_PATH"/channel_id_from"
#define CB_CHANNEL_ID_TO		DEFAULT_CB_MSG_OPT_PATH"/channel_id_to"
#define CB_CHANNEL_NAME			DEFAULT_CB_MSG_OPT_PATH"/channel_name"
#define CB_LANGUAGE					DEFAULT_CB_MSG_OPT_PATH"/language"

#define VOICEMAIL_NUMBER				DEFAULT_VOICE_MAIL_OPT_PATH"/voice_mail_number"
#define VOICEMAIL_COUNT				DEFAULT_VOICE_MAIL_OPT_PATH"/voice_mail_count"

#define MSGSIZE_OPTION					DEFAULT_MSGSIZE_OPT_PATH"/msg_size"

#define SIM_USED_COUNT					DEFAULT_SIM_COUNT_PATH"/used_cnt"
#define SIM_TOTAL_COUNT				DEFAULT_SIM_COUNT_PATH"/total_cnt"


/*==================================================================================================
                                         TYPES
==================================================================================================*/

/**
 *	@brief	Represents a message main type. \n
 *	The values for this type SHOULD be in _MSG_MAIN_TYPE_E.
 */
typedef unsigned char MSG_MAIN_TYPE_T;


/**
 *	@brief	Represents a message sub type. \n
 *	Each main type of a message can be divided into some sub types. \n
 *	For instance of SMS, the message sub type can be one of the NORMAL, WAPPUSH, CB and so on. \n
 *	The values for this type SHOULD be in _MSG_SUB_TYPE_E.
 */
typedef unsigned char MSG_SUB_TYPE_T;


/**
 *	@brief	Represents a message class. \n
 *	The values for this type SHOULD be in _MSG_CLASS_TYPE_E.
 */
typedef unsigned char MSG_CLASS_TYPE_T;


/**
 *	@brief	Represents a message class. \n
 *	The values for this type SHOULD be in _MSG_MMS_TRANSACTION_TYPE_E.
 */
typedef unsigned char MSG_MMS_TRANSACTION_TYPE_T;


/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/

/**
 *	@brief	Represents a message type.
 */
typedef struct
{
	MSG_MAIN_TYPE_T		mainType;	/**< Message main type. See enum _MSG_MAIN_TYPE_E */
	MSG_SUB_TYPE_T		subType;	/**< Message sub type. See enum _MSG_SUB_TYPE_E */
	MSG_CLASS_TYPE_T	classType;	/**< Message class type. See enum _MSG_CLASS_TYPE_E */
} MSG_MESSAGE_TYPE_S;


/**
 *	@brief	Represents a message in the framework.
 */
typedef struct
{
	msg_message_id_t		msgId;											/**< Indicates the message ID of this message. */
	msg_thread_id_t			threadId;										/**< Indicates the thread ID. */
	msg_folder_id_t			folderId;										/**< Indicates the folder ID. */
	MSG_MESSAGE_TYPE_S		msgType;										/**< Indicates the message type such as SMS and MMS */
	msg_storage_id_t		storageId;										/**< Indicates where the message is saved. */
	int						nAddressCnt;									/**< Indicates the count of addresses. */
	MSG_ADDRESS_INFO_S		addressList[MAX_TO_ADDRESS_CNT];				/**< Indicates the address information list. */
	char					replyAddress[MAX_PHONE_NUMBER_LEN+1];			/**< Indicates the reply address. */
	char					subject[MAX_SUBJECT_LEN+1];						/**< Indicates the message subject. */
	time_t					displayTime;									/**< Indicates the display time related to the specific operation. */
	msg_network_status_t	networkStatus;									/**< Indicates the network status of the message. */
	msg_encode_type_t		encodeType;										/**< Indicates the string encoding type. */
	bool					bRead;											/**< Indicates whether the message is read or not. */
	bool					bProtected;										/**< Indicates whether the message is protected or not. */
	bool					bBackup;										/**< Indicates whether the message was restored from PC. */
	msg_priority_type_t		priority;										/**< Indicates the priority of the message. */
	msg_direction_type_t	direction;										/**< Indicates whether the message is MO or MT (affecting address). */
	MSG_PORT_INFO_S			msgPort;										/**< Indicates the port number information. */
	bool					bTextSms;										/**< Indicates whether the message is just a text message or not. */
	size_t					dataSize;										/**< Indicates the data size. The unit is byte. */
	char					msgData[MAX_MSG_DATA_LEN+1];					/**< Indicates the message payload information as a body. */
	char					msgText[MAX_MSG_TEXT_LEN+1];
	char					thumbPath[MSG_FILEPATH_LEN_MAX+1];
	bool					bStore;											/**< Indicates whether the message is stored or not if it is MWI message. */
} MSG_MESSAGE_INFO_S;

typedef struct
{
	msg_message_id_t		msgId;									/**< Indicates the message ID of this message. */
	msg_thread_id_t			threadId;								/**< Indicates the thread ID. */
	msg_folder_id_t			folderId;								/**< Indicates the folder ID. see enum _MSG_FOLDER_TYPE_E */
	MSG_MAIN_TYPE_T			mainType;								/**< Message main type. See enum _MSG_MAIN_TYPE_E */
	MSG_SUB_TYPE_T			subType;								/**< Message sub type. See enum _MSG_SUB_TYPE_E */
	MSG_CLASS_TYPE_T		classType;								/**< Message class type. See enum _MSG_CLASS_TYPE_E */
	msg_storage_id_t		storageId;								/**< Indicates where the message is saved. see enum _MSG_FOLDER_TYPE_E*/
	msg_struct_list_s 		*addr_list;
	char					replyAddress[MAX_PHONE_NUMBER_LEN+1];	/**< Indicates the reply address. */
	char					subject[MAX_SUBJECT_LEN+1];				/**< Indicates the message subject. */
	time_t					displayTime;							/**< Indicates the display time related to the specific operation. */
	msg_network_status_t	networkStatus;							/**< Indicates the network status of the message. */
	msg_encode_type_t		encodeType;								/**< Indicates the string encoding type. */
	bool					bRead;									/**< Indicates whether the message is read or not. */
	bool					bProtected;								/**< Indicates whether the message is protected or not. */
	bool					bBackup;								/**< Indicates whether the message was restored from PC. */
	msg_priority_type_t		priority;								/**< Indicates the priority of the message. */
	msg_direction_type_t	direction;								/**< Indicates whether the message is MO or MT, affecting address. */
	bool					bPortValid;									/**< Indicates whether port information is used or not. */
	unsigned short			dstPort;								/**< Recipient port number, not greater than 16 bit */
	unsigned short			srcPort;								/**< Sender port number, not greater than 16 bit */
	int						attachCount;							/**< Indicates the count of attached files in mms. */
	char					thumbPath[MSG_FILEPATH_LEN_MAX+1];
	size_t					dataSize;								/**< Indicates the data size. The unit is byte. */
	void					*pData;									/**< Indicates the message payload information as a body. default character encoding is UTF-8*/
	void					*pMmsData;								/**< Indicates the message payload information as a body. default character encoding is UTF-8*/
} MSG_MESSAGE_HIDDEN_S;

/**
 *	@brief	Represents message information for thread view.
 */
typedef struct
{
	msg_thread_id_t			threadId;															/**< Indicates the thread ID of this peer. */
	char					threadName[MAX_THREAD_NAME_LEN+1];		/**< Indicates the name of this peer. > */
	MSG_MAIN_TYPE_T			mainType;								/**< Indicates the latest msg main type. */
	MSG_SUB_TYPE_T			subType;								/**< Indicates the latest msg sub type. */
	char					threadData[MAX_THREAD_DATA_LEN+1];		/**< Indicates the latest msg data. */
	time_t					threadTime;														/**< Indicates the latest msg time. */
	msg_direction_type_t	direction;															/**< Indicates whether the message is MO or MT (affecting address). */
	int						unreadCnt;														/**< Indicates the unread messages from the Peer. */
	int						smsCnt;															/**< Indicates the SMS messages from the Peer. */
	int						mmsCnt;															/**< Indicates the MMS messages from the Peer. */
	bool					bProtected;														/**< Indicates whether the thread includes protected messages.  */
} MSG_THREAD_VIEW_S;


/**
 *	@brief	Represents message information for conversation view.
 */
typedef struct
{
	msg_message_id_t			msgId;									/**< Indicates the message ID of this message. */
	msg_thread_id_t			threadId;								/**< Indicates the thread ID of this peer. */
	MSG_MAIN_TYPE_T			mainType;							/**< Message main type. See enum _MSG_MAIN_TYPE_E */
	MSG_SUB_TYPE_T			subType;								/**< Message sub type. See enum _MSG_SUB_TYPE_E */
	msg_folder_id_t			folderId;								/**< Indicates the folder ID. see enum _MSG_FOLDER_TYPE_E */
	msg_storage_id_t				storageId;								/**< Indicates where the message is saved. see enum _MSG_FOLDER_TYPE_E*/
	time_t								displayTime;						/**< Indicates the display time related to the specific operation. */
	time_t								scheduledTime;					/**< Indicates the time to send scheduled message. */
	msg_network_status_t		networkStatus;						/**< Indicates the network status of the message. */
	bool									bRead;									/**< Indicates whether the message is read or not. */
	bool									bProtected;							/**< Indicates whether the message is protected or not. */
	msg_direction_type_t		direction;								/**< Indicates whether the message is MO or MT, affecting address. */
	int									pageCount;						/**< Indicates the count of pageCount in mms. */
	int									attachCount;						/**< Indicates the count of attached files in mms. */
	char									attachFileName[MSG_FILENAME_LEN_MAX+1];	/**< Indicates the thumbnail path. */
	char									audioFileName[MSG_FILENAME_LEN_MAX+1];	/**< Indicates the thumbnail path. */
	char									imageThumbPath[MSG_FILEPATH_LEN_MAX+1];	/**< Indicates the thumbnail path. */
	char									videoThumbPath[MSG_FILEPATH_LEN_MAX+1];		/**< Indicates the thumbnail path. */
	char									subject[MAX_SUBJECT_LEN+1];							/**< Indicates the message subject. */
	size_t								textSize;								/**< Indicates the data size. The unit is byte. */
	char									*pText;									/**< Indicates the message payload information as a body. default character encoding is UTF-8*/
} MSG_CONVERSATION_VIEW_S;


/**
 *	@brief	Represents sim message informatioin list.
 */
typedef struct
{
	int nIdxCnt;		/**< The count of sim index */
	int nMsgCnt;		/**< The count of sim message */
	MSG_MESSAGE_INFO_S		*simMsgInfo;		/**< The pointer to sim message informatioin */
} MSG_SIM_MSG_INFO_LIST_S;


/**
 *	@brief	Represents a request in the framework. \n
 *	Applications compose a request and send it to the framework via Message handle. \n
 *	This request ID is used to manage the request by the framework.
 */
typedef struct
{
	msg_request_id_t		reqId;		/**< Indicates the request ID, which is unique.
										When applications submit a request to the framework, this value will be set by the framework. */
	MSG_MESSAGE_INFO_S		msgInfo;	/**< Indicates the message structure to be sent by applications. */
	MSG_SENDINGOPT_INFO_S	sendOptInfo;
} MSG_REQUEST_INFO_S;


/**
 *	@brief	Represents proxy information. \n
 *	This stucture contains the information about the status cnf of a sent message.
 */
typedef struct
{
	int						listenerFd;		/**< Rx fd for status cnf */
	unsigned int			handleAddr;		/**< Handle address for status cnf */
	msg_message_id_t		sentMsgId;		/**< The ID of a sent message for updating message status */
} MSG_PROXY_INFO_S;


/**
 *	@brief	Aux data structure for MSG_CMD_REG_INCOMING_MSG_CB. \n
 *	This stucture contains the information about the receiver for msgType and port.
 */
typedef struct
{
	int 				listenerFd;
	MSG_MAIN_TYPE_T 	msgType;
	unsigned short 		port;
} MSG_CMD_REG_INCOMING_MSG_CB_S;

typedef struct
{
	int 				listenerFd;
	MSG_MAIN_TYPE_T 	msgType;
	bool 				bsave;
} MSG_CMD_REG_CB_INCOMING_MSG_CB_S;


/**
 *	@brief	Aux data structure for MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB. \n
 *	This stucture contains the information about the receiver for msgType and port.
 */
typedef struct
{
	int 				listenerFd;
	MSG_MAIN_TYPE_T 	msgType;
	char appId[MAX_MMS_JAVA_APPID_LEN+1];
} MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB_S;


/**
 *	@brief	Aux data structure for MSG_CMD_REG_INCOMING_SYNCML_MSG_CB. \n
 *	This stucture contains the information about the receiver for msgType and port.
 */
typedef struct
{
	int 				listenerFd;
	MSG_MAIN_TYPE_T 	msgType;
} MSG_CMD_REG_INCOMING_SYNCML_MSG_CB_S;


/**
 *	@brief	Aux data structure for MSG_CMD_REG_INCOMING_LBS_MSG_CB_S. \n
 *	This stucture contains the information about the receiver for msgType and port.
 */
typedef struct
{
	int 				listenerFd;
	MSG_MAIN_TYPE_T 	msgType;
} MSG_CMD_REG_INCOMING_LBS_MSG_CB_S;


/**
 *	@brief	Aux data structure for MSG_CMD_REG_INCOMING_JAVAMMS_TRID_S. \n
 *	This stucture contains the information about the sent Java MMS messge transactionId.
 */
typedef struct
{
	bool 			posted;
	char 			id[MMS_TR_ID_LEN+1];
	char				pduFileName[MAX_COMMON_INFO_SIZE+1];
} MSG_CMD_REG_INCOMING_JAVAMMS_TRID_S;


/**
 *	@brief	Aux data structure for MSG_CMD_REG_SYNCML_MSG_OPERATION_CB. \n
 *	This stucture contains the information about the receiver for msgType and port.
 */
typedef struct
{
	int 				listenerFd;
	MSG_MAIN_TYPE_T 	msgType;
} MSG_CMD_REG_SYNCML_MSG_OPERATION_CB_S;

typedef struct
{
	int 				listenerFd;
	MSG_MAIN_TYPE_T 	msgType;
	char appId[MAX_WAPPUSH_ID_LEN+1];
	char content_type[MAX_WAPPUSH_CONTENT_TYPE_LEN+1];
} MSG_CMD_REG_INCOMING_PUSH_MSG_CB_S;

typedef struct
{
	int 				listenerFd;
	MSG_MAIN_TYPE_T 	msgType;
	bool				bsave;
} MSG_CMD_REG_INCOMING_CB_MSG_CB_S;


typedef struct
{
	int						alarm_id;
	MSG_REQUEST_INFO_S		reqInfo;
}MSG_SCHEDULED_MSG_S;

/**
 *	@brief	Represents a CB message in the framework.
 */
typedef struct
{
	MSG_SUB_TYPE_T			type;
	time_t					receivedTime;

	unsigned short			serialNum;
	unsigned short			messageId;	// Message Identifier
	unsigned char			dcs;		// data coding scheme
	int						cbTextLen;	// length of cbText
	unsigned char			cbText[MAX_CB_MSG_TEXT_LEN];// cb message text (UTF8)

	unsigned short			etwsWarningType;
	unsigned char			etwsWarningSecurityInfo[MAX_ETWS_WARNING_SECURITY_INFO_LEN];
	unsigned char			language_type[MAX_CB_MSG_LANGUAGE_TYPE_LEN];
} MSG_CB_MSG_S;


/*==================================================================================================
                                         ENUMS
==================================================================================================*/

/**
 *	@brief	Represents the values of a message main type. \n
 *	Three main types of a message are predefined : SMS, MMS, and Email. More main types of a message can be defined here. \n
 *	This enum is used as the value of MSG_MAIN_TYPE_T.
 */
enum _MSG_MAIN_TYPE_E
{
	MSG_UNKNOWN_TYPE = 0,		/**< Unknown main type */
	MSG_SMS_TYPE,				/**< SMS */
	MSG_MMS_TYPE,				/**< MMS */
};


/**
 *	@brief	Represents the values of a message sub type. \n
 *	Three sub types of a message are predefined : NORMAL, WAPPUSH, and CB. More sub types of a message can be defined here. \n
 *	This enum is used as the value of MSG_SUB_TYPE_T.
 */
enum _MSG_SUB_TYPE_E
{
	/* SMS Specific Message Type */
	MSG_NORMAL_SMS = 0,			/**< Text SMS message */
	MSG_CB_SMS,					/**< Cell Broadcasting  message */
	MSG_JAVACB_SMS,				/**< JAVA Cell Broadcasting  message */
	MSG_TYPE0_SMS,					/**< Short Message Type 0 */
	MSG_REPLACE_TYPE1_SMS,		/**< Replace Short Message Type 1 */
	MSG_REPLACE_TYPE2_SMS,		/**< Replace Short Message Type 2 */
	MSG_REPLACE_TYPE3_SMS,		/**< Replace Short Message Type 3 */
	MSG_REPLACE_TYPE4_SMS,		/**< Replace Short Message Type 4 */
	MSG_REPLACE_TYPE5_SMS,		/**< Replace Short Message Type 5 */
	MSG_REPLACE_TYPE6_SMS,		/**< Replace Short Message Type 6 */
	MSG_REPLACE_TYPE7_SMS,		/**< Replace Short Message Type 7 */
	MSG_WAP_SI_SMS,				/**< WAP Push Message SI */
	MSG_WAP_SL_SMS,				/**< WAP Push Message SL */
	MSG_WAP_CO_SMS,				/**< WAP Push Message CO */
	MSG_MWI_VOICE_SMS,			/**< MWI Message Voice */
	MSG_MWI_FAX_SMS,				/**< MWI Message Fax */
	MSG_MWI_EMAIL_SMS,			/**< MWI Message Email */
	MSG_MWI_OTHER_SMS,			/**< MWI Message Other */
	MSG_STATUS_REPORT_SMS,		/**< SMS-STATUS-REPORT */
	MSG_SYNCML_CP,				/**< SyncML Message CP */
	MSG_LBS_SMS,					/**< LBS Message */
	MSG_REJECT_SMS,				/**< Reject Message */
	MSG_CONCAT_SIM_SMS,			/**< Concatenated Message in SIM */

	/* MMS Specific Message Type */
	MSG_SENDREQ_MMS = 24,					/**< MMS Send Request message */
	MSG_SENDCONF_MMS,				/**< MMS Send Confirm message */
	MSG_NOTIFICATIONIND_MMS,			/**< MMS Notification Indication message */
	MSG_GET_MMS,						/**< MMS GET MMS message */
	MSG_NOTIFYRESPIND_MMS,			/**< MMS Notify Response Indication message */
	MSG_RETRIEVE_MMS,					/**< MMS Retrive MMS message */
	MSG_RETRIEVE_AUTOCONF_MMS,		/**< MMS Retrieve Confirm message by auto retrieving*/
	MSG_RETRIEVE_MANUALCONF_MMS,	/**< MMS Retrieve Confirm message by manual retrieving*/
	MSG_ACKNOWLEGEIND_MMS,			/**< MMS Acknowledge Indication message */
	MSG_DELIVERYIND_MMS,				/**< MMS Delivery Indication message */
	MSG_READRECIND_MMS,				/**< MMS Read Receive Indication message */
	MSG_READORGIND_MMS,				/**< MMS Read Origin Indication message */
	MSG_FORWARD_MMS,					/**< MMS Forward message */
	MSG_FORWARDREQ_MMS,				/**< MMS Forward Request message */
	MSG_FORWARDCONF_MMS,			/**< MMS Forward Confirm message */
	MSG_READREPLY_MMS,				/**< MMS Read Reply message */
	MSG_SENDREQ_JAVA_MMS,  			/**< MMS Send Request message for JAVA MMS */

	MSG_ETWS_SMS,
};

/**
 *	@brief	Represents the values of a message transaction type. \n
 *	This enum is used as the value of MSG_MMS_TRANSACTION_TYPE_T.
 */
enum _MSG_MMS_TRANSACTION_TYPE_E
{
	MSG_MMS_SEND_COMPLETE = 0,
	MSG_MMS_RETRIEVE_COMPLETE,
	MSG_MMS_UNKNOWN,
};

/**
 *	@brief	Represents the values of File Type of MMS. \n
 *	This enum is used as the value of .
 */
enum _MSG_MMS_ITEM_TYPE_E
{
	MSG_MMS_ITEM_TYPE_IMG,			/**< Indicates the image media */
	MSG_MMS_ITEM_TYPE_AUDIO,		/**< Indicates the audio media */
	MSG_MMS_ITEM_TYPE_VIDEO,		/**< Indicates the video media */
	MSG_MMS_ITEM_TYPE_ATTACH,		/**< Indicates the attach file */
	MSG_MMS_ITEM_TYPE_PAGE,	/**< Indicates the page count */
};
#endif

