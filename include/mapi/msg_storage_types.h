/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef MSG_STORAGE_TYPES_H_
#define MSG_STORAGE_TYPES_H_

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "msg_types.h"

/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/

/**
 * @addtogroup MSG_SERVICE_FRAMEWORK_STORAGE_MODULE
 * @{
 */


/**
 *	@brief  The structure type that represents message ID list.
 */
typedef struct {
	int					nCount;				/**< The count of message ID information */
	msg_message_id_t	*msgIdList;			/**< The pointer to message ID information */
}msg_id_list_s;


/*==================================================================================================
                                         TYPES
==================================================================================================*/

/**
 *	@brief  The structure type that represents a folder type. \n
 *	The values for this type SHOULD be in @ref _MSG_FOLDER_TYPE_E.
 */
typedef unsigned char msg_folder_type_t;


/**
 *	@brief  The structure type that represents a sort type. \n
 *	The values for this type SHOULD be in @ref _MSG_SORT_TYPE_E.
 */
typedef unsigned char msg_sort_type_t;


/**
 *	@brief  The structure type that represents a saved SIM message ID.
 */
typedef signed short msg_sim_id_t;

/**
 *	@brief  The structure type that represents a Saved SIM slot ID.
 */
typedef int msg_sim_slot_id_t;


/**
 *	@brief  The structure type that represents a message type for quick panel. \n
 *	The values for this type SHOULD be in @ref _MSG_QUICKPANEL_TYPE_E.
 */
typedef unsigned char msg_quickpanel_type_t;


/**
 *	@brief  The structure type that represents a storage change CB type. \n
 *	The values for this type SHOULD be in @ref _MSG_STORAGE_CHANGE_TYPE_E.
 */
typedef unsigned char msg_storage_change_type_t;


/** @brief	Called when the database of message framework is changed.
 *          Applications SHOULD implement this callback function and register it into Message handle.
 *	        For how to register this callback function, please refer to msg_reg_storage_change_callback().
 *	        The callback function runs in the application process, not in the framework process.
 * @param[in]  handle      The Message handle.
 * @param[in]  user_param  A pointer to user data
 */
typedef void (*msg_storage_change_cb)(msg_handle_t handle, msg_storage_change_type_t storageChangeType, msg_id_list_s *pMsgIdList, void *user_param);


/*==================================================================================================
                                         ENUMS
==================================================================================================*/

/**
 *	@brief Enumeration for the values of a storage type. \n
 *	       This enum is used as the value of msg_storage_id_t.
 */
enum _MSG_STORAGE_ID_E {
	MSG_STORAGE_UNKNOWN = 0,		/**< Storage ID is unknown. */
	MSG_STORAGE_PHONE,			    /**< Storage ID is Phone. */
	MSG_STORAGE_SIM,				/**< Storage ID is SIM card in slot 1. */
	MSG_STORAGE_SIM2,				/**< Storage ID is SIM card in slot 2. */
};


/**
 *	@brief  Enumeration for the values of a storage type. \n
 *	        This enum is used as the value of msg_folder_id_t.
 */
enum _MSG_FOLDER_ID_E {
	MSG_IOSBOX_ID = -1,			    /**< Indicates INBOX, OUTBOX and SENTBOX group folder ID. (Only for search option.) */
	MSG_ALLBOX_ID = 0,				/**< Indicates INBOX, OUTBOX, SENTBOX and DRAFTBOX group folder ID. (Only for search option.) */
	MSG_INBOX_ID = 1,				/**< Indicates INBOX folder ID */
	MSG_OUTBOX_ID = 2,				/**< Indicates OUTBOX folder ID */
	MSG_SENTBOX_ID = 3,				/**< Indicates SENTBOX folder ID */
	MSG_DRAFT_ID = 4,				/**< Indicates DRAFT folder ID */
	MSG_CBMSGBOX_ID = 5,			/**< Indicates CALLBACK message folder ID */
	MSG_SPAMBOX_ID = 6,				/**< Indicates SPAM message folder ID */
	MSG_MAX_FOLDER_ID				/**< Indicates MAX folder ID. Folder ID to be accessed should be smaller than this */
};


/**
 *	@brief  Enumeration for the values of a folder type. \n
 *	        This enum is used as the value of msg_folder_type_t.
 */
enum _MSG_FOLDER_TYPE_E {
	MSG_FOLDER_TYPE_INBOX = 1,		/**< Inbox folder */
	MSG_FOLDER_TYPE_OUTBOX,		    /**< Outbox folder */
	MSG_FOLDER_TYPE_DRAFT, 		    /**< Draft folder */
	MSG_FOLDER_TYPE_SPAMBOX, 		/**< Spambox folder */
	MSG_FOLDER_TYPE_USER_DEF		/**< Folder which is created by a user */
};


/**
 *	@brief  Enumeration for the values of a sort type. \n
 *	        This enum is used as the value of msg_sort_type_t.
 */
enum _MSG_SORT_TYPE_E {
	MSG_SORT_BY_UNKNOWN = 0,			/**< Unknown sort type */
	MSG_SORT_BY_DISPLAY_FROM,		    /**< Sort by display from */
	MSG_SORT_BY_DISPLAY_TO,			    /**< Sort by display to */
	MSG_SORT_BY_DISPLAY_TIME,			/**< Sort by display time */
	MSG_SORT_BY_MSG_TYPE,				/**< Sort by msg type */
	MSG_SORT_BY_READ_STATUS,			/**< Sort by read status */
	MSG_SORT_BY_STORAGE_TYPE,		    /**< Sort by storage type */
	MSG_SORT_BY_THREAD_NAME,			/**< Sort by name for thread view*/
	MSG_SORT_BY_THREAD_DATE,			/**< Sort by date for thread view*/
	MSG_SORT_BY_THREAD_COUNT,		    /**< Sort by count for thread view*/
};


/**
 *	@brief  Enumeration for the values of a message type for quick panel. \n
 *	        This enum is used as the value of msg_quickpanel_type_t.
 */
enum _MSG_QUICKPANEL_TYPE_E {
	MSG_QUICKPANEL_SMS = 0,				/**< Quickpanel SMS */
	MSG_QUICKPANEL_MMS,					/**< Quickpanel MMS */
	MSG_QUICKPANEL_DELIVER_REP,			/**< Quickpanel delivery report */
	MSG_QUICKPANEL_READ_REP,			/**< Quickpanel message read report */
	MSG_QUICKPANEL_VOICEMAIL,			/**< Quickpanel voicemail */
	MSG_QUICKPANEL_MMS_NOTI,			/**< Quickpanel MMS notification */
};


/**
 *	@brief  Enumeration for the values of a count limit for each mailbox.
 */
enum _MSG_COUNT_LIMIT_MAILBOX_TYPE_E {
	MSG_COUNT_LIMIT_INBOX_TYPE,				/**< Message count limit of inbox folder*/
	MSG_COUNT_LIMIT_OUTBOX_TYPE,			/**< Message count limit of outbox folder*/
	MSG_COUNT_LIMIT_SENTBOX_TYPE,			/**< Message count limit of sentbox folder*/
	MSG_COUNT_LIMIT_DRAFTBOX_TYPE,			/**< Message count limit of draftbox folder*/
	MSG_COUNT_LIMIT_CBMSGBOX_TYPE,			/**< Message count limit of call-back-message-box folder */
	MSG_COUNT_LIMIT_MAILBOX_TYPE_MAX,		/**< Indicates the maximum number of types of folder message count limits */
};


/**
 *	@brief  Enumeration for the values of a count limit for each message type.
 */
enum _MSG_COUNT_LIMIT_MSG_TYPE_E {
	MSG_COUNT_LIMIT_SMS_TYPE,				/**< Message count limit for SMS */
	MSG_COUNT_LIMIT_MMS_TYPE,				/**< Message count limit for MMS */
	MSG_COUNT_LIMIT_CB_TYPE,				/**< Message count limit for callback messages */
	MSG_COUNT_LIMIT_WAPPUSH_TYPE,			/**< Message count limit for wappush messages */
	MSG_COUNT_LIMIT_PROVISION_TYPE,			/**< Message count limit for provision messages */
	MSG_COUNT_LIMIT_MSG_TYPE_MAX,			/**< Indicates maximum number of types of message count limits */
};


/**
 *	@brief  Enumeration for the values of a storage change type. \n
 *	        This enum is used with storage change callback functions.
 */
enum _MSG_STORAGE_CHANGE_TYPE_E {
	// msg data
	MSG_STORAGE_CHANGE_INSERT = 1,			/**< Indicates storage change by an INSERT operation*/
	MSG_STORAGE_CHANGE_UPDATE,				/**< Indicates storage change by an UPDATE operation*/
	MSG_STORAGE_CHANGE_DELETE,				/**< Indicates storage change by a DELETE operation*/

	// thread data
	MSG_STORAGE_CHANGE_CONTACT,				/**< Indicates storage change by a CHANGE CONTACT operation*/
};

/**
 * @}
 */

#endif /* MSG_STORAGE_TYPES_H_ */
