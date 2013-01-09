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
		case MSG_CMD_ADD_FILTER:
			return "MSG_CMD_ADD_FILTER";

// 20
		case MSG_CMD_UPDATE_FILTER:
			return "MSG_CMD_UPDATE_FILTER";
		case MSG_CMD_DELETE_FILTER:
			return "MSG_CMD_DELETE_FILTER";
		case MSG_CMD_GET_FILTERLIST:
			return "MSG_CMD_GET_FILTERLIST";
		case MSG_CMD_SET_FILTER_OPERATION:
			return "MSG_CMD_SET_FILTER_OPERATION";
		case MSG_CMD_GET_FILTER_OPERATION:
			return "MSG_CMD_GET_FILTER_OPERATION";

// 25
		case MSG_CMD_GET_MSG_TYPE:
			return "MSG_CMD_GET_MSG_TYPE";
		case MSG_CMD_SUBMIT_REQ:
			return "MSG_CMD_SUBMIT_REQ";
		case MSG_CMD_CANCEL_REQ:
			return "MSG_CMD_CANCEL_REQ";
		case MSG_CMD_REG_SENT_STATUS_CB:
			return "MSG_CMD_REG_SENT_STATUS_CB";
		case MSG_CMD_REG_STORAGE_CHANGE_CB:
			return "MSG_CMD_REG_STORAGE_CHANGE_CB";

// 30
		case MSG_CMD_REG_INCOMING_MSG_CB:
			return "MSG_CMD_REG_INCOMING_MSG_CB";
		case MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB:
			return "MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB";
		case MSG_CMD_REG_INCOMING_SYNCML_MSG_CB:
			return "MSG_CMD_REG_INCOMING_SYNCML_MSG_CB";
		case MSG_CMD_REG_INCOMING_LBS_MSG_CB:
			return "MSG_CMD_REG_INCOMING_LBS_MSG_CB";
		case MSG_CMD_PLG_SENT_STATUS_CNF:
			return "MSG_CMD_PLG_SENT_STATUS_CNF";

// 35
		case MSG_CMD_PLG_STORAGE_CHANGE_IND:
			return "MSG_CMD_PLG_STORAGE_CHANGE_IND";
		case MSG_CMD_PLG_INCOMING_MSG_IND:
			return "MSG_CMD_PLG_INCOMING_MSG_IND";
		case MSG_CMD_PLG_INCOMING_MMS_CONF:
			return "MSG_CMD_PLG_INCOMING_MMS_CONF";
		case MSG_CMD_PLG_INCOMING_SYNCML_IND:
			return "MSG_CMD_PLG_INCOMING_SYNCML_IND";
		case MSG_CMD_PLG_INCOMING_LBS_IND:
			return "MSG_CMD_PLG_INCOMING_LBS_IND";

// 40
		case MSG_CMD_PLG_INIT_SIM_BY_SAT:
			return "MSG_CMD_PLG_INIT_SIM_BY_SAT";
		case MSG_CMD_GET_THREADVIEWLIST:
			return "MSG_CMD_GET_THREADVIEWLIST";
		case MSG_CMD_GET_CONVERSATIONVIEWLIST:
			return "MSG_CMD_GET_CONVERSATIONVIEWLIST";
		case MSG_CMD_DELETE_THREADMESSAGELIST:
			return "MSG_CMD_DELETE_THREADMESSAGELIST";
		case MSG_CMD_GET_CONTACT_COUNT:
			return "MSG_CMD_GET_CONTACT_COUNT";

// 45
		case MSG_CMD_GET_QUICKPANEL_DATA:
			return "MSG_CMD_GET_QUICKPANEL_DATA";
		case MSG_CMD_COUNT_BY_MSGTYPE:
			return "MSG_CMD_COUNT_BY_MSGTYPE";
		case MSG_CMD_RESET_DB:
			return "MSG_CMD_RESET_DB";
		case MSG_CMD_GET_MEMSIZE:
			return "MSG_CMD_GET_MEMSIZE";
		case MSG_CMD_BACKUP_MESSAGE:
			return "MSG_CMD_BACKUP_MESSAGE";

// 50
		case MSG_CMD_RESTORE_MESSAGE:
			return "MSG_CMD_RESTORE_MESSAGE";
		case MSG_CMD_UPDATE_THREAD_READ:
			return "MSG_CMD_UPDATE_THREAD_READ";
		case MSG_CMD_REG_SYNCML_MSG_OPERATION_CB:
			return "MSG_CMD_REG_SYNCML_MSG_OPERATION_CB";
		case MSG_CMD_SYNCML_OPERATION:
			return "MSG_CMD_SYNCML_OPERATION";
		case MSG_CMD_GET_REPORT_STATUS:
			return "MSG_CMD_GET_REPORT_STATUS";

// 55
		case MSG_CMD_GET_THREAD_ID_BY_ADDRESS:
			return "MSG_CMD_GET_THREAD_ID_BY_ADDRESS";
		case MSG_CMD_GET_THREAD_INFO:
			return "MSG_CMD_GET_THREAD_INFO";
		case MSG_CMD_GET_SMSC_OPT:
			return "MSG_CMD_GET_SMSC_OPT";
		case MSG_CMD_GET_CB_OPT:
			return "MSG_CMD_GET_CB_OPT";

// 60
		case MSG_CMD_GET_SMS_SEND_OPT:
			return "MSG_CMD_GET_SMS_SEND_OPT";
		case MSG_CMD_GET_MMS_SEND_OPT:
			return "MSG_CMD_GET_MMS_SEND_OPT";
		case MSG_CMD_GET_MMS_RECV_OPT:
			return "MSG_CMD_GET_MMS_RECV_OPT";
		case MSG_CMD_GET_PUSH_MSG_OPT:
			return "MSG_CMD_GET_PUSH_MSG_OPT";
		case MSG_CMD_GET_VOICE_MSG_OPT:
			return "MSG_CMD_GET_VOICE_MSG_OPT";

// 65
		case MSG_CMD_GET_GENERAL_MSG_OPT:
			return "MSG_CMD_GET_GENERAL_MSG_OPT";

		case MSG_CMD_GET_MSG_SIZE_OPT:
			return "MSG_CMD_GET_MSG_SIZE_OPT";
		case MSG_CMD_SET_SMSC_OPT:
			return "MSG_CMD_SET_SMSC_OPT";


// 70
		case MSG_CMD_SET_CB_OPT:
			return "MSG_CMD_SET_CB_OPT";
		case MSG_CMD_SET_SMS_SEND_OPT:
			return "MSG_CMD_SET_SMS_SEND_OPT";
		case MSG_CMD_SET_MMS_SEND_OPT:
			return "MSG_CMD_SET_MMS_SEND_OPT";
		case MSG_CMD_SET_MMS_RECV_OPT:
			return "MSG_CMD_SET_MMS_RECV_OPT";
		case MSG_CMD_SET_PUSH_MSG_OPT:
			return "MSG_CMD_SET_PUSH_MSG_OPT";

// 75
		case MSG_CMD_SET_VOICE_MSG_OPT:
			return "MSG_CMD_SET_VOICE_MSG_OPT";
		case MSG_CMD_SET_GENERAL_MSG_OPT:
			return "MSG_CMD_SET_GENERAL_MSG_OPT";
		case MSG_CMD_SET_MSG_SIZE_OPT:
			return "MSG_CMD_SET_MSG_SIZE_OPT";
// 80
		case MSG_CMD_REG_INCOMING_PUSH_MSG_CB:
			return "MSG_CMD_REG_INCOMING_PUSH_MSG_CB";
		case MSG_CMD_PLG_INCOMING_PUSH_IND:
			return "MSG_CMD_PLG_INCOMING_PUSH_IND";
		case MSG_CMD_REG_INCOMING_CB_MSG_CB:
			return "MSG_CMD_REG_INCOMING_CB_MSG_CB";
		case MSG_CMD_PLG_INCOMING_CB_IND:
			return "MSG_CMD_PLG_INCOMING_CB_IND";
		case MSG_CMD_ADD_PUSH_EVENT:
			return "MSG_CMD_ADD_PUSH_EVENT";
//85
		case MSG_CMD_DELETE_PUSH_EVENT:
			return "MSG_CMD_DELETE_PUSH_EVENT";
		case MSG_CMD_UPDATE_PUSH_EVENT:
			return "MSG_CMD_UPDATE_PUSH_EVENT";

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
		case MSG_EVENT_ADD_FILTER:
			return "MSG_EVENT_ADD_FILTER";
//20
		case MSG_EVENT_UPDATE_FILTER:
			return "MSG_EVENT_UPDATE_FILTER";
		case MSG_EVENT_DELETE_FILTER:
			return "MSG_EVENT_DELETE_FILTER";
		case MSG_EVENT_GET_FILTERLIST:
			return "MSG_EVENT_GET_FILTERLIST";
		case MSG_EVENT_SET_FILTER_OPERATION:
			return "MSG_EVENT_SET_FILTER_OPERATION";
		case MSG_EVENT_GET_FILTER_OPERATION:
			return "MSG_EVENT_GET_FILTER_OPERATION";
//25
		case MSG_EVENT_GET_MSG_TYPE:
			return "MSG_EVENT_GET_MSG_TYPE";
		case MSG_EVENT_SUBMIT_REQ:
			return "MSG_EVENT_SUBMIT_REQ";
		case MSG_EVENT_CANCEL_REQ:
			return "MSG_EVENT_CANCEL_REQ";
		case MSG_EVENT_REG_SENT_STATUS_CB:
			return "MSG_EVENT_REG_SENT_STATUS_CB";
		case MSG_EVENT_REG_STORAGE_CHANGE_CB:
			return "MSG_EVENT_REG_STORAGE_CHANGE_CB";

// 30
		case MSG_EVENT_REG_INCOMING_MSG_CB:
			return "MSG_EVENT_REG_INCOMING_MSG_CB";
		case MSG_EVENT_REG_INCOMING_MMS_CONF_MSG_CB:
			return "MSG_EVENT_REG_INCOMING_MMS_CONF_MSG_CB";
		case MSG_EVENT_REG_INCOMING_SYNCML_MSG_CB:
			return "MSG_EVENT_REG_INCOMING_SYNCML_MSG_CB";
		case MSG_EVENT_REG_INCOMING_LBS_MSG_CB:
			return "MSG_EVENT_REG_INCOMING_LBS_MSG_CB";
		case MSG_EVENT_PLG_SENT_STATUS_CNF:
			return "MSG_EVENT_PLG_SENT_STATUS_CNF";

// 35
		case MSG_EVENT_PLG_STORAGE_CHANGE_IND:
			return "MSG_EVENT_STORAGE_CHANGE_CB";
		case MSG_EVENT_PLG_INCOMING_MSG_IND:
			return "MSG_EVENT_PLG_INCOMING_MSG_IND";
		case MSG_EVENT_PLG_INCOMING_MMS_CONF:
			return "MSG_EVENT_PLG_INCOMING_MMS_CONF";
		case MSG_EVENT_PLG_INCOMING_SYNCML_MSG_IND:
			return "MSG_EVENT_PLG_INCOMING_SYNCML_MSG_IND";
		case MSG_EVENT_PLG_INCOMING_LBS_MSG_IND:
			return "MSG_EVENT_PLG_INCOMING_LBS_MSG_IND";

// 40
		case MSG_EVENT_PLG_INIT_SIM_BY_SAT:
			return "MSG_EVENT_PLG_INIT_SIM_BY_SAT";
		case MSG_EVENT_GET_THREADVIEWLIST:
			return "MSG_EVENT_GET_THREADVIEWLIST";
		case MSG_EVENT_GET_CONVERSATIONVIEWLIST:
			return "MSG_EVENT_GET_CONVERSATIONVIEWLIST";
		case MSG_EVENT_DELETE_THREADMESSAGELIST:
			return "MSG_EVENT_DELETE_THREADMESSAGELIST";
		case MSG_EVENT_GET_CONTACT_COUNT:
			return "MSG_EVENT_GET_CONTACT_COUNT";

// 45
		case MSG_EVENT_GET_QUICKPANEL_DATA:
			return "MSG_EVENT_GET_QUICKPANEL_DATA";
		case MSG_EVENT_COUNT_BY_MSGTYPE:
			return "MSG_EVENT_COUNT_BY_MSGTYPE";
		case MSG_EVENT_RESET_DB:
			return "MSG_EVENT_RESET_DB";
		case MSG_EVENT_GET_MEMSIZE:
			return "MSG_EVENT_GET_MEMSIZE";
		case MSG_EVENT_BACKUP_MESSAGE:
			return "MSG_EVENT_BACKUP_MESSAGE";

// 50
		case MSG_EVENT_RESTORE_MESSAGE:
			return "MSG_EVENT_RESTORE_MESSAGE";
		case MSG_EVENT_UPDATE_THREAD_READ:
			return "MSG_EVENT_UPDATE_THREAD_READ";
		case MSG_EVENT_REG_SYNCML_MSG_OPERATION_CB:
			return "MSG_EVENT_REG_SYNCML_MSG_OPERATION_CB";
		case MSG_EVENT_SYNCML_OPERATION:
			return "MSG_EVENT_SYNCML_OPERATION";
		case MSG_EVENT_GET_REPORT_STATUS:
			return "MSG_EVENT_GET_REPORT_STATUS";

// 55
		case MSG_CMD_GET_THREAD_ID_BY_ADDRESS:
			return "MSG_CMD_GET_THREAD_ID_BY_ADDRESS";
		case MSG_CMD_GET_THREAD_INFO:
			return "MSG_CMD_GET_THREAD_INFO";
		case MSG_EVENT_GET_SMSC_OPT:
			return "MSG_EVENT_GET_SMSC_OPT";
		case MSG_EVENT_GET_CB_OPT:
			return "MSG_EVENT_GET_CB_OPT";

// 60
		case MSG_EVENT_GET_SMS_SEND_OPT:
			return "MSG_EVENT_GET_SMS_SEND_OPT";
		case MSG_EVENT_GET_MMS_SEND_OPT:
			return "MSG_EVENT_GET_MMS_SEND_OPT";
		case MSG_EVENT_GET_MMS_RECV_OPT:
			return "MSG_EVENT_GET_MMS_RECV_OPT";
		case MSG_EVENT_GET_PUSH_MSG_OPT:
			return "MSG_EVENT_GET_PUSH_MSG_OPT";
		case MSG_EVENT_GET_VOICE_MSG_OPT:
			return "MSG_EVENT_GET_VOICE_MSG_OPT";

// 65
		case MSG_EVENT_GET_GENERAL_MSG_OPT:
			return "MSG_EVENT_GET_GENERAL_MSG_OPT";

// 65
		case MSG_EVENT_GET_MSG_SIZE_OPT:
			return "MSG_EVENT_GET_MSG_SIZE_OPT";
		case MSG_EVENT_SET_SMSC_OPT:
			return "MSG_EVENT_SET_SMSC_OPT";
		case MSG_EVENT_SET_CB_OPT:
			return "MSG_EVENT_SET_CB_OPT";
		case MSG_EVENT_SET_SMS_SEND_OPT:
			return "MSG_EVENT_SET_SMS_SEND_OPT";
		case MSG_EVENT_SET_MMS_SEND_OPT:
			return "MSG_EVENT_SET_MMS_SEND_OPT";
		case MSG_EVENT_SET_MMS_RECV_OPT:
			return "MSG_EVENT_SET_MMS_RECV_OPT";

// 70
		case MSG_EVENT_SET_PUSH_MSG_OPT:
			return "MSG_EVENT_SET_PUSH_MSG_OPT";
		case MSG_EVENT_SET_VOICE_MSG_OPT:
			return "MSG_EVENT_SET_VOICE_MSG_OPT";
		case MSG_EVENT_SET_GENERAL_MSG_OPT:
			return "MSG_EVENT_SET_GENERAL_MSG_OPT";
		case MSG_EVENT_SET_MSG_SIZE_OPT:
			return "MSG_EVENT_SET_MSG_SIZE_OPT";
		case MSG_EVENT_REG_INCOMING_PUSH_MSG_CB:
			return "MSG_EVENT_REG_INCOMING_PUSH_MSG_CB";
		case MSG_EVENT_PLG_INCOMING_PUSH_MSG_IND:
			return "MSG_EVENT_PLG_INCOMING_PUSH_MSG_IND";
		case MSG_EVENT_REG_INCOMING_CB_MSG_CB:
			return "MSG_EVENT_REG_INCOMING_CB_MSG_CB";
		case MSG_EVENT_PLG_INCOMING_CB_MSG_IND:
			return "MSG_EVENT_PLG_INCOMING_CB_MSG_IND";
		case MSG_EVENT_ADD_PUSH_EVENT:
			return "MSG_EVENT_ADD_PUSH_EVENT";

		case MSG_EVENT_DELETE_PUSH_EVENT:
			return "MSG_EVENT_DELETE_PUSH_EVENT";
		case MSG_EVENT_UPDATE_PUSH_EVENT:
			return "MSG_EVENT_UPDATE_PUSH_EVENT";

		default:
			return "Unknown Event Type!!!";
	}

	return NULL;
}


int get_tid()
{
    return syscall(__NR_gettid);
}

