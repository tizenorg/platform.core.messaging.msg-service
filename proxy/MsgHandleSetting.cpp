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

