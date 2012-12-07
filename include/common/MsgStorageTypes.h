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

#include "msg_storage_types.h"


/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_STORAGE_TYPES	Messaging Storage Types
 *	@{
 */
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
 *	@brief	Represents a reject message information.
 */
typedef struct
{
	msg_message_id_t		msgId;      		/**< Indicates the unique message ID. */
	char						msgText[MAX_MSG_TEXT_LEN+1];	/**< Indicates the message text. */
	time_t					displayTime;
} MSG_REJECT_MSG_INFO_S;


/**
 *	@brief	Represents folder information.
 */
typedef struct
{
	msg_folder_id_t 	folderId;			/**< Indicates the unique folder ID. */
	char 				folderName[MAX_FOLDER_NAME_SIZE+1];	/**< Indicates the name of the folder. */
	msg_folder_type_t	folderType;		/**< Indicates the folder type. */
} MSG_FOLDER_INFO_S;


/**
 *	@brief	Represents contact information.
 */
typedef struct
{
	msg_contact_id_t	contactId;							/**< Indicates the unique contact ID. */
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
	msg_sort_type_t	sortType;		/**< Indicates the sort type */
	bool					bAscending;		/**< Indicates the sort order which is ascending or descending */
}MSG_SORT_RULE_S;


/**
 *	@brief	Represents SIM ID List.
 */
typedef struct
{
	unsigned int		count;										/**< The total number of SIM Msg ID*/
	msg_sim_id_t	simId[MAX_SEGMENT_NUM];						/**< The SIM Msg ID List */
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
	msg_folder_id_t		folderId;
	msg_message_type_t		msgType;
	char						*pAddressVal;
	char						*pSearchVal;
	int						reserved;
} MSG_SEARCH_CONDITION_S;


typedef struct
{
	int appcode;
	char appid[MAX_WAPPUSH_ID_LEN];
} PUSH_APPLICATION_INFO_S;

#endif // MSG_STORAGE_TYPES_H

