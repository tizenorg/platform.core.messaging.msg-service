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
