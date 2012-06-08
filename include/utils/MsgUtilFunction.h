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
#include "MsgInternalTypes.h"
#include "MsgCmdTypes.h"

/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/

// Encoders
int MsgEncodeCountInfo(MSG_COUNT_INFO_S *pCountInfo, char **ppDest);

int MsgEncodeCountByMsgType(int MsgCount, char **ppDest);

int MsgEncodeRecipientList(MSG_RECIPIENTS_LIST_S *pRecipientList, char **ppDest);

int MsgEncodeMsgId(MSG_MESSAGE_ID_T *pMsgId, char **ppDest);

int MsgEncodeMsgInfo(MSG_MESSAGE_INFO_S *pMsgInfo, char **ppDest);

int MsgEncodeMsgInfo(MSG_MESSAGE_INFO_S *pMsgInfo,  MSG_SENDINGOPT_INFO_S* pSendOptInfo, char **ppDest);

int MsgEncodeFolderViewList(MSG_LIST_S *pFolderViewList, char **ppDest);

int MsgEncodeFolderList(MSG_FOLDER_LIST_S *pFolderList, char **ppDest);

int MsgEncodeSetting(MSG_SETTING_S *pSetting, char **ppDest);

int MsgEncodeMsgType(MSG_MESSAGE_TYPE_S *pMsgType, char **ppDest);

int MsgEncodeThreadViewList(MSG_THREAD_VIEW_LIST_S *pThreadViewList, char **ppDest);

int MsgEncodeConversationViewList(MSG_LIST_S *pConvViewList, char **ppDest);

int MsgEncodeMsgGetContactCount(MSG_THREAD_COUNT_INFO_S *threadCountInfo, char **ppDest);

int MsgEncodeMemSize(unsigned int *memsize, char **ppDest);

int MsgEncodeSyncMLOperationData(int msgId, int extId, char **ppDest);

int MsgEncodeStorageChangeData(const MSG_STORAGE_CHANGE_TYPE_T storageChangeType, const MSG_MSGID_LIST_S *pMsgIdList, char **ppDest);

int MsgEncodeReportStatus(MSG_REPORT_STATUS_INFO_S* pReportStatus, char **ppDest);


// Decoders
void MsgDecodeMsgId(char *pSrc, MSG_MESSAGE_ID_T *pMsgId);

void MsgDecodeCountInfo(char *pSrc, MSG_COUNT_INFO_S *pCountInfo);

void MsgDecodeMsgInfo(char *pSrc, MSG_MESSAGE_INFO_S *pMsgInfo,  MSG_SENDINGOPT_INFO_S* pSendOptInfo);

void MsgDecodeFolderViewList(char *pSrc, MSG_LIST_S *pFolderViewList);

void MsgDecodeRecipientList(char *pSrc, MSG_RECIPIENTS_LIST_S *pRecipientList);

void MsgDecodeFolderList(char *pSrc, MSG_FOLDER_LIST_S *pFolderList);

void MsgDecodeSetting(char *pSrc, MSG_SETTING_S *pSetting);

void MsgDecodeMsgType(char *pSrc, MSG_MESSAGE_TYPE_S* pMsgType);

void MsgDecodeThreadViewList(char *pSrc, MSG_THREAD_VIEW_LIST_S *pThreadViewList);

void MsgDecodeConversationViewList(char *pSrc, MSG_LIST_S *pConvViewList);

void	MsgDecodeContactCount(char *pSrc,  MSG_THREAD_COUNT_INFO_S *pMsgThreadCountList);

void MsgDecodeMemSize(char *pSrc, unsigned int *memsize);

void	MsgDecodeReportStatus(char *pSrc,  MSG_REPORT_STATUS_INFO_S *pReportStatus);


// Event Encoder
int MsgMakeEvent(const void *pData, int DataSize, MSG_EVENT_TYPE_T MsgEvent, MSG_ERROR_T MsgError, void **ppEvent);


#endif // MSG_UTIL_FUNCTION_H
