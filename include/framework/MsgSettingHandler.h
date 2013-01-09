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

#ifndef MSG_SETTING_HANDLER_H
#define MSG_SETTING_HANDLER_H


/*==================================================================================================
					INCLUDE FILES
==================================================================================================*/
#include "MsgSettingTypes.h"


/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/
msg_error_t MsgInitSimConfig(MSG_SIM_STATUS_T SimStatus);

msg_error_t MsgSetConfigData(const MSG_SETTING_S *pSetting);
msg_error_t MsgGetConfigData(MSG_SETTING_S *pSetting);

msg_error_t MsgSetGeneralOpt(const MSG_SETTING_S *pSetting);
msg_error_t MsgSetSMSSendOpt(const MSG_SETTING_S *pSetting);
msg_error_t MsgSetSMSCList(const MSG_SETTING_S *pSetting, bool bSetSim);
msg_error_t MsgSetMMSSendOpt(const MSG_SETTING_S *pSetting);
msg_error_t MsgSetMMSRecvOpt(const MSG_SETTING_S *pSetting);
msg_error_t MsgSetMMSStyleOpt(const MSG_SETTING_S *pSetting);
msg_error_t MsgSetPushMsgOpt(const MSG_SETTING_S *pSetting);
msg_error_t MsgSetCBMsgOpt(const MSG_SETTING_S *pSetting, bool bSetSim);

msg_error_t MsgSetVoiceMailOpt(const MSG_SETTING_S *pSetting, bool bSetSim);
msg_error_t MsgSetMsgSizeOpt(const MSG_SETTING_S *pSetting);

void MsgGetGeneralOpt(MSG_SETTING_S *pSetting);
void MsgGetSMSSendOpt(MSG_SETTING_S *pSetting);
void MsgGetSMSCList(MSG_SETTING_S *pSetting);
void MsgGetMMSSendOpt(MSG_SETTING_S *pSetting);
void MsgGetMMSRecvOpt(MSG_SETTING_S *pSetting);
void MsgGetMMSStyleOpt(MSG_SETTING_S *pSetting);
void MsgGetPushMsgOpt(MSG_SETTING_S *pSetting);
void MsgGetCBMsgOpt(MSG_SETTING_S *pSetting);
void MsgGetVoiceMailOpt(MSG_SETTING_S *pSetting);
void MsgGetMsgSizeOpt(MSG_SETTING_S *pSetting);

msg_error_t MsgSetConfigInSim(const MSG_SETTING_S *pSetting);

//void MsgSetDefaultConfig();

#endif

