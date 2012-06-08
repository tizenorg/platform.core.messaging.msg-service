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

#ifndef MMS_PLUGIN_MAIN_H
#define MMS_PLUGIN_MAIN_H

/*==================================================================================================
							INCLUDE FILES
==================================================================================================*/
#include "MsgTypes.h"
#include "MsgPluginInterface.h"

#ifdef __cplusplus

extern "C"
{
#endif

/*==================================================================================================
							FUNCTION PROTOTYPES
==================================================================================================*/
MSG_ERROR_T MmsInitialize();

MSG_ERROR_T MmsFinalize();

MSG_ERROR_T MmsRegisterListener(MSG_PLUGIN_LISTENER_S *pListener);

MSG_ERROR_T MmsSubmitRequest(MSG_REQUEST_INFO_S *pReqInfo, bool bReqCb);

MSG_ERROR_T MmsAddMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData);

MSG_ERROR_T MmsProcessReceivedInd(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S *pRequest, bool *bReject);

MSG_ERROR_T MmsUpdateMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData);

MSG_ERROR_T MmsGetMmsMessage(MSG_MESSAGE_INFO_S *pMsg,  MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMmsMsg, char **pDestMsg);

MSG_ERROR_T MmsUpdateRejectStatus(MSG_MESSAGE_INFO_S *pMsgInfo);

MSG_ERROR_T MmsComposeReadReport(MSG_MESSAGE_INFO_S *pMsgInfo);

MSG_ERROR_T MmsRestoreMsg(MSG_MESSAGE_INFO_S *pMsgInfo, char *pRcvBody, int rcvdBodyLen, char *filePath);

#ifdef __cplusplus
}
#endif

#endif //MMS_PLUGIN_MAIN_H

