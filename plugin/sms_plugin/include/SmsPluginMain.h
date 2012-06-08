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

