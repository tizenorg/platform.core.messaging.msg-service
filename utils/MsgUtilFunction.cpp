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

#include <gio/gio.h>

#include "MsgDebug.h"
#include "MsgContact.h"
#include "MsgUtilFile.h"
#include "MsgUtilFunction.h"

#include <system_info.h>
#include <libintl.h>
#include <locale.h>
#include <vconf.h>
#include <ctype.h>


enum _FEATURE_INDEX_E {
	FEATURE_INDEX_SMS = 0,
	FEATURE_INDEX_MMS = 1,
};

static bool b_feature_cache_flag = false;
static bool b_feature_support[] = {
		[FEATURE_INDEX_SMS] = false,
		[FEATURE_INDEX_MMS] = false,
};

int _dbus_owner_id = 0;

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

bool MsgCheckFeatureSupport(const char *feature_name)
{
	bool result = false;

	if (b_feature_cache_flag == false) {
		if (system_info_get_platform_bool(MSG_TELEPHONY_SMS_FEATURE, &b_feature_support[FEATURE_INDEX_SMS]) != SYSTEM_INFO_ERROR_NONE)
			MSG_WARN("fail to system_info_get_platform_bool [%s]", MSG_TELEPHONY_SMS_FEATURE);

		if (system_info_get_platform_bool(MSG_TELEPHONY_MMS_FEATURE, &b_feature_support[FEATURE_INDEX_MMS]) != SYSTEM_INFO_ERROR_NONE)
			MSG_WARN("fail to system_info_get_platform_bool [%s]", MSG_TELEPHONY_MMS_FEATURE);

		MSG_INFO("[%s] feature is [%d]", MSG_TELEPHONY_SMS_FEATURE, b_feature_support[FEATURE_INDEX_SMS]);
		MSG_INFO("[%s] feature is [%d]", MSG_TELEPHONY_MMS_FEATURE, b_feature_support[FEATURE_INDEX_MMS]);

		b_feature_cache_flag = true;
	}

	if (!g_strcmp0(feature_name, MSG_TELEPHONY_SMS_FEATURE)) {
		result = b_feature_support[FEATURE_INDEX_SMS];
	} else if (!g_strcmp0(feature_name, MSG_TELEPHONY_MMS_FEATURE)) {
		result = b_feature_support[FEATURE_INDEX_MMS];
	}

	return result;
}

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


int MsgEncodeMsgInfo(const MSG_MESSAGE_INFO_S *pMsgInfo, char **ppDest)
{
	int dataSize = 0;

	dataSize = sizeof(MSG_MESSAGE_INFO_S) + (sizeof(MSG_ADDRESS_INFO_S)*pMsgInfo->nAddressCnt);

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

	p = (void*)((char*)p + sizeof(MSG_MESSAGE_INFO_S));

	for (int i=0; i < pMsgInfo->nAddressCnt; i++) {
		memcpy(p, &(pMsgInfo->addressList[i]), sizeof(MSG_ADDRESS_INFO_S));
		p = (void*)((char*)p + sizeof(MSG_ADDRESS_INFO_S));
	}

	return dataSize;
}


int MsgEncodeMsgInfo(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S* pSendOptInfo, char **ppDest)
{
	int dataSize = 0;

	dataSize = (sizeof(MSG_MESSAGE_INFO_S) + sizeof(MSG_SENDINGOPT_INFO_S) + (sizeof(MSG_ADDRESS_INFO_S)*pMsgInfo->nAddressCnt));

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

	p = (void*)((char*)p + sizeof(MSG_MESSAGE_INFO_S));

	memcpy(p, pSendOptInfo, sizeof(MSG_SENDINGOPT_INFO_S));

	p = (void*)((char*)p + sizeof(MSG_SENDINGOPT_INFO_S));

	for (int i=0; i < pMsgInfo->nAddressCnt; i++) {
		memcpy(p, &(pMsgInfo->addressList[i]), sizeof(MSG_ADDRESS_INFO_S));
		p = (void*)((char*)p + sizeof(MSG_ADDRESS_INFO_S));
	}

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


int MsgEncodeReportMsgData(const msg_report_type_t msgReportType, const MSG_MESSAGE_INFO_S *pMsgInfo, char **ppDest)
{
	int dataSize = 0;
	int addr_len = 0;

	addr_len = strlen(pMsgInfo->addressList->addressVal);

	dataSize = sizeof(msg_report_type_t) + sizeof(msg_message_id_t) + sizeof(int) + addr_len;

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &msgReportType, sizeof(msg_report_type_t));
	p = (void*)((char*)p + sizeof(msg_report_type_t));

	memcpy(p, &(pMsgInfo->msgId), sizeof(msg_message_id_t));
	p = (void*)((char*)p + sizeof(msg_message_id_t));

	memcpy(p, &addr_len, sizeof(int));
	p = (void*)((char*)p + sizeof(int));

	memcpy(p, &(pMsgInfo->addressList->addressVal), addr_len);
	p = (void*)((char*)p + addr_len);

	return dataSize;
}


int MsgEncodeReportStatus(MSG_REPORT_STATUS_INFO_S* pReportStatus, int count, char **ppDest)
{
	int dataSize = 0;

	dataSize = (sizeof(MSG_REPORT_STATUS_INFO_S)*count + sizeof(int));

	*ppDest = (char*)new char[dataSize];

	void* p = (void*)*ppDest;

	memcpy(p, &count, sizeof(int));

	p = (void*)((char*)p + sizeof(int));

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


void MsgDecodeMsgInfo(char *pSrc, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	memcpy(pMsgInfo, pSrc, sizeof(MSG_MESSAGE_INFO_S));

	pSrc = pSrc + sizeof(MSG_MESSAGE_INFO_S);

	pMsgInfo->addressList = NULL;

	pMsgInfo->addressList = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S) * pMsgInfo->nAddressCnt];
	memset(pMsgInfo->addressList, 0x00, sizeof(MSG_ADDRESS_INFO_S) * pMsgInfo->nAddressCnt);

	for (int i=0; i<pMsgInfo->nAddressCnt; i++) {
		memcpy(&(pMsgInfo->addressList[i]), pSrc + (sizeof(MSG_ADDRESS_INFO_S)*i), sizeof(MSG_ADDRESS_INFO_S));
	}
}


void MsgDecodeMsgInfo(char *pSrc, MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S* pSendOptInfo)
{
	memcpy(pMsgInfo, pSrc, sizeof(MSG_MESSAGE_INFO_S));

	pSrc = pSrc + sizeof(MSG_MESSAGE_INFO_S);

	memcpy(pSendOptInfo, pSrc, sizeof(MSG_SENDINGOPT_INFO_S));

	pSrc = pSrc + sizeof(MSG_SENDINGOPT_INFO_S);


	if(pMsgInfo->nAddressCnt > 0) {
		pMsgInfo->addressList = NULL;

		pMsgInfo->addressList = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S) * pMsgInfo->nAddressCnt];
		memset(pMsgInfo->addressList, 0x00, sizeof(MSG_ADDRESS_INFO_S) * pMsgInfo->nAddressCnt);

		for (int i=0; i<pMsgInfo->nAddressCnt; i++) {
			memcpy(&(pMsgInfo->addressList[i]), pSrc + (sizeof(MSG_ADDRESS_INFO_S)*i), sizeof(MSG_ADDRESS_INFO_S));
		}
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


void MsgDecodeFolderList(char *pSrc, msg_struct_list_s *pFolderList)
{
	int count = 0;

	memcpy(&count, pSrc, sizeof(int));
	pSrc = pSrc + sizeof(int);

	if( count > 0 )
	{
		pFolderList->nCount = count;
		pFolderList->msg_struct_info = (msg_struct_t *)calloc(count, sizeof(msg_struct_t));
		if (pFolderList->msg_struct_info == NULL)
		{
			pFolderList->nCount = 0;
			return;
		}
		msg_struct_s *pInfoTmp = NULL;

		for (int i = 0; i < count; i++)
		{
			pFolderList->msg_struct_info[i] = (msg_struct_t )new msg_struct_s;
			pInfoTmp = (msg_struct_s *)pFolderList->msg_struct_info[i];
			pInfoTmp->type = MSG_STRUCT_FOLDER_INFO;
			pInfoTmp->data = new MSG_FOLDER_INFO_S;
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
		pFilterList->msg_struct_info = (msg_struct_t *)calloc(count, sizeof(MSG_FILTER_S *));

		if (pFilterList->msg_struct_info == NULL) {
			pFilterList->nCount = 0;
			return;
		}

		msg_struct_s *pStructTmp = NULL;

		for (int i = 0; i < count; i++)
		{
			pFilterList->msg_struct_info[i] = (msg_struct_t )new msg_struct_s;
			pStructTmp = (msg_struct_s *)pFilterList->msg_struct_info[i];
			pStructTmp->type = MSG_STRUCT_FILTER;
			pStructTmp->data = new MSG_FILTER_S;
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

	if (*ppEvent) {
		MSG_DEBUG("*ppEvent is not NULL.");
		delete [] (char *)*ppEvent;
	}

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


char* msg_clean_country_code(char *src)
{
	int ret = 1;

	switch (src[ret++]-'0')
	{
		case 1:
		case 7:
			break;
		case 2:
			switch (src[ret++]-'0')
			{
				case 0:
				case 7:
					break;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 8:
				case 9:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 3:
			switch (src[ret++]-'0')
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 6:
				case 9:
					break;
				case 5:
				case 7:
				case 8:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 4:
			switch (src[ret++]-'0')
			{
				case 0:
				case 1:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					break;
				case 2:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 5:
			switch (src[ret++]-'0')
			{
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
					break;
				case 0:
				case 9:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 6:
			switch (src[ret++]-'0')
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
					break;
				case 7:
				case 8:
				case 9:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 8:
			switch (src[ret++]-'0')
			{
				case 1:
				case 2:
				case 4:
				case 6:
					break;
				case 0:
				case 3:
				case 5:
				case 7:
				case 8:
				case 9:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 9:
			switch (src[ret++]-'0')
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 8:
					break;
				case 6:
				case 7:
				case 9:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 0:
		default:
			MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
			return src;
	}

	return &src[ret];
}


char* msg_normalize_number(char *src)
{
	char *normalized_number;

	if ('+' == src[0])
		normalized_number = msg_clean_country_code(src);
	else if ('0' == src[0])
		normalized_number = src+1;
	else
		normalized_number = src;

	MSG_DEBUG("src = %s, normalized = %s", src, normalized_number);

	return normalized_number;
}


msg_error_t MsgMakeSortRule(const MSG_SORT_RULE_S *pSortRule, char *pSqlSort)
{
	char sql[128];
	char order[6];

	memset(sql, 0x00, sizeof(sql));
	memset(order, 0x00, sizeof(order));

	if (pSortRule->bAscending == true)
		strncpy(order, "ASC", 5);
	else
		strncpy(order, "DESC", 5);

	int nameOrder = MsgGetContactNameOrder();

	switch (pSortRule->sortType)
	{
		case MSG_SORT_BY_DISPLAY_FROM :
			if (nameOrder == 0)
				snprintf(sql, sizeof(sql), "ORDER BY B.FIRST_NAME %s, B.LAST_NAME %s, B.ADDRESS_VAL, A.DISPLAY_TIME DESC;", order, order);
			else
				snprintf(sql, sizeof(sql), "ORDER BY B.LAST_NAME %s, B.FIRST_NAME %s, B.ADDRESS_VAL, A.DISPLAY_TIME DESC;", order, order);
			break;
		case MSG_SORT_BY_DISPLAY_TO :
			if (nameOrder == 0)
				snprintf(sql, sizeof(sql), "ORDER BY B.FIRST_NAME %s, B.LAST_NAME %s, B.ADDRESS_VAL, A.DISPLAY_TIME DESC;", order, order);
			else
				snprintf(sql, sizeof(sql), "ORDER BY B.LAST_NAME %s, B.FIRST_NAME %s, B.ADDRESS_VAL, A.DISPLAY_TIME DESC;", order, order);
			break;
		case MSG_SORT_BY_DISPLAY_TIME :
			snprintf(sql, sizeof(sql), "ORDER BY DISPLAY_TIME %s;", order);
			break;
		case MSG_SORT_BY_MSG_TYPE :
			snprintf(sql, sizeof(sql), "ORDER BY MAIN_TYPE %s, DISPLAY_TIME DESC;", order);
			break;
		case MSG_SORT_BY_READ_STATUS :
			snprintf(sql, sizeof(sql), "ORDER BY READ_STATUS %s, DISPLAY_TIME DESC;", order);
			break;
		case MSG_SORT_BY_STORAGE_TYPE :
			snprintf(sql, sizeof(sql), "ORDER BY A.STORAGE_ID %s, A.DISPLAY_TIME DESC;", order);
			break;
		case MSG_SORT_BY_THREAD_NAME :
			if (nameOrder == 0)
				snprintf(sql, sizeof(sql), "ORDER BY FIRST_NAME %s, LAST_NAME %s;", order, order);
			else
				snprintf(sql, sizeof(sql), "ORDER BY LAST_NAME %s, FIRST_NAME %s;", order, order);
			break;
		case MSG_SORT_BY_THREAD_DATE :
			snprintf(sql, sizeof(sql), "ORDER BY MSG_TIME %s;", order);
			break;
		case MSG_SORT_BY_THREAD_COUNT :
			snprintf(sql, sizeof(sql), "ORDER BY UNREAD_CNT %s;", order);
			break;
		default :
			snprintf(sql, sizeof(sql), "ORDER BY A.DISPLAY_TIME %s;", order);
			break;
	}

	memcpy(pSqlSort, sql, strlen(sql));

	return MSG_SUCCESS;
}
bool msg_is_valid_email(char *pAddress)
{
	if (!pAddress || pAddress[0] == 0)
		return false;
	if (!strchr (pAddress, MSG_UTIL_CH_EMAIL_AT))
		return false;
	return true;
}

msg_error_t msg_write_text_to_msg_info(MSG_MESSAGE_INFO_S *pMsgInfo, char *text)
{
	if (pMsgInfo->dataSize > MAX_MSG_TEXT_LEN) {
		pMsgInfo->bTextSms = false;

		// Save Message Data into File
		char fileName[MSG_FILENAME_LEN_MAX+1];
		memset(fileName, 0x00, sizeof(fileName));

		if(MsgCreateFileName(fileName) == false) {
			MSG_DEBUG("MsgCreateFileName error");
			return MSG_ERR_STORAGE_ERROR;
		}

		MSG_SEC_DEBUG("Save text into file : size[%d] name[%s]", pMsgInfo->dataSize, fileName);

		if (MsgWriteIpcFile(fileName, text, pMsgInfo->dataSize) == false) {
			MSG_DEBUG("MsgWriteIpcFile error");
			return MSG_ERR_STORAGE_ERROR;
		}

		memset(pMsgInfo->msgData, 0x00, sizeof(pMsgInfo->msgData));
		strncpy(pMsgInfo->msgData, fileName, MAX_MSG_DATA_LEN);
	} else {
		pMsgInfo->bTextSms = true;

		memset(pMsgInfo->msgText, 0x00, sizeof(pMsgInfo->msgText));
		memcpy(pMsgInfo->msgText, text, pMsgInfo->dataSize);
	}

	return MSG_SUCCESS;
}

/* change illegal filename character to '_' */
void msg_replace_available_file_name(char *fileName)
{
	int idx = 0;
	int len = 0;
	bool is_converted = false;

	if (fileName) {
		len = strlen(fileName);

		while (fileName[idx] != 0) {
			if (idx >= len) {
				MSG_WARN("idx : %d, len : %d", idx, len);
				break;
			}

			if (fileName[idx] == '\\' || fileName[idx] == '/' || fileName[idx] == '?' || fileName[idx] == '%' || fileName[idx] == '*' ||
				fileName[idx] == ':' || fileName[idx] == '|' || fileName[idx] == '"' || fileName[idx] == '<' || fileName[idx] == '>') {
				fileName[idx++] = '_';
				is_converted = true;
			} else {
				idx++;
			}
		}
	}

	if (is_converted)
		MSG_SEC_DEBUG("converted filename : [%s]", fileName);
}

/* change character ' ' to '_' */
void msg_replace_space_char(char *pszText)
{
	if (!pszText) {
		MSG_ERR("pszText is NULL");
		return;
	}

	char *spaceCharPtr = strchr(pszText, ' ');

	while (spaceCharPtr) {
		*spaceCharPtr = '_';
		spaceCharPtr = strchr(pszText, ' ');
	}
}

/* change non-ascii character to underscore */
gchar * msg_replace_non_ascii_char(const gchar *pszText, gunichar replacementChar)
{
	if (!pszText) {
		MSG_ERR(" msg_replace_non_ascii_char error : pszText is NULL");
		return NULL;
	}
	gchar *res;
	gsize result_len = 0;
	const gchar *p;
	result_len = g_utf8_strlen (pszText, -1) + 1; //+1 for malloc of non-terminating chracter
	res = (gchar *)g_malloc (result_len * sizeof (gchar));
	int i = 0;
	for (p = pszText, i = 0; *p != '\0'; p = g_utf8_next_char (p), i++) {
		res[i] = isascii(g_utf8_get_char (p)) ? *p : replacementChar;
	}
	res[i] = '\0';
	return res;
}

void MsgDbusInit()
{
	MSG_DEBUG();

	_dbus_owner_id = g_bus_own_name(G_BUS_TYPE_SYSTEM,
									"msg-service.dbus.service",
									G_BUS_NAME_OWNER_FLAGS_NONE,
									NULL, NULL, NULL,
									NULL, NULL);

	if (_dbus_owner_id == 0) {
		MSG_ERR("g_bus_own_name() error");
	}

	MSG_DEBUG("owner_id = [%d]", _dbus_owner_id);
}

void MsgDbusDeinit()
{
	MSG_DEBUG();
	if (_dbus_owner_id)
		g_bus_unown_name(_dbus_owner_id);

	_dbus_owner_id = 0;
}
