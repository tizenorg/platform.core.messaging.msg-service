/*
*
* Copyright (c) 2000-2012 Samsung Electronics Co., Ltd. All Rights Reserved.
*
* This file is part of msg-service.
*
* Contact: Jaeyun Jeong <jyjeong@samsung.com>
*          Sangkoo Kim <sangkoo.kim@samsung.com>
*          Seunghwan Lee <sh.cat.lee@samsung.com>
*          SoonMin Jung <sm0415.jung@samsung.com>
*          Jae-Young Lee <jy4710.lee@samsung.com>
*          KeeBum Kim <keebum.kim@samsung.com>
*
* PROPRIETARY/CONFIDENTIAL
*
* This software is the confidential and proprietary information of
* SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered
* into with SAMSUNG ELECTRONICS.
*
* SAMSUNG make no representations or warranties about the suitability
* of the software, either express or implied, including but not limited
* to the implied warranties of merchantability, fitness for a particular
* purpose, or non-infringement. SAMSUNG shall not be liable for any
* damages suffered by licensee as a result of using, modifying or
* distributing this software or its derivatives.
*
*/

#ifndef MSG_TYPES_H
#define MSG_TYPES_H

/**
 *	@file 		MsgTypes.h
 *	@brief 		Defines common types of messaging framework
 *	@version 	1.0
 */

/**
 *	@section		Introduction
 *	- Introduction : Overview on Message Common Types
 *	@section		Program
 *	- Program : Message Common Types Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_COMMON_TYPES	Messaging Common Types
 *	@{
 */

/*==================================================================================================
                                    DEFINES
==================================================================================================*/
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

#define MAX_WAPPUSH_ID_LEN			40
#define MAX_WAPPUSH_HREF_LEN		300
#define MAX_WAPPUSH_CONTENTS_LEN	2048

#define MAX_PUSH_CACHEOP_INVALID_OBJECT_MAX	5
#define MAX_PUSH_CACHEOP_INVALID_SERVICE_MAX	5
#define MAX_PUSH_CACHEOP_MAX_URL_LEN			200

#define MAX_COMMON_INFO_SIZE	20

/**
 *	@brief	Defines the maximum file path length
 */
#define	MSG_FILEPATH_LEN_MAX	1024

#define DEPRECATED __attribute__((deprecated))

#ifndef EXPORT_API
#define EXPORT_API __attribute__ ((visibility("default")))
#endif

/*==================================================================================================
                                         TYPES
==================================================================================================*/

/**
 *	@brief	Defines Message handle.
 */
typedef struct MessagingHandle *MSG_HANDLE_T;

typedef struct opq_message_s *msg_message_t;

typedef struct opq_threadview_s *msg_thread_view_t;

/**
 *	@brief	Represents a messsage ID.
 */
typedef unsigned int MSG_MESSAGE_ID_T;


/**
 *	@brief	Represents a reference ID.
 */
typedef unsigned int MSG_REFERENCE_ID_T;


/**
 *	@brief	Represents a storage type.
 	See enum _MSG_STORAGE_ID_E
 */
typedef unsigned char MSG_STORAGE_ID_T;


/**
 *	@brief	Represents a folder ID.
 	See enum _MSG_FOLDER_ID_E
 */
typedef char MSG_FOLDER_ID_T;


/**
 *	@brief	Represents a request ID, which is unique for each request.
 */
typedef unsigned int MSG_REQUEST_ID_T;


/**
 *	@brief	Represents a message priority. \n
 *	The values for this type SHOULD be in _MSG_PRIORITY_TYPE_E.
 */
typedef unsigned char MSG_PRIORITY_TYPE_T;


/**
 *	@brief	Represents the network status of a message. \n
 *	The values for this type SHOULD be in _MSG_NETWORK_STATUS_E.
 */
typedef unsigned char MSG_NETWORK_STATUS_T;


/**
 *	@brief	Represents an address type. \n
 *	The values for this type SHOULD be in _MSG_ADDRESS_TYPE_E.
 */
typedef unsigned char MSG_ADDRESS_TYPE_T;


/**
 *	@brief	Represents an recipient type. \n
 *	The values for this type SHOULD be in _MSG_RECIPIENT_TYPE_E.
 */
typedef unsigned char MSG_RECIPIENT_TYPE_T;


/**
 *	@brief	Represents the type of a message direction. \n
 *	The values for this type SHOULD be in _MSG_DIRECTION_TYPE_E.
 */
typedef unsigned char MSG_DIRECTION_TYPE_T;


/**
 *	@brief	Represents an encoding type. \n
 *	The values for this type SHOULD be in _MSG_ENCODE_TYPE_E.
 */
typedef unsigned char MSG_ENCODE_TYPE_T;


/**
 *	@brief	Represents an error code. \n
 *	The values for this type SHOULD be in _MSG_ERROR_E
 */
typedef int MSG_ERROR_T;


/**
\brief Represents WAP Push App Code.
*/
typedef unsigned char MSG_PUSH_ACTION_T;


/**
\brief Represents SyncML Message Type.
*/
typedef unsigned short MSG_SYNCML_MESSAGE_TYPE_T;


/**
 *	@brief	Represents a Contact ID.
 */
typedef unsigned int MSG_CONTACT_ID_T;


/**
  *  @brief	Represents a Delivery Report Status.
 *	The values for this type SHOULD be in _MSG_DELIVERY_REPORT_STATUS_E
  */
typedef int MSG_DELIVERY_REPORT_STATUS_T;


/**
 *	@brief	Represents a Read Report Status.
 *	The values for this type SHOULD be in _MSG_READ_REPORT_STATUS_E
 */
typedef int MSG_READ_REPORT_STATUS_T;


/**
 *	@brief	Represents a Message Type.
 *	The values for this type SHOULD be in _MSG_MESSAGE_TYPE_E
*/
typedef unsigned short MSG_MESSAGE_TYPE_T;


/**
 *	@brief	Represents a thread ID. \n
 */
typedef unsigned int MSG_THREAD_ID_T;


/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/

/**
 *	@brief	Represents address information.
 */
typedef struct
{
	MSG_THREAD_ID_T		threadId;								/**< the thread ID of address */
	MSG_ADDRESS_TYPE_T		addressType;							/**< The type of an address in case of an Email or a mobile phone */
	MSG_RECIPIENT_TYPE_T	recipientType;							/**< The type of recipient address in case of To, Cc, and Bcc */
	MSG_CONTACT_ID_T		contactId;							/**< The contact ID of address */
	char						addressVal[MAX_ADDRESS_VAL_LEN+1];	/**< The actual value of an address */
	char						displayName[MAX_DISPLAY_NAME_LEN+1];	/**< The display name of an address */
} MSG_ADDRESS_INFO_S;


/**
 *	@brief	Represents port number information.
 */
typedef struct
{
	bool				valid;		/**< Indicates whether port information is used or not. */
	unsigned short		dstPort;		/**< Recipient port number, not greater than 16 bit */
	unsigned short		srcPort;		/**< Sender port number, not greater than 16 bit */
} MSG_PORT_INFO_S;


/**
 *	@brief	Represents MMS Sending Option Structure in application.
 */
typedef struct
{
	bool					bReadReq;
	time_t				expiryTime;
	bool					bUseDeliveryCustomTime;
	time_t				deliveryTime;
	MSG_PRIORITY_TYPE_T	priority;
} MMS_SENDINGOPT_S;


/**
 *	@brief	Represents SMS Sending Option Structure in application.
 */
typedef struct
{
	bool bReplyPath;
} SMS_SENDINGOPT_S;


/**
 *	@brief	Represents MSG Sending Option Structure in application.
 */
typedef struct
{
	bool bSetting;
	bool bDeliverReq;
	bool bKeepCopy;

	union
	{
		MMS_SENDINGOPT_S 	mmsSendOpt;
		SMS_SENDINGOPT_S 	smsSendOpt;
	} option;
} MSG_SENDINGOPT_S;


typedef	enum
{
	MMS_TIMETYPE_NONE		= -1,	// fixme: backward compatibility
	MMS_TIMETYPE_ERROR		= -1,	// error return in Get method
	MMS_TIMETYPE_RELATIVE	= 0,	// default
	MMS_TIMETYPE_ABSOLUTE   = 1
}MmsTimeType;


typedef struct
{
	MmsTimeType	type;
	unsigned int	time;
}MmsTimeStruct;


/**
 *	@brief	Represents MMS Sending Option Structure in framework.
 */
typedef struct
{
	bool					bReadReq;
	bool					bUseDeliveryCustomTime;
	MSG_PRIORITY_TYPE_T	priority;
	MmsTimeStruct			expiryTime;
	MmsTimeStruct			deliveryTime;
} MMS_SENDINGOPT_INFO_S;


/**
 *	@brief	Represents SMS Sending Option Structure in framework.
 */
typedef struct
{
	bool	bReplyPath;
} SMS_SENDINGOPT_INFO_S;


/**
 *	@brief	Represents MSG Sending Option Structure in framework.
 */
typedef struct
{
	bool bSetting;
	bool bDeliverReq;
	bool bKeepCopy;

	union
	{
		MMS_SENDINGOPT_INFO_S 	mmsSendOptInfo;
		SMS_SENDINGOPT_INFO_S 	smsSendOptInfo;
	} option;
} MSG_SENDINGOPT_INFO_S;


/**
 *	@brief	Represents a request in the application. \n
 *	Applications compose a request and send it to the framework via Message handle. \n
 *	This request ID is used to manage the request by the framework.
 */
typedef struct
{
	MSG_REQUEST_ID_T	reqId;	/**< Indicates the request ID, which is unique.
									When applications submit a request to the framework, this value will be set by the framework. */
	msg_message_t		msg;	/**< Indicates the message structure to be sent by applications. */
	MSG_SENDINGOPT_S	sendOpt;
} MSG_REQUEST_S;


/**
 *	@brief	Represents Address information list.
 */
typedef struct
{
	MSG_CONTACT_ID_T		contactId;			/**< The contact id of message common informatioin */
	MSG_ADDRESS_INFO_S		msgAddrInfo;		/**< The pointer to message common informatioin */
} MSG_THREAD_LIST_INDEX_S;


/**
 *	@brief	Represents Peer Count Info.
 */
typedef struct
{
	int totalCount;			/**< Indicates the total number of messages from the Peer. */
	int unReadCount;		/**< Indicates the unread messages from the Peer. */
	int smsMsgCount;		/**< Indicates the SMS messages from the Peer. */
	int mmsMsgCount;		/**< Indicates the MMS messages from the Peer. */
}MSG_THREAD_COUNT_INFO_S;


/**
 *	@brief	Represents the request information of Push Message (SI, SL).
 */
typedef struct
{
	MSG_PUSH_ACTION_T 	action;
	unsigned long			received;
	unsigned long			created;
	unsigned long			expires;
	char					id[MAX_WAPPUSH_ID_LEN];
	char					href[MAX_WAPPUSH_HREF_LEN];
	char					contents[MAX_WAPPUSH_CONTENTS_LEN];
} MSG_PUSH_MESSAGE_S;


/**
 *	@brief	Represents the request information of Push Message (CO).
 */
typedef struct
{
	int		invalObjectCnt;
	int		invalServiceCnt;
	char		invalObjectUrl[MAX_PUSH_CACHEOP_INVALID_OBJECT_MAX][MAX_PUSH_CACHEOP_MAX_URL_LEN];
	char		invalServiceUrl[MAX_PUSH_CACHEOP_INVALID_SERVICE_MAX][MAX_PUSH_CACHEOP_MAX_URL_LEN];
} MSG_PUSH_CACHEOP_S;


/**
 *	@brief	Represents the SyncML Message Information.
 */
typedef struct
{
	int					extId;
	int					pinCode;
	msg_message_t		msg;
}MSG_SYNCML_MESSAGE_S;


/**
 *	@brief	Represents the SyncML Message Data.
 */
 typedef struct
{
	MSG_SYNCML_MESSAGE_TYPE_T 	syncmlType;
	int                                             	pushBodyLen;
	char                                            	pushBody[MAX_WAPPUSH_CONTENTS_LEN];
	int 								wspHeaderLen;
	char 							wspHeader[MAX_WAPPUSH_CONTENTS_LEN];
}MSG_SYNCML_MESSAGE_DATA_S;


/**
 *	@brief	Represents the SyncML Message Data.
 */
 typedef struct
{
	char							pushHeader[MAX_WAPPUSH_CONTENTS_LEN];
	int 							pushBodyLen;
	char							pushBody[MAX_WAPPUSH_CONTENTS_LEN];
}MSG_LBS_MESSAGE_DATA_S;



/**
 *	@brief	Represents the Report Status Data.
 */
 typedef struct
{
	MSG_DELIVERY_REPORT_STATUS_T	deliveryStatus;		/**< Indicates the message ID of this message. */
	time_t							deliveryStatusTime;	/**< Indicates the display time related to the specific operation. */	//MAX_DISPLAY_TIME_LEN
	MSG_READ_REPORT_STATUS_T		readStatus;			/**< Indicates the message ID of this message. */
	time_t							readStatusTime;		/**< Indicates the display time related to the specific operation. */	//MAX_DISPLAY_TIME_LEN
}MSG_REPORT_STATUS_INFO_S;


/*==================================================================================================
                                         ENUMS
==================================================================================================*/

/**
 *	@brief	Represents the type of Message. More members maybe added if needed \n
 *	This enum is used as the value of MSG_MESSAGE_TYPE_T.
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
};


/**
 *	@brief	Represents the values of an error code. \n
 *	Success code is zero, but all error codes SHOULD be negative and smaller than MSG_ERROR_BEGIN. \n
 *	This enum is used as the value of MSG_ERROR_T.
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
 *	This enum is used as the value of MSG_PRIORITY_TYPE_T.
 */
enum _MSG_PRIORITY_TYPE_E
{
	MSG_MESSAGE_PRIORITY_LOW,		/**< Low priority */
	MSG_MESSAGE_PRIORITY_NORMAL,	/**< Normal priority */
	MSG_MESSAGE_PRIORITY_HIGH,		/**< High priority */
};


/**
 *	@brief	Represents the values of a network status. \n
 *	This enum is used as the value of MSG_NETWORK_STATUS_T.
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
 *	This enum is used as the value of MSG_ADDRESS_TYPE_T.
*/
enum _MSG_ADDRESS_TYPE_E
{
	MSG_ADDRESS_TYPE_UNKNOWN = 0,	/**< The address type is unknown. */
	MSG_ADDRESS_TYPE_PLMN,			/**< The address type is for a phone number like +1012345678. */
	MSG_ADDRESS_TYPE_EMAIL,			/**< The address type is for an email address like abc@example.email. */
};


/**
 *	@brief	Represents the values of a recipient type. \n
 *	This enum is used as the value of MSG_RECIPIENT_TYPE_T.
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
 *	This enum is used as the value of MSG_DIRECTION_TYPE_T.
 */
enum _MSG_DIRECTION_TYPE_E
{
	MSG_DIRECTION_TYPE_MO = 0,		/**< The direction type is for mobile originated */
	MSG_DIRECTION_TYPE_MT,			/**< The direction type is for mobile terminated */
};


/**
 *	@brief	Represents the values of a string encoding type. \n
 *	This enum is used as the value of MSG_ENCODE_TYPE_T.
 */
enum _MSG_ENCODE_TYPE_E
{
	MSG_ENCODE_GSM7BIT = 0,	/**< The string encoding type is GSM7BIT */
	MSG_ENCODE_8BIT,		/**< The string encoding type is 8 BIT */
	MSG_ENCODE_UCS2,		/**< The string encoding type is UCS2 */
	MSG_ENCODE_AUTO,		/**< The string encoding type is AUTO */
};


/**
 *	@brief	Represents the action type of Push Message. \n
 *	This enum is used as the value of MSG_PUSH_ACTION_T.
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
 *	This enum is used as the value of MSG_SYNCML_MESSAGE_TYPE_T.
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
 *	This enum is used as the value of MSG_DELIVERY_REPORT_STATUS_T.
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
 *	This enum is used as the value of MSG_READ_REPORT_STATUS_T.
*/
enum _MSG_READ_REPORT_STATUS_E
 {
	 MSG_READ_REPORT_NONE			= 	-1,	  /**< Indicates the status unavailable */
	 MSG_READ_REPORT_IS_READ	 	= 	0,	  /**< Indicates the message is read */
	 MSG_READ_REPORT_IS_DELETED  	= 	1	  /**< Indicates the message is deleted */
 };

/**
 *	@}
 */

#endif // MSG_TYPES_H

