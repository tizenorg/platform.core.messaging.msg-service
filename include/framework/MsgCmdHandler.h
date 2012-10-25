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

#ifndef MSG_CMD_HANDLER_H
#define MSG_CMD_HANDLER_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgCmdTypes.h"
#include "MsgInternalTypes.h"


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
int MsgAddMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgAddSyncMLMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgUpdateMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgUpdateReadStatusHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgUpdateThreadReadStatusHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgUpdateProtectedStatusHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgDeleteMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgDeleteAllMessageInFolderHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgDeleteMessageByListHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgMoveMessageToFolderHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgMoveMessageToStorageHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgCountMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgCountMsgByTypeHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetFolderViewListHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgAddFolderHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgUpdateFolderHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgDeleteFolderHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetFolderListHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgInitSimBySatHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetMsgTypeHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgGetThreadViewListHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetConversationViewListHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgDeleteThreadMessageListHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgCountMsgByContactHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetQuickPanelDataHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgResetDatabaseHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetMemSizeHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetReportStatusHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetThreadIdByAddressHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetThreadInfoHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgBackupMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRestoreMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgSetConfigHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetConfigHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgSubmitReqHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgCancelReqHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgRegSentStatusCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegIncomingMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegIncomingMMSConfMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegIncomingPushMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegIncomingCBMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegIncomingSyncMLMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegIncomingLBSMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegSyncMLMsgOperationCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegStorageChangeCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgStorageChangeHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgSentStatusHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgIncomingMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgIncomingPushMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgIncomingCBMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgIncomingSyncMLMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgIncomingLBSMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgSyncMLMsgOperationHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgIncomingMMSConfMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgAddFilterHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgUpdateFilterHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgDeleteFilterHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetFilterListHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgSetFilterOperationHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetFilterOperationHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgSetFilterActivationHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgAddPushEventHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgDeletePushEventHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgUpdatePushEventHandler(const MSG_CMD_S *pCmd, char **ppEvent);


#endif // MSG_CMD_HANDLER_H
