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
#include "SmsPluginDSHandler.h"
#include "MsgGconfWrapper.h"

extern "C"
{
	#include <tapi_common.h>
	#include <TelNetwork.h>
	#include <ITapiNetwork.h>
	#include <ITapiSim.h>
#if 0
	#include <telephony_common.h>
	#include <telephony_sim.h>
#endif
}

/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginCbMsgHandler - Member Functions
==================================================================================================*/
SmsPluginDSHandler* SmsPluginDSHandler::pInstance = NULL;


SmsPluginDSHandler::SmsPluginDSHandler()
{
}


SmsPluginDSHandler::~SmsPluginDSHandler()
{
}


SmsPluginDSHandler* SmsPluginDSHandler::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginDSHandler();

	return pInstance;
}

int SmsPluginDSHandler::initTelHandle()
{
	int cnt = 0;
	cp_list = tel_get_cp_name_list();

	if (!cp_list) {
		MSG_FATAL("tel_get_cp_name_list returns null");
		goto FINISH;
	}

	while(cp_list[cnt] && cnt < MAX_TELEPHONY_HANDLE_CNT)
	{
		MSG_SEC_INFO("cp_list[%d]:[%s]", cnt, cp_list[cnt]);
		handle_list.handle[cnt]= tel_init(cp_list[cnt]);
		cnt++;
	}

	g_strfreev(cp_list);

FINISH:
	handle_list.count = cnt;
	return cnt;
}

void SmsPluginDSHandler::deinitTelHandle()
{
	int ret = 0;

	for (int i = 0; i < handle_list.count; i++)
	{
		ret = tel_deinit(handle_list.handle[i]);
		MSG_DEBUG("tel_deinit ret=[%d]", ret);
		handle_list.handle[i] = NULL;
	}

	handle_list.count = 0;
	cp_list = NULL;

	return;
}

struct tapi_handle *SmsPluginDSHandler::getTelHandle(int sim_idx)
{
	if (sim_idx > 0 && sim_idx < MAX_TELEPHONY_HANDLE_CNT)
		return handle_list.handle[sim_idx-1];
	else {
		int SIM_Status = 0;
		SIM_Status = MsgSettingGetInt(VCONFKEY_TELEPHONY_SIM_SLOT);
		if (SIM_Status == 1) {
			return handle_list.handle[0];
		}

		SIM_Status = MsgSettingGetInt(VCONFKEY_TELEPHONY_SIM_SLOT2);
		if (SIM_Status == 1) {
			return handle_list.handle[1];
		}
	}

	return handle_list.handle[handle_list.count - 1];
}

int SmsPluginDSHandler::getSimIndex(struct tapi_handle *handle)
{
	for(int index=0; index < handle_list.count; ++index)
	{
		if(handle_list.handle[index] == handle)
			return index+1;
	}
	return 0;
}


void SmsPluginDSHandler::getDefaultNetworkSimId(int *simId)
{
	TelNetworkDefaultDataSubs_t defaultSimId;

	int tapi_ret = TAPI_API_SUCCESS;

	if (handle_list.count == 1) {
		*simId = 1;
		return;
	}

	tapi_ret = tel_get_network_default_data_subscription(handle_list.handle[0], &defaultSimId);

	MSG_INFO("Default network subscription = [SIM %d]", (int)defaultSimId+1);

	if (tapi_ret != TAPI_API_SUCCESS) {
		*simId = 0;
		THROW(MsgException::SMS_PLG_ERROR, "########  tel_get_network_default_data_subscription Fail !!! return : %d #######", tapi_ret);
	} else {
		*simId = (int)defaultSimId + 1;
	}
}


int SmsPluginDSHandler::getTelHandleCount()
{
	return handle_list.count;
}

int SmsPluginDSHandler::getActiveSimCount()
{
	int active_count = 0;
	int sim_status = VCONFKEY_TELEPHONY_SIM_UNKNOWN;

	sim_status = MsgSettingGetInt(VCONFKEY_TELEPHONY_SIM_SLOT);
	MSG_DEBUG("sim1 status : %d", sim_status);
	if (sim_status == VCONFKEY_TELEPHONY_SIM_INSERTED)
		active_count++;

	sim_status = VCONFKEY_TELEPHONY_SIM_UNKNOWN;
	sim_status = MsgSettingGetInt(VCONFKEY_TELEPHONY_SIM_SLOT2);

	MSG_DEBUG("sim2 status : %d", sim_status);
	if (sim_status == VCONFKEY_TELEPHONY_SIM_INSERTED)
		active_count++;

	MSG_DEBUG("active sim count : %d", active_count);

	return active_count;
}


int SmsPluginDSHandler::getSubscriberId(unsigned int simIndex, char **subscriber_id)
{
#if 0
	if (simIndex <= 0 || simIndex > (unsigned int)handle_list.count) {
		MSG_DEBUG("Invalid SIM index");
		return MSG_ERR_INVALID_PARAMETER;
	}

	int tel_ret = TELEPHONY_ERROR_NONE;
	telephony_handle_list_s capi_tel_handle_list;

	tel_ret = telephony_init(&capi_tel_handle_list);
	if (tel_ret != TELEPHONY_ERROR_NONE) {
		MSG_DEBUG("Initialize failed!!!");
		return MSG_ERR_PLUGIN_TAPIINIT;
	}

	tel_ret = telephony_sim_get_subscriber_id(capi_tel_handle_list.handle[simIndex-1], subscriber_id);
	if (tel_ret != TELEPHONY_ERROR_NONE) {
		MSG_DEBUG("telephony_sim_get_subscriber_id() failed!!! [%d]", tel_ret);
	} else {
		MSG_DEBUG("Subscriber ID is [%s]", *subscriber_id);
	}

	tel_ret = telephony_deinit(&capi_tel_handle_list);
	if (tel_ret != TELEPHONY_ERROR_NONE) {
		MSG_DEBUG("Deinitialize failed!!!");
	}

#endif
    return MSG_SUCCESS;
}
