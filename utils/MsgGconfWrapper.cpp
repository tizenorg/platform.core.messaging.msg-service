/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include <stdio.h>

#include "MsgDebug.h"
#include "MsgUtilStorage.h"
#include "MsgGconfWrapper.h"
#include "MsgException.h"

int autoReject = 0;
bool bUnknownAutoReject = false;



/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

msg_error_t MsgSettingSetString(const char *pKey, const char *pSetValue)
{
	if (pKey == NULL || pSetValue == NULL) {
		MSG_DEBUG("IN Parameter is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if (vconf_set_str(pKey, pSetValue) != 0) {
		int vconf_err = vconf_get_ext_errno();
		MSG_DEBUG("Fail to vconf_set_str with [%s], err=[%d]", pKey, vconf_err);
		return MSG_ERR_SET_SETTING;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgSettingSetInt(const char *pKey, int nSetValue)
{
	if (pKey == NULL) {
		MSG_DEBUG("IN Parameter is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if (vconf_set_int(pKey, nSetValue) != 0) {
		int vconf_err = vconf_get_ext_errno();
		MSG_DEBUG("Fail to vconf_set_int with [%s], err=[%d]", pKey, vconf_err);
		return MSG_ERR_SET_SETTING;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgSettingSetBool(const char *pKey, bool bSetValue)
{
	if (pKey == NULL) {
		MSG_DEBUG("IN Parameter is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if (vconf_set_bool(pKey, bSetValue) != 0) {
		int vconf_err = vconf_get_ext_errno();
		MSG_DEBUG("Fail to vconf_set_bool with [%s], err=[%d]", pKey, vconf_err);
		return MSG_ERR_SET_SETTING;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgSettingGetString(const char *pKey, char **pVal)
{
	if (pKey == NULL) {
		MSG_DEBUG("IN Parameter is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	msg_error_t retVal = MSG_SUCCESS;
	char *param = NULL;

	param = vconf_get_str(pKey);
	if (param == NULL) {
		int vconf_err = vconf_get_ext_errno();
		MSG_DEBUG("Fail to vconf_get_str with [%s], err=[%d]", pKey, vconf_err);
		if (vconf_err == VCONF_ERROR_FILE_PERM)
			retVal = MSG_ERR_PERMISSION_DENIED;
		else
			retVal = MSG_ERR_UNKNOWN;
	}

	*pVal = param;

	return retVal;
}


msg_error_t MsgSettingGetInt(const char *pKey, int *pVal)
{
	if (pKey == NULL) {
		MSG_DEBUG("IN Parameter is NULL");
		return -1;
	}

	msg_error_t retVal = MSG_SUCCESS;
	int param = 0;

	if (vconf_get_int(pKey, &param) != 0) {
		int vconf_err = vconf_get_ext_errno();
		MSG_DEBUG("Fail to vconf_get_int with [%s], err=[%d]", pKey, vconf_err);
		if (vconf_err == VCONF_ERROR_FILE_PERM)
			retVal = MSG_ERR_PERMISSION_DENIED;
		else
			retVal = MSG_ERR_UNKNOWN;
	}

	*pVal = (int)param;

	return retVal;
}


msg_error_t MsgSettingGetBool(const char *pKey, bool *pVal)
{
	if (pKey == NULL) {
		MSG_DEBUG("IN Parameter is NULL");
		return -1;
	}

	msg_error_t retVal = MSG_SUCCESS;
	int param = 0;

	if (vconf_get_bool(pKey, &param) != 0) {
		int vconf_err = vconf_get_ext_errno();
		MSG_DEBUG("Fail to vconf_get_bool with [%s], err=[%d]", pKey, vconf_err);
		if (vconf_err == VCONF_ERROR_FILE_PERM)
			retVal = MSG_ERR_PERMISSION_DENIED;
		else
			retVal = MSG_ERR_UNKNOWN;
	}

	*pVal = (bool)param;

	return retVal;
}


msg_error_t MsgSettingHandleNewMsg(int SmsCnt, int MmsCnt)
{
	MSG_BEGIN();

	MSG_DEBUG("smsCnt = %d, mmsCnt = %d ##", SmsCnt, MmsCnt);

	/* Set Msg Count into VConf */
	if (MsgSettingSetIndicator(SmsCnt, MmsCnt) != MSG_SUCCESS) {
		MSG_DEBUG("MsgSettingSetIndicator() FAILED");
		return MSG_ERR_SET_SETTING;
	}

#if 0
	if (SmsCnt == 0 && MmsCnt == 0) {
		MSG_DEBUG("No New Message.");
	} else {
		MSG_DEBUG("New Message.");

		bool bNotification = true;

		if (MsgSettingGetBool(MSG_SETTING_NOTIFICATION, &bNotification) != MSG_SUCCESS) {
			MSG_DEBUG("MsgSettingGetBool is failed.");
		}

		if (bNotification)
			MsgChangePmState();
	}
#endif

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgSettingSetIndicator(int SmsCnt, int MmsCnt)
{
	if (MsgSettingSetInt(VCONFKEY_MESSAGE_RECV_SMS_STATE, SmsCnt) != 0)
		return MSG_ERR_SET_SETTING;
	if (MsgSettingSetInt(VCONFKEY_MESSAGE_RECV_MMS_STATE, MmsCnt) != 0)
		return MSG_ERR_SET_SETTING;

	return MSG_SUCCESS;
}


int MsgSettingGetAutoReject()
{
	return autoReject;
}

bool MsgSettingGetUnknownAutoReject()
{
	return bUnknownAutoReject;
}

msg_error_t MsgSettingRegVconfCBCommon(const char *pKey, _vconf_change_cb pCb)
{
	msg_error_t err = MSG_SUCCESS;

	if (vconf_notify_key_changed(pKey, pCb, NULL) != 0) {
		int vconf_err = vconf_get_ext_errno();
		MSG_DEBUG("Fail to vconf_notify_key_changed with [%s], err=[%d]", pKey, vconf_err);
		if (vconf_err == VCONF_ERROR_FILE_PERM)
			err = MSG_ERR_PERMISSION_DENIED;
		else
			err = MSG_ERR_UNKNOWN;
	} else {
		MSG_DEBUG("Success to regist vconf CB with [%s]", pKey);
	}

	return err;
}

msg_error_t MsgSettingRemoveVconfCBCommon(const char *pKey, _vconf_change_cb pCb)
{
	msg_error_t err = MSG_SUCCESS;

	if (vconf_ignore_key_changed(pKey, pCb) != 0) {
		int vconf_err = vconf_get_ext_errno();
		MSG_DEBUG("Fail to vconf_ignore_key_changed [%s], err=[%d]", pKey, vconf_err);
		if (vconf_err == VCONF_ERROR_FILE_PERM)
			err = MSG_ERR_PERMISSION_DENIED;
		else
			err = MSG_ERR_UNKNOWN;
	} else {
		MSG_DEBUG("Success to remove vconf CB with [%s]", pKey);
	}

	return err;
}
