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

#ifndef MSG_STORAGE_HANDLER_H
#define MSG_STORAGE_HANDLER_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgStorageTypes.h"
#include "MsgSettingTypes.h"
#include "MsgMmsTypes.h"
#include "MsgTransportTypes.h"
#include "MsgInternalTypes.h"


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
MSG_ERROR_T MsgStoConnectDB();
MSG_ERROR_T MsgStoDisconnectDB();

MSG_ERROR_T MsgStoInitDB(bool bSimChanged);

MSG_ERROR_T MsgCreateAddressTable();
MSG_ERROR_T MsgCreateFolderTable();
MSG_ERROR_T MsgCreateMsgTable();
MSG_ERROR_T MsgCreateSimMessageTable();
MSG_ERROR_T MsgCreateWAPMessageTable();
MSG_ERROR_T MsgCreateCBMessageTable();
MSG_ERROR_T MsgCreateSyncMLMessageTable();
MSG_ERROR_T MsgCreateScheduledMessageTable();
MSG_ERROR_T MsgCreateSmsSendOptTable();
MSG_ERROR_T MsgCreateFilterTable();
MSG_ERROR_T MsgCreateMmsMsgTable();
MSG_ERROR_T MsgCreateMmsAttributeTable();
MSG_ERROR_T MsgAddDefaultFolders();
MSG_ERROR_T MsgAddDefaultAddress();
MSG_ERROR_T MsgStoResetDatabase();

MSG_ERROR_T MsgStoAddMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S* pSendOptInfo, int addrIdx = 0);
MSG_ERROR_T MsgStoUpdateMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S* pSendOptInfo, int addrIdx = 0);
MSG_ERROR_T MsgStoUpdateReadStatus(MSG_MESSAGE_ID_T MsgId, bool bRead);
MSG_ERROR_T MsgStoUpdateThreadReadStatus(MSG_THREAD_ID_T ThreadId);
MSG_ERROR_T MsgStoUpdateProtectedStatus(MSG_MESSAGE_ID_T MsgId, bool bProtected);
MSG_ERROR_T MsgStoDeleteMessage(MSG_MESSAGE_ID_T MsgId, bool bCheckIndication);
MSG_ERROR_T MsgStoDeleteAllMessageInFolder(MSG_FOLDER_ID_T FolderId, bool bOnlyDB, MSG_MSGID_LIST_S *pMsgIdList);
MSG_ERROR_T MsgStoMoveMessageToFolder(MSG_MESSAGE_ID_T MsgId, MSG_FOLDER_ID_T DestFolderId);
MSG_ERROR_T MsgStoMoveMessageToStorage(const MSG_MESSAGE_ID_T MsgId, const MSG_STORAGE_ID_T DestStorageId);
MSG_ERROR_T MsgStoCountMessage(MSG_FOLDER_ID_T FolderId, MSG_COUNT_INFO_S *pCountInfo);
MSG_ERROR_T MsgStoCountMsgByType(const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount);
MSG_ERROR_T MsgStoGetMessage(MSG_MESSAGE_ID_T MsgId, MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S* pSendOptInfo);
MSG_ERROR_T MsgStoGetFolderViewList(MSG_FOLDER_ID_T FolderId, const MSG_SORT_RULE_S *pSortRule, MSG_LIST_S *pMsgFolderViewList);
MSG_ERROR_T MsgStoAddSyncMLMessage(MSG_MESSAGE_INFO_S *pMsgInfo, int ExtId, int PinCode);
MSG_ERROR_T MsgStoGetMsgType(MSG_MESSAGE_ID_T msgId, MSG_MESSAGE_TYPE_S* pMsgType);
MSG_ERROR_T MsgStoGetText(MSG_MESSAGE_ID_T MsgId, char *pSubject, char *pMsgText);
MSG_ERROR_T MsgStoGetQuickPanelData(MSG_QUICKPANEL_TYPE_T Type, MSG_MESSAGE_INFO_S *pMsg);
MSG_ERROR_T MsgStoGetThreadViewList(const MSG_SORT_RULE_S *pSortRule, MSG_THREAD_VIEW_LIST_S *pThreadViewList);
MSG_ERROR_T MsgStoGetConversationViewList(MSG_THREAD_ID_T ThreadId, MSG_LIST_S *pConvViewList);
MSG_ERROR_T MsgStoDeleteThreadMessageList(MSG_THREAD_ID_T ThreadId, MSG_MSGID_LIST_S *pMsgIdList);
MSG_ERROR_T MsgStoCountMsgByContact(const MSG_THREAD_LIST_INDEX_S *pAddrInfo, MSG_THREAD_COUNT_INFO_S *pThreadCountInfo);
MSG_ERROR_T MsgStoSearchMessage(const char *pSearchString, MSG_THREAD_VIEW_LIST_S *pThreadViewList);
MSG_ERROR_T MsgStoSearchMessage(const MSG_SEARCH_CONDITION_S *pSearchCon, int offset, int limit, MSG_LIST_S *pMsgList);
MSG_ERROR_T MsgStoGetMsgIdList(MSG_REFERENCE_ID_T RefId, MSG_MSGID_LIST_S *pMsgIdList);
MSG_ERROR_T MsgStoGetRejectMsgList(const char *pNumber, MSG_REJECT_MSG_LIST_S *pRejectMsgList);
MSG_ERROR_T MsgStoGetReportStatus(MSG_MESSAGE_ID_T msgId, MSG_REPORT_STATUS_INFO_S* pReportStatus);
MSG_ERROR_T MsgStoGetThreadUnreadCnt(MSG_THREAD_ID_T ThreadId, int *cnt);

// Folder
MSG_ERROR_T MsgStoAddFolder(const MSG_FOLDER_INFO_S *pFolderInfo);
MSG_ERROR_T MsgStoUpdateFolder(const MSG_FOLDER_INFO_S *pFolderInfo);
MSG_ERROR_T MsgStoDeleteFolder(MSG_FOLDER_ID_T FolderId);
MSG_ERROR_T MsgStoGetFolderList(MSG_FOLDER_LIST_S *pFolderList);

// Sim Operation related Functions
MSG_ERROR_T MsgInitSimMessage(MSG_SIM_STATUS_T SimStatus);
MSG_ERROR_T MsgStoClearSimMessageInDB();

// Internal Function
MSG_ERROR_T MsgMakeSortRule(const MSG_SORT_RULE_S *pSortRule, char *pSqlSort);
MSG_ERROR_T MsgStoGetSmsSendOpt(MSG_MESSAGE_ID_T MsgId, MSG_SENDINGOPT_INFO_S* pSendOpt);
MSG_ERROR_T MsgStoGetMmsSendOpt(MSG_MESSAGE_ID_T MsgId, MSG_SENDINGOPT_INFO_S* pSendOpt);

// Scheduled Msg
MSG_ERROR_T MsgStoAddScheduledMessage(MSG_MESSAGE_ID_T MsgID, int AlarmId, int ListenerFd);
MSG_ERROR_T MsgStoGetScheduledMessage(int AlarmId, MSG_REQUEST_INFO_S *pReqInfo, int *pListenerFd);
MSG_ERROR_T MsgStoDeleteScheduledMessage(MSG_MESSAGE_ID_T MsgId);

// SyncML Msg
MSG_ERROR_T MsgStoGetSyncMLExtId(MSG_MESSAGE_ID_T msgId, int *extId);
bool MsgStoCheckSyncMLMsgInThread(MSG_THREAD_ID_T threadId);

MSG_ERROR_T MsgStoUpdateMMSMessage(MSG_MESSAGE_INFO_S *pMsg);
MSG_ERROR_T MsgStoGetContentLocation(MSG_MESSAGE_INFO_S* pMsgInfo);
MSG_ERROR_T MsgStoSetReadReportSendStatus(MSG_MESSAGE_ID_T msgId, int readReportSendStatus);


///////////////////////////////////////////////////////////////////////////////////
MSG_ERROR_T MsgStoGetOrgAddressList(MSG_MESSAGE_INFO_S *pMsg);
MSG_ERROR_T MsgStoUpdateNetworkStatus(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_NETWORK_STATUS_T Status);
MSG_ERROR_T MsgStoGetSubject(MSG_MESSAGE_ID_T MsgId, char* pSubject);
MSG_ERROR_T MsgStoGetRecipientList(MSG_MESSAGE_ID_T msgId, MSG_RECIPIENTS_LIST_S *pRecipientList);
MSG_ERROR_T MsgStoGetReadStatus(MSG_MESSAGE_ID_T MsgId, bool *pReadStatus);
MSG_ERROR_T MsgStoGetAddrInfo(MSG_MESSAGE_ID_T MsgId, MSG_ADDRESS_INFO_S *pAddrInfo);

///////////////////////////////////////////////////////////////////////////////////

MSG_ERROR_T MsgStoResetNetworkStatus();
MSG_ERROR_T MsgStoCleanAbnormalMmsData();
MSG_ERROR_T MsgStoCheckReadReportStatus(MSG_MESSAGE_ID_T msgId);

#endif // MSG_STORAGE_HANDLER_H

