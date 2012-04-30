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


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/syscall.h>
#include <glib.h>

using namespace std;

#include "MapiControl.h"
#include "MapiStorage.h"
#include "MapiTransport.h"
#include "MapiMessage.h"
#include "MsgTestStorage.h"
#include "MsgTestSetting.h"
#include "MsgTestThreadView.h"
#include "MsgTestConvert.h"
#include "main.h"
#include <stdio.h>

/*==================================================================================================
                                         VARIABLES
==================================================================================================*/
extern MSG_FOLDER_LIST_S g_folderList;

static MSG_HANDLE_T msgHandle = NULL;

GMainLoop *mainloop;

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
int main(void)
{
	print("======================================");
	print("==== Start Messaging FW Test App. ====");
	print("======================================");

	if (init_app() != MSG_SUCCESS)
		return -1;

	return 0;
}


void print(const char* string)
{
	cout << string << endl;
}


void print(int i)
{
	cout << i << endl;
}


int get_tid()
{
    return syscall(__NR_gettid);
}


void printMessageS(msg_message_t pNewMsg)
{
	MSG_DEBUG("msgId = %d", msg_get_message_id(pNewMsg));
	MSG_DEBUG("folderId = %d", msg_get_folder_id(pNewMsg));
	MSG_DEBUG("msgType = %d", msg_get_message_type(pNewMsg));
	MSG_DEBUG("nAddressCnt = %d", msg_get_address_count(pNewMsg));
	MSG_DEBUG("addressList[0].threadId = %d", msg_get_ith_thread_id(pNewMsg, 0));
	MSG_DEBUG("addressList[0].addressVal = %s", msg_get_ith_address(pNewMsg, 0));
	MSG_DEBUG("addressList[0].displayname = %s", msg_get_ith_name(pNewMsg, 0));
	MSG_DEBUG("displayTime = %s", ctime(msg_get_time(pNewMsg)));
	MSG_DEBUG("networkStatus = %d", msg_get_network_status(pNewMsg));
	MSG_DEBUG("bRead = %d", msg_is_read(pNewMsg));
	MSG_DEBUG("bProtected = %d", msg_is_protected(pNewMsg));
	MSG_DEBUG("priority = %d", msg_get_priority_info(pNewMsg));
	MSG_DEBUG("dataSize = %d", msg_get_message_body_size(pNewMsg));

	int msgType = msg_get_message_type(pNewMsg);

	if(msgType==MSG_TYPE_MMS_NOTI || msgType==MSG_TYPE_MMS_JAVA || msgType==MSG_TYPE_MMS)
		MSG_DEBUG("msgData = %s", msg_mms_get_text_contents(pNewMsg));
	else
		MSG_DEBUG("msgData = %s", msg_sms_get_message_body(pNewMsg));
}


void sentStatusCB(MSG_HANDLE_T Handle, MSG_SENT_STATUS_S *pStatus, void *pUserParam)
{
	MSG_DEBUG("Receive SentStatus !!!!!!!!!!!!!!!!!");

	if (pStatus->status == MSG_NETWORK_SEND_SUCCESS)
		MSG_DEBUG("reqId : %d  MSG SENT SUCCESS !!! ", pStatus->reqId);
	else
		MSG_DEBUG("reqId : %d  MSG SENT FAIL !!! [%d]", pStatus->reqId, pStatus->status);

	MSG_DEBUG("User Param [%s]", (char*)pUserParam);
}


void incomingSmsCB(MSG_HANDLE_T Handle, msg_message_t msg, void *pUserParam)
{
	MSG_DEBUG("Receive New Message !!!!!!!!!!!!!!!!!");

	printMessageS(msg);

	MSG_DEBUG("User Param [%s]", (char*)pUserParam);
}


void incomingMmsConfCB(MSG_HANDLE_T Handle, msg_message_t msg, void *pUserParam)
{
	if ( msg_get_network_status(msg) == MSG_NETWORK_RETRIEVE_SUCCESS)
	{
		MSG_DEBUG("Receive New MMS Message !!!!!!!!!!!!!!!!!msg Id [%d]", msg_get_message_id(msg));
	}
	else
		MSG_DEBUG("MMS RETRIEVE FAIL !!!!!!! [%d]", msg_get_network_status(msg));
}


void appPortCB (MSG_HANDLE_T Handle, msg_message_t msg, void *pUserParam)
{
	MSG_DEBUG("Receive App Port Message - Port Number [%d] !!!!!!!!!!!!!!!!!", msg_get_dest_port(msg));

	printMessageS(msg);
}


void syncMLCB(MSG_HANDLE_T hMsgHandle, MSG_SYNCML_MESSAGE_TYPE_T msgType, const char* pPushBody, int PushBodyLen, const char* wspHeader, int wspHeaderLen, void *pUserParam)
{
	MSG_DEBUG("syncMLCB() called");

	MSG_DEBUG("msgType [%d]", msgType);

//	MSG_DEBUG("PushHeaderLen [%d]", PushHeaderLen);

//	MSG_DEBUG("pPushHeader [%s]", pPushHeader);

	MSG_DEBUG("PushBodyLen [%d]", PushBodyLen);

	MSG_DEBUG("[pPushBody]");
	for (int i = 0; i < PushBodyLen; i++)
	{
		printf("[%02x]", pPushBody[i]);
	}
	printf("\n\n");

	MSG_SYNCML_MESSAGE_S syncMLMsg;

	memset(&syncMLMsg, 0x00, sizeof(MSG_SYNCML_MESSAGE_S));

	syncMLMsg.extId = 11;
	syncMLMsg.pinCode = 1111;

	syncMLMsg.msg = msg_new_message();

	msg_set_folder_id(syncMLMsg.msg, MSG_INBOX_ID); //	syncMLMsg.msg.folderId = 1;
	msg_set_message_type(syncMLMsg.msg, MSG_TYPE_SMS_SYNCML);
	msg_set_network_status(syncMLMsg.msg, MSG_NETWORK_RECEIVED);	//syncMLMsg.msg.networkStatus = MSG_NETWORK_RECEIVED;
	msg_set_direction_info(syncMLMsg.msg, MSG_DIRECTION_TYPE_MT);

	msg_add_address(syncMLMsg.msg, "+1004", MSG_RECIPIENTS_TYPE_TO);

	// setting received time
	time_t t 		= 	time(NULL);
	time_t utfTime	= 	time(&t);
	msg_set_time(syncMLMsg.msg, utfTime);

	char msg_body[]="SyncML Message";
	msg_sms_set_message_body(syncMLMsg.msg, msg_body, strlen(msg_body));

	int err = msg_add_syncml_message(msgHandle, &syncMLMsg);

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgAddSyncMLMessage() Fail [%d]", err);
		return;
	}
}


void syncMLOperationCB(MSG_HANDLE_T handle, int msgId, int extId, void *user_param)
{
	MSG_DEBUG("syncMLOperationCB() called");

	MSG_DEBUG("msgId [%d]", msgId);

	MSG_DEBUG("extId [%d]", extId);

	//SyncML operation proccess hsa to be place here.
}


void lbsCB(MSG_HANDLE_T hMsgHandle, const char* pPushHeader, const char* pPushBody, int pushBodyLen, void *pUserParam)
{
	MSG_DEBUG("lbsCB() called");

	MSG_DEBUG("pPushHeader : [%s]", pPushHeader);

	MSG_DEBUG("pushBodyLen [%d]", pushBodyLen);

	MSG_DEBUG("[pPushBody]");
	for (int i = 0; i < pushBodyLen; i++)
	{
		printf("[%02x]", pPushBody[i]);
	}
	printf("\n\n");
}


void storageChangeCB(MSG_HANDLE_T hMsgHandle, MSG_STORAGE_CHANGE_TYPE_T storageChangeType, MSG_MSGID_LIST_S *pMsgIdList, void *pUserParam)
{
	MSG_DEBUG("storageChangeCB() called");

	MSG_DEBUG("storageChangeType[%d]", storageChangeType);

	MSG_DEBUG("pMsgIdList->nCount [%d]", pMsgIdList->nCount);

	if (pMsgIdList->nCount > 0) {
		for (int i = 0; i < pMsgIdList->nCount; i++)
			MSG_DEBUG("pMsgIdList->msgIdList[%d] [%d]", i, pMsgIdList->msgIdList[i]);
	}
}


MSG_ERROR_T init_app()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char str[TEST_APP_PIRNT_STR_LEN+1] = {0,};

	err = msg_open_msg_handle(&msgHandle);

	if (err != MSG_SUCCESS)
	{
		snprintf(str, TEST_APP_PIRNT_STR_LEN+1, "msg_open_msg_handle() Fail [%d]", err);
		print(str);

		return err;
	}

	err = msg_reg_sent_status_callback(msgHandle, &sentStatusCB, (void*)"sent status callback");

	if (err != MSG_SUCCESS)
	{
		snprintf(str, TEST_APP_PIRNT_STR_LEN+1, "msg_reg_sent_status_callback() Fail [%d]", err);
		print(str);

		return err;
	}

	err = msg_reg_sms_message_callback(msgHandle, &incomingSmsCB, 0, (void*)"sms message callback");

	if (err != MSG_SUCCESS)
	{
		snprintf(str, TEST_APP_PIRNT_STR_LEN+1, "msg_reg_sms_message_callback() Fail [%d]", err);
		print(str);

		return err;
	}

	err = msg_reg_sms_message_callback(msgHandle, &appPortCB, 4010, (void*)"sms message callback with port");

	if (err != MSG_SUCCESS)
	{
		snprintf(str, TEST_APP_PIRNT_STR_LEN+1, "msg_reg_sms_message_callback() Fail [%d]", err);
		print(str);

		return err;
	}

        err = msg_reg_syncml_message_callback(msgHandle, &syncMLCB, NULL);

	if (err != MSG_SUCCESS)
	{
		snprintf(str, TEST_APP_PIRNT_STR_LEN+1, "msg_reg_syncml_message_callback() Fail [%d]", err);
		print(str);

		return err;
	}

	err = msg_reg_syncml_message_operation_callback(msgHandle, &syncMLOperationCB, NULL);

	if (err != MSG_SUCCESS)
	{
		snprintf(str, TEST_APP_PIRNT_STR_LEN+1, "msg_reg_syncml_message_operation_callback() Fail [%d]", err);
		print(str);

		return err;
	}

	err = msg_reg_lbs_message_callback(msgHandle, &lbsCB, NULL);

	if (err != MSG_SUCCESS)
	{
		snprintf(str, TEST_APP_PIRNT_STR_LEN+1, "msg_reg_lbs_message_callback() Fail [%d]", err);
		print(str);

		return err;
	}

	err = msg_reg_mms_conf_message_callback(msgHandle, &incomingMmsConfCB, NULL, NULL);

	if (err != MSG_SUCCESS)
	{
		snprintf(str, TEST_APP_PIRNT_STR_LEN+1, "msg_reg_mms_conf_message_callback() Fail [%d]", err);
		print(str);

		return err;
	}

	err = msg_reg_storage_change_callback(msgHandle, &storageChangeCB, NULL);

	if (err != MSG_SUCCESS)
	{
		snprintf(str, TEST_APP_PIRNT_STR_LEN+1, "msg_reg_storage_change_callback() Fail [%d]", err);
		print(str);

		return err;
	}

	pthread_t tid;

	if (pthread_create(&tid, NULL, show_menu, NULL) != 0)
	{
		MSG_DEBUG("MsgInitThread not invoked: %s", strerror(errno));
		return MSG_ERR_UNKNOWN;
	}

	mainloop = g_main_loop_new(NULL, FALSE);

	MSG_DEBUG("Entering GMain Loop to Receive Notifications...");

	g_main_loop_run(mainloop);

	return err;
}


void* show_menu(void*)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char menu[3];

	char strPrint[TEST_APP_PIRNT_STR_LEN+1] = {0,};

	// Get Message Count
	MSG_COUNT_INFO_S countInfo;

	do
	{
		msg_release_folder_list(&g_folderList);

		// Get Folder List
		err = msg_get_folder_list(msgHandle, &g_folderList);

		if (err != MSG_SUCCESS)
		{
			MSG_DEBUG("MsgGetFolderList() Fail [%d]", err);
			return (void*)1;
		}

		system ("clear");

		print("======================================");
		print("============== Box List ==============");
		print("======================================");

		for (int i = 0; i < g_folderList.nCount; i++)
		{
			if (msg_count_message(msgHandle, g_folderList.folderInfo[i].folderId, &countInfo) != MSG_SUCCESS)
				return (void*)1;

			memset(strPrint, 0x00, sizeof(strPrint));
			snprintf(strPrint, TEST_APP_PIRNT_STR_LEN+1, "[%d] %s \t [%d/%d] [SMS %d/MMS %d]",
				g_folderList.folderInfo[i].folderId, g_folderList.folderInfo[i].folderName, countInfo.nUnreadCnt, countInfo.nUnreadCnt + countInfo.nReadCnt, countInfo.nSms, countInfo.nMms);
			print(strPrint);
		}

		print("======================================");
		print("================ Menu ================");
		print("======================================");
		print("[C] Create Message");
		print("[S] Setting");
		print("[A] Add New Folder");
		print("[D] Delete a Folder");
		print("[U] Update Folder Name");
		print("[V] Convert(Encode/Decode) Charset");
		print("[M] Msg Generator");
		print("[T] Thread View Test");
		print("[G] Search Message");
		print("[Q] Quit");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 3);

		run_app(menu);
	}
	while (strcmp(menu, "Q"));

	msg_release_folder_list(&g_folderList);

	msg_close_msg_handle(&msgHandle);

	if (msgHandle != NULL)
	{
		MSG_DEBUG("msgHandle is not NULL [%p]", msgHandle);
	}

	if (g_main_loop_is_running(mainloop))
		g_main_loop_quit(mainloop);

	print("======================================");
	print("==== End Messaging FW Test App. Bye===");
	print("======================================");

	exit(0);
}


void run_app(char *pMenu)
{
	if (!strcmp(pMenu, "C") || !strcmp(pMenu, "c"))
	{
		MsgTestAddMessage(msgHandle);
	}
	else if (!strcmp(pMenu, "S") || !strcmp(pMenu, "s"))
	{
		MsgTestSettingMain(msgHandle);
	}
	else if (!strcmp(pMenu, "A") || !strcmp(pMenu, "a"))
	{
		MsgTestAddFolder(msgHandle);
	}
	else if (!strcmp(pMenu, "D") || !strcmp(pMenu, "d"))
	{
		MsgTestDeleteFolder(msgHandle);
	}
	else if (!strcmp(pMenu, "U") || !strcmp(pMenu, "u"))
	{
		MsgTestUpdateFolder(msgHandle);
	}
	else if (!strcmp(pMenu, "V") || !strcmp(pMenu, "v"))
	{
//		MsgTestConvertMain();
	}
	else if (!strcmp(pMenu, "M") || !strcmp(pMenu, "m"))
	{
		MsgTestMsgGen(msgHandle);
	}
	else if (!strcmp(pMenu, "T") || !strcmp(pMenu, "t"))
	{
		MsgThreadViewMain(msgHandle);
	}
	else if (!strcmp(pMenu, "G") || !strcmp(pMenu, "g"))
	{
		MsgSearchViewMain(msgHandle);
	}
	else if (!strcmp(pMenu, "Q") || !strcmp(pMenu, "q"))
	{
		return;
	}
	else
	{
		int folderId = atoi(pMenu);

		if (folderId > 0 && folderId < 100)
		{
			// Query Msg List in Msg Box
			MsgTestGetMessageList(msgHandle, folderId);
		}
		else
			print("Not Supported");
	}
}

