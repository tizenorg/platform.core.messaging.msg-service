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
#include "MsgException.h"
#include "MsgGconfWrapper.h"

#include "SmsPluginTransport.h"
#include "SmsPluginSimMsg.h"
#include "SmsPluginStorage.h"
#include "SmsPluginSetting.h"
#include "SmsPluginCallback.h"
#include "SmsPluginEventHandler.h"
#include "SmsPluginUAManager.h"
#include "SmsPluginMain.h"

extern "C"
{
	#include <ITapiSim.h>
	#include <ITapiNetText.h>
}


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
MSG_ERROR_T MsgPlgCreateHandle(MSG_PLUGIN_HANDLER_S *pPluginHandle)
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
		pPluginHandle->pfCheckSimStatus = SmsPlgCheckSimStatus;
		pPluginHandle->pfCheckDeviceStatus = SmsPlgCheckDeviceStatus;
		pPluginHandle->pfSubmitRequest = SmsPlgSubmitRequest;
		pPluginHandle->pfInitSimMessage = SmsPlgInitSimMessage;
		pPluginHandle->pfSaveSimMessage = SmsPlgSaveSimMessage;
		pPluginHandle->pfDeleteSimMessage = SmsPlgDeleteSimMessage;
		pPluginHandle->pfSetReadStatus = SmsPlgSetReadStatus;
		pPluginHandle->pfSetMemoryStatus = SmsPlgSetMemoryStatus;
		pPluginHandle->pfInitConfigData = SmsPlgInitConfigData;
		pPluginHandle->pfSetConfigData = SmsPlgSetConfigData;
		pPluginHandle->pfGetConfigData = SmsPlgGetConfigData;

		pPluginHandle->pfRestoreMsg = NULL;

		MSG_DEBUG("SMS plugin: create handler OK");
		MSG_DEBUG ("SMS plugin %p", pPluginHandle);
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgPlgDestroyHandle(MSG_PLUGIN_HANDLER_S *pPluginHandle)
{
	if (pPluginHandle != NULL)
	{
		free(pPluginHandle);
		pPluginHandle = NULL;
	}

	MSG_DEBUG("SMS plugin: destory handler OK");

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPlgInitialize()
{
	MSG_BEGIN();

	int tapiRet = TAPI_API_SUCCESS;

	tapiRet = tel_init();

	if (tapiRet != TAPI_API_SUCCESS)
		return MSG_ERR_PLUGIN_TAPIINIT;

	try
	{
		SmsPluginCallback::instance()->registerEvent();
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_REGEVENT;
	}
	catch (exception& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_REGEVENT;
	}

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPlgFinalize()
{
	MSG_BEGIN();

	SmsPluginCallback::instance()->deRegisterEvent();

	 tel_deinit();

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPlgRegisterListener(MSG_PLUGIN_LISTENER_S *pListener)
{
	MSG_BEGIN();

	SmsPluginEventHandler::instance()->registerListener(pListener);

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPlgCheckSimStatus(MSG_SIM_STATUS_T *pStatus)
{
	MSG_BEGIN();

	int tryNum = 0, tapiRet = TAPI_API_SUCCESS;

	TelSimCardStatus_t status = TAPI_SIM_STATUS_CARD_ERROR;
	int cardChanged = 0;

	// Check SIM Status
	while (1)
	{
		if (tryNum > 30) return MSG_ERR_PLUGIN_TAPIINIT;

		tapiRet = tel_get_sim_init_info(&status, &cardChanged);

		if (tapiRet == TAPI_API_SUCCESS)
		{
			if (status == TAPI_SIM_STATUS_SIM_PIN_REQUIRED || status == TAPI_SIM_STATUS_SIM_PUK_REQUIRED)
			{
				MSG_DEBUG("PIN or PUK is required [%d]", status);

				sleep(3);

				continue;
			}

			if (status == TAPI_SIM_STATUS_SIM_INIT_COMPLETED)
			{
				MSG_DEBUG("SIM status is OK [%d]", status);

				MSG_DEBUG("SIM Changed [%d]", cardChanged);

				if (cardChanged == 1)
					*pStatus = MSG_SIM_STATUS_CHANGED;
				else
					*pStatus = MSG_SIM_STATUS_NORMAL;

				break;
			}
			else if (status == TAPI_SIM_STATUS_CARD_NOT_PRESENT)
			{
				MSG_DEBUG("SIM is not present [%d]", status);

				*pStatus = MSG_SIM_STATUS_NOT_FOUND;

				break;
			}
			else
			{
				MSG_DEBUG("SIM status is not OK [%d]", status);

				tryNum++;

				sleep(3);
			}
		}
		else if (tapiRet == TAPI_API_SIM_NOT_FOUND)
		{
			MSG_DEBUG("tel_get_sim_init_info() result is TAPI_API_SIM_NOT_FOUND");

			*pStatus = MSG_SIM_STATUS_NOT_FOUND;

			break;
		}
		else
		{
			MSG_DEBUG("tel_get_sim_init_info() result is unknown!!!!!!!!!! [%d]", tapiRet);

			tryNum++;

			sleep(3);
		}
	}

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPlgCheckDeviceStatus()
{
	MSG_BEGIN();

	int status = 0, tapiRet = TAPI_API_SUCCESS;

	tapiRet = tel_check_sms_device_status(&status);

	if (tapiRet != TAPI_API_SUCCESS)
	{
		MSG_DEBUG("tel_check_sms_device_status() Error! [%d], Status [%d]", tapiRet, status);

		return MSG_ERR_PLUGIN_TAPI_FAILED;
	}

	if (status == TAPI_NETTEXT_DEVICE_READY)
	{
		MSG_DEBUG("Device Is Ready");
		return MSG_SUCCESS;
	}
	else if (status == TAPI_NETTEXT_DEVICE_NOT_READY)
	{
		MSG_DEBUG("Device Is Not Ready.. Waiting For Ready Callback");

		if (SmsPluginEventHandler::instance()->getDeviceStatus() == true)
		{
			MSG_DEBUG("Device Is Ready");
			return MSG_SUCCESS;
		}
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPlgSubmitRequest(MSG_REQUEST_INFO_S *pReqInfo, bool bReqCb)
{
	// Add Submit SMS into DB
	if ((pReqInfo->msgInfo.msgId == 0) && pReqInfo->msgInfo.msgPort.valid == false) {
		if (SmsPluginStorage::instance()->addMessage(&(pReqInfo->msgInfo)) != MSG_SUCCESS) {
		 	MSG_DEBUG("########  addMessage Fail !!");
			return MSG_ERR_PLUGIN_STORAGE;
		}
	}

	// Check SIM is present or not
	MSG_SIM_STATUS_T simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(MSG_SIM_CHANGED);

	if (simStatus == MSG_SIM_STATUS_NOT_FOUND)
	{
		MSG_DEBUG("SIM is not present..");

		// Update Msg Status
		if (pReqInfo->msgInfo.msgPort.valid == false)
			SmsPluginStorage::instance()->updateSentMsg(&(pReqInfo->msgInfo), MSG_NETWORK_SEND_FAIL);

		return MSG_ERR_NO_SIM;
	}

	SMS_REQUEST_INFO_S request = {};

	request.reqId = pReqInfo->reqId;

	memcpy(&(request.msgInfo), &(pReqInfo->msgInfo), sizeof(MSG_MESSAGE_INFO_S));
	memcpy(&(request.sendOptInfo), &(pReqInfo->sendOptInfo), sizeof(MSG_SENDINGOPT_INFO_S));

	request.bReqCb = bReqCb;

	// Add Request into Queue and Start UA Manger
	SmsPluginUAManager::instance()->addReqEntity(request);

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPlgInitSimMessage()
{
	try
	{
		SmsPluginSimMsg::instance()->initSimMessage();
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


MSG_ERROR_T SmsPlgSaveSimMessage(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = SmsPluginSimMsg::instance()->saveSimMessage(pMsgInfo, pSimIdList);

	return err;
}


MSG_ERROR_T SmsPlgDeleteSimMessage(MSG_SIM_ID_T SimMsgId)
{
	try
	{
		SmsPluginSimMsg::instance()->deleteSimMessage(SimMsgId);
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


MSG_ERROR_T SmsPlgSetReadStatus(MSG_SIM_ID_T SimMsgId)
{
	try
	{
		SmsPluginSimMsg::instance()->setReadStatus(SimMsgId);
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


MSG_ERROR_T SmsPlgSetMemoryStatus(MSG_ERROR_T Error)
{
	int tapiRet = TAPI_API_SUCCESS, reqId = 0;
	int status = TAPI_NETTEXT_PDA_MEMORY_STATUS_AVAILABLE;

	if (Error == MSG_ERR_SIM_STORAGE_FULL || Error == MSG_ERR_MESSAGE_COUNT_FULL)
	{
		status = TAPI_NETTEXT_PDA_MEMORY_STATUS_FULL;
	}

	MSG_DEBUG("Set Status : [%d]", status);

	tapiRet = tel_set_sms_memory_status(status, &reqId);

	if (tapiRet == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! req Id : [%d] #######", reqId);
	}
	else
	{
		MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! req Id : [%d] return : [%d] #######", reqId, tapiRet);
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPlgInitConfigData(MSG_SIM_STATUS_T SimStatus)
{
	try
	{
		SmsPluginSetting::instance()->initConfigData(SimStatus);
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


MSG_ERROR_T SmsPlgSetConfigData(const MSG_SETTING_S *pSetting)
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


MSG_ERROR_T SmsPlgGetConfigData(MSG_SETTING_S *pSetting)
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

