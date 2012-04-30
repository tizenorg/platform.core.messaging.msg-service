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

#include "MsgDebug.h"
#include <sys/syscall.h>


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
const char * MsgDbgCmdStr(MSG_CMD_TYPE_T cmdType)
{
	switch ( cmdType )
	{
// 0
		case MSG_CMD_OPEN_HANDLE:
			return "MSG_CMD_OPEN_HANDLE";
		case MSG_CMD_CLOSE_HANDLE:
			return "MSG_CMD_CLOSE_HANDLE";
		case MSG_CMD_GET_STORAGELIST:
			return "MSG_CMD_GET_STORAGELIST";
		case MSG_CMD_ADD_MSG:
			return "MSG_CMD_ADD_MSG";
		case MSG_CMD_ADD_SYNCML_MSG:
			return "MSG_CMD_ADD_SYNCML_MSG";

// 5
		case MSG_CMD_UPDATE_MSG:
			return "MSG_CMD_UPDATE_MSG";
		case MSG_CMD_UPDATE_READ:
			return "MSG_CMD_UPDATE_READ";
		case MSG_CMD_UPDATE_PROTECTED:
			return "MSG_CMD_UPDATE_PROTECTED";
		case MSG_CMD_DELETE_MSG:
			return "MSG_CMD_DELETE_MSG";
		case MSG_CMD_DELALL_MSGINFOLDER:
			return "MSG_CMD_DELALL_MSGINFOLDER";

// 10
		case MSG_CMD_MOVE_MSGTOFOLDER:
			return "MSG_CMD_MOVE_MSGTOFOLDER";
		case MSG_CMD_MOVE_MSGTOSTORAGE:
			return "MSG_CMD_MOVE_MSGTOSTORAGE";
		case MSG_CMD_COUNT_MSG:
			return "MSG_CMD_COUNT_MSG";
		case MSG_CMD_GET_MSG:
			return "MSG_CMD_GET_MSG";
		case MSG_CMD_GET_FOLDERVIEWLIST:
			return "MSG_CMD_GET_FOLDERVIEWLIST";

// 15
		case MSG_CMD_ADD_FOLDER:
			return "MSG_CMD_ADD_FOLDER";
		case MSG_CMD_UPDATE_FOLDER:
			return "MSG_CMD_UPDATE_FOLDER";
		case MSG_CMD_DELETE_FOLDER:
			return "MSG_CMD_DELETE_FOLDER";
		case MSG_CMD_GET_FOLDERLIST:
			return "MSG_CMD_GET_FOLDERLIST";
		case MSG_CMD_SET_CONFIG:
			return "MSG_CMD_SET_CONFIG";

// 20
		case MSG_CMD_GET_CONFIG:
			return "MSG_CMD_GET_CONFIG";
		case MSG_CMD_GET_MSG_TYPE:
			return "MSG_CMD_GET_MSG_TYPE";
		case MSG_CMD_SUBMIT_REQ:
			return "MSG_CMD_SUBMIT_REQ";
		case MSG_CMD_CANCEL_REQ:
			return "MSG_CMD_CANCEL_REQ";
		case MSG_CMD_REG_SENT_STATUS_CB:
			return "MSG_CMD_REG_SENT_STATUS_CB";

// 25
		case MSG_CMD_REG_STORAGE_CHANGE_CB:
			return "MSG_CMD_REG_STORAGE_CHANGE_CB";
		case MSG_CMD_REG_INCOMING_MSG_CB:
			return "MSG_CMD_REG_INCOMING_MSG_CB";
		case MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB:
			return "MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB";
		case MSG_CMD_REG_INCOMING_SYNCML_MSG_CB:
			return "MSG_CMD_REG_INCOMING_SYNCML_MSG_CB";
		case MSG_CMD_REG_INCOMING_LBS_MSG_CB:
			return "MSG_CMD_REG_INCOMING_LBS_MSG_CB";

// 30
		case MSG_CMD_PLG_SENT_STATUS_CNF:
			return "MSG_CMD_PLG_SENT_STATUS_CNF";
		case MSG_CMD_PLG_STORAGE_CHANGE_IND:
			return "MSG_CMD_PLG_STORAGE_CHANGE_IND";
		case MSG_CMD_PLG_INCOMING_MSG_IND:
			return "MSG_CMD_PLG_INCOMING_MSG_IND";
		case MSG_CMD_PLG_INCOMING_MMS_CONF:
			return "MSG_CMD_PLG_INCOMING_MMS_CONF";
		case MSG_CMD_PLG_INCOMING_SYNCML_IND:
			return "MSG_CMD_PLG_INCOMING_SYNCML_IND";

// 35
		case MSG_CMD_PLG_INCOMING_LBS_IND:
			return "MSG_CMD_PLG_INCOMING_LBS_IND";
		case MSG_CMD_PLG_INIT_SIM_BY_SAT:
			return "MSG_CMD_PLG_INIT_SIM_BY_SAT";
		case MSG_CMD_GET_THREADVIEWLIST:
			return "MSG_CMD_GET_THREADVIEWLIST";
		case MSG_CMD_GET_CONVERSATIONVIEWLIST:
			return "MSG_CMD_GET_CONVERSATIONVIEWLIST";
		case MSG_CMD_DELETE_THREADMESSAGELIST:
			return "MSG_CMD_DELETE_THREADMESSAGELIST";

// 40
		case MSG_CMD_GET_CONTACT_COUNT:
			return "MSG_CMD_GET_CONTACT_COUNT";
		case MSG_CMD_GET_QUICKPANEL_DATA:
			return "MSG_CMD_GET_QUICKPANEL_DATA";
		case MSG_CMD_COUNT_BY_MSGTYPE:
			return "MSG_CMD_COUNT_BY_MSGTYPE";
		case MSG_CMD_RESET_DB:
			return "MSG_CMD_RESET_DB";
		case MSG_CMD_GET_MEMSIZE:
			return "MSG_CMD_GET_MEMSIZE";

// 45
		case MSG_CMD_BACKUP_MESSAGE:
			return "MSG_CMD_BACKUP_MESSAGE";
		case MSG_CMD_RESTORE_MESSAGE:
			return "MSG_CMD_RESTORE_MESSAGE";
		case MSG_CMD_UPDATE_THREAD_READ:
			return "MSG_CMD_UPDATE_THREAD_READ";
		case MSG_CMD_REG_SYNCML_MSG_OPERATION_CB:
			return "MSG_CMD_REG_SYNCML_MSG_OPERATION_CB";
		case MSG_CMD_SYNCML_OPERATION:
			return "MSG_CMD_SYNCML_OPERATION";

// 50
		case MSG_CMD_GET_REPORT_STATUS:
			return "MSG_CMD_GET_REPORT_STATUS";

		default:
			return "Unknown Command Type!!!";
	}

	return NULL;
}

const char * MsgDbgEvtStr(MSG_EVENT_TYPE_T evtType)
{
	switch ( evtType )
	{
// 0
		case MSG_EVENT_OPEN_HANDLE:
			return "MSG_EVENT_OPEN_HANDLE";
		case MSG_EVENT_CLOSE_HANDLE:
			return "MSG_EVENT_CLOSE_HANDLE";
		case MSG_EVENT_GET_STORAGELIST:
			return "MSG_EVENT_GET_STORAGELIST";
		case MSG_EVENT_ADD_MSG:
			return "MSG_EVENT_ADD_MSG";
		case MSG_EVENT_ADD_SYNCML_MSG:
			return "MSG_EVENT_ADD_SYNCML_MSG";
// 5
		case MSG_EVENT_UPDATE_MSG:
			return "MSG_EVENT_UPDATE_MSG";
		case MSG_EVENT_UPDATE_READ:
			return "MSG_EVENT_UPDATE_READ";
		case MSG_EVENT_UPDATE_PROTECTED:
			return "MSG_EVENT_UPDATE_PROTECTED";
		case MSG_EVENT_DELETE_MSG:
			return "MSG_EVENT_DELETE_MSG";
		case MSG_EVENT_DELALL_MSGINFOLDER:
			return "MSG_EVENT_DELALL_MSGINFOLDER";
// 10
		case MSG_EVENT_MOVE_MSGTOFOLDER:
			return "MSG_EVENT_MOVE_MSGTOFOLDER";
		case MSG_EVENT_MOVE_MSGTOSTORAGE:
			return "MSG_EVENT_MOVE_MSGTOSTORAGE";
		case MSG_EVENT_COUNT_MSG:
			return "MSG_EVENT_COUNT_MSG";
		case MSG_EVENT_GET_MSG:
			return "MSG_EVENT_GET_MSG";
		case MSG_EVENT_GET_FOLDERVIEWLIST:
			return "MSG_EVENT_GET_FOLDERVIEWLIST";
// 15
		case MSG_EVENT_ADD_FOLDER:
			return "MSG_EVENT_ADD_FOLDER";
		case MSG_EVENT_UPDATE_FOLDER:
			return "MSG_EVENT_UPDATE_FOLDER";
		case MSG_EVENT_DELETE_FOLDER:
			return "MSG_EVENT_DELETE_FOLDER";
		case MSG_EVENT_GET_FOLDERLIST:
			return "MSG_EVENT_GET_FOLDERLIST";
		case MSG_EVENT_SET_CONFIG:
			return "MSG_EVENT_SET_CONFIG";
// 20
		case MSG_EVENT_GET_CONFIG:
			return "MSG_EVENT_GET_CONFIG";
		case MSG_EVENT_GET_MSG_TYPE:
			return "MSG_EVENT_GET_MSG_TYPE";
		case MSG_EVENT_SUBMIT_REQ:
			return "MSG_EVENT_SUBMIT_REQ";
		case MSG_EVENT_CANCEL_REQ:
			return "MSG_EVENT_CANCEL_REQ";
		case MSG_EVENT_REG_SENT_STATUS_CB:
			return "MSG_EVENT_REG_SENT_STATUS_CB";
// 25
		case MSG_EVENT_REG_STORAGE_CHANGE_CB:
			return "MSG_EVENT_REG_STORAGE_CHANGE_CB";
		case MSG_EVENT_REG_INCOMING_MSG_CB:
			return "MSG_EVENT_REG_INCOMING_MSG_CB";
		case MSG_EVENT_REG_INCOMING_MMS_CONF_MSG_CB:
			return "MSG_EVENT_REG_INCOMING_MMS_CONF_MSG_CB";
		case MSG_EVENT_REG_INCOMING_SYNCML_MSG_CB:
			return "MSG_EVENT_REG_INCOMING_SYNCML_MSG_CB";
		case MSG_EVENT_REG_INCOMING_LBS_MSG_CB:
			return "MSG_EVENT_REG_INCOMING_LBS_MSG_CB";
// 30
		case MSG_EVENT_PLG_SENT_STATUS_CNF:
			return "MSG_EVENT_PLG_SENT_STATUS_CNF";
		case MSG_EVENT_PLG_STORAGE_CHANGE_IND:
			return "MSG_EVENT_STORAGE_CHANGE_CB";
		case MSG_EVENT_PLG_INCOMING_MSG_IND:
			return "MSG_EVENT_PLG_INCOMING_MSG_IND";
		case MSG_EVENT_PLG_INCOMING_MMS_CONF:
			return "MSG_EVENT_PLG_INCOMING_MMS_CONF";
		case MSG_EVENT_PLG_INCOMING_SYNCML_MSG_IND:
			return "MSG_EVENT_PLG_INCOMING_SYNCML_MSG_IND";
// 35
		case MSG_EVENT_PLG_INCOMING_LBS_MSG_IND:
			return "MSG_EVENT_PLG_INCOMING_LBS_MSG_IND";
		case MSG_EVENT_PLG_INIT_SIM_BY_SAT:
			return "MSG_EVENT_PLG_INIT_SIM_BY_SAT";
		case MSG_EVENT_GET_THREADVIEWLIST:
			return "MSG_EVENT_GET_THREADVIEWLIST";
		case MSG_EVENT_GET_CONVERSATIONVIEWLIST:
			return "MSG_EVENT_GET_CONVERSATIONVIEWLIST";
		case MSG_EVENT_DELETE_THREADMESSAGELIST:
			return "MSG_EVENT_DELETE_THREADMESSAGELIST";
// 40
		case MSG_EVENT_GET_CONTACT_COUNT:
			return "MSG_EVENT_GET_CONTACT_COUNT";
		case MSG_EVENT_GET_QUICKPANEL_DATA:
			return "MSG_EVENT_GET_QUICKPANEL_DATA";
		case MSG_EVENT_COUNT_BY_MSGTYPE:
			return "MSG_EVENT_COUNT_BY_MSGTYPE";
		case MSG_EVENT_RESET_DB:
			return "MSG_EVENT_RESET_DB";
		case MSG_EVENT_GET_MEMSIZE:
			return "MSG_EVENT_GET_MEMSIZE";
// 45
		case MSG_EVENT_BACKUP_MESSAGE:
			return "MSG_EVENT_BACKUP_MESSAGE";
		case MSG_EVENT_RESTORE_MESSAGE:
			return "MSG_EVENT_RESTORE_MESSAGE";
		case MSG_EVENT_UPDATE_THREAD_READ:
			return "MSG_EVENT_UPDATE_THREAD_READ";
		case MSG_EVENT_REG_SYNCML_MSG_OPERATION_CB:
			return "MSG_EVENT_REG_SYNCML_MSG_OPERATION_CB";
		case MSG_EVENT_SYNCML_OPERATION:
			return "MSG_EVENT_SYNCML_OPERATION";
// 50
		case MSG_EVENT_GET_REPORT_STATUS:
			return "MSG_EVENT_GET_REPORT_STATUS";


		default:
			return "Unknown Event Type!!!";
	}

	return NULL;
}


int get_tid()
{
    return syscall(__NR_gettid);
}

