/*
*
* Copyright (c) 2000-2012 Samsung Electronics Co., Ltd. All Rights Reserved.
*
* This file is part of msg-service.
*
* Contact: Jaeyun Jeong <jyjeong@samsung.com>
*          Sangkoo Kim <sangkoo.kim@samsung.com>
*          Seunghwan Lee <sh.cat.lee@samsung.com>
*          SoonMin Jung <sm0415.jung@samsung.com>
*          Jae-Young Lee <jy4710.lee@samsung.com>
*          KeeBum Kim <keebum.kim@samsung.com>
*
* PROPRIETARY/CONFIDENTIAL
*
* This software is the confidential and proprietary information of
* SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered
* into with SAMSUNG ELECTRONICS.
*
* SAMSUNG make no representations or warranties about the suitability
* of the software, either express or implied, including but not limited
* to the implied warranties of merchantability, fitness for a particular
* purpose, or non-infringement. SAMSUNG shall not be liable for any
* damages suffered by licensee as a result of using, modifying or
* distributing this software or its derivatives.
*
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
#include "SmsPluginSimMsg.h"
#include "SmsPluginSetting.h"
#include "SmsPluginCallback.h"

extern "C"
{
	#include <ITapiProductivity.h>
}


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
int TapiEventDeviceReady(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventDeviceReady is called. : request id = [%d] status = [%d]", pEvent->RequestId, pEvent->Status);

	try
	{
		// Call Event Handler
		SmsPluginEventHandler::instance()->setDeviceStatus();
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return e.errorCode();
	}

	return 0;
}


int TapiEventSentStatus(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventSentStatus is called. : request id = [%d] status = [%d]", pEvent->RequestId, pEvent->Status);

	MSG_NETWORK_STATUS_T netStatus;

	// Convert TAPI status -> Messaging status
	if ((TelSmsResponse_t)pEvent->Status == TAPI_NETTEXT_SENDSMS_SUCCESS)
		netStatus = MSG_NETWORK_SEND_SUCCESS;
	else
		netStatus = MSG_NETWORK_SEND_FAIL;

	try
	{
		// Call Event Handler
		SmsPluginEventHandler::instance()->handleSentStatus(pEvent->RequestId, netStatus);

		// Call SAT Handler
		SmsPluginSatHandler::instance()->ctrlSms(netStatus);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return e.errorCode();
	}

	return 0;
}


int TapiEventMsgIncoming(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventMsgIncoming is called. Red Id [%d]", pEvent->RequestId);

	if (pEvent->Status != TAPI_API_SUCCESS || pEvent->pData == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");
		return 0;
	}

	TelSmsDatapackageInfo_t* pDataPackage = (TelSmsDatapackageInfo_t*)pEvent->pData;

	SMS_TPDU_S tpdu;

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
		return e.errorCode();
	}

	return 0;
}


int TapiEventCbMsgIncoming(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventCbMsgIncoming is called.");

	if (pEvent->Status != TAPI_API_SUCCESS || pEvent->pData == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");
		return 0;
	}

	TelSmsCbMsg_t *pCbMsg = (TelSmsCbMsg_t*)pEvent->pData;

	try
	{
		SmsPluginCbMsgHandler::instance()->handleCbMsg(pCbMsg);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return 0;
	}

	return 0;
}


int TapiEventDeliveryReportCNF(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventDeliveryReportCNF is called. : request id = [%d] status = [%d]", pEvent->RequestId, pEvent->Status);

	return 0;
}


int TapiEventGetSimMsgCnt(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventGetSimMsgCnt is called.");

	if (pEvent->Status != TAPI_API_SUCCESS || pEvent->pData == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");
		MSG_SIM_COUNT_S simCnt;
		memset(&simCnt, 0x00, sizeof(MSG_SIM_COUNT_S));
		SmsPluginSimMsg::instance()->setSimMsgCntEvent(&simCnt);
		return 0;
	}

	SmsPluginSimMsg::instance()->setSimMsgCntEvent((MSG_SIM_COUNT_S *)pEvent->pData);

	return 0;
}


int TapiEventGetSimMsg(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventGetSimMsg is called.");

	if (pEvent->Status != TAPI_API_SUCCESS || pEvent->pData == NULL)
	{
		MSG_DEBUG("Error!! pEvent->Status [%d]", pEvent->Status);

		SmsPluginSimMsg::instance()->setSimMsgEvent(NULL, false);

		return 0;
	}

	TelSmsData_t* pSmsTpdu = (TelSmsData_t*)pEvent->pData;

	// Reading TelSmsData_t
	MSG_DEBUG ("sim index %d", pSmsTpdu->SimIndex);
	MSG_DEBUG ("status %d", pSmsTpdu->MsgStatus);

	// Reading TelSmsDatapackageInfo_t
	if (pSmsTpdu->SmsData.MsgLength > MAX_TPDU_DATA_LEN)
	{
		MSG_DEBUG ("WARNING: tpdu_len > MAX_SMS_TPDU_SIZE [%d] bytes. setting to 0.", pSmsTpdu->SmsData.MsgLength);

		SmsPluginSimMsg::instance()->setSimMsgEvent(NULL, false);

		return 0;
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
		MSG_DEBUG("headerCnt [%d]", tpdu.data.deliver.userData.headerCnt);

		for (int i = 0; i < tpdu.data.deliver.userData.headerCnt; i++)
		{
			// Handler Concatenated Message
			if (tpdu.data.deliver.userData.header[i].udhType == SMS_UDH_CONCAT_8BIT ||
				tpdu.data.deliver.userData.header[i].udhType == SMS_UDH_CONCAT_16BIT)
			{
				SmsPluginSimMsg::instance()->setSimMsgEvent(NULL, false);
				return 0;
			}
		}
	}
	else if (tpdu.tpduType == SMS_TPDU_SUBMIT)
	{
		MSG_DEBUG("headerCnt [%d]", tpdu.data.submit.userData.headerCnt);

		for (int i = 0; i < tpdu.data.submit.userData.headerCnt; i++)
		{
			// Handler Concatenated Message
			if (tpdu.data.submit.userData.header[i].udhType == SMS_UDH_CONCAT_8BIT ||
				tpdu.data.submit.userData.header[i].udhType == SMS_UDH_CONCAT_16BIT)
			{
				SmsPluginSimMsg::instance()->setSimMsgEvent(NULL, false);
				return 0;
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

	return 0;
}


int TapiEventSaveSimMsg(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventSaveSimMsg is called. Red Id [%d]", pEvent->RequestId);

	if (pEvent->Status != TAPI_API_SUCCESS || pEvent->pData == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");
		SmsPluginSimMsg::instance()->setSimEvent((MSG_SIM_ID_T)0, false);
		return 0;
	}

	int simId = *((int*)pEvent->pData);

	MSG_DEBUG("sim ID : [%d], status : [%d]", simId, (TelSmsCause_t)pEvent->Status);

	SmsPluginSimMsg::instance()->setSimEvent((MSG_SIM_ID_T)simId, true);

	return 0;
}


int TapiEventDeleteSimMsg(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventDeleteSimMsg is called. Red Id [%d]", pEvent->RequestId);
	MSG_DEBUG("status : [%d]", (TelSmsCause_t)pEvent->Status);

	if (pEvent->Status != TAPI_API_SUCCESS || pEvent->pData == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");
		SmsPluginSimMsg::instance()->setSimEvent((MSG_SIM_ID_T)0, false);
		return 0;
	}

	int sim_id = *((int*)pEvent->pData);

	SmsPluginSimMsg::instance()->setSimEvent((MSG_SIM_ID_T)sim_id, true);

	return 0;
}


int TapiEventSetConfigData(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventSetConfigData is called.");

	if (pEvent->pData == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");
		return 0;
	}

	TelSmsSetResponse* responseType = (TelSmsSetResponse*)pEvent->pData;

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

	MSG_DEBUG("status : [%d]", (TelSmsCause_t)pEvent->Status);

	if ((TelSmsCause_t)pEvent->Status != TAPI_NETTEXT_SUCCESS) bRet = false;

	if (*responseType == TAPI_NETTEXT_SETMESSAGESTATUS_RSP)
		SmsPluginSimMsg::instance()->setSimEvent(0, bRet);
	else
		SmsPluginSetting::instance()->setResultFromSim(bRet);

	return 0;
}


int TapiEventGetParamCnt(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventGetParamCnt is called.");

	if (pEvent->Status != TAPI_API_SUCCESS || pEvent->pData == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");
		SmsPluginSetting::instance()->setParamCntEvent(0);
		return 0;
	}

	int paramCnt = 0;
	paramCnt = *((int*)pEvent->pData);

	SmsPluginSetting::instance()->setParamCntEvent(paramCnt);

	return 0;
}


int TapiEventGetParam(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventGetConfigData is called.");

	if (pEvent->Status != TAPI_API_SUCCESS || pEvent->pData == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");
		SmsPluginSetting::instance()->setParamEvent(NULL, -1, false);
		return 0;
	}

	TelSmsParams_t* smsParam = (TelSmsParams_t*)pEvent->pData;

	int alphaIdLen = 0;
	int addrLen = 0;
	MSG_SMSC_DATA_S smscData = {};

	/*Check Alpha ID value*/
	alphaIdLen = smsParam->AlphaIdLen;
	MSG_DEBUG("alphaId_len[%d]", alphaIdLen);

	if (alphaIdLen < 0 || alphaIdLen > SMSC_NAME_MAX)
	{
		MSG_DEBUG("Wrong Alpha ID Length[%d]", alphaIdLen);

		SmsPluginSetting::instance()->setParamEvent(NULL, -1, false);

		return 0;
	}


	/*Check Address value*/
	addrLen = smsParam->TpSvcCntrAddr.DialNumLen;

	if(addrLen > SMSC_ADDR_MAX)
	{
		MSG_DEBUG("addrLen is too long: %d", addrLen);
		SmsPluginSetting::instance()->setParamEvent(NULL, -1, false);
		return 0;
	}
	else if(addrLen < 2)
	{
		MSG_DEBUG("addrLen is too short: %d", addrLen);
		SmsPluginSetting::instance()->setParamEvent(NULL, -1, false);
		return 0;
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

		SmsPluginSetting::instance()->setParamEvent(NULL, -1, false);

		return 0;
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

	return 0;
}


int TapiEventGetCBConfig(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventGetCBConfig is called.");

	MSG_CBMSG_OPT_S cbOpt = {0};

	if (pEvent->Status != TAPI_API_SUCCESS || pEvent->pData == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");

		SmsPluginSetting::instance()->setCbConfigEvent(NULL, false);

		return 0;
	}

	TelSmsCbConfig_t* pCBConfig = (TelSmsCbConfig_t*)pEvent->pData;

	cbOpt.bReceive = (bool)pCBConfig->bCBEnabled;

	if (pCBConfig->SelectedId == 0x01)
		cbOpt.bAllChannel = true;
	else if (pCBConfig->SelectedId == 0x02)
		cbOpt.bAllChannel = false;

	cbOpt.maxSimCnt = pCBConfig->MsgIdMaxCount;

	MSG_DEBUG("Receive [%d], All Channel [%d], Max SIM Count [%d]", cbOpt.bReceive, cbOpt.bAllChannel, cbOpt.maxSimCnt);

	cbOpt.channelData.channelCnt = pCBConfig->MsgIdCount;

	if (cbOpt.channelData.channelCnt > CB_CHANNEL_MAX)
	{
		MSG_DEBUG("Channel Count [%d] from TAPI is over MAX", cbOpt.channelData.channelCnt);
		cbOpt.channelData.channelCnt = CB_CHANNEL_MAX;
	}

	MSG_DEBUG("Channel Count [%d]", cbOpt.channelData.channelCnt);

	for (int i = 0; i < cbOpt.channelData.channelCnt; i++)
	{
		cbOpt.channelData.channelInfo[i].bActivate = cbOpt.bReceive;
		cbOpt.channelData.channelInfo[i].id = pCBConfig->MsgIDs[i];
		memset(cbOpt.channelData.channelInfo[i].name, 0x00, CB_CHANNEL_NAME_MAX+1);

		MSG_DEBUG("Channel ID [%d]", cbOpt.channelData.channelInfo[i].id);
	}

	SmsPluginSetting::instance()->setCbConfigEvent(&cbOpt, true);

	return 0;
}


int TapiEventSatSmsRefresh(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventSatSmsRefresh is called.");

	if (pEvent->Status != TAPI_API_SUCCESS || pEvent->pData == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");
		return 0;
	}

	try
	{
		SmsPluginSatHandler::instance()->refreshSms(pEvent->pData);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return e.errorCode();
	}

	return 0;
}


int TapiEventSatSendSms(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventSatSendSms is called.");

	if (pEvent->Status != TAPI_API_SUCCESS || pEvent->pData == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");
		return 0;
	}

	try
	{
		SmsPluginSatHandler::instance()->sendSms(pEvent->pData);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return e.errorCode();
	}

	return 0;
}


int TapiEventSatMoSmsCtrl(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventSatMoSmsCtrl is called.");

	if (pEvent->Status != TAPI_API_SUCCESS || pEvent->pData == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");
		return 0;
	}

	try
	{
		SmsPluginSatHandler::instance()->ctrlSms(pEvent->pData);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return e.errorCode();
	}

	return 0;

}


int TapiEventFactoryDftSms(const TelTapiEvent_t *pEvent, void*)
{
	MSG_DEBUG("TapiEventFactoryDftSms is called.");

	if (pEvent->Status != TAPI_API_SUCCESS || pEvent->pData == NULL)
	{
		MSG_DEBUG("Error. evt->pData is NULL.");
		return 0;
	}

	TelFactoryDftSmsInfo_t* pSmsInfo = (TelFactoryDftSmsInfo_t *)pEvent->pData;

	MSG_FOLDER_ID_T folderId = MSG_INBOX_ID;

	if (pSmsInfo->option == TAPI_FACTORY_DFT_SMS_INBOX)
	{
		folderId = MSG_INBOX_ID;
	}
	else if (pSmsInfo->option == TAPI_FACTORY_DFT_SMS_DRAFTS)
	{
		folderId = MSG_DRAFT_ID;
	}
	else if (pSmsInfo->option == TAPI_FACTORY_DFT_SMS_OUTBOX)
	{
		folderId = MSG_OUTBOX_ID;
	}
	else if (pSmsInfo->option == TAPI_FACTORY_DFT_SMS_SENTBOX)
	{
		folderId = MSG_SENTBOX_ID;
	}

	try
	{
		SmsPluginEventHandler::instance()->handleDftSms(folderId, pSmsInfo->number, pSmsInfo->data); // Call Event Handler
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return e.errorCode();
	}

	return 0;
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
	unsigned int tempId = 0;

	tel_register_event(TAPI_EVENT_NETTEXT_DEVICE_READY_IND, &tempId, (TelAppCallback)&TapiEventDeviceReady, NULL);

	tel_register_event(TAPI_EVENT_NETTEXT_SENTSTATUS_CNF, &tempId, (TelAppCallback)&TapiEventSentStatus, NULL);
	tel_register_event(TAPI_EVENT_NETTEXT_INCOM_IND, &tempId, (TelAppCallback)&TapiEventMsgIncoming, NULL);
	tel_register_event(TAPI_EVENT_NETTEXT_CB_INCOM_IND, &tempId, (TelAppCallback)&TapiEventCbMsgIncoming, NULL);
	tel_register_event(TAPI_EVENT_NETTEXT_DELIVERY_REPORT_CNF, &tempId, (TelAppCallback)&TapiEventDeliveryReportCNF, NULL);

	tel_register_event(TAPI_EVENT_NETTEXT_SAVE_STATUS_CNF, &tempId, (TelAppCallback)&TapiEventSaveSimMsg, NULL);
	tel_register_event(TAPI_EVENT_NETTEXT_DELETE_STATUS_CNF, &tempId, (TelAppCallback)&TapiEventDeleteSimMsg, NULL);
	tel_register_event(TAPI_EVENT_NETTEXT_GET_COUNT_CNF, &tempId, (TelAppCallback)&TapiEventGetSimMsgCnt, NULL);
	tel_register_event(TAPI_EVENT_NETTEXT_READ_SMS_CNF, &tempId, (TelAppCallback)&TapiEventGetSimMsg, NULL);

	tel_register_event(TAPI_EVENT_NETTEXT_SET_REQUEST_CNF, &tempId, (TelAppCallback)&TapiEventSetConfigData, NULL);
	tel_register_event(TAPI_EVENT_NETTEXT_PARAM_COUNT_IND, &tempId, (TelAppCallback)&TapiEventGetParamCnt, NULL);
	tel_register_event(TAPI_EVENT_NETTEXT_GET_PARAM_CNF, &tempId, (TelAppCallback)&TapiEventGetParam, NULL);
	tel_register_event(TAPI_EVENT_NETTEXT_GET_CB_CONFIG_CNF, &tempId, (TelAppCallback)&TapiEventGetCBConfig, NULL);

	tel_register_event(TAPI_EVENT_SAT_SMS_REFRESH_IND, &tempId, (TelAppCallback)&TapiEventSatSmsRefresh, NULL);
	tel_register_event(TAPI_EVENT_SAT_SEND_SMS_IND, &tempId, (TelAppCallback)&TapiEventSatSendSms, NULL);
	tel_register_event(TAPI_EVENT_SAT_MO_SMS_CONTROL_IND, &tempId, (TelAppCallback)&TapiEventSatMoSmsCtrl, NULL);

	tel_register_event(TAPI_EVENT_FACTORY_DFT_SMS, &tempId, (TelAppCallback)&TapiEventFactoryDftSms, NULL);

	int tapiRet = TAPI_API_SUCCESS;

	// Register app name to telephony server
	tapiRet = tel_register_app_name((char*)"org.tizen.msgfw");

	if (tapiRet != TAPI_API_SUCCESS)
	{
                THROW(MsgException::SMS_PLG_ERROR, "Failed to register applicatoin name on Telephony Server [%d]", tapiRet);
	}
}


void SmsPluginCallback::deRegisterEvent()
{


}

