/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved
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
#include "SmsCdmaPluginTransport.h"
#include "SmsCdmaPluginStorage.h"
#include "SmsCdmaPluginCallback.h"
#include "SmsCdmaPluginEventHandler.h"
#include "SmsCdmaPluginUAManager.h"
#include "SmsCdmaPluginSetting.h"
#include "SmsCdmaPluginMain.h"
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

struct tapi_handle *pTapiHandle = NULL;
bool isMemAvailable = true;

GDBusConnection *gdbus_conn = NULL;
GDBusProxy *gdbus_proxy = NULL;
gint subs_id = 0;

Mutex mx;
CndVar cv;


void MsgResourceMonitorInit(void);
void MsgResourceMonitorDeinit(void);

/*==================================================================================================
                                FUNCTION IMPLEMENTATION
==================================================================================================*/

static void MsgTapiInitCB(keynode_t *key, void* data)
{
	MSG_DEBUG("MsgTapiInitCB is called.");
	mx.lock();
	cv.signal();
	mx.unlock();
}


msg_error_t MsgPlgCreateHandle(MSG_PLUGIN_HANDLER_S *pPluginHandle)
{
	if (pPluginHandle == NULL) {
		MSG_DEBUG("SMS plugin: create handler error ");
		return MSG_ERR_NULL_POINTER;
	} else {
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

		MSG_DEBUG("SMS plugin: create handler OK");
		MSG_DEBUG ("SMS plugin %p", pPluginHandle);
	}

	return MSG_SUCCESS;
}


msg_error_t MsgPlgDestroyHandle(MSG_PLUGIN_HANDLER_S *pPluginHandle)
{
	if (pPluginHandle != NULL) {
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
		MSG_DEBUG("Fail to wait telephony init complete.");
		return MSG_ERR_PLUGIN_TAPIINIT;
	}

	pTapiHandle = tel_init(NULL);

	if (pTapiHandle) {
		/* register event. */
		SmsPluginCallback::instance()->registerEvent();

		/* set sim change status. */
		MSG_DEBUG("Try to initialize SIM on init");
		SmsPluginSetting::instance()->setSimChangeStatus();

		/* set resource monitor */
		MsgResourceMonitorInit();
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t SmsPlgFinalize()
{
	MSG_BEGIN();

	MsgResourceMonitorDeinit();

	if (!pTapiHandle)
		return MSG_ERR_PLUGIN_TAPIINIT;

	SmsPluginCallback::instance()->deRegisterEvent();

	tel_deinit(pTapiHandle);

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

	/* Add Submit SMS into DB */
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

	/* Check SIM is present or not */
	/*
	MSG_SIM_STATUS_T simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(MSG_SIM_CHANGED);

	if (simStatus == MSG_SIM_STATUS_NOT_FOUND) {
		MSG_DEBUG("SIM is not present..");

		if (pReqInfo->msgInfo.msgPort.valid == false)
			SmsPluginStorage::instance()->updateSentMsg(&(pReqInfo->msgInfo), MSG_NETWORK_SEND_FAIL);

		return MSG_ERR_NO_SIM;
	}
	*/

	sms_request_info_s *request = NULL;

	request = (sms_request_info_s *)calloc(1, sizeof(sms_request_info_s));

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
	MSG_DEBUG("CDMA does not support sim card operations.");
	return MSG_SUCCESS;
}


msg_error_t SmsPlgDeleteSimMessage(msg_sim_id_t SimMsgId)
{
	MSG_DEBUG("CDMA does not support sim card operations.");
	return MSG_SUCCESS;
}


msg_error_t SmsPlgSetReadStatus(msg_sim_id_t SimMsgId)
{
	MSG_DEBUG("CDMA does not support sim card operations.");
	return MSG_SUCCESS;
}


msg_error_t SmsPlgSetMemoryStatus(msg_error_t Error)
{

	int tapiRet = TAPI_API_SUCCESS;
	int status = TAPI_NETTEXT_PDA_MEMORY_STATUS_AVAILABLE;

	if (Error == MSG_ERR_SIM_STORAGE_FULL || Error == MSG_ERR_MESSAGE_COUNT_FULL) {
		status = TAPI_NETTEXT_PDA_MEMORY_STATUS_FULL;
	}

	MSG_DEBUG("Set Status : [%d]", status);

	tapiRet = tel_set_sms_memory_status(pTapiHandle, status, TapiEventMemoryStatus, NULL);

	if (tapiRet == TAPI_API_SUCCESS) {
		MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! #######");
	} else {
		MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! return : [%d] #######", tapiRet);
	}

	return MSG_SUCCESS;
}


msg_error_t SmsPlgSetConfigData(const MSG_SETTING_S *pSetting)
{
	try {
		SmsPluginSetting::instance()->setConfigData(pSetting);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_SETTING;
	} catch (exception& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_SETTING;
	}

	return MSG_SUCCESS;
}


msg_error_t SmsPlgGetConfigData(MSG_SETTING_S *pSetting)
{
	try {
		SmsPluginSetting::instance()->getConfigData(pSetting);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_SETTING;
	} catch (exception& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_SETTING;
	}

	return MSG_SUCCESS;
}


msg_error_t SmsPlgAddMessage(MSG_MESSAGE_INFO_S *pMsgInfo,  MSG_SENDINGOPT_INFO_S* pSendOptInfo, char* pFileData)
{
	MSG_DEBUG("CDMA does not support sim card operations.");
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
		if (memStatus == 0) {
			SmsPlgSetMemoryStatus(MSG_SUCCESS);
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
