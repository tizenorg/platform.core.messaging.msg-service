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

#include <glib.h>
#include <pthread.h>

#include "MsgDebug.h"
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
#include "SmsPluginCallback.h"

extern struct tapi_handle *pTapiHandle;

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
void TapiEventDeviceReady(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventDeviceReady is called. : noti_id = [%d]", noti_id);

	try
	{
		// Call Event Handler
		SmsPluginEventHandler::instance()->setDeviceStatus();
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return;
	}

}


void TapiEventSentStatus(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSentStatus is called. : result = [%d]", result);

	msg_network_status_t netStatus;

	// Convert TAPI status -> Messaging status
	if ((TelSmsResponse_t)result == TAPI_NETTEXT_SENDSMS_SUCCESS)
		netStatus = MSG_NETWORK_SEND_SUCCESS;
	else
		netStatus = MSG_NETWORK_SEND_FAIL;

	try
	{
		// Call Event Handler
		SmsPluginEventHandler::instance()->handleSentStatus(netStatus);

		// Call SAT Handler
		SmsPluginSatHandler::instance()->ctrlSms(netStatus);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return;
	}

}


void TapiEventMsgIncoming(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventMsgIncoming is called. noti_id [%s]", noti_id);

	if (data == NULL) {
		MSG_DEBUG("Error. evt->pData is NULL.");
		return;
	}

	TelSmsDatapackageInfo_t* pDataPackage = (TelSmsDatapackageInfo_t*)data;

	SMS_TPDU_S tpdu;
	memset(&tpdu, 0x00, sizeof(SMS_TPDU_S));

	// Decode Incoming Message
	SmsPluginTpduCodec::decodeTpdu(pDataPackage->szData, pDataPackage->MsgLength, &tpdu);

	/// Print tpdu
	if (tpdu.tpduType == SMS_TPDU_DELIVER)
	{
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
		MSG_DEBUG("tpdu.data.deliver.originAddress.address : %s", tpdu.data.deliver.originAddress.address);
		MSG_DEBUG("tpdu.data.deliver.timeStamp.time : %d/%d/%d %d:%d:%d ", tpdu.data.deliver.timeStamp.time.absolute.year, tpdu.data.deliver.timeStamp.time.absolute.month, tpdu.data.deliver.timeStamp.time.absolute.day,
			tpdu.data.deliver.timeStamp.time.absolute.hour, tpdu.data.deliver.timeStamp.time.absolute.minute, tpdu.data.deliver.timeStamp.time.absolute.second);
		MSG_DEBUG("tpdu.data.deliver.userData.headerCnt : %d", tpdu.data.deliver.userData.headerCnt);
		MSG_DEBUG("tpdu.data.deliver.userData.length : %d", tpdu.data.deliver.userData.length);
		MSG_DEBUG("tpdu.data.deliver.userData.data : %s", tpdu.data.deliver.userData.data);
		MSG_DEBUG("#####################################################");
	}
	else if (tpdu.tpduType == SMS_TPDU_STATUS_REP)
	{
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
		MSG_DEBUG("tpdu.data.statusRep.recipAddress.address : %s", tpdu.data.statusRep.recipAddress.address);
		MSG_DEBUG("tpdu.data.statusRep.timeStamp.time : %d/%d/%d %d:%d:%d ", tpdu.data.statusRep.timeStamp.time.absolute.year, tpdu.data.statusRep.timeStamp.time.absolute.month, tpdu.data.statusRep.timeStamp.time.absolute.day,
			tpdu.data.statusRep.timeStamp.time.absolute.hour, tpdu.data.statusRep.timeStamp.time.absolute.minute, tpdu.data.statusRep.timeStamp.time.absolute.second);
		MSG_DEBUG("tpdu.data.statusRep.dischargeTime.time : %d/%d/%d %d:%d:%d ", tpdu.data.statusRep.dischargeTime.time.absolute.year, tpdu.data.statusRep.dischargeTime.time.absolute.month, tpdu.data.statusRep.dischargeTime.time.absolute.day,
			tpdu.data.statusRep.dischargeTime.time.absolute.hour, tpdu.data.statusRep.dischargeTime.time.absolute.minute, tpdu.data.statusRep.dischargeTime.time.absolute.second);
		MSG_DEBUG("tpdu.data.statusRep.userData.headerCnt : %d", tpdu.data.statusRep.userData.headerCnt);
		MSG_DEBUG("tpdu.data.statusRep.userData.length : %d", tpdu.data.statusRep.userData.length);
		MSG_DEBUG("tpdu.data.statusRep.userData.data : %s", tpdu.data.statusRep.userData.data);
		MSG_DEBUG("#####################################################");
	}

	try
	{
		if (tpdu.tpduType == SMS_TPDU_DELIVER)
		{
			if (tpdu.data.deliver.dcs.msgClass == SMS_MSG_CLASS_2) {
				// For GCF test, 34.2.5.3
				SmsPluginSimMsg::instance()->setSmsData((const char*)pDataPackage->Sca, (const char *)pDataPackage->szData, pDataPackage->MsgLength);
			}

			if (SmsPluginConcatHandler::instance()->IsConcatMsg(&(tpdu.data.deliver.userData)) == true ||
				SmsPluginWapPushHandler::instance()->IsWapPushMsg(&(tpdu.data.deliver.userData)) == true)
			{
				SmsPluginConcatHandler::instance()->handleConcatMsg(&tpdu); // Call Concat Msg Handler
			}
			else
			{
				SmsPluginEventHandler::instance()->handleMsgIncoming(&tpdu); // Call Event Handler
			}
		}
		else if (tpdu.tpduType == SMS_TPDU_STATUS_REP)
		{
			SmsPluginEventHandler::instance()->handleMsgIncoming(&tpdu); // Call Event Handler
		}
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return;
	}

}


void TapiEventCbMsgIncoming(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventCbMsgIncoming is called. noti_id [%s]", noti_id);

	if (data == NULL) {
		MSG_DEBUG("Error. evt->pData is NULL.");
		return;
	}

	TelSmsCbMsg_t *pCbMsg = (TelSmsCbMsg_t*)data;

	try
	{
		SmsPluginCbMsgHandler::instance()->handleCbMsg(pCbMsg);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return;
	}

}


void TapiEventEtwsMsgIncoming(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventEtwsMsgIncoming is called. noti_id [%s]", noti_id);

	if (data == NULL) {
		MSG_DEBUG("Error. evt->pData is NULL.");
		return;
	}

	TelSmsEtwsMsg_t *pEtwsMsg = (TelSmsEtwsMsg_t*)data;

	try
	{
		SmsPluginCbMsgHandler::instance()->handleEtwsMsg(pEtwsMsg);
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


void TapiEventGetSimMsgCnt(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetSimMsgCnt is called.");

	if (result != TAPI_API_SUCCESS || data == NULL)
	{
		MSG_DEBUG("Error. data is NULL.");
		MSG_SIM_COUNT_S simCnt;
		memset(&simCnt, 0x00, sizeof(MSG_SIM_COUNT_S));
		SmsPluginSimMsg::instance()->setSimMsgCntEvent(&simCnt);
		return;
	}

	SmsPluginSimMsg::instance()->setSimMsgCntEvent((MSG_SIM_COUNT_S *)data);

}


void TapiEventGetSimMsg(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetSimMsg is called.");

	if (result != TAPI_API_SUCCESS || data == NULL)
	{
		MSG_DEBUG("Error!! pEvent->Status [%d]", result);

		SmsPluginSimMsg::instance()->setSimMsgEvent(NULL, false);

		return;
	}

	TelSmsData_t* pSmsTpdu = (TelSmsData_t*)data;

	// Reading TelSmsData_t
	MSG_DEBUG ("sim index %d", pSmsTpdu->SimIndex);
	MSG_DEBUG ("status %d", pSmsTpdu->MsgStatus);

	// Reading TelSmsDatapackageInfo_t
	if (pSmsTpdu->SmsData.MsgLength > MAX_TPDU_DATA_LEN)
	{
		MSG_DEBUG ("WARNING: tpdu_len > MAX_SMS_TPDU_SIZE [%d] bytes. setting to 0.", pSmsTpdu->SmsData.MsgLength);

		SmsPluginSimMsg::instance()->setSimMsgEvent(NULL, false);

		return;
	}

	SMS_TPDU_S tpdu;

	// decode Tpdu
	SmsPluginTpduCodec::decodeTpdu(pSmsTpdu->SmsData.szData, pSmsTpdu->SmsData.MsgLength, &tpdu);

	MSG_DEBUG("Sim Message Type [%d]", tpdu.tpduType);

	bool bRead = false;

	// set read status
	if (pSmsTpdu->MsgStatus == TAPI_NETTEXT_STATUS_READ)
		bRead = true;
	else if (pSmsTpdu->MsgStatus == TAPI_NETTEXT_STATUS_UNREAD)
		bRead = false;

	if (tpdu.tpduType == SMS_TPDU_DELIVER)
	{
		if (tpdu.data.deliver.dcs.codingScheme == SMS_CHARSET_8BIT && tpdu.data.deliver.pid == 0x11) {
			MSG_DEBUG("Unsupported message!!");
			SmsPluginSimMsg::instance()->setSimMsgEvent(NULL, false);
			return;
		}

		MSG_DEBUG("headerCnt [%d]", tpdu.data.deliver.userData.headerCnt);
		for (int i = 0; i < tpdu.data.deliver.userData.headerCnt; i++)
		{
			// Handler Concatenated Message
			if (tpdu.data.deliver.userData.header[i].udhType == SMS_UDH_CONCAT_8BIT ||
				tpdu.data.deliver.userData.header[i].udhType == SMS_UDH_CONCAT_16BIT)
			{
				SmsPluginSimMsg::instance()->setSimMsgEvent(NULL, false);
				return;
			}
		}
	}
	else if (tpdu.tpduType == SMS_TPDU_SUBMIT)
	{
		if (tpdu.data.submit.dcs.codingScheme == SMS_CHARSET_8BIT && tpdu.data.submit.pid == 0x11) {
			MSG_DEBUG("Unsupported message!!");
			SmsPluginSimMsg::instance()->setSimMsgEvent(NULL, false);
			return;
		}

		MSG_DEBUG("headerCnt [%d]", tpdu.data.submit.userData.headerCnt);

		for (int i = 0; i < tpdu.data.submit.userData.headerCnt; i++)
		{
			// Handler Concatenated Message
			if (tpdu.data.submit.userData.header[i].udhType == SMS_UDH_CONCAT_8BIT ||
				tpdu.data.submit.userData.header[i].udhType == SMS_UDH_CONCAT_16BIT)
			{
				SmsPluginSimMsg::instance()->setSimMsgEvent(NULL, false);
				return;
			}
		}
	}

	// Make MSG_MESSAGE_INFO_S
	MSG_MESSAGE_INFO_S msgInfo;
	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	SmsPluginEventHandler::instance()->convertTpduToMsginfo(&tpdu, &msgInfo);

	// set Sim Message ID
	msgInfo.msgId = pSmsTpdu->SimIndex;

	// set read status
	msgInfo.bRead = bRead;

	// set storage id
	msgInfo.storageId = MSG_STORAGE_SIM;

	/// Print MSG_MESSAGE_INFO_S
	MSG_DEBUG("############# Convert  tpdu values to Message Info values ####################");
	MSG_DEBUG("msgInfo.msgId : %d", msgInfo.msgId);
	MSG_DEBUG("msgInfo.nAddressCnt : %d", msgInfo.nAddressCnt);
	MSG_DEBUG("msgInfo.addressList[0].addressType : %d", msgInfo.addressList[0].addressType);
	MSG_DEBUG("msgInfo.addressList[0].addressVal : %s", msgInfo.addressList[0].addressVal);
	MSG_DEBUG("msgInfo.priority : %d", msgInfo.priority);
	MSG_DEBUG("msgInfo.bProtected : %d", msgInfo.bProtected);
	MSG_DEBUG("msgInfo.bRead : %d", msgInfo.bRead);
	MSG_DEBUG("msgInfo.bTextSms : %d", msgInfo.bTextSms);
	MSG_DEBUG("msgInfo.direction : %d", msgInfo.direction);
	MSG_DEBUG("msgInfo.msgType.mainType : %d", msgInfo.msgType.mainType);
	MSG_DEBUG("msgInfo.msgType.subType : %d", msgInfo.msgType.subType);
	MSG_DEBUG("msgInfo.msgType.classType : %d", msgInfo.msgType.classType);
	MSG_DEBUG("msgInfo.displayTime : %s", ctime(&msgInfo.displayTime));
	MSG_DEBUG("msgInfo.dataSize : %d", msgInfo.dataSize);
	if (msgInfo.bTextSms == true)
		MSG_DEBUG("msgInfo.msgText : %s", msgInfo.msgText);
	else
	MSG_DEBUG("msgInfo.msgData : %s", msgInfo.msgData);
	MSG_DEBUG("###############################################################");

	SmsPluginSimMsg::instance()->setSimMsgEvent(&msgInfo, true); // Call Event Handler

}


void TapiEventSaveSimMsg(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSaveSimMsg is called. result [%d]", result);

	int simId = -1;

	if (data != NULL)
		simId = *((int*)data);
	else
		MSG_DEBUG("Data(SIM Msg ID) is NULL");

	SmsPluginSimMsg::instance()->setSaveSimMsgEvent(simId, result);
}


void TapiEventSaveClass2Msg(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSaveSimMsg is called. result [%d]", result);

	int simId = -1;

	if (data != NULL)
		simId = *((int*)data);
	else
		MSG_DEBUG("Data(SIM Msg ID) is NULL");

	SmsPluginSimMsg::instance()->setSaveClass2MsgEvent(simId, result);
}


void TapiEventDeleteSimMsg(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventDeleteSimMsg is called. result [%d]", result);

	MSG_DEBUG("status : [%d]", (TelSmsCause_t)result);

	if (result != TAPI_API_SUCCESS || data == NULL)
	{
		MSG_DEBUG("Error. data is NULL.");
		SmsPluginSimMsg::instance()->setSimEvent((msg_sim_id_t)0, false);
		return;
	}

	int sim_id = *((int*)data);

	SmsPluginSimMsg::instance()->setSimEvent((msg_sim_id_t)sim_id, true);

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

	if (*responseType == TAPI_NETTEXT_SETMESSAGESTATUS_RSP)
		SmsPluginSimMsg::instance()->setSimEvent(0, bRet);
	else
		SmsPluginSetting::instance()->setResultFromSim(bRet);

}


void TapiEventGetParamCnt(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetParamCnt is called.");

	if (result != TAPI_API_SUCCESS || data == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");
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

	if (result != TAPI_API_SUCCESS || data == NULL)
	{
		MSG_DEBUG("Error. data is NULL.");
		SmsPluginSetting::instance()->setParamEvent(NULL, -1, false);
		return;
	}

	TelSmsParams_t* smsParam = (TelSmsParams_t*)data;

	int alphaIdLen = 0;
	int addrLen = 0;
	MSG_SMSC_DATA_S smscData = {0, };

	/*Check Alpha ID value*/
	alphaIdLen = smsParam->AlphaIdLen;
	MSG_DEBUG("alphaId_len[%d]", alphaIdLen);

	if (alphaIdLen < 0 || alphaIdLen > SMSC_NAME_MAX)
	{
		MSG_DEBUG("Wrong Alpha ID Length[%d]", alphaIdLen);

		SmsPluginSetting::instance()->setParamEvent(NULL, -1, false);

		return;
	}


	/*Check Address value*/
	addrLen = smsParam->TpSvcCntrAddr.DialNumLen;

	if(addrLen > SMSC_ADDR_MAX)
	{
		MSG_DEBUG("addrLen is too long: %d", addrLen);
		SmsPluginSetting::instance()->setParamEvent(NULL, -1, false);
		return;
	}
	else if(addrLen < 2)
	{
		MSG_DEBUG("addrLen is too short: %d", addrLen);
		SmsPluginSetting::instance()->setParamEvent(NULL, -1, false);
		return;
	}

	MSG_DEBUG("addrLen : %d", addrLen);


	/*SMSP Parameter Indicator value*/
	MSG_DEBUG("ParamIndicator[%02x]", smsParam->ParamIndicator);

	/*Get SMSC Address*/
	if(0x00 == (0x02 & smsParam->ParamIndicator))
	{
		MSG_DEBUG("record index[%d]", (int)smsParam->RecordIndex);

		MSG_DEBUG("TON : %d", smsParam->TpSvcCntrAddr.Ton);
		MSG_DEBUG("NPI : %d", smsParam->TpSvcCntrAddr.Npi);

		smscData.smscAddr.ton = smsParam->TpSvcCntrAddr.Ton;
		smscData.smscAddr.npi = smsParam->TpSvcCntrAddr.Npi;

		SmsPluginParamCodec paramCodec;

		memset(smscData.smscAddr.address, 0x00, SMSC_ADDR_MAX+1);
		paramCodec.decodeSMSC(smsParam->TpSvcCntrAddr.szDiallingNum, addrLen, smscData.smscAddr.ton, smscData.smscAddr.address);

		MSG_DEBUG("SMSC Address : [%s]", smscData.smscAddr.address);

		memset(smscData.name, 0x00, SMSC_NAME_MAX+1);
		memcpy(smscData.name, smsParam->szAlphaId, alphaIdLen);
		smscData.name[alphaIdLen] = '\0';

		MSG_DEBUG("SMSC Name : [%s]", smscData.name);
	}
	else
	{
		MSG_DEBUG("SMSC Address is not present");

//		smscData.smscAddr.ton = MSG_TON_UNKNOWN;
//		smscData.smscAddr.npi = MSG_NPI_UNKNOWN;
//
//		memset(smscData.smscAddr.address, 0x00, SMSC_ADDR_MAX+1);
//		memset(smscData.name, 0x00, SMSC_NAME_MAX+1);

		SmsPluginSetting::instance()->setParamEvent(NULL, -1, false);

		return;
	}

	/*Get the PID value*/
	if (0x00 == (0x04 & smsParam->ParamIndicator))
	{
		SMS_PID_T pid = (SMS_PID_T)smsParam->TpProtocolId;

		MSG_DEBUG("smsParam->TpProtocolId : %d", smsParam->TpProtocolId);

		switch (pid)
		{
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
	}
	else
	{
		MSG_DEBUG("PID is not present");
		smscData.pid = MSG_PID_TEXT;
		MSG_DEBUG("MSG_PID_TEXT is inserted to PID");
	}

#if 0
	/*Get the DCS value*/
	if (0x00 == (0x08 & smsParam->ParamIndicator))
	{
		smscList.smscData[index].dcs = smsParam->TpDataCodingScheme;
		MSG_DEBUG("dcs : %d", smscList.smscData[index].dcs);
	}
	else
	{
		smscList.smscData[index].dcs = MSG_ENCODE_GSM7BIT;
		MSG_DEBUG("DCS is not present");
	}
#endif

	/*Get the ValidityPeriod value*/
	if (0x00 == (0x10 & smsParam->ParamIndicator))
	{
		smscData.valPeriod = smsParam->TpValidityPeriod;
		MSG_DEBUG("valPeriod : %d", smscData.valPeriod);
	}
	else
	{
		smscData.valPeriod = 0;
		MSG_DEBUG("Validity Period is not present");
	}

	SmsPluginSetting::instance()->setParamEvent(&smscData, (int)smsParam->RecordIndex, true);

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

	cbOpt.maxSimCnt = pCBConfig->MsgIdMaxCount;

	MSG_DEBUG("Receive [%d], Max SIM Count [%d]", cbOpt.bReceive, cbOpt.maxSimCnt);

	cbOpt.channelData.channelCnt = pCBConfig->MsgIdRangeCount;

	if (cbOpt.channelData.channelCnt > CB_CHANNEL_MAX)
	{
		MSG_DEBUG("Channel Count [%d] from TAPI is over MAX", cbOpt.channelData.channelCnt);
		cbOpt.channelData.channelCnt = CB_CHANNEL_MAX;
	}

	if (MsgSettingSetInt(CB_CHANNEL_COUNT, cbOpt.channelData.channelCnt) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", CB_CHANNEL_COUNT);
	}

	MSG_DEBUG("Channel Count [%d]", cbOpt.channelData.channelCnt);

	for (int i = 0; i < cbOpt.channelData.channelCnt; i++)
	{
		cbOpt.channelData.channelInfo[i].bActivate = pCBConfig->MsgIDs[i].Net3gpp.Selected;
		cbOpt.channelData.channelInfo[i].from = pCBConfig->MsgIDs[i].Net3gpp.FromMsgId;
		cbOpt.channelData.channelInfo[i].to = pCBConfig->MsgIDs[i].Net3gpp.ToMsgId;
		memset(cbOpt.channelData.channelInfo[i].name, 0x00, CB_CHANNEL_NAME_MAX+1);

		MSG_DEBUG("Channel FROM [%d], Channel TO [%d] ", cbOpt.channelData.channelInfo[i].from, cbOpt.channelData.channelInfo[i].to);
	}

	SmsPluginSetting::instance()->setCbConfigEvent(&cbOpt, true);
}

void TapiEventSetMailboxInfo(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSetMailboxInfo is called. result = [%d]", result);

	bool bRet = true;

	if (result != TAPI_SIM_ACCESS_SUCCESS)
		bRet = false;

	SmsPluginSetting::instance()->setResultFromSim(bRet);
}

void TapiEventGetMailboxInfo(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetMailboxInfo is called.");

	if (result != TAPI_SIM_ACCESS_SUCCESS || data == NULL)
	{
		MSG_DEBUG("Error. data is NULL.");
		SmsPluginSetting::instance()->setMailboxInfoEvent(NULL, false);

		return;
	}

	TelSimMailboxList_t *list = (TelSimMailboxList_t *)data;
	SMS_SIM_MAILBOX_LIST_S mbList = {0,};

	if (list->count <= 0) {
		SmsPluginSetting::instance()->setMailboxInfoEvent(NULL, true);
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
	}

	SmsPluginSetting::instance()->setMailboxInfoEvent(&mbList, true);
}

void TapiEventSetMwiInfo(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSetMwiInfo is called. result = [%d]", result);
}

void TapiEventGetMwiInfo(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetMwiInfo is called.");

	if (result != TAPI_SIM_ACCESS_SUCCESS || data == NULL)
	{
		MSG_DEBUG("Error. data is NULL.");
		SmsPluginSetting::instance()->setMwiInfoEvent(NULL, false);

		return;
	}

	TelSimMessageWaitingResp_t *MwiInfo = (TelSimMessageWaitingResp_t *)data;
	SMS_SIM_MWI_INFO_S simMwiInfo = {0,};

	memcpy(&simMwiInfo, MwiInfo, sizeof(SMS_SIM_MWI_INFO_S));

	SmsPluginSetting::instance()->setMwiInfoEvent(&simMwiInfo, true);
}

void TapiEventGetMsisdnInfo(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventGetMsisdnInfo is called.");

	if (result != TAPI_SIM_ACCESS_SUCCESS || data == NULL)
	{
		MSG_DEBUG("Error. data is NULL.");
		return;
	}

	TelSimMsisdnList_t *list = (TelSimMsisdnList_t *)data;

	for (int i = 0; i < list->count; i++) {
		if (list->list[i].num[0] != '\0') {
			if (MsgSettingSetString(MSG_SIM_MSISDN, list->list[i].num) == MSG_SUCCESS)
				MSG_DEBUG("Get MSISDN from SIM : [%s]", list->list[i].num);
			else
				MSG_DEBUG("Getting MSISDN is failed!");

			break;
		}
	}
}

void TapiEventSatSmsRefresh(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSatSmsRefresh is called.");

	if (result != TAPI_API_SUCCESS || data == NULL)
	{
		MSG_DEBUG("Error. data is NULL.");
		return;
	}

	try
	{
		SmsPluginSatHandler::instance()->refreshSms(data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return;
	}

}


void TapiEventSatSendSms(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSatSendSms is called.");

	if (data == NULL)
	{
		MSG_DEBUG("Error. data is NULL.");
		return;
	}

	try
	{
		SmsPluginSatHandler::instance()->sendSms(data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return;
	}

}


void TapiEventSatMoSmsCtrl(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSatMoSmsCtrl is called.");

	if (result != TAPI_API_SUCCESS || data == NULL)
	{
		MSG_DEBUG("Error. data is NULL.");
		return;
	}

	try
	{
		SmsPluginSatHandler::instance()->ctrlSms(data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return;
	}

}

void TapiEventMemoryStatus(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("Tapi result is [%d]", result);
}

void TapiEventSetMsgStatus(TapiHandle *handle, int result, void *data, void *user_data)
{
	MSG_DEBUG("TapiEventSetMsgStatus is called. result [%d]", result);

	if (result != TAPI_API_SUCCESS || data == NULL)
	{
		MSG_DEBUG("Error. data is NULL.");
		SmsPluginSimMsg::instance()->setSimEvent((msg_sim_id_t)0, false);
		return;
	}

	msg_sim_id_t sim_id = *((msg_sim_id_t *)user_data);

	SmsPluginSimMsg::instance()->setSimEvent(sim_id, true);
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
	if (pInstance != NULL)
	{
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
	tel_register_noti_event(pTapiHandle, TAPI_NOTI_SMS_DEVICE_READY, TapiEventDeviceReady, NULL);
	tel_register_noti_event(pTapiHandle, TAPI_NOTI_SMS_INCOM_MSG, TapiEventMsgIncoming, NULL);
	tel_register_noti_event(pTapiHandle, TAPI_NOTI_SMS_CB_INCOM_MSG, TapiEventCbMsgIncoming, NULL);
	tel_register_noti_event(pTapiHandle, TAPI_NOTI_SMS_ETWS_INCOM_MSG, TapiEventEtwsMsgIncoming, NULL);
//	tel_register_noti_event(pTapiHandle, TAPI_NOTI_SAT_REFRESH, TapiEventSatSmsRefresh, NULL);
	tel_register_noti_event(pTapiHandle, TAPI_NOTI_SAT_SEND_SMS, TapiEventSatSendSms, NULL);
//	tel_register_noti_event(pTapiHandle, TAPI_NOTI_SAT_MO_SMS_CTRL, TapiEventSatMoSmsCtrl, NULL);
}


void SmsPluginCallback::deRegisterEvent()
{


}

