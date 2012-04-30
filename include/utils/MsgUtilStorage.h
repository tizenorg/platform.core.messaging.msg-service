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

#ifndef MSG_UTIL_STORAGE_H
#define MSG_UTIL_STORAGE_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgInternalTypes.h"
#include "MsgStorageTypes.h"
#include "MsgSqliteWrapper.h"


/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/
// Common Function
unsigned int MsgStoAddMessageTable(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsgInfo, unsigned int AddrId);
MSG_ERROR_T MsgStoSetReadStatus(MsgDbHandler *pDbHandle, MSG_MESSAGE_ID_T MsgId, bool bRead);
MSG_ERROR_T MsgStoGetOldestMessage(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsgInfo, MSG_MESSAGE_ID_T *pMsgId);
MSG_ERROR_T MsgStoCheckMsgCntFull(MsgDbHandler *pDbHandle, const MSG_MESSAGE_TYPE_S* pMsgType, MSG_FOLDER_ID_T FolderId);
MSG_ERROR_T MsgStoCountMsgByLimitCategory(MsgDbHandler *pDbHandle, const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount, MSG_FOLDER_ID_T folderId );
int MsgStoCheckMsgCntLimit(const MSG_MESSAGE_TYPE_S* pMsgType, MSG_FOLDER_ID_T FolderId);

MSG_ERROR_T MsgStoAddAddress(MsgDbHandler *pDbHandle, const MSG_ADDRESS_INFO_S *pAddrInfo, unsigned int *pAddrId);
MSG_ERROR_T MsgStoUpdateAddress(MsgDbHandler *pDbHandle, unsigned int AddrId);
MSG_ERROR_T MsgStoClearAddressTable(MsgDbHandler *pDbHandle);
bool MsgExistAddress(MsgDbHandler *pDbHandle, const char *pAddress, unsigned int *pAddrId);

int MsgStoGetUnreadCnt(MsgDbHandler *pDbHandle, MSG_MAIN_TYPE_T MsgType);
MSG_ERROR_T MsgStoAddContactInfo(MsgDbHandler *pDbHandle, MSG_CONTACT_INFO_S *pContactInfo, const char *pNumber);
MSG_ERROR_T MsgStoClearContactInfo(MsgDbHandler *pDbHandle, int ContactId);
MSG_ERROR_T MsgStoClearContactInfo(MsgDbHandler *pDbHandle, int ContactId, const char *pNumber);
MSG_ERROR_T MsgStoGetMmsRawFilePath(MsgDbHandler *pDbHandle, MSG_MESSAGE_ID_T msgId, char *pFilePath);
bool MsgStoCheckReadReportRequested(MsgDbHandler *pDbHandle, MSG_MESSAGE_ID_T MsgId);
bool MsgStoCheckReadReportIsSent(MsgDbHandler *pDbHandle, MSG_MESSAGE_ID_T MsgId);
char *MsgStoReplaceString(const char *org_str, const char *old_str, const char *new_str);

#endif // MSG_UTIL_STORAGE_H

