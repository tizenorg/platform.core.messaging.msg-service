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

