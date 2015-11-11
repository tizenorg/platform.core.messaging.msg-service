/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved
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

#ifndef SMS_CDMA_PLUGIN_MAIN_H
#define SMS_CDMA_PLUGIN_MAIN_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "SmsCdmaPluginTypes.h"
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

msg_error_t SmsPlgSubmitRequest(MSG_REQUEST_INFO_S *pReqInfo);

msg_error_t SmsPlgSaveSimMessage(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList);

msg_error_t SmsPlgDeleteSimMessage(msg_sim_id_t SimMsgId);

msg_error_t SmsPlgGetSimMessage(msg_sim_id_t SimMsgId);

msg_error_t SmsPlgSetReadStatus(msg_sim_id_t SimMsgId);

msg_error_t SmsPlgSetMemoryStatus(msg_error_t Error);

msg_error_t SmsPlgSetConfigData(const MSG_SETTING_S *pSetting);

msg_error_t SmsPlgGetConfigData(MSG_SETTING_S *pSetting);

msg_error_t SmsPlgAddMessage(MSG_MESSAGE_INFO_S *pMsgInfo,  MSG_SENDINGOPT_INFO_S* pSendOptInfo, char* pFileData);

#ifdef __cplusplus
}
#endif

#endif /* SMS_CDMA_PLUGIN_MAIN_H */
