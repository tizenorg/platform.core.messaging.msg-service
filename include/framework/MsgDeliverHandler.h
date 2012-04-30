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

