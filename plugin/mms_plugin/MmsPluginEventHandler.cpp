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
#include <stdlib.h>
#include <ITapiCall.h>
#include <ITapiNetwork.h>
#include <tapi_common.h>

#include "MmsPluginTypes.h"
#include "MmsPluginDebug.h"
#include "MmsPluginEventHandler.h"
#include "MmsPluginInternal.h"
#include "MsgGconfWrapper.h"

/*==================================================================================================
                                     IMPLEMENTATION OF MmsPluginEventHandler - Member Functions
==================================================================================================*/

typedef struct {
	int count;
	TapiHandle *handle[MAX_TELEPHONY_HANDLE_CNT]; /* max is 3 */
} SMS_TELEPHONY_HANDLE_LIST_S;

SMS_TELEPHONY_HANDLE_LIST_S handle_list;

MmsPluginEventHandler *MmsPluginEventHandler::pInstance = NULL;


MmsPluginEventHandler::MmsPluginEventHandler()
{
	memset(&listener, 0x00, sizeof(MSG_PLUGIN_LISTENER_S));
}


MmsPluginEventHandler::~MmsPluginEventHandler()
{
}


MmsPluginEventHandler *MmsPluginEventHandler::instance()
{
	if (!pInstance)
		pInstance = new MmsPluginEventHandler();

	return pInstance;
}


void MmsPluginEventHandler::registerListener(MSG_PLUGIN_LISTENER_S *pListener)
{
	listener = *pListener;
}

int MmsPluginEventHandler::initTelHandle()
{
	int cnt = 0;
	memset(&handle_list, 0x00, sizeof(handle_list));

	char **cp_list = NULL;
	cp_list = tel_get_cp_name_list();

	if (!cp_list) {
		MSG_FATAL("tel_get_cp_name_list returns null");
		goto FINISH;
	}

	while (cp_list[cnt] && cnt < MAX_TELEPHONY_HANDLE_CNT) {
		MSG_SEC_INFO("cp_list[%d]:[%s]", cnt, cp_list[cnt]);
		handle_list.handle[cnt]= tel_init(cp_list[cnt]);
		cnt++;
	}

	g_strfreev(cp_list);
	cp_list = NULL;

FINISH:
	handle_list.count = cnt;
	return cnt;
}

void MmsPluginEventHandler::deinitTelHandle()
{
	int ret = 0;

	for (int i = 0; i < handle_list.count; i++) {
		ret = tel_deinit(handle_list.handle[i]);
		MSG_DEBUG("tel_deinit ret=[%d]", ret);
		handle_list.handle[i] = NULL;
	}

	handle_list.count = 0;
	return;
}

TapiHandle *MmsPluginEventHandler::getTelHandle(int sim_idx)
{
	if (sim_idx > 0 && sim_idx < MAX_TELEPHONY_HANDLE_CNT) {
		return handle_list.handle[sim_idx-1];
	} else {
		int SIM_Status = 0;
		if (MsgSettingGetInt(VCONFKEY_TELEPHONY_SIM_SLOT, &SIM_Status) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}
		if (SIM_Status == 1) {
			return handle_list.handle[0];
		}

		if (MsgSettingGetInt(VCONFKEY_TELEPHONY_SIM_SLOT2, &SIM_Status) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}
		if (SIM_Status == 1) {
			return handle_list.handle[1];
		}
	}

	return handle_list.handle[handle_list.count - 1];
}

void MmsPluginEventHandler::handleMmsReceivedData(mmsTranQEntity *pRequest, char *pRetrievedFilePath)
{
	MSG_MESSAGE_INFO_S msgInfo = {0, };
	msgInfo.sim_idx = pRequest->simId;

	switch (pRequest->eMmsPduType) {
	/* received data is send-conf */
	case eMMS_SEND_CONF:
		MmsPluginInternal::instance()->processSendConf(&msgInfo, pRequest);

		/* callback to MSG FW */
		listener.pfMmsConfIncomingCb(&msgInfo, &pRequest->reqID);

		if (remove(pRetrievedFilePath) != 0)
			MSG_DEBUG("remove fail");
		break;

	/* received data is retrieve-conf */
	case eMMS_RETRIEVE_AUTO_CONF:
	case eMMS_RETRIEVE_MANUAL_CONF:
		MmsPluginInternal::instance()->processRetrieveConf(&msgInfo, pRequest, pRetrievedFilePath);

		/* callback to MSG FW */
		listener.pfMmsConfIncomingCb(&msgInfo, &pRequest->reqID);

		if (msgInfo.addressList)
			delete [] msgInfo.addressList;
		break;

	case eMMS_FORWARD_CONF:
		MmsPluginInternal::instance()->processForwardConf(&msgInfo, pRequest);
		break;

	case eMMS_READREPORT_CONF:
		if (remove(pRetrievedFilePath) != 0)
			MSG_DEBUG("remove fail");
		break;
	default:
		break;
	}
}


void MmsPluginEventHandler::handleMmsError(mmsTranQEntity *pRequest)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	MSG_MESSAGE_INFO_S msgInfo;
	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	MSG_DEBUG("pRequest->msgId [%d]", pRequest->msgId);

	time_t curTime;
	curTime = time(NULL);

	msgInfo.displayTime = curTime;
	msgInfo.sim_idx = pRequest->simId;

	switch (pRequest->eMmsPduType) {
	case eMMS_SEND_REQ:
	case eMMS_SEND_CONF:
		msgInfo.msgId = pRequest->msgId;
		/* Set only changed members */
		msgInfo.msgType.mainType = MSG_MMS_TYPE;

		if (pRequest->eMmsPduType == eMMS_SEND_REQ)
			msgInfo.msgType.subType = MSG_SENDREQ_MMS;
		else
			msgInfo.msgType.subType = MSG_SENDCONF_MMS;

		msgInfo.networkStatus = MSG_NETWORK_SEND_FAIL;

		msgInfo.folderId = MSG_OUTBOX_ID;

		break;

	case eMMS_RETRIEVE_AUTO:
	case eMMS_RETRIEVE_AUTO_CONF:
		msgInfo.msgId = pRequest->msgId;
		/* Set only changed members */
		msgInfo.msgType.mainType = MSG_MMS_TYPE;
		msgInfo.msgType.subType = MSG_RETRIEVE_AUTOCONF_MMS;

		msgInfo.networkStatus = MSG_NETWORK_RETRIEVE_FAIL;
		msgInfo.folderId = MSG_INBOX_ID;

		break;

	case eMMS_RETRIEVE_MANUAL:
	case eMMS_RETRIEVE_MANUAL_CONF:
		msgInfo.msgId = pRequest->msgId;
		/* Set only changed members */
		msgInfo.msgType.mainType = MSG_MMS_TYPE;
		msgInfo.msgType.subType = MSG_RETRIEVE_MANUALCONF_MMS;

		msgInfo.networkStatus = MSG_NETWORK_RETRIEVE_FAIL;
		msgInfo.folderId = MSG_INBOX_ID;

		break;

	default:
		MSG_WARN("No matching MMS pdu type [%d]", pRequest->eMmsPduType);
		return;
	}

	int dnet_state = 0;
	if (msgInfo.sim_idx == 1) {
		if (MsgSettingGetInt(VCONFKEY_DNET_STATE, &dnet_state) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}
	} else if (msgInfo.sim_idx == 2) {
		if (MsgSettingGetInt(VCONFKEY_DNET_STATE2, &dnet_state) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}
	}

	int net_cell_type = 0;

	initTelHandle();

	TapiHandle *handle = getTelHandle(msgInfo.sim_idx);
	tel_get_property_int(handle, TAPI_PROP_NETWORK_SERVICE_TYPE, &net_cell_type);

	deinitTelHandle();

	bool data_enable = FALSE;
	int call_status = 0;
	if (MsgSettingGetInt(MSG_MESSAGE_DURING_CALL, &call_status) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetInt() is failed");
	}

	if (MsgSettingGetBool(VCONFKEY_3G_ENABLE, &data_enable) != MSG_SUCCESS)
		MSG_INFO("MsgSettingGetBool() is failed");

	MSG_INFO("Call[%d], Data[%d], SubType[%d]", call_status, data_enable, msgInfo.msgType.subType);
	MSG_INFO("sim_idx[%d], dnet_state[%d], net_cell_type[%d]", msgInfo.sim_idx, dnet_state, net_cell_type);

	if (pRequest->eMmsTransactionStatus == eMMS_CM_DISCONNECTED || ((call_status || net_cell_type <= TAPI_NETWORK_SERVICE_TYPE_SEARCH || dnet_state <= VCONFKEY_DNET_OFF) && data_enable)) {
		MSG_INFO("MMS network status goes to pending for subtype [%d]", msgInfo.msgType.subType);
		if (msgInfo.msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || msgInfo.msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS)
			msgInfo.networkStatus = MSG_NETWORK_RETRIEVE_PENDING;
		else if (msgInfo.msgType.subType == MSG_SENDREQ_MMS)
			msgInfo.networkStatus = MSG_NETWORK_SEND_PENDING;
	}

	err = listener.pfMmsConfIncomingCb(&msgInfo, &pRequest->reqID);

	MSG_DEBUG("Error value of MsgMmsConfIncomingListner [%d]", err);

	MSG_END();
}

