/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
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


int MsgEncodeMsgId(msg_message_id_t *pMsgId, char **ppDest)
{
	int dataSize = 0;

	dataSize = (sizeof(msg_message_id_t));

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

int MsgEncodeFolderList(msg_struct_list_s *pFolderList, char **ppDest)
{
	int count = 0, dataSize = 0;

	count = pFolderList->nCount;
	dataSize = sizeof(int) + (sizeof(MSG_FOLDER_INFO_S)*count);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &count, sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	msg_struct_s *folder_info = NULL;

	for (int i = 0; i < count; i++)
	{
		folder_info = (msg_struct_s *)pFolderList->msg_struct_info[i];
		memcpy(p, folder_info->data, sizeof(MSG_FOLDER_INFO_S));
		p = (void*)((char*)p + sizeof(MSG_FOLDER_INFO_S));
	}

	return dataSize;
}

int MsgEncodeFilterList(msg_struct_list_s *pFilterList, char **ppDest)
{
	int count = 0, dataSize = 0;

	count = pFilterList->nCount;
	dataSize = sizeof(int) + (sizeof(MSG_FILTER_S)*count);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &count, sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	msg_struct_s *filter_info = NULL;

	for (int i = 0; i < count; i++)
	{
		filter_info = (msg_struct_s *)pFilterList->msg_struct_info[i];
		memcpy(p, filter_info->data, sizeof(MSG_FILTER_S));
		p = (void*)((char*)p + sizeof(MSG_FILTER_S));
	}

	return dataSize;
}


int MsgEncodeFilterFlag(bool *pSetFlag, char **ppDest)
{
	int dataSize = 0;

	dataSize = (sizeof(bool));

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, pSetFlag, dataSize);

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


int MsgEncodeThreadViewList(msg_struct_list_s *pThreadViewList, char **ppDest)
{
	int count = 0, dataSize = 0;

	count = pThreadViewList->nCount;

	dataSize = sizeof(int) + (sizeof(MSG_THREAD_VIEW_S)*count);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &count, sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	msg_struct_s *thread_info = NULL;

	for (int i = 0; i < count; i++)
	{
		thread_info = (msg_struct_s *)pThreadViewList->msg_struct_info[i];
		memcpy(p, thread_info->data, sizeof(MSG_THREAD_VIEW_S));
		p = (void*)((char*)p + sizeof(MSG_THREAD_VIEW_S));
	}

	return dataSize;
}


int MsgEncodeConversationViewList(msg_struct_list_s *pConvViewList, char **ppDest)
{
	int count = 0, dataSize = 0;

	count = pConvViewList->nCount;

	dataSize = sizeof(int) + (sizeof(msg_struct_list_s)*count);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &count, sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	for (int i = 0; i < count; i++)
	{
		memcpy(p, &(pConvViewList->msg_struct_info[i]), sizeof(msg_struct_list_s));
		p = (void*)((char*)p + sizeof(msg_struct_list_s));
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


int MsgEncodeStorageChangeData(const msg_storage_change_type_t storageChangeType, const msg_id_list_s *pMsgIdList, char **ppDest)
{
	int dataSize = 0;
	int count = 0;

	count = pMsgIdList->nCount;

	dataSize = sizeof(msg_storage_change_type_t) + sizeof(int) + (sizeof(msg_message_id_t)*count);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &storageChangeType, sizeof(msg_storage_change_type_t));
	p = (void*)((char*)p + sizeof(msg_storage_change_type_t));

	memcpy(p, &count, sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	for (int i = 0; i < count; i++) {
		memcpy(p, &(pMsgIdList->msgIdList[i]), sizeof(msg_message_id_t));
		p = (void*)((char*)p + sizeof(msg_message_id_t));
	}

	return dataSize;
}


int MsgEncodeReportStatus(MSG_REPORT_STATUS_INFO_S* pReportStatus, int count, char **ppDest)
{
	int dataSize = 0;

	dataSize = (sizeof(MSG_REPORT_STATUS_INFO_S)*count + sizeof(int));

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &count, sizeof(int));

	p = (void*)((int)p + sizeof(int));

	memcpy(p, pReportStatus, sizeof(MSG_REPORT_STATUS_INFO_S)*count);

	return dataSize;
}


int MsgEncodeThreadId(msg_thread_id_t *pThreadId, char **ppDest)
{
	int dataSize = 0;

	dataSize = (sizeof(msg_thread_id_t));

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, pThreadId, dataSize);

	return dataSize;
}


int MsgEncodeThreadInfo(MSG_THREAD_VIEW_S *pThreadInfo, char **ppDest)
{
	int dataSize = 0;

	dataSize = sizeof(MSG_THREAD_VIEW_S);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, pThreadInfo, sizeof(MSG_THREAD_VIEW_S));

	p = (void*)((char*)p + sizeof(MSG_THREAD_VIEW_S));

	return dataSize;
}



// Decoders
void MsgDecodeMsgId(char *pSrc, msg_message_id_t *pMsgId)
{
	memcpy(pMsgId, pSrc, sizeof(msg_message_id_t));
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


void MsgDecodeFolderList(char *pSrc, msg_struct_list_s *pFolderList)
{
	int count = 0;

	memcpy(&count, pSrc, sizeof(int));
	pSrc = pSrc + sizeof(int);

	if( count > 0 )
	{
		pFolderList->nCount = count;
		pFolderList->msg_struct_info = (msg_struct_t *)new char[sizeof(MSG_FOLDER_INFO_S *)*count];

		msg_struct_s *pInfoTmp = NULL;

		for (int i = 0; i < count; i++)
		{

			pFolderList->msg_struct_info[i] = (msg_struct_t )new char[sizeof(msg_struct_s)];
			pInfoTmp = (msg_struct_s *)pFolderList->msg_struct_info[i];
			pInfoTmp->type = MSG_STRUCT_FOLDER_INFO;
			pInfoTmp->data = new char[sizeof(MSG_FOLDER_INFO_S)];
			memcpy(pInfoTmp->data, pSrc, sizeof(MSG_FOLDER_INFO_S));
			pSrc = pSrc + sizeof(MSG_FOLDER_INFO_S);
		}
	}
	else if ( count == 0 )
	{
		pFolderList->nCount = count;
		pFolderList->msg_struct_info = NULL;
	}
}

void MsgDecodeFilterList(char *pSrc, msg_struct_list_s *pFilterList)
{
	int count = 0;

	memcpy(&count, pSrc, sizeof(int));
	pSrc = pSrc + sizeof(int);

	if( count > 0 )
	{
		pFilterList->nCount = count;
		pFilterList->msg_struct_info = (msg_struct_t *)new char[sizeof(MSG_FILTER_S *)*count];

		msg_struct_s *pStructTmp = NULL;

		for (int i = 0; i < count; i++)
		{
			pFilterList->msg_struct_info[i] = (msg_struct_t )new char[sizeof(msg_struct_s)];
			pStructTmp = (msg_struct_s *)pFilterList->msg_struct_info[i];
			pStructTmp->type = MSG_STRUCT_FILTER;
			pStructTmp->data = new char[sizeof(MSG_FILTER_S)];
			memcpy(pStructTmp->data, pSrc, sizeof(MSG_FILTER_S));
			pSrc = pSrc + sizeof(MSG_FILTER_S);
		}
	}
	else if ( count == 0 )
	{
		pFilterList->nCount = count;
		pFilterList->msg_struct_info = NULL;
	}

}


void MsgDecodeFilterFlag(char *pSrc, bool *pSetFlag)
{
	memcpy(pSetFlag, pSrc, sizeof(bool));
}


void MsgDecodeMsgType(char *pSrc, MSG_MESSAGE_TYPE_S* pMsgType)
{
	memcpy(pMsgType, pSrc, sizeof(MSG_MESSAGE_TYPE_S));
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


void MsgDecodeReportStatus(char *pSrc,  msg_struct_list_s *report_list)
{
	int count = 0;

	if(pSrc == NULL)
		return;

	memcpy(&count, pSrc, sizeof(int));
	pSrc = pSrc + sizeof(int);

	report_list->nCount = count;

	msg_struct_t *report_status =  (msg_struct_t *)new char[sizeof(msg_struct_t)*count];
	for (int i = 0; i < count; i++) {

		msg_struct_s *report_status_item = new msg_struct_s;
		report_status_item->type = MSG_STRUCT_REPORT_STATUS_INFO;
		report_status_item->data = new MSG_REPORT_STATUS_INFO_S;
		memset(report_status_item->data, 0x00, sizeof(MSG_REPORT_STATUS_INFO_S));

		MSG_REPORT_STATUS_INFO_S *report_status_info =  (MSG_REPORT_STATUS_INFO_S *)report_status_item->data;
		memcpy(report_status_info, pSrc, sizeof(MSG_REPORT_STATUS_INFO_S));

		pSrc = pSrc + sizeof(MSG_REPORT_STATUS_INFO_S);

		report_status[i] = (msg_struct_t)report_status_item;

		MSG_DEBUG("Report_type = %d, status addr = %s, status = %d, time = %d",
				report_status_info->type, report_status_info->addressVal,
				report_status_info->status, report_status_info->statusTime);
	}

	report_list->msg_struct_info = report_status;
	return;
}

void MsgDecodeThreadId(char *pSrc, msg_thread_id_t *pThreadId)
{
	memcpy(pThreadId, pSrc, sizeof(msg_thread_id_t));
}

void MsgDecodeThreadInfo(char *pSrc, MSG_THREAD_VIEW_S *pThreadInfo)
{
	memcpy(pThreadInfo, pSrc, sizeof(MSG_THREAD_VIEW_S));
}

// Event Encoder
int MsgMakeEvent(const void *pData, int DataSize, MSG_EVENT_TYPE_T MsgEvent, msg_error_t MsgError, void **ppEvent)
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

int msg_verify_number(const char *raw, char *trimmed)
{
	if (!(raw && trimmed)) {
		MSG_DEBUG("Phone Number is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	for (int i=0, j=0 ; raw[i] ; i++) {
		if ((raw[i] >= '0' && raw[i] <= '9') || raw[i] == '+' || raw[i] == ',' || raw[i] == ' ' \
				|| raw[i] == '*' ||  raw[i] == '#') {
			trimmed[j++] = raw[i];
		} else if (raw[i] == '-') {
			continue;
		} else {
			MSG_DEBUG("Unacceptable character in telephone number: [%c]", raw[i]);
			return MSG_ERR_INVALID_PARAMETER;
		}
	}

	MSG_DEBUG("Trimming [%s]->[%s]", raw, trimmed);
	return MSG_SUCCESS;
}

int msg_verify_email(const char *raw)
{
	bool onlyNum = true;
	bool atExist = false;

	if (!raw) {
		MSG_DEBUG("Email is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	for (int i = 0; raw[i]; i++) {

		if (raw[i] == '@') {
			onlyNum = false;

			if (atExist == false) {
				atExist = true;
				continue;
			} else {
				MSG_DEBUG("Character [@] is included more than twice in email address.");
				return MSG_ERR_INVALID_PARAMETER;
			}
		}

		if ((raw[i] >= '0' && raw[i] <= '9') || raw[i] == '+' || raw[i] == '*' ||  raw[i] == '#') {
			continue;
		} else if ((raw[i] >= 'a' && raw[i] <= 'z') ||(raw[i] >= 'A' && raw[i] <= 'Z') ||(raw[i] == '.') || raw[i] == '_' || raw[i] == '-') {
			onlyNum = false;
			continue;
		} else if (raw[i] == ',') {
			if (onlyNum == false && atExist == false) {
				MSG_DEBUG("Unacceptable type in address.");
				return MSG_ERR_INVALID_PARAMETER;
			}
			atExist = false;
			onlyNum = true;
			continue;
		} else {
			MSG_DEBUG("Unacceptable character in address : [%c]", raw[i]);
			return MSG_ERR_INVALID_PARAMETER;
		}
	}

	return MSG_SUCCESS;
}
