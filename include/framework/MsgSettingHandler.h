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
MSG_ERROR_T MsgInitSimConfig(MSG_SIM_STATUS_T SimStatus);

MSG_ERROR_T MsgSetConfigData(const MSG_SETTING_S *pSetting);
MSG_ERROR_T MsgGetConfigData(MSG_SETTING_S *pSetting);

MSG_ERROR_T MsgSetGeneralOpt(const MSG_SETTING_S *pSetting);
MSG_ERROR_T MsgSetSMSSendOpt(const MSG_SETTING_S *pSetting);
MSG_ERROR_T MsgSetSMSCList(const MSG_SETTING_S *pSetting, bool bSetSim);
MSG_ERROR_T MsgSetMMSSendOpt(const MSG_SETTING_S *pSetting);
MSG_ERROR_T MsgSetMMSRecvOpt(const MSG_SETTING_S *pSetting);
MSG_ERROR_T MsgSetMMSStyleOpt(const MSG_SETTING_S *pSetting);
MSG_ERROR_T MsgSetPushMsgOpt(const MSG_SETTING_S *pSetting);
MSG_ERROR_T MsgSetCBMsgOpt(const MSG_SETTING_S *pSetting, bool bSetSim);
MSG_ERROR_T MsgSetVoiceMailOpt(const MSG_SETTING_S *pSetting);
MSG_ERROR_T MsgSetMsgSizeOpt(const MSG_SETTING_S *pSetting);

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

MSG_ERROR_T MsgSetConfigInSim(const MSG_SETTING_S *pSetting);


#endif

