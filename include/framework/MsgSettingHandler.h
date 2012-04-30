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

