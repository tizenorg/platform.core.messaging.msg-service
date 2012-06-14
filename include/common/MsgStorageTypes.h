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

#ifndef MSG_STORAGE_TYPES_H
#define MSG_STORAGE_TYPES_H


/**
 *	@file 		MsgStorageTypes.h
 *	@brief 		Defines transport types of messaging framework
 *	@version 	1.0
 */


/**
 *	@section		Introduction
 *	- Introduction : Overview on Messaging Storage Types
 *	@section		Program
 *	- Program : Messaging Storage Types Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <time.h>

#include "MsgTypes.h"


/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_STORAGE_TYPES	Messaging Storage Types
 *	@{
 */

/*==================================================================================================
                                    DEFINES
==================================================================================================*/
#define MAX_FOLDER_NAME_SIZE		20
#define MAX_SEGMENT_NUM			12


/*==================================================================================================
                                         TYPES
==================================================================================================*/

/**
 *	@brief	Represents a folder type. \n
 *	The values for this type SHOULD be in _MSG_FOLDER_TYPE_E.
 */
typedef unsigned char MSG_FOLDER_TYPE_T;


/**
 *	@brief	Represents a sort type. \n
 *	The values for this type SHOULD be in \ref _MSG_SORT_TYPE_E.
 */
typedef unsigned char MSG_SORT_TYPE_T;


/**
 *	@brief	Represents a Saved SIM message ID.
 */
typedef signed short MSG_SIM_ID_T;


/**
 *	@brief	Represents a message type for quick panel. \n
 *	The values for this type SHOULD be in \ref _MSG_QUICKPANEL_TYPE_E.
 */
typedef unsigned char MSG_QUICKPANEL_TYPE_T;


/**
 *	@brief	Represents a storage change CB type. \n
 *	The values for this type SHOULD be in \ref _MSG_STORAGE_CHANGE_TYPE_E.
 */
typedef unsigned char MSG_STORAGE_CHANGE_TYPE_T;


/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/
/**
 *	@brief	Represents the count of read/unread messages.
 */
typedef struct
{
	int	nReadCnt;		/**< The count of read messages */
	int	nUnreadCnt;		/**< The count of unread messages */
	int	nSms;			/**< The count of sms type messages */
	int	nMms;			/**< The count of mms type messages */
} MSG_COUNT_INFO_S;


/**
 *	@brief	Represents message thread view information list.
 */
typedef struct
{
	int					nCount;				/**< The count of message thread informatioin */
	msg_thread_view_t	*msgThreadInfo;		/**< The pointer to message thread informatioin */
} MSG_THREAD_VIEW_LIST_S;


/**
 *	@brief	Represents message information list.
 */
typedef struct
{
	int					nCount;				/**< The count of message informatioin */
	msg_message_t		*msgInfo;		/**< The pointer to message informatioin */
}MSG_LIST_S;


/**
 *	@brief	Represents message id list.
 */
typedef struct
{
	int					nCount;				/**< The count of message id informatioin */
	MSG_MESSAGE_ID_T	*msgIdList;			/**< The pointer to message id informatioin */
}MSG_MSGID_LIST_S;


/**
 *	@brief	Represents a reject message information.
 */
typedef struct
{
	MSG_MESSAGE_ID_T		msgId;      		/**< Indicates the unique message ID. */
	char						msgText[MAX_MSG_TEXT_LEN+1];	/**< Indicates the message text. */
	time_t					displayTime;
} MSG_REJECT_MSG_INFO_S;


/**
 *	@brief	Represents reject message list.
 */
typedef struct
{
	int						nCount;				/**< The count of reject message informatioin */
	MSG_REJECT_MSG_INFO_S	*rejectMsgInfo;		/**< The pointer to reject message informatioin */
}MSG_REJECT_MSG_LIST_S;


/**
 *	@brief	Represents folder information.
 */
typedef struct
{
	MSG_FOLDER_ID_T 	folderId;			/**< Indicates the unique folder ID. */
	char 				folderName[MAX_FOLDER_NAME_SIZE+1];	/**< Indicates the name of the folder. */
	MSG_FOLDER_TYPE_T	folderType;		/**< Indicates the folder type. */
} MSG_FOLDER_INFO_S;


/**
 *	@brief	Represents a folder list.
 */
typedef struct
{
	int					nCount;			/**< The count of folder information */
	MSG_FOLDER_INFO_S	*folderInfo;		/**< The pointer to folder information */
} MSG_FOLDER_LIST_S;


/**
 *	@brief	Represents contact information.
 */
typedef struct
{
	MSG_CONTACT_ID_T	contactId;							/**< Indicates the unique contact ID. */
	char 				displayName[MAX_DISPLAY_NAME_LEN+1];	/**< Indicates the display name of contact. */
	char 				firstName[MAX_DISPLAY_NAME_LEN+1];		/**< Indicates the first name of contact. */
	char 				lastName[MAX_DISPLAY_NAME_LEN+1];		/**< Indicates the last name of contact. */
	char 				imagePath[MAX_IMAGE_PATH_LEN+1];		/**< Indicates the image path of contact. */
} MSG_CONTACT_INFO_S;


/**
 *	@brief	Represents a sort rule. \n
 *	The sort rule structure includes a sort type and a sort order. \n
 *	Applications can use the sort rule when querying messages.
 */
typedef struct
{
	MSG_SORT_TYPE_T	sortType;		/**< Indicates the sort type */
	bool					bAscending;		/**< Indicates the sort order which is ascending or descending */
}MSG_SORT_RULE_S;


/**
 *	@brief	Represents SIM ID List.
 */
typedef struct
{
	unsigned int		count;										/**< The total number of SIM Msg ID*/
	MSG_SIM_ID_T	simId[MAX_SEGMENT_NUM];						/**< The SIM Msg ID List */
} SMS_SIM_ID_LIST_S;


/**
 *	@brief	Represents recipien list information.
 */
typedef struct
{
	int						recipientCnt;
	MSG_ADDRESS_INFO_S*	recipientAddr;
} MSG_RECIPIENTS_LIST_S;


/**
 *	@brief	Represents search condition values.
 */
typedef struct
{
	MSG_FOLDER_ID_T		folderId;
	MSG_MESSAGE_TYPE_T		msgType;
	char						*pAddressVal;
	char						*pSearchVal;
	int						reserved;
} MSG_SEARCH_CONDITION_S;


/** @brief	Prototype of the function that will be called when the database of message framework is changed.
 *	Applications SHOULD implement this callback function and register it into Message handle.
 *	For how to register this callback function, please refer to msg_reg_storage_change_callback.
 *	The callback function runs in the application process, not in the framework process.
 *	@param[in]	handle is Message handle.
 *	@param[in]	user_param is a pointer to user data.
 *	@return	void
 */
typedef void (*msg_storage_change_cb)(MSG_HANDLE_T handle, MSG_STORAGE_CHANGE_TYPE_T storageChangeType, MSG_MSGID_LIST_S *pMsgIdList, void *user_param);


/*==================================================================================================
                                         ENUMS
==================================================================================================*/

/**
 *	@brief	Represents the values of a storage type. \n
 *	This enum is used as the value of MSG_STORAGE_ID_T.
 */
enum _MSG_STORAGE_ID_E
{
	MSG_STORAGE_UNKNOWN = 0,		/**< Storage Id is unknown. */
	MSG_STORAGE_PHONE,			/**< Storage Id is Phone. */
	MSG_STORAGE_SIM,				/**< Storage Id is SIM card. */
};


/**
 *	@brief	Represents the values of a storage type. \n
 *	This enum is used as the value of MSG_FOLDER_ID_T.
 */
enum _MSG_FOLDER_ID_E
{
	MSG_IOSBOX_ID = -1,			/**< Indicates INBOX, OUTBOX and SENTBOX group folder id. (Only for search option.) */
	MSG_ALLBOX_ID = 0,				/**< Indicates INBOX, OUTBOX, SENTBOX and DRAFTBOX group folder id. (Only for search option.) */
	MSG_INBOX_ID = 1,
	MSG_OUTBOX_ID = 2,
	MSG_SENTBOX_ID = 3,
	MSG_DRAFT_ID = 4,
	MSG_CBMSGBOX_ID = 5,

	// new folder should be placed here

	MSG_MAX_FOLDER_ID
};


/**
 *	@brief	Represents the values of a folder type. \n
 *	This enum is used as the value of MSG_FOLDER_TYPE_T.
 */
enum _MSG_FOLDER_TYPE_E
{
	MSG_FOLDER_TYPE_INBOX = 1,		/**< Inbox folder */
	MSG_FOLDER_TYPE_OUTBOX,		/**< Outbox folder */
	MSG_FOLDER_TYPE_DRAFT, 		/**< Draft folder */
	MSG_FOLDER_TYPE_USER_DEF		/**< Folder which is created by a user */
};


/**
 *	@brief	Represents the values of a sort type. \n
 *	This enum is used as the value of MSG_SORT_TYPE_T.
 */
enum _MSG_SORT_TYPE_E
{
	MSG_SORT_BY_UNKNOWN = 0,			/**< Unknown sort type */
	MSG_SORT_BY_DISPLAY_FROM,		/**< Sort by display from */
	MSG_SORT_BY_DISPLAY_TO,			/**< Sort by display to */
	MSG_SORT_BY_DISPLAY_TIME,			/**< Sort by display time */
	MSG_SORT_BY_MSG_TYPE,				/**< Sort by msg type */
	MSG_SORT_BY_READ_STATUS,			/**< Sort by read status */
	MSG_SORT_BY_STORAGE_TYPE,		/**< Sort by storage type */
	MSG_SORT_BY_THREAD_NAME,			/**< Sort by name for thread view*/
	MSG_SORT_BY_THREAD_DATE,			/**< Sort by date for thread view*/
	MSG_SORT_BY_THREAD_COUNT,		/**< Sort by count for thread view*/
};


/**
 *	@brief	Represents the values of a message type for quick panel. \n
 *	This enum is used as the value of MSG_QUICKPANEL_TYPE_T.
 */
enum _MSG_QUICKPANEL_TYPE_E
{
	MSG_QUICKPANEL_SMS = 0,
	MSG_QUICKPANEL_MMS,
	MSG_QUICKPANEL_DELIVER_REP,
	MSG_QUICKPANEL_READ_REP,
	MSG_QUICKPANEL_VOICEMAIL,
	MSG_QUICKPANEL_MMS_NOTI,
};

/**
 *	@}
 */
enum _MSG_COUNT_LIMIT_MAILBOX_TYPE_E
{
	MSG_COUNT_LIMIT_INBOX_TYPE,
	MSG_COUNT_LIMIT_OUTBOX_TYPE,
	MSG_COUNT_LIMIT_SENTBOX_TYPE,
	MSG_COUNT_LIMIT_DRAFTBOX_TYPE,
	MSG_COUNT_LIMIT_CBMSGBOX_TYPE,
	MSG_COUNT_LIMIT_MAILBOX_TYPE_MAX,
};

enum _MSG_COUNT_LIMIT_MSG_TYPE_E
{
	MSG_COUNT_LIMIT_SMS_TYPE,
	MSG_COUNT_LIMIT_MMS_TYPE,
	MSG_COUNT_LIMIT_CB_TYPE,
	MSG_COUNT_LIMIT_WAPPUSH_TYPE,
	MSG_COUNT_LIMIT_PROVISION_TYPE,
	MSG_COUNT_LIMIT_MSG_TYPE_MAX,
};

enum _MSG_STORAGE_CHANGE_TYPE_E
{
	// msg data
	MSG_STORAGE_CHANGE_INSERT = 1,
	MSG_STORAGE_CHANGE_UPDATE,
	MSG_STORAGE_CHANGE_DELETE,

	// thread data
	MSG_STORAGE_CHANGE_CONTACT,
};

#endif // MSG_STORAGE_TYPES_H

