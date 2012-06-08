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

#ifndef MSG_SUBMIT_HANDLER_H
#define MSG_SUBMIT_HANDLER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgInternalTypes.h"


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
MSG_ERROR_T MsgSubmitReq(MSG_REQUEST_INFO_S* pReq, bool bScheduled);
MSG_ERROR_T MsgSubmitReqSMS(MSG_REQUEST_INFO_S *pReqInfo);
MSG_ERROR_T MsgSubmitReqMMS(MSG_REQUEST_INFO_S *pReqInfo, bool bScheduled);

MSG_ERROR_T MsgCancelReq(MSG_REQUEST_ID_T reqId);
MSG_ERROR_T MsgUpdateSentMsg(MSG_MESSAGE_ID_T MsgId, MSG_NETWORK_STATUS_T Status);

void MsgCopyReqInfo(MSG_REQUEST_INFO_S *pSrc, int addrIdx, MSG_REQUEST_INFO_S *pDest);

#endif // MSG_SUBMIT_HANDLER_H

