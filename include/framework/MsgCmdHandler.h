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

int MsgSetConfigHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetConfigHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgSubmitReqHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgCancelReqHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgRegSentStatusCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegIncomingMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegIncomingMMSConfMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegIncomingSyncMLMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegIncomingLBSMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegSyncMLMsgOperationCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgRegStorageChangeCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgStorageChangeHandler(const MSG_CMD_S *pCmd, char **ppEvent);

int MsgSentStatusHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgIncomingMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgIncomingSyncMLMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgIncomingLBSMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgSyncMLMsgOperationHandler(const MSG_CMD_S *pCmd, char **ppEvent);

MSG_ERROR_T MsgPrepareScheduledMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOpt, int ListenerFd);
int MsgCreateScheduledAlarm(MSG_MESSAGE_INFO_S *pMsgInfo);
int MsgScheduledAlarmCB(int AlarmId, void *pUserParam);
int MsgIncomingMMSConfMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgAddFilterHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgUpdateFilterHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgDeleteFilterHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetFilterListHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgSetFilterOperationHandler(const MSG_CMD_S *pCmd, char **ppEvent);
int MsgGetFilterOperationHandler(const MSG_CMD_S *pCmd, char **ppEvent);

#endif // MSG_CMD_HANDLER_H
