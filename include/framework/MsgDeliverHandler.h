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
MSG_ERROR_T MsgHandleMmsConfIncomingMsg(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_ID_T reqID);
MSG_ERROR_T MsgHandleIncomingMsg(MSG_MESSAGE_INFO_S *pMsgInfo, bool *pSendNoti);
MSG_ERROR_T MsgHandleSMS(MSG_MESSAGE_INFO_S *pMsgInfo, bool *pSendNoti);
MSG_ERROR_T MsgHandleMMS(MSG_MESSAGE_INFO_S *pMsgInfo, bool *pSendNoti);

/*==================================================================================================
					DEFINES
==================================================================================================*/
#define MSG_SVC_PKG_NAME_BROWSER	"org.tizen.browser"

#endif // MSG_DELIVER_HANDLER_H

