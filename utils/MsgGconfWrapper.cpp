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

#include <stdio.h>

#include <pmapi.h>

#include "MsgDebug.h"
#include "MsgUtilStorage.h"
#include "MsgGconfWrapper.h"


/*==================================================================================================
                                     DEFINES
==================================================================================================*/
#define MSG_UNREAD_CNT		"db/badge/org.tizen.message"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
MSG_ERROR_T MsgSettingSetString(const char *pKey, const char *pSetValue)
{
	if (pKey == NULL || pSetValue == NULL)
	{
		MSG_DEBUG("IN Parameter is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if (vconf_set_str(pKey, pSetValue) != 0)
		return MSG_ERR_SET_SETTING;

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgSettingSetInt(const char *pKey, int nSetValue)
{
	if (pKey == NULL)
	{
		MSG_DEBUG("IN Parameter is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if (vconf_set_int(pKey, nSetValue) != 0)
		return MSG_ERR_SET_SETTING;

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgSettingSetBool(const char *pKey, bool bSetValue)
{
	if (pKey == NULL)
	{
		MSG_DEBUG("IN Parameter is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if (vconf_set_bool(pKey, bSetValue) != 0)
		return MSG_ERR_SET_SETTING;

	return MSG_SUCCESS;
}


char* MsgSettingGetString(const char *pKey)
{
	if (pKey == NULL)
	{
		MSG_DEBUG("IN Parameter is NULL");
		return NULL;
	}

	return vconf_get_str(pKey);
}


int MsgSettingGetInt(const char *pKey)
{
	if (pKey == NULL)
	{
		MSG_DEBUG("IN Parameter is NULL");
		return -1;
	}

	int retVal = 0;

	if (vconf_get_int(pKey, &retVal) < 0)
		return -1;

	return retVal;
}


int MsgSettingGetBool(const char *pKey, bool *pVal)
{
	if (pKey == NULL)
	{
		MSG_DEBUG("IN Parameter is NULL");
		return -1;
	}

	int retVal = 0, param = 0;

	if (vconf_get_bool(pKey, &param) < 0)
		return -1;

 	*pVal = (bool)param;

	return retVal;
}


MSG_ERROR_T MsgSettingHandleNewMsg(int SmsCnt, int MmsCnt)
{
	MSG_BEGIN();

	MSG_DEBUG("smsCnt = %d, mmsCnt = %d ##", SmsCnt, MmsCnt);

	// Set Msg Count into VConf
	if (MsgSettingSetIndicator(SmsCnt, MmsCnt) != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgSettingSetIndicator() FAILED");
		return MSG_ERR_SET_SETTING;
	}

	if (SmsCnt == 0 && MmsCnt == 0)
	{
		MSG_DEBUG("No New Message.");
	}
	else
	{
		MSG_DEBUG("New Message.");
		pm_change_state(LCD_NORMAL);
	}

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgSettingSetIndicator(int SmsCnt, int MmsCnt)
{

	if (MsgSettingSetInt(VCONFKEY_MESSAGE_RECV_SMS_STATE, SmsCnt) != 0)
		return MSG_ERR_SET_SETTING;
	if (MsgSettingSetInt(VCONFKEY_MESSAGE_RECV_MMS_STATE, MmsCnt) != 0)
		return MSG_ERR_SET_SETTING;

	int sumCnt = SmsCnt + MmsCnt;

	if (MsgSettingSetInt(MSG_UNREAD_CNT, sumCnt) != 0)
		return MSG_ERR_SET_SETTING;

	return MSG_SUCCESS;
}
