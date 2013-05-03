/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
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
msg_error_t SmsPlgInitialize();

msg_error_t SmsPlgFinalize();

msg_error_t SmsPlgRegisterListener(MSG_PLUGIN_LISTENER_S *pListener);

msg_error_t SmsPlgCheckSimStatus(MSG_SIM_STATUS_T *pStatus);

msg_error_t SmsPlgCheckDeviceStatus();

msg_error_t SmsPlgSubmitRequest(MSG_REQUEST_INFO_S *pReqInfo);

msg_error_t SmsPlgInitSimMessage();

msg_error_t SmsPlgSaveSimMessage(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList);

msg_error_t SmsPlgDeleteSimMessage(msg_sim_id_t SimMsgId);

msg_error_t SmsPlgGetSimMessage(msg_sim_id_t SimMsgId);

msg_error_t SmsPlgGetSimMessageCount();

msg_error_t SmsPlgSetReadStatus(msg_sim_id_t SimMsgId);

msg_error_t SmsPlgSetMemoryStatus(msg_error_t Error);

msg_error_t SmsPlgInitConfigData(MSG_SIM_STATUS_T SimStatus);

msg_error_t SmsPlgSetConfigData(const MSG_SETTING_S *pSetting);

msg_error_t SmsPlgGetConfigData(MSG_SETTING_S *pSetting);

#ifdef __cplusplus
}
#endif

#endif //SMS_PLUGIN_MAIN_H

