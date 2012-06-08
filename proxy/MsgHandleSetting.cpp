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
#include "MsgUtilFunction.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgHandle.h"


/*==================================================================================================
                                     IMPLEMENTATION OF MsgHandle - Setting Member Functions
==================================================================================================*/
MSG_ERROR_T MsgHandle::setConfig(const MSG_SETTING_S *pSetting)
{
	// Allocate Memory to Command Data
	int cmdSize = getSettingCmdSize(pSetting->type);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SET_CONFIG;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy(pCmd->cmdData, pSetting, cmdSize-sizeof(MSG_CMD_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_SET_CONFIG)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


MSG_ERROR_T MsgHandle::getConfig(MSG_SETTING_S *pSetting)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_OPTION_TYPE_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_CONFIG;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &(pSetting->type), sizeof(MSG_OPTION_TYPE_T));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_CONFIG)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if (pEvent->result == MSG_SUCCESS)
		MsgDecodeSetting(pEvent->data, pSetting);

	return pEvent->result;
}

