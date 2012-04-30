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

