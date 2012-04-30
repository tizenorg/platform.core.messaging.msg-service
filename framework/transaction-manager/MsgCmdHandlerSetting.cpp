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

#include "MsgDebug.h"
#include "MsgCmdHandler.h"
#include "MsgSettingHandler.h"
#include "MsgUtilFunction.h"
#include "MsgCppTypes.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
int MsgSetConfigHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	// Get Setting Structure
	MSG_SETTING_S* pSetting = (MSG_SETTING_S*)pCmd->cmdData;

	// Set Config Data
	err = MsgSetConfigData(pSetting);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgSetConfigData()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgSetConfigData()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_SET_CONFIG, err, (void**)ppEvent);

	return eventSize;
}


int MsgGetConfigHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	// Get Option Type
	MSG_OPTION_TYPE_T* type = (MSG_OPTION_TYPE_T*)pCmd->cmdData;

	// Get Config Data
	MSG_SETTING_S setting;
	setting.type = *type;

	err = MsgGetConfigData(&setting);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgGetConfigData()");

		// Encoding Config Data
		dataSize = MsgEncodeSetting(&setting, &encodedData);
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgGetConfigData()");
	}

	MSG_DEBUG("dataSize [%d]", dataSize);

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_GET_CONFIG, err, (void**)ppEvent);

	return eventSize;
}
