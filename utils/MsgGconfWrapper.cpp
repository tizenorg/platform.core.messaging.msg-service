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
#ifdef MSG_PENDING_PUSH_MESSAGE
#include "MsgIpcSocket.h"
#endif

#ifdef USE_GCONF

#define GCONF_SUCCESS 1

MSG_GOBJECT_CLIENT_S* pClient = NULL;

#endif

#ifdef MSG_PENDING_PUSH_MESSAGE
int bPushServiceReady = 0;
#endif

int autoReject = 0;
bool bUnknownAutoReject = false;



/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
static void MsgVconfCB(keynode_t *key, void* data)
{
#if 0
	char *keyStr = NULL;
	keyStr = vconf_keynode_get_name(key);

	if (!keyStr)
		return;

	if (!g_strcmp0(keyStr, VCONFKEY_CISSAPPL_AUTO_REJECT_INT)) {
		autoReject = vconf_keynode_get_int(key);
		MSG_DEBUG("[%s] key CB called. set to [%d].", VCONFKEY_CISSAPPL_AUTO_REJECT_INT, autoReject);
	} else if (!g_strcmp0(keyStr, VCONFKEY_CISSAPPL_AUTO_REJECT_UNKNOWN_BOOL)) {
		bUnknownAutoReject = vconf_keynode_get_bool(key);
		MSG_DEBUG("[%s] key CB called. set to [%d].", VCONFKEY_CISSAPPL_AUTO_REJECT_UNKNOWN_BOOL, bUnknownAutoReject);
	}
#ifdef MSG_PENDING_PUSH_MESSAGE
	else if (!g_strcmp0(keyStr, VCONFKEY_USER_SERVICE_READY)){
		bPushServiceReady = vconf_keynode_get_int(key);
		MSG_DEBUG("[%s] key CB called. set to [%d].", VCONFKEY_USER_SERVICE_READY, bPushServiceReady);

		if(bPushServiceReady)
		{
			try {
				if (MsgSendPendingPushMsg() == MSG_SUCCESS) {
					MSG_DEBUG("MsgSendPendingPushMsg success");
				} else {
					MSG_DEBUG("MsgSendPendingPushMsg fail");
				}
			} catch (MsgException& e) {
				MSG_FATAL("%s", e.what());
				MSG_DEBUG("MsgSendPendingPushMsg fail");
			}
		}
	}
#endif
	else {
		MSG_DEBUG("key did not match.");
	}
#endif
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

//	if (SmsCnt == 0 && MmsCnt == 0)
//	{
//		MSG_DEBUG("No New Message.");
//	}
//	else
//	{
//		MSG_DEBUG("New Message.");
//
//		bool bNotification = true;
//
//		if (MsgSettingGetBool(MSG_SETTING_NOTIFICATION, &bNotification) != MSG_SUCCESS) {
//			MSG_DEBUG("MsgSettingGetBool is failed.");
//		}
//
//		if (bNotification)
//			MsgChangePmState();
//	}

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

void MsgSettingRegVconfCBCommon(const char *pKey, _vconf_change_cb pCb)
{
	if (vconf_notify_key_changed(pKey, pCb, NULL) < 0) {
		MSG_DEBUG("Fail to regist vconf CB with [%s]", pKey);
	} else {
		MSG_DEBUG("Success to regist vconf CB with [%s]", pKey);
	}
}

void MsgSettingRemoveVconfCBCommon(const char *pKey, _vconf_change_cb pCb)
{
	if (vconf_ignore_key_changed(pKey, pCb) < 0) {
		MSG_DEBUG("Fail to remove vconf CB with [%s]", pKey);
	} else {
		MSG_DEBUG("Success to remove vconf CB with [%s]", pKey);
	}
}


void MsgSettingRegVconfCB()
{
#if 0
	// Set default values.
	autoReject = MsgSettingGetInt(VCONFKEY_CISSAPPL_AUTO_REJECT_INT);
	MsgSettingGetBool(VCONFKEY_CISSAPPL_AUTO_REJECT_UNKNOWN_BOOL, &bUnknownAutoReject);

	MsgSettingRegVconfCBCommon(VCONFKEY_CISSAPPL_AUTO_REJECT_INT, MsgVconfCB);
	MsgSettingRegVconfCBCommon(VCONFKEY_CISSAPPL_AUTO_REJECT_UNKNOWN_BOOL, MsgVconfCB);

#ifdef MSG_PENDING_PUSH_MESSAGE
	MsgSettingRegVconfCBCommon(VCONFKEY_USER_SERVICE_READY, MsgVconfCB);
#endif
#endif
}

void MsgSettingRemoveVconfCB()
{
#if 0
	MsgSettingRemoveVconfCBCommon(VCONFKEY_CISSAPPL_AUTO_REJECT_INT, MsgVconfCB);
	MsgSettingRemoveVconfCBCommon(VCONFKEY_CISSAPPL_AUTO_REJECT_UNKNOWN_BOOL, MsgVconfCB);
#endif
}

msg_error_t MsgSendPendingPushMsg(void)
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	client.connect(MSG_SOCKET_PATH);

	// composing command
	int cmdSize = sizeof(MSG_CMD_S); // cmd type, MSG_SYNCML_MESSAGE_DATA_S

	MSG_DEBUG("cmdSize: %d", cmdSize);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SEND_PENDING_PUSH_MESSAGE;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	// Send Command to Messaging FW
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	char* retBuf = NULL;
	AutoPtr<char> wrap(&retBuf);
	unsigned int retSize;
	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	// the result is used for making delivery report
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_SEND_PENDING_PUSH_MESSAGE)
		MSG_FATAL("Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));
		//THROW(MsgException::INCOMING_MSG_ERROR, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}
