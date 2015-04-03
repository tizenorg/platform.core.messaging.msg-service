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

#ifndef MSG_STORAGE_HANDLER_H
#define MSG_STORAGE_HANDLER_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgStorageTypes.h"
#include "MsgSettingTypes.h"
#include "MsgFilterTypes.h"
#include "MsgMmsTypes.h"
#include "MsgTransportTypes.h"
#include "MsgInternalTypes.h"

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
msg_error_t MsgStoConnectDB();
msg_error_t MsgStoDisconnectDB();

msg_error_t MsgStoInitDB(bool bSimChanged);

msg_error_t MsgCreateConversationTable();
msg_error_t MsgCreateAddressTable();
msg_error_t MsgCreateFolderTable();
msg_error_t MsgCreateMsgTable();
msg_error_t MsgCreateSimMessageTable();
msg_error_t MsgCreateWAPMessageTable();
msg_error_t MsgCreateCBMessageTable();
msg_error_t MsgCreateSyncMLMessageTable();
msg_error_t MsgCreateSmsSendOptTable();
msg_error_t MsgCreateFilterTable();
msg_error_t MsgCreateMmsTable();
msg_error_t MsgAddDefaultFolders();
msg_error_t MsgAddDefaultAddress();
msg_error_t MsgStoResetDatabase();

msg_error_t MsgStoBackupMessage(msg_message_backup_type_t type, const char *filepath);
msg_error_t MsgStoRestoreMessage(const char *filepath, msg_id_list_s**result_id_list);

msg_error_t MsgStoAddMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S* pSendOptInfo);
msg_error_t MsgStoRestoreMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo);
msg_error_t MsgStoUpdateMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S* pSendOptInfo);
msg_error_t MsgStoUpdateReadStatus(msg_message_id_t MsgId, bool bRead);
msg_error_t MsgStoUpdateThreadReadStatus(msg_thread_id_t ThreadId, msg_id_list_s *pMsgIdList);
msg_error_t MsgStoUpdateProtectedStatus(msg_message_id_t MsgId, bool bProtected);
msg_error_t MsgStoDeleteMessage(msg_message_id_t MsgId, bool bCheckIndication);
msg_error_t MsgStoDeleteAllMessageInFolder(msg_folder_id_t FolderId, bool bOnlyDB, msg_id_list_s *pMsgIdList);
msg_error_t MsgStoDeleteMessageByList(msg_id_list_s *pMsgIdList);
msg_error_t MsgStoMoveMessageToFolder(msg_message_id_t MsgId, msg_folder_id_t DestFolderId);
msg_error_t MsgStoMoveMessageToStorage(const msg_message_id_t MsgId, const msg_storage_id_t DestStorageId);
msg_error_t MsgStoCountMessage(msg_folder_id_t FolderId, MSG_COUNT_INFO_S *pCountInfo);
msg_error_t MsgStoCountMsgByType(const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount);
msg_error_t MsgStoGetMessage(msg_message_id_t MsgId, MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S* pSendOptInfo);
msg_error_t MsgStoAddSyncMLMessage(MSG_MESSAGE_INFO_S *pMsgInfo, int ExtId, int PinCode);
msg_error_t MsgStoGetMsgType(msg_message_id_t msgId, MSG_MESSAGE_TYPE_S* pMsgType);
msg_error_t MsgStoGetText(msg_message_id_t MsgId, char *pSubject, char *pMsgText);
msg_error_t MsgStoGetQuickPanelData(msg_quickpanel_type_t Type, MSG_MESSAGE_INFO_S *pMsg);
msg_error_t MsgStoDeleteThreadMessageList(msg_thread_id_t ThreadId, bool bIncludeProtect, msg_id_list_s *pMsgIdList);
msg_error_t MsgStoCountMsgByContact(const MSG_THREAD_LIST_INDEX_S *pAddrInfo, MSG_THREAD_COUNT_INFO_S *pThreadCountInfo);
msg_error_t MsgStoGetSmsReportStatus(msg_message_id_t msgId, int *count, MSG_REPORT_STATUS_INFO_S **pReportStatus);
msg_error_t MsgStoGetMmsReportStatus(msg_message_id_t msgId, int *count, MSG_REPORT_STATUS_INFO_S **pReportStatus);
msg_error_t MsgStoGetThreadIdByAddress(const MSG_MESSAGE_INFO_S *pMsg, msg_thread_id_t *pThreadId);
msg_error_t MsgStoGetThreadUnreadCnt(msg_thread_id_t ThreadId, int *cnt);

msg_error_t MsgStoGetThreadInfo(msg_thread_id_t threadId, MSG_THREAD_VIEW_S *pThreadInfo);

// Folder
msg_error_t MsgStoAddFolder(const MSG_FOLDER_INFO_S *pFolderInfo);
msg_error_t MsgStoUpdateFolder(const MSG_FOLDER_INFO_S *pFolderInfo);
msg_error_t MsgStoDeleteFolder(msg_folder_id_t FolderId);
msg_error_t MsgStoGetFolderList(msg_struct_list_s *pFolderList);

// Filter
msg_error_t MsgStoAddFilter(const MSG_FILTER_S *pFilter);
msg_error_t MsgStoUpdateFilter(const MSG_FILTER_S *pFilter);
msg_error_t MsgStoDeleteFilter(msg_filter_id_t FilterId);
msg_error_t MsgStoGetFilterList(msg_struct_list_s *pFilterList);
msg_error_t MsgStoSetFilterActivation(msg_filter_id_t filterId, bool bActive);

// Sim Operation related Functions
msg_error_t MsgStoClearSimMessageInDB();

// Internal Function
msg_error_t MsgStoGetSmsSendOpt(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S* pSendOpt);
msg_error_t MsgStoGetMmsSendOpt(msg_message_id_t MsgId, MSG_SENDINGOPT_INFO_S* pSendOpt);

// SyncML Msg
msg_error_t MsgStoGetSyncMLExtId(msg_message_id_t msgId, int *extId);
bool MsgStoCheckSyncMLMsgInThread(msg_thread_id_t threadId);

msg_error_t MsgStoUpdateMMSMessage(MSG_MESSAGE_INFO_S *pMsg);
msg_error_t MsgStoGetContentLocation(MSG_MESSAGE_INFO_S* pMsgInfo);
msg_error_t MsgStoSetReadReportSendStatus(msg_message_id_t msgId, int readReportSendStatus);
msg_error_t MsgStoGetReadReportSendStatus(msg_message_id_t msgId, int *pReadReportSendStatus);

///////////////////////////////////////////////////////////////////////////////////
// For MMS - will be removed
msg_error_t MsgStoGetOrgAddressList(MSG_MESSAGE_INFO_S *pMsg);
msg_error_t MsgStoGetSubject(msg_message_id_t MsgId, char* pSubject);
msg_error_t MsgStoGetRecipientList(msg_message_id_t msgId, MSG_RECIPIENTS_LIST_S *pRecipientList);
msg_error_t MsgStoGetReadStatus(msg_message_id_t MsgId, bool *pReadStatus);
msg_error_t MsgStoGetAddrInfo(msg_message_id_t MsgId, MSG_ADDRESS_INFO_S *pAddrInfo);

///////////////////////////////////////////////////////////////////////////////////

msg_error_t MsgStoResetNetworkStatus();
#if 0
msg_error_t MsgStoResetCBMessage();
#endif
msg_error_t MsgStoCleanAbnormalMmsData();
msg_error_t MsgStoCheckReadReportStatus(msg_message_id_t msgId);
msg_error_t MsgStoAutoDeleteConversation(msg_thread_id_t threadId, msg_id_list_s *msgIdList);

msg_error_t MsgStoAddPushEvent(MSG_PUSH_EVENT_INFO_S* pPushEvent);
msg_error_t MsgStoDeletePushEvent(MSG_PUSH_EVENT_INFO_S* pPushEvent);
msg_error_t MsgStoUpdatePushEvent(MSG_PUSH_EVENT_INFO_S* pSrc, MSG_PUSH_EVENT_INFO_S* pDst);

msg_message_id_t MsgStoAddSimMessage(MSG_MESSAGE_INFO_S *pMsg, int *simIdList, int listSize);
msg_error_t MsgStoGetFailedMessage(int **failed_msg_list, int *count);

msg_error_t MsgStoGetReplaceMsgId(MSG_MESSAGE_INFO_S *pMsgInfo);
msg_error_t MsgStoAddWAPMsg(MSG_MESSAGE_INFO_S *pMsgInfo);
msg_error_t MsgStoAddCOWAPMsg(MSG_MESSAGE_INFO_S *pMsgInfo);
msg_error_t MsgStoAddCBMsg(MSG_MESSAGE_INFO_S *pMsgInfo);
msg_error_t MsgStoCheckPushMsgValidation(MSG_PUSH_MESSAGE_S *pPushMsg, bool *pbProceed);
msg_error_t MsgStoUpdateAllAddress();
#ifdef FEATURE_SMS_CDMA
msg_error_t MsgCheckUniqueness(bool bInsert, msg_message_id_t msgId, MSG_UNIQUE_INDEX_S *p_msg);
#endif
msg_error_t MsgStoUpdateIMSI(int sim_idx);

#endif // MSG_STORAGE_HANDLER_H

