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

