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
#include "MsgSettingTypes.h"
#include "MapiStorage.h"
#include "MapiSetting.h"
#include "MapiMessage.h"
#include "MapiTransport.h"
#include "MsgTestStorage.h"
#include "MsgTestTransport.h"
#include "main.h"

MSG_FOLDER_LIST_S g_folderList;

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
void MsgTestAddMessage(MSG_HANDLE_T hMsgHandle)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	// Make Message
	msg_message_t msgInfo = msg_new_message();
	MSG_SENDINGOPT_S sendOpt = {0, };

	print("\n===== Input Message =====");
	print("Input Message Type 0:SMS 1:MMS :");
	unsigned int msgType;
	msgType = cin.get();	//&msgType, 1);
	cin.get(); // consume "enter key"

	if( msgType == '0' )
	{
		msg_set_message_type(msgInfo, MSG_TYPE_SMS);
	}
	else if ( msgType == '1' )
	{
		msg_set_message_type(msgInfo, MSG_TYPE_MMS);
	}
	else
	{
		MSG_DEBUG("Msg Type input error %d", msgType);
	}

	print("\nInput Message Sending Option ?. 'Y' : Yes 'N' : No  ");

	char bSendOpt = 0, ch = 0;
	cin >> bSendOpt;
	ch = cin.get();

	if( bSendOpt == 'Y' || bSendOpt == 'y')
	{
		sendOpt.bSetting = true;

		/* EX : Set SendingOption */
		if( msg_is_sms(msgInfo) ) //msgInfo.msgType.mainType == MSG_SMS_TYPE)
		{
			print("\nRequest Delivery Report? Press 'Y' or 'N' :");
			char bDelivery = 0;
			ch = 0;
			cin >> bDelivery;
			ch = cin.get();

			if( bDelivery == 'Y' || bDelivery == 'y')
				sendOpt.bDeliverReq = true;
			else
				sendOpt.bDeliverReq = false;

			print("\nKeep a Copy? Press 'Y' or 'N' :");
			char bKeepCopy = 0;
			ch = 0;
			cin >> bKeepCopy;
			ch = cin.get();

			if( bKeepCopy == 'Y' || bKeepCopy == 'y')
				sendOpt.bKeepCopy = true;
			else
				sendOpt.bKeepCopy = false;

			print("\nSet Reply Path? Press 'Y' or 'N' :");
			char bReplyPath = 0;
			ch = 0;
			cin >> bReplyPath;
			ch = cin.get();

			if( bReplyPath == 'Y' || bReplyPath == 'y')
				sendOpt.option.smsSendOpt.bReplyPath = true;
			else
				sendOpt.option.smsSendOpt.bReplyPath = false;
		}
		else if( msg_is_mms(msgInfo) )//msgInfo.msgType.mainType == MSG_MMS_TYPE)
		{
			print("\nRequest Delivery Report? Press 'Y' or 'N' :");
			char bDelivery = 0;
			ch = 0;
			cin >> bDelivery;
			ch = cin.get();

			if( bDelivery == 'Y' || bDelivery == 'y')
				sendOpt.bDeliverReq = true;
			else
				sendOpt.bDeliverReq = false;

			print("\nKeep a Copy? Press 'Y' or 'N' :");
			char bKeepCopy = 0;
			ch = 0;
			cin >> bKeepCopy;
			ch = cin.get();

			if( bKeepCopy == 'Y' || bKeepCopy == 'y')
				sendOpt.bKeepCopy = true;
			else
				sendOpt.bKeepCopy = false;

			print("\nRequest Read Report? Press 'Y' or 'N' :");
			char bRead = 0;
			ch = 0;
			cin >> bRead;
			ch = cin.get();

			if( bRead == 'Y' || bRead == 'y')
				sendOpt.option.mmsSendOpt.bReadReq = true;
			else
				sendOpt.option.mmsSendOpt.bReadReq = false;

			print("\nSet Priority? 'U' : Ugent, 'N' : Normal, 'L' : Low");
			char priority = 0;
			ch = 0;
			cin >> priority;
			ch = cin.get();

			if( priority == 'U' || priority == 'u')
				sendOpt.option.mmsSendOpt.priority = MSG_MESSAGE_PRIORITY_HIGH;
			else if( priority == 'N' || priority == 'n')
				sendOpt.option.mmsSendOpt.priority = MSG_MESSAGE_PRIORITY_NORMAL;
			else if( priority == 'L' || priority == 'l')
				sendOpt.option.mmsSendOpt.priority = MSG_MESSAGE_PRIORITY_LOW;

			print("\nSet Expiry Time? '0' : Max '1' : 1 Day, '2' : 2 Days, '3' : 1 Week, '4' : 2 Weeks ");
			char expiryTime = 0;
			ch = 0;
			cin >> expiryTime;
			ch = cin.get();

			if( expiryTime == '0')
				sendOpt.option.mmsSendOpt.expiryTime = MSG_EXPIRY_TIME_MAXIMUM;
			else if( expiryTime == '1')
				sendOpt.option.mmsSendOpt.expiryTime = MSG_EXPIRY_TIME_1DAY;
			else if( expiryTime == '2')
				sendOpt.option.mmsSendOpt.expiryTime = MSG_EXPIRY_TIME_2DAYS;
			else if( expiryTime == '3')
				sendOpt.option.mmsSendOpt.expiryTime = MSG_EXPIRY_TIME_1WEEK;
			else if( expiryTime == '4')
				sendOpt.option.mmsSendOpt.expiryTime = MSG_EXPIRY_TIME_2WEEKS;

			print("\nSet Delivery Time? '0' : Immediately '1' : 1 Hour, '2' : 1 Day, '3' : 1 Week ");
			char deliveryTime = 0;
			ch = 0;
			cin >> deliveryTime;
			ch = cin.get();

			if( deliveryTime == '0')
				sendOpt.option.mmsSendOpt.deliveryTime = MSG_DELIVERY_TIME_IMMEDIATLY;
			else if( deliveryTime == '1')
				sendOpt.option.mmsSendOpt.deliveryTime = MSG_DELIVERY_TIME_1HOUR;
			else if( deliveryTime == '2')
				sendOpt.option.mmsSendOpt.deliveryTime = MSG_DELIVERY_TIME_1DAY;
			else if( deliveryTime == '3')
				sendOpt.option.mmsSendOpt.deliveryTime = MSG_DELIVERY_TIME_1WEEK;

			MSG_DEBUG("sendOpt.option.mmsSendOpt.deliveryTime = %lu", sendOpt.option.mmsSendOpt.deliveryTime);
		}
	}
	else		/* In case of No setting per Message, Set Message option with Global Setting value */
	{
		sendOpt.bSetting = false;
	}

	MSG_DEBUG("### bSetting = %d ###", sendOpt.bSetting);
	MSG_DEBUG("MsgType [%d]", msg_get_message_type(msgInfo));

	if(msg_is_sms(msgInfo)) //msgInfo.msgType.mainType == MSG_SMS_TYPE)
	{
		print("\n===== Input Message =====");
		print("Input Message Text :");

		char strMsg[1200];
		memset(strMsg, 0x00, sizeof(strMsg));

//		strcpy(strMsg, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
//		strcpy(strMsg, "ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNO");
//		strcpy(strMsg, "ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNO ABCDEFGHIJKLMNO ABCDEFGHIJKLMNO ABCDEFGHIJKLMNO ABCDEFGHIJKLMNO ABCDEFGHIJKLMNO");
//		strcpy(strMsg, "AAAAAAAAAAAAAAAAAAAAAAAAAAA AAAAAAAAAAAAAAAAAAAAAAAAAAA AAAAAAAAAAAAAAAAAAAAAAAAAAA AAAAAAAAAAAAAAAAAAAAAAAAAAA AAAAAAAAAAAAAAAAAAAAAAAAAAA AAAAAAAAAAAAAAAAAAAAAAAAAAA AAAAAAAAAAAAAAAAAAAAAAAAAAA");

		cin.getline(strMsg, 1024);

		if (msg_sms_set_message_body(msgInfo, strMsg, strlen(strMsg)) != MSG_SUCCESS)
		{
			return;
		}
	}

	else if(msg_is_mms(msgInfo)) //msgInfo.msgType.mainType == MSG_MMS_TYPE)
	{
		// Compose MMS Message Body (Text, Image, Sound, Video)
		//MMS_MESSAGE_DATA_S	msgData ={0, };
		MMS_MESSAGE_DATA_S*	 mms_data;//msgHandle;
		MMS_PAGE_S* page[2];
		MMS_MEDIA_S* media[5];
		MMS_ATTACH_S* attachment[5];
		int nSize = 0;

		print("\n===== Input Subject =====");

		char 					strSubject[120];
		int						subjectLen;
		memset(strSubject, 0x00, sizeof(strSubject));

		cin.getline(strSubject, 120);
		subjectLen = strlen(strSubject);

		msg_set_subject(msgInfo, strSubject);

		mms_data = msg_mms_create_message();

		print("\n===== Input Message =====");
		print("Input Multipart type 0: Multiprat Related, 1:Multipart Mixed");

		unsigned int multipartType;
		multipartType = cin.get();
		cin.get(); // consume "enter key"

		if(multipartType == '0')
		{
			msg_mms_set_rootlayout(mms_data, 100, 100, 0xffffff);
			msg_mms_add_region(mms_data, "Image", 0, 50, 100, 50, 0xffffff);
			msg_mms_add_region(mms_data, "Text", 0, 0, 100, 50, 0xffffff);

			//------------>  1st Slide Composing
			page[0] = msg_mms_add_page(mms_data, 5440);

			media[0] = msg_mms_add_media(page[0], MMS_SMIL_MEDIA_IMG, "Image", (char*)"/opt/etc/msg-service/P091120_104633.jpg");
			media[1] = msg_mms_add_media(page[0], MMS_SMIL_MEDIA_AUDIO, NULL, (char*)"/opt/etc/msg-service/audio.amr");
			media[2] = msg_mms_add_media(page[0], MMS_SMIL_MEDIA_TEXT, "Text", (char*)"/opt/etc/msg-service/Temp0_2.txt");
			media[2]->sMedia.sText.nColor = 0x000000;
			media[2]->sMedia.sText.nSize = MMS_SMIL_FONT_SIZE_NORMAL;
			media[2]->sMedia.sText.bBold = true;

			//------------>  2nd Slide Composing
			page[1] = msg_mms_add_page(mms_data, 4544);

			media[3] = msg_mms_add_media(page[1], MMS_SMIL_MEDIA_TEXT, "Text", (char*)"/opt/etc/msg-service/Temp1_0.txt");
			media[3]->sMedia.sText.nColor = 0x000000;
			media[3]->sMedia.sText.nSize = MMS_SMIL_FONT_SIZE_NORMAL;
			media[3]->sMedia.sText.bItalic = true;
			media[4] = msg_mms_add_media(page[1], MMS_SMIL_MEDIA_VIDEO, "Text", (char*)"/opt/etc/msg-service/V091120_104905.3gp");
			strncpy(media[4]->szAlt, "Video Load Fail", MAX_SMIL_ALT_LEN-1);
		}
		else
		{
			attachment[0] = msg_mms_add_attachment(mms_data, (char*)"/opt/etc/msg-service/P091120_104633.jpg");
			attachment[1] = msg_mms_add_attachment(mms_data, (char*)"/opt/etc/msg-service/audio.amr");
			attachment[2] = msg_mms_add_attachment(mms_data, (char*)"/opt/etc/msg-service/Temp0_2.txt");
			attachment[3] = msg_mms_add_attachment(mms_data, (char*)"/opt/etc/msg-service/Temp1_0.txt");
			attachment[4] = msg_mms_add_attachment(mms_data, (char*)"/opt/etc/msg-service/V091120_104905.3gp");
		}

		MSG_DEBUG("nSize = %d",  nSize);

		msg_mms_set_message_body(msgInfo, mms_data);

		msg_mms_release_page_list(mms_data);
		msg_mms_release_region_list(mms_data);
		msg_mms_release_attachment_list(mms_data);
		msg_mms_release_transition_list(mms_data);
		msg_mms_release_meta_list(mms_data);

		msg_mms_destroy_message(mms_data);
	}

	int nToCnt = 0;

	char strNumber[MAX_ADDRESS_VAL_LEN];
	memset(strNumber, 0x00, MAX_ADDRESS_VAL_LEN);

	print("\n===== Add Recipient =====");

	while (nToCnt < MAX_TO_ADDRESS_CNT)
	{
		print("\nInput Recipient Number. Press 'N' if you don't want : ");

		cin.getline(strNumber, 1024);

		if (!strcmp(strNumber, "N")) break;

		MSG_DEBUG("strNumber [%s]", strNumber);

		if(msg_is_mms(msgInfo)) // in case of MMS
		{
			print("\n===== Input Recipient Type =====");
			print("Input Message Type 0:To 1:Cc 2: Bcc :");

			unsigned int recipientType;
			recipientType = cin.get();	//&msgType, 1);
			cin.get(); // consume "enter key"

			if(recipientType == '0')
				msg_add_address(msgInfo, strNumber, MSG_RECIPIENTS_TYPE_TO);
			else if(recipientType == '1')
				msg_add_address(msgInfo, strNumber, MSG_RECIPIENTS_TYPE_CC);
			else if(recipientType == '2')
				msg_add_address(msgInfo, strNumber, MSG_RECIPIENTS_TYPE_BCC);
			else
				MSG_DEBUG("Recipient Type input error %d", recipientType);
		}
		else // in case of SMS
			msg_add_address(msgInfo, strNumber, MSG_RECIPIENTS_TYPE_TO);

		nToCnt++;
	}

	MSG_DEBUG("nToContactCnt [%d]", msg_get_address_count(msgInfo));

	if(msg_is_sms(msgInfo))
	{
		char strReplyAddr[MAX_PHONE_NUMBER_LEN];
		memset(strReplyAddr, 0x00, MAX_PHONE_NUMBER_LEN);

		print("\n===== Set Reply Address =====");

		print("\nInput Reply Number. Press 'N' if you don't want : ");

		cin.getline(strReplyAddr, 1024);

		if (!strcmp(strReplyAddr, "N"))
		{
			print("\nReply Number is not set.");
		}
		else
		{
			MSG_DEBUG("strReplyAddr [%s]", strReplyAddr);

			msg_set_reply_address(msgInfo, strReplyAddr);
		}

		// for test
//		msg_set_backup_status(msgInfo, true);
	}

	// Port Number Test
//	msg_set_port(msgInfo, 4010, 0);

	print("\n======================================");
	print("[1] Send Message");
	print("[2] Save Message into Draft");
	print("[3] Scheduled Message Send");
	print("======================================");
	print("Input : ");

	int select = 0;
	char ch1 = 0;

	cin >> select;
	ch1 = cin.get();

	if (select == 1)
	{
		MsgTestSubmitReq(hMsgHandle, msgInfo, &sendOpt);
	}
	else if (select == 2)
	{
		print("Start Saving Message...");

		int MsgId = 0;

		MsgId = msg_add_message(hMsgHandle, msgInfo, &sendOpt);

		MSG_DEBUG("Saving Message is Finished![%d]", MsgId);
	}

	else if (select == 3)
	{
		MsgTestScheduledSubmitReq(hMsgHandle, msgInfo, &sendOpt);
	}

	msg_release_message(&msgInfo);
}

void MsgTestMsgGen(MSG_HANDLE_T hMsgHandle)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	char menu[5];

	print("Input Message Count: ");

	memset(menu, 0x00, sizeof(menu));
	cin.getline(menu, 5);

	unsigned int count = atoi(menu);

	print("Input Message Type(SMS[1], MMS[2]): ");

	memset(menu, 0x00, sizeof(menu));
	cin.getline(menu, 5);

	MSG_MESSAGE_TYPE_T msgType = MSG_TYPE_INVALID;

	if (atoi(menu) == 1)
		msgType = MSG_TYPE_SMS;
	else if (atoi(menu) == 2)
		msgType = MSG_TYPE_MMS;

	print("Input folder(INBOX[1], OUTBOX[2], SENTBOX[3], DRAFTBOX[4]): ");

	memset(menu, 0x00, sizeof(menu));
	cin.getline(menu, 5);

	MSG_FOLDER_ID_T folderId = atoi(menu);

	MSG_ERROR_T err = msg_generate_message(hMsgHandle, msgType, folderId, count);

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("err [%d]", err);
	}
}


void MsgTestGetMessage(MSG_HANDLE_T hMsgHandle, int MsgId)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	msg_message_t msg = msg_new_message();
	MSG_SENDINGOPT_S sendOpt = {0, };

	err = msg_get_message(hMsgHandle, (MSG_MESSAGE_ID_T)MsgId, msg, &sendOpt);

	if (err != MSG_SUCCESS)
	{
		msg_release_message(&msg);
		return;
	}

	// Update Read Status
	msg_update_read_status(hMsgHandle, MsgId, true);

	// Send Read Report
	if (msg_is_mms(msg))
	{
		int folder_id = msg_get_folder_id(msg);
		if (folder_id == MSG_INBOX_ID) {
			msg_mms_send_read_report(hMsgHandle, MsgId, MSG_READ_REPORT_IS_READ);
		}
	}

	MsgPrintMessage(hMsgHandle, msg, &sendOpt);

	msg_release_message(&msg);
}


void MsgTestGetMessageList(MSG_HANDLE_T hMsgHandle, int FolderId)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	char menu[5];

	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_SORT_RULE_S sortRule = {0};

	sortRule.sortType		= MSG_SORT_BY_READ_STATUS;
	sortRule.bAscending	= false;

	MSG_LIST_S folderViewList;

	MSG_MESSAGE_TYPE_T MsgType;
	MSG_NETWORK_STATUS_T NetworkStatus;

	char displayTime[32];
	memset(displayTime, 0x00, sizeof(displayTime));

	do
	{
		err = msg_get_folder_view_list(hMsgHandle, (MSG_FOLDER_ID_T)FolderId, NULL, &folderViewList);

		if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD)
		{
			print("Get Message List is failed!");
			return;
		}

		system ("clear" );

		printf("======================================\n");
		for (int i = 0; i < g_folderList.nCount; i++)
		{
			if (FolderId == g_folderList.folderInfo[i].folderId)
			{
				printf("=============== %s ===============\n", g_folderList.folderInfo[i].folderName);
				break;
			}
		}
		printf("======================================\n");

		if (folderViewList.nCount <= 0)
		{
			printf("Empty...\n");
		}
		else
		{
			MSG_PROFILE_BEGIN(2);

			for (int i = 0; i < folderViewList.nCount; i++)
			{
				memset(displayTime, 0x00, sizeof(displayTime));

				MsgConvertTime(msg_get_time(folderViewList.msgInfo[i]), displayTime);

				const char* msgText = NULL;

				int msgType = msg_get_message_type(folderViewList.msgInfo[i]);

				if(msgType==MSG_TYPE_MMS_NOTI || msgType==MSG_TYPE_MMS_JAVA || msgType==MSG_TYPE_MMS)
					msgText = msg_mms_get_text_contents(folderViewList.msgInfo[i]);
				else
					msgText = msg_sms_get_message_body(folderViewList.msgInfo[i]);

				printf("[%02d]\t[%s %s %s %s] [%s]\t[%s]\nDate [%s]\nMessage Text [%s] \tMessage data size [%d]\n",
						msg_get_message_id(folderViewList.msgInfo[i]),
						MsgConvertMsgType(msg_get_message_type(folderViewList.msgInfo[i])),
						MsgConvertStorageId(msg_get_storage_id(folderViewList.msgInfo[i])),
						MsgConvertReadStatus(msg_is_read(folderViewList.msgInfo[i])),
						MsgConvertProtectedStatus(msg_is_protected(folderViewList.msgInfo[i])),
						MsgConvertNetworkStatus(msg_get_network_status(folderViewList.msgInfo[i])),
						msg_get_ith_address(folderViewList.msgInfo[i], 0),
						displayTime,
						msgText,
						msg_get_message_body_size(folderViewList.msgInfo[i]));
				printf("--------------------------------------------------------\n");
			}

			MSG_PROFILE_END(2);
		}

		printf("======================================\n");
		printf("================ Menu ================\n");
		printf("======================================\n");
		printf("[C] Create Message\n");
		printf("[D] Delete All Messages\n");
		printf("[B] Back\n");
		printf("======================================\n");

		printf("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 5);

		if (!strcmp(menu, "C") || !strcmp(menu, "c")) // Add Message
		{
			MsgTestAddMessage(hMsgHandle);
		}
		else if (!strcmp(menu, "D") || !strcmp(menu, "d"))
		{
			char menu[2];

			printf("Do you really wanna delete all messages [Y or N] ?\n");

			memset(menu, 0x00, sizeof(menu));
			cin.getline(menu, 2);

			if (!strcmp(menu, "Y"))
				msg_delete_all_msgs_in_folder(hMsgHandle, (MSG_FOLDER_ID_T)FolderId, false);
		}
		else if (!strcmp(menu, "B") || !strcmp(menu, "b"))
		{
			break;
		}
		else
		{
			int msgId = atoi(menu);

			for (int i = 0; i < folderViewList.nCount; i++)
			{
				if ( msg_get_message_id( folderViewList.msgInfo[i] ) == msgId)
				{
					MsgType = msg_get_message_type(folderViewList.msgInfo[i]);
					NetworkStatus = msg_get_network_status(folderViewList.msgInfo[i]);

					MsgRunMsgListMenu(hMsgHandle, msgId, FolderId, MsgType, NetworkStatus);

					break;
				}
			}
		}

		msg_release_message_list(&folderViewList);
		printf("release folderview list [%d]\n", folderViewList.nCount);
	}
	while (strcmp(menu, "B"));
}


void MsgTestUpdateMessage(MSG_HANDLE_T hMsgHandle, msg_message_t pMsg)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_SENDINGOPT_S sendOpt = {0, };

	print("\n===== Input Message =====");
	print("Input Message Text :");

	char strMsg[1024] = {0};

//	strcpy(strMsg, "ABCDEFGHIGKLMNOPQRSTUVWXYZ ABCDEFGHIGKLMNOPQRSTUVWXYZ ABCDEFGHIGKLMNOPQRSTUVWXYZ ABCDEFGHIGKLMNOPQRSTUVWXYZ ABCDEFGHIGKLMNOPQRSTUVWXYZ ABCDEFGHIGKLMNOPQRSTUVWXYZ");
	cin.getline(strMsg, 1024);

	msg_sms_set_message_body(pMsg, strMsg, strlen(strMsg));

	int nToCnt = msg_get_address_count(pMsg);

	char strNumber[MAX_ADDRESS_VAL_LEN] = {0};

	printf("\n===== Add Recipient =====\n");

	MSG_DEBUG("address count [%d]", nToCnt);

	if (nToCnt > 0)
	{
		msg_reset_address(pMsg);
	}

	printf("\nInput Recipient Number. Press 'N' if you don't want : \n");

	cin.getline(strNumber, 1024);

	if (strcmp(strNumber, "N"))
	{
		MSG_DEBUG("strNumber [%s]", strNumber);

		msg_add_address(pMsg, strNumber, MSG_RECIPIENTS_TYPE_TO);

		MSG_DEBUG("contactVal [%s]", msg_get_ith_address(pMsg, 0));

		MSG_DEBUG("nToContactCnt [%d]", msg_get_address_count(pMsg));
	}

	print("\nInput Message Sending Option ?. 'Y' : Yes 'N' : No  ");

	char bSendOpt = 0, ch = 0;
	cin >> bSendOpt;
	ch = cin.get();

	if( bSendOpt == 'Y' || bSendOpt == 'y')
	{
		sendOpt.bSetting = true;

		/* EX : Set SendingOption */
		if( msg_is_sms(pMsg) )
		{
			printf("\nRequest Delivery Report? Press 'Y' or 'N' :\n");
			char bDelivery = 0;
			ch = 0;
			cin >> bDelivery;
			ch = cin.get();

			if( bDelivery == 'Y' || bDelivery == 'y')
				sendOpt.bDeliverReq = true;
			else
				sendOpt.bDeliverReq = false;

			print("\nKeep a Copy? Press 'Y' or 'N' :");
			char bKeepCopy = 0;
			ch = 0;
			cin >> bKeepCopy;
			ch = cin.get();

			if( bKeepCopy == 'Y' || bKeepCopy == 'y')
				sendOpt.bKeepCopy = true;
			else
				sendOpt.bKeepCopy = false;

			print("\nSet Reply Path? Press 'Y' or 'N' :");
			char bReplyPath = 0;
			ch = 0;
			cin >> bReplyPath;
			ch = cin.get();

			if( bReplyPath == 'Y' || bReplyPath == 'y')
				sendOpt.option.smsSendOpt.bReplyPath = true;
			else
				sendOpt.option.smsSendOpt.bReplyPath = false;
		}
		else if( msg_is_mms(pMsg) )
		{
			print("\nRequest Delivery Report? Press 'Y' or 'N' :");
			char bDelivery = 0;
			ch = 0;
			cin >> bDelivery;
			ch = cin.get();

			if( bDelivery == 'Y' || bDelivery == 'y')
				sendOpt.bDeliverReq = true;
			else
				sendOpt.bDeliverReq = false;

			print("\nKeep a Copy? Press 'Y' or 'N' :");
			char bKeepCopy = 0;
			ch = 0;
			cin >> bKeepCopy;
			ch = cin.get();

			if( bKeepCopy == 'Y' || bKeepCopy == 'y')
				sendOpt.bKeepCopy = true;
			else
				sendOpt.bKeepCopy = false;

			print("\nRequest Read Report? Press 'Y' or 'N' :");
			char bRead = 0;
			ch = 0;
			cin >> bRead;
			ch = cin.get();

			if( bRead == 'Y' || bRead == 'y')
				sendOpt.option.mmsSendOpt.bReadReq = true;
			else
				sendOpt.option.mmsSendOpt.bReadReq = false;

			print("\nSet Priority? 'U' : Ugent, 'N' : Normal, 'L' : Low");
			char priority = 0;
			ch = 0;
			cin >> priority;
			ch = cin.get();

			if( priority == 'U' || priority == 'u')
				sendOpt.option.mmsSendOpt.priority = MSG_MESSAGE_PRIORITY_HIGH;
			else if( priority == 'N' || priority == 'n')
				sendOpt.option.mmsSendOpt.priority = MSG_MESSAGE_PRIORITY_NORMAL;
			else if( priority == 'L' || priority == 'l')
				sendOpt.option.mmsSendOpt.priority = MSG_MESSAGE_PRIORITY_LOW;

			print("\nSet Expiry Time? '0' : Max '1' : 1 Day, '2' : 2 Days, '3' : 1 Week, '4' : 2 Weeks ");
			char expiryTime = 0;
			ch = 0;
			cin >> expiryTime;
			ch = cin.get();

			if( expiryTime == '0')
				sendOpt.option.mmsSendOpt.expiryTime = MSG_EXPIRY_TIME_MAXIMUM;
			else if( expiryTime == '1')
				sendOpt.option.mmsSendOpt.expiryTime = MSG_EXPIRY_TIME_1DAY;
			else if( expiryTime == '2')
				sendOpt.option.mmsSendOpt.expiryTime = MSG_EXPIRY_TIME_2DAYS;
			else if( expiryTime == '3')
				sendOpt.option.mmsSendOpt.expiryTime = MSG_EXPIRY_TIME_1WEEK;
			else if( expiryTime == '4')
				sendOpt.option.mmsSendOpt.expiryTime = MSG_EXPIRY_TIME_2WEEKS;

			print("\nSet Delivery Time? '0' : Immediately '1' : 1 Hour, '2' : 1 Day, '3' : 1 Week ");
			char deliveryTime = 0;
			ch = 0;
			cin >> deliveryTime;
			ch = cin.get();

			if( expiryTime == '0')
				sendOpt.option.mmsSendOpt.deliveryTime = MSG_DELIVERY_TIME_IMMEDIATLY;
			else if( expiryTime == '1')
				sendOpt.option.mmsSendOpt.deliveryTime = MSG_DELIVERY_TIME_1HOUR;
			else if( expiryTime == '2')
				sendOpt.option.mmsSendOpt.deliveryTime = MSG_DELIVERY_TIME_1DAY;
			else if( expiryTime == '3')
				sendOpt.option.mmsSendOpt.deliveryTime = MSG_DELIVERY_TIME_1WEEK;

		}
	}
	else		/* In case of No setting per Message, Set Message option with Global Setting value */
	{
		sendOpt.bSetting = false;
	}

	err = msg_update_message(hMsgHandle, pMsg, &sendOpt);

	if (err == MSG_SUCCESS)
		print("Update Message is OK!");
	else
		print("Update Message is failed!");
}


void MsgTestMoveMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T MsgId)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	char strFolder[3];

	print("======================================");

	for (int i = 3; i < g_folderList.nCount; i++)
		printf("[%d] %s", g_folderList.folderInfo[i].folderId, g_folderList.folderInfo[i].folderName);

	print("======================================");

	print("Select Folder :");

	memset(strFolder, 0x00, sizeof(strFolder));
	cin.getline(strFolder, 3);

	int nFolder = atoi(strFolder);

	if (nFolder <= 3 || nFolder > g_folderList.nCount)
	{
		print("Select Wrong Folder!!!");
	}
	else
	{
		err = msg_move_msg_to_folder(hMsgHandle, MsgId, nFolder);

		if (err == MSG_SUCCESS)
			print("Moving Message is OK!");
		else
			print("Moving Message is failed!");
	}
}

void MsgTestMoveStorageMessage(MSG_HANDLE_T hMsgHandle, msg_message_t pmsg)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	if(msg_is_in_sim(pmsg))
		err = msg_move_msg_to_storage( hMsgHandle, msg_get_message_id(pmsg), MSG_STORAGE_PHONE);
	else
		err = msg_move_msg_to_storage( hMsgHandle, msg_get_message_id(pmsg), MSG_STORAGE_SIM);

	if (err == MSG_SUCCESS)
		print("Moving Message is OK!");
	else
		print("Moving Message is failed!");
}


void MsgTestAddFolder(MSG_HANDLE_T hMsgHandle)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	// Make Folder
	MSG_FOLDER_INFO_S folderInfo = {};

	folderInfo.folderId = 0;
	folderInfo.folderType = MSG_FOLDER_TYPE_USER_DEF;

	print("Input Folder Name :");

	char strName[MAX_FOLDER_NAME_SIZE+1];
	memset(strName, 0x00, sizeof(strName));

	cin.getline(strName, MAX_FOLDER_NAME_SIZE);

	strncpy(folderInfo.folderName, strName, MAX_FOLDER_NAME_SIZE);

	MSG_DEBUG("folderId [%d]", folderInfo.folderId);
	MSG_DEBUG("folderType [%d]", folderInfo.folderType);
	MSG_DEBUG("folderName [%s]", folderInfo.folderName);

	print("Start Creating New Folder...");

	err = msg_add_folder(hMsgHandle, &folderInfo);

	if (err == MSG_SUCCESS)
		print("Creating New Folder is OK!");
	else
		print("Creating New Folder is failed!");
}


void MsgTestUpdateFolder(MSG_HANDLE_T hMsgHandle)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	// Make Folder
	MSG_FOLDER_INFO_S folderInfo;

	print("Select Folder to Update :");

	char strId[3];
	memset(strId, 0x00, sizeof(strId));

	cin.getline(strId, 3);

	folderInfo.folderId = atoi(strId);

	if (folderInfo.folderId <= MSG_DRAFT_ID)
	{
		print("Wrong Folder ID!!");
		return;
	}

	folderInfo.folderType = g_folderList.folderInfo[folderInfo.folderId-1].folderType;

	print("Input New Folder Name :");

	char strName[MAX_FOLDER_NAME_SIZE+1];
	memset(strName, 0x00, sizeof(strName));

	cin.getline(strName, MAX_FOLDER_NAME_SIZE);

	strncpy(folderInfo.folderName, strName, MAX_FOLDER_NAME_SIZE);

	MSG_DEBUG("folderId [%d]", folderInfo.folderId);
	MSG_DEBUG("folderType [%d]", folderInfo.folderType);
	MSG_DEBUG("folderName [%s]", folderInfo.folderName);

	print("Start Updating Folder...");

	err = msg_update_folder(hMsgHandle, &folderInfo);

	if (err == MSG_SUCCESS)
		print("Updating Folder is OK!");
	else
		print("Updating Folder is failed!");
}


void MsgTestDeleteFolder(MSG_HANDLE_T hMsgHandle)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	print("Select Folder to Delete :");

	char strId[3];
	memset(strId, 0x00, sizeof(strId));

	cin.getline(strId, 3);

	MSG_FOLDER_ID_T folderId = atoi(strId);

	if (folderId <= MSG_DRAFT_ID)
	{
		print("Wrong Folder ID!!");
		return;
	}

	print("Start Deleting Folder...");

	err = msg_delete_folder(hMsgHandle, folderId);

	if (err == MSG_SUCCESS)
		print("Deleting Folder is OK!");
	else
		print("Deleting Folder is failed!");
}


void MsgTestDeleteMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T nMsgId)
{
	msg_mms_send_read_report(hMsgHandle, nMsgId, MSG_READ_REPORT_IS_DELETED);

	if (msg_delete_message(hMsgHandle, nMsgId) != MSG_SUCCESS)
		print("Failed to delete Message");
}


void MsgTestForwardMMSMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T nMsgId)
{
	msg_message_t msg = msg_new_message();

	MSG_SENDINGOPT_S sendOpt = {0};

	if (msg_get_message(hMsgHandle, (MSG_MESSAGE_ID_T)nMsgId, msg, &sendOpt) != MSG_SUCCESS)
	{
		print("Failed to get Message");
		msg_release_message(&msg);
		return;
	}

	// Update Read Status
	msg_update_read_status(hMsgHandle, msg_get_message_id(msg), true);

	// send read report
	msg_mms_send_read_report(hMsgHandle, nMsgId, MSG_READ_REPORT_IS_READ);

	// read address from stdin
	char strNumber[MAX_ADDRESS_VAL_LEN];

	memset(strNumber, 0x00, sizeof(strNumber));

	printf("\nInput Recipient Number. Press 'N' if you don't want : \n");

	cin.getline(strNumber, 1024);

	if (!strcmp(strNumber, "N"))
	{
		msg_release_message(&msg);
		return;
	}

	// write address
	msg_reset_address(msg);
	msg_add_address(msg, strNumber, MSG_RECIPIENTS_TYPE_TO);

	// write subject for forward msg
	char strSubject[MAX_SUBJECT_LEN] = {0};
	snprintf(strSubject, MAX_SUBJECT_LEN, "FW: %s", msg_get_subject(msg));
	msg_set_subject(msg, strSubject);

	// forward the message
	MSG_REQUEST_S req = {0};
	req.msg = msg;
	req.sendOpt = sendOpt;

	if (msg_mms_forward_message(hMsgHandle, &req) != MSG_SUCCESS)
		printf("Failed to Forward Message\n");

	msg_release_message(&msg);
}


void MsgTestRetrieveMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T nMsgId)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	msg_message_t msg = msg_new_message();
	MSG_SENDINGOPT_S sendOpt = {0};

	err = msg_get_message(hMsgHandle, nMsgId, msg, &sendOpt);

	msg_update_read_status(hMsgHandle, msg_get_message_id(msg), true);

	if( err != MSG_SUCCESS)
		print("Get Message Failed!");

	MSG_REQUEST_S req = {0, msg, sendOpt};

	err = msg_mms_retrieve_message(hMsgHandle, &req);

	if( err != MSG_SUCCESS)
		print("Retrieve MMS Message Failed!");
}

/* reject_msg_support */
void MsgTestRejectMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T nMsgId)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	msg_message_t msg = msg_new_message();
	MSG_SENDINGOPT_S sendOpt = {0, };

	err = msg_get_message(hMsgHandle, nMsgId, msg, &sendOpt);

	if( err != MSG_SUCCESS)
		print("Get Message Failed!");

	MSG_REQUEST_S req = {0};
	req.msg = msg;
	req.sendOpt = sendOpt;

	err = msg_mms_reject_message(hMsgHandle, &req);

	if( err != MSG_SUCCESS)
		print("Retrieve MMS Message Failed!");

	msg_release_message(&msg);
}
/* reject_msg_support */

void MsgTestUpdateMMSMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T nMsgId)
{
	MSG_DEBUG("Update MMS Message");

	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	msg_message_t msg = msg_new_message();

	MSG_SENDINGOPT_S sendOpt = {0};

	err = msg_get_message(hMsgHandle, (MSG_MESSAGE_ID_T)nMsgId, msg, &sendOpt);

	if (err != MSG_SUCCESS)
	{
		print("Get Message Failed!");
		msg_release_message(&msg);

		return;
	}

	MMS_MESSAGE_DATA_S *msgBody = msg_mms_create_message();

	msg_mms_get_message_body(msg, msgBody);

	msg_mms_add_attachment(msgBody, (char*)"/opt/etc/msg-service/P091120_104633.jpg");
	msg_mms_add_attachment(msgBody, (char*)"/opt/etc/msg-service/audio.amr");

	msg_mms_set_message_body(msg, msgBody);

	time_t curTime = time(NULL);
	msg_set_time(msg, curTime);
	msg_set_network_status(msg, MSG_NETWORK_NOT_SEND);
	msg_set_read_status(msg, false);
	msg_set_protect_status(msg, false);
	msg_set_direction_info(msg, MSG_DIRECTION_TYPE_MO);

	err= msg_update_message(hMsgHandle, msg, &sendOpt);

	msg_mms_destroy_message(msgBody);

	msg_release_message(&msg);
}


void MsgPrintMMSBody(msg_message_t pMsg)
{
	MMS_MESSAGE_DATA_S *msgBody = msg_mms_create_message();
	msg_mms_get_message_body(pMsg, msgBody);

	//Multipart Related
	if (msgBody->pageCnt)
	{
		printf("multipart type: Multipart Related\n");

		//Print root-layout info
		printf("\n** ROOT LAYOUT INFO **\n");
		printf("width: %d %s\nheight: %d %s\nbgColor:%x\n", msgBody->rootlayout.width.value, msgBody->rootlayout.width.bUnitPercent ? "%" : "",
			 msgBody->rootlayout.height.value,  msgBody->rootlayout.height.bUnitPercent ? "%" : "", msgBody->rootlayout.bgColor);

		// Print Region Info
		printf("\n** REGION INFO **\n");
		printf("Region Count: %d\n", msgBody->regionCnt);

		for(int i = 0; i < msgBody->regionCnt; ++i)
		{
			MMS_SMIL_REGION *pRegion = msg_mms_get_smil_region(msgBody, i);

			printf("%d region id: %s\n%d region left : %d %s\n%d region top : %d %s\n%d region width : %d %s\n%d region height : %d %s\n%d region bgColor : %x\n%d region fit : %s\n",
				i, pRegion->szID, i, pRegion->nLeft.value, pRegion->nLeft.bUnitPercent ? "%": "", i, pRegion->nTop.value, pRegion->nTop.bUnitPercent ? "%": "",
				i, pRegion->width.value, pRegion->width.bUnitPercent ? "%": "", i, pRegion->height.value, pRegion->height.bUnitPercent ? "%": "",
				i, pRegion->bgColor, i, (pRegion->fit == MMSUI_IMAGE_REGION_FIT_MEET) ? "MEET" : "HIDDEN");
		}

		printf("\n** PAGES & CONTENTS INFO **\n");
		printf("PAGE Count: %d\n", msgBody->pageCnt);

		// Print Page info
		for(int i = 0; i< msgBody->pageCnt; ++i)
		{
			MMS_PAGE_S *pPage = msg_mms_get_page(msgBody, i);

			printf("%d page's duration: %d msec \n%d page's media count: %d\n", i, pPage->nDur, i, pPage->mediaCnt);

			// Print Contents Info
			for(int j = 0; j < pPage->mediaCnt; ++j)
			{
				MMS_MEDIA_S *pMedia = msg_mms_get_media(pPage, j);
				printf("%d media's filename: %s\n%d media's filepath: %s\n%d media's regionId: %s\n Bold: %d\n Italic: %d\n", j, pMedia->szFileName, j, pMedia->szFilePath, j, pMedia->regionId, pMedia->sMedia.sText.bBold, pMedia->sMedia.sText.bItalic);

				if (pMedia->drmType != MSG_DRM_TYPE_NONE)
				{
					printf("%d media's drmtype: %d (1: Fowward Lock, 2:Combined Delivery, 3: Separated Delivery\n%d media's drmpath: %s\n", j, pMedia->drmType, j, pMedia->szDrm2FullPath);
				}
			}
			printf("\n");
		}
	}
	else
	{
		printf("multipart type: Multipart Mixed\n");
	}

	printf("Attachment Count: %d\n", msgBody->attachCnt);

	for(int i = 0; i < msgBody->attachCnt; ++i)
	{
		MMS_ATTACH_S *pAttach = msg_mms_get_attachment(msgBody, i);
		printf("Attachment file Name: %s\n", pAttach->szFileName);
		printf("Attachment file Path: %s\n", pAttach->szFilePath);
		printf("Attached file size: %d\n", pAttach->fileSize);

		if (pAttach->drmType != MSG_DRM_TYPE_NONE)
			printf("%d media's drmtype: %d (1: Fowward Lock, 2:Combined Delivery, 3: Separated Delivery\n%d media's drmpath: %s\n", i, pAttach->drmType, i, pAttach->szDrm2FullPath);
	}

	/*destroy and free message data*/
	msg_mms_destroy_message(msgBody);

}


void MsgPrintMessage(MSG_HANDLE_T hMsgHandle, msg_message_t pMsg, MSG_SENDINGOPT_S* pSendOpt)
{
	MSG_FOLDER_ID_T folderId = msg_get_folder_id(pMsg);

	system ("clear" );

	printf("======================================\n");
	printf("============== Message ==============\n");
	printf("======================================\n");

	if (msg_is_mms(pMsg) && msg_get_message_type(pMsg) == MSG_TYPE_MMS_NOTI)
	{
		MMS_MESSAGE_DATA_S *msgBody = msg_mms_create_message();

		msg_mms_get_message_body(pMsg, msgBody);

		//Multipart Related
		if (msgBody->pageCnt)
		{
			printf("multipart type: Multipart Related\n");

			//Print root-layout info
			printf("\n** ROOT LAYOUT INFO **\n");
			printf("width: %d %s\nheight: %d %s\nbgColor:%x\n", msgBody->rootlayout.width.value, msgBody->rootlayout.width.bUnitPercent ? "%" : "",
				 msgBody->rootlayout.height.value,  msgBody->rootlayout.height.bUnitPercent ? "%" : "", msgBody->rootlayout.bgColor);

			// Print Region Info
			printf("\n** REGION INFO **\n");
			printf("Region Count: %d\n", msgBody->regionCnt);

			for(int i = 0; i < msgBody->regionCnt; ++i)
			{
				MMS_SMIL_REGION* pRegion = msg_mms_get_smil_region(msgBody, i);

				printf("%d region id: %s\n%d region left : %d %s\n%d region top : %d %s\n%d region width : %d %s\n%d region height : %d %s\n%d region bgColor : %x\n %d region fit : %s\n",
					i, pRegion->szID, i, pRegion->nLeft.value, pRegion->nLeft.bUnitPercent ? "%": "", i, pRegion->nTop.value, pRegion->nTop.bUnitPercent ? "%": "",
					i, pRegion->width.value, pRegion->width.bUnitPercent ? "%": "", i, pRegion->height.value, pRegion->height.bUnitPercent ? "%": "",
					i, pRegion->bgColor, i, (pRegion->fit == MMSUI_IMAGE_REGION_FIT_MEET) ? "MEET" : "HIDDEN");
			}

			printf("\n** PAGES & CONTENTS INFO **\n");
			printf("PAGE Count: %d\n", msgBody->pageCnt);

			// Print Page info
			for(int i = 0; i< msgBody->pageCnt; ++i)
			{
				MMS_PAGE_S* pPage = msg_mms_get_page(msgBody, i);

				printf("%d page's duration: %d msec \n%d page's media count: %d\n", i, pPage->nDur, i, pPage->mediaCnt);

				// Print Contents Info
				for(int j = 0; j < pPage->mediaCnt; ++j)
				{
					MMS_MEDIA_S* pMedia = msg_mms_get_media(pPage, j);
					printf("%d media's filename: %s\n%d media's filepath: %s\n%d media's regionId: %s\n Bold: %d\n Italic: %d\n", j, pMedia->szFileName, j, pMedia->szFilePath, j, pMedia->regionId, pMedia->sMedia.sText.bBold, pMedia->sMedia.sText.bItalic);

					if(pMedia->drmType != MSG_DRM_TYPE_NONE)
					{
						printf("%d media's drmtype: %d (1: Fowward Lock, 2:Combined Delivery, 3: Separated Delivery\n%d media's drmpath: %s\n", j, pMedia->drmType, j, pMedia->szDrm2FullPath);
					}

				}
				printf("\n");
			}
		}

		else
		{
			printf("multipart type: Multipart Mixed\n");
		}

		for(int i = 0; i < msgBody->attachCnt; ++i)
		{
			MMS_ATTACH_S*	pAttach = msg_mms_get_attachment(msgBody, i);
			printf("Attachment file Name: %s\n", pAttach->szFileName);
			printf("Attachment file Path: %s\n", pAttach->szFilePath);
			printf("Attached file size: %d\n", pAttach->fileSize);

			if(pAttach->drmType != MSG_DRM_TYPE_NONE)
				printf("%d media's drmtype: %d (1: Fowward Lock, 2:Combined Delivery, 3: Separated Delivery\n%d media's drmpath: %s\n", i, pAttach->drmType, i, pAttach->szDrm2FullPath);
		}

		char displayTime[32];
		memset(displayTime, 0x00, sizeof(displayTime));

		MsgConvertTime(msg_get_time(pMsg), displayTime);

		printf("date & time: %s\n", displayTime);
		printf("from address: %s\n", msg_get_ith_address(pMsg, 0));

		if(msg_is_sms(pMsg))
		{
			printf("*** SMS Setting Value ***\n");
			printf("Delivery Request : %d\n", pSendOpt->bDeliverReq);
			printf("Reply Path : %d\n", pSendOpt->option.smsSendOpt.bReplyPath);
		}
		else if(msg_is_mms(pMsg))
		{
			printf("Subject: %s\n", msg_get_subject(pMsg));

			char expiryTime[32];
			memset(expiryTime, 0x00, sizeof(expiryTime));

			MsgConvertTime(&(pSendOpt->option.mmsSendOpt.expiryTime), expiryTime);

			printf("*** MMS Setting Value ***\n ");
			printf("Delivery Requeset : %d\n Read Report Request : %d\n Priority : %d\n ExpiryTime : %s\n\n",
				pSendOpt->bDeliverReq, pSendOpt->option.mmsSendOpt.bReadReq, pSendOpt->option.mmsSendOpt.priority, expiryTime);

			MSG_REPORT_STATUS_INFO_S reportStatus = {};

			msg_get_report_status(hMsgHandle, msg_get_message_id(pMsg), &reportStatus);

			memset(expiryTime, 0x00, sizeof(expiryTime));
			MsgConvertTime(&(reportStatus.deliveryStatusTime), expiryTime);

			printf("Delivery Report Status : %d\n Delivery Report Status Time: %s\n ",
				reportStatus.deliveryStatus, expiryTime);

			memset(expiryTime, 0x00, sizeof(expiryTime));
			MsgConvertTime(&(reportStatus.readStatusTime), expiryTime);

			printf("Read Report Status : %d\n Read Report Status Time: %s\n ",
				reportStatus.readStatus, expiryTime);
		}

		printf("======================================\n");
		printf("[R] Reply\n[T] rTreive MMS\n[F] Forward\n[D] Delete\n[C] Change Folder\n[M] Move to Sim\n[P] Protect\n[B] Back");

		/*destroy and free message data*/
		msg_mms_destroy_message(msgBody);
	}
	else if (folderId == MSG_INBOX_ID)
	{
		char displayTime[32];
		memset(displayTime, 0x00, sizeof(displayTime));

		MsgConvertTime(msg_get_time(pMsg), displayTime);

		const char* msgText = NULL;

		int msgType = msg_get_message_type(pMsg);

		if (msgType==MSG_TYPE_MMS_NOTI || msgType==MSG_TYPE_MMS_JAVA || msgType==MSG_TYPE_MMS) {
			msgText = msg_mms_get_text_contents(pMsg);
		} else {
			msgText = msg_sms_get_message_body(pMsg);
		}

		printf("message text\n[%s]\n\ndate & time : [%s]\n\naddress : [%s]\nname : [%s]\ncontact ID : [%d]\n",
					   msgText, displayTime, msg_get_ith_address(pMsg, 0), msg_get_ith_name(pMsg, 0), msg_get_ith_contact_id(pMsg, 0));

		if (msg_is_mms(pMsg)) {
			printf("Subject: %s\n", msg_get_subject(pMsg));
			MsgPrintMMSBody(pMsg);
		}

		if(msg_is_in_sim(pMsg))
		{
			printf("\nstorage : [Sim Card]\n");
			printf("======================================\n");
//			if(msg_get_message_type(pMsg) == MSG_TYPE_SMS_SYNCML)
				printf("[R] Reply\n[F] Forward\n[D] Delete\n[C] Change Folder\n[M] Move to Phone\n[P] Protect\n[B] Back\n[I] Install CP Message");
//			else
//				printf("[R] Reply\n[F] Forward\n[D] Delete\n[C] Change Folder\n[M] Move to Phone\n[P] Protect\n[B] Back");
		}
		else
		{
			printf("======================================\n");
//			if(msg_get_message_type(pMsg) == MSG_TYPE_SMS_SYNCML)
				printf("[R] Reply\n[F] Forward\n[D] Delete\n[C] Change Folder\n[M] Move to Sim\n[P] Protect\n[B] Back\n[I] Install CP Message");
//			else
//				printf("[R] Reply\n[T] rTreive MMS\n[F] Forward\n[D] Delete\n[C] Change Folder\n[M] Move to Sim\n[P] Protect\n[B] Back");
		}
	}
	else if (folderId == MSG_OUTBOX_ID)
	{
		char displayTime[32];
		memset(displayTime, 0x00, sizeof(displayTime));

		MsgConvertTime(msg_get_time(pMsg), displayTime);

		const char* msgText = NULL;

		int msgType = msg_get_message_type(pMsg);

		if(msgType==MSG_TYPE_MMS_NOTI || msgType==MSG_TYPE_MMS_JAVA || msgType==MSG_TYPE_MMS)
			msgText = msg_mms_get_text_contents(pMsg);
		else
			msgText = msg_sms_get_message_body(pMsg);

		printf("message text\n[%s]\n\ndate & time : [%s]\n\n", msgText, displayTime);

		int addr_count = msg_get_address_count(pMsg);
		for (int i = 0; i < addr_count; i++)
			printf("address : [%s]\nname : [%s]\ncontact ID : [%d]\n\n", msg_get_ith_address(pMsg, i), msg_get_ith_name (pMsg, i), msg_get_ith_contact_id(pMsg, i) );


		if(msg_is_in_sim(pMsg))
		{
			printf("\nstorage : [Sim Card]\n");
			printf("======================================\n");
			printf("[S] Send\n[F] Forward\n[D] Delete\n[C] Change Folder\n[M] Move to Phone\n[P] Protect\n[B] Back");
		}
		else
		{
			printf("======================================\n");
			printf("[S] Send\n[F] Forward\n[D] Delete\n[C] Change Folder\n[M] Move to Sim\n[P] Protect\n[B] Back");
		}
	}
	else if (folderId == MSG_DRAFT_ID)
	{
		char displayTime[32];
		memset(displayTime, 0x00, sizeof(displayTime));

		MsgConvertTime(msg_get_time(pMsg), displayTime);

		const char* msgText = NULL;

		int msgType = msg_get_message_type(pMsg);

		if(msgType==MSG_TYPE_MMS_NOTI || msgType==MSG_TYPE_MMS_JAVA || msgType==MSG_TYPE_MMS)
			msgText = msg_mms_get_text_contents(pMsg);
		else
			msgText = msg_sms_get_message_body(pMsg);

		printf("message text\n[%s]\n\ndate & time : [%s]\n\n", msgText, displayTime);

		int addr_count = msg_get_address_count(pMsg);
		for (int i = 0; i < addr_count; i++)
			printf("address : [%s]\nname : [%s]\ncontact ID : [%d]\n\n", msg_get_ith_address(pMsg, i), msg_get_ith_name (pMsg, i), msg_get_ith_contact_id(pMsg, i));

		printf("======================================\n");
		printf("[S] Send\n[E] Edit\n[D] Delete\n[C] Change Folder\n[P] Protect\n[B] Back");
	}
	else
	{
		char displayTime[32];
		memset(displayTime, 0x00, sizeof(displayTime));

		MsgConvertTime(msg_get_time(pMsg), displayTime);

		const char* msgText = NULL;

		int msgType = msg_get_message_type(pMsg);

		if(msgType==MSG_TYPE_MMS_NOTI || msgType==MSG_TYPE_MMS_JAVA || msgType==MSG_TYPE_MMS)
			msgText = msg_mms_get_text_contents(pMsg);
		else
			msgText = msg_sms_get_message_body(pMsg);

		printf("message text\n[%s]\n\ndate & time : [%s]\n\n", msgText, displayTime);

		int addr_count = msg_get_address_count(pMsg);
		for (int i = 0; i < addr_count; i++)
			printf("address : [%s]\nname : [%s]\ncontact ID : [%d]\n\n", msg_get_ith_address(pMsg, i), msg_get_ith_name(pMsg, i), msg_get_ith_contact_id(pMsg, i));

		if(msg_is_sms(pMsg))
		{
			printf("*** SMS Setting Value ***\n");
			printf("Delivery Requeset : %d\nReply Path : %d\n", pSendOpt->bDeliverReq, pSendOpt->option.smsSendOpt.bReplyPath);
		}
		else if(msg_is_mms(pMsg))
		{
			printf("Subject: %s\n", msg_get_subject(pMsg));

			char expiryTime[32];
			memset(expiryTime, 0x00, sizeof(expiryTime));

			MsgConvertTime(&(pSendOpt->option.mmsSendOpt.expiryTime), expiryTime);

			printf("*** MMS Setting Value ***\n ");
			printf("Delivery Requeset : %d\n Read Report Request : %d\n Priority : %d\n ExpiryTime : %s\n\n",
				pSendOpt->bDeliverReq, pSendOpt->option.mmsSendOpt.bReadReq, pSendOpt->option.mmsSendOpt.priority, expiryTime);

			MSG_REPORT_STATUS_INFO_S reportStatus = {};

			msg_get_report_status(hMsgHandle, msg_get_message_id(pMsg), &reportStatus);

			memset(expiryTime, 0x00, sizeof(expiryTime));
			MsgConvertTime(&(reportStatus.deliveryStatusTime), expiryTime);

			printf("Delivery Report Status : %d\n Delivery Report Status Time: %s\n ",
				reportStatus.deliveryStatus, expiryTime);

			memset(expiryTime, 0x00, sizeof(expiryTime));
			MsgConvertTime(&(reportStatus.readStatusTime), expiryTime);

			printf("Read Report Status : %d\n Read Report Status Time: %s\n ",
				reportStatus.readStatus, expiryTime);

			MsgPrintMMSBody(pMsg);
		}


		printf("======================================\n");
		printf("[D] Delete\n[P] Protect\n[B] Back");
	}

	printf("\n======================================\n");
	print("Input : ");

	char select = 0, ch = 0;

	cin >> select;
	ch = cin.get();

	MsgRunMsgMenu(hMsgHandle, select, pMsg, pSendOpt);
}


void MsgRunMsgMenu(MSG_HANDLE_T hMsgHandle, char Menu, msg_message_t pMsg, MSG_SENDINGOPT_S *pSendOpt)
{
	switch (Menu)
	{
		case 'R' :
		{


		}
		break;

		case 'T' :
		{

		}

		case 'F' :
		{


		}
		break;

		case 'D' : // Delete Message
		{
			if ( msg_is_protected(pMsg))
				print("The message is protected. You cannot delete.");
			else
				msg_delete_message(hMsgHandle, msg_get_message_id(pMsg));
		}
		break;

		case 'C' : // Move Message to other folder
		{
			MsgTestMoveMessage(hMsgHandle, msg_get_message_id(pMsg));
		}
		break;

		case 'M' : // Move to Sim
		{
			MsgTestMoveStorageMessage(hMsgHandle, pMsg);
		}
		break;

		case 'P' :  // Set Protect
		{
			if (msg_is_protected(pMsg))
				msg_update_protected_status(hMsgHandle, msg_get_message_id(pMsg), false);
			else
				msg_update_protected_status(hMsgHandle, msg_get_message_id(pMsg), true);
		}
		break;

		case 'S' :
		{
			MsgTestSubmitReq(hMsgHandle, pMsg, NULL); // fuction call to "MsgTestTransport.cpp"
		}
		break;

		case 'E' :
		{
			MsgTestUpdateMessage(hMsgHandle, pMsg);
		}
		break;

		case 'B' :
		break;

		case 'I' :
		{
			//if(msg_get_message_type(pMsg) == MSG_TYPE_SMS_SYNCML)
				msg_syncml_message_operation(hMsgHandle, (MSG_MESSAGE_ID_T)msg_get_message_id(pMsg));
		}
		break;

		default :
		{
			print("Not Supported Menu");
		}
		break;
	}
}


void MsgRunMsgListMenu(MSG_HANDLE_T hMsgHandle, int MsgId, int FolderId, MSG_MESSAGE_TYPE_T MsgType, MSG_NETWORK_STATUS_T NetworkStatus)
{
	if (MsgType == MSG_TYPE_SMS || MsgType == MSG_TYPE_SMS_CB || MsgType == MSG_TYPE_SMS_JAVACB ||
		MsgType == MSG_TYPE_SMS_MWI || MsgType == MSG_TYPE_SMS_SYNCML || MsgType == MSG_TYPE_SMS_WAPPUSH)
	{
		if (MsgId > 0 && MsgId <= 9999)
		{
			MsgTestGetMessage(hMsgHandle, MsgId);
		}
		else
		{
			print("Not Supported Msg Id");
			return;
		}
	}
	else if (MsgType == MSG_TYPE_MMS || MsgType == MSG_TYPE_MMS_JAVA || MsgType == MSG_TYPE_MMS_NOTI)
	{
		char menu[2];

		if(NetworkStatus == MSG_NETWORK_RETRIEVE_SUCCESS  ||NetworkStatus == MSG_NETWORK_SEND_SUCCESS ||
			NetworkStatus == MSG_NETWORK_SEND_FAIL || NetworkStatus == MSG_NETWORK_SENDING)
		{
			print("======== Select ========");
			print("[R] Read MMS Message");
			print("[F] Forward MMS Message");
			print("[D] Delete MMS Message");
			print("[B] BACK");
			print("====================");
			print("Input : ");
		}
		else if(NetworkStatus == MSG_NETWORK_RETRIEVE_FAIL || NetworkStatus == MSG_NETWORK_RECEIVED)
		{
			print("======== Select ========");
			print("[T] Retrieve MMS Message");
			print("[J] Reject MMS Message");
			print("[D] Delete MMS Message");
			print("[B] BACK");
			print("====================");
			print("Input : ");
		}
		else if(NetworkStatus == MSG_NETWORK_NOT_SEND)
		{
			print("======== Select ========");
			print("[R] Read MMS Message");
			print("[U] Update MMS Message");
			print("[B] BACK");
			print("====================");
			print("Input : ");
		}

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 2);

		if (MsgId <= 0 || MsgId > 9999)
		{
			print("Not Supported Msg Id");
			return;
		}

		if (!strcmp(menu, "R"))
		{
			MsgTestGetMessage(hMsgHandle, MsgId);
		}
		else if (!strcmp(menu, "D"))
		{
			MsgTestDeleteMessage(hMsgHandle, MsgId);
		}
		else if(!strcmp(menu, "F"))
		{
			MsgTestForwardMMSMessage(hMsgHandle, MsgId);
		}
		else if(!strcmp(menu, "T"))
		{
			MsgTestRetrieveMessage(hMsgHandle, MsgId);
		}
		/* reject_msg_support */
		else if(!strcmp(menu, "J"))
		{
			MsgTestRejectMessage(hMsgHandle, MsgId);
		}
		/* reject_msg_support */
		else if(!strcmp(menu, "U"))
		{
			MsgTestUpdateMMSMessage(hMsgHandle, MsgId);
		}
		else if (!strcmp(menu, "B"))
		{
			return;
		}
	}
}


void MsgGetCurrentTime(time_t *pTime)
{

	time_t curTime;
	time(&curTime);
	//struct tm timeStruct = {0};
	//localtime_r(&curTime, &timeStruct);

	//memset(pTime, 0x00, sizeof(pTime));
	//sprintf(pTime, "%04d-%02d-%02d %02d:%02d:%02d",
	//	timeStruct.tm_year+1900, timeStruct.tm_mon+1, timeStruct.tm_mday,
	//	timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec);

	*pTime = curTime;
	MSG_DEBUG("Current time is %s", ctime(&curTime));
}


const char* MsgConvertMsgType(MSG_MESSAGE_TYPE_T MsgType)
{
	if (MsgType == MSG_TYPE_SMS)
			return "SMS";
	else if (MsgType == MSG_TYPE_SMS_CB || MsgType == MSG_TYPE_SMS_JAVACB)
			return "CB";
	else if (MsgType == MSG_TYPE_SMS_WAPPUSH)
			return "WAP";
	else if (MsgType == MSG_TYPE_SMS_MWI)
			return "MWI";
	else if (MsgType == MSG_TYPE_MMS || MsgType == MSG_TYPE_MMS_NOTI)
		return "MMS";

	return "UNKNOWN";
}


const char* MsgConvertStorageId(MSG_STORAGE_ID_T StorageId)
{
	if (StorageId == MSG_STORAGE_PHONE)
		return "PHONE";
	else if (StorageId == MSG_STORAGE_SIM)
		return "SIM";

	return "PHONE";
}


const char* MsgConvertNetworkStatus(MSG_NETWORK_STATUS_T status)
{
	switch (status)
	{
		case MSG_NETWORK_NOT_SEND :
			return "NOT SEND";
		case MSG_NETWORK_SENDING :
			return "SENDING";
		case MSG_NETWORK_SEND_SUCCESS :
			return "SENT";
		case MSG_NETWORK_SEND_FAIL :
			return "FAIL TO SEND";
		case MSG_NETWORK_DELIVER_SUCCESS :
			return "DELIVERED";
		case MSG_NETWORK_DELIVER_FAIL :
			return "FAIL TO DELIVER";
		case MSG_NETWORK_RECEIVED :
			return "RECEIVED";
		case MSG_NETWORK_RETRIEVE_FAIL:
			return "NOT YET RETRIEVED";
		case MSG_NETWORK_RETRIEVE_SUCCESS :
			return "RETRIEVED";
		case MSG_NETWORK_RETRIEVING:
			return "RETRIEVING";
		default :
			return "NOT SEND";
	}

	return "NOT SEND";
}


const char* MsgConvertReadStatus(bool ReadStatus)
{
	if (ReadStatus == true)
		return "READ";
	else if (ReadStatus == false)
		return "UNREAD";

	return "UNREAD";
}


const char* MsgConvertProtectedStatus(bool ProtectedStatus)
{
	if (ProtectedStatus == true)
		return "PROTECTED";
	else if (ProtectedStatus == false)
		return "UNPROTECTED";

	return "PROTECTED";
}


void MsgConvertTime(time_t *pTime, char *pDisplayTme)
{
	struct tm * timeinfo;

	tzset();

	timeinfo = localtime(pTime);

//	MSG_DEBUG("time  zone is %s", timeinfo->tm_zone);

	memset(pDisplayTme, 0x00, sizeof(pDisplayTme));

//	strftime(timeBuff, 32, "%c", timeinfo);

	strftime(pDisplayTme, 32, "%Y-%02m-%02d %T %z", timeinfo);
}

