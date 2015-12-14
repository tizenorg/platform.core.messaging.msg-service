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

#ifndef MSG_DELIVER_HANDLER_H
#define MSG_DELIVER_HANDLER_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgPluginInterface.h"
#include "MsgTypes.h"


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
msg_error_t MsgHandleMmsConfIncomingMsg(MSG_MESSAGE_INFO_S *pMsgInfo, msg_request_id_t reqID);
msg_error_t MsgHandleIncomingMsg(MSG_MESSAGE_INFO_S *pMsgInfo, bool *pSendNoti);
msg_error_t MsgHandleSMS(MSG_MESSAGE_INFO_S *pMsgInfo, bool *pSendNoti, bool *bOnlyNoti);
msg_error_t MsgHandleMMS(MSG_MESSAGE_INFO_S *pMsgInfo, bool *pSendNoti);

void MsgPlayTTSMode(MSG_SUB_TYPE_T msgSubType, msg_message_id_t msgId, bool isFavorites);
void MsgLaunchClass0(msg_message_id_t msgId);

/*==================================================================================================
					DEFINES
==================================================================================================*/
#define MSG_SVC_PKG_NAME_BROWSER	"org.tizen.browser"

#endif /* MSG_DELIVER_HANDLER_H */

