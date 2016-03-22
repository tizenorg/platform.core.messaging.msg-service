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

#include <glib.h>
#include <pthread.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "SmsPluginEventHandler.h"
#include "SmsPluginCbMsgHandler.h"
#include "SmsPluginConcatHandler.h"
#include "SmsPluginWapPushHandler.h"
#include "SmsPluginSatHandler.h"
#include "SmsPluginParamCodec.h"
#include "SmsPluginTpduCodec.h"
#include "SmsPluginTransport.h"
#include "SmsPluginSimMsg.h"
#include "SmsPluginSetting.h"
#include "MsgGconfWrapper.h"
#include "MsgDevicedWrapper.h"
#include "SmsPluginCallback.h"
#include "SmsPluginDSHandler.h"

extern bool isMemAvailable;


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
void TapiEventDeviceReady(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventDeviceReady is called. : noti_id = [%d]", noti_id);

	try {
		/* Call Event Handler */
		SmsPluginEventHandler::instance()->setDeviceStatus(handle);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return;
	}
}

SMS_NETWORK_STATUS_T convertTapiRespToSmsPlugin(int result)
{
	SMS_NETWORK_STATUS_T sentStatus;

	/* Convert TAPI status -> SMS network status */
	switch ((TelSmsResponse_t)result) {
	case TAPI_NETTEXT_SENDSMS_SUCCESS :
		sentStatus = SMS_NETWORK_SEND_SUCCESS;
		break;

	case TAPI_NETTEXT_INVALID_MANDATORY_INFO :
		sentStatus = SMS_NETWORK_SEND_FAIL_MANDATORY_INFO_MISSING;
		break;

	case TAPI_NETTEXT_DESTINAITION_OUTOFSERVICE :
	case TAPI_NETTEXT_TEMPORARY_FAILURE :
	case TAPI_NETTEXT_CONGESTION :
	case TAPI_NETTEXT_RESOURCES_UNAVAILABLE :
	case TAPI_NETTEXT_MESSAGE_NOT_COMPAT_PROTOCOL :
	case TAPI_NETTEXT_NETWORK_OUTOFORDER:
		sentStatus = SMS_NETWORK_SEND_FAIL_TEMPORARY;
		break;

	case TAPI_NETTEXT_MESSAGE_TRANSFER_REJECTED :
		sentStatus = SMS_NETWORK_SEND_FAIL_BY_MO_CONTROL_NOT_ALLOWED;
		break;

	case TAPI_NETTEXT_DEST_ADDRESS_FDN_RESTRICTED :
	case TAPI_NETTEXT_SCADDRESS_FDN_RESTRICTED :
		sentStatus = SMS_NETWORK_SEND_FAIL_FDN_RESTRICED;
		break;
	case TAPI_NETTEXT_ROUTING_NOT_AVAILABLE :
		sentStatus = SMS_NETWORK_SEND_FAIL_NO_ROUTING;
		break;
	default :
		sentStatus = SMS_NETWORK_SEND_FAIL;
		break;
	}

	return sentStatus;
}

void TapiEventSentStatus(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSentStatus is called. : result = [0x%x]", result);

	SMS_NETWORK_STATUS_T sentStatus;

	TelSatMoSmCtrlIndData_t *moCtrlStatus = (TelSatMoSmCtrlIndData_t *)user_data;

	sentStatus = convertTapiRespToSmsPlugin(result);

	if (moCtrlStatus && sentStatus == SMS_NETWORK_SEND_FAIL_BY_MO_CONTROL_NOT_ALLOWED) {
		if (moCtrlStatus->moSmsCtrlResult == TAPI_SAT_CALL_CTRL_R_ALLOWED_WITH_MOD)
			sentStatus = SMS_NETWORK_SEND_FAIL_BY_MO_CONTROL_WITH_MOD;
	}

	if (result != TAPI_NETTEXT_SENDSMS_SUCCESS)
		MSG_INFO("sentStatus:[%d], tapi_result:[0x%x]", sentStatus, result);

	MSG_DEBUG("SMS Network Status = [%d]", sentStatus);

	/* only temporary errors should be returned without calling handleSentStatus() in order to resend sms  */
	if (sentStatus == SMS_NETWORK_SEND_FAIL_TEMPORARY ||
			sentStatus == SMS_NETWORK_SEND_FAIL_BY_MO_CONTROL_WITH_MOD ||
			sentStatus == SMS_NETWORK_SEND_FAIL_FDN_RESTRICED) {
		SmsPluginTransport::instance()->setNetStatus(sentStatus);
		return;
	}

	if (sentStatus == SMS_NETWORK_SEND_FAIL) {
		int svc_type;
		tel_get_property_int(handle, TAPI_PROP_NETWORK_SERVICE_TYPE, &svc_type);
		if (svc_type < TAPI_NETWORK_SERVICE_TYPE_2G)
			sentStatus = SMS_NETWORK_SEND_PENDING;
	}

	/* Convert SMS status -> Messaging network status */
	msg_network_status_t netStatus;

	if (sentStatus == SMS_NETWORK_SEND_SUCCESS) {
		netStatus = MSG_NETWORK_SEND_SUCCESS;
	} else if (sentStatus == SMS_NETWORK_SENDING) {
		netStatus = MSG_NETWORK_SENDING;
	} else if (sentStatus == SMS_NETWORK_SEND_PENDING) {
		netStatus = MSG_NETWORK_SEND_PENDING;
	} else {
		netStatus = MSG_NETWORK_SEND_FAIL;
	}

	try {
		SmsPluginEventHandler::instance()->handleSentStatus(netStatus);

		SmsPluginTransport::instance()->setNetStatus(sentStatus);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return;
	}
}

void TapiEventSatSmsSentStatus(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_INFO("TapiEventSatSmsSentStatus is called. : result = [%d]", result);

	SMS_NETWORK_STATUS_T sentStatus;

	sentStatus = convertTapiRespToSmsPlugin(result);

	MSG_DEBUG("SMS Network Status = [%d]", sentStatus);

	if (sentStatus == SMS_NETWORK_SEND_FAIL && result != TAPI_NETTEXT_DEVICE_FAILURE) {
		int svc_type;
		tel_get_property_int(handle, TAPI_PROP_NETWORK_SERVICE_TYPE, &svc_type);
		if (svc_type < TAPI_NETWORK_SERVICE_TYPE_2G)
			sentStatus = SMS_NETWORK_SEND_PENDING;
	}

	try {
		SmsPluginSatHandler::instance()->ctrlSms(handle, sentStatus);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return;
	}
}

void TapiEventMsgIncoming(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_SEC_DEBUG("TapiEventMsgIncoming is called. noti_id [%s]", noti_id);

	if (data == NULL) {
		MSG_ERR("Error. data is NULL.");
		return;
	}
#if 0
	SmsPluginCbMsgHandler::instance()->handleCbMsg(handle, NULL);
	return;
#endif
	/* make a margin timeout(500ms) till suspending status */
	MsgDisplayLock();

	TelSmsDatapackageInfo_t* pDataPackage = (TelSmsDatapackageInfo_t*)data;

	SMS_TPDU_S tpdu;
	memset(&tpdu, 0x00, sizeof(SMS_TPDU_S));

	/* Decode Incoming Message */
	SmsPluginTpduCodec::decodeTpdu(pDataPackage->szData, pDataPackage->MsgLength, &tpdu);

	/* Print tpdu */
	if (tpdu.tpduType == SMS_TPDU_DELIVER) {
		MSG_DEBUG("############# SMS_TPDU_DELIVER Incoming decoded tpdu values ####################");
		MSG_DEBUG("tpdu.data.deliver.bMoreMsg : %d", tpdu.data.deliver.bMoreMsg);
		MSG_DEBUG("tpdu.data.deliver.bStatusReport : %d", tpdu.data.deliver.bStatusReport);
		MSG_DEBUG("tpdu.data.deliver.bHeaderInd : %d", tpdu.data.deliver.bHeaderInd);
		MSG_DEBUG("tpdu.data.deliver.bReplyPath : %d", tpdu.data.deliver.bReplyPath);
		MSG_DEBUG("tpdu.data.deliver.pid : %d", tpdu.data.deliver.pid);
		MSG_DEBUG("tpdu.data.deliver.dcs.bCompressed : %d", tpdu.data.deliver.dcs.bCompressed);
		MSG_DEBUG("tpdu.data.deliver.dcs.msgClass : %d", tpdu.data.deliver.dcs.msgClass);
		MSG_DEBUG("tpdu.data.deliver.dcs.codingScheme : %d", tpdu.data.deliver.dcs.codingScheme);
		MSG_DEBUG("tpdu.data.deliver.dcs.codingGroup : %d", tpdu.data.deliver.dcs.codingGroup);
		MSG_DEBUG("tpdu.data.deliver.dcs.bIndActive : %d", tpdu.data.deliver.dcs.bIndActive);
		MSG_SEC_DEBUG("tpdu.data.deliver.originAddress.address : %s", tpdu.data.deliver.originAddress.address);
		MSG_DEBUG("tpdu.data.deliver.timeStamp.time : %d/%d/%d %d:%d:%d ", tpdu.data.deliver.timeStamp.time.absolute.year, tpdu.data.deliver.timeStamp.time.absolute.month, tpdu.data.deliver.timeStamp.time.absolute.day,
			tpdu.data.deliver.timeStamp.time.absolute.hour, tpdu.data.deliver.timeStamp.time.absolute.minute, tpdu.data.deliver.timeStamp.time.absolute.second);
		MSG_DEBUG("tpdu.data.deliver.userData.headerCnt : %d", tpdu.data.deliver.userData.headerCnt);
		MSG_DEBUG("tpdu.data.deliver.userData.length : %d", tpdu.data.deliver.userData.length);
		MSG_DEBUG("tpdu.data.deliver.userData.data : %s", tpdu.data.deliver.userData.data);
		MSG_DEBUG("#####################################################");
	} else if (tpdu.tpduType == SMS_TPDU_STATUS_REP) {
		MSG_DEBUG("############# SMS_TPDU_STATUS_REP Incoming decoded tpdu values ####################");
		MSG_DEBUG("tpdu.data.statusRep.msgRef : %d", tpdu.data.statusRep.msgRef);
		MSG_DEBUG("tpdu.data.statusRep.bMoreMsg : %d", tpdu.data.statusRep.bMoreMsg);
		MSG_DEBUG("tpdu.data.statusRep.bStatusReport : %d", tpdu.data.statusRep.bStatusReport);
		MSG_DEBUG("tpdu.data.statusRep.statusRep : %d", tpdu.data.statusRep.bHeaderInd);
		MSG_DEBUG("tpdu.data.statusRep.status : %02x", tpdu.data.statusRep.status);
		MSG_DEBUG("tpdu.data.statusRep.pid : %d", tpdu.data.statusRep.pid);
		MSG_DEBUG("tpdu.data.statusRep.dcs.bCompressed : %d", tpdu.data.statusRep.dcs.bCompressed);
		MSG_DEBUG("tpdu.data.statusRep.dcs.msgClass : %d", tpdu.data.statusRep.dcs.msgClass);
		MSG_DEBUG("tpdu.data.statusRep.dcs.codingScheme : %d", tpdu.data.statusRep.dcs.codingScheme);
		MSG_DEBUG("tpdu.data.statusRep.dcs.codingGroup : %d", tpdu.data.statusRep.dcs.codingGroup);
		MSG_SEC_DEBUG("tpdu.data.statusRep.recipAddress.address : %s", tpdu.data.statusRep.recipAddress.address);
		MSG_DEBUG("tpdu.data.statusRep.timeStamp.time : %d/%d/%d %d:%d:%d ", tpdu.data.statusRep.timeStamp.time.absolute.year, tpdu.data.statusRep.timeStamp.time.absolute.month, tpdu.data.statusRep.timeStamp.time.absolute.day,
			tpdu.data.statusRep.timeStamp.time.absolute.hour, tpdu.data.statusRep.timeStamp.time.absolute.minute, tpdu.data.statusRep.timeStamp.time.absolute.second);
		MSG_DEBUG("tpdu.data.statusRep.dischargeTime.time : %d/%d/%d %d:%d:%d ", tpdu.data.statusRep.dischargeTime.time.absolute.year, tpdu.data.statusRep.dischargeTime.time.absolute.month, tpdu.data.statusRep.dischargeTime.time.absolute.day,
			tpdu.data.statusRep.dischargeTime.time.absolute.hour, tpdu.data.statusRep.dischargeTime.time.absolute.minute, tpdu.data.statusRep.dischargeTime.time.absolute.second);
		MSG_DEBUG("tpdu.data.statusRep.userData.headerCnt : %d", tpdu.data.statusRep.userData.headerCnt);
		MSG_DEBUG("tpdu.data.statusRep.userData.length : %d", tpdu.data.statusRep.userData.length);
		MSG_DEBUG("tpdu.data.statusRep.userData.data : %s", tpdu.data.statusRep.userData.data);
		MSG_DEBUG("#####################################################");
	}

	MsgDisplayUnlock();

	try {
		if (tpdu.tpduType == SMS_TPDU_DELIVER) {
			if (tpdu.data.deliver.dcs.msgClass == SMS_MSG_CLASS_2) {
				/* For GCF test, 34.2.5.3 */
				SmsPluginSimMsg::instance()->setSmsData((const char*)pDataPackage->Sca, (const char *)pDataPackage->szData, pDataPackage->MsgLength);
			}

			if (SmsPluginConcatHandler::instance()->IsConcatMsg(&(tpdu.data.deliver.userData)) == true ||
				SmsPluginWapPushHandler::instance()->IsWapPushMsg(&(tpdu.data.deliver.userData)) == true) {
				/* Call Concat Msg Handler */
				SmsPluginConcatHandler::instance()->handleConcatMsg(handle, &tpdu);
			} else {
				/* Call Event Handler */
				SmsPluginEventHandler::instance()->handleMsgIncoming(handle, &tpdu);
			}
		} else if (tpdu.tpduType == SMS_TPDU_STATUS_REP) {
			/* Call Event Handler */
			SmsPluginEventHandler::instance()->handleMsgIncoming(handle, &tpdu);
		}
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return;
	}
}


void TapiEventCbMsgIncoming(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_SEC_DEBUG("TapiEventCbMsgIncoming is called. noti_id [%s]", noti_id);

	if (data == NULL) {
		MSG_ERR("Error. data is NULL.");
		return;
	}

	TelSmsCbMsg_t *pCbMsg = (TelSmsCbMsg_t*)data;

	try {
		SmsPluginCbMsgHandler::instance()->handleCbMsg(handle, pCbMsg);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return;
	}
}


void TapiEventEtwsMsgIncoming(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_SEC_DEBUG("TapiEventEtwsMsgIncoming is called. noti_id [%s]", noti_id);

	if (data == NULL) {
		MSG_ERR("Error. data is NULL.");
		return;
	}

	TelSmsEtwsMsg_t *pEtwsMsg = (TelSmsEtwsMsg_t*)data;

	try {
		SmsPluginCbMsgHandler::instance()->handleEtwsMsg(handle, pEtwsMsg);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return;
	}
}


void TapiEventDeliveryReportCNF(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventDeliveryReportCNF is called. : result = [%d]", result);

	return;
}


void TapiEventGetSimMsgCnt(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetSimMsgCnt is called.");

	if (result != TAPI_API_SUCCESS || data == NULL) {
		MSG_ERR("Error. data is NULL. result:[0x%x]", result);
		MSG_SIM_COUNT_S simCnt;
		memset(&simCnt, 0x00, sizeof(MSG_SIM_COUNT_S));
		SmsPluginSimMsg::instance()->setSimMsgCntEvent(handle, &simCnt);
		return;
	}

	SmsPluginSimMsg::instance()->setSimMsgCntEvent(handle, (MSG_SIM_COUNT_S *)data);
}


void TapiEventGetSimMsg(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetSimMsg is called.");

	if (result != TAPI_API_SUCCESS || data == NULL) {
		MSG_ERR("Error!! result [0x%x]", result);

		SmsPluginSimMsg::instance()->setSimMsgEvent(handle, NULL, false);

		return;
	}


	TelSmsData_t* pSmsTpdu = (TelSmsData_t*)data;

	int *simIdList = (int *)user_data;

	/* Reading TelSmsData_t */
	MSG_DEBUG("sim index %d", pSmsTpdu->SimIndex);
	MSG_DEBUG("status %d", pSmsTpdu->MsgStatus);
	MSG_DEBUG("sim msg [%s]", pSmsTpdu->SmsData.szData);

	/* Reading TelSmsDatapackageInfo_t */
	if (pSmsTpdu->SmsData.MsgLength > MAX_TPDU_DATA_LEN) {
		MSG_DEBUG("WARNING: tpdu_len > MAX_SMS_TPDU_SIZE [%d] bytes. setting to 0.", pSmsTpdu->SmsData.MsgLength);

		SmsPluginSimMsg::instance()->setSimMsgEvent(handle, NULL, false);

		return;
	}

	SMS_TPDU_S tpdu;

	/* decode Tpdu */
	SmsPluginTpduCodec::decodeTpdu(pSmsTpdu->SmsData.szData, pSmsTpdu->SmsData.MsgLength, &tpdu);

	MSG_DEBUG("Sim Message Type [%d]", tpdu.tpduType);

	bool bRead = false;

	/* set read status */
	if (pSmsTpdu->MsgStatus == TAPI_NETTEXT_STATUS_READ)
		bRead = true;
	else if (pSmsTpdu->MsgStatus == TAPI_NETTEXT_STATUS_UNREAD)
		bRead = false;

	if (tpdu.tpduType == SMS_TPDU_DELIVER) {
		if (tpdu.data.deliver.dcs.codingScheme == SMS_CHARSET_8BIT && tpdu.data.deliver.pid == 0x11) {
			MSG_DEBUG("Unsupported message!!");
			SmsPluginSimMsg::instance()->setSimMsgEvent(handle, NULL, false);
			return;
		}

		MSG_DEBUG("headerCnt [%d]", tpdu.data.deliver.userData.headerCnt);
		for (int i = 0; i < tpdu.data.deliver.userData.headerCnt; i++) {
			/* Handler Concatenated Message */
			if (tpdu.data.deliver.userData.header[i].udhType == SMS_UDH_CONCAT_8BIT ||
				tpdu.data.deliver.userData.header[i].udhType == SMS_UDH_CONCAT_16BIT) {
				SmsPluginConcatHandler::instance()->handleSimConcatMsg(handle, &tpdu, pSmsTpdu->SimIndex, bRead, simIdList);
				return;
			}

			if (tpdu.data.deliver.userData.header[i].udhType == SMS_UDH_SPECIAL_SMS) {
				MSG_DEBUG("Unsupported Special SMS!!");
				SmsPluginSimMsg::instance()->setSimMsgEvent(handle, NULL, false);
				return;
			}
		}
	} else if (tpdu.tpduType == SMS_TPDU_SUBMIT) {
		if (tpdu.data.submit.dcs.codingScheme == SMS_CHARSET_8BIT && tpdu.data.submit.pid == 0x11) {
			MSG_DEBUG("Unsupported message!!");
			SmsPluginSimMsg::instance()->setSimMsgEvent(handle, NULL, false);
			return;
		}
		MSG_DEBUG("headerCnt [%d]", tpdu.data.submit.userData.headerCnt);

		for (int i = 0; i < tpdu.data.submit.userData.headerCnt; i++) {
			/* Handler Concatenated Message */
			if (tpdu.data.submit.userData.header[i].udhType == SMS_UDH_CONCAT_8BIT ||
				tpdu.data.submit.userData.header[i].udhType == SMS_UDH_CONCAT_16BIT) {
				SmsPluginConcatHandler::instance()->handleSimConcatMsg(handle, &tpdu, pSmsTpdu->SimIndex, bRead, simIdList);
				return;
			}
		}
	}

	/* Make MSG_MESSAGE_INFO_S */
	MSG_MESSAGE_INFO_S msgInfo;
	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	/* set storage id */
	msgInfo.storageId = MSG_STORAGE_SIM;

	msgInfo.addressList = NULL;
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

	SmsPluginEventHandler::instance()->convertTpduToMsginfo(&tpdu, &msgInfo);

	msgInfo.sim_idx = SmsPluginDSHandler::instance()->getSimIndex(handle);

	if (tpdu.tpduType == SMS_TPDU_DELIVER && tpdu.data.deliver.dcs.bMWI == true) {
		if (tpdu.data.deliver.pid == 0x20 && tpdu.data.deliver.originAddress.ton == SMS_TON_ALPHANUMERIC) {
			char keyName[MAX_VCONFKEY_NAME_LEN];
			char *voiceNumber = NULL;
			char *voiceAlphaId = NULL;

			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_NUMBER, msgInfo.sim_idx);
			if (MsgSettingGetString(keyName, &voiceNumber) != MSG_SUCCESS) {
				MSG_INFO("MsgSettingGetString() is failed");
			}

			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_ALPHA_ID, msgInfo.sim_idx);
			if (MsgSettingGetString(keyName, &voiceAlphaId) != MSG_SUCCESS) {
				MSG_INFO("MsgSettingGetString() is failed");
			}

			memset(msgInfo.addressList[0].addressVal, 0x00, sizeof(msgInfo.addressList[0].addressVal));
			memset(msgInfo.addressList[0].displayName, 0x00, sizeof(msgInfo.addressList[0].displayName));

			if (voiceNumber) {
				snprintf(msgInfo.addressList[0].addressVal, sizeof(msgInfo.addressList[0].addressVal), "%s", voiceNumber);
				free(voiceNumber);
				voiceNumber = NULL;
			}

			if (voiceAlphaId) {
				snprintf(msgInfo.addressList[0].displayName, sizeof(msgInfo.addressList[0].displayName), "%s", voiceAlphaId);
				free(voiceAlphaId);
				voiceAlphaId = NULL;
			}

			memset(msgInfo.msgText, 0x00, sizeof(msgInfo.msgText));
			snprintf(msgInfo.msgText, sizeof(msgInfo.msgText), "Voice message");
		}
	} else if (tpdu.tpduType == SMS_TPDU_SUBMIT) {
		msgInfo.displayTime =  time(NULL);
	}

	/* set read status */
	msgInfo.bRead = bRead;

	simIdList[0] = pSmsTpdu->SimIndex + 1;
	/* Print MSG_MESSAGE_INFO_S */
	MSG_DEBUG("############# Convert  tpdu values to Message Info values ####################");
	MSG_DEBUG("msgInfo.msgId : %d", msgInfo.msgId);
	MSG_DEBUG("msgInfo.nAddressCnt : %d", msgInfo.nAddressCnt);
	MSG_DEBUG("msgInfo.addressList[0].addressType : %d", msgInfo.addressList[0].addressType);
	MSG_SEC_DEBUG("msgInfo.addressList[0].addressVal : %s", msgInfo.addressList[0].addressVal);
	MSG_SEC_DEBUG("msgInfo.addressList[0].displayName : %s", msgInfo.addressList[0].displayName);
	MSG_DEBUG("msgInfo.priority : %d", msgInfo.priority);
	MSG_DEBUG("msgInfo.bProtected : %d", msgInfo.bProtected);
	MSG_DEBUG("msgInfo.bRead : %d", msgInfo.bRead);
	MSG_DEBUG("msgInfo.bTextSms : %d", msgInfo.bTextSms);
	MSG_DEBUG("msgInfo.direction : %d", msgInfo.direction);
	MSG_DEBUG("msgInfo.msgType.mainType : %d", msgInfo.msgType.mainType);
	MSG_DEBUG("msgInfo.msgType.subType : %d", msgInfo.msgType.subType);
	MSG_DEBUG("msgInfo.msgType.classType : %d", msgInfo.msgType.classType);
	MSG_DEBUG("msgInfo.displayTime : %d", msgInfo.displayTime);
	MSG_DEBUG("msgInfo.dataSize : %d", msgInfo.dataSize);
	if (msgInfo.bTextSms == true)
		MSG_SEC_DEBUG("msgInfo.msgText : %s", msgInfo.msgText);
	else
		MSG_SEC_DEBUG("msgInfo.msgData : %s", msgInfo.msgData);
	MSG_DEBUG("msgInfo.sim_idx : %d", msgInfo.sim_idx);
	MSG_DEBUG("###############################################################");

	/* Call Event Handler */
	SmsPluginSimMsg::instance()->setSimMsgEvent(handle, &msgInfo, true);
}


void TapiEventSaveSimMsg(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSaveSimMsg is called. result [%d]", result);

	int simId = -1;

	if (data != NULL)
		simId = *((int*)data);
	else
		MSG_ERR("Data(SIM Msg ID) is NULL");

	SmsPluginSimMsg::instance()->setSaveSimMsgEvent(handle, simId, result);
}


void TapiEventSaveClass2Msg(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSaveClass2Msg is called. result [%d]", result);

	int simId = -1;

	if (data != NULL) {
		simId = *((int*)data);
		MSG_DEBUG("SIM Msg ID : %d", simId);
	} else {
		MSG_ERR("Data(SIM Msg ID) is NULL");
	}

	MSG_MESSAGE_INFO_S *pMsgInfo = (MSG_MESSAGE_INFO_S *)user_data;

	SmsPluginSimMsg::instance()->setSaveClass2MsgEvent(handle, simId, result, pMsgInfo);

	if (result == TAPI_NETTEXT_SENDSMS_SUCCESS)
		SmsPluginSimMsg::instance()->setSimEvent((msg_sim_id_t)simId, true);
	else
		SmsPluginSimMsg::instance()->setSimEvent((msg_sim_id_t)0, false);

	if (pMsgInfo) {
		if (pMsgInfo->addressList) {
			delete[] pMsgInfo->addressList;
			pMsgInfo->addressList = NULL;
		}
		free(pMsgInfo);
		pMsgInfo = NULL;
	}
}


void TapiEventDeleteSimMsg(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventDeleteSimMsg is called. result [%d]", result);

	if (result != TAPI_API_SUCCESS || data == NULL) {
		MSG_ERR("Error. data is NULL. result:0x%x", result);
		SmsPluginSimMsg::instance()->setDelSimEvent(-1, false);
		return;
	}

	int sim_id = *((int*)data);

	SmsPluginSimMsg::instance()->setDelSimEvent(sim_id, true);
}


void TapiEventSetConfigData(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSetConfigData is called.");

	if (data == NULL) {
		MSG_ERR("Error. data is NULL. result:%d", result);
		return;
	}

	TelSmsSetResponse* responseType = (TelSmsSetResponse*)data;

	MSG_DEBUG("responseType : [%d]", *responseType);

	switch (*responseType) {
	case TAPI_NETTEXT_SETPREFERREDBEARER_RSP :
		MSG_DEBUG("TAPI_NETTEXT_SETPREFERREDBEARER_RSP is called");
		break;

	case TAPI_NETTEXT_SETPARAMETERS_RSP :
		MSG_DEBUG("TAPI_NETTEXT_SETPARAMETERS_RSP is called");
		break;

	case TAPI_NETTEXT_CBSETCONFIG_RSP :
		MSG_DEBUG("TAPI_NETTEXT_CBSETCONFIG_RSP is called");
		break;

	case TAPI_NETTEXT_SETMEMORYSTATUS_RSP :
		MSG_DEBUG("TAPI_NETTEXT_SETMEMORYSTATUS_RSP is called");
		break;

	case TAPI_NETTEXT_SETMESSAGESTATUS_RSP :
		MSG_DEBUG("TAPI_NETTEXT_SETMESSAGESTATUS_RSP is called");
		break;

	default :
		MSG_DEBUG("Unknown Response is called [%d]", *responseType);
		break;
	}

	bool bRet = true;

	MSG_DEBUG("status : [%d]", (TelSmsCause_t)result);

	if ((TelSmsCause_t)result != TAPI_NETTEXT_SUCCESS) bRet = false;

	if (*responseType == TAPI_NETTEXT_SETMESSAGESTATUS_RSP)
		SmsPluginSimMsg::instance()->setSimEvent(0, bRet);
	else
		SmsPluginSetting::instance()->setResultFromSim(bRet);
}


void TapiEventGetParamCnt(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetParamCnt is called.");

	if (result != TAPI_API_SUCCESS || data == NULL) {
		MSG_ERR("Error. data is NULL. result:0x%x", result);
		SmsPluginSetting::instance()->setParamCntEvent(0);
		return;
	}

	int paramCnt = 0;
	paramCnt = *((int*)data);

	SmsPluginSetting::instance()->setParamCntEvent(paramCnt);
}


void TapiEventGetParam(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetConfigData is called.");

	if (result != TAPI_API_SUCCESS || data == NULL) {
		MSG_ERR("Error. data is NULL. result:0x%x", result);
		SmsPluginSetting::instance()->setParamEvent(handle, NULL, -1, false);
		return;
	}

	TelSmsParams_t* smsParam = (TelSmsParams_t*)data;

	int alphaIdLen = 0;
	int addrLen = 0;
	MSG_SMSC_DATA_S smscData = {0, };

	/*Check Alpha ID value*/
	alphaIdLen = smsParam->AlphaIdLen;
	MSG_DEBUG("alphaId_len[%d]", alphaIdLen);

	if (alphaIdLen < 0 || alphaIdLen > SMSC_NAME_MAX) {
		MSG_DEBUG("Wrong Alpha ID Length[%d]", alphaIdLen);

		SmsPluginSetting::instance()->setParamEvent(handle, NULL, -1, false);

		return;
	}


	/*Check Address value*/
	addrLen = smsParam->TpSvcCntrAddr.DialNumLen;

	if (addrLen > SMSC_ADDR_MAX) {
		MSG_DEBUG("addrLen is too long: %d", addrLen);
		SmsPluginSetting::instance()->setParamEvent(handle, NULL, -1, false);
		return;
	} else if (addrLen < 2) {
		MSG_DEBUG("addrLen is too short: %d", addrLen);
		SmsPluginSetting::instance()->setParamEvent(handle, NULL, -1, false);
		return;
	}

	MSG_DEBUG("addrLen : %d", addrLen);

	/*SMSP Parameter Indicator value*/
	MSG_DEBUG("ParamIndicator[%02x]", smsParam->ParamIndicator);

	/*Get SMSC Address*/
	if (0x00 == (0x02 & smsParam->ParamIndicator)) {
		MSG_DEBUG("record index[%d]", (int)smsParam->RecordIndex);

		MSG_DEBUG("TON : %d", smsParam->TpSvcCntrAddr.Ton);
		MSG_DEBUG("NPI : %d", smsParam->TpSvcCntrAddr.Npi);

		smscData.smscAddr.ton = smsParam->TpSvcCntrAddr.Ton;
		smscData.smscAddr.npi = smsParam->TpSvcCntrAddr.Npi;

		SmsPluginParamCodec paramCodec;

		memset(smscData.smscAddr.address, 0x00, SMSC_ADDR_MAX+1);
		paramCodec.decodeSMSC(smsParam->TpSvcCntrAddr.szDiallingNum, addrLen, smscData.smscAddr.ton, smscData.smscAddr.address);

		MSG_SEC_DEBUG("SMSC Address : [%s]", smscData.smscAddr.address);

		memset(smscData.name, 0x00, SMSC_NAME_MAX+1);
		memcpy(smscData.name, smsParam->szAlphaId, alphaIdLen);
		smscData.name[alphaIdLen] = '\0';

		MSG_SEC_DEBUG("SMSC Name : [%s]", smscData.name);
	} else {
		MSG_DEBUG("SMSC Address is not present");

		SmsPluginSetting::instance()->setParamEvent(handle, NULL, -1, false);

		return;
	}

	/*Get the PID value*/
	if (0x00 == (0x04 & smsParam->ParamIndicator)) {
		SMS_PID_T pid = (SMS_PID_T)smsParam->TpProtocolId;

		MSG_DEBUG("smsParam->TpProtocolId : %d", smsParam->TpProtocolId);

		switch (pid) {
		case SMS_PID_NORMAL:
			smscData.pid = MSG_PID_TEXT;
			break;
		case SMS_PID_VOICE:
			smscData.pid = MSG_PID_VOICE;
			break;
		case SMS_PID_TELEX:
			smscData.pid = MSG_PID_FAX;
			break;
		case SMS_PID_x400:
			smscData.pid = MSG_PID_X400;
			break;
		case SMS_PID_ERMES:
			smscData.pid = MSG_PID_ERMES;
			break;
		case SMS_PID_EMAIL:
			smscData.pid = MSG_PID_EMAIL;
			break;
		default:
			smscData.pid = MSG_PID_TEXT;
			break;
		}

		MSG_DEBUG("smscData.pid : %d", smscData.pid);
	} else {
		MSG_DEBUG("PID is not present");
		smscData.pid = MSG_PID_TEXT;
		MSG_DEBUG("MSG_PID_TEXT is inserted to PID");
	}

#if 0
	/*Get the DCS value*/
	if (0x00 == (0x08 & smsParam->ParamIndicator)) {
		smscList.smscData[index].dcs = smsParam->TpDataCodingScheme;
		MSG_DEBUG("dcs : %d", smscList.smscData[index].dcs);
	} else {
		smscList.smscData[index].dcs = MSG_ENCODE_GSM7BIT;
		MSG_DEBUG("DCS is not present");
	}
#endif

	/*Get the ValidityPeriod value*/
	if (0x00 == (0x10 & smsParam->ParamIndicator)) {
		smscData.valPeriod = smsParam->TpValidityPeriod;
		MSG_DEBUG("valPeriod : %d", smscData.valPeriod);
	} else {
		smscData.valPeriod = 0;

		MSG_DEBUG("Validity Period is not present");
	}

	SmsPluginSetting::instance()->setParamEvent(handle, &smscData, (int)smsParam->RecordIndex, true);
}


void TapiEventSetSmscInfo(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSetSmscInfo is called. result=[%d]", result);

	if (result != TAPI_API_SUCCESS)
		SmsPluginSetting::instance()->setResultFromSim(false);
	else
		SmsPluginSetting::instance()->setResultFromSim(true);
}


void TapiEventGetCBConfig(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetCBConfig is called.");

	MSG_CBMSG_OPT_S cbOpt = {0};

	int simIndex = SmsPluginDSHandler::instance()->getSimIndex(handle);

	if (result != TAPI_API_SUCCESS || data == NULL || simIndex == 0) {
		MSG_ERR("Error. data is NULL. result:0x%x, simIndex:%d", result, simIndex);

		SmsPluginSetting::instance()->setCbConfigEvent(handle, NULL, false);

		return;
	}

	TelSmsCbConfig_t* pCBConfig = (TelSmsCbConfig_t*)data;

	cbOpt.bReceive = (bool)pCBConfig->CBEnabled;

	cbOpt.maxSimCnt = TAPI_NETTEXT_SMS_CBMI_LIST_SIZE_MAX;

	cbOpt.simIndex = simIndex;

	MSG_DEBUG("Sim Index [%d], Receive [%d], Max SIM Count [%d]", simIndex, cbOpt.bReceive, cbOpt.maxSimCnt);

	cbOpt.channelData.channelCnt = pCBConfig->MsgIdRangeCount;

	if (cbOpt.channelData.channelCnt > CB_CHANNEL_MAX) {
		MSG_DEBUG("Channel Count [%d] from TAPI is over MAX", cbOpt.channelData.channelCnt);
		cbOpt.channelData.channelCnt = CB_CHANNEL_MAX;
	}

	MSG_DEBUG("Channel Count [%d]", cbOpt.channelData.channelCnt);

	for (int i = 0; i < cbOpt.channelData.channelCnt; i++) {
		cbOpt.channelData.channelInfo[i].bActivate = pCBConfig->MsgIDs[i].Net3gpp.Selected;
		cbOpt.channelData.channelInfo[i].from = pCBConfig->MsgIDs[i].Net3gpp.FromMsgId;
		cbOpt.channelData.channelInfo[i].to = pCBConfig->MsgIDs[i].Net3gpp.ToMsgId;
		memset(cbOpt.channelData.channelInfo[i].name, 0x00, CB_CHANNEL_NAME_MAX+1);

		MSG_DEBUG("Channel FROM [%d], Channel TO [%d] ", cbOpt.channelData.channelInfo[i].from, cbOpt.channelData.channelInfo[i].to);
	}

	SmsPluginSetting::instance()->setCbConfigEvent(handle, &cbOpt, true);
}

void TapiEventSetMailboxInfo(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSetMailboxInfo is called. result = [%d]", result);

	bool bRet = true;
	bool *bShowError = (bool*)user_data;

	if (result != TAPI_SIM_ACCESS_SUCCESS && *bShowError == true)
		bRet = false;

	SmsPluginSetting::instance()->setResultFromSim(bRet);
}

void TapiEventGetMailboxInfo(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetMailboxInfo is called. result=[%d]", result);

	if (result == TAPI_API_SIM_SERVICE_IS_DISABLED) {
		MSG_INFO("result is TAPI_API_SIM_SERVICE_IS_DISABLED");
		char keyName[MAX_VCONFKEY_NAME_LEN];
		int sim_idx = SmsPluginDSHandler::instance()->getSimIndex(handle);
		char *voiceNum = NULL;
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_NUMBER, sim_idx);
		if (MsgSettingGetString(keyName, &voiceNum) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetString() is failed");
		}

		if (voiceNum && strlen(voiceNum)) {
			MSG_DEBUG("Voice mailbox number exist in vconf");
			SmsPluginSetting::instance()->setMailboxInfoEvent(handle, NULL, true, false);
			free(voiceNum);
			voiceNum = NULL;
			return;
		} else {
			SmsPluginSetting::instance()->setMailboxInfoEvent(handle, NULL, false, false);
			if (voiceNum) {
				free(voiceNum);
				voiceNum = NULL;
			}
			return;
		}
	} else if (result != TAPI_SIM_ACCESS_SUCCESS || data == NULL) {
		MSG_ERR("Error. data is NULL.");
		/*If result is not TAPI_SIM_ACCESS_SUCCESS, set bMbdnEnable to false*/
		SmsPluginSetting::instance()->setMailboxInfoEvent(handle, NULL, false, false);
		return;
	}

	TelSimMailboxList_t *list = (TelSimMailboxList_t *)data;
	SMS_SIM_MAILBOX_LIST_S mbList = {0, };

	if (list->count <= 0) {
		MSG_INFO("Mailbox list is empty");
		SmsPluginSetting::instance()->setMailboxInfoEvent(handle, NULL, false, true);
		return;
	}

	mbList.count = list->count;

	for (int i = 0; i < mbList.count; i++) {
		mbList.list[i].b_cphs = list->list[i].b_cphs;
		mbList.list[i].alpha_id_max_len = list->list[i].alpha_id_max_len;
		mbList.list[i].mb_type = list->list[i].mb_type;
		mbList.list[i].profile_num = list->list[i].profile_num;
		mbList.list[i].rec_index = list->list[i].rec_index;
		mbList.list[i].ton = list->list[i].ton;
		mbList.list[i].npi = list->list[i].npi;
		snprintf(mbList.list[i].alpha_id, sizeof(mbList.list[i].alpha_id), "%s", list->list[i].alpha_id);
		snprintf(mbList.list[i].num, sizeof(mbList.list[i].num), "%s", list->list[i].num);
		mbList.list[i].cc_id = list->list[i].cc_id;
		mbList.list[i].ext1_id = list->list[i].ext1_id;
		mbList.list[i].num_len = strlen(mbList.list[i].num);
	}

	SmsPluginSetting::instance()->setMailboxInfoEvent(handle, &mbList, true, true);
}

void TapiEventSetMwiInfo(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSetMwiInfo is called. result = [%d]", result);
}

void TapiEventGetMwiInfo(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetMwiInfo is called.");

	if (result != TAPI_SIM_ACCESS_SUCCESS || data == NULL) {
		MSG_ERR("Error. data is NULL. result:0x%x", result);
		SmsPluginSetting::instance()->setMwiInfoEvent(handle, NULL, false);

		return;
	}

	TelSimMessageWaitingResp_t *MwiInfo = (TelSimMessageWaitingResp_t *)data;
	SMS_SIM_MWI_INFO_S simMwiInfo = {0, };

	memcpy(&simMwiInfo, MwiInfo, sizeof(SMS_SIM_MWI_INFO_S));

	SmsPluginSetting::instance()->setMwiInfoEvent(handle, &simMwiInfo, true);
}

void TapiEventGetMsisdnInfo(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetMsisdnInfo is called.");

	bool bRet = false;

	if (result != TAPI_SIM_ACCESS_SUCCESS || data == NULL) {
		MSG_ERR("Error. data is NULL. result:0x%x", result);
		SmsPluginSetting::instance()->setResultFromSim(bRet);
		return;
	}

	TelSimMsisdnList_t *list = (TelSimMsisdnList_t *)data;

	for (int i = 0; i < list->count; i++) {
		if (list->list[i].num[0] != '\0') {
			char keyName[MAX_VCONFKEY_NAME_LEN];
			int simIndex = SmsPluginDSHandler::instance()->getSimIndex(handle);

			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_MSISDN, simIndex);

			if (MsgSettingSetString(keyName, list->list[i].num) == MSG_SUCCESS) {
				MSG_SEC_INFO("Get MSISDN from SIM : [%s]", list->list[i].num);
				bRet = true;
			} else {
				MSG_DEBUG("Getting MSISDN is failed!");
			}
			break;
		}
	}

	SmsPluginSetting::instance()->setResultFromSim(bRet);
}

void TapiEventGetSimServiceTable(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetSimServiceTable is called.");

    TelSimAccessResult_t access_rt = (TelSimAccessResult_t)result;
    TelSimServiceTable_t *svct = (TelSimServiceTable_t *)data;

	bool bRet = true;

	if (access_rt != TAPI_SIM_ACCESS_SUCCESS || svct == NULL) {
		MSG_ERR("Error. data is NULL and access_rt [%d] failed", access_rt);
		SmsPluginSetting::instance()->setResultFromSim(false);
		return;
	}

	msg_error_t err = MSG_SUCCESS;

	char sstKey[128];
	char moCtrlKey[128];

	int simIndex = SmsPluginDSHandler::instance()->getSimIndex(handle);

	memset(sstKey, 0x00, sizeof(sstKey));
	snprintf(sstKey, sizeof(sstKey), "%s/%d", MSG_SIM_SERVICE_TABLE, simIndex);

	memset(moCtrlKey, 0x00, sizeof(moCtrlKey));
	snprintf(moCtrlKey, sizeof(moCtrlKey), "%s/%d", MSG_SIM_MO_CONTROL, simIndex);

	if (svct->sim_type == TAPI_SIM_CARD_TYPE_GSM) {
		if (svct->table.sst.service[TAPI_SIM_SST_SMS] == 1) {
			err = MsgSettingSetBool(sstKey, true);
		} else {
			err = MsgSettingSetBool(sstKey, false);
		}
		MSG_DEBUG("Setting result = [%d]", err);

		if (svct->table.sst.service[TAPI_SIM_SST_MO_SMS_CTRL_BY_SIM] == 1) {
			err = MsgSettingSetBool(moCtrlKey, true);
		} else {
			err = MsgSettingSetBool(moCtrlKey, false);
		}
		MSG_DEBUG("Setting result = [%d]", err);

	} else if (svct->sim_type == TAPI_SIM_CARD_TYPE_USIM) {
		if (svct->table.ust.service[TAPI_SIM_UST_SMS] == 1) {
			err = MsgSettingSetBool(sstKey, true);
		} else {
			err = MsgSettingSetBool(sstKey, false);
		}
		MSG_DEBUG("Setting result = [%d]", err);

		if (svct->table.ust.service[TAPI_SIM_UST_MO_SMS_CTRL] == 1) {
			err = MsgSettingSetBool(moCtrlKey, true);
		} else {
			err = MsgSettingSetBool(moCtrlKey, false);
		}
		MSG_DEBUG("Setting result = [%d]", err);

	} else {
		MSG_DEBUG("Unknown SIM type value");
	}

	SmsPluginSetting::instance()->setResultFromSim(bRet);
}

void TapiEventSatSmsRefresh(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSatSmsRefresh is called.");

	if (result != TAPI_API_SUCCESS || data == NULL) {
		MSG_ERR("Error. data is NULL. result:0x%x", result);
		return;
	}

	try {
		SmsPluginSatHandler::instance()->refreshSms(handle, data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return;
	}
}


void TapiEventSatSendSms(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSatSendSms is called.");

	if (data == NULL) {
		MSG_ERR("Error. data is NULL.");
		return;
	}

	try {
		SmsPluginSatHandler::instance()->sendSms(handle, data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return;
	}
}


void TapiEventSatMoSmsCtrl(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSatMoSmsCtrl is called.");

	if (data == NULL) {
		MSG_ERR("Error. data is NULL.");
		return;
	}

	try {
		SmsPluginSatHandler::instance()->ctrlSms(handle, data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return;
	}
}

void TapiEventMemoryStatus(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("Tapi result is [%d]", result);
	if (result == TAPI_API_SUCCESS)
		isMemAvailable = true;
}

void TapiEventSetMsgStatus(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSetMsgStatus is called. result [%d]", result);

	if (result != TAPI_API_SUCCESS || data == NULL) {
		MSG_ERR("Error. data is NULL. result:0x%x", result);
		SmsPluginSimMsg::instance()->setSimEvent((msg_sim_id_t)0, false);
		return;
	}

	msg_sim_id_t sim_id = *((msg_sim_id_t *)user_data);

	SmsPluginSimMsg::instance()->setSimEvent(sim_id, true);
}


void TapiEventGetMeImei(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_SEC_DEBUG("TapiEventGetMeImei is called. result [%d]", result);

	if (result != TAPI_API_SUCCESS || data == NULL) {
		MSG_ERR("Error. data is NULL. result:0x%x", result);
		SmsPluginSetting::instance()->setResultImei(false, NULL);
		return;
	}

	char *tmpImei = (char *)data;

	SmsPluginSetting::instance()->setResultImei(true, tmpImei);
}


void TapiEventSimStatusChange(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSimStatusChange is called.");

	if (data == NULL) {
		MSG_ERR("Error. data is NULL.");
		return;
	}

	 int status = *(int *)data;

	 MSG_DEBUG("SIM Status [%d]", status);

	 if (status == TAPI_SIM_STATUS_SIM_INIT_COMPLETED) {
		 MSG_INFO("SIM Initialize by sim status change callback");
		 SmsPluginSetting::instance()->setSimChangeStatus(handle, false);
	 }
}

void TapiEventNetworkStatusChange(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventNetworkStatusChange is called.");

	if (data == NULL) {
		MSG_ERR("Error. data is NULL.");
		return;
	}

	TelNetworkServiceType_t *type = (TelNetworkServiceType_t *)data;

	MSG_INFO("network status type [%d]", *type);

	if (*type > TAPI_NETWORK_SERVICE_TYPE_SEARCH) {
		/* Call Event Handler */
		SmsPluginEventHandler::instance()->handleResendMessage();
	}
}

void TapiEventSimRefreshed(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSimRefreshed is called.");

	SmsPluginSetting::instance()->SimRefreshCb(handle);
}

/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginCallback - Member Functions
==================================================================================================*/
SmsPluginCallback* SmsPluginCallback::pInstance = NULL;


SmsPluginCallback::SmsPluginCallback()
{
}


SmsPluginCallback::~SmsPluginCallback()
{
	if (pInstance != NULL) {
		delete pInstance;
		pInstance = NULL;
	}
}


SmsPluginCallback* SmsPluginCallback::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginCallback();

	return pInstance;
}


void SmsPluginCallback::registerEvent()
{
	TapiHandle *pTapiHandle;

	int count = SmsPluginDSHandler::instance()->getTelHandleCount();

	for (int i = 1; i <= count; ++i) {
		pTapiHandle = SmsPluginDSHandler::instance()->getTelHandle(i);
/*		int simIndex = SmsPluginDSHandler::instance()->getSimIndex(pTapiHandle); */

		if (tel_register_noti_event(pTapiHandle, TAPI_NOTI_SMS_DEVICE_READY, TapiEventDeviceReady, NULL) != TAPI_API_SUCCESS)
			MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_NOTI_SMS_DEVICE_READY);
		if (tel_register_noti_event(pTapiHandle, TAPI_NOTI_SMS_INCOM_MSG, TapiEventMsgIncoming, NULL) != TAPI_API_SUCCESS)
			MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_NOTI_SMS_INCOM_MSG);
		if (tel_register_noti_event(pTapiHandle, TAPI_NOTI_SMS_CB_INCOM_MSG, TapiEventCbMsgIncoming, NULL) != TAPI_API_SUCCESS)
			MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_NOTI_SMS_CB_INCOM_MSG);
		if (tel_register_noti_event(pTapiHandle, TAPI_NOTI_SMS_ETWS_INCOM_MSG, TapiEventEtwsMsgIncoming, NULL) != TAPI_API_SUCCESS)
			MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_NOTI_SMS_ETWS_INCOM_MSG);
		if (tel_register_noti_event(pTapiHandle, TAPI_NOTI_SAT_SEND_SMS, TapiEventSatSendSms, NULL) != TAPI_API_SUCCESS)
			MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_NOTI_SAT_SEND_SMS);
		if (tel_register_noti_event(pTapiHandle, TAPI_NOTI_SAT_MO_SM_CONTROL_RESULT, TapiEventSatMoSmsCtrl, NULL) != TAPI_API_SUCCESS)
			MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_NOTI_SAT_MO_SM_CONTROL_RESULT);
		if (tel_register_noti_event(pTapiHandle, TAPI_NOTI_SIM_STATUS, TapiEventSimStatusChange, NULL) != TAPI_API_SUCCESS)
			MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_NOTI_SIM_STATUS);
		if (tel_register_noti_event(pTapiHandle, TAPI_PROP_NETWORK_SERVICE_TYPE, TapiEventNetworkStatusChange, NULL) != TAPI_API_SUCCESS)
			MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_PROP_NETWORK_SERVICE_TYPE);
		if (tel_register_noti_event(pTapiHandle, TAPI_NOTI_SIM_REFRESHED, TapiEventSimRefreshed, NULL) != TAPI_API_SUCCESS)
			MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_NOTI_SIM_REFRESHED);
/*		if (tel_register_noti_event(pTapiHandle, TAPI_NOTI_SAT_REFRESH, TapiEventSatSmsRefresh, NULL) != TAPI_API_SUCCESS) */
/*			MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_NOTI_SAT_REFRESH); */
/*		if (tel_register_noti_event(pTapiHandle, TAPI_NOTI_SAT_MO_SMS_CTRL, TapiEventSatMoSmsCtrl, NULL) != TAPI_API_SUCCESS) */
/*			MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_NOTI_SAT_MO_SMS_CTRL); */
	}
}


void SmsPluginCallback::deRegisterEvent()
{
}

