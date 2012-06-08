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


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <time.h>
#include <iostream>
#include <string>
#include <stdlib.h>
using namespace std;

#include "MsgMmsTypes.h"
#include "MsgTypes.h"
#include "MapiStorage.h"
#include "MapiSetting.h"
#include "MapiMessage.h"
#include "MsgTestStorage.h"
#include "MsgTestTransport.h"
#include "MsgTestThreadView.h"
#include "main.h"


#define MSG_PROFILE_BEGIN(pfid) \
	unsigned int __prf_l1_##pfid = __LINE__;    \
	struct timeval __prf_1_##pfid;              \
	struct timeval __prf_2_##pfid;              \
	do {                                        \
		gettimeofday(&__prf_1_##pfid, 0);       \
	} while (0)

#define MSG_PROFILE_END(pfid) \
	unsigned int __prf_l2_##pfid = __LINE__;\
	do { \
		gettimeofday(&__prf_2_##pfid, 0);\
		long __ds = __prf_2_##pfid.tv_sec - __prf_1_##pfid.tv_sec;\
		long __dm = __prf_2_##pfid.tv_usec - __prf_1_##pfid.tv_usec;\
		if ( __dm < 0 ) { __ds--; __dm = 1000000 + __dm; } \
		printf("**PROFILE** [MSGFW: %s: %s() %u ~ %u] " #pfid " -> Elapsed Time: %u.%06u seconds\n",                    \
		rindex(__FILE__, '/')+1,                \
		__FUNCTION__, \
		__prf_l1_##pfid,                                         \
		__prf_l2_##pfid,                                         \
		(unsigned int)(__ds),                                    \
		(unsigned int)(__dm));                                   \
	} while (0)


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
void MsgThreadViewMain(MSG_HANDLE_T hMsgHandle)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	char menu[5];

	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_SORT_RULE_S sortRule = {0};

	// Set Sort Rule
	sortRule.sortType = MSG_SORT_BY_THREAD_DATE;
	sortRule.bAscending = false;

	MSG_THREAD_VIEW_LIST_S threadViewList;

	char displayTime[32];

	do
	{
		err = msg_get_thread_view_list(hMsgHandle, NULL, &threadViewList);

		if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD)
		{
			print("Get Message List is failed!");
			return;
		}

		system ("clear" );

		print("======================================");
		print("============ Thread View =============");
		print("======================================");

		if (threadViewList.nCount <= 0)
		{
			print("Empty...");
		}
		else
		{
			MSG_PROFILE_BEGIN(1);

			for (int i = 0; i < threadViewList.nCount; i++)
			{
				memset(displayTime, 0x00, sizeof(displayTime));

				MsgConvertTime(msg_thread_view_get_time(threadViewList.msgThreadInfo[i]), displayTime);

				printf("[%04d]\tUnread Msg [%d]\t[%s] [%s] [%s]\n[%s]\tType [%s]\tMessage Text [%s]\n",
						msg_thread_view_get_thread_id(threadViewList.msgThreadInfo[i]),
						msg_thread_view_get_unread_cnt(threadViewList.msgThreadInfo[i]),
						msg_thread_view_get_address(threadViewList.msgThreadInfo[i]),
						msg_thread_view_get_name(threadViewList.msgThreadInfo[i]),
						displayTime,
						MsgConvertDirection(msg_thread_view_get_direction(threadViewList.msgThreadInfo[i])),
						MsgConvertType(msg_thread_view_get_message_type(threadViewList.msgThreadInfo[i])),
						msg_thread_view_get_data(threadViewList.msgThreadInfo[i]));

				printf("--------------------------------------------------------\n");
			}
			MSG_PROFILE_END(1);
		}

		print("======================================");
		print("================ Menu ================");
		print("======================================");
		print("[C] Create Message");
		print("[D] Delete Thread");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 5);

		if (!strcmp(menu, "C") || !strcmp(menu, "c")) // Add Message
		{
			MsgTestAddMessage(hMsgHandle);
		}
		else if (!strcmp(menu, "D") || !strcmp(menu, "d"))
		{
			print("Choose Thread ID : ");

			char id[5];

			memset(id, 0x00, sizeof(id));
			cin.getline(id, 5);

			print("Do you really wanna delete messages in thread [Y or N] ?");

			char select[2];

			memset(select, 0x00, sizeof(select));
			cin.getline(select, 2);

MSG_PROFILE_BEGIN(3);
			if (!strcmp(select, "Y") || !strcmp(select, "y"))
				msg_delete_thread_message_list(hMsgHandle, (MSG_THREAD_ID_T)atoi(id));
MSG_PROFILE_END(3);
		}
		else if (!strcmp(menu, "B") || !strcmp(menu, "b"))
		{
			msg_release_thread_view_list(&threadViewList);
			MSG_DEBUG("release thread view list [%d]", threadViewList.nCount);
			break;
		}
		else
		{
			MSG_THREAD_ID_T threadId = atoi(menu);

			for (int i = 0; i < threadViewList.nCount; i++)
			{
				if ((MSG_THREAD_ID_T)msg_thread_view_get_thread_id(threadViewList.msgThreadInfo[i]) == threadId)
				{
					MsgRunConversationView(hMsgHandle, threadId, msg_thread_view_get_address(threadViewList.msgThreadInfo[i]), msg_thread_view_get_name(threadViewList.msgThreadInfo[i]));

					break;
				}
			}
		}

		msg_release_thread_view_list(&threadViewList);
		MSG_DEBUG("release thread view list [%d]", threadViewList.nCount);
	}
	while (strcmp(menu, "B"));
}


void MsgSearchViewMain(MSG_HANDLE_T hMsgHandle)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	char menu[5], displayTime[32], searchString[1024];

	MSG_ERROR_T err = MSG_SUCCESS;

	do
	{
		print("Search Mode 0:Message 1:Thread :");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 4);

		int searchType = atoi(menu);


		print("Input String to Search :");

		memset(searchString, 0x00, sizeof(searchString));

		cin.getline(searchString, 1024);


		if(searchType == 0)
		{

			MSG_LIST_S msgList;

			print("Target folder (0:ALLBOX 1:INBOX 2:OUTBOX 3:SENTBOX) :");

			memset(menu, 0x00, sizeof(menu));
			cin.getline(menu, 4);

			int folderId = atoi(menu);


			print("Target message type (0:ALLTYPE 1:SMS 9:MMS) :");

			memset(menu, 0x00, sizeof(menu));
			cin.getline(menu, 4);

			int msgType = atoi(menu);


			print("Search Result offset (0~) :");

			memset(menu, 0x00, sizeof(menu));
			cin.getline(menu, 4);

			int offset = atoi(menu);


			print("Search Result limit (0~) :");

			memset(menu, 0x00, sizeof(menu));
			cin.getline(menu, 4);

			int limit = atoi(menu);

			MSG_SEARCH_CONDITION_S searchCon;
			memset(&searchCon, 0x00, sizeof(MSG_SEARCH_CONDITION_S));

			searchCon.msgType = msgType;
			searchCon.folderId = folderId;
			searchCon.pSearchVal = searchString;
			searchCon.pAddressVal = searchString;

			MSG_PROFILE_BEGIN(_msg_search_message_);
			err = msg_search_message(hMsgHandle, &searchCon, offset, limit, &msgList);
			MSG_PROFILE_END(_msg_search_message_);

			if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD)
			{
				print("Get Message List is failed!");
				return;
			}


			system ("clear" );

			print("======================================");
			print("============ Search View =============");
			print("======================================");

			if (msgList.nCount <= 0)
			{
				print("Empty...");
			}
			else
			{

				for (int i = 0; i < msgList.nCount; i++)
				{
					memset(displayTime, 0x00, sizeof(displayTime));

					MsgConvertTime(msg_get_time(msgList.msgInfo[i]), displayTime);

					const char* msgText = NULL;

					int msgType = msg_get_message_type(msgList.msgInfo[i]);

					if(msgType==MSG_TYPE_MMS_NOTI || msgType==MSG_TYPE_MMS_JAVA || msgType==MSG_TYPE_MMS)
						msgText = msg_mms_get_text_contents(msgList.msgInfo[i]);
					else
						msgText = msg_sms_get_message_body(msgList.msgInfo[i]);

					printf("[%02d]\t[%s %s %s %s] [%s]\t[%s]\nDate [%s]\nMessage Text [%s] \tMessage data size [%d]\n",
							msg_get_message_id(msgList.msgInfo[i]),
							MsgConvertMsgType(msg_get_message_type(msgList.msgInfo[i])),
							MsgConvertStorageId(msg_get_storage_id(msgList.msgInfo[i])),
							MsgConvertReadStatus(msg_is_read(msgList.msgInfo[i])),
							MsgConvertProtectedStatus(msg_is_protected(msgList.msgInfo[i])),
							MsgConvertNetworkStatus(msg_get_network_status(msgList.msgInfo[i])),
							msg_get_ith_address(msgList.msgInfo[i], 0),
							displayTime,
							msgText,
							msg_get_message_body_size(msgList.msgInfo[i]));
					printf("--------------------------------------------------------\n");
				}
			}

			print("======================================");
			print("================ Menu ================");
			print("======================================");
			print("[S] Search Again");
			print("[B] Back");
			print("======================================");

			print("Input : ");

			memset(menu, 0x00, sizeof(menu));
			cin.getline(menu, 5);

			if (!strcmp(menu, "S") || !strcmp(menu, "s"))
			{
				continue;
			}
			else if (!strcmp(menu, "B") || !strcmp(menu, "b"))
			{
				msg_release_message_list(&msgList);
				MSG_DEBUG("release msg list [%d]", msgList.nCount);
				break;
			}
			else
			{
				int msgId = atoi(menu);

				MsgTestGetMessage(hMsgHandle, msgId);
				break;
			}

			msg_release_message_list(&msgList);
			MSG_DEBUG("release msg list [%d]", msgList.nCount);
		}
		else if(searchType == 1)
		{

			MSG_THREAD_VIEW_LIST_S threadViewList;

		MSG_PROFILE_BEGIN(1);

			err = msg_search_message_for_thread_view(hMsgHandle, searchString, &threadViewList);

		MSG_PROFILE_END(1);

		if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD)
		{
			print("Get Message List is failed!");
			return;
		}

		system ("clear" );

		print("======================================");
		print("============ Search View =============");
		print("======================================");

		if (threadViewList.nCount <= 0)
		{
			print("Empty...");
		}
		else
		{

			for (int i = 0; i < threadViewList.nCount; i++)
			{
				memset(displayTime, 0x00, sizeof(displayTime));

				MsgConvertTime(msg_thread_view_get_time(threadViewList.msgThreadInfo[i]), displayTime);

				printf("[%04d]\tUnread Msg [%d]\t[%s] [%s] [%s] [%s]\n[%s]\tMessage Text [%s]\n",
						msg_thread_view_get_thread_id(threadViewList.msgThreadInfo[i]),
						msg_thread_view_get_unread_cnt(threadViewList.msgThreadInfo[i]),
						msg_thread_view_get_address(threadViewList.msgThreadInfo[i]),
						msg_thread_view_get_name(threadViewList.msgThreadInfo[i]),
						msg_thread_view_get_image_path(threadViewList.msgThreadInfo[i]),
						displayTime,
						MsgConvertDirection(msg_thread_view_get_direction(threadViewList.msgThreadInfo[i])),
						msg_thread_view_get_data(threadViewList.msgThreadInfo[i]));

				printf("--------------------------------------------------------\n");
			}
		}

		print("======================================");
		print("================ Menu ================");
		print("======================================");
		print("[C] Create Message");
		print("[D] Delete Thread");
		print("[S] Search Again");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 5);

		if (!strcmp(menu, "C") || !strcmp(menu, "c")) // Add Message
		{
			MsgTestAddMessage(hMsgHandle);
		}
		else if (!strcmp(menu, "D") || !strcmp(menu, "d"))
		{
			print("Choose Thread ID : ");

			char id[5];

			memset(id, 0x00, sizeof(id));
			cin.getline(id, 5);

			print("Do you really wanna delete messages in thread [Y or N] ?");

			char select[2];

			memset(select, 0x00, sizeof(select));
			cin.getline(select, 2);

			if (!strcmp(select, "Y") || !strcmp(select, "y"))
				msg_delete_thread_message_list(hMsgHandle, (MSG_THREAD_ID_T)atoi(id));
		}
		else if (!strcmp(menu, "S") || !strcmp(menu, "s"))
		{
			continue;
		}
		else if (!strcmp(menu, "B") || !strcmp(menu, "b"))
		{
			msg_release_thread_view_list(&threadViewList);
			MSG_DEBUG("release thread view list [%d]", threadViewList.nCount);
			break;
		}
		else
		{
			MSG_THREAD_ID_T threadId = atoi(menu);

			for (int i = 0; i < threadViewList.nCount; i++)
			{
				if ((MSG_THREAD_ID_T)msg_thread_view_get_thread_id(threadViewList.msgThreadInfo[i]) == threadId)
				{
					MsgRunConversationView(hMsgHandle, threadId, msg_thread_view_get_address(threadViewList.msgThreadInfo[i]), msg_thread_view_get_name(threadViewList.msgThreadInfo[i]));

					break;
				}
			}
		}

		msg_release_thread_view_list(&threadViewList);
		MSG_DEBUG("release thread view list [%d]", threadViewList.nCount);
	}
	}
	while (strcmp(menu, "B"));
}


void MsgRunConversationView(MSG_HANDLE_T hMsgHandle, MSG_THREAD_ID_T ThreadId, const char *pAddress, const char *pName)
{
	char menu[5];

	char displayTime[32];

	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_LIST_S convViewList;

	do
	{
		err = msg_get_conversation_view_list(hMsgHandle, ThreadId, &convViewList);

		if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD)
		{
			print("Get Message List is failed!");
			return;
		}

		system ("clear" );

		printf("======================================\n");
		printf("============ %s ============\n", pAddress);
		printf("============ %s ============\n", pName);
		printf("======================================\n");

		if (convViewList.nCount <= 0)
		{
			print("Empty...");
		}
		else
		{
			for (int i = 0; i < convViewList.nCount; i++)
			{
				memset(displayTime, 0x00, sizeof(displayTime));

				MsgConvertTime(msg_get_time(convViewList.msgInfo[i]), displayTime);

				const char* msgText = NULL;

				int msgType = msg_get_message_type(convViewList.msgInfo[i]);

				if(msgType==MSG_TYPE_MMS_NOTI || msgType==MSG_TYPE_MMS_JAVA || msgType==MSG_TYPE_MMS)
					msgText = msg_mms_get_text_contents(convViewList.msgInfo[i]);
				else
					msgText = msg_sms_get_message_body(convViewList.msgInfo[i]);

				printf("[%04d]\t[%s] [%s]\tText [%s] Attachment count [%d] Date [%s]\n",
						msg_get_message_id(convViewList.msgInfo[i]),
						MsgConvertDirection(msg_get_direction_info(convViewList.msgInfo[i])),
						MsgConvertMsgType(msg_get_message_type(convViewList.msgInfo[i])),
						msgText,
						msg_get_attachment_count(convViewList.msgInfo[i]),
						displayTime);

				printf("--------------------------------------------------------\n");
			}
		}

		print("======================================");
		print("================ Menu ================");
		print("======================================");
		print("[R] Reply");
		print("[D] Delete Message");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 5);

		if (!strcmp(menu, "R") || !strcmp(menu, "r"))
		{

		}
		else if (!strcmp(menu, "D") || !strcmp(menu, "d"))
		{

		}

		msg_release_message_list(&convViewList);
		MSG_DEBUG("release conversation view list [%d]", convViewList.nCount);
	}
	while (strcmp(menu, "B") && strcmp(menu, "b"));
}


const char* MsgConvertDirection(MSG_DIRECTION_TYPE_T Direction)
{
	if (Direction == MSG_DIRECTION_TYPE_MO)
		return "SENT";
	else if (Direction == MSG_DIRECTION_TYPE_MT)
		return "RECEIVED";

	return "RECEIVED";
}


const char* MsgConvertType(MSG_MESSAGE_TYPE_T MsgType)
{
	if (MsgType == MSG_TYPE_SMS)
		return "SMS";
	else if (MsgType == MSG_TYPE_SMS_CB ||MsgType == MSG_TYPE_SMS_JAVACB)
		return "CB";
	else if (MsgType == MSG_TYPE_SMS_WAPPUSH)
		return "WAP Push";
	else if (MsgType == MSG_TYPE_SMS_MWI)
		return "MWI";
	else if (MsgType == MSG_TYPE_SMS_SYNCML)
		return "SyncML";
	else if (MsgType == MSG_TYPE_SMS_REJECT)
		return "Reject SMS";
	else if (MsgType == MSG_TYPE_MMS)
		return "MMS";
	else if (MsgType == MSG_TYPE_MMS_NOTI)
		return "MMS Noti";
	else if (MsgType == MSG_TYPE_MMS_JAVA)
		return "Java MMS";

	return "SMS";
}

