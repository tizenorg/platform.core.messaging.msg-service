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

#include <errno.h>

#include "MsgDebug.h"
#include "MsgException.h"
#include "MsgGconfWrapper.h"

#include "MsgMutex.h"
#include "SmsPluginTransport.h"
#include "SmsPluginSimMsg.h"
#include "SmsPluginStorage.h"
#include "SmsPluginSetting.h"
#include "SmsPluginCallback.h"
#include "SmsPluginEventHandler.h"
#include "SmsPluginUAManager.h"
#include "SmsPluginMain.h"
#include "SmsPluginDSHandler.h"
#include <gio/gio.h>

extern "C"
{
	#include <tapi_common.h>
	#include <TelSms.h>
	#include <TapiUtility.h>
	#include <ITapiSim.h>
	#include <ITapiNetText.h>
}

#define BUS_NAME "org.tizen.system.deviced"
#define PATH_NAME "/Org/Tizen/System/DeviceD/Lowmem"
#define INTERFACE_NAME BUS_NAME".lowmem"
#define MEMBER_NAME "Full"

GDBusConnection *gdbus_conn = NULL;
GDBusProxy *gdbus_proxy = NULL;
gint subs_id = 0;

bool isMemAvailable = true;

Mutex mx;
CndVar cv;


void MsgResourceMonitorInit(void);
void MsgResourceMonitorDeinit(void);

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

msg_error_t MsgPlgCreateHandle(MSG_PLUGIN_HANDLER_S *pPluginHandle)
{
	if (pPluginHandle == NULL)
	{
		MSG_DEBUG("SMS plugin: create handler error ");
		return MSG_ERR_NULL_POINTER;
	}
	else
	{
		pPluginHandle->pfInitialize = SmsPlgInitialize;
		pPluginHandle->pfFinalize = SmsPlgFinalize;
		pPluginHandle->pfRegisterListener = SmsPlgRegisterListener;
		pPluginHandle->pfSubmitRequest = SmsPlgSubmitRequest;
		pPluginHandle->pfSaveSimMessage = SmsPlgSaveSimMessage;
		pPluginHandle->pfDeleteSimMessage = SmsPlgDeleteSimMessage;
		pPluginHandle->pfSetReadStatus = SmsPlgSetReadStatus;
		pPluginHandle->pfSetMemoryStatus = SmsPlgSetMemoryStatus;
		pPluginHandle->pfSetConfigData = SmsPlgSetConfigData;
		pPluginHandle->pfGetConfigData = SmsPlgGetConfigData;
		pPluginHandle->pfAddMessage = SmsPlgAddMessage;
		pPluginHandle->pfGetDefaultNetworkSimId = SmsPlgGetDefaultNetworkSimId;

		MSG_DEBUG("SMS plugin: create handler OK");
		MSG_DEBUG ("SMS plugin %p", pPluginHandle);
	}

	return MSG_SUCCESS;
}


msg_error_t MsgPlgDestroyHandle(MSG_PLUGIN_HANDLER_S *pPluginHandle)
{
	if (pPluginHandle != NULL)
	{
		free(pPluginHandle);
		pPluginHandle = NULL;
	}

	MSG_DEBUG("SMS plugin: destory handler OK");

	return MSG_SUCCESS;
}


msg_error_t SmsPlgInitialize()
{
	MSG_BEGIN();

	bool bReady = false;

	for (int i = 0; i < 100; i++) {
		MsgSettingGetBool(VCONFKEY_TELEPHONY_READY, &bReady);
		MSG_DEBUG("Get VCONFKEY_TELEPHONY_READY [%d].", bReady ? 1 : 0);

		if (bReady)
			break;

		sleep(1);
	}

	if (!bReady) {
		MSG_ERR("Fail to wait telephony init complete.");
		return MSG_ERR_PLUGIN_TAPIINIT;
	}

	int simCnt = 0;
	char keyName[MAX_VCONFKEY_NAME_LEN];

	SmsPluginDSHandler::instance()->initTelHandle();
	simCnt = SmsPluginDSHandler::instance()->getTelHandleCount();

	MSG_DEBUG("simCnt [%d]", simCnt);

	for (int i = 1; i <= simCnt; i++) {
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_CHANGED, i);
		MSG_DEBUG("set MSG_SIM_CHANGED to MSG_SIM_STATUS_NOT_FOUND.");
		if (MsgSettingSetInt(keyName, MSG_SIM_STATUS_NOT_FOUND) != MSG_SUCCESS)
			MSG_DEBUG("MsgSettingSetInt is failed!!");
	}

	SmsPluginCallback::instance()->registerEvent();

	for(int i=1; i <= simCnt; ++i)
	{
		struct tapi_handle *handle;
		handle = SmsPluginDSHandler::instance()->getTelHandle(i);
		SmsPluginSetting::instance()->setSimChangeStatus(handle, true);
	}

	// set resource monitor
	MsgResourceMonitorInit();

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t SmsPlgFinalize()
{
	MSG_BEGIN();

	MsgResourceMonitorDeinit();

	SmsPluginCallback::instance()->deRegisterEvent();

	SmsPluginDSHandler::instance()->deinitTelHandle();

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t SmsPlgRegisterListener(MSG_PLUGIN_LISTENER_S *pListener)
{
	MSG_BEGIN();

	SmsPluginEventHandler::instance()->registerListener(pListener);

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t SmsPlgSubmitRequest(MSG_REQUEST_INFO_S *pReqInfo)
{
	msg_error_t err = MSG_SUCCESS;

	// Add Submit SMS into DB
	if (pReqInfo->msgInfo.msgId == 0) {
		if (pReqInfo->msgInfo.msgPort.valid == false) {
			err = SmsPluginStorage::instance()->checkMessage(&(pReqInfo->msgInfo));

			if (err != MSG_SUCCESS) {
				MSG_DEBUG("########  checkMessage Fail !! [err=%d]", err);
				return MSG_ERR_PLUGIN_STORAGE;
			}

			err = SmsPluginStorage::instance()->addSmsMessage(&(pReqInfo->msgInfo));
			if (err != MSG_SUCCESS) {
				MSG_DEBUG("########  addSmsMessage Fail !! [err=%d]", err);
				return MSG_ERR_PLUGIN_STORAGE;
			}

			if (SmsPluginStorage::instance()->addSmsSendOption(&(pReqInfo->msgInfo), &(pReqInfo->sendOptInfo)) != MSG_SUCCESS) {
				MSG_DEBUG("########  addSmsSendOption Fail !!");
				return MSG_ERR_PLUGIN_STORAGE;
			}
		}
	}

	// Check SIM is present or not
	char keyName[MAX_VCONFKEY_NAME_LEN] = {0,};
	sprintf(keyName, "%s/%d", MSG_SIM_CHANGED, pReqInfo->msgInfo.sim_idx);
	MSG_SIM_STATUS_T simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(keyName);

	if (simStatus == MSG_SIM_STATUS_NOT_FOUND)
	{
		MSG_DEBUG("SIM is not present..");

		// Update Msg Status
		if (pReqInfo->msgInfo.msgPort.valid == false)
			SmsPluginStorage::instance()->updateSentMsg(&(pReqInfo->msgInfo), MSG_NETWORK_SEND_FAIL);

		return MSG_ERR_NO_SIM;
	}

	SMS_REQUEST_INFO_S *request = NULL;

	request = (SMS_REQUEST_INFO_S *)calloc(1, sizeof(SMS_REQUEST_INFO_S));

	if (request != NULL) {
		request->reqId = pReqInfo->reqId;

		memcpy(&(request->msgInfo), &(pReqInfo->msgInfo), sizeof(MSG_MESSAGE_INFO_S));
		memcpy(&(request->sendOptInfo), &(pReqInfo->sendOptInfo), sizeof(MSG_SENDINGOPT_INFO_S));

		/* Add Request into Queue and Start UA Manger */
		SmsPluginUAManager::instance()->addReqEntity(request);

		free(request);
	}


	return MSG_SUCCESS;
}


msg_error_t SmsPlgSaveSimMessage(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList)
{
	// Check SIM is present or not
	char keyName[MAX_VCONFKEY_NAME_LEN];
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_CHANGED, pMsgInfo->sim_idx);
	MSG_SIM_STATUS_T simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(keyName);

	if (simStatus == MSG_SIM_STATUS_NOT_FOUND) {
		MSG_DEBUG("SIM is not present..");
		return MSG_ERR_NO_SIM;
	}

	msg_error_t err = MSG_SUCCESS;

	try
	{
		err = SmsPluginSimMsg::instance()->saveSimMessage(pMsgInfo, pSimIdList);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	}
	catch (exception& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	}

	return err;
}


msg_error_t SmsPlgDeleteSimMessage(msg_sim_slot_id_t sim_idx, msg_sim_id_t SimMsgId)
{
	// Check SIM is present or not
	char keyName[MAX_VCONFKEY_NAME_LEN]={0,};
	sprintf(keyName, "%s/%d", MSG_SIM_CHANGED, sim_idx);
	MSG_SIM_STATUS_T simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(keyName);

	if (simStatus == MSG_SIM_STATUS_NOT_FOUND) {
		MSG_DEBUG("SIM is not present..");
		return MSG_ERR_NO_SIM;
	}

	try
	{
		SmsPluginSimMsg::instance()->deleteSimMessage(sim_idx, SimMsgId);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	}
	catch (exception& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	}

	return MSG_SUCCESS;
}


msg_error_t SmsPlgSetReadStatus(msg_sim_slot_id_t sim_idx, msg_sim_id_t SimMsgId)
{
	// Check SIM is present or not
	char keyName[MAX_VCONFKEY_NAME_LEN]={0,};
	sprintf(keyName, "%s/%d", MSG_SIM_CHANGED, sim_idx);
	MSG_SIM_STATUS_T simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(keyName);

	if (simStatus == MSG_SIM_STATUS_NOT_FOUND) {
		MSG_DEBUG("SIM is not present..");
		return MSG_ERR_NO_SIM;
	}

	try
	{
		SmsPluginSimMsg::instance()->setReadStatus(sim_idx, SimMsgId);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	}
	catch (exception& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	}

	return MSG_SUCCESS;
}


msg_error_t SmsPlgSetMemoryStatus(msg_sim_slot_id_t simIndex, msg_error_t Error)
{
	// Check SIM is present or not
	char keyName[MAX_VCONFKEY_NAME_LEN];

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_CHANGED, simIndex);
	MSG_SIM_STATUS_T simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(keyName);

	if (simStatus == MSG_SIM_STATUS_NOT_FOUND) {
		MSG_DEBUG("SIM is not present..");
		return MSG_ERR_NO_SIM;
	}

	int tapiRet = TAPI_API_SUCCESS;
	int status = TAPI_NETTEXT_PDA_MEMORY_STATUS_AVAILABLE;

	if (Error == MSG_ERR_SIM_STORAGE_FULL || Error == MSG_ERR_MESSAGE_COUNT_FULL)
	{
		status = TAPI_NETTEXT_PDA_MEMORY_STATUS_FULL;
	}

	MSG_DEBUG("Set Status : [%d]", status);

	struct tapi_handle *handle = SmsPluginDSHandler::instance()->getTelHandle(simIndex);

	tapiRet = tel_set_sms_memory_status(handle, status, TapiEventMemoryStatus, NULL);

	if (tapiRet == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! #######");
	}
	else
	{
		MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! return : [%d] #######", tapiRet);
	}

	return MSG_SUCCESS;
}


msg_error_t SmsPlgSetConfigData(const MSG_SETTING_S *pSetting)
{
	try
	{
		SmsPluginSetting::instance()->setConfigData(pSetting);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_SETTING;
	}
	catch (exception& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_SETTING;
	}

	return MSG_SUCCESS;
}


msg_error_t SmsPlgGetConfigData(MSG_SETTING_S *pSetting)
{
	try
	{
		SmsPluginSetting::instance()->getConfigData(pSetting);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_SETTING;
	}
	catch (exception& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_SETTING;
	}

	return MSG_SUCCESS;
}


msg_error_t SmsPlgAddMessage(MSG_MESSAGE_INFO_S *pMsgInfo,  MSG_SENDINGOPT_INFO_S* pSendOptInfo, char* pFileData)
{

	int *simIdList = (int*)pFileData;
	try
	{
		SmsPluginStorage::instance()->addSmsSendOption(pMsgInfo, pSendOptInfo);
		if (simIdList)
			SmsPluginStorage::instance()->addSimMessage(pMsgInfo, simIdList);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_SETTING;
	}
	catch (exception& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_SETTING;
	}

	return MSG_SUCCESS;
}


msg_error_t SmsPlgGetDefaultNetworkSimId(int *simId)
{

	try
	{
		SmsPluginDSHandler::instance()->getDefaultNetworkSimId(simId);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_SETTING;
	}
	catch (exception& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_SETTING;
	}

	return MSG_SUCCESS;
}


static void on_change_received(GDBusConnection *connection, const gchar *sender_name,
		const gchar *object_path, const gchar *interface_name, const gchar *signal_name,
		GVariant *parameters, gpointer user_data)
{
	MSG_DEBUG("signal_name = [%s]", signal_name);

	if (g_strcmp0(signal_name, MEMBER_NAME) == 0) {
		gint memStatus;
		g_variant_get(parameters, "(i)", &memStatus);
		MSG_DEBUG("memStatus = [%d]", memStatus);
		if(memStatus == 0) {
			int sim_count = SmsPluginDSHandler::instance()->getTelHandleCount();

			for (int i = 0; i < sim_count; i++) {
				SmsPlgSetMemoryStatus(i, MSG_SUCCESS);
			}
		}
	}
}

void MsgResourceMonitorInit(void)
{
    MSG_BEGIN();

	GError *error = NULL;

	if (gdbus_conn) {
		g_object_unref(gdbus_conn);
		gdbus_conn = NULL;
	}

	gdbus_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (error) {
		MSG_FATAL("g_bus_get_sync() failed : %s", error->message);
		g_error_free(error);
		error = NULL;
		goto _DBUS_ERROR;
	}

	if (gdbus_proxy) {
		g_object_unref(gdbus_proxy);
		gdbus_proxy = NULL;
	}

	gdbus_proxy = g_dbus_proxy_new_sync(gdbus_conn, G_DBUS_PROXY_FLAGS_NONE,
							NULL, BUS_NAME, PATH_NAME, INTERFACE_NAME, NULL, &error);
	if (error) {
		MSG_FATAL("g_dbus_proxy_new_sync() failed : %s", error->message);
		g_error_free(error);
		error = NULL;
		goto _DBUS_ERROR;
	}

	subs_id = g_dbus_connection_signal_subscribe(gdbus_conn, NULL,
							INTERFACE_NAME, MEMBER_NAME, PATH_NAME,
							NULL, G_DBUS_SIGNAL_FLAGS_NONE,
							on_change_received,
							NULL, NULL);
	MSG_END();
	return;

_DBUS_ERROR:
	if (gdbus_conn) {
		g_object_unref(gdbus_conn);
		gdbus_conn = NULL;
	}

	if (gdbus_proxy) {
		g_object_unref(gdbus_proxy);
		gdbus_proxy = NULL;
	}

	MSG_END();
	return;

}


void MsgResourceMonitorDeinit(void)
{
	MSG_BEGIN();

	if (subs_id) {
		g_dbus_connection_signal_unsubscribe(gdbus_conn, subs_id);
		subs_id = 0;
	}

	if (gdbus_conn) {
		g_object_unref(gdbus_conn);
		gdbus_conn = NULL;
	}

	if (gdbus_proxy) {
		g_object_unref(gdbus_proxy);
		gdbus_proxy = NULL;
	}

	MSG_END();
}
