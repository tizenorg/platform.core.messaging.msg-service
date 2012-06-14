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
