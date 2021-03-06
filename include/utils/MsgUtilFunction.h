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

#ifndef MSG_UTIL_FUNCTION_H
#define MSG_UTIL_FUNCTION_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgStorageTypes.h"
#include "MsgSettingTypes.h"
#include "MsgFilterTypes.h"
#include "MsgInternalTypes.h"
#include "MsgCmdTypes.h"

/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/

// Encoders
int MsgEncodeCountInfo(MSG_COUNT_INFO_S *pCountInfo, char **ppDest);

int MsgEncodeCountByMsgType(int MsgCount, char **ppDest);

int MsgEncodeRecipientList(MSG_RECIPIENTS_LIST_S *pRecipientList, char **ppDest);

int MsgEncodeMsgId(msg_message_id_t *pMsgId, char **ppDest);

int MsgEncodeMsgInfo(MSG_MESSAGE_INFO_S *pMsgInfo, char **ppDest);

int MsgEncodeMsgInfo(MSG_MESSAGE_INFO_S *pMsgInfo,  MSG_SENDINGOPT_INFO_S* pSendOptInfo, char **ppDest);

int MsgEncodeFolderViewList(msg_struct_list_s *pFolderViewList, char **ppDest);

int MsgEncodeFolderList(msg_struct_list_s *pFolderList, char **ppDest);

int MsgEncodeSetting(MSG_SETTING_S *pSetting, char **ppDest);

int MsgEncodeFilterList(msg_struct_list_s *pFilterList, char **ppDest);

int MsgEncodeFilterFlag(bool *pSetFlag, char **ppDest);

int MsgEncodeMsgType(MSG_MESSAGE_TYPE_S *pMsgType, char **ppDest);

int MsgEncodeThreadViewList(msg_struct_list_s *pThreadViewList, char **ppDest);

int MsgEncodeConversationViewList(msg_struct_list_s *pConvViewList, char **ppDest);

int MsgEncodeMsgGetContactCount(MSG_THREAD_COUNT_INFO_S *threadCountInfo, char **ppDest);

int MsgEncodeMemSize(unsigned int *memsize, char **ppDest);

int MsgEncodeSyncMLOperationData(int msgId, int extId, char **ppDest);

int MsgEncodeStorageChangeData(const msg_storage_change_type_t storageChangeType, const msg_id_list_s *pMsgIdList, char **ppDest);

int MsgEncodeReportStatus(MSG_REPORT_STATUS_INFO_S* pReportStatus, char **ppDest);

int MsgEncodeThreadId(msg_thread_id_t *pThreadId, char **ppDest);

int MsgEncodeThreadInfo(MSG_THREAD_VIEW_S *pThreadInfo, char **ppDest);


// Decoders
void MsgDecodeMsgId(char *pSrc, msg_message_id_t *pMsgId);

void MsgDecodeCountInfo(char *pSrc, MSG_COUNT_INFO_S *pCountInfo);

void MsgDecodeMsgInfo(char *pSrc, MSG_MESSAGE_INFO_S *pMsgInfo,  MSG_SENDINGOPT_INFO_S* pSendOptInfo);

void MsgDecodeFolderViewList(char *pSrc, msg_struct_list_s *pFolderViewList);

void MsgDecodeRecipientList(char *pSrc, MSG_RECIPIENTS_LIST_S *pRecipientList);

void MsgDecodeFolderList(char *pSrc, msg_struct_list_s *pFolderList);

void MsgDecodeSetting(char *pSrc, MSG_SETTING_S *pSetting);

void MsgDecodeFilterList(char *pSrc, msg_struct_list_s *pFilterList);

void MsgDecodeFilterFlag(char *pSrc, bool *pSetFlag);

void MsgDecodeMsgType(char *pSrc, MSG_MESSAGE_TYPE_S* pMsgType);

void	MsgDecodeContactCount(char *pSrc,  MSG_THREAD_COUNT_INFO_S *pMsgThreadCountList);

void MsgDecodeMemSize(char *pSrc, unsigned int *memsize);

void	MsgDecodeReportStatus(char *pSrc,  MSG_REPORT_STATUS_INFO_S *pReportStatus);

void MsgDecodeThreadId(char *pSrc, msg_thread_id_t *pThreadId);

void MsgDecodeThreadInfo(char *pSrc, MSG_THREAD_VIEW_S *pThreadInfo);


// Event Encoder
int MsgMakeEvent(const void *pData, int DataSize, MSG_EVENT_TYPE_T MsgEvent, msg_error_t MsgError, void **ppEvent);


int msg_verify_number(const char *raw, char *trimmed);

int msg_verify_email(const char *raw);

#endif // MSG_UTIL_FUNCTION_H
