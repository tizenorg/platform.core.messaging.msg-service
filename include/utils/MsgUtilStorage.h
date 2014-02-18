/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
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
unsigned int MsgStoAddMessageTable(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsgInfo);
msg_error_t MsgStoSetReadStatus(MsgDbHandler *pDbHandle, msg_message_id_t MsgId, bool bRead);
msg_error_t MsgStoGetOldestMessage(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsgInfo, msg_message_id_t *pMsgId);
msg_error_t MsgStoCheckMsgCntFull(MsgDbHandler *pDbHandle, const MSG_MESSAGE_TYPE_S* pMsgType, msg_folder_id_t FolderId);
msg_error_t MsgStoCountMsgByLimitCategory(MsgDbHandler *pDbHandle, const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount, msg_folder_id_t folderId );
int MsgStoCheckMsgCntLimit(const MSG_MESSAGE_TYPE_S* pMsgType, msg_folder_id_t FolderId);

msg_error_t MsgStoAddAddress(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsg, msg_thread_id_t *pConvId);
msg_error_t MsgStoGetAddressByMsgId(MsgDbHandler *pDbHandle, msg_message_id_t msgId, int contactNameOrder, int *nAddressCnt, MSG_ADDRESS_INFO_S *pAddress);
msg_error_t MsgStoGetAddressByMsgId(MsgDbHandler *pDbHandle, msg_message_id_t msgId, int contactNameOrder, msg_struct_list_s *pAddress);
msg_error_t MsgStoGetAddressByConvId(MsgDbHandler *pDbHandle, msg_thread_id_t convId, int contactNameOrder, msg_struct_list_s *pAddrlist);
msg_error_t MsgStoAddConversation(MsgDbHandler *pDbHandle, msg_thread_id_t *pConvId);
msg_error_t MsgStoUpdateConversation(MsgDbHandler *pDbHandle, msg_thread_id_t convId);
msg_error_t MsgStoSetConversationDisplayName(MsgDbHandler *pDbHandle, int contactId);
msg_error_t MsgStoSetConversationDisplayName(MsgDbHandler *pDbHandle, msg_thread_id_t convId);
msg_error_t MsgStoClearConversationTable(MsgDbHandler *pDbHandle);
bool MsgExistAddress(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsg, msg_thread_id_t *pConvId);

int MsgStoGetUnreadCnt(MsgDbHandler *pDbHandle, MSG_MAIN_TYPE_T MsgType);
msg_error_t MsgStoAddContactInfo(MsgDbHandler *pDbHandle, MSG_CONTACT_INFO_S *pContactInfo, const char *pNumber);
msg_error_t MsgStoResetContactInfo(MsgDbHandler *pDbHandle, int ContactId);
msg_error_t MsgStoClearContactInfo(MsgDbHandler *pDbHandle, int ContactId, const char *pNumber);
msg_error_t MsgStoGetMmsRawFilePath(MsgDbHandler *pDbHandle, msg_message_id_t msgId, char *pFilePath);
bool MsgStoCheckReadReportRequested(MsgDbHandler *pDbHandle, msg_message_id_t MsgId);
bool MsgStoCheckReadReportIsSent(MsgDbHandler *pDbHandle, msg_message_id_t MsgId);
msg_error_t MsgStoUpdateNetworkStatus(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S *pMsgInfo, msg_network_status_t status);
char *MsgStoReplaceString(const char *org_str, const char *old_str, const char *new_str);
void MsgConvertNumber(const char* pSrcNum, char* pDestNum);
msg_error_t MsgStoRefreshConversationDisplayName();
#endif // MSG_UTIL_STORAGE_H

