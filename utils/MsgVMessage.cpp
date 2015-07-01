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

#include <VMessage.h>
#include <VCard.h>
#include <time.h>

#include "MsgMmsTypes.h"

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgCppTypes.h"
#include "MsgVMessage.h"
#include "MsgMmsMessage.h"
#include "MsgSerialize.h"

/*==================================================================================================
                                     DEFINES
==================================================================================================*/
#define INSERT_VMSG_OBJ pObject = (VObject*)calloc(1, sizeof(VObject));	\
                                      if ( !pObject )\
{\
   vmsg_free_vtree_memory( pMessage );\
   return false;\
}\
if (pMessage->pTop == NULL)\
{\
   pMessage->pTop = pObject;\
}\
else\
{\
   pMessage->pCur->pSibling = pObject;\
}\
pMessage->pCur = pObject;


#define INSERT_VBODY_OBJ pObject = (VObject*)calloc(1, sizeof(VObject));	\
                                      if ( !pObject )\
{\
   vmsg_free_vtree_memory( pMessage );\
   return false;\
}\
if (pBody->pTop == NULL)\
{\
   pBody->pTop = pObject;\
}\
else\
{\
   pBody->pCur->pSibling = pObject;\
}\
pBody->pCur = pObject;


#define INSERT_VCARD_OBJ pObject = (VObject*)calloc(1, sizeof(VObject));	\
                                      if ( !pObject )\
{\
   vmsg_free_vtree_memory( pMessage );\
   return false;\
}\
if (pCard->pTop == NULL)\
{\
   pCard->pTop = pObject;\
}\
else\
{\
   pCard->pCur->pSibling = pObject;\
}\
pCard->pCur = pObject;


#define INSERT_PARAM  param = (VParam*)calloc(1, sizeof(VParam));\
                             if (!param)\
{\
   vmsg_free_vtree_memory( pMessage );\
   if (pObject != NULL)\
   {\
      free(pObject);\
      pObject = NULL;\
   }\
   return false;\
}


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
char* MsgVMessageAddRecord(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S* pMsg)
{
	MSG_BEGIN();

	VObject*		pObject = NULL;
	VParam*		param = NULL;
	VTree *		pBody = NULL;
	VTree *		pCard = NULL;
	VTree *		pCurrent= NULL;
	struct tm	display_time;
	char*		encoded_data = NULL;

	VTree* pMessage = NULL;
	if (pMessage == NULL) {
		pMessage = (VTree *)malloc(sizeof(VTree));
		if (!pMessage) {
			return NULL;
		}
		pMessage->treeType = VMESSAGE;
		pMessage->pTop = NULL;
		pMessage->pCur = NULL;
		pMessage->pNext = NULL;

	}
		pCurrent = pMessage;

	//Insert VObject (X-MESSAGE-TYPE) to VMessage tree
	INSERT_VMSG_OBJ;

	pObject->property = VMSG_TYPE_MSGTYPE;

	if(pMsg->msgType.subType == MSG_NORMAL_SMS)
		pObject->pszValue[0] = strdup("SMS");
#if 0
	else if(pMsg->msgType.mainType == MSG_MMS_TYPE && pMsg->msgType.subType == MSG_NOTIFICATIONIND_MMS)
		pObject->pszValue[0] = strdup("MMS NOTIFICATION");
#endif

	else if(pMsg->msgType.mainType == MSG_MMS_TYPE && (pMsg->msgType.subType == MSG_SENDREQ_MMS || pMsg->msgType.subType == MSG_SENDCONF_MMS))
		pObject->pszValue[0] = strdup("MMS SEND");

	else if(pMsg->msgType.mainType == MSG_MMS_TYPE && (pMsg->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsg->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS))
		pObject->pszValue[0] = strdup("MMS RETRIEVED");

	else
		goto __CATCH_FAIL__;

	pObject->valueCount = 1;


	//Insert VObject (X-IRMC-BOX) to VMessate tree
	INSERT_VMSG_OBJ;

	pObject->property = VMSG_TYPE_MSGBOX;

	switch(pMsg->folderId)
	{
		case MSG_INBOX_ID:
			pObject->pszValue[0] = strdup("INBOX");
			break;
		case MSG_OUTBOX_ID:
			pObject->pszValue[0] = strdup("OUTBOX");
			break;
		case MSG_SENTBOX_ID:
			pObject->pszValue[0] = strdup("SENTBOX");
			break;
		case MSG_DRAFT_ID:
			pObject->pszValue[0] = strdup("DRAFTBOX");
			break;
		default:
			// Discard User Defined or Spam folder's messages.
			goto __CATCH_FAIL__;
	}
	pObject->valueCount = 1;


	//Insert VObject (X-SS-DT) to VMessage tree
	INSERT_VMSG_OBJ;

	pObject->property = VMSG_TYPE_DATE;
	tzset();
	localtime_r(&(pMsg->displayTime), &display_time);
	pObject->pszValue[0] = _convert_tm_to_vdata_str(&display_time);
	pObject->valueCount = 1;


	//Insert Vobject read status to VMessage tree
	INSERT_VMSG_OBJ;

	pObject->property = VMSG_TYPE_STATUS;

	if(pMsg->bRead)
		pObject->pszValue[0] = strdup("READ");
	else
		pObject->pszValue[0] = strdup("UNREAD");

	pObject->valueCount = 1;


	//Insert VBody  tree for message body;
	pBody = (VTree*)calloc(1, sizeof(VTree));
	if ( !pBody )
		goto __CATCH_FAIL__;
	pBody->treeType = VBODY;
	pBody->pTop = NULL;
	pBody->pCur = NULL;
	pBody->pNext = NULL;
	pCurrent->pNext = pBody;
	pCurrent = pBody;

if(strlen(pMsg->subject) > 0)
{
	//Insert Subject object
	INSERT_VBODY_OBJ;
	pObject->property = VMSG_TYPE_SUBJECT;
	pObject->pszValue[0] = strdup(pMsg->subject);
	pObject->valueCount = 1;
}
	//Insert VBody object
	INSERT_VBODY_OBJ;
	pObject->property = VMSG_TYPE_BODY;

	if(pMsg->msgType.mainType == MSG_SMS_TYPE)
	{
		if(pMsg->msgType.subType == MSG_NORMAL_SMS)
		{
			if (pMsg->bTextSms == false)
			{
				char* pFileData = NULL;
				AutoPtr<char> buf(&pFileData);

				int		fileSize = 0;
				char*	msgText = NULL;

				if (MsgOpenAndReadFile(pMsg->msgData, &pFileData, &fileSize) == false)
					goto __CATCH_FAIL__;

				msgText = (char *)calloc(1, fileSize);
				if(pFileData && msgText)
					memcpy(msgText, pFileData, fileSize);

				pObject->numOfBiData = fileSize;
				pObject->pszValue[0] = msgText;
			}
			else
			{
				pObject->numOfBiData = pMsg->dataSize;
				pObject->pszValue[0] = strdup(pMsg->msgText);
			}
			pObject->valueCount = 1;
		}
		else
			goto __CATCH_FAIL__;
	}

	else if(pMsg->msgType.mainType == MSG_MMS_TYPE)
	{
		//Insert VBody for mms raw data;
		char* pFileData = NULL;
		MMS_DATA_S *pMmsData = NULL;
		int fileSize = 0;
		char* msgText = NULL;
#if 0
		char		filePath[MSG_FILEPATH_LEN_MAX] = {0, };
		if(pMsg->msgType.subType == MSG_NOTIFICATIONIND_MMS)
			pFileData = MsgOpenAndReadMmsFile(pMsg->msgData, 0, -1, &fileSize);

		else
		{
			err = MsgStoGetMmsRawFilePath(pDbHandle, pMsg->msgId, filePath);

			if (err != MSG_SUCCESS)
				goto __CATCH_FAIL__;

			pFileData = MsgOpenAndReadMmsFile(filePath, 0, -1, &fileSize);
		}
#else

		if (pMsg->bTextSms == false) {
			if (MsgOpenAndReadFile(pMsg->msgData, &pFileData, &fileSize) == false) {
				//CID 16205: Memory leak in case of failure
				if (pFileData) {
					free(pFileData);
					pFileData = NULL;
				}
				goto __CATCH_FAIL__;
			}
		} else {
			fileSize = strlen(pMsg->msgData);
			pFileData = (char *)calloc(1, fileSize+1);
			if (!pFileData)
				goto __CATCH_FAIL__;
			snprintf(pFileData, fileSize, "%s", pMsg->msgData);
		}

		//CID 46178: Pulling up the null check
		if (pFileData) {
			if (MsgDeserializeMmsData(pFileData, fileSize, &pMmsData) != 0) {
				MSG_DEBUG("Fail to Deserialize Message Data");
				MsgMmsRelease(&pMmsData);
				goto __CATCH_FAIL__;
			}

			free(pFileData);
			pFileData = NULL;
		}

		MsgMmsSetMultipartListData(pMmsData);//app file -> data

		int serializedDataSize = 0;

		if (pMmsData) {
			MsgMmsSetMultipartListData(pMmsData);//app file -> data
			serializedDataSize = MsgSerializeMms(pMmsData, &pFileData);
		}

		if (pFileData) {
			fileSize = serializedDataSize;
		}

		MsgMmsRelease(&pMmsData);

#endif
		MSG_DEBUG("FILE SIZE IS %d, %s", fileSize, pFileData);
		msgText = (char *)calloc(1, fileSize);
		if(pFileData && msgText)
			memcpy(msgText, pFileData, fileSize);

		pObject->numOfBiData = fileSize;
		pObject->pszValue[0] = msgText;
		pObject->valueCount = 1;

		if (pFileData) {
			free(pFileData);
			pFileData = NULL;
		}
	}

	// Insert parameter for base64 encoding
	MSG_DEBUG("before to start INSERT_PARAM");
	INSERT_PARAM;
	pObject->pParam = param;
	param->parameter = VMSG_PARAM_ENCODING;
	param->paramValue = VMSG_ENC_PARAM_BASE64;

	// Add VCard tree for recipient address information.
	for(int i = 0; i < pMsg->nAddressCnt; ++i)
	{
		pCard = (VTree*)calloc(1, sizeof(VTree));
		if ( !pCard )
			goto __CATCH_FAIL__;

		pCard->treeType = VCARD;
		pCard->pTop = NULL;
		pCard->pCur = NULL;
		pCard->pNext = NULL;
		pCurrent->pNext = pCard;
		pCurrent = pCard;

		INSERT_VCARD_OBJ;
		pObject->property = VCARD_TYPE_TEL;
		pObject->pszValue	[0] = strdup(pMsg->addressList[i].addressVal);
		pObject->valueCount = 1;
	}
	MSG_DEBUG("before to start vmsg_encode");
	encoded_data = vmsg_encode(pMessage);

	vmsg_free_vtree_memory(pMessage);
	MSG_END();
	return encoded_data;

__CATCH_FAIL__ :
	vmsg_free_vtree_memory( pMessage );

	return NULL;
}


char* _convert_tm_to_vdata_str(const struct tm * tm)
{
	char str[17] = {0, };

	snprintf(str, 17, "%04d%02d%02dT%02d%02d%02dZ",
		tm->tm_year + 1900,
		tm->tm_mon +1,
		tm->tm_mday,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec);

	return strdup(str);
}


bool _convert_vdata_str_to_tm(const char* szText, struct tm * tm)
{

   if (szText == NULL) return false;
   if (strlen(szText) < 15) return false;
   if (szText[8] != 'T') return false;

   char szBuff[8]={0};
   memset(tm, 0, sizeof(struct tm));

   // year, month, day
   memcpy(szBuff, &(szText[0]), 4);
   szBuff[4] = '\0';
   tm->tm_year = atol(szBuff) - 1900;
   if ((tm->tm_year > 137) || (tm->tm_year < 0))
      tm->tm_year = 0;

   memcpy(szBuff, &(szText[4]), 2);
   szBuff[2] = '\0';
   tm->tm_mon = atol(szBuff)-1;
   if ((tm->tm_mon > 11) || (tm->tm_mon < 0))
      tm->tm_mon = 11;

   memcpy(szBuff, &(szText[6]), 2);
   szBuff[2] = '\0';
   tm->tm_mday = atol(szBuff);
   if ((tm->tm_mday > 31) || (tm->tm_mday < 1))
      tm->tm_mday = 31;

   // hour, minute, second
   memcpy(szBuff, &(szText[9]), 2);
   szBuff[2] = '\0';
   tm->tm_hour = atol(szBuff);
   if ((tm->tm_hour > 23) || (tm->tm_hour < 0))
      tm->tm_hour = 23;

   memcpy(szBuff, &(szText[11]), 2);
   szBuff[2] = '\0';
   tm->tm_min = atol(szBuff);
   if ((tm->tm_min > 59) || (tm->tm_min < 0))
      tm->tm_min = 59;

   memcpy(szBuff, &(szText[13]), 2);
   szBuff[2] = '\0';
   tm->tm_sec = atol(szBuff);
   if ((tm->tm_sec > 59) || (tm->tm_sec < 0))
      tm->tm_sec = 59;

   return true;
}

char *MsgVMessageEncode(MSG_MESSAGE_INFO_S *pMsg)
{
	MSG_BEGIN();

	MsgDbHandler dbHandle;
	VObject *pObject = NULL;
	VParam *param = NULL;
	VTree *pBody = NULL;
	VTree *pCard = NULL;
	VTree *pCurrent= NULL;
	VTree *pMessage = NULL;

	struct tm	display_time;
	char *encoded_data = NULL;
	int err = MSG_SUCCESS;

	if (pMessage == NULL) {
		pMessage = (VTree *)malloc(sizeof(VTree));
		if (!pMessage) {
			return NULL;
		}
		pMessage->treeType = VMESSAGE;
		pMessage->pTop = NULL;
		pMessage->pCur = NULL;
		pMessage->pNext = NULL;

	}
		pCurrent = pMessage;

	//Insert VObject (X-MESSAGE-TYPE) to VMessage tree
	INSERT_VMSG_OBJ;

	pObject->property = VMSG_TYPE_MSGTYPE;

	if(pMsg->msgType.subType == MSG_NORMAL_SMS)
		pObject->pszValue[0] = strdup("SMS");
#if 0
	else if(pMsg->msgType.mainType == MSG_MMS_TYPE && pMsg->msgType.subType == MSG_NOTIFICATIONIND_MMS)
		pObject->pszValue[0] = strdup("MMS NOTIFICATION");
#endif

	else if(pMsg->msgType.mainType == MSG_MMS_TYPE && (pMsg->msgType.subType == MSG_SENDREQ_MMS || pMsg->msgType.subType == MSG_SENDCONF_MMS))
		pObject->pszValue[0] = strdup("MMS SEND");

	else if(pMsg->msgType.mainType == MSG_MMS_TYPE && (pMsg->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsg->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS))
		pObject->pszValue[0] = strdup("MMS RETRIEVED");

	else
		goto __CATCH_FAIL__;

	pObject->valueCount = 1;


	//Insert VObject (X-IRMC-BOX) to VMessate tree
	INSERT_VMSG_OBJ;

	pObject->property = VMSG_TYPE_MSGBOX;

	switch(pMsg->folderId)
	{
		case MSG_INBOX_ID:
			pObject->pszValue[0] = strdup("INBOX");
			break;
		case MSG_OUTBOX_ID:
			pObject->pszValue[0] = strdup("OUTBOX");
			break;
		case MSG_SENTBOX_ID:
			pObject->pszValue[0] = strdup("SENTBOX");
			break;
		case MSG_DRAFT_ID:
			pObject->pszValue[0] = strdup("DRAFTBOX");
			break;
		default:
			// Discard User Defined or Spam folder's messages.
			goto __CATCH_FAIL__;
	}
	pObject->valueCount = 1;


	//Insert VObject (X-SS-DT) to VMessage tree
	INSERT_VMSG_OBJ;

	pObject->property = VMSG_TYPE_DATE;
	tzset();
	localtime_r(&(pMsg->displayTime), &display_time);
	pObject->pszValue[0] = _convert_tm_to_vdata_str(&display_time);
	pObject->valueCount = 1;


	//Insert Vobject read status to VMessage tree
	INSERT_VMSG_OBJ;

	pObject->property = VMSG_TYPE_STATUS;

	if(pMsg->bRead)
		pObject->pszValue[0] = strdup("READ");
	else
		pObject->pszValue[0] = strdup("UNREAD");

	pObject->valueCount = 1;


	//Insert VBody  tree for message body;
	pBody = (VTree*)calloc(1, sizeof(VTree));
	if ( !pBody )
		goto __CATCH_FAIL__;
	pBody->treeType = VBODY;
	pBody->pTop = NULL;
	pBody->pCur = NULL;
	pBody->pNext = NULL;
	pCurrent->pNext = pBody;
	pCurrent = pBody;

if(strlen(pMsg->subject) > 0)
{
	//Insert Subject object
	INSERT_VBODY_OBJ;
	pObject->property = VMSG_TYPE_SUBJECT;
	pObject->pszValue[0] = strdup(pMsg->subject);
	pObject->valueCount = 1;
}
	//Insert VBody object
	INSERT_VBODY_OBJ;
	pObject->property = VMSG_TYPE_BODY;

	if(pMsg->msgType.mainType == MSG_SMS_TYPE)
	{
		if(pMsg->msgType.subType == MSG_NORMAL_SMS)
		{
			if (pMsg->bTextSms == false)
			{
				char* pFileData = NULL;
				AutoPtr<char> buf(&pFileData);

				int		fileSize = 0;
				char*	msgText = NULL;

				if (MsgOpenAndReadFile(pMsg->msgData, &pFileData, &fileSize) == false)
					goto __CATCH_FAIL__;

				msgText = (char *)calloc(1, fileSize);
				memcpy(msgText, pFileData, fileSize);
				pObject->numOfBiData = fileSize;
				pObject->pszValue[0] = msgText;
			}
			else
			{
				pObject->numOfBiData = pMsg->dataSize;
				pObject->pszValue[0] = strdup(pMsg->msgText);
			}
			pObject->valueCount = 1;
		}
		else
			goto __CATCH_FAIL__;
	}

	else if(pMsg->msgType.mainType == MSG_MMS_TYPE)
	{
		//Insert VBody for mms raw data;
		char* pFileData = NULL;

		int		fileSize = 0;
		char*	msgText = NULL;
		char		filePath[MSG_FILEPATH_LEN_MAX] = {0, };

		if(pMsg->msgType.subType == MSG_NOTIFICATIONIND_MMS)
			pFileData = MsgOpenAndReadMmsFile(pMsg->msgData, 0, -1, &fileSize);

		else
		{
			err = MsgStoGetMmsRawFilePath(&dbHandle, pMsg->msgId, filePath);

			if (err != MSG_SUCCESS)
				goto __CATCH_FAIL__;

			pFileData = MsgOpenAndReadMmsFile(filePath, 0, -1, &fileSize);
		}
		MSG_DEBUG("FILE SIZE IS %d", fileSize);
		msgText = (char *)calloc(1, fileSize);
		if(pFileData)
			memcpy(msgText, pFileData, fileSize);
		pObject->numOfBiData = fileSize;
		pObject->pszValue[0] = msgText;
		pObject->valueCount = 1;

		if (pFileData) {
			free(pFileData);
			pFileData = NULL;
		}
	}

	// Insert parameter for base64 encoding
	MSG_DEBUG("before to start INSERT_PARAM");
	INSERT_PARAM;
	pObject->pParam = param;
	param->parameter = VMSG_PARAM_ENCODING;
	param->paramValue = VMSG_ENC_PARAM_BASE64;

	// Add VCard tree for recipient address information.
	for(int i = 0; i < pMsg->nAddressCnt; ++i)
	{
		pCard = (VTree*)calloc(1, sizeof(VTree));
		if ( !pCard )
			goto __CATCH_FAIL__;

		pCard->treeType = VCARD;
		pCard->pTop = NULL;
		pCard->pCur = NULL;
		pCard->pNext = NULL;
		pCurrent->pNext = pCard;
		pCurrent = pCard;

		INSERT_VCARD_OBJ;
		pObject->property = VCARD_TYPE_TEL;
		pObject->pszValue	[0] = strdup(pMsg->addressList[i].addressVal);
		pObject->valueCount = 1;
	}
	MSG_DEBUG("before to start vmsg_encode");
	encoded_data = vmsg_encode(pMessage);

	vmsg_free_vtree_memory(pMessage);
	MSG_END();
	return encoded_data;

__CATCH_FAIL__ :
	vmsg_free_vtree_memory( pMessage );

	return NULL;
}
