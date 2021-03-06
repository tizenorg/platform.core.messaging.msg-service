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

#ifdef USE_GCONF

#define GCONF_SUCCESS 1

MSG_GOBJECT_CLIENT_S* pClient = NULL;

#endif

bool bAutoReject = false;
bool bUnknownAutoReject = false;


/*==================================================================================================
                                     DEFINES
==================================================================================================*/


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
static void MsgVconfCB(keynode_t *key, void* data)
{
	char *keyStr = NULL;
	keyStr = vconf_keynode_get_name(key);

	if (!keyStr)
		return;

	if (!strcmp(keyStr, VCONFKEY_CISSAPPL_AUTO_REJECT_BOOL)) {
		bAutoReject = vconf_keynode_get_bool(key);
		MSG_DEBUG("[%s] key CB called. set to [%d].", VCONFKEY_CISSAPPL_AUTO_REJECT_BOOL, bAutoReject);
	} else if (!strcmp(keyStr, VCONFKEY_CISSAPPL_AUTO_REJECT_UNKNOWN_BOOL)) {
		bUnknownAutoReject = vconf_keynode_get_bool(key);
		MSG_DEBUG("[%s] key CB called. set to [%d].", VCONFKEY_CISSAPPL_AUTO_REJECT_UNKNOWN_BOOL, bUnknownAutoReject);
	} else {
		MSG_DEBUG("key did not match.");
	}
}

msg_error_t MsgSettingSetString(const char *pKey, const char *pSetValue)
{
	if (pKey == NULL || pSetValue == NULL)
	{
		MSG_DEBUG("IN Parameter is NULL");
		return MSG_ERR_NULL_POINTER;
	}

#ifdef USE_GCONF
	if (gconf_client_set_string((GConfClient*)pClient, pKey, pSetValue, NULL) !=  GCONF_SUCCESS)
		return MSG_ERR_SET_SETTING;
#else
	if (vconf_set_str(pKey, pSetValue) != 0)
		return MSG_ERR_SET_SETTING;
#endif

	return MSG_SUCCESS;
}


msg_error_t MsgSettingSetInt(const char *pKey, int nSetValue)
{
	if (pKey == NULL)
	{
		MSG_DEBUG("IN Parameter is NULL");
		return MSG_ERR_NULL_POINTER;
	}

#ifdef USE_GCONF
	if (gconf_client_set_int((GConfClient*)pClient, pKey, nSetValue, NULL) !=  GCONF_SUCCESS)
		return MSG_ERR_SET_SETTING;
#else
	if (vconf_set_int(pKey, nSetValue) != 0)
		return MSG_ERR_SET_SETTING;
#endif

	return MSG_SUCCESS;
}


msg_error_t MsgSettingSetBool(const char *pKey, bool bSetValue)
{
	if (pKey == NULL)
	{
		MSG_DEBUG("IN Parameter is NULL");
		return MSG_ERR_NULL_POINTER;
	}

#ifdef USE_GCONF
	if (gconf_client_set_bool((GConfClient*)pClient, pKey, bSetValue, NULL) !=  GCONF_SUCCESS)
		return MSG_ERR_SET_SETTING;
#else
	if (vconf_set_bool(pKey, bSetValue) != 0)
		return MSG_ERR_SET_SETTING;
#endif

	return MSG_SUCCESS;
}


char* MsgSettingGetString(const char *pKey)
{
	if (pKey == NULL)
	{
		MSG_DEBUG("IN Parameter is NULL");
		return NULL;
	}

#ifdef USE_GCONF
	return gconf_client_get_string((GConfClient*)pClient, pKey, NULL);
#else
	return vconf_get_str(pKey);
#endif
}


int MsgSettingGetInt(const char *pKey)
{
	if (pKey == NULL)
	{
		MSG_DEBUG("IN Parameter is NULL");
		return -1;
	}

	int retVal = 0;

#ifdef USE_GCONF
	retVal = gconf_client_get_int((GConfClient*)pClient, pKey, NULL);
#else
	if (vconf_get_int(pKey, &retVal) < 0)
		return -1;
#endif

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

#ifdef USE_GCONF
	*pVal = gconf_client_get_bool((GConfClient*)pClient, pKey, NULL);
#else
	if (vconf_get_bool(pKey, &param) < 0)
		return -1;
#endif

 	*pVal = (bool)param;

	return retVal;
}


msg_error_t MsgSettingHandleNewMsg(int SmsCnt, int MmsCnt)
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


msg_error_t MsgSettingSetIndicator(int SmsCnt, int MmsCnt)
{

	if (MsgSettingSetInt(VCONFKEY_MESSAGE_RECV_SMS_STATE, SmsCnt) != 0)
		return MSG_ERR_SET_SETTING;
	if (MsgSettingSetInt(VCONFKEY_MESSAGE_RECV_MMS_STATE, MmsCnt) != 0)
		return MSG_ERR_SET_SETTING;

	/* Not used currently.
	int sumCnt = SmsCnt + MmsCnt;

	if (MsgSettingSetInt(MSG_UNREAD_CNT, sumCnt) != 0)
		return MSG_ERR_SET_SETTING;
	*/

	return MSG_SUCCESS;
}


bool MsgSettingGetAutoReject()
{
	return bAutoReject;
}

bool MsgSettingGetUnknownAutoReject()
{
	return bUnknownAutoReject;
}


void MsgSettingRegVconfCB()
{
	// Set default values.
	MsgSettingGetBool(VCONFKEY_CISSAPPL_AUTO_REJECT_BOOL, &bAutoReject);
	MsgSettingGetBool(VCONFKEY_CISSAPPL_AUTO_REJECT_UNKNOWN_BOOL, &bUnknownAutoReject);

	if (vconf_notify_key_changed(VCONFKEY_CISSAPPL_AUTO_REJECT_BOOL, MsgVconfCB, NULL) < 0) {
		MSG_DEBUG("Fail to regist vconf CB with [%s]", VCONFKEY_CISSAPPL_AUTO_REJECT_BOOL);
	}

	if (vconf_notify_key_changed(VCONFKEY_CISSAPPL_AUTO_REJECT_UNKNOWN_BOOL, MsgVconfCB, NULL) < 0) {
		MSG_DEBUG("Fail to regist vconf CB with [%s]", VCONFKEY_CISSAPPL_AUTO_REJECT_UNKNOWN_BOOL);
	}
}

void MsgSettingRemoveVconfCB()
{
	if (vconf_ignore_key_changed(VCONFKEY_CISSAPPL_AUTO_REJECT_BOOL, MsgVconfCB) < 0) {
		MSG_DEBUG("Fail to regist vconf CB with [%s]", VCONFKEY_CISSAPPL_AUTO_REJECT_BOOL);
	}

	if (vconf_ignore_key_changed(VCONFKEY_CISSAPPL_AUTO_REJECT_UNKNOWN_BOOL, MsgVconfCB) < 0) {
		MSG_DEBUG("Fail to regist vconf CB with [%s]", VCONFKEY_CISSAPPL_AUTO_REJECT_UNKNOWN_BOOL);
	}
}

void MsgSettingRegVconfCBCommon(const char *pKey, _vconf_change_cb pCb)
{
	if (vconf_notify_key_changed(pKey, pCb, NULL) < 0) {
		MSG_DEBUG("Fail to regist vconf CB with [%s]", pKey);
	} else {
		MSG_DEBUG("Success to regist vconf CB with [%s]", pKey);
	}
}
