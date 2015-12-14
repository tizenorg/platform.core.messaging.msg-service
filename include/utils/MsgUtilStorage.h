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

#ifndef MSG_UTIL_STORAGE_H
#define MSG_UTIL_STORAGE_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgInternalTypes.h"
#include "MsgStorageTypes.h"
#include "MsgSqliteWrapper.h"
#include "MsgSettingTypes.h"

/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/
/* Common Function */
unsigned int MsgStoAddMessageTable(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsgInfo);
msg_error_t MsgStoSetReadStatus(MsgDbHandler *pDbHandle, msg_message_id_t MsgId, bool bRead);
msg_error_t MsgStoGetOldestMessage(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsgInfo, msg_message_id_t *pMsgId);
msg_error_t MsgStoCheckMsgCntFull(MsgDbHandler *pDbHandle, const MSG_MESSAGE_TYPE_S* pMsgType, msg_folder_id_t FolderId);
msg_error_t MsgStoCountMsgByLimitCategory(MsgDbHandler *pDbHandle, const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount, msg_folder_id_t folderId);
msg_error_t MsgStocheckMemoryStatus();
int MsgStoCheckMsgCntLimit(const MSG_MESSAGE_TYPE_S* pMsgType, msg_folder_id_t FolderId);

msg_error_t MsgStoAddAddress(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsg, msg_thread_id_t *pConvId);

/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
msg_error_t MsgStoGetAddressByMsgId(MsgDbHandler *pDbHandle, msg_message_id_t msgId, int contactNameOrder, int *nAddressCnt, MSG_ADDRESS_INFO_S **pAddress);
msg_error_t MsgStoGetAddressByMsgId(MsgDbHandler *pDbHandle, msg_message_id_t msgId, int contactNameOrder, msg_struct_list_s *pAddress);
msg_error_t MsgStoGetAddressByConvId(MsgDbHandler *pDbHandle, msg_thread_id_t convId, int contactNameOrder, msg_struct_list_s *pAddrlist);
#else /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
/* contactNameOrder is never used */
msg_error_t MsgStoGetAddressByMsgId(MsgDbHandler *pDbHandle, msg_message_id_t msgId, int *nAddressCnt, MSG_ADDRESS_INFO_S **pAddress);
msg_error_t MsgStoGetAddressByMsgId(MsgDbHandler *pDbHandle, msg_message_id_t msgId, msg_struct_list_s *pAddress);
msg_error_t MsgStoGetAddressByConvId(MsgDbHandler *pDbHandle, msg_thread_id_t convId, msg_struct_list_s *pAddrlist);
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */

void MsgStoUpdateAddress(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsg, msg_thread_id_t convId);
msg_error_t MsgStoAddConversation(MsgDbHandler *pDbHandle, msg_thread_id_t *pConvId);
msg_error_t MsgStoUpdateConversation(MsgDbHandler *pDbHandle, msg_thread_id_t convId);
msg_error_t MsgStoSetConversationDisplayName(MsgDbHandler *pDbHandle, int contactId);
msg_error_t MsgStoSetConversationDisplayName(MsgDbHandler *pDbHandle, msg_thread_id_t convId);
msg_error_t MsgStoClearConversationTable(MsgDbHandler *pDbHandle);

msg_thread_id_t MsgGetThreadId(MsgDbHandler *pDbHandle, msg_message_id_t msgId);

#ifdef FEATURE_SMS_CDMA
msg_error_t MsgStoAddCBChannelInfo(MsgDbHandler *pDbHandle, MSG_CB_CHANNEL_S *pCBChannel);
msg_error_t MsgStoGetCBChannelInfo(MsgDbHandler *pDbHandle, MSG_CB_CHANNEL_S *pCBChannel);
#else
msg_error_t MsgStoAddCBChannelInfo(MsgDbHandler *pDbHandle, MSG_CB_CHANNEL_S *pCBChannel, msg_sim_slot_id_t simIndex);
msg_error_t MsgStoGetCBChannelInfo(MsgDbHandler *pDbHandle, MSG_CB_CHANNEL_S *pCBChannel, msg_sim_slot_id_t simIndex);
#endif

bool MsgExistAddress(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsg, msg_thread_id_t *pConvId);
bool MsgExistAddress(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S *pMsg,  msg_thread_id_t convId, int index);
bool MsgExistConversation(MsgDbHandler *pDbHandle, msg_thread_id_t convId);
bool MsgExistMessage(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S *pMsg);

int MsgStoGetUnreadCnt(MsgDbHandler *pDbHandle, MSG_MAIN_TYPE_T MsgType);
msg_error_t MsgStoGetMmsRawFilePath(MsgDbHandler *pDbHandle, msg_message_id_t msgId, char *pFilePath);
bool MsgStoCheckReadReportRequested(MsgDbHandler *pDbHandle, msg_message_id_t MsgId);
bool MsgStoCheckReadReportIsSent(MsgDbHandler *pDbHandle, msg_message_id_t MsgId);
msg_error_t MsgStoUpdateNetworkStatus(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S *pMsgInfo, msg_network_status_t status);

/* Lists */
msg_error_t MsgStoGetThreadViewList(const MSG_SORT_RULE_S *pSortRule, msg_struct_list_s *pThreadViewList);
msg_error_t MsgStoGetConversationViewItem(msg_message_id_t msgId, MSG_CONVERSATION_VIEW_S *pConv);
msg_error_t MsgStoGetConversationViewList(msg_thread_id_t ThreadId, msg_struct_list_s *pConvViewList);
msg_error_t MsgStoSearchMessage(const char *pSearchString, msg_struct_list_s *pThreadViewList, int contactCount);

msg_error_t MsgStoGetRejectMsgList(const char *pNumber, msg_struct_list_s *pRejectMsgList);
msg_error_t MsgStoGetAddressList(const msg_thread_id_t threadId, msg_struct_list_s *pAddrList);
msg_error_t MsgStoGetMessageList(const MSG_LIST_CONDITION_S *pListCond, msg_struct_list_s *pMsgList, int contactCount);
msg_error_t MsgStoGetMediaList(const msg_thread_id_t threadId, msg_list_handle_t *pMediaList);

#ifdef FEATURE_SMS_CDMA
msg_error_t MsgStoClearUniquenessTable();
#endif

#endif /* MSG_UTIL_STORAGE_H */

