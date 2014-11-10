/*
 * msg-service
 *
 * Copyright (c) 2000 - 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef MSG_TYPES_H_
#define MSG_TYPES_H_

/*==================================================================================================
                                    DEFINES
==================================================================================================*/

#define DEPRECATED __attribute__((deprecated))

#ifndef EXPORT_API
#define EXPORT_API __attribute__ ((visibility("default")))
#endif

#define MAX_TO_ADDRESS_CNT		10
#define MAX_PHONE_NUMBER_LEN	20
#define MAX_ADDRESS_VAL_LEN	254
#define MAX_SIGN_VAL_LEN		128
#define MAX_SUBJECT_LEN			120
#define MAX_DISPLAY_NAME_LEN	195
#define MAX_IMAGE_PATH_LEN		1024
#define MAX_MSG_DATA_LEN		320
#define MAX_MSG_TEXT_LEN		1530
#define MAX_FILTER_VALUE_LEN 	40
#define MAX_COMMAND_LEN		1024
#define MAX_FOLDER_NAME_SIZE		20

#define MAX_WAPPUSH_ID_LEN				100
#define MAX_WAPPUSH_CONTENT_TYPE_LEN	100
#define MAX_WAPPUSH_HREF_LEN			300
#define MAX_WAPPUSH_CONTENTS_LEN		2048

#define MAX_PUSH_CACHEOP_INVALID_OBJECT_MAX	5
#define MAX_PUSH_CACHEOP_INVALID_SERVICE_MAX	5
#define MAX_PUSH_CACHEOP_MAX_URL_LEN			200

#define MAX_COMMON_INFO_SIZE	20

#define MAX_SEGMENT_NUM			15

/**
 *	@brief	Defines the maximum file path length
 */
#define	MSG_FILEPATH_LEN_MAX	1024

// setting
#define SMSC_NAME_MAX			127		/* Max SMSC Name Length */
#define SMSC_ADDR_MAX			20		/* Max SMSC Adress Length  */
#define SMSC_LIST_MAX			5		/* Max List Count of SMSC */
#define CB_CHANNEL_NAME_MAX 	32		/* cb channel name max size */
#define CB_CHANNEL_MAX			30		/* cb channel max count */
#define CB_LANG_TYPE_MAX		10

/**
 *	@brief	Defines the maximum file name length
 */
#define	MSG_FILENAME_LEN_MAX	1024

/**
 *	@brief	Defines the maximum message id length
 */
#define	MSG_MSG_ID_LEN			100

/**
 *	@brief	Defines the maximum in transition id length
 */
#define MAX_SMIL_TRANSIN_ID		100

/**
 *	@brief	Defines the maximum out transition id length
 */
#define MAX_SMIL_TRANSOUT_ID	100

/**
 *	@brief	Defines the maximum region id length
 */
#define	MAX_SMIL_REGION_ID		151

/**
 *	@brief	Defines the maximum transition id length
 */
#define MAX_SMIL_TRANSITION_ID		151

/**
 *	@brief	Defines the maximum meta id length
 */
#define MAX_SMIL_META_ID		151

/**
 *	@brief	Defines the maximum meta name length
 */
#define MAX_SMIL_META_NAME		100

/**
 *	@brief	Defines the maximum meta content length
 */
#define MAX_SMIL_META_CONTENT		255

/**
 *	@brief	Defines the maximum fit size
 */
#define	MAX_SMIL_FIT_SIZE		31

/**
 *	@brief	Defines the maximum pages in a MMS
 */
#define	MMS_PAGE_MAX			20

/**
 *	@brief	Defines the maximum media in a MMS
 */
#define	MMS_MEDIA_MAX			60

/**
 *	@brief	Defines the maximum alternate text length
 */
#define	MAX_SMIL_ALT_LEN		255


//font size
/**
 *	@brief	Defines the small font size
 */
#define	MMS_SMIL_FONT_SIZE_SMALL 		24

/**
 *	@brief	Defines the normal font size
 */
#define	MMS_SMIL_FONT_SIZE_NORMAL		30

/**
 *	@brief	Defines the large font size
 */
#define	MMS_SMIL_FONT_SIZE_LARGE 		36

#define MAX_SMIL_ANIMATE_ATTRIBUTE_NAME 100
#define MAX_SMIL_ANIMATE_ATTRIBUTE_TYPE 100
#define MAX_SMIL_ANIMATE_TARGET_ELEMENT 100
#define MAX_SMIL_ANIMATE_CALC_MODE 100

#define MAX_MMS_JAVA_APPID_LEN 32

#define CONV_THUMB_WIDTH	250
#define CONV_THUMB_HEIGHT 170

#define MMS_TR_ID_LEN		40
#define MMS_MSG_ID_LEN		40
#define MMS_LOCATION_LEN	100

#define MAX_MMS_TEXT_LEN		2000
/*==================================================================================================
                                         TYPES
==================================================================================================*/

typedef struct	opq_msg_svc_s *			msg_handle_t;
typedef struct	opq_struct_s *				msg_struct_t;


typedef struct msg_list_handle *msg_list_handle_t;


typedef struct {
	int					nCount;				/**< Indicates the count of msg_struct_t. */
	msg_struct_t		*msg_struct_info;	/**< Indicates the msg_struct_t information list. */
}msg_struct_list_s;


/**
 *	@brief	Represents a messsage ID.
 */
typedef unsigned int msg_message_id_t;


/**
 *	@brief	Represents a storage type.
 	See enum _MSG_STORAGE_ID_E
 */
typedef unsigned char msg_storage_id_t;


/**
 *	@brief	Represents a folder ID.
 	See enum _MSG_FOLDER_ID_E
 */
typedef char msg_folder_id_t;


/**
 *	@brief	Represents a request ID, which is unique for each request.
 */
typedef unsigned int msg_request_id_t;


/**
 *	@brief	Represents a message priority. \n
 *	The values for this type SHOULD be in _MSG_PRIORITY_TYPE_E.
 */
typedef unsigned char msg_priority_type_t;


/**
 *	@brief	Represents the network status of a message. \n
 *	The values for this type SHOULD be in _MSG_NETWORK_STATUS_E.
 */
typedef unsigned char msg_network_status_t;


/**
 *	@brief	Represents an address type. \n
 *	The values for this type SHOULD be in _MSG_ADDRESS_TYPE_E.
 */
typedef unsigned char msg_address_type_t;


/**
 *	@brief	Represents an recipient type. \n
 *	The values for this type SHOULD be in _MSG_RECIPIENT_TYPE_E.
 */
typedef unsigned char msg_recipient_type_t;


/**
 *	@brief	Represents the type of a message direction. \n
 *	The values for this type SHOULD be in _MSG_DIRECTION_TYPE_E.
 */
typedef unsigned char msg_direction_type_t;


/**
 *	@brief	Represents an encoding type. \n
 *	The values for this type SHOULD be in _MSG_ENCODE_TYPE_E.
 */
typedef unsigned char msg_encode_type_t;


/**
 *	@brief	Represents an error code. \n
 *	The values for this type SHOULD be in _MSG_ERROR_E
 */
typedef int msg_error_t;


/**
\brief Represents WAP Push App Code.
*/
typedef unsigned char msg_push_action_t;


/**
\brief Represents SyncML Message Type.
*/
typedef unsigned short msg_syncml_message_type_t;


/**
 *	@brief	Represents a Contact ID.
 */
typedef unsigned int msg_contact_id_t;


/**
  *  @brief	Represents a Report Type.
 *	The values for this type SHOULD be in _MSG_REPORT_TYPE_E
  */
typedef int msg_report_type_t;


/**
  *  @brief	Represents a Delivery Report Status.
 *	The values for this type SHOULD be in _MSG_DELIVERY_REPORT_STATUS_E
  */
typedef int msg_delivery_report_status_t;


/**
 *	@brief	Represents a Read Report Status.
 *	The values for this type SHOULD be in _MSG_READ_REPORT_STATUS_E
 */
typedef int msg_read_report_status_t;


/**
 *	@brief	Represents a Message Type.
 *	The values for this type SHOULD be in _MSG_MESSAGE_TYPE_E
*/
typedef unsigned short msg_message_type_t;

/**
 *	@brief	Represents a Message Backup Type.
 *	The values for this type SHOULD be in _MSG_MESSAGE_BACKUP_TYPE_E
*/
typedef unsigned int msg_message_backup_type_t;

/**
 *	@brief	Represents a thread ID. \n
 */
typedef unsigned int msg_thread_id_t;

// filter
/**
 *	@brief	Represents a filter ID.
 */
typedef unsigned char msg_filter_id_t;


/**
 *	@brief	Represents a filter Type. \n
 *	The filter Type represents a unique filter type. \n
 *	The values for this type SHOULD be in _MSG_FILTER_TYPE_E.
 */
typedef unsigned char msg_filter_type_t;



/*==================================================================================================
                                         ENUMS
==================================================================================================*/

enum _MSG_STRUCT_E {
	MSG_STRUCT_FILTER = 0X0000,								/**< Indicates the MSG_STRUCT_FILTER */

	MSG_STRUCT_MESSAGE_INFO = 0x0200,				/**< Indicates the MSG_STRUCT_MESSAGE_INFO */
	MSG_STRUCT_THREAD_INFO = 0x0300,
	MSG_STRUCT_CONV_INFO = 0x0400,
	MSG_STRUCT_MMS = 0x0500,								/**< Indicates the MSG_STRUCT_MMS */
	MSG_STRUCT_MMS_PAGE = 0x0600,						/**< Indicates the MSG_STRUCT_MMS_PAGE */
	MSG_STRUCT_MMS_MEDIA = 0x0700,					/**< Indicates the MSG_STRUCT_MMS_MEDIA */
	MSG_STRUCT_MMS_ATTACH = 0x0800,					/**< Indicates the MSG_STRUCT_MMS_ATTACH */
	MSG_STRUCT_MMS_REGION = 0x0900,					/**< Indicates the MSG_STRUCT_MMS_REGION */
	MSG_STRUCT_MMS_TRANSITION = 0x0a00,			/**< Indicates the MSG_STRUCT_MMS_TRANSITION */
	MSG_STRUCT_MMS_META = 0x0b00,						/**< Indicates the MSG_STRUCT_MMS_META */
	MSG_STRUCT_MMS_SMIL_TEXT = 0x0c00,			/**< Indicates the MSG_STRUCT_MMS_SMIL_TEXT */
	MSG_STRUCT_MMS_SMIL_AVI = 0x0d00,				/**< Indicates the MSG_STRUCT_MMS_SMIL_AVI */

	MSG_STRUCT_SETTING_SMSC_OPT = 0x2000,
	MSG_STRUCT_SETTING_SMSC_INFO = 0x2100,						/**< Indicates the MSG_STRUCT_SETTING_SMSC_INFO */
	MSG_STRUCT_SETTING_CB_OPT = 0x2200,							/**< Indicates the MSG_STRUCT_SETTING_CB_OPT */
	MSG_STRUCT_SETTING_CB_CHANNEL_INFO = 0x2300,			/**< Indicates the MSG_STRUCT_CB_CHANNEL_INFO */
	MSG_STRUCT_SETTING_SMS_SEND_OPT = 0x2400,				/**< Indicates the MSG_STRUCT_SETTING_SMS_SEND_OPT */
	MSG_STRUCT_SETTING_MMS_SEND_OPT = 0x2500,				/**< Indicates the MSG_STRUCT_SETTING_MMS_SEND_OPT */
	MSG_STRUCT_SETTING_MMS_RECV_OPT = 0x2600,				/**< Indicates the MSG_STRUCT_SETTING_MMS_RECV_OPT */
	MSG_STRUCT_SETTING_PUSH_MSG_OPT = 0x2700,				/**< Indicates the MSG_STRUCT_SETTING_PUSH_MSG_OPT */
	MSG_STRUCT_SETTING_VOICE_MSG_OPT = 0x2800,				/**< Indicates the MSG_STRUCT_SETTING_VOICE_MSG_OPT */
	MSG_STRUCT_SETTING_GENERAL_OPT = 0x2900,				/**< Indicates the MSG_STRUCT_SETTING_GENERAL_OPT */
	MSG_STRUCT_SETTING_MSGSIZE_OPT = 0x2c00,					/**< Indicates the MSG_STRUCT_SETTING_MSGSIZE_OPT */


	MSG_STRUCT_SYNCML_INFO = 0x3100,					/**< Indicates the MSG_STRUCT_SYNCML_INFO */
	MSG_STRUCT_COUNT_INFO = 0x3200,					/**< Indicates the MSG_STRUCT_COUNT_INFO */
	MSG_STRUCT_THREAD_COUNT_INFO = 0x3300,		/**< Indicates the MSG_STRUCT_THREAD_COUNT_INFO */
	MSG_STRUCT_THREAD_LIST_INDEX = 0x3400,		/**< Indicates the MSG_STRUCT_THREAD_LIST_INDEX */
	MSG_STRUCT_SORT_RULE = 0x3500,						/**< Indicates the MSG_STRUCT_SORT_RULE */
	MSG_STRUCT_FOLDER_INFO = 0x3600,					/**< Indicates the MSG_STRUCT_FOLDER_INFO */
	MSG_STRUCT_SEARCH_CONDITION = 0x3700,		/**< Indicates the MSG_STRUCT_SEARCH_CONDITION */
	MSG_STRUCT_REPORT_STATUS_INFO = 0x3800,	/**< Indicates the MSG_STRUCT_REPORT_STATUS_INFO */

	MSG_STRUCT_ADDRESS_INFO = 0x4000,				/**< Indicates the MSG_STRUCT_ADDRESS_INFO */
	MSG_STRUCT_SENDOPT = 0x4100,							/**< Indicates the MSG_STRUCT_SENDOPT */
	MSG_STRUCT_MMS_SENDOPT = 0x4200,				/**< Indicates the MSG_STRUCT_MMS_SENDOPT */
	MSG_STRUCT_SMS_SENDOPT = 0x4300,				/**< Indicates the MSG_STRUCT_SMS_SENDOPT */
	MSG_STRUCT_REJECT_MSG_INFO = 0x4400,			/**< Indicates the MSG_STRUCT_REJECT_MSG_INFO */
	MSG_STRUCT_REQUEST_INFO = 0x4500,				/**< Indicates the MSG_STRUCT_REQUEST_INFO */
	MSG_STRUCT_SENT_STATUS_INFO = 0x4600,				/**< Indicates the MSG_STRUCT_SENT_STATUS_INFO */
	MSG_STRUCT_PUSH_CONFIG_INFO = 0x4700,				/**< Indicates the MSG_STRUCT_PUSH_CONFIG_INFO */
	MSG_STRUCT_CB_MSG = 0x4800,						/**< Indicates the MSG_STRUCT_CB_MSG */
};

enum _MSG_MESSAGE_INFO_E_ {
	MSG_MESSAGE_ID_INT = MSG_STRUCT_MESSAGE_INFO+1,		/**< Indicates the message ID of this message. */
	MSG_MESSAGE_THREAD_ID_INT,				/**< Indicates the thread ID. */
	MSG_MESSAGE_FOLDER_ID_INT,				/**< Indicates the folder ID. see enum _MSG_FOLDER_TYPE_E */
	MSG_MESSAGE_TYPE_INT,				/**< Indicates the message type. See enum _MSG_MESSAGE_TYPE_E */
	MSG_MESSAGE_CLASS_TYPE_INT,				/**< Message class type. See enum _MSG_CLASS_TYPE_E */
	MSG_MESSAGE_STORAGE_ID_INT,				/**< Indicates where the message is saved. see enum _MSG_STORAGE_ID_E*/
	MSG_MESSAGE_ADDR_LIST_STRUCT,  				/**< Indicates the address information list. */
	MSG_MESSAGE_REPLY_ADDR_STR,				/**< Indicates the reply address. */
	MSG_MESSAGE_SUBJECT_STR,				/**< Indicates the message subject. */
	MSG_MESSAGE_DISPLAY_TIME_INT,				/**< Indicates the display time related to the specific operation. */
	MSG_MESSAGE_NETWORK_STATUS_INT,				/**< Indicates the network status of the message. See enum _MSG_NETWORK_STATUS_E */
	MSG_MESSAGE_ENCODE_TYPE_INT,				/**< Indicates the string encoding type. See enum _MSG_ENCODE_TYPE_E */
	MSG_MESSAGE_READ_BOOL,					/**< Indicates whether the message is read or not. */
	MSG_MESSAGE_PROTECTED_BOOL,				/**< Indicates whether the message is protected or not. */
	MSG_MESSAGE_BACKUP_BOOL,				/**< Indicates whether the message was restored from PC. */
	MSG_MESSAGE_PRIORITY_INT,				/**< Indicates the priority of the message. See enum _MSG_PRIORITY_TYPE_E */
	MSG_MESSAGE_DIRECTION_INT,				/**< Indicates whether the message is MO or MT, affecting address. See enum _MSG_DIRECTION_TYPE_E */
	MSG_MESSAGE_PORT_VALID_BOOL,				/**< Indicates whether port information is used or not. */
	MSG_MESSAGE_DEST_PORT_INT,				/**< Recipient port number, not greater than 16 bit */
	MSG_MESSAGE_SRC_PORT_INT,				/**< Sender port number, not greater than 16 bit */
	MSG_MESSAGE_ATTACH_COUNT_INT,				/**< Indicates the count of attached files in mms. */
	MSG_MESSAGE_THUMBNAIL_PATH_STR,				/**< Indicates the thumbnail path of message. */
	MSG_MESSAGE_DATA_SIZE_INT,				/**< Indicates the data size. The unit is byte. */
	MSG_MESSAGE_SMS_DATA_STR,				/**< Indicates the message payload information as a body. default character encoding is UTF-8*/
	MSG_MESSAGE_MMS_TEXT_STR,
};

enum MSG_FILTER_INFO_E {
	MSG_FILTER_ID_INT = MSG_STRUCT_FILTER+1,		/**< Indicates the filter ID. */
	MSG_FILTER_TYPE_INT,					/**< Indicates the filter type. See enum _MSG_FILTER_TYPE_E */
	MSG_FILTER_VALUE_STR,					/**< The value of the filter. */
	MSG_FILTER_ACTIVE_BOOL,					/**< Indicates the activation of the filter. */
};

enum MSG_STRUCT_THREAD_INFO_E {
	MSG_THREAD_ID_INT = MSG_STRUCT_THREAD_INFO+1,		/**< Indicates the message thread ID. */
	MSG_THREAD_NAME_STR,					/**< Indicates the message thread name */
	MSG_THREAD_MSG_TYPE_INT,				/**< Indicates the message type. See enum _MSG_MESSAGE_TYPE_E  */
	MSG_THREAD_MSG_DATA_STR,				/**< The latest message text. */
	MSG_THREAD_MSG_TIME_INT,				/**< The latest message time */
	MSG_THREAD_DIRECTION_INT,				/**< The latest message direction See enum _MSG_DIRECTION_TYPE_E */
	MSG_THREAD_UNREAD_COUNT_INT,				/**< Indicates unread count of thread */
	MSG_THREAD_SMS_COUNT_INT,				/**< Indicates sms message count of thread */
	MSG_THREAD_MMS_COUNT_INT,				/**< Indicates mms message count of thread */
	MSG_THREAD_PROTECTED_BOOL		/**< Indicates whether thread includes protected messages. */
};

enum MSG_STRUCT_CONV_INFO_E {
	MSG_CONV_MSG_ID_INT = MSG_STRUCT_CONV_INFO+1,		/**< Indicates the message ID of this message. */
	MSG_CONV_MSG_THREAD_ID_INT,				/**< Indicates the thread ID. */
	MSG_CONV_MSG_TYPE_INT,													/**< Indicates the message type. See enum _MSG_MESSAGE_TYPE_E */
	MSG_CONV_MSG_FOLDER_ID_INT,				/**< Indicates the folder ID. see enum _MSG_FOLDER_TYPE_E */
	MSG_CONV_MSG_STORAGE_ID_INT,										/**< Indicates where the message is saved. see enum _MSG_STORAGE_ID_E*/
	MSG_CONV_MSG_SUBJECT_STR,											/**< Indicates the message subject. */
	MSG_CONV_MSG_DISPLAY_TIME_INT,									/**< Indicates the display time related to the specific operation. */
	MSG_CONV_MSG_SCHEDULED_TIME_INT,								/**< Indicates the time to send scheduled message. */
	MSG_CONV_MSG_NETWORK_STATUS_INT,							/**< Indicates the network status of the message. See enum _MSG_NETWORK_STATUS_E */
	MSG_CONV_MSG_READ_BOOL,												/**< Indicates whether the message is read or not. */
	MSG_CONV_MSG_PROTECTED_BOOL,									/**< Indicates whether the message is protected or not. */
	MSG_CONV_MSG_DIRECTION_INT,											/**< Indicates whether the message is MO or MT, affecting address. See enum _MSG_DIRECTION_TYPE_E */
	MSG_CONV_MSG_PAGE_COUNT_INT,									/**< Indicates the count of pages in mms. */
	MSG_CONV_MSG_ATTACH_COUNT_INT,									/**< Indicates the count of attached files in mms. */
	MSG_CONV_MSG_ATTACH_NAME_STR,								/**< Indicates the attached file name of message. */
	MSG_CONV_MSG_AUDIO_NAME_STR,									/**< Indicates the audio file name of message. */
	MSG_CONV_MSG_IMAGE_THUMB_PATH_STR,						/**< Indicates the image thumbnail path of message. */
	MSG_CONV_MSG_VIDEO_THUMB_PATH_STR,						/**< Indicates the video thumbnail path of message. */
	MSG_CONV_MSG_TEXT_SIZE_INT,											/**< Indicates the data size. The unit is byte. */
	MSG_CONV_MSG_TEXT_STR													/**< Indicates the message payload information as a body. default character encoding is UTF-8*/
};

enum _MSG_STRUCT_SETTING_SMSC_OPT_E {
	MSG_SMSC_SELECTED_ID_INT = MSG_STRUCT_SETTING_SMSC_OPT+1,
	MSG_SMSC_LIST_STRUCT
};

enum _MSG_STRUCT_SETTING_SMSC_INFO_E {
	MSG_SMSC_ADDR_TON_INT = MSG_STRUCT_SETTING_SMSC_INFO+1,		/**< Type of number. See enum _MSG_SMS_TON_E */
	MSG_SMSC_ADDR_NPI_INT,					/**< Numbering plan ID. See enum _MSG_SMS_NPI_E */
	MSG_SMSC_ADDR_STR,					/**< SMSC address */
	MSG_SMSC_NAME_STR,					/**< SMSC name */
	MSG_SMSC_PID_INT,					/**< Protocol idendifier See enum _MSG_SMS_PID_E */
	MSG_SMSC_VAL_PERIOD_INT					/**< Validity period value. See enum _MSG_VAL_PERIOD_E*/
};

enum _MSG_STRUCT_CB_OPT_E {
	MSG_CB_RECEIVE_BOOL = MSG_STRUCT_SETTING_CB_OPT+1,		/**< Indicates whether the CB message is received or not. */
	MSG_CB_MAX_SIM_COUNT_INT,				/**< Indicates the number of channels which can be stored in SIM. */
	MSG_CB_CHANNEL_LIST_STRUCT,				/**< Indicates the cell broadcasting channel information. */
	MSG_CB_LANGUAGE_TYPE_ALL_BOOL,
	MSG_CB_LANGUAGE_TYPE_ENG_BOOL,			/**< CB message English  */
	MSG_CB_LANGUAGE_TYPE_GER_BOOL,			/**< CB message Germany */
	MSG_CB_LANGUAGE_TYPE_FRE_BOOL,			/**< CB message France */
	MSG_CB_LANGUAGE_TYPE_ITA_BOOL,			/**< CB message Italy */
	MSG_CB_LANGUAGE_TYPE_NED_BOOL,			/**< CB message Netherland */
	MSG_CB_LANGUAGE_TYPE_SPA_BOOL,			/**< CB message Spain */
	MSG_CB_LANGUAGE_TYPE_POR_BOOL,			/**< CB message Portugal */
	MSG_CB_LANGUAGE_TYPE_SWE_BOOL,			/**< CB message Sweden */
	MSG_CB_LANGUAGE_TYPE_TUR_BOOL,			/**< CB message Turkey */
};

enum _MSG_STRUCT_CB_CHANNEL_INFO_E {
	MSG_CB_CHANNEL_ACTIVATE_BOOL = MSG_STRUCT_SETTING_CB_CHANNEL_INFO+1,	/**< Indicates whether the CB channel is activate or passive. */
	MSG_CB_CHANNEL_ID_FROM_INT,						/**< Indicates the ID of a CB channel. */
	MSG_CB_CHANNEL_ID_TO_INT,						/**< Indicates the ID of a CB channel. */
	MSG_CB_CHANNEL_NAME_STR						/**< Indicates the name of a CB channel. */
};

enum _MSG_STRUCT_SMS_SEND_OPT_E {
	MSG_SMS_SENDOPT_ENCODE_TYPE_INT = MSG_STRUCT_SETTING_SMS_SEND_OPT+1,	/**< Indicates the string encoding type.  See enum _MSG_ENCODE_TYPE_E*/
	MSG_SMS_SENDOPT_NETWORK_MODE_INT,				/**< Indicates the network mode (CS/PS) to send SMS. See enum _MSG_SMS_NETWORK_MODE_E*/
	MSG_SMS_SENDOPT_REPLY_PATH_BOOL,				/**< Indicates whether the SMS reply path is set or not. */
	MSG_SMS_SENDOPT_DELIVERY_REPORT_BOOL,				/**< Indicates whether the SMS delivery report will be sent or not. */
	MSG_SMS_SENDOPT_SAVE_STORAGE_INT				/**< Indicates the default storage to save SMS. See enum _MSG_SMS_SAVE_STORAGE_E */
};

enum _MSG_STRUCT_MMS_SEND_OPT_E {
	MSG_MMS_SENDOPT_CLASS_TYPE_INT = MSG_STRUCT_SETTING_MMS_SEND_OPT+1,		/**< Indicates the class type of message.  See enum _MSG_MMS_MSG_CLASS_TYPE_E */
	MSG_MMS_SENDOPT_PRIORITY_TYPE_INT,					/**< Indicates the priority of the message. See enum _MSG_PRIORITY_TYPE_E */
	MSG_MMS_SENDOPT_EXPIRY_TIME_INT,					/**< Indicates the time when the message is to be removed from the MMSC. See enum _MSG_MMS_EXPIRY_TIME_E */
	MSG_MMS_SENDOPT_DELIVERY_TIME_INT,					/**< Indicates the message transmission time which is set in the MMSC. See enum _MSG_MMS_DELIVERY_TIME_E */
	MSG_MMS_SENDOPT_CUSTOM_DELIVERY_TIME_INT,				/**< Indicates the message transmission time which is set in the MMSC. */
	MSG_MMS_SENDOPT_SENDER_VISIBILITY_BOOL,					/**< Indicates whether the address is hidden or not. */
	MSG_MMS_SENDOPT_DELIVERY_REPORT_BOOL,					/**< Indicates whether the delivery report will be sent or not. */
	MSG_MMS_SENDOPT_READ_REPLY_BOOL,					/**< Indicates whether the read report will be sent or not. */
	MSG_MMS_SENDOPT_KEEP_COPY_BOOL,						/**< Indicates whether the message copy is kept or not. */
	MSG_MMS_SENDOPT_BODY_REPLYING_BOOL,					/**< Indicates whether the body is included when replying or not. */
	MSG_MMS_SENDOPT_HIDE_RECIPIENTS_BOOL,					/**< Indicates whether the recipients are hidden or not. */
	MSG_MMS_SENDOPT_REPLY_CHARGING_INT,					/**< Indicates the reply charging type of message. See enum _MSG_MMS_REPLY_CHARGING_TYPE_E */
	MSG_MMS_SENDOPT_REPLY_CHARGING_DEADLINE_INT,				/**< Indicates the deadline for replying charging. is set in the MMSC. */
	MSG_MMS_SENDOPT_REPLY_CHARGING_SIZE_INT,				/**< Indicates the reply charging size. */
	MSG_MMS_SENDOPT_CREATION_MODE_INT					/**< Indicates the creation mode of MMS. See enum _MSG_MMS_CREATION_MODE_E */
};

enum _MSG_STRUCT_MMS_RECV_OPT_E {
	MSG_MMS_RECVOPT_HOME_RETRIEVE_TYPE_INT = MSG_STRUCT_SETTING_MMS_RECV_OPT+1,	/**< Indicates the retrieve type for MMS home network. See enum _MSG_MMS_HOME_RETRIEVE_TYPE_E */
	MSG_MMS_RECVOPT_ABROAD_RETRIEVE_TYPE_INT,				/**< Indicates the retrieve type for MMS abroad network. See enum _MSG_MMS_ABROAD_RETRIEVE_TYPE_E */
	MSG_MMS_RECVOPT_READ_REPORT_BOOL,					/**< Indicates whether the read report will be sent or not. */
	MSG_MMS_RECVOPT_DELIVERY_REPORT_BOOL,					/**< Indicates whether the delivery report will be sent or not. */
	MSG_MMS_RECVOPT_REJECT_UNKNOWN_BOOL,					/**< Indicates whether unknown addresses are rejected or not. */
	MSG_MMS_RECVOPT_REJECT_ADVERTISEMENT_BOOL				/**< Indicates whether advertisement is rejected or not. */
};

enum _MSG_STRUCT_GENERAL_OPT_E {
	MSG_GENERAL_KEEP_COPY_BOOL = MSG_STRUCT_SETTING_GENERAL_OPT+1,			/**< Indicates whether the SMS message copy is kept or not. */
	MSG_GENERAL_ALERT_TONE_INT,						/**< Indicates the period of playing alert tone. See enum _MSG_ALERT_TONE_E */
	MSG_GENERAL_AUTO_ERASE_BOOL						/**< Indicates whether the auto-erase option is enabled or not. */
};

enum _MSG_STRUCT_PUSH_MSG_OPT_E {
	MSG_PUSHMSG_RECEIVE_BOOL = MSG_STRUCT_SETTING_PUSH_MSG_OPT+1,		/**< Indicates whether the push message is received or not. */
	MSG_PUSHMSG_SERVICE_TYPE_INT					/**< Indicates the service type of a push message. See enum _MSG_PUSH_SERVICE_TYPE_E */
};

enum _MSG_STRUCT_VOICE_MSG_OPT_E {
	MSG_VOICEMSG_ADDRESS_STR = MSG_STRUCT_SETTING_VOICE_MSG_OPT+1,		/**< Indicates the address for voice message. */
};

enum _MSG_STRUCT_MSGSISE_OPT_E {
	MSG_MESSAGE_SIZE_INT = MSG_STRUCT_SETTING_MSGSIZE_OPT+1,		/**< Indicates the message size */
};

//MSG_MMS_DATA
enum MSG_MMS_INFO_E {
	MSG_MMS_PAGE_LIST_HND = MSG_STRUCT_MMS+1,			/**< The pointer to SMIL pages list */
	MSG_MMS_REGION_LIST_HND,					/**< The pointer to SMIL regions list */
	MSG_MMS_ATTACH_LIST_HND,					/**< The pointer to attachment list */
	MSG_MMS_TRANSITION_LIST_HND,					/**< The pointer to SMIL transitions list */
	MSG_MMS_META_LIST_HND,						/**< The pointer to SMIL meta list */
	MSG_MMS_ROOTLAYOUT_WIDTH_INT,					/**< Indicates the width of the root layout */
	MSG_MMS_ROOTLAYOUT_WIDTH_PERCENT_BOOL,				/**< Indicates the length is in percentage(%) or not */
	MSG_MMS_ROOTLAYOUT_HEIGHT_INT,					/**< Indicates the height of the root layout */
	MSG_MMS_ROOTLAYOUT_HEIGHT_PERCENT_BOOL,				/**< Indicates the length is in percentage(%) or not */
	MSG_MMS_ROOTLAYOUT_BGCOLOR_INT,					/**< Indicates the background color of the root layout */
	MSG_MMS_ROOTLAYOUT_BGCOLOR_BOOL,				/**< Indicates the background color is set in the root layout */
};

//MSG_MMS_PAGE
enum MSG_MMS_PAGE_INFO_E {
	MSG_MMS_PAGE_MEDIA_LIST_HND = MSG_STRUCT_MMS_PAGE+1,		/**< The pointer to media list */
	MSG_MMS_PAGE_PAGE_DURATION_INT,					/**< Indicates the duration of the page */
	MSG_MMS_PAGE_BEGIN_TIME_INT,					/**< Indicates the begin time of the page */
	MSG_MMS_PAGE_END_TIME_INT,					/**< Indicates the end time of the page */
	MSG_MMS_PAGE_MIN_INT,						/**< Indicates the min attribute of the page */
	MSG_MMS_PAGE_MAX_INT,						/**< Indicates the max attribute of the page */
	MSG_MMS_PAGE_REPEAT_INT,					/**< Indicates the page needs to be displayed repeatedly */
};

enum MSG_MMS_MEDIA_INFO {
	MSG_MMS_MEDIA_TYPE_INT = MSG_STRUCT_MMS_MEDIA+1,		/**< Indicates the SMIL media type. see enum MmsSmilMediaType */
	MSG_MMS_MEDIA_SRC_STR,						/**< Indicates the media source name */
	MSG_MMS_MEDIA_FILENAME_STR,					/**< Indicates the file name */
	MSG_MMS_MEDIA_FILEPATH_STR,					/**< Indicates the file path */
	MSG_MMS_MEDIA_CONTENT_ID_STR,					/**< Indicates the content id */
	MSG_MMS_MEDIA_REGION_ID_STR,					/**< Indicates the region id */
	MSG_MMS_MEDIA_ALTERNATIVE_STR,					/**< Indicates the alternative text to be displayed in failure case */
	MSG_MMS_MEDIA_DRM_TYPE_INT,					/**< Indicates the drm type. see enum MsgDrmType */
	MSG_MMS_MEDIA_DRM_FULLPATH_STR,					/**< Indicates the fullpath of the DRM */
	MSG_MMS_MEDIA_SMIL_TEXT_HND,					/**< Indicates the text attributes */
	MSG_MMS_MEDIA_SMIL_AVI_HND,					/**< Indicates the video attributes */
};

//MSG_MMS_ATTACH
enum MSG_MMS_ATTACH_INFO_E {
	MSG_MMS_ATTACH_MIME_TYPE_INT = MSG_STRUCT_MMS_ATTACH+1,		/**< Indicates the file mime type. see enum MimeType */
	MSG_MMS_ATTACH_FILENAME_STR,					/**< Indicates the file name */
	MSG_MMS_ATTACH_FILEPATH_STR,					/**< Indicates the file path */
	MSG_MMS_ATTACH_FILESIZE_INT,					/**< Indicates the size of the file */
	MSG_MMS_ATTACH_DRM_TYPE_INT,					/**< Indicates the drm type. see enum MsgDrmType */
	MSG_MMS_ATTACH_DRM_FULLPATH_STR,				/**< Indicates the fullpath of the DRM */
};

enum MSG_MMS_REGION_INFO_E {
	MSG_MMS_REGION_ID_STR = MSG_STRUCT_MMS_REGION+1,		/**< Indicates the ID of region information */
	MSG_MMS_REGION_LENGTH_LEFT_INT,					/**< Indicates the left co-ordinate of the region */
	MSG_MMS_REGION_LENGTH_LEFT_PERCENT_BOOL,			/**< Indicates the length is in percentage(%) or not */
	MSG_MMS_REGION_LENGTH_TOP_INT,					/**< Indicates the top co-ordinate of the region */
	MSG_MMS_REGION_LENGTH_TOP_PERCENT_BOOL,				/**< Indicates the length is in percentage(%) or not */
	MSG_MMS_REGION_LENGTH_WIDTH_INT,				/**< Indicates the width of the region */
	MSG_MMS_REGION_LENGTH_WIDTH_PERCENT_BOOL,			/**< Indicates the length is in percentage(%) or not */
	MSG_MMS_REGION_LENGTH_HEIGHT_INT,				/**< Indicates the width of the region */
	MSG_MMS_REGION_LENGTH_HEIGHT_PERCENT_BOOL,			/**< Indicates the length is in percentage(%) or not */
	MSG_MMS_REGION_BGCOLOR_INT,					/**< Indicates the background color of the region */
	MSG_MMS_REGION_FIT_TYPE_INT,					/**< Indicates the fit type. see enum REGION_FIT_TYPE_T */
	MSG_MMS_REGION_BGCOLOR_BOOL,				/**< Indicates the background color is set in the region */
};

enum MSG_MMS_META_INFO_E {
	MSG_MMS_META_ID_STR = MSG_STRUCT_MMS_META+1,			/**< Indicates the ID of meta information */
	MSG_MMS_META_NAME_STR,						/**< Indicates the Name */
	MSG_MMS_META_CONTENT_STR,					/**< Indicates the content */
};

enum MSG_MMS_TRANSION_INFO_E {
	MSG_MMS_TRANSITION_ID_STR = MSG_STRUCT_MMS_TRANSITION+1,	/**< Indicates the ID of transition information */
	MSG_MMS_TRANSITION_TYPE_INT,					/**< Indicates the transition type. see enum MmsSmilTransType */
	MSG_MMS_TRANSITION_SUBTYPE_INT,					/**< Indicates the transition sub type. see enum MmsSmilTransSubType */
	MSG_MMS_TRANSITION_DURATION_INT,				/**< Indicates the transition duration */
};

enum MSG_MMS_SMIL_TEXT_INFO_E {
	MSG_MMS_SMIL_TEXT_TRANSITION_IN_ID_STR = MSG_STRUCT_MMS_SMIL_TEXT+1,	/**< Indicates the In SMIL transition id */
	MSG_MMS_SMIL_TEXT_TRANSITION_OUT_ID_STR,				/**< Indicates the Out SMIL transition id */
	MSG_MMS_SMIL_TEXT_REPEAT_INT,						/**< Indicates the text needs to be displayed repeatedly */
	MSG_MMS_SMIL_TEXT_BEGIN_INT,						/**< Indicates the begin time */
	MSG_MMS_SMIL_TEXT_END_INT,						/**< Indicates the end time */
	MSG_MMS_SMIL_TEXT_DURTIME_INT,						/**< Indicates the duration */
	MSG_MMS_SMIL_TEXT_BGCOLOR_INT,						/**< Indicates the background color of the text */
	MSG_MMS_SMIL_TEXT_BOLD_BOOL,						/**< Indicates whether the text is bold */
	MSG_MMS_SMIL_TEXT_UNDERLINE_BOOL,					/**< Indicates whether the text is underlined */
	MSG_MMS_SMIL_TEXT_ITALIC_BOOL,						/**< Indicates whether the text is Italic */
	MSG_MMS_SMIL_TEXT_REVERSE_BOOL,						/**< Indicates whether the text is reversed */
	MSG_MMS_SMIL_TEXT_DIRECTION_TYPE_INT,					/**< Indicates the text direction type. see enum MmsTextDirection */
	MSG_MMS_SMIL_TEXT_SIZE_INT,						/**< Indicates the font size */
	MSG_MMS_SMIL_TEXT_COLOR_INT,						/**< Indicates the font color */
};

enum MSG_MMS_SMIL_AVI_INFO_E {
	MSG_MMS_SMIL_AVI_TRANSITION_IN_ID_STR = MSG_STRUCT_MMS_SMIL_AVI+1,	/**< Indicates the In SMIL transition id */
	MSG_MMS_SMIL_AVI_TRANSITION_OUT_ID_STR,					/**< Indicates the Out SMIL transition id */
	MSG_MMS_SMIL_AVI_REPEAT_INT,						/**< Indicates the video needs to be displayed repeatedly */
	MSG_MMS_SMIL_AVI_BEGIN_INT,						/**< Indicates the begin time */
	MSG_MMS_SMIL_AVI_END_INT,						/**< Indicates the end time */
	MSG_MMS_SMIL_AVI_DURTIME_INT,						/**< Indicates the duration */
	MSG_MMS_SMIL_AVI_BGCOLOR_INT,						/**< Indicates the background color of the text */
};

enum MSG_SEND_OPT_E {
	MSG_SEND_OPT_SETTING_BOOL = MSG_STRUCT_SENDOPT+1,			/**< Indicates whether the sending option is set ot not for a message */
	MSG_SEND_OPT_DELIVER_REQ_BOOL,						/**< Indicates whether the delivey custom time is used or not */
	MSG_SEND_OPT_KEEPCOPY_BOOL,						/**< Indicates whether the message copy is kept or not */
	MSG_SEND_OPT_MMS_OPT_HND,						/**< The handle of MMS sending option for a message */
	MSG_SEND_OPT_SMS_OPT_HND,						/**< The handle of SMS sending option for a message */
};

enum MSG_SYNCML_INFO_E {
	MSG_SYNCML_INFO_EXTID_INT = MSG_STRUCT_SYNCML_INFO+1,			/**< Indicates the ext ID */
	MSG_SYNCML_INFO_PINCODE_INT,						/**< Indicates the PIN code */
	MSG_SYNCML_INFO_MESSAGE_HND,						/**< Indicate the handle of a message */
};

enum MSG_COUNT_INFO_E {
	MSG_COUNT_INFO_READ_INT = MSG_STRUCT_COUNT_INFO+1,			/**< The count of read messages */
	MSG_COUNT_INFO_UNREAD_INT,						/**< The count of unread messages */
	MSG_COUNT_INFO_SMS_INT,							/**< The count of sms type messages */
	MSG_COUNT_INFO_MMS_INT,							/**< The count of mms type messages */
};

enum MSG_THREAD_COUNT_INFO_E {
	MSG_THREAD_COUNT_TOTAL_INT = MSG_STRUCT_THREAD_COUNT_INFO+1,		/**< Indicates the total number of messages from the Peer. */
	MSG_THREAD_COUNT_UNREAD_INT,						/**< Indicates the unread messages from the Peer. */
	MSG_THREAD_COUNT_SMS_INT,						/**< Indicates the SMS messages from the Peer. */
	MSG_THREAD_COUNT_MMS_INT,						/**< Indicates the MMS messages from the Peer. */
};

enum MSG_THREAD_LIST_INDEX_E {
	MSG_THREAD_LIST_INDEX_CONTACTID_INT = MSG_STRUCT_THREAD_LIST_INDEX+1,	/**< The contact id of message common informatioin */
	MSG_THREAD_LIST_INDEX_ADDR_INFO_HND,					/**< The pointer to message common informatioin */
};

enum MSG_SORT_RULE_E {
	MSG_SORT_RULE_SORT_TYPE_INT = MSG_STRUCT_SORT_RULE+1,		/**< Indicates the sort type. See enum _MSG_SORT_TYPE_E */
	MSG_SORT_RULE_ACSCEND_BOOL,					/**< Indicates the sort order which is ascending or descending */
};

enum MSG_FOLDER_INFO_E {
	MSG_FOLDER_INFO_ID_INT = MSG_STRUCT_FOLDER_INFO+1,		/**< Indicates the unique folder ID. */
	MSG_FOLDER_INFO_NAME_STR,					/**< Indicates the name of the folder. */
	MSG_FOLDER_INFO_TYPE_INT,					/**< Indicates the folder type. */
};

enum MSG_SEARCH_CONDITION_E {
	MSG_SEARCH_CONDITION_FOLDERID_INT = MSG_STRUCT_SEARCH_CONDITION+1,		/**< Indicates the folder ID for searching messages. See enum _MSG_FOLDER_ID_E */
	MSG_SEARCH_CONDITION_MSGTYPE_INT,						/**< Indicates the message type for searching messages. See enum _MSG_MESSAGE_TYPE_E */
	MSG_SEARCH_CONDITION_ADDRESS_VALUE_STR,						/**< The address value for searching messages. */
	MSG_SEARCH_CONDITION_SEARCH_VALUE_STR,						/**< The string to search */
	MSG_SEARCH_CONDITION_RESERVED_INT,
};

enum MSG_REPORT_E {
	MSG_REPORT_ADDRESS_STR = MSG_STRUCT_REPORT_STATUS_INFO+1,	/**< Indicates Report address */
	MSG_REPORT_TYPE_INT,							/**< Indicates Report type. See the msg_report_type_t type*/
	MSG_REPORT_STATUS_INT,						/**< Indicates Report status. See the msg_delivery_report_status_t or msg_read_report_status_t type*/
	MSG_REPORT_TIME_INT,							/**< Indicates Report time */
};

enum MSG_ADDRESS_INFO_E {
	MSG_ADDRESS_INFO_ADDRESS_TYPE_INT = MSG_STRUCT_ADDRESS_INFO+1,			/**< The type of an address in case of an Email or a mobile phone. See enum _MSG_ADDRESS_TYPE_E */
	MSG_ADDRESS_INFO_RECIPIENT_TYPE_INT,						/**< The type of recipient address in case of To, Cc, and Bcc. See enum _MSG_RECIPIENT_TYPE_E */
	MSG_ADDRESS_INFO_CONTACT_ID_INT,						/**< The contact ID of address */
	MSG_ADDRESS_INFO_ADDRESS_VALUE_STR,						/**< The actual value of an address */
	MSG_ADDRESS_INFO_DISPLAYNAME_STR,						/**< The display name of an address */
};

enum MSG_MMS_SENDOPT_E {
	MSG_MMS_SENDOPTION_READ_REQUEST_BOOL = MSG_STRUCT_MMS_SENDOPT+1,		/**< Indicates whether it requests read report or not */
	MSG_MMS_SENDOPTION_EXPIRY_TIME_INT,						/**< Indicates MMS expiry time */
	MSG_MMS_SENDOPTION_DELIVERY_CUSTOMTIME_BOOL,					/**< Indicates whether is use MMS delivery custom time */
	MSG_MMS_SENDOPTION_DELIVERY_TIME_INT,						/**< Indicates MMS delivery time */
	MSG_MMS_SENDOPTION_PRIORITY_INT,						/**< Indicates MMS priority. See enum _MSG_PRIORITY_TYPE_E */
};

enum MSG_SMS_SENDOPT_E {
	MSG_SMS_SENDOPT_REPLYPATH_BOOL = MSG_STRUCT_SMS_SENDOPT+1,		/**< Indicates whether reply path is set */
};

enum MSG_REJECT_MESSAGE_E {
	MSG_REJECT_MESSAGE_MSGID_INT = MSG_STRUCT_REJECT_MSG_INFO+1,		/**< Indicates the font type none */
	MSG_REJECT_MESSAGE_MSGTEXT_STR,						/**< Indicates the font type none */
	MSG_REJECT_MESSAGE_DISPLAY_TIME_INT,					/**< Indicates the font type none */
};

enum MSG_REQUEST_INFO_E {
	MSG_REQUEST_REQUESTID_INT = MSG_STRUCT_REQUEST_INFO+1,	/**< Indicates the request ID, which is unique. When applications submit a request to the framework, this value will be set by the framework. */
	MSG_REQUEST_MESSAGE_HND,				/**< Indicates the message structure to be sent by applications. */
	MSG_REQUEST_SENDOPT_HND,
};

enum MSG_SENT_STATUS_INFO_E {
	MSG_SENT_STATUS_REQUESTID_INT = MSG_STRUCT_SENT_STATUS_INFO+1,	/**< Indicates the corresponding request Id. */
	MSG_SENT_STATUS_NETWORK_STATUS_INT,				/**< Indicates the status of the corresponding request. Refer to enum _MSG_NETWORK_STATUS_E*/
};

enum MSG_PUSH_CONFIG_INFO_E {
	MSG_PUSH_CONFIG_CONTENT_TYPE_STR = MSG_STRUCT_PUSH_CONFIG_INFO+1,
	MSG_PUSH_CONFIG_APPLICATON_ID_STR,
	MSG_PUSH_CONFIG_PACKAGE_NAME_STR,
	MSG_PUSH_CONFIG_LAUNCH_BOOL,
};

enum MSG_CB_MSG_E {
	MSG_CB_MSG_TYPE_INT	= MSG_STRUCT_CB_MSG+1,		/**<  MSG_TYPE_SMS_CB/ETWS_PRIMARY/ETWS_SECONDARY (see _MSG_MESSAGE_TYPE_E) */
	MSG_CB_MSG_RECV_TIME_INT,
	MSG_CB_MSG_SERIAL_NUM_INT,						/**< serial number of CB/ETWS Primary Noti. : 2 bytes binary data */
	MSG_CB_MSG_MSG_ID_INT,							/**< message identifier of CB/ETWS Primary Noti. */
	MSG_CB_MSG_DCS_INT,								/**< Data coding scheme of CB MSG. */
	MSG_CB_MSG_CB_TEXT_LEN_INT,						/**< length of CB text (except NULL) */
	MSG_CB_MSG_CB_TEXT_STR,							/**< CB text */
	MSG_CB_MSG_ETWS_WARNING_TYPE_INT,				/**< warning type of ETWS Primary Noti. : 2 bytes binary data */
	MSG_CB_MSG_ETWS_WARNING_SECU_INFO_STR,			/**< warning security information of ETWS Primary Noti. : 50 bytes binary data */
	MSG_CB_MSG_LANGUAGE_TYPE_STR,					/**< Language type of CB message data */
};

/**
 *	@brief	Represents the values of a message class type. \n
 *	This enum is used as the value of MSG_CLASS_TYPE_T.
 */
enum _MSG_CLASS_TYPE_E
{
	MSG_CLASS_0 = 0,		/**< Immediately presented on the recipient device display */
	MSG_CLASS_1,			/**< Stored in the mobile equipment or SIM (depending on memory availability) */
	MSG_CLASS_2,			/**< Stored in SIM */
	MSG_CLASS_3,			/**< Transferred to the terminal equipment (such as PDA or PC) which is connected to the mobile equipment */
	MSG_CLASS_NONE,
};

/**
 *	@brief	Represents the type of Message. More members maybe added if needed \n
 *	This enum is used as the value of msg_message_type_t.
 */
enum _MSG_MESSAGE_TYPE_E
{
 	MSG_TYPE_INVALID = 0,			/** < Invalid Type Message */

 	MSG_TYPE_SMS,					/** < Normal SMS Message */
	MSG_TYPE_SMS_CB,				/** < Cell Broadcasting SMS Message */
	MSG_TYPE_SMS_JAVACB,			/** < JAVA Cell Broadcasting SMS Message */
	MSG_TYPE_SMS_WAPPUSH,		/** < WAP Push SMS Message */
	MSG_TYPE_SMS_MWI,				/** < MWI SMS Message */
	MSG_TYPE_SMS_SYNCML,			/** < SyncML CP SMS Message */
	MSG_TYPE_SMS_REJECT,			/** < Reject Message */

	MSG_TYPE_MMS, 					/** < Normal MMS Message */
	MSG_TYPE_MMS_JAVA, 			/** < JAVA MMS Message */
	MSG_TYPE_MMS_NOTI, 			/** < MMS Notification Message */

	MSG_TYPE_SMS_ETWS_PRIMARY,		/** < CB - ETWS Primary Notification */
	MSG_TYPE_SMS_ETWS_SECONDARY,	/** < CB - ETWS Secondary Notification */
};

/**
 *	@brief	Represents the backup type of Message. More members maybe added if needed \n
 *	This enum is used as the value of msg_message_backup_type_t.
 */
enum _MSG_MESSAGE_BACKUP_TYPE_E
{
	MSG_BACKUP_TYPE_ALL = 0,
	MSG_BACKUP_TYPE_SMS,
	MSG_BACKUP_TYPE_MMS,
};

/**
 *	@brief	Represents the values of an error code. \n
 *	Success code is zero, but all error codes SHOULD be negative and smaller than MSG_ERROR_BEGIN. \n
 *	This enum is used as the value of msg_error_t.
 */
enum _MSG_ERROR_E
{
	MSG_SUCCESS = 0,  				/**< Successful */

	MSG_ERR_NULL_MSGHANDLE = -1,	/**< Message handle is NULL */
	MSG_ERR_NULL_POINTER = -2,		/**< Pointer is NULL */
	MSG_ERR_NULL_MESSAGE = -3,		/**< Message is NULL */
	MSG_ERR_INVALID_STORAGE_ID = -4,	/**< Storage ID is invalid */
	MSG_ERR_INVALID_MSG_TYPE = -5,		/**< Message type is invalid */

	MSG_ERR_INVALID_STORAGE_REG= -6,	/**< Storage registry is invalid */
	MSG_ERR_INVALID_MESSAGE_ID = -7,	/**< Message ID is invalid */
	MSG_ERR_INVALID_MSGHANDLE = -8,	/**< Message handle is invalid */
	MSG_ERR_INVALID_PARAMETER = -9,	/**< Parameter is invalid */
	MSG_ERR_INVALID_MESSAGE = -10,			/**< Message is invalid */

	MSG_ERR_INVALID_PLUGIN_HANDLE = -11,	/**< Plugin handle is invalid */
	MSG_ERR_MEMORY_ERROR = -12,  			/**< Memory is error */
	MSG_ERR_COMMUNICATION_ERROR = -13,  	/**< Communication between client and server is error */
	MSG_ERR_SIM_STORAGE_FULL = -14,		/**< SIM Storage is full */
	MSG_ERR_TRANSPORT_ERROR = -15,			/**< Transport event error */

	MSG_ERR_CALLBACK_ERROR = -16,			/**< Callback event error */
	MSG_ERR_STORAGE_ERROR = -17,			/**< Storage event error */
	MSG_ERR_FILTER_ERROR = -18,				/**< Filter event error */
	MSG_ERR_MMS_ERROR = -19,				/**< MMS event error */
	MSG_ERR_MMPLAYER_CREATE = -20,			/**< Multimedia Error*/

	MSG_ERR_MMPLAYER_SET_ATTRS = -21,		/**< Multimedia Error*/
	MSG_ERR_MMPLAYER_REALIZE = -22,		/**< Multimedia Error*/
	MSG_ERR_MMPLAYER_PLAY = -23,			/**< Multimedia Error*/
	MSG_ERR_MMPLAYER_STOP = -24,			/**< Multimedia Error*/
	MSG_ERR_MMPLAYER_DESTROY = -25,		/**< Multimedia Error*/

	MSG_ERR_UNKNOWN = -26,		      			/**< Unknown errors */

	/* Start Database Errors */
	MSG_ERR_DB_CONNECT = -27,
	MSG_ERR_DB_DISCONNECT = -28,
	MSG_ERR_DB_EXEC = -29,
	MSG_ERR_DB_GETTABLE = -30,

	MSG_ERR_DB_PREPARE = -31,
	MSG_ERR_DB_STEP = -32,
	MSG_ERR_DB_NORECORD= -33,
	MSG_ERR_DB_STORAGE_INIT = -34,
	MSG_ERR_DB_MAKE_DIR = -35,

	MSG_ERR_DB_ROW = -36,
	MSG_ERR_DB_DONE = -37,
	MSG_ERR_DB_GENERIC= -38,
	MSG_ERR_DB_END = -39,
	/* End Database Errors */

	/* Start Setting Errors */
	MSG_ERR_SET_SETTING = -40,
	MSG_ERR_SET_SIM_SET = -41,
	MSG_ERR_SET_READ_ERROR = -42,
	MSG_ERR_SET_WRITE_ERROR = -43,
	MSG_ERR_SET_DELETE_ERROR = -44,
	/* End Setting Errors */

	/* Start Plugin Errors */
	MSG_ERR_PLUGIN_TAPIINIT = -45,
	MSG_ERR_PLUGIN_REGEVENT = -46,
	MSG_ERR_PLUGIN_TRANSPORT = -47,
	MSG_ERR_PLUGIN_STORAGE = -48,
	MSG_ERR_PLUGIN_SETTING = -49,

	MSG_ERR_PLUGIN_WAPDECODE = -50,
	MSG_ERR_PLUGIN_TAPI_FAILED = -51,
	MSG_ERR_PLUGIN_SIM_MSG_FULL = -52,
	/* End Plugin Errors */

	MSG_ERR_MESSAGE_COUNT_FULL = -53,
	MSG_ERR_READREPORT_NOT_REQUESTED = -54,
	MSG_ERR_READREPORT_ALEADY_SENT = -55,

	MSG_ERR_FILTER_DUPLICATED = -56,				/**< Filter duplicate error */
	MSG_ERR_SECURITY_ERROR = -57,
	MSG_ERR_NO_SIM = -58,
	MSG_ERR_SERVER_NOT_READY= -59,
};


/**
 *	@brief	Represents the values of a message priority. \n
 *	This enum is used as the value of msg_priority_type_t.
 */
enum _MSG_PRIORITY_TYPE_E
{
	MSG_MESSAGE_PRIORITY_LOW,		/**< Low priority */
	MSG_MESSAGE_PRIORITY_NORMAL,	/**< Normal priority */
	MSG_MESSAGE_PRIORITY_HIGH,		/**< High priority */
};


/**
 *	@brief	Represents the values of a network status. \n
 *	This enum is used as the value of msg_network_status_t.
 */
enum _MSG_NETWORK_STATUS_E
{
	MSG_NETWORK_NOT_SEND = 0,		/**< Message is not sending */
	MSG_NETWORK_SENDING,				/**< Message is sending */
	MSG_NETWORK_SEND_SUCCESS,		/**< Message is sent successfully */
	MSG_NETWORK_SEND_FAIL,			/**< Message is failed to send */
	MSG_NETWORK_DELIVER_SUCCESS,	/**< Message is delivered */
	MSG_NETWORK_DELIVER_FAIL,		/**< Message is failed to deliver */
	MSG_NETWORK_RECEIVED,			/**< Message is received */
	MSG_NETWORK_REQ_CANCELLED,		/**< Request is cancelled */
	MSG_NETWORK_RETRIEVING,				/**< Message is retrieving */
	MSG_NETWORK_RETRIEVE_SUCCESS,		/**< Message is retrieved successfully */
	MSG_NETWORK_RETRIEVE_FAIL,			/**< Message is failed to retrieve */
	MSG_NETWORK_SEND_TIMEOUT,			/**< Message is failed to send by timeout */
};


/**
 *	@brief	Represents the values of an address type. \n
 *	This enum is used as the value of msg_address_type_t.
*/
enum _MSG_ADDRESS_TYPE_E
{
	MSG_ADDRESS_TYPE_UNKNOWN = 0,	/**< The address type is unknown. */
	MSG_ADDRESS_TYPE_PLMN,			/**< The address type is for a phone number like +1012345678. */
	MSG_ADDRESS_TYPE_EMAIL,			/**< The address type is for an email address like abc@example.email. */
};


/**
 *	@brief	Represents the values of a recipient type. \n
 *	This enum is used as the value of msg_recipient_type_t.
*/
enum _MSG_RECIPIENT_TYPE_E
{
	MSG_RECIPIENTS_TYPE_UNKNOWN = 0,	/**< The recipient type is unknown. */
	MSG_RECIPIENTS_TYPE_TO,			/**< The recipient type is for "To". */
	MSG_RECIPIENTS_TYPE_CC,			/**< The recipient type is for "Cc". */
	MSG_RECIPIENTS_TYPE_BCC,			/**< The recipient type is for "Bcc". */
};


/**
 *	@brief	Represents the values of a direction type. \n
 *	This enum is used as the value of msg_direction_type_t.
 */
enum _MSG_DIRECTION_TYPE_E
{
	MSG_DIRECTION_TYPE_MO = 0,		/**< The direction type is for mobile originated */
	MSG_DIRECTION_TYPE_MT,			/**< The direction type is for mobile terminated */
};


/**
 *	@brief	Represents the values of a string encoding type. \n
 *	This enum is used as the value of msg_encode_type_t.
 */
enum _MSG_ENCODE_TYPE_E
{
	MSG_ENCODE_GSM7BIT = 0,	/**< The string encoding type is GSM7BIT */
	MSG_ENCODE_8BIT,		/**< The string encoding type is 8 BIT */
	MSG_ENCODE_UCS2,		/**< The string encoding type is UCS2 */
	MSG_ENCODE_AUTO,		/**< The string encoding type is AUTO */

	MSG_ENCODE_GSM7BIT_ABNORMAL, /**< The string encoding type is GSM7BIT, but abnormal character included */
};


/**
 *	@brief	Represents the action type of Push Message. \n
 *	This enum is used as the value of msg_push_action_t.
 */
enum _MSG_PUSH_ACTION_E
{
	// SI Action
	MSG_PUSH_SI_ACTION_SIGNAL_NONE = 0x00,
	MSG_PUSH_SI_ACTION_SIGNAL_LOW,
	MSG_PUSH_SI_ACTION_SIGNAL_MEDIUM,
	MSG_PUSH_SI_ACTION_SIGNAL_HIGH,
	MSG_PUSH_SI_ACTION_DELETE,

	// SL Action
	MSG_PUSH_SL_ACTION_EXECUTE_LOW,
	MSG_PUSH_SL_ACTION_EXECUTE_HIGH,
	MSG_PUSH_SL_ACTION_CACHE,
};


/**
 *	@brief	Represents the type of SyncML Message. \n
 *	This enum is used as the value of msg_syncml_message_type_t.
 */
 enum _MSG_SYNCML_MESSAGE_TYPE_E
 {
 	DM_WBXML,					/** < DM WBXML SyncML Message */
	DM_XML,						/** < DM XML SyncML Message */
	DM_NOTIFICATION,			/** < DM Notification SyncML Message */

	DS_NOTIFICATION,			/** < DS Notification SyncML Message */
	DS_WBXML,					/** < DS WBXML SyncML Message */

	CP_XML,						/** < CP XML SyncML Message */
	CP_WBXML,					/** < CP WBXML SyncML Message */

	OTHERS,						/** < Unknown SyncML Message */
 };


/**
 *  	@brief  Represents the values of a Delivery Report Status. \n
 *	This enum is used as the value of msg_delivery_report_status_t.
*/
 enum _MSG_DELIVERY_REPORT_STATUS_E
 {
	 MSG_DELIVERY_REPORT_NONE				=	-1,	/**< Indicates the status unavailable */
	 MSG_DELIVERY_REPORT_EXPIRED 			=	0, 	/**< Indicates the expired status of message */
	 MSG_DELIVERY_REPORT_SUCCESS				=	1,	/**< Indicates the success status of message */
	 MSG_DELIVERY_REPORT_REJECTED			=	2, 	/**< Indicates the rejected status of message */
	 MSG_DELIVERY_REPORT_DEFERRED			=	3, 	/**< Indicates the deferred status of message */
	 MSG_DELIVERY_REPORT_UNRECOGNISED 		=	4, 	/**< Indicates the unrecongnised status of message */
	 MSG_DELIVERY_REPORT_INDETERMINATE 		=  	5, 	/**< Indicates the unrecongnised status of message */
	 MSG_DELIVERY_REPORT_FORWARDED	 		=	6, 	/**< Indicates the forwarded status of message */
	 MSG_DELIVERY_REPORT_UNREACHABLE  		=	7,	/**< Indicates the unreachable status of message */
	 MSG_DELIVERY_REPORT_ERROR				=	8,	/**< Indicates the error status of message */
 };


/**
 *  	@brief  Represents the values of a Read Report Status. \n
 *	This enum is used as the value of msg_read_report_status_t.
*/
enum _MSG_READ_REPORT_STATUS_E
 {
	 MSG_READ_REPORT_NONE			= 	-1,	  /**< Indicates the status unavailable */
	 MSG_READ_REPORT_IS_READ	 	= 	0,	  /**< Indicates the message is read */
	 MSG_READ_REPORT_IS_DELETED  	= 	1	  /**< Indicates the message is deleted */
 };

/**
 *  	@brief  Represents the values of a Report Type. \n
 *	This enum is used as the value of msg_read_report_status_t.
*/
enum _MSG_REPORT_TYPE_E
 {
	 MSG_REPORT_TYPE_DELIVERY	= 	0,	  /**< Indicates the type is delivery report*/
	 MSG_REPORT_TYPE_READ	= 	1,	  /**< Indicates the type is read report */

 };

// filter
/**
 *	@brief	Represents the values of a filter type. \n
 *	This enum is used as the value of msg_filter_type_t.
 */
enum _MSG_FILTER_TYPE_E
{
	MSG_FILTER_BY_WORD = 0,			/**< Filtered by sub string in the text */
	MSG_FILTER_BY_ADDRESS_SAME,			/**< Filtered by address exactly same as */
	MSG_FILTER_BY_ADDRESS_START,			/**< Filtered by address start with */
	MSG_FILTER_BY_ADDRESS_INCLUDE,			/**< Filtered by address include */
};


// mms

/**
 *	@brief	Represents the values of a mime type.
 */
typedef enum	_MimeType
{
	// 0
	MIME_ASTERISK											= 0x0000, /**< Indicates the valid default mime type  */

	// 1
	MIME_APPLICATION_XML									= 0x1000, /**< Indicates the application xml type */
	MIME_APPLICATION_WML_XML							= 0x1001,	/**< Indicates the application wml xml type  */
	MIME_APPLICATION_XHTML_XML							= 0x1002,	/**< Indicates the application xhtml xml type  */
	MIME_APPLICATION_JAVA_VM								= 0x1003, /**< Indicates the application java vm type */
	MIME_APPLICATION_SMIL								= 0x1004, /**< Indicates the application smil  type */
	MIME_APPLICATION_JAVA_ARCHIVE						= 0x1005, /**< Indicates the application java archive type */
	MIME_APPLICATION_JAVA								= 0x1006, /**< Indicates the application java  type */
	MIME_APPLICATION_OCTET_STREAM						= 0x1007, /**< Indicates the application octect stream type */
	MIME_APPLICATION_STUDIOM							= 0x1008, /**< Indicates the application studiom type */
	MIME_APPLICATION_FUNMEDIA							= 0x1009, /**< Indicates the application fun media type */
	MIME_APPLICATION_MSWORD								= 0x100a,	/**< Indicates the application ms word type */
	MIME_APPLICATION_PDF									= 0x100b, /**< Indicates the application pdf type */
	MIME_APPLICATION_SDP									= 0x100c,  /**< Indicates the application sdp type */
	MIME_APPLICATION_RAM									= 0x100d, /**< Indicates the application ram type */
	MIME_APPLICATION_ASTERIC								= 0x100e, /**< Indicates the application as main type and generic sub type */

	// 16
	MIME_APPLICATION_VND_WAP_XHTMLXML					= 0x1100, /**< Indicates the application wap xhtml xml type */
	MIME_APPLICATION_VND_WAP_WMLC						= 0x1101,  /**< Indicates the application wap wmlc type */
	MIME_APPLICATION_VND_WAP_WMLSCRIPTC				= 0x1102,  /**< Indicates the application wap wmlscrpitc type */
	MIME_APPLICATION_VND_WAP_WTA_EVENTC				= 0x1103, /**< Indicates the application wap  wta event type */
	MIME_APPLICATION_VND_WAP_UAPROF					= 0x1104, /**< Indicates the application wap uaprof type */
	MIME_APPLICATION_VND_WAP_SIC						= 0x1105,  /**< Indicates the application wap sic type */
	MIME_APPLICATION_VND_WAP_SLC						= 0x1106, /**< Indicates the application wap slc type */
	MIME_APPLICATION_VND_WAP_COC						= 0x1107, /**< Indicates the application wap coc type */
	MIME_APPLICATION_VND_WAP_SIA						= 0x1108, /**< Indicates the application wap sia type */
	MIME_APPLICATION_VND_WAP_CONNECTIVITY_WBXML		= 0x1109,  /**< Indicates the application wap connectivity wbxml type */
	MIME_APPLICATION_VND_WAP_MULTIPART_FORM_DATA	= 0x110a, /**< Indicates the application wap multipart data type */
	MIME_APPLICATION_VND_WAP_MULTIPART_BYTERANGES	= 0x110b, /**< Indicates the application wap multipart byte type */
	MIME_APPLICATION_VND_WAP_MULTIPART_MIXED			= 0x110c,  /**< Indicates the application wap multipart mixed type */
	MIME_APPLICATION_VND_WAP_MULTIPART_RELATED		= 0x110d,  /**< Indicates the application wap multipart related type */
	MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE	= 0x110e,  /**< Indicates the application wap  multipart alternative type */
	MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC		= 0x110f, /**< Indicates the application wap mulitpart as main type and generic sub type */
	MIME_APPLICATION_VND_WAP_WBXML					= 0x1110, /**< Indicates the application wap wbxml type */
	MIME_APPLICATION_VND_OMA_DD_XML					= 0x1111, /**< Indicates the application oma dd xml type */
	MIME_APPLICATION_VND_OMA_DRM_MESSAGE				= 0x1112,  /**< Indicates the application oma drm message type */
	MIME_APPLICATION_VND_OMA_DRM_CONTENT				= 0x1113, /**< Indicates the application oma drm content type */
	MIME_APPLICATION_VND_OMA_DRM_RIGHTS_XML			= 0x1114, /**< Indicates the application oma drm rights xml type */
	MIME_APPLICATION_VND_OMA_DRM_RIGHTS_WBXML		= 0x1115,  /**< Indicates the application oma drm rights wbxml type */
	MIME_APPLICATION_VND_OMA_DRM_RO_XML				= 0x1116, /**< Indicates the application oma drm ro xml type */
	MIME_APPLICATION_VND_OMA_DRM_DCF					= 0x1117, /**< Indicates the application oma drm dcf type */
	MIME_APPLICATION_VND_OMA_ROAPPDU_XML				= 0x1118,  /**< Indicates the application oma roap pdu xml type */
	MIME_APPLICATION_VND_OMA_ROAPTRIGGER_XML			= 0x1119,	/**< Indicates the application oma roap trigger xml  type */
	MIME_APPLICATION_VND_SMAF							= 0x111a,  /**< Indicates the application smaf type */
	MIME_APPLICATION_VND_RN_REALMEDIA					= 0x111b,  /**< Indicates the application rn real media type */
	MIME_APPLICATION_VND_SUN_J2ME_JAVA_ARCHIVE		= 0x111c,  /**< Indicates the application j2me java archive type */
	MIME_APPLICATION_VND_SAMSUNG_THEME				= 0x111d,  /**< Indicates the application samsung theme type */
	MIME_APPLICATION_VND_EXCEL							= 0x111e,  /**< Indicates the application excel type */
	MIME_APPLICATION_VND_POWERPOINT					= 0x111f,  /**< Indicates the application power point type */
	MIME_APPLICATION_VND_MSWORD						= 0x1120,	 /**< Indicates the application ms word type */

	// 49
	MIME_APPLICATION_X_HDMLC							= 0x1200, /**< Indicates the application x hdmlc type */
	MIME_APPLICATION_X_X968_USERCERT					= 0x1201, /**< Indicates the application x x968 user certified type */
	MIME_APPLICATION_X_WWW_FORM_URLENCODED			= 0x1202, /**< Indicates the application x www form url encoded type */
	MIME_APPLICATION_X_SMAF								= 0x1203, /**< Indicates the application x smaf type */
	MIME_APPLICATION_X_FLASH								= 0x1204, /**< Indicates the application x flash type */
	MIME_APPLICATION_X_EXCEL								= 0x1205, /**< Indicates the application x excel type */
	MIME_APPLICATION_X_POWERPOINT						= 0x1206, /**< Indicates the application x power point type */

	// 56
	MIME_AUDIO_BASIC										= 0x2000, /**< Indicates the audio basic type  */
	MIME_AUDIO_MPEG										= 0x2001, /**< Indicates the audio mpeg type  */
	MIME_AUDIO_MP3										= 0x2002, /**< Indicates the audio mp3 type  */
	MIME_AUDIO_MPG3										= 0x2003,  /**< Indicates the audio mpg3 type  */
	MIME_AUDIO_MPEG3										= 0x2004, /**< Indicates the audio mpeg3 type  */
	MIME_AUDIO_MPG										= 0x2005, /**< Indicates the audio mpg type  */
	MIME_AUDIO_AAC										= 0x2006, /**< Indicates the audio aac type  */
	MIME_AUDIO_G72										= 0x2007, /**< Indicates the audio g72 type  */
	MIME_AUDIO_AMR										= 0x2008, /**< Indicates the audio amr type  */
	MIME_AUDIO_AMR_WB									= 0x2009, /**< Indicates the audio amr wb type  */
	MIME_AUDIO_MMF										= 0x200a, /**< Indicates the audio mmf type  */
	MIME_AUDIO_SMAF										= 0x200b, /**< Indicates the audio smaf type  */
	MIME_AUDIO_IMELODY									= 0x200c, /**< Indicates the audio imelody type  */
	MIME_AUDIO_IMELODY2									= 0x200d, /**< Indicates the audio imelody2 type  */
	MIME_AUDIO_MELODY									= 0x200e, /**< Indicates the audio melody type  */
	MIME_AUDIO_MID										= 0x200f, /**< Indicates the audio mid type  */
	MIME_AUDIO_MIDI										= 0x2010, /**< Indicates the audio midi type  */
	MIME_AUDIO_SP_MIDI									= 0x2011, /**< Indicates the audio sp midi type  */
	MIME_AUDIO_WAVE										= 0x2012, /**< Indicates the audio wave type  */
	MIME_AUDIO_WAV										= 0x2013, /**< Indicates the audio wav type  */
	MIME_AUDIO_3GPP										= 0x2014, /**< Indicates the audio 3gpp type  */
	MIME_AUDIO_MP4										= 0x2015, /**< Indicates the audio mp4 type  */
	MIME_AUDIO_MP4A_LATM								= 0x2016, /**< Indicates the audio mp4 latm type  */
	MIME_AUDIO_M4A										= 0x2017, /**< Indicates the audio m4a type  */
	MIME_AUDIO_MPEG4										= 0x2018, /**< Indicates the audio mpeg4 type  */
	MIME_AUDIO_WMA										= 0x2019, /**< Indicates the audio wma type  */
	MIME_AUDIO_XMF										= 0x201a, /**< Indicates the audio xmf type  */
	MIME_AUDIO_IMY										= 0x201b, /**< Indicates the audio imy type  */
	MIME_AUDIO_MOBILE_XMF								= 0x201c, /**< Indicates the audio mobile xmf type  */

	//85
	MIME_AUDIO_VND_RN_REALAUDIO						= 0x2100, /**< Indicates the audio rn real audio type  */

	// 86
	MIME_AUDIO_X_MPEG									= 0x2200, /**< Indicates the audio x mpeg type  */
	MIME_AUDIO_X_MP3										= 0x2201, /**< Indicates the audio x mp3 type  */
	MIME_AUDIO_X_MPEG3									= 0x2202,    /**< Indicates the audio x mpeg3 type  */
	MIME_AUDIO_X_MPG										= 0x2203, /**< Indicates the audio x mpg type  */
	MIME_AUDIO_X_AMR										= 0x2204, /**< Indicates the audio x amr type  */
	MIME_AUDIO_X_MMF										= 0x2205, /**< Indicates the audio x mmf type  */
	MIME_AUDIO_X_SMAF									= 0x2206, /**< Indicates the audio x smaf type  */
	MIME_AUDIO_X_IMELODY									= 0x2207, /**< Indicates the audio x imelody type  */
	MIME_AUDIO_X_MIDI										= 0x2208, /**< Indicates the audio x midi type  */
	MIME_AUDIO_X_MPEGAUDIO								= 0x2209, /**< Indicates the audio x mpeg  type  */
	MIME_AUDIO_X_PN_REALAUDIO							= 0x220a, /**< Indicates the audio x pn real audio type  */
	MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO				= 0x220b, /**< Indicates the audio x pn multirate real audio  type  */
	MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO_LIVE			= 0x220c, /**< Indicates the audio x pn multirate real audio live type  */
	MIME_AUDIO_X_WAVE									= 0x220d, /**< Indicates the audio x wave  type  */
	MIME_AUDIO_X_WAV										= 0x220e, /**< Indicates the audio x wav  type  */
	MIME_AUDIO_X_MS_WMA									= 0x220f, /**< Indicates the audio ms wma type  */
	MIME_AUDIO_X_MID										= 0x2210, /**< Indicates the audio mid type  */
	MIME_AUDIO_X_MS_ASF									= 0x2211, /**< Indicates the audio ms asf  type  */
	MIME_AUDIO_X_XMF										= 0x2212, /**< Indicates the audio x xmf  type  */

	// 105
	MIME_IMAGE_GIF										= 0x3000, /**< Indicates the image gif type  */
	MIME_IMAGE_JPEG										= 0x3001, /**< Indicates the image jpeg type  */
	MIME_IMAGE_JPG										= 0x3002, /**< Indicates the image jpg type  */
	MIME_IMAGE_TIFF										= 0x3003, /**< Indicates the image tiff type  */
	MIME_IMAGE_TIF										= 0x3004, /**< Indicates the image tif type  */
	MIME_IMAGE_PNG										= 0x3005, /**< Indicates the image png type  */
	MIME_IMAGE_WBMP										= 0x3006, /**< Indicates the image wbmp type  */
	MIME_IMAGE_PJPEG										= 0x3007, /**< Indicates the image pjpeg type  */
	MIME_IMAGE_BMP										= 0x3008, /**< Indicates the image bmp type  */
	MIME_IMAGE_SVG										= 0x3009, /**< Indicates the image svg type  */
	MIME_IMAGE_SVG1										= 0x300a, /**< Indicates the image svg1 type  */

	// 116
	MIME_IMAGE_VND_WAP_WBMP							= 0x3100,  /**< Indicates the image vnd wap wbmp type  */
	MIME_IMAGE_VND_TMO_GIF								= 0x3101,  /**< Indicates the image vnd tmo gif type  */
	MIME_IMAGE_VND_TMO_JPG								= 0x3102,  /**< Indicates the image vnd tmo jpg type  */

	// 119
	MIME_IMAGE_X_BMP										= 0x3200, /**< Indicates the image x bmp type  */

	// 120
	MIME_MESSAGE_RFC822									= 0x4000,  /**< Indicates the message rfc822 type  */

	// 121
	MIME_MULTIPART_MIXED									= 0x5000,  /**< Indicates the multipart mixed type  */
	MIME_MULTIPART_RELATED								= 0x5001, /**< Indicates the multipart related type  */
	MIME_MULTIPART_ALTERNATIVE							= 0x5002, /**< Indicates the multipart alternative type  */
	MIME_MULTIPART_FORM_DATA							= 0x5003, /**< Indicates the multipart form data type  */
	MIME_MULTIPART_BYTERANGE							= 0x5004, /**< Indicates the multipart byte range type  */
	MIME_MULTIPART_REPORT								= 0x5005, /**< Indicates the multipart report type  */
	MIME_MULTIPART_VOICE_MESSAGE						= 0x5006, /**< Indicates the multipart voice message type  */

	// 128
	MIME_TEXT_TXT											= 0x6000, /**< Indicates the text txt type  */
	MIME_TEXT_HTML										= 0x6001, /**< Indicates the text html type  */
	MIME_TEXT_PLAIN										= 0x6002,  /**< Indicates the text plain type  */
	MIME_TEXT_CSS											= 0x6003,  /**< Indicates the text css type  */
	MIME_TEXT_XML											= 0x6004,  /**< Indicates the text xml type  */
	MIME_TEXT_IMELODY										= 0x6005,  /**< Indicates the text imelody type  */

	// 134
	MIME_TEXT_VND_WAP_WMLSCRIPT						= 0x6100, /**< Indicates the text wap wmlscript  type  */
	MIME_TEXT_VND_WAP_WML								= 0x6101, /**< Indicates the text wap wml  type  */
	MIME_TEXT_VND_WAP_WTA_EVENT						= 0x6102, /**< Indicates the text wap wta event  type  */
	MIME_TEXT_VND_WAP_CONNECTIVITY_XML				= 0x6103, /**< Indicates the text wap connectivity xml  type  */
	MIME_TEXT_VND_WAP_SI									= 0x6104,  /**< Indicates the text wap si  type  */
	MIME_TEXT_VND_WAP_SL								= 0x6105, /**< Indicates the text wap sl  type  */
	MIME_TEXT_VND_WAP_CO								= 0x6106, /**< Indicates the text wap co  type  */
	MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR				= 0x6107, /**< Indicates the text sun j2me type  */

	// 142
	MIME_TEXT_X_HDML										= 0x6200, /**< Indicates the x html  type  */
	MIME_TEXT_X_VCALENDAR								= 0x6201,  /**< Indicates the x calendar  type  */
	MIME_TEXT_X_VCARD									= 0x6202, /**< Indicates the x vcard  type  */
	MIME_TEXT_X_IMELODY									= 0x6203, /**< Indicates the x imelody  type  */
	MIME_TEXT_X_IMELODY2									= 0x6204, /**< Indicates the x imelody2  type  */
	MIME_TEXT_X_VNOTE									= 0x6205,  /**< Indicates the x vnote  type  */

	// 148
	MIME_VIDEO_MPEG4										= 0x7000, /**< Indicates the mpeg4  type  */
	MIME_VIDEO_MP4										= 0x7001, /**< Indicates the mp4  type  */
	MIME_VIDEO_H263										= 0x7002, /**< Indicates the h263  type  */
	MIME_VIDEO_3GPP										= 0x7003, /**< Indicates the 3gpp  type  */
	MIME_VIDEO_3GP										= 0x7004, /**< Indicates the 3gp  type  */
	MIME_VIDEO_AVI										= 0x7005, /**< Indicates the avi  type  */
	MIME_VIDEO_SDP										= 0x7006, /**< Indicates the sdp  type  */
	MIME_VIDEO_MP4_ES									= 0x7007, /**< Indicates the mp4 es  type  */
	MIME_VIDEO_MPEG										= 0x7008, /**< Indicates the mpeg  type  */

	// 157
	MIME_VIDEO_VND_RN_REALVIDEO							= 0x7100, /**< Indicates the pn real video type  */
	MIME_VIDEO_VND_RN_REALMEDIA							= 0x7101, /**< Indicates the pn multi rate real media type  */

	// 159
	MIME_VIDEO_X_MP4										= 0x7200, /**< Indicates the video x mp4 type  */
	MIME_VIDEO_X_PV_MP4									= 0x7201, /**< Indicates the video x pv mp4 type  */
	MIME_VIDEO_X_PN_REALVIDEO							= 0x7202, /**< Indicates the x pn real video type  */
	MIME_VIDEO_X_PN_MULTIRATE_REALVIDEO				= 0x7203, /**< Indicates the x pn multi rate real video type  */
	MIME_VIDEO_X_MS_WMV									= 0x7204, /**< Indicates the x ms wmv type  */
	MIME_VIDEO_X_MS_ASF									= 0x7205, /**< Indicates the x ms asf type  */
	MIME_VIDEO_X_PV_PVX									= 0x7206, /**< Indicates the x pv pvx type  */

	MIME_TYPE_VALUE_MAX									= 0x7207,		/**< Indicates the maximum mime type  */
	MIME_UNKNOWN											= 0xffff	/**< Indicates the unknown mime type  */

} MimeType;

/**
 *	@brief	Represents the values of a DRM type.
 */
//#ifdef __SUPPORT_DRM__
typedef enum
{
	MSG_DRM_TYPE_NONE	= 0, /**< Indicates the drm type none */
	MSG_DRM_TYPE_FL		= 1, /**< Indicates the forward lock */		/* 2004-07-09: forwardLock type */
	MSG_DRM_TYPE_CD		= 2, /**< Indicates the combined delivery */	/* 2004-07-09: combined delivery type */
	MSG_DRM_TYPE_SD		= 3, /**< Indicates the separate delivery */	/* 2004-07-09: seperate delivery type */
	MSG_DRM_TYPE_SSD	= 4	/**< Indicates the special separate delivery */	// 2005-02-28: add Special Sperate Delivery
}MsgDrmType;
//#endif

/**
 *	@brief	Represents the values of a SMIL region type.
 */
typedef enum _REGION_FIT_TYPE_T
{
	MMSUI_IMAGE_REGION_FIT_HIDDEN,	 /**< Indicates the hidden fit type */
	MMSUI_IMAGE_REGION_FIT_MEET,	 /**< Indicates the meet fit type */
}REGION_FIT_TYPE_T;


/**
 *	@brief	Represents the values of a SMIL media type.
 */
typedef enum
{
	MMS_SMIL_MEDIA_INVALID = 0, /**< Indicates the invalid media type */
	MMS_SMIL_MEDIA_IMG,			/**< Indicates the image media */
	MMS_SMIL_MEDIA_AUDIO,		/**< Indicates the audio media */
	MMS_SMIL_MEDIA_VIDEO,		/**< Indicates the video media */
	MMS_SMIL_MEDIA_TEXT,		/**< Indicates the text media */
	MMS_SMIL_MEDIA_ANIMATE,		/**< Indicates the animation media */
	MMS_SMIL_MEDIA_IMG_OR_VIDEO	, /**< Indicates the image or video media */
	MMS_SMIL_MEDIA_MAX = 0xffffffff,	/**< Indicates the maximum media type */
}MmsSmilMediaType;

/**
 *	@brief	Represents the values of a SMIL transition type.
 */
typedef enum
{
	MMS_SMIL_TRANS_NONE = 0,	 	/**< Indicates the transition type none */			/* default */
	MMS_SMIL_TRANS_SLIDEWIPE = 1,	/**< Indicates the slide wipe transition */
	MMS_SMIL_TRANS_BARWIPE = 2,		/**< Indicates the bar wipe transition */
	MMS_SMIL_TRANS_BARNDOORWIPE = 3, /**< Indicates the bar and door wipe transition */
	MMS_SMIL_TRANS_FADE = 4,		/**< Indicates the fade transition */
	MMS_SMIL_TRANS_RANDOMBLOCK = 5,	/**< Indicates the random block transition */
	MMS_SMIL_TRANS_ZOOMIN = 6,		/**< Indicates the zoom in transition */
	MMS_SMIL_TRANS_IRISWIPE = 7,	/**< Indicates the iris wipe transition */
 	MMS_SMIL_TRANS_BOXWIPE = 8,		/**< Indicates the box wipe transition */
	MMS_SMIL_TRANS_FOURBOXWIPE = 9,	/**< Indicates the four box wipe transition */
	MMS_SMIL_TRANS_PUSHWIPE  =10,	/**< Indicates the push wipe transition */
	MMS_SMIL_TRANS_ELLIPSEWIPE  = 11 /**< Indicates the ellipse wipe transition */
}MmsSmilTransType;

/**
 *	@brief	Represents the values of a SMIL transition sub type.
 */
typedef enum
{
	MMS_SMIL_TRANS_SUB_NONE = 0,	/**< Indicates the transition sub type none */
	MMS_SMIL_TRANS_SUB_FROM_LEFT = 1,	/**< Indicates the from left transition */	/* slideWipe's default */
	MMS_SMIL_TRANS_SUB_FROM_TOP = 2,	/**< Indicates the from top transition */
	MMS_SMIL_TRANS_SUB_FROM_BOTTOM = 3,	/**< Indicates the from bottom transition */
	MMS_SMIL_TRANS_SUB_TOP_TO_BOTTOM = 4, /**< Indicates the from top to bottom transition */			/* barWipe's default */
	MMS_SMIL_TRANS_SUB_BOTTOM_TO_TOP = 5, /**< Indicates the from bottom to top transition */
	MMS_SMIL_TRANS_SUB_HORIZONTAL = 6,	/**< Indicates the horizontal transition */		/* barDoorWipe's default */
    MMS_SMIL_TRANS_SUB_FROM_RIGHT = 7, /**< Indicates the from right transition */
    MMS_SMIL_TRANS_SUB_VERTICAL = 8 /**< Indicates the vertical transition */
}MmsSmilTransSubType;

/**
 *	@brief	Represents the values of a text font type.
 */
typedef enum
{
	MMS_SMIL_FONT_TYPE_NONE = 0, /**< Indicates the font type none */
	MMS_SMIL_FONT_TYPE_NORMAL = 1, /**< Indicates the font type normal */
	MMS_SMIL_FONT_TYPE_ITALIC = 2, /**< Indicates the font type italic */
	MMS_SMIL_FONT_TYPE_BOLD = 3, /**< Indicates the font type bold */
	MMS_SMIL_FONT_TYPE_UNDERLINE = 4 /**< Indicates the font type underline */
}MmsSmilFontType;

/**
 *	@brief	Represents the values of a MMS text direction.
 */
typedef enum	_MmsTextDir{
	MMS_TEXT_DIRECTION_INVALID = -1, /**< Indicates the invalid direction */
	MMS_TEXT_DIRECTION_RIGHT = 0,	/**< Indicates the right direction */
	MMS_TEXT_DIRECTION_DOWN,		/**< Indicates the down direction */		// supported to GC
} MmsTextDirection;

/**
 *	@brief	Represents the values of MMS Read Report Sent Status.
 */
typedef enum
{
	MMS_RECEIVE_READ_REPORT_NO_SEND,	// didn't send yet
	MMS_RECEIVE_READ_REPORT_MUST_SEND,	// didn't send yet but will send
	MMS_RECEIVE_READ_REPORT_SENT,		// sent
	MMS_RECEIVE_READ_REPORT_NO_SENT,	// required but, didn't send by user's choice
} MmsRecvReadReportSendStatus ;


// setting

/**
 *	@brief	Represents the values of an option type. \n
 *	This enum is used as the value of MSG_OPTION_TYPE_T.
 */
enum _MSG_OPTION_TYPE_E
{
	MSG_GENERAL_OPT,			/**< General option */
	MSG_SMS_SENDOPT,			/**< SMS send option */
	MSG_SMSC_LIST,				/**< SMSC list option */
	MSG_MMS_SENDOPT,			/**< MMS send option */
	MSG_MMS_RECVOPT,			/**< MMS receive option */
	MSG_MMS_CLASSOPT,			/**< MMS class option such as personal, advertisement, and informational */
	MSG_MMS_STYLEOPT,			/**< MMS style option */
	MSG_PUSHMSG_OPT,			/**< Push message option */
	MSG_CBMSG_OPT,			/**< Cell broadcasting message option */
	MSG_VOICEMAIL_OPT,			/**< Voice mail option */
	MSG_MSGSIZE_OPT,			/**< Message size option */
};


/**
 *	@brief	Represents the values of an SMS network mode. \n
 *	This enum is used as the value of MSG_SMS_NETWORK_MODE_T.
 */
enum _MSG_SMS_NETWORK_MODE_E
{
	MSG_SMS_NETWORK_PS_ONLY = 0x01,		/**< Packet switched */
	MSG_SMS_NETWORK_CS_ONLY = 0x02,		/**< Circuit switched */
	MSG_SMS_NETWORK_PS_FIRST = 0x03,		/**< Packet switching preferred */
};


/**
 *	@brief	Represents the value of period of playing alert tone. \n
 *	The values for this type SHOULD be in MSG_ALERT_TONE_T.
 */
enum _MSG_ALERT_TONE_E
{
	MSG_ALERT_TONE_ONCE,
	MSG_ALERT_TONE_2MINS,
	MSG_ALERT_TONE_5MINS,
	MSG_ALERT_TONE_10MINS,
};


/**
 *	@brief	Represents the value of a default storage to save SMS. \n
 *	The values for this type SHOULD be in MSG_SMS_SAVE_STORAGE_T.
 */
enum _MSG_SMS_SAVE_STORAGE_E
{
	MSG_SMS_SAVE_STORAGE_SIM,
	MSG_SMS_SAVE_STORAGE_PHONE,
};


/**
 *	@brief	Represents the values of the type of number for SMS center address. \n
 *	This enum is used as the value of MSG_SMS_TON_T.
 */
enum _MSG_SMS_TON_E
{
	MSG_TON_UNKNOWN = 0,			/**< Unknown */
	MSG_TON_INTERNATIONAL,		/**< International */
	MSG_TON_NATIONAL,				/**< National */
	MSG_TON_NETWORK,				/**< Network */
	MSG_TON_SUBSCRIBER,			/**< Subscriber */
	MSG_TON_ALPHANUMERIC,		/**< Alpha numeric */
	MSG_TON_ABBREVIATED,			/**< Abbreviated */
	MSG_TON_RESERVE,				/**< Reserve */
};


/**
 *	@brief	Represents the values of the numbering plan ID for SMS center address. \n
 *	This enum is used as the value of MSG_SMS_NPI_T.
 */
enum _MSG_SMS_NPI_E
{
	MSG_NPI_UNKNOWN = 0,			/**< Unknown */
	MSG_NPI_ISDN = 1,				/**< ISDN */
	MSG_NPI_DATA = 3,				/**< Data */
	MSG_NPI_TELEX = 4,				/**< Telex */
	MSG_NPI_SMSC = 5,				/**< SMSC */
	MSG_NPI_NATIONAL = 8,			/**< National */
	MSG_NPI_PRIVATE = 9,			/**< Private */
	MSG_NPI_IDENTIFICATION_ERMES = 10,		/**< Identification ermes */
	MSG_NPI_IDENTIFICATION_RESERVED = 0xF		/**< Identification reserved */
};


/**
 *	@brief	Represents the values of the protocol ID for SMS center address. \n
 *	This enum is used as the value of MSG_SMS_PID_T.
 */
enum  _MSG_SMS_PID_E
{
	MSG_PID_NONE,		/**< None */
	MSG_PID_TEXT,		/**< Text */
	MSG_PID_VOICE,		/**< Voice */
	MSG_PID_FAX,		/**< Fax */
	MSG_PID_X400,		/**< X400 */
	MSG_PID_ERMES,		/**< Ermes */
	MSG_PID_EMAIL,		/**< Email */
	MSG_PID_MAX		/**< Default */
};


/**
 *	@brief	Represents the values of the Validity Period for SMS center. \n
 *	This enum is used as the value of MSG_VAL_PERIOD_T.
 */
enum  _MSG_VAL_PERIOD_E
{
	MSG_VAL_1DAY = 167,
	MSG_VAL_2DAYS = 168,
	MSG_VAL_1WEEK = 173,
	MSG_VAL_2WEEKS = 180,
	MSG_VAL_MAXIMUM = 255,
};


enum  _MSG_MMS_EXPIRY_TIME_E
{
	MSG_EXPIRY_TIME_MAXIMUM = 0,
	MSG_EXPIRY_TIME_1DAY = 86400,
	MSG_EXPIRY_TIME_2DAYS = 2*86400,
	MSG_EXPIRY_TIME_1WEEK = 604800,
	MSG_EXPIRY_TIME_2WEEKS = 2*604800,
};


enum  _MSG_MMS_DELIVERY_TIME_E
{
	MSG_DELIVERY_TIME_IMMEDIATLY = 0,
	MSG_DELIVERY_TIME_1HOUR = 3600,
	MSG_DELIVERY_TIME_1DAY = 86400,
	MSG_DELIVERY_TIME_1WEEK = 604800,
	MSG_DELIVERY_TIME_CUSTOM
};


enum _MSG_MMS_MSG_CLASS_TYPE_E
{
	MSG_CLASS_PERSONAL,			/**<Personal message class*/
	MSG_CLASS_ADVERTISEMENT,		/**<Advertisement message class*/
	MSG_CLASS_INFORMATIONAL,		/**<Informational message class */
	MSG_CLASS_AUTO,				/**<Automatic */
	MSG_CLASS_MAX					/**< */
};


enum _MSG_MMS_REPLY_CHARGING_TYPE_E
{
	MSG_REPLY_CHARGING_NONE,					/**<Reply-Charging none */
	MSG_REPLY_CHARGING_REQUESTED,				/**<Reply-Charging requested */
	MSG_REPLY_CHARGING_REQUESTED_TEXT_ONLY,	/**<Reply-Charging requested text-only */
	MSG_REPLY_CHARGING_ACCEPTED,				/**<Reply-Charging accepted */
	MSG_REPLY_CHARGING_ACCEPTED_TEXT_ONLY,	/**<Reply-Charging accepted text-only */
	MSG_REPLY_CHARGING_MAX
};


enum _MSG_MMS_CREATION_MODE_E
{
	MSG_CREATION_MODE_RESTRICTED,
	MSG_CREATION_MODE_WARNING,
	MSG_CREATION_MODE_FREE
};


/**
 *	@brief	Represents the values of the retrieve type for MMS home network. \n
 *	This enum is used as the value of MSG_MMS_HOME_RETRIEVE_TYPE_T.
 */
enum _MSG_MMS_HOME_RETRIEVE_TYPE_E
{
	MSG_HOME_AUTO_DOWNLOAD,		/**< Home auto download */
	MSG_HOME_MANUAL,					/**< Home manual */
	MSG_HOME_RESTRICTED,				/**< Home restricted */
	MSG_HOME_REJECT,					/**< Home reject */
	MSG_HOME_MAX						/**< Default */
};


/**
 *	@brief	Represents the values of the retrieve type for MMS abroad network. \n
 *	This enum is used as the value of MSG_MMS_ABROAD_RETRIEVE_TYPE_T.
 */
enum _MSG_MMS_ABROAD_RETRIEVE_TYPE_E
{
	MSG_ABROAD_AUTO_DOWNLOAD,	/**< Abroad auto download */
	MSG_ABROAD_MANUAL,			/**< Abroad manual */
	MSG_ABROAD_RESTRICTED,		/**< Abroad restricted */
	MSG_ABROAD_REJECT,			/**< Abroad reject */
	MSG_ABROAD_MAX				/**< Default */
};


/**
 *	@brief	Represents the values of the frequency of sending MMS read report. \n
 *	This enum is used as the value of MSG_MMS_SEND_READ_REPORT_T.
 */
enum _MSG_MMS_SEND_READ_REPORT_E
{
	MSG_SEND_READ_REPORT_ALWAYS,	/**< Read report always */
	MSG_SEND_READ_REPORT_NEVER,		/**< Read report never */
	MSG_SEND_READ_REPORT_REQUEST,	/**< Read report request */
	MSG_SEND_READ_REPORT_MAX		/**< Default */
};


/**
 *	@brief	Represents the values of the service type for a push message. \n
 *	This enum is used as the value of MSG_PUSH_SERVICE_TYPE_T.
 */
enum _MSG_PUSH_SERVICE_TYPE_E
{
	MSG_PUSH_SERVICE_ALWAYS,	/**< Push message service always */
	MSG_PUSH_SERVICE_PROMPT,		/**< Push message service prompt */
	MSG_PUSH_SERVICE_NEVER,		/**< Push message service never */
};


/**
 *	@brief	Represents the values of the language type for a cell broadcasting message. \n
 *	This enum is used as the value of MSG_CB_LANGUAGE_TYPE_T.
 */
enum _MSG_CB_LANGUAGE_TYPE_E
{
	MSG_CBLANG_TYPE_ALL,			/**< CB check all */
	MSG_CBLANG_TYPE_ENG,			/**< CB message English  */
	MSG_CBLANG_TYPE_GER,			/**< CB message Germany */
	MSG_CBLANG_TYPE_FRE,			/**< CB message France */
	MSG_CBLANG_TYPE_ITA,			/**< CB message Italy */
	MSG_CBLANG_TYPE_NED,			/**< CB message Netherland */
	MSG_CBLANG_TYPE_SPA,			/**< CB message Spain */
	MSG_CBLANG_TYPE_POR,			/**< CB message Portugal */
	MSG_CBLANG_TYPE_SWE,			/**< CB message Sweden */
	MSG_CBLANG_TYPE_TUR,			/**< CB message Turkey */
	MSG_CBLANG_TYPE_MAX			/**< Default */
};


/**
 *	@brief	Represents the values of SIM status. \n
 *	This enum is used as the value of MSG_SIM_STATUS_T.
 */
enum _MSG_SIM_STATUS_E
{
	MSG_SIM_STATUS_NORMAL = 0,
	MSG_SIM_STATUS_CHANGED,
	MSG_SIM_STATUS_NOT_FOUND,
};


#endif /* MSG_TYPES_H_ */
