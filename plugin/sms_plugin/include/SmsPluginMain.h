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

#ifndef SMS_PLUGIN_MAIN_H
#define SMS_PLUGIN_MAIN_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "SmsPluginTypes.h"
#include "MsgPluginInterface.h"


#ifdef __cplusplus
extern "C"
{
#endif


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
MSG_ERROR_T SmsPlgInitialize();

MSG_ERROR_T SmsPlgFinalize();

MSG_ERROR_T SmsPlgRegisterListener(MSG_PLUGIN_LISTENER_S *pListener);

MSG_ERROR_T SmsPlgCheckSimStatus(MSG_SIM_STATUS_T *pStatus);

MSG_ERROR_T SmsPlgCheckDeviceStatus();

MSG_ERROR_T SmsPlgSubmitRequest(MSG_REQUEST_INFO_S *pReqInfo, bool bReqCb);

MSG_ERROR_T SmsPlgInitSimMessage();

MSG_ERROR_T SmsPlgSaveSimMessage(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList);

MSG_ERROR_T SmsPlgDeleteSimMessage(MSG_SIM_ID_T SimMsgId);

MSG_ERROR_T SmsPlgGetSimMessage(MSG_SIM_ID_T SimMsgId);

MSG_ERROR_T SmsPlgGetSimMessageCount();

MSG_ERROR_T SmsPlgSetReadStatus(MSG_SIM_ID_T SimMsgId);

MSG_ERROR_T SmsPlgSetMemoryStatus(MSG_ERROR_T Error);

MSG_ERROR_T SmsPlgInitConfigData(MSG_SIM_STATUS_T SimStatus);

MSG_ERROR_T SmsPlgSetConfigData(const MSG_SETTING_S *pSetting);

MSG_ERROR_T SmsPlgGetConfigData(MSG_SETTING_S *pSetting);

#ifdef __cplusplus
}
#endif

#endif //SMS_PLUGIN_MAIN_H

