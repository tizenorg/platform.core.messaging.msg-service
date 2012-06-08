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
