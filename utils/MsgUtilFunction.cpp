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
#include "MsgUtilFunction.h"

 /*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

// Encoders
int MsgEncodeCountInfo(MSG_COUNT_INFO_S *pCountInfo, char **ppDest)
{
	int dataSize = 0;

	dataSize = sizeof(MSG_COUNT_INFO_S);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, pCountInfo, dataSize);

	return dataSize;
}


int MsgEncodeRecipientList(MSG_RECIPIENTS_LIST_S *pRecipientList, char **ppDest)
{
	int count = 0, dataSize = 0;

	count = pRecipientList->recipientCnt;
	dataSize = sizeof(int) + (sizeof(MSG_ADDRESS_INFO_S)*count);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &count, sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	for (int i = 0; i < count; i++)
	{
		memcpy(p, &(pRecipientList->recipientAddr[i]), sizeof(MSG_ADDRESS_INFO_S));
		p = (void*)((char*)p + sizeof(MSG_ADDRESS_INFO_S));
	}

	return dataSize;
}


int MsgEncodeCountByMsgType(int MsgCount, char **ppDest)
{
	int dataSize = 0;

	dataSize = sizeof(int);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &MsgCount, dataSize);

	return dataSize;
}


int MsgEncodeMsgId(MSG_MESSAGE_ID_T *pMsgId, char **ppDest)
{
	int dataSize = 0;

	dataSize = (sizeof(MSG_MESSAGE_ID_T));

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, pMsgId, dataSize);

	return dataSize;
}


int MsgEncodeMsgInfo(MSG_MESSAGE_INFO_S *pMsgInfo, char **ppDest)
{
	int dataSize = 0;

	dataSize = sizeof(MSG_MESSAGE_INFO_S);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

	p = (void*)((char*)p + sizeof(MSG_MESSAGE_INFO_S));

	return dataSize;
}


int MsgEncodeMsgInfo(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S* pSendOptInfo, char **ppDest)
{
	int dataSize = 0;

	dataSize = (sizeof(MSG_MESSAGE_INFO_S) + sizeof(MSG_SENDINGOPT_INFO_S));

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

	p = (void*)((char*)p + sizeof(MSG_MESSAGE_INFO_S));

	memcpy(p, pSendOptInfo, sizeof(MSG_SENDINGOPT_INFO_S));

	return dataSize;
}


int MsgEncodeFolderViewList(MSG_LIST_S *pFolderViewList, char **ppDest)
{
	int count = 0, dataSize = 0;

	count = pFolderViewList->nCount;

	dataSize = sizeof(int) + (sizeof(MSG_MESSAGE_S)*count);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &count, sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	for (int i = 0; i < count; i++)
	{
		memcpy(p, &(pFolderViewList->msgInfo[i]), sizeof(MSG_MESSAGE_S));
		p = (void*)((char*)p + sizeof(MSG_MESSAGE_S));
	}

	return dataSize;
}


int MsgEncodeFolderList(MSG_FOLDER_LIST_S *pFolderList, char **ppDest)
{
	int count = 0, dataSize = 0;

	count = pFolderList->nCount;
	dataSize = sizeof(int) + (sizeof(MSG_FOLDER_INFO_S)*count);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &count, sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	for (int i = 0; i < count; i++)
	{
		memcpy(p, &(pFolderList->folderInfo[i]), sizeof(MSG_FOLDER_INFO_S));
		p = (void*)((char*)p + sizeof(MSG_FOLDER_INFO_S));
	}

	return dataSize;
}


int MsgEncodeSetting(MSG_SETTING_S *pSetting, char **ppDest)
{
	int dataSize = sizeof(MSG_OPTION_TYPE_T);

	switch (pSetting->type)
	{
		case MSG_GENERAL_OPT :
			dataSize += sizeof(MSG_GENERAL_OPT_S);
		break;
		case MSG_SMS_SENDOPT :
			dataSize += sizeof(MSG_SMS_SENDOPT_S);
		break;
		case MSG_SMSC_LIST :
			dataSize += sizeof(MSG_SMSC_LIST_S);
		break;
		case MSG_MMS_SENDOPT :
			dataSize += sizeof(MSG_MMS_SENDOPT_S);
		break;
		case MSG_MMS_RECVOPT :
			dataSize += sizeof(MSG_MMS_RECVOPT_S);
		break;
		case MSG_MMS_STYLEOPT :
			dataSize += sizeof(MSG_MMS_STYLEOPT_S);
		break;
		case MSG_PUSHMSG_OPT :
			dataSize += sizeof(MSG_PUSHMSG_OPT_S);
		break;
		case MSG_CBMSG_OPT :
			dataSize += sizeof(MSG_CBMSG_OPT_S);
		break;
		case MSG_VOICEMAIL_OPT :
			dataSize += sizeof(MSG_VOICEMAIL_OPT_S);
		break;
		case MSG_MSGSIZE_OPT :
			dataSize += sizeof(MSG_MSGSIZE_OPT_S);
		break;
	}

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, pSetting, dataSize);

	return dataSize;
}


int MsgEncodeMsgType(MSG_MESSAGE_TYPE_S *pMsgType, char **ppDest)
{
	int dataSize = 0;

	dataSize = (sizeof(MSG_MESSAGE_TYPE_S));

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, pMsgType, dataSize);

	return dataSize;
}


int MsgEncodeThreadViewList(MSG_THREAD_VIEW_LIST_S *pThreadViewList, char **ppDest)
{
	int count = 0, dataSize = 0;

	count = pThreadViewList->nCount;

	dataSize = sizeof(int) + (sizeof(MSG_THREAD_VIEW_S)*count);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &count, sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	for (int i = 0; i < count; i++)
	{
		memcpy(p, &(pThreadViewList->msgThreadInfo[i]), sizeof(MSG_THREAD_VIEW_S));
		p = (void*)((char*)p + sizeof(MSG_THREAD_VIEW_S));
	}

	return dataSize;
}


int MsgEncodeConversationViewList(MSG_LIST_S *pConvViewList, char **ppDest)
{
	int count = 0, dataSize = 0;

	count = pConvViewList->nCount;

	dataSize = sizeof(int) + (sizeof(MSG_LIST_S)*count);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &count, sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	for (int i = 0; i < count; i++)
	{
		memcpy(p, &(pConvViewList->msgInfo[i]), sizeof(MSG_LIST_S));
		p = (void*)((char*)p + sizeof(MSG_LIST_S));
	}

	return dataSize;
}


int MsgEncodeMsgGetContactCount(MSG_THREAD_COUNT_INFO_S *threadCountInfo, char **ppDest)
{
	int dataSize = sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int);

	MSG_DEBUG("datasize = [%d] \n", dataSize);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &(threadCountInfo->totalCount), sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	memcpy(p, &(threadCountInfo->unReadCount), sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	memcpy(p, &(threadCountInfo->mmsMsgCount), sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	memcpy(p, &(threadCountInfo->smsMsgCount), sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	return dataSize;
}

int MsgEncodeMemSize(unsigned int *memsize, char **ppDest)
{
	int dataSize = 0;

	dataSize = sizeof(unsigned int);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, memsize, dataSize);

	return dataSize;
}


int MsgEncodeSyncMLOperationData(int msgId, int extId, char **ppDest)
{
	int dataSize = 0;

	dataSize = sizeof(int) + sizeof(int);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &msgId, sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	memcpy(p, &extId, sizeof(int));

	return dataSize;
}


int MsgEncodeStorageChangeData(const MSG_STORAGE_CHANGE_TYPE_T storageChangeType, const MSG_MSGID_LIST_S *pMsgIdList, char **ppDest)
{
	int dataSize = 0;
	int count = 0;

	count = pMsgIdList->nCount;

	dataSize = sizeof(MSG_STORAGE_CHANGE_TYPE_T) + sizeof(int) + (sizeof(MSG_MESSAGE_ID_T)*count);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &storageChangeType, sizeof(MSG_STORAGE_CHANGE_TYPE_T));
	p = (void*)((char*)p + sizeof(MSG_STORAGE_CHANGE_TYPE_T));

	memcpy(p, &count, sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	for (int i = 0; i < count; i++) {
		memcpy(p, &(pMsgIdList->msgIdList[i]), sizeof(MSG_MESSAGE_ID_T));
		p = (void*)((char*)p + sizeof(MSG_MESSAGE_ID_T));
	}

	return dataSize;
}


int MsgEncodeReportStatus(MSG_REPORT_STATUS_INFO_S* pReportStatus, char **ppDest)
{
	int dataSize = 0;

	dataSize = (sizeof(MSG_REPORT_STATUS_INFO_S));

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, pReportStatus, dataSize);

	return dataSize;
}


// Decoders
void MsgDecodeMsgId(char *pSrc, MSG_MESSAGE_ID_T *pMsgId)
{
	memcpy(pMsgId, pSrc, sizeof(MSG_MESSAGE_ID_T));
}


void MsgDecodeCountInfo(char *pSrc, MSG_COUNT_INFO_S *pCountInfo)
{
	memcpy(pCountInfo, pSrc, sizeof(MSG_COUNT_INFO_S));
}


void MsgDecodeMemSize(char *pSrc, unsigned int *memsize)
{
	memcpy(memsize, pSrc, sizeof(unsigned int));
}


void MsgDecodeMsgInfo(char *pSrc, MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S* pSendOptInfo)
{
	memcpy(pMsgInfo, pSrc, sizeof(MSG_MESSAGE_INFO_S));

	pSrc = pSrc + sizeof(MSG_MESSAGE_INFO_S);

	memcpy(pSendOptInfo, pSrc, sizeof(MSG_SENDINGOPT_INFO_S));
}


void MsgDecodeFolderViewList(char *pSrc, MSG_LIST_S *pFolderViewList)
{
	int count = 0;

	memcpy(&count, pSrc, sizeof(int));
	pSrc = pSrc + sizeof(int);

	if( count > 0 )
	{
		pFolderViewList->nCount = count;
		pFolderViewList->msgInfo = (msg_message_t*)new char[sizeof(MSG_MESSAGE_S)*count];

		MSG_MESSAGE_S* pInfoTmp = (MSG_MESSAGE_S*)pFolderViewList->msgInfo;

		for (int i = 0; i < count; i++)
		{
			memcpy(pInfoTmp, pSrc, sizeof(MSG_MESSAGE_S));
			pSrc = pSrc + sizeof(MSG_MESSAGE_S);
			pInfoTmp++;
		}
	}
	else if ( count == 0 )
	{
		pFolderViewList->nCount = count;
		pFolderViewList->msgInfo = NULL;
	}
}


void MsgDecodeRecipientList(char *pSrc, MSG_RECIPIENTS_LIST_S *pRecipientList)
{
	int count = 0;

	memcpy(&count, pSrc, sizeof(int));
	pSrc = pSrc + sizeof(int);

	pRecipientList->recipientCnt= count;
	pRecipientList->recipientAddr = (MSG_ADDRESS_INFO_S*)new char[sizeof(MSG_ADDRESS_INFO_S)*count];

	MSG_ADDRESS_INFO_S* pInfoTmp = pRecipientList->recipientAddr;

	for (int i = 0; i < count; i++)
	{
		memcpy(pInfoTmp, pSrc, sizeof(MSG_ADDRESS_INFO_S));
		pSrc = pSrc + sizeof(MSG_ADDRESS_INFO_S);
		pInfoTmp++;
	}
}


void MsgDecodeFolderList(char *pSrc, MSG_FOLDER_LIST_S *pFolderList)
{
	int count = 0;

	memcpy(&count, pSrc, sizeof(int));
	pSrc = pSrc + sizeof(int);

	pFolderList->nCount = count;
	pFolderList->folderInfo = (MSG_FOLDER_INFO_S*)new char[sizeof(MSG_FOLDER_INFO_S)*count];

	MSG_FOLDER_INFO_S* pInfoTmp = pFolderList->folderInfo;

	for (int i = 0; i < count; i++)
	{
		memcpy(pInfoTmp, pSrc, sizeof(MSG_FOLDER_INFO_S));
		pSrc = pSrc + sizeof(MSG_FOLDER_INFO_S);
		pInfoTmp++;
	}
}


void MsgDecodeSetting(char *pSrc, MSG_SETTING_S *pSetting)
{
	int dataSize = sizeof(MSG_OPTION_TYPE_T);

	switch (pSetting->type)
	{
		case MSG_GENERAL_OPT :
			dataSize += sizeof(MSG_GENERAL_OPT_S);
		break;
		case MSG_SMS_SENDOPT :
			dataSize += sizeof(MSG_SMS_SENDOPT_S);
		break;
		case MSG_SMSC_LIST :
			dataSize += sizeof(MSG_SMSC_LIST_S);
		break;
		case MSG_MMS_SENDOPT :
			dataSize += sizeof(MSG_MMS_SENDOPT_S);
		break;
		case MSG_MMS_RECVOPT :
			dataSize += sizeof(MSG_MMS_RECVOPT_S);
		break;
		case MSG_MMS_STYLEOPT :
			dataSize += sizeof(MSG_MMS_STYLEOPT_S);
		break;
		case MSG_PUSHMSG_OPT :
			dataSize += sizeof(MSG_PUSHMSG_OPT_S);
		break;
		case MSG_CBMSG_OPT :
			dataSize += sizeof(MSG_CBMSG_OPT_S);
		break;
		case MSG_VOICEMAIL_OPT :
			dataSize += sizeof(MSG_VOICEMAIL_OPT_S);
		break;
		case MSG_MSGSIZE_OPT :
			dataSize += sizeof(MSG_MSGSIZE_OPT_S);
		break;
	}

	memcpy(pSetting, pSrc, dataSize);
}


void MsgDecodeMsgType(char *pSrc, MSG_MESSAGE_TYPE_S* pMsgType)
{
	memcpy(pMsgType, pSrc, sizeof(MSG_MESSAGE_TYPE_S));
}


void MsgDecodeThreadViewList(char *pSrc, MSG_THREAD_VIEW_LIST_S *pThreadViewList)
{
	int count = 0;

	memcpy(&count, pSrc, sizeof(int));
	pSrc = pSrc + sizeof(int);

	if (count > 0)
	{
		pThreadViewList->nCount = count;
		pThreadViewList->msgThreadInfo = (msg_thread_view_t*)new char[sizeof(MSG_THREAD_VIEW_S)*count];

		MSG_THREAD_VIEW_S* pInfoTmp = (MSG_THREAD_VIEW_S*)pThreadViewList->msgThreadInfo;

		for (int i = 0; i < count; i++)
		{
			memcpy(pInfoTmp, pSrc, sizeof(MSG_THREAD_VIEW_S));
			pSrc = pSrc + sizeof(MSG_THREAD_VIEW_S);
			pInfoTmp++;
		}
	}
	else if (count == 0)
	{
		pThreadViewList->nCount = count;
		pThreadViewList->msgThreadInfo = NULL;
	}
}


void MsgDecodeConversationViewList(char *pSrc, MSG_LIST_S *pConvViewList)
{
	int count = 0;

	memcpy(&count, pSrc, sizeof(int));
	pSrc = pSrc + sizeof(int);

	if (count > 0)
	{
		pConvViewList->nCount = count;
		pConvViewList->msgInfo = (msg_message_t*)new char[sizeof(MSG_MESSAGE_S)*count];

		MSG_MESSAGE_S* pInfoTmp = (MSG_MESSAGE_S*)pConvViewList->msgInfo;

		for (int i = 0; i < count; i++)
		{
			memcpy(pInfoTmp, pSrc, sizeof(MSG_MESSAGE_S));
			pSrc = pSrc + sizeof(MSG_MESSAGE_S);
			pInfoTmp++;
		}
	}
	else if (count == 0)
	{
		pConvViewList->nCount = count;
		pConvViewList->msgInfo = NULL;
	}
}


void	MsgDecodeContactCount(char *pSrc,  MSG_THREAD_COUNT_INFO_S *pMsgThreadCountList)
{
	int count = 0;

	if(pSrc == NULL)
		return;

	memcpy(&count, pSrc, sizeof(int));
	pSrc = pSrc + sizeof(int);
	pMsgThreadCountList->totalCount	= count;


	memcpy(&count, pSrc, sizeof(int));
	pSrc = pSrc + sizeof(int);
	pMsgThreadCountList->unReadCount	= count;


	memcpy(&count, pSrc, sizeof(int));
	pSrc = pSrc + sizeof(int);
	pMsgThreadCountList->mmsMsgCount	= count;


	memcpy(&count, pSrc, sizeof(int));
	pSrc = pSrc + sizeof(int);
	pMsgThreadCountList->smsMsgCount	= count;


	return;
}


void	MsgDecodeReportStatus(char *pSrc,  MSG_REPORT_STATUS_INFO_S *pReportStatus)
{
	int count = 0;

	if(pSrc == NULL)
		return;

	memcpy(&count, pSrc, sizeof(MSG_DELIVERY_REPORT_STATUS_T));
	pSrc = pSrc + sizeof(MSG_DELIVERY_REPORT_STATUS_T);
	pReportStatus->deliveryStatus = count;


	memcpy(&count, pSrc, sizeof(time_t));
	pSrc = pSrc + sizeof(time_t);
	pReportStatus->deliveryStatusTime = count;


	memcpy(&count, pSrc, sizeof(MSG_READ_REPORT_STATUS_T));
	pSrc = pSrc + sizeof(MSG_READ_REPORT_STATUS_T);
	pReportStatus->readStatus = count;


	memcpy(&count, pSrc, sizeof(time_t));
	pSrc = pSrc + sizeof(time_t);
	pReportStatus->readStatusTime = count;


	return;
}


// Event Encoder
int MsgMakeEvent(const void *pData, int DataSize, MSG_EVENT_TYPE_T MsgEvent, MSG_ERROR_T MsgError, void **ppEvent)
{
	MSG_EVENT_S* pMsgEvent = NULL;

	*ppEvent = (MSG_EVENT_S*)new char[sizeof(MSG_EVENT_S) + DataSize];

	pMsgEvent = (MSG_EVENT_S*)*ppEvent;

	pMsgEvent->eventType = MsgEvent;
	pMsgEvent->result = MsgError;

	MSG_DEBUG("eventType [%d : %s]", pMsgEvent->eventType, MsgDbgEvtStr(pMsgEvent->eventType));
	MSG_DEBUG("result [%d]", pMsgEvent->result);

	if (DataSize > 0)
		memcpy((void*)pMsgEvent->data, pData, DataSize);

	return (sizeof(MSG_EVENT_S) + DataSize);
}
