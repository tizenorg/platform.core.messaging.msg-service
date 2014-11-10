/*
 * msg-service
 *
 * Copyright (c) 2000 - 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgGconfWrapper.h"
#include "SmsPluginParamCodec.h"
#include "SmsPluginUDCodec.h"
#include "SmsPluginSetting.h"
#include "SmsPluginTransport.h"
#include "SmsPluginCallback.h"
#include "SmsPluginEventHandler.h"
#include "SmsPluginSatHandler.h"

extern "C"
{
	#include <tapi_common.h>
	#include <TelSms.h>
	#include <TapiUtility.h>
	#include <ITapiNetText.h>
	#include <ITapiSat.h>
}

extern struct tapi_handle *pTapiHandle;
/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginCbMsgHandler - Member Functions
==================================================================================================*/
SmsPluginSatHandler* SmsPluginSatHandler::pInstance = NULL;


SmsPluginSatHandler::SmsPluginSatHandler()
{
	commandId = 0;

	bSendSms = false;
	bInitSim = false;
	bSMSPChanged = false;
	bCBMIChanged = false;
}


SmsPluginSatHandler::~SmsPluginSatHandler()
{

}


SmsPluginSatHandler* SmsPluginSatHandler::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginSatHandler();

	return pInstance;
}


void SmsPluginSatHandler::refreshSms(void *pData)
{
	/*
	TelSatRefreshInd_t* pRefreshData = (TelSatRefreshInd_t*)pData;

	if (pRefreshData->appType != TAPI_SAT_REFRESH_MSG)
	{
		MSG_DEBUG("App Type is wrong. [%d]", pRefreshData->appType);
		return;
	}

	commandId = pRefreshData->commandId;

	switch (pRefreshData->refreshMode)
	{
		case TAPI_SAT_REFRESH_SIM_INIT_AND_FULL_FCN :
		case TAPI_SAT_REFRESH_SIM_INIT_AND_FCN :
		{
			for (int i = 0; i < pRefreshData->fileCount; i++)
			{
				if ((SMS_SIM_EFILE_NAME_T)pRefreshData->fileId[i].FileName == SMS_SIM_EFILE_USIM_SMSP ||
					(SMS_SIM_EFILE_NAME_T)pRefreshData->fileId[i].FileName == SMS_SIM_EFILE_SMSP)
				{
					bSMSPChanged = true;
				}
				else if ((SMS_SIM_EFILE_NAME_T)pRefreshData->fileId[i].FileName == SMS_SIM_EFILE_USIM_CBMI ||
					(SMS_SIM_EFILE_NAME_T)pRefreshData->fileId[i].FileName == SMS_SIM_EFILE_CBMI)
				{
					bCBMIChanged = true;
				}
			}

			initSim();
		}
		break;

		case TAPI_SAT_REFRESH_FCN :
		{
			for (int i = 0; i < pRefreshData->fileCount; i++)
			{
				if ((SMS_SIM_EFILE_NAME_T)pRefreshData->fileId[i].FileName == SMS_SIM_EFILE_USIM_SMSP ||
					(SMS_SIM_EFILE_NAME_T)pRefreshData->fileId[i].FileName == SMS_SIM_EFILE_SMSP)
				{
					bSMSPChanged = true;
				}
				else if ((SMS_SIM_EFILE_NAME_T)pRefreshData->fileId[i].FileName == SMS_SIM_EFILE_USIM_CBMI ||
					(SMS_SIM_EFILE_NAME_T)pRefreshData->fileId[i].FileName == SMS_SIM_EFILE_CBMI)
				{
					bCBMIChanged = true;
				}
			}
		}
		break;

		case TAPI_SAT_REFRESH_SIM_INIT :
		{
			initSim();
		}
		break;

		default:
		break;
	}
	*/
}


void SmsPluginSatHandler::sendSms(void *pData)
{
	TelSatSendSmsIndSmsData_t* pSmsData = (TelSatSendSmsIndSmsData_t*)pData;

	bSendSms = true;

	commandId = pSmsData->commandId;

	MSG_DEBUG("commandId [%d]", commandId);

	// The TPDU Maximum Length at SAT side is 175
	// This is because Sat can send 160 bytes unpacked and header
	unsigned char tpdu[MAX_SAT_TPDU_LEN+1];
	int tpduLen = 0;

	memset(tpdu, 0x00, sizeof(tpdu));
	memcpy(tpdu, pSmsData->smsTpdu.data, pSmsData->smsTpdu.dataLen);

	// Modify Parameters, Pack User Data
	tpduLen = handleSatTpdu(tpdu, pSmsData->smsTpdu.dataLen, pSmsData->bIsPackingRequired);

	if (tpduLen <= 0 || tpduLen > MAX_TPDU_DATA_LEN)
	{
		sendResult(SMS_SAT_CMD_SEND_SMS, TAPI_SAT_R_BEYOND_ME_CAPABILITIES);
		return;
	}

	// Make Telephony Structure
	TelSmsDatapackageInfo_t pkgInfo;

	// Set TPDU data
	memset((void*)pkgInfo.szData, 0x00, sizeof(pkgInfo.szData));
	memcpy((void*)pkgInfo.szData, tpdu, tpduLen);

	pkgInfo.szData[tpduLen - 1] = '\0';
	pkgInfo.MsgLength = tpduLen;

	// Set SMSC Address
	SMS_ADDRESS_S smsc;

	if (pSmsData->address.diallingNumberLen > 0)
	{
		smsc.ton = pSmsData->address.ton;
		smsc.npi = pSmsData->address.npi;
		memcpy(smsc.address, pSmsData->address.diallingNumber, pSmsData->address.diallingNumberLen);
		smsc.address[pSmsData->address.diallingNumberLen] = '\0';

		MSG_DEBUG("SCA TON[%d], NPI[%d], LEN[%d], ADDR[%s]", smsc.ton, smsc.npi, strlen(smsc.address), smsc.address);
	}
	else
	{
		// Set SMSC Options
		SmsPluginTransport::instance()->setSmscOptions(&smsc);
	}

	unsigned char smscAddr[MAX_SMSC_LEN];
	memset(smscAddr, 0x00, sizeof(smscAddr));

	int smscLen = SmsPluginParamCodec::encodeSMSC(&smsc, smscAddr);

	if (smscLen <= 0) return;

	// Set SMSC Address
	memset(pkgInfo.Sca, 0x00, sizeof(pkgInfo.Sca));
	memcpy((void*)pkgInfo.Sca, smscAddr, smscLen);
	pkgInfo.Sca[smscLen] = '\0';

	int tapiRet = TAPI_API_SUCCESS;

	// Send SMS
	tapiRet = tel_send_sms(pTapiHandle, &pkgInfo, 0, TapiEventSentStatus, NULL);

	if (tapiRet == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("########  TelTapiSmsSend Success !!! return : %d #######", tapiRet);
	}
	else
	{
		MSG_DEBUG("########  TelTapiSmsSend Fail !!! return : %d #######", tapiRet);
		sendResult(SMS_SAT_CMD_SEND_SMS, TAPI_SAT_R_BEYOND_ME_CAPABILITIES);
	}
}


void SmsPluginSatHandler::ctrlSms(void *pData)
{
	TelSatMoSmCtrlIndData_t* pCtrlData = (TelSatMoSmCtrlIndData_t*)pData;

	if (bSendSms == true) // Send SMS By SAT
	{
		if (pCtrlData->moSmsCtrlResult == TAPI_SAT_CALL_CTRL_R_NOT_ALLOWED)
		{
			MSG_DEBUG("SIM does not allow to send SMS");

			sendResult(SMS_SAT_CMD_SEND_SMS, TAPI_SAT_R_INTRCTN_WITH_CC_OR_SMS_CTRL_PRMNT_PRBLM);
		}
		else if (pCtrlData->moSmsCtrlResult == TAPI_SAT_CALL_CTRL_R_ALLOWED_WITH_MOD)
		{
			MSG_DEBUG("SIM allows to send SMS with modification");

			sendResult(SMS_SAT_CMD_SEND_SMS, TAPI_SAT_R_SUCCESS);
		}
	}
	else // Send SMS By APP
	{
		msg_network_status_t netStatus = MSG_NETWORK_NOT_SEND;

		if (pCtrlData->moSmsCtrlResult == TAPI_SAT_CALL_CTRL_R_NOT_ALLOWED)
		{
			MSG_DEBUG("SIM does not allow to send SMS");

			netStatus = MSG_NETWORK_SEND_FAIL;
		}
		else if (pCtrlData->moSmsCtrlResult == TAPI_SAT_CALL_CTRL_R_ALLOWED_WITH_MOD)
		{
			MSG_DEBUG("SIM allows to send SMS with modification");

			netStatus = MSG_NETWORK_SEND_SUCCESS;
		}

		// Call Event Handler
		SmsPluginEventHandler::instance()->handleSentStatus(netStatus);
	}
}


void SmsPluginSatHandler::ctrlSms(msg_network_status_t netStatus)
{
	if (bSendSms == true) // Send SMS By SAT
	{
		if (netStatus == MSG_NETWORK_SEND_SUCCESS)
		{
			MSG_DEBUG("Sending SMS by SAT is OK");

			sendResult(SMS_SAT_CMD_SEND_SMS, TAPI_SAT_R_SUCCESS);
		}
		else if (netStatus == MSG_NETWORK_SEND_FAIL)
		{
			MSG_DEBUG("Sending SMS by SAT is failed");

			sendResult(SMS_SAT_CMD_SEND_SMS, TAPI_SAT_R_SMS_RP_ERROR);
		}
	}
}


void SmsPluginSatHandler::finishSimMsgInit(msg_error_t Err)
{
	// SAT Handler is initializing SIM now
	if (bInitSim == true)
	{
		// Init SMSC List and CB ID Info
		try
		{
			if (bSMSPChanged == true) initSMSCList();
		}
		catch (MsgException& e)
		{
			// Send Result to TAPI
			sendResult(SMS_SAT_CMD_REFRESH, TAPI_SAT_R_ME_UNABLE_TO_PROCESS_COMMAND);

			MSG_FATAL("%s", e.what());
			return;
		}

		try
		{
			if (bCBMIChanged == true) initCBConfig();
		}
		catch (MsgException& e)
		{
			// Send Result to TAPI
			sendResult(SMS_SAT_CMD_REFRESH, TAPI_SAT_R_ME_UNABLE_TO_PROCESS_COMMAND);

			MSG_FATAL("%s", e.what());
			return;
		}

		// Send Result to TAPI
		sendResult(SMS_SAT_CMD_REFRESH, TAPI_SAT_R_SUCCESS);
	}
}


void SmsPluginSatHandler::initSim()
{
	bInitSim = true;

	// Init SIM Message
	if (SmsPluginEventHandler::instance()->callbackInitSimBySat() != MSG_SUCCESS)
	{
		MSG_DEBUG("Init Sim Error");

		// Send Result to TAPI
		sendResult(SMS_SAT_CMD_REFRESH, TAPI_SAT_R_ME_UNABLE_TO_PROCESS_COMMAND);
	}
}


void	SmsPluginSatHandler::initSMSCList()
{
	MSG_SETTING_S settingData;

	settingData.type = MSG_SMSC_LIST;

	// Get Data From SIM
	SmsPluginSetting::instance()->getConfigData(&settingData);

	MSG_DEBUG("total_count[%d]", settingData.option.smscList.totalCnt);
	MSG_DEBUG("selected[%d]", settingData.option.smscList.selected);

	for (int i = 0; i < settingData.option.smscList.totalCnt; i++)
	{
		MSG_DEBUG("pid[%d]", settingData.option.smscList.smscData[i].pid);
//		MSG_DEBUG("dcs[%d]", settingData.option.smscList.smscData[i].dcs);
		MSG_DEBUG("val_period[%d]", settingData.option.smscList.smscData[i].valPeriod);
		MSG_DEBUG("name[%s]", settingData.option.smscList.smscData[i].name);

		MSG_DEBUG("ton[%d]", settingData.option.smscList.smscData[i].smscAddr.ton);
		MSG_DEBUG("npi[%d]", settingData.option.smscList.smscData[i].smscAddr.npi);
		MSG_DEBUG("address[%s]", settingData.option.smscList.smscData[i].smscAddr.address);
	}

	if (MsgSettingSetInt(SMSC_SELECTED, settingData.option.smscList.selected) != MSG_SUCCESS)
	{
		THROW(MsgException::SMS_PLG_ERROR, "Error to set config data [%s]", SMSC_SELECTED);
		return;
	}

	if (MsgSettingSetInt(SMSC_TOTAL_COUNT, settingData.option.smscList.totalCnt) != MSG_SUCCESS)
	{
		THROW(MsgException::SMS_PLG_ERROR, "Error to set config data [%s]", SMSC_TOTAL_COUNT);
		return;
	}

	char keyName[128];
	msg_error_t err = MSG_SUCCESS;

	for (int i = 0; i < settingData.option.smscList.totalCnt; i++)
	{
		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", SMSC_PID, i);

		if ((err = MsgSettingSetInt(keyName, (int)settingData.option.smscList.smscData[i].pid)) != MSG_SUCCESS)
			break;

#if 0
		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", SMSC_DCS, i);

		if ((err = MsgSettingSetInt(keyName, (int)settingData.option.smscList.smscData[i].dcs)) != MSG_SUCCESS)
			break;
#endif

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", SMSC_VAL_PERIOD, i);

		if ((err = MsgSettingSetInt(keyName, (int)settingData.option.smscList.smscData[i].valPeriod)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", SMSC_NAME, i);

		if ((err = MsgSettingSetString(keyName, settingData.option.smscList.smscData[i].name)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", SMSC_TON, i);

		if ((err = MsgSettingSetInt(keyName, (int)settingData.option.smscList.smscData[i].smscAddr.ton)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", SMSC_NPI, i);

		if ((err = MsgSettingSetInt(keyName, (int)settingData.option.smscList.smscData[i].smscAddr.npi)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", SMSC_ADDRESS, i);

		if ((err = MsgSettingSetString(keyName, settingData.option.smscList.smscData[i].smscAddr.address)) != MSG_SUCCESS)
			break;
	}

	if (err != MSG_SUCCESS)
	{
		THROW(MsgException::SMS_PLG_ERROR, "Error to set config data [%s]", keyName);
	}
}


void	SmsPluginSatHandler::initCBConfig()
{
	MSG_SETTING_S settingData;

	settingData.type = MSG_CBMSG_OPT;

	// Get Data From SIM
	SmsPluginSetting::instance()->getConfigData(&settingData);

	if (MsgSettingSetBool(CB_RECEIVE, settingData.option.cbMsgOpt.bReceive) != MSG_SUCCESS)
	{
		THROW(MsgException::SMS_PLG_ERROR, "Error to set config data [%s]", CB_RECEIVE);
		return;
	}

	if (MsgSettingSetInt(CB_MAX_SIM_COUNT, settingData.option.cbMsgOpt.maxSimCnt) != MSG_SUCCESS)
	{
		THROW(MsgException::SMS_PLG_ERROR, "Error to set config data [%s]", CB_MAX_SIM_COUNT);
		return;
	}

	if (MsgSettingSetInt(CB_CHANNEL_COUNT, settingData.option.cbMsgOpt.channelData.channelCnt) != MSG_SUCCESS)
	{
		THROW(MsgException::SMS_PLG_ERROR, "Error to set config data [%s]", CB_CHANNEL_COUNT);
		return;
	}

	char keyName[128];
	msg_error_t err = MSG_SUCCESS;

	for (int i = 0; i < settingData.option.cbMsgOpt.channelData.channelCnt; i++)
	{
		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ACTIVATE, i);

		if ((err = MsgSettingSetBool(keyName, settingData.option.cbMsgOpt.channelData.channelInfo[i].bActivate)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ID_FROM, i);

		if ((err = MsgSettingSetInt(keyName, settingData.option.cbMsgOpt.channelData.channelInfo[i].from)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ID_TO, i);

		if ((err = MsgSettingSetInt(keyName, settingData.option.cbMsgOpt.channelData.channelInfo[i].to)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_NAME, i);

		if ((err = MsgSettingSetString(keyName, settingData.option.cbMsgOpt.channelData.channelInfo[i].name)) != MSG_SUCCESS)
			break;
	}

	if (err != MSG_SUCCESS)
	{
		THROW(MsgException::SMS_PLG_ERROR, "Error to set config data [%s]", keyName);
	}
}


int SmsPluginSatHandler::handleSatTpdu(unsigned char *pTpdu, unsigned char TpduLen, int bIsPackingRequired)
{
	if (pTpdu == NULL)
		THROW(MsgException::SMS_PLG_ERROR, "SAT TPDU is NULL");

	int pos = 0;

	// TP-MTI, TP-RD, TP-VPF,  TP-RP, TP-UDHI, TP-SRR
	// TP-VPF
	SMS_VPF_T vpf = (SMS_VPF_T)(pTpdu[pos++] & 0x18) >> 3;

	// TP-MR
	unsigned char tmpRef = pTpdu[pos];

	MSG_DEBUG("current Msg Ref : %d", tmpRef);

	pTpdu[pos] = tmpRef + 1;

	MSG_DEBUG("new Msg Ref : %d", pTpdu[pos]);

	pos++;

//	pTpdu[pos++] = SmsPluginTransport::instance()->getMsgRef();


	// TP-DA
	SMS_ADDRESS_S destAddr = {0};
	int addrLen = SmsPluginParamCodec::decodeAddress(&pTpdu[pos], &destAddr);

	pos += addrLen;

	// TP-PID
	pos++;

	// TP-DCS
	SMS_DCS_S dcs = {0};

	int dcsLen = SmsPluginParamCodec::decodeDCS(&pTpdu[pos], &dcs);

	if (bIsPackingRequired == true)
	{
		dcs.codingScheme = SMS_CHARSET_7BIT;

		char* pDcs = NULL;
		AutoPtr<char> dcsBuf(&pDcs);

		SmsPluginParamCodec::encodeDCS(&dcs, &pDcs);

		memcpy(&(pTpdu[pos]), pDcs, dcsLen);
	}

	pos++;

	// TP-VP
	if (vpf == SMS_VPF_RELATIVE)
	{
		pos += MAX_REL_TIME_PARAM_LEN;
	}
	else if (vpf == SMS_VPF_ABSOLUTE)
	{
		pos += MAX_ABS_TIME_PARAM_LEN;
	}

	// TP-UDL
	int udl = pTpdu[pos];
	int retLen = 0;

	if (bIsPackingRequired == true)
	{
		SMS_USERDATA_S userData = {0};

		userData.headerCnt = 0;
		userData.length = udl;
		memcpy(userData.data, &pTpdu[pos+1], udl);
		userData.data[udl] = '\0';

MSG_DEBUG("user data : [%s]", userData.data);

		int encodeSize = SmsPluginUDCodec::encodeUserData(&userData, dcs.codingScheme, (char*)&pTpdu[pos]);

		retLen = pos + encodeSize;
	}
	else
	{
		retLen = TpduLen;
	}

	return retLen;
}


void SmsPluginSatHandler::sendResult(SMS_SAT_CMD_TYPE_T CmdType, int ResultType)
{
	TelSatAppsRetInfo_t satRetInfo;

	satRetInfo.commandId = commandId;

	MSG_DEBUG("satRetInfo.commandId [%d]", satRetInfo.commandId);

	if (CmdType == SMS_SAT_CMD_REFRESH)
	{
		satRetInfo.commandType = TAPI_SAT_CMD_TYPE_REFRESH;

		satRetInfo.appsRet.refresh.appType = TAPI_SAT_REFRESH_MSG;
		satRetInfo.appsRet.refresh.resp = (TelSatResultType_t)ResultType;
	}
	else if (CmdType == SMS_SAT_CMD_SEND_SMS)
	{
		satRetInfo.commandType = TAPI_SAT_CMD_TYPE_SEND_SMS;

		satRetInfo.appsRet.sendSms.resp = (TelSatResultType_t)ResultType;
	}

	int tapiRet = TAPI_API_SUCCESS;

	tapiRet = tel_send_sat_app_exec_result(pTapiHandle, &satRetInfo);

	if (tapiRet == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("TelTapiSatSendAppExecutionResult() SUCCESS");
	}
	else
	{
		MSG_DEBUG("TelTapiSatSendAppExecutionResult() FAIL [%d]", tapiRet);
	}

	bSendSms = false;
	bInitSim = false;
	bSMSPChanged = false;
	bCBMIChanged = false;
}

