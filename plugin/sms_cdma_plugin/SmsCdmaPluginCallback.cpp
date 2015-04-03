/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
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
#include "MsgGconfWrapper.h"
#include "SmsCdmaPluginEventHandler.h"
#include "SmsCdmaPluginParamCodec.h"
#include "SmsCdmaPluginCodec.h"
#include "SmsCdmaPluginTransport.h"
#include "SmsCdmaPluginSetting.h"
#include "SmsCdmaPluginCallback.h"


extern struct tapi_handle *pTapiHandle;
extern bool isMemAvailable;

void _dnet_state_changed_cb(keynode_t *key, void* data);
void _TapiMdnChangedCb(keynode_t *key, void *data);

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
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
	if (tel_register_noti_event(pTapiHandle, TAPI_NOTI_SMS_DEVICE_READY, TapiEventDeviceReady, NULL) != TAPI_API_SUCCESS)
		MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_NOTI_SMS_DEVICE_READY);
	if (tel_register_noti_event(pTapiHandle, TAPI_NOTI_SMS_INCOM_MSG, TapiEventMsgIncoming, NULL) != TAPI_API_SUCCESS)
		MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_NOTI_SMS_INCOM_MSG);
	if (tel_register_noti_event(pTapiHandle, TAPI_PROP_NETWORK_SERVICE_TYPE, TapiEventNetworkStatusChange, NULL) != TAPI_API_SUCCESS)
		MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_PROP_NETWORK_SERVICE_TYPE);
	if (tel_register_noti_event(pTapiHandle, TAPI_NOTI_SIM_REFRESHED, TapiEventSimRefreshed, NULL) != TAPI_API_SUCCESS)
		MSG_DEBUG("tel_register_noti_event is failed : [%s]", TAPI_NOTI_SIM_REFRESHED);

	MsgSettingRegVconfCBCommon(VCONFKEY_DNET_STATE, _dnet_state_changed_cb);
//	MsgSettingRegVconfCBCommon(VCONFKEY_TELEPHONY_MDN, _TapiMdnChangedCb);
}


void SmsPluginCallback::deRegisterEvent()
{


}


void TapiEventDeviceReady(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventDeviceReady is called. : noti_id = [%d]", noti_id);

	try
	{
		// Call Event Handler
		SmsPluginEventHandler::instance()->setDeviceStatus();

		if (SmsPluginEventHandler::instance()->getNeedInitConfig() == true) {
			SmsPluginEventHandler::instance()->setNeedInitConfig(false);
			SmsPluginSetting::instance()->SimRefreshCb();
		}
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return;
	}

}


void TapiEventMsgIncoming(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_WARN("TapiEventMsgIncoming is called. noti_id [%s]", noti_id);

	if (data == NULL) {
		MSG_DEBUG("Error. evt->pData is NULL.");
		return;
	}

	TelSmsDatapackageInfo_t* pDataPackage = (TelSmsDatapackageInfo_t*)data;

	MSG_DEBUG("pDataPackage->format = [%d]", pDataPackage->format);
	MSG_DEBUG("pDataPackage->Sca = [%s]", pDataPackage->Sca);
	MSG_DEBUG("pDataPackage->MsgLength = [%d]", pDataPackage->MsgLength);
	MSG_DEBUG("pDataPackage->szData = ");
	for (int i = 0; i < pDataPackage->MsgLength; i++) {
		MSG_DEBUG("[%02x]", pDataPackage->szData[i]);
	}

	char tpduTmp[(pDataPackage->MsgLength*2)+1];
	memset(tpduTmp, 0x00, sizeof(tpduTmp));
	for (int i = 0; i < pDataPackage->MsgLength; i++) {
		snprintf(tpduTmp+(i*2), sizeof(tpduTmp)-(i*2), "%02X", pDataPackage->szData[i]);
	}
	MSG_WARN("[%s]", tpduTmp);

	sms_trans_msg_s	sms_trans_msg;
	memset(&sms_trans_msg, 0x00, sizeof(sms_trans_msg_s));

	if (pDataPackage->format == (TelSmsNetType_t)0x03) {				// voice mail notification
		sms_trans_msg.data.p2p_msg.telesvc_msg.type = SMS_TYPE_DELIVER;
		sms_trans_msg.data.p2p_msg.telesvc_id = SMS_TRANS_TELESVC_VMN_95;

		int num_msg = 0;
		for (int i = 0; i < pDataPackage->MsgLength; i++) {
			num_msg*=256;
			num_msg+=pDataPackage->szData[i];
		}

		sms_trans_msg.data.p2p_msg.telesvc_msg.data.deliver.num_msg = num_msg;

		char *voiceNumber = MsgSettingGetString(VOICEMAIL_NUMBER);
		if (voiceNumber) {
			snprintf(sms_trans_msg.data.p2p_msg.address.szData, sizeof(sms_trans_msg.data.p2p_msg.address.szData), "%s", voiceNumber);
			free(voiceNumber);
			voiceNumber = NULL;
		}
		sms_trans_msg.data.p2p_msg.address.addr_len = strlen(sms_trans_msg.data.p2p_msg.address.szData);
	} else {
		bool bInvalid = SmsPluginMsgCodec::checkInvalidPDU(pDataPackage->szData, pDataPackage->MsgLength);
		if (bInvalid == true) {
			// Decode Incoming Message
			SmsPluginMsgCodec::decodeMsg(pDataPackage->szData, pDataPackage->MsgLength, &sms_trans_msg);

			if (sms_trans_msg.data.cb_msg.telesvc_msg.data.deliver.cmas_data.is_wrong_recode_type) {
				MSG_WARN("Invalid CMAS Record Type");
				return;
			}
		}
		else {
			MSG_WARN("Invalid PDU");
			return;
		}
	}

	/// Print tpdu
	if (sms_trans_msg.type == SMS_TRANS_P2P_MSG) {
		MSG_DEBUG("############# SMS_TRANS_P2P_MSG Incoming decoded tpdu values ####################");
		MSG_DEBUG("------------------------------ transport layer data -----------------------------");
		MSG_DEBUG("sms_trans_msg.type = [%d]", sms_trans_msg.type);
		MSG_DEBUG("sms_trans_msg.data.p2p_msg.telesvc_id = [%d]", sms_trans_msg.data.p2p_msg.telesvc_id);
		MSG_DEBUG("sms_trans_msg.data.p2p_msg.address.digit_mode = [%d]", sms_trans_msg.data.p2p_msg.address.digit_mode);
		MSG_DEBUG("sms_trans_msg.data.p2p_msg.address.number_mode = [%d]", sms_trans_msg.data.p2p_msg.address.number_mode);
		MSG_DEBUG("sms_trans_msg.data.p2p_msg.address.number_plan = [%d]", sms_trans_msg.data.p2p_msg.address.number_plan);
		MSG_DEBUG("sms_trans_msg.data.p2p_msg.address.number_type = [%d]", sms_trans_msg.data.p2p_msg.address.number_type);
		MSG_DEBUG("sms_trans_msg.data.p2p_msg.address.addr_len = [%d]", sms_trans_msg.data.p2p_msg.address.addr_len);
		MSG_DEBUG("sms_trans_msg.data.p2p_msg.address.szData = [%s]", sms_trans_msg.data.p2p_msg.address.szData);
		MSG_DEBUG("sms_trans_msg.data.p2p_msg.svc_ctg = [%d]", sms_trans_msg.data.p2p_msg.svc_ctg);
		MSG_DEBUG("----------------------- teleservice layer : deliver data -------------------------");
		sms_telesvc_deliver_s *deliver_msg = &(sms_trans_msg.data.p2p_msg.telesvc_msg.data.deliver);
		MSG_DEBUG("sms_trans_msg.data.p2p_msg.telesvc_msg.type = [%d]", sms_trans_msg.data.p2p_msg.telesvc_msg.type);
		MSG_DEBUG("priority= [%d]", deliver_msg->priority);
		MSG_DEBUG("privacy= [%d]", deliver_msg->privacy);
		MSG_DEBUG("display_mode= [%d]", deliver_msg->display_mode);
		MSG_DEBUG("language= [%d]", deliver_msg->language);
		MSG_DEBUG("msg_id= [%d]", deliver_msg->msg_id);
		MSG_DEBUG("alert_priority= [%d]", deliver_msg->alert_priority);
		MSG_DEBUG("num_msg= [%d]", deliver_msg->num_msg);
		MSG_DEBUG("user_data.msg_type= [%d]", deliver_msg->user_data.msg_type);
		MSG_DEBUG("user_data.encode_type= [%d]", deliver_msg->user_data.encode_type);
		MSG_DEBUG("user_data.data_len= [%d]", deliver_msg->user_data.data_len);
		MSG_DEBUG("user_data.user_data= [%s]", deliver_msg->user_data.user_data);
		MSG_DEBUG("time_stamp.year= [%d]", deliver_msg->time_stamp.year);
		MSG_DEBUG("time_stamp.month= [%d]", deliver_msg->time_stamp.month);
		MSG_DEBUG("time_stamp.day= [%d]", deliver_msg->time_stamp.day);
		MSG_DEBUG("time_stamp.hours= [%d]", deliver_msg->time_stamp.hours);
		MSG_DEBUG("time_stamp.minutes= [%d]", deliver_msg->time_stamp.minutes);
		MSG_DEBUG("time_stamp.seconds= [%d]", deliver_msg->time_stamp.seconds);
		MSG_DEBUG("deliver_msg->callback_number.addr_len= [%d]", deliver_msg->callback_number.addr_len);
		MSG_DEBUG("deliver_msg->callback_number.digit_mode= [%d]", deliver_msg->callback_number.digit_mode);
		MSG_DEBUG("deliver_msg->callback_number.number_plan= [%d]", deliver_msg->callback_number.number_plan);
		MSG_DEBUG("deliver_msg->callback_number.number_type= [%d]", deliver_msg->callback_number.number_type);
		MSG_DEBUG("deliver_msg->callback_number.szData= [%s]", deliver_msg->callback_number.szData);
		MSG_DEBUG("#####################################################");
	}
	else if (sms_trans_msg.type == SMS_TRANS_BROADCAST_MSG) {
		MSG_DEBUG("############# SMS_TRANS_BROADCAST_MSG Incoming decoded tpdu values ####################");
		MSG_DEBUG("------------------------------ transport layer data -----------------------------");
		MSG_DEBUG("sms_trans_msg.data.cb_msg.svc_ctg = [%d]", sms_trans_msg.data.cb_msg.svc_ctg);
		MSG_DEBUG("----------------------- teleservice layer : deliver data -------------------------");
		sms_telesvc_deliver_s *deliver_msg = &(sms_trans_msg.data.cb_msg.telesvc_msg.data.deliver);
		MSG_DEBUG("sms_trans_msg.data.cb_msg.telesvc_msg.type = [%d]", sms_trans_msg.data.cb_msg.telesvc_msg.type);
		MSG_DEBUG("priority= [%d]", deliver_msg->priority);
		MSG_DEBUG("display_mode= [%d]", deliver_msg->display_mode);
		MSG_DEBUG("language= [%d]", deliver_msg->language);
		MSG_DEBUG("msg_id= [%d]", deliver_msg->msg_id);
		MSG_DEBUG("msg_id.msg_id)= [%d]", deliver_msg->msg_id.msg_id);
		MSG_DEBUG("header_ind= [%d]", deliver_msg->msg_id.header_ind);
		MSG_DEBUG("alert_priority= [%d]", deliver_msg->alert_priority);
		MSG_DEBUG("cmas_data.encode_type= [%d]", deliver_msg->cmas_data.encode_type);
		MSG_DEBUG("cmas_data.data_len= [%d]", deliver_msg->cmas_data.data_len);
		MSG_DEBUG("cmas_data.alert_text= [%s]", deliver_msg->cmas_data.alert_text);
		MSG_DEBUG("cmas_data.response_type= [%d]", deliver_msg->cmas_data.response_type);
		MSG_DEBUG("cmas_data.severity= [%d]", deliver_msg->cmas_data.severity);
		MSG_DEBUG("cmas_data.urgency= [%d]", deliver_msg->cmas_data.urgency);
		MSG_DEBUG("cmas_data.certainty= [%d]", deliver_msg->cmas_data.certainty);
		MSG_DEBUG("cmas_data.id= [%d]", deliver_msg->cmas_data.id);
		MSG_DEBUG("time_stamp.year= [%d]", deliver_msg->time_stamp.year);
		MSG_DEBUG("time_stamp.month= [%d]", deliver_msg->time_stamp.month);
		MSG_DEBUG("time_stamp.day= [%d]", deliver_msg->time_stamp.day);
		MSG_DEBUG("time_stamp.hours= [%d]", deliver_msg->time_stamp.hours);
		MSG_DEBUG("time_stamp.minutes= [%d]", deliver_msg->time_stamp.minutes);
		MSG_DEBUG("time_stamp.seconds= [%d]", deliver_msg->time_stamp.seconds);
		MSG_DEBUG("deliver_msg->callback_number.addr_len= [%d]", deliver_msg->callback_number.addr_len);
		MSG_DEBUG("deliver_msg->callback_number.digit_mode= [%d]", deliver_msg->callback_number.digit_mode);
		MSG_DEBUG("deliver_msg->callback_number.number_plan= [%d]", deliver_msg->callback_number.number_plan);
		MSG_DEBUG("deliver_msg->callback_number.number_type= [%d]", deliver_msg->callback_number.number_type);
		MSG_DEBUG("deliver_msg->callback_number.szData= [%s]", deliver_msg->callback_number.szData);
		MSG_DEBUG("#####################################################");
	} else if (sms_trans_msg.type == SMS_TRANS_ACK_MSG) {
		// DLOG
		MSG_DEBUG("#####################################################");
	}

	try
	{
		if (sms_trans_msg.type == SMS_TRANS_P2P_MSG) {
			if (sms_trans_msg.data.p2p_msg.telesvc_id == SMS_TRANS_TELESVC_WAP) {
				SmsPluginEventHandler::instance()->handleWapMsgIncoming(&(sms_trans_msg.data.p2p_msg));
			}
			else {
				SmsPluginEventHandler::instance()->handleMsgIncoming(&(sms_trans_msg.data.p2p_msg)); // Call Event Handler
			}
		} else if (sms_trans_msg.type == SMS_TRANS_BROADCAST_MSG) {
			SmsPluginEventHandler::instance()->handleCbMsgIncoming(&(sms_trans_msg.data.cb_msg)); // Call Event Handler
		} else if (sms_trans_msg.type == SMS_TRANS_ACK_MSG) {
			//SmsPluginEventHandler::instance()->handleAckMsgIncoming(&(sms_trans_msg.data.ack_msg)); // Call Event Handler
		}
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return;
	}
}


void TapiEventNetworkStatusChange(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventNetworkStatusChange is called.");

	if (data == NULL) {
		MSG_DEBUG("Error. data is NULL.");
		return;
	}

	TelNetworkServiceType_t *type = (TelNetworkServiceType_t *)data;

	MSG_DEBUG("network status type [%d]", *type);

	if(*type > TAPI_NETWORK_SERVICE_TYPE_SEARCH)
	{
		SmsPluginEventHandler::instance()->handleResendMessage(); // Call Event Handler
	}
}


void TapiEventSimRefreshed(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSimRefreshed is called.");

	SmsPluginSetting::instance()->SimRefreshCb();
}


void TapiEventMemoryStatus(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("Tapi result is [%d]", result);
	if(result == TAPI_API_SUCCESS)
	{
		isMemAvailable = true;
	}
}


void TapiEventSentStatus(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSentStatus is called. result = [%d]", result);

	sms_network_status_t sentStatus;

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
			sentStatus = SMS_NETWORK_SEND_FAIL_TEMPORARY;
			break;
		case TAPI_NETTEXT_DEST_ADDRESS_FDN_RESTRICTED :
		case TAPI_NETTEXT_SCADDRESS_FDN_RESTRICTED :
			sentStatus = SMS_NETWORK_SEND_FAIL_FDN_RESTRICED;
			break;
		default :
			sentStatus = SMS_NETWORK_SEND_FAIL;
			break;
	}

	MSG_DEBUG("SMS Network Status = [%d]", sentStatus);

	if(sentStatus == SMS_NETWORK_SEND_FAIL_TEMPORARY ||
			sentStatus == SMS_NETWORK_SEND_FAIL_BY_MO_CONTROL_WITH_MOD ||
			sentStatus == SMS_NETWORK_SEND_FAIL_FDN_RESTRICED) {
		SmsPluginTransport::instance()->setNetStatus(sentStatus);
		return;
	}

	if(sentStatus == SMS_NETWORK_SEND_FAIL)
	{
		int svc_type;
		tel_get_property_int(handle, TAPI_PROP_NETWORK_SERVICE_TYPE, &svc_type);
		if(svc_type < TAPI_NETWORK_SERVICE_TYPE_2G){
			sentStatus = SMS_NETWORK_SEND_PENDING;
		}
	}

	/* Convert SMS status -> Messaging network status */
	msg_network_status_t netStatus;

	if (sentStatus == SMS_NETWORK_SEND_SUCCESS) {
		netStatus = MSG_NETWORK_SEND_SUCCESS;
	} else if (sentStatus == SMS_NETWORK_SENDING) {
		netStatus = MSG_NETWORK_SENDING;
	} else if (sentStatus == SMS_NETWORK_SEND_PENDING) {
		netStatus = MSG_NETWORK_SEND_PENDING;
	} else if (sentStatus == SMS_NETWORK_SEND_FAIL_UNKNOWN_SUBSCRIBER) {
		netStatus = MSG_NETWORK_SEND_FAIL_UNKNOWN_SUBSCRIBER;
	} else if (sentStatus == SMS_NETWORK_SEND_FAIL_MS_DISABLED) {
		netStatus = MSG_NETWORK_SEND_FAIL_MS_DISABLED;
	} else if (sentStatus == SMS_NETWORK_SEND_FAIL_NETWORK_NOT_READY) {
		netStatus = MSG_NETWORK_SEND_FAIL_NETWORK_NOT_READY;
	} else {
		netStatus = MSG_NETWORK_SEND_FAIL;
	}

	try
	{
		SmsPluginEventHandler::instance()->handleSentStatus(netStatus);

		SmsPluginTransport::instance()->setNetStatus(sentStatus);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return;
	}

}


void TapiEventDeliveryReportCNF(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventDeliveryReportCNF is called. : result = [%d]", result);

	return;
}


void TapiEventSetConfigData(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSetConfigData is called.");

	if (data == NULL)
	{
		MSG_DEBUG("Error. data is NULL.");
		return;
	}

	TelSmsSetResponse* responseType = (TelSmsSetResponse*)data;

	MSG_DEBUG("responseType : [%d]", *responseType);

	switch (*responseType)
	{
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

	SmsPluginSetting::instance()->setResultFromEvent(bRet);

}


void TapiEventGetCBConfig(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetCBConfig is called.");

	MSG_CBMSG_OPT_S cbOpt = {0};

	if (result != TAPI_API_SUCCESS || data == NULL)
	{
		MSG_DEBUG("Error. data is NULL.");

		SmsPluginSetting::instance()->setCbConfigEvent(NULL, false);

		return;
	}

	TelSmsCbConfig_t* pCBConfig = (TelSmsCbConfig_t*)data;

	cbOpt.bReceive = (bool)pCBConfig->CBEnabled;

//	cbOpt.maxSimCnt = pCBConfig->MsgIdMaxCount;

//	MSG_DEBUG("Receive [%d], Max SIM Count [%d]", cbOpt.bReceive, cbOpt.maxSimCnt);
	MSG_DEBUG("Receive [%d]", cbOpt.bReceive);

	cbOpt.channelData.channelCnt = pCBConfig->MsgIdRangeCount;

	if (cbOpt.channelData.channelCnt > CB_CHANNEL_MAX)
	{
		MSG_DEBUG("Channel Count [%d] from TAPI is over MAX", cbOpt.channelData.channelCnt);
		cbOpt.channelData.channelCnt = CB_CHANNEL_MAX;
	}

	MSG_DEBUG("Channel Count [%d]", cbOpt.channelData.channelCnt);

	for (int i = 0; i < cbOpt.channelData.channelCnt; i++)
	{
		cbOpt.channelData.channelInfo[i].bActivate = pCBConfig->MsgIDs[i].Net3gpp2.Selected;
		cbOpt.channelData.channelInfo[i].ctg = pCBConfig->MsgIDs[i].Net3gpp2.CBCategory;
		cbOpt.channelData.channelInfo[i].lang = pCBConfig->MsgIDs[i].Net3gpp2.CBLanguage;
		memset(cbOpt.channelData.channelInfo[i].name, 0x00, CB_CHANNEL_NAME_MAX+1);

		MSG_DEBUG("Channel Category [%d], Channel Language [%d] ", cbOpt.channelData.channelInfo[i].ctg, cbOpt.channelData.channelInfo[i].lang);
	}

	SmsPluginSetting::instance()->setCbConfigEvent(&cbOpt, true);
}

void TapiEventGetMsisdnInfo(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetMsisdnInfo is called.");

	bool bRet = false;

	if (result != TAPI_SIM_ACCESS_SUCCESS || data == NULL) {
		MSG_DEBUG("Error. data is NULL.");
		SmsPluginSetting::instance()->setResultFromSim(bRet);
		return;
	}

	TelSimMsisdnList_t *list = (TelSimMsisdnList_t *)data;

	char keyName[MAX_VCONFKEY_NAME_LEN];
	int simIndex = 1;

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_MSISDN, simIndex);

	for (int i = 0; i < list->count; i++) {
		if (list->list[i].num[0] != '\0') {
			if (MsgSettingSetString(keyName, list->list[i].num) == MSG_SUCCESS) {
				MSG_SEC_DEBUG("Get MSISDN from SIM : [%s]", list->list[i].num);
				bRet = true;
			} else {
				MSG_DEBUG("Getting MSISDN is failed!");
			}
			break;
		}
	}

	SmsPluginSetting::instance()->setResultFromSim(bRet);
}

void _dnet_state_changed_cb(keynode_t *key, void* data)
{
	int dnet_state = MsgSettingGetInt(VCONFKEY_DNET_STATE);

	if (dnet_state > VCONFKEY_DNET_OFF) {
		SmsPluginEventHandler::instance()->handleResendMessage(); // Call Event Handler
	}
}

void _TapiMdnChangedCb(keynode_t *key, void *data)
{
	MSG_INFO("_TapiMdnChangedCb is called.");

	char *mdn = vconf_get_str("db/telephony/mdn");

	if (mdn) {
		bool bNeedToUpdateVoicemail = SmsPluginSetting::instance()->getUpdateVoicemailByMdn();
		MSG_INFO("bNeedToUpdateVoicemail:%d, mdn:[%s]", bNeedToUpdateVoicemail, mdn);

		if (bNeedToUpdateVoicemail) {
			if (MsgSettingSetString(VOICEMAIL_NUMBER, mdn) != MSG_SUCCESS)
				MSG_DEBUG("MsgSettingSetInt is failed!!");

			if (MsgSettingSetString(VOICEMAIL_DEFAULT_NUMBER, mdn) != MSG_SUCCESS)
				MSG_DEBUG("Error to set config data [%s]", VOICEMAIL_DEFAULT_NUMBER);
		}

		free(mdn);
	}
}
