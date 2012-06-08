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

#ifndef MSG_CMD_TYPES_H
#define MSG_CMD_TYPES_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgTypes.h"


/*==================================================================================================
                                         DEFINES
==================================================================================================*/
#define MAX_COOKIE_LEN 20


/*==================================================================================================
                                         TYPES
==================================================================================================*/
typedef unsigned int MSG_CMD_TYPE_T;

typedef unsigned int MSG_EVENT_TYPE_T;


/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/
typedef struct _MSG_CMD_S
{
	MSG_CMD_TYPE_T 	cmdType;
	char				cmdCookie[MAX_COOKIE_LEN];
	char 				cmdData[2];
} MSG_CMD_S;


typedef struct _MSG_EVENT_S
{
	MSG_EVENT_TYPE_T 	eventType;
	MSG_ERROR_T			result;
	char					data[2];
} MSG_EVENT_S;


/*==================================================================================================
                                         ENUMS
==================================================================================================*/

enum _MSG_CMD_TYPE_E
{
	MSG_CMD_OPEN_HANDLE = 0,
	MSG_CMD_CLOSE_HANDLE,
	MSG_CMD_GET_STORAGELIST,
	MSG_CMD_ADD_MSG,
	MSG_CMD_ADD_SYNCML_MSG,

// 05
	MSG_CMD_UPDATE_MSG,
	MSG_CMD_UPDATE_READ,
	MSG_CMD_UPDATE_PROTECTED,
	MSG_CMD_DELETE_MSG,
	MSG_CMD_DELALL_MSGINFOLDER,

// 10
	MSG_CMD_MOVE_MSGTOFOLDER,
	MSG_CMD_MOVE_MSGTOSTORAGE,
	MSG_CMD_COUNT_MSG,
	MSG_CMD_GET_MSG,
	MSG_CMD_GET_FOLDERVIEWLIST,

// 15
	MSG_CMD_ADD_FOLDER,
	MSG_CMD_UPDATE_FOLDER,
	MSG_CMD_DELETE_FOLDER,
	MSG_CMD_GET_FOLDERLIST,
	MSG_CMD_SET_CONFIG,

// 20
	MSG_CMD_GET_CONFIG,
	MSG_CMD_GET_MSG_TYPE,
	MSG_CMD_SUBMIT_REQ,
	MSG_CMD_CANCEL_REQ,
	MSG_CMD_REG_SENT_STATUS_CB,

// 25
	MSG_CMD_REG_STORAGE_CHANGE_CB,
	MSG_CMD_REG_INCOMING_MSG_CB,
	MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB,
	MSG_CMD_REG_INCOMING_SYNCML_MSG_CB,
	MSG_CMD_REG_INCOMING_LBS_MSG_CB,

// 30
	MSG_CMD_PLG_SENT_STATUS_CNF,
	MSG_CMD_PLG_STORAGE_CHANGE_IND,
	MSG_CMD_PLG_INCOMING_MSG_IND,
	MSG_CMD_PLG_INCOMING_MMS_CONF,
	MSG_CMD_PLG_INCOMING_SYNCML_IND,

// 35
	MSG_CMD_PLG_INCOMING_LBS_IND,
	MSG_CMD_PLG_INIT_SIM_BY_SAT,
	MSG_CMD_GET_THREADVIEWLIST,
	MSG_CMD_GET_CONVERSATIONVIEWLIST,
	MSG_CMD_DELETE_THREADMESSAGELIST,

// 40
	MSG_CMD_GET_CONTACT_COUNT,
	MSG_CMD_GET_QUICKPANEL_DATA,
	MSG_CMD_COUNT_BY_MSGTYPE,
	MSG_CMD_RESET_DB,
	MSG_CMD_GET_MEMSIZE,

// 45
	MSG_CMD_BACKUP_MESSAGE,
	MSG_CMD_RESTORE_MESSAGE,
	MSG_CMD_UPDATE_THREAD_READ,
	MSG_CMD_REG_SYNCML_MSG_OPERATION_CB,
	MSG_CMD_SYNCML_OPERATION,

// 50
	MSG_CMD_GET_REPORT_STATUS,

// end of MSG_CMD; new CMD should be defined before MSG_CMD_NUM
	MSG_CMD_NUM
};


enum _MSG_EVENT_TYPE_E
{
	MSG_EVENT_OPEN_HANDLE = 0,
	MSG_EVENT_CLOSE_HANDLE,
	MSG_EVENT_GET_STORAGELIST,
	MSG_EVENT_ADD_MSG,
	MSG_EVENT_ADD_SYNCML_MSG,

// 5
	MSG_EVENT_UPDATE_MSG,
	MSG_EVENT_UPDATE_READ,
	MSG_EVENT_UPDATE_PROTECTED,
	MSG_EVENT_DELETE_MSG,
	MSG_EVENT_DELALL_MSGINFOLDER,

// 10
	MSG_EVENT_MOVE_MSGTOFOLDER,
	MSG_EVENT_MOVE_MSGTOSTORAGE,
	MSG_EVENT_COUNT_MSG,
	MSG_EVENT_GET_MSG,
	MSG_EVENT_GET_FOLDERVIEWLIST,

// 15
	MSG_EVENT_ADD_FOLDER,
	MSG_EVENT_UPDATE_FOLDER,
	MSG_EVENT_DELETE_FOLDER,
	MSG_EVENT_GET_FOLDERLIST,
	MSG_EVENT_SET_CONFIG,

// 20
	MSG_EVENT_GET_CONFIG,
	MSG_EVENT_GET_MSG_TYPE,
	MSG_EVENT_SUBMIT_REQ,
	MSG_EVENT_CANCEL_REQ,
	MSG_EVENT_REG_SENT_STATUS_CB,

// 25
	MSG_EVENT_REG_INCOMING_MSG_CB,
	MSG_EVENT_REG_INCOMING_MMS_CONF_MSG_CB,
	MSG_EVENT_REG_INCOMING_SYNCML_MSG_CB,
	MSG_EVENT_REG_INCOMING_LBS_MSG_CB,
	MSG_EVENT_REG_STORAGE_CHANGE_CB,

// 30
	MSG_EVENT_PLG_SENT_STATUS_CNF,
	MSG_EVENT_PLG_STORAGE_CHANGE_IND,
	MSG_EVENT_PLG_INCOMING_MSG_IND,
	MSG_EVENT_PLG_INCOMING_MMS_CONF,
	MSG_EVENT_PLG_INCOMING_SYNCML_MSG_IND,

// 35
	MSG_EVENT_PLG_INCOMING_LBS_MSG_IND,
	MSG_EVENT_PLG_INIT_SIM_BY_SAT,
	MSG_EVENT_GET_THREADVIEWLIST,
	MSG_EVENT_GET_CONVERSATIONVIEWLIST,
	MSG_EVENT_DELETE_THREADMESSAGELIST,

// 40
	MSG_EVENT_GET_CONTACT_COUNT,
	MSG_EVENT_GET_QUICKPANEL_DATA,
	MSG_EVENT_COUNT_BY_MSGTYPE,
	MSG_EVENT_RESET_DB,
	MSG_EVENT_GET_MEMSIZE,

// 45
	MSG_EVENT_BACKUP_MESSAGE, 
	MSG_EVENT_RESTORE_MESSAGE, 
	MSG_EVENT_UPDATE_THREAD_READ,
	MSG_EVENT_REG_SYNCML_MSG_OPERATION_CB,
	MSG_EVENT_SYNCML_OPERATION,

// 50
	MSG_EVENT_GET_REPORT_STATUS,

// end of MSG_EVENT; new EVENT should be defined before MSG_EVENT_NUM
	MSG_EVENT_NUM
};

#endif // MSG_CMD_TYPES_H

