/*
* Copyright 2012  Samsung Electronics Co., Ltd
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

#include "msg_types.h"

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_COMMON_TYPES	Messaging Common Types
 *	@{
 */

/*==================================================================================================
                                         TYPES
==================================================================================================*/

/**
 *	@brief	Defines the enabled MMS version
 */
#define	MMS_V1_2								// MMS Version : MMS_V1_0 / MMS_V1_1 / MMS_V1_2

/**
 *	@brief	Defines the enabled DRM support
 */
#define	__SUPPORT_DRM__

/**
 *	@brief	Defines the enabled JAVA MMS Application Id
 */
#define FEATURE_JAVA_MMS


/**
 *	@brief	Defines message struct handle.
 */

typedef struct _msg_struct{
	int type;
	void *data;
}msg_struct_s;

/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/

/**
 *	@brief	Represents address information.
 */

typedef struct
{
	msg_address_type_t		addressType;													/**< The type of an address in case of an Email or a mobile phone */
	msg_recipient_type_t	recipientType;													/**< The type of recipient address in case of To, Cc, and Bcc */
	msg_contact_id_t			contactId;															/**< The contact ID of address */
	char										addressVal[MAX_ADDRESS_VAL_LEN+1];		/**< The actual value of an address */
	char										displayName[MAX_DISPLAY_NAME_LEN+1];	/**< The display name of an address */
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
	msg_priority_type_t	priority;
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
	msg_struct_t mmsSendOpt;
	msg_struct_t smsSendOpt;

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
	msg_priority_type_t	priority;
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
	msg_request_id_t	reqId;	/**< Indicates the request ID, which is unique.
									When applications submit a request to the framework, this value will be set by the framework. */
	msg_struct_t		msg;	/**< Indicates the message structure to be sent by applications. */
	msg_struct_t		sendOpt;
} MSG_REQUEST_S;


/**
 *	@brief	Represents Address information list.
 */

typedef struct
{
	msg_contact_id_t		contactId;			/**< The contact id of message common informatioin */
	MSG_ADDRESS_INFO_S		msgAddrInfo;		/**< The pointer to message common informatioin */
} MSG_THREAD_LIST_INDEX_S;

typedef struct
{
	msg_contact_id_t		contactId;			/**< The contact id of message common informatioin */
	msg_struct_t			msgAddrInfo;		/**< The pointer to message common informatioin */
} MSG_THREAD_LIST_INDEX_INFO_S;


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
	msg_push_action_t 	action;
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
	msg_struct_t		msg;
}MSG_SYNCML_MESSAGE_S;


/**
 *	@brief	Represents the SyncML Message Data.
 */
 typedef struct
{
	msg_syncml_message_type_t 	syncmlType;
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


typedef struct
{
	char							pushHeader[MAX_WAPPUSH_CONTENTS_LEN];
	int 							pushBodyLen;
	char							pushBody[MAX_WAPPUSH_CONTENTS_LEN];
	char							pushAppId[MAX_WAPPUSH_ID_LEN];
}MSG_PUSH_MESSAGE_DATA_S;

/**
 *	@brief	Represents the Report Status Data.
 */
typedef struct
{
	char addressVal[MAX_ADDRESS_VAL_LEN+1];
	int type;
	int status;
	time_t statusTime;
} MSG_REPORT_STATUS_INFO_S;


typedef struct
{
	char contentType[MAX_WAPPUSH_CONTENT_TYPE_LEN];
	char appId[MAX_WAPPUSH_ID_LEN];
	char pkgName[MSG_FILEPATH_LEN_MAX];
	bool bLaunch;
}MSG_PUSH_EVENT_INFO_S;

/**
 *	@}
 */

#endif // MSG_TYPES_H

