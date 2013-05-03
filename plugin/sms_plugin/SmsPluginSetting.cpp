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

#include <errno.h>

#include "MsgDebug.h"
#include "MsgException.h"
#include "MsgGconfWrapper.h"
#include "MsgNotificationWrapper.h"
#include "MsgSoundPlayer.h"

#include "SmsPluginParamCodec.h"
#include "SmsPluginCallback.h"
#include "SmsPluginSetting.h"


extern "C"
{
	#include <tapi_common.h>
	#include <TelSms.h>
	#include <TapiUtility.h>
	#include <ITapiNetText.h>
	#include <ITapiSim.h>
}

extern struct tapi_handle *pTapiHandle;

/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginSetting - Member Functions
==================================================================================================*/
SmsPluginSetting* SmsPluginSetting::pInstance = NULL;


SmsPluginSetting::SmsPluginSetting()
{
	// Initialize member variables
	memset(&smscData, 0x00, sizeof(MSG_SMSC_DATA_S));
	memset(&cbOpt, 0x00, sizeof(MSG_CBMSG_OPT_S));
	memset(&simMailboxList, 0x00, sizeof(SMS_SIM_MAILBOX_LIST_S));

	bTapiResult = false;
	paramCnt = 0;
	selectedParam = 0;
}


SmsPluginSetting::~SmsPluginSetting()
{


}


SmsPluginSetting* SmsPluginSetting::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginSetting();

	return pInstance;
}


void SmsPluginSetting::initConfigData(MSG_SIM_STATUS_T SimStatus)
{
	MSG_BEGIN();

	msg_error_t	err = MSG_SUCCESS;

	// Init SMS Parameter
	int paramCnt = 0;
	int failCnt = 0;

	paramCnt = getParamCount();

	MSG_DEBUG("Parameter Count [%d]", paramCnt);

	MSG_SMSC_DATA_S tmpSmscData = {};
	MSG_SMSC_LIST_S tmpSmscList = {};

	for (int index = 0; index < paramCnt; index++)
	{
		if (getParam(index, &tmpSmscData) == false) {
			failCnt++;
			continue;
		}

		memcpy(&(tmpSmscList.smscData[index]), &tmpSmscData, sizeof(MSG_SMSC_DATA_S));

		MSG_DEBUG("pid[%d]", tmpSmscList.smscData[index].pid);
		MSG_DEBUG("val_period[%d]", tmpSmscList.smscData[index].valPeriod);
		MSG_DEBUG("name[%s]", tmpSmscList.smscData[index].name);

		MSG_DEBUG("ton[%d]", tmpSmscList.smscData[index].smscAddr.ton);
		MSG_DEBUG("npi[%d]", tmpSmscList.smscData[index].smscAddr.npi);
		MSG_DEBUG("address[%s]", tmpSmscList.smscData[index].smscAddr.address);
	}

	tmpSmscList.totalCnt = (paramCnt - failCnt);
//	below is commented to be the first smsc is selected.
//	tmpSmscList.selected = selectedParam;

	if (paramCnt > 0) {
		err = addSMSCList(&tmpSmscList);

		if (err == MSG_SUCCESS) {
			MSG_DEBUG("########  Add SMSC List Success !!! #######");
		} else {
			MSG_DEBUG("########  Add SMSC List Fail !!! return : %d #######", err);
		}
	}

	// Init CB Config
	if (SimStatus == MSG_SIM_STATUS_CHANGED) {
		MSG_DEBUG("simStatus == MSG_SIM_STATUS_CHANGED");

		MSG_CBMSG_OPT_S cbMsgOpt = {};

		if (getCbConfig(&cbMsgOpt) == true) {
			err = addCbOpt(&cbMsgOpt);

			if (err == MSG_SUCCESS) {
				MSG_DEBUG("########  Add CB Option Success !!! #######");
			} else {
				MSG_DEBUG("########  Add CB Option Fail !!! return : %d #######", err);
			}
		}
	} else if (SimStatus == MSG_SIM_STATUS_NORMAL) {
		MSG_DEBUG("simStatus == MSG_SIM_STATUS_NORMAL");

		// Set CB Data into SIM in case of same SIM
		MSG_SETTING_S cbSetting;
		cbSetting.type = MSG_CBMSG_OPT;

		getCbOpt(&cbSetting);

		setCbConfig(&(cbSetting.option.cbMsgOpt));
	}

	if (SimStatus != MSG_SIM_STATUS_NOT_FOUND)
	{
		MSG_VOICEMAIL_OPT_S tmpVoiceMail;
		memset(&tmpVoiceMail, 0x00, sizeof(MSG_VOICEMAIL_OPT_S));

		if (getVoiceMailInfo(&tmpVoiceMail) == true) {
			MSG_DEBUG("########  getVoiceMailInfo Success !!! #######");
		} else {
			MSG_DEBUG("########  getVoiceMailInfo Fail !!! #######");
		}

		if (getMwiInfo() == true) {
			MSG_DEBUG("########  getMwiInfo Success !!! #######");
		} else {
			MSG_DEBUG("########  getMwiInfo Fail !!! #######");
		}

		if (getMsisdnInfo() == true) {
			MSG_DEBUG("########  getMsisdnInfo Success !!! #######");
		} else {
			MSG_DEBUG("########  getMsisdnInfo Fail !!! #######");
		}
	}

	MSG_END();
}


void SmsPluginSetting::setConfigData(const MSG_SETTING_S *pSetting)
{
	MSG_DEBUG("Setting Type : [%d]", pSetting->type);

	switch (pSetting->type)
	{
#if 0
		case MSG_SMS_SENDOPT :
			setNetworkMode(&pSetting->option.smsSendOpt);
			break;
#endif
		case MSG_SMSC_LIST :
			setParamList(&pSetting->option.smscList);
			break;
		case MSG_CBMSG_OPT :
			setCbConfig(&pSetting->option.cbMsgOpt);
			break;
		case MSG_VOICEMAIL_OPT:
			setVoiceMailInfo(&pSetting->option.voiceMailOpt);
			break;
		default :
			THROW(MsgException::SMS_PLG_ERROR, "The Setting type is not supported. [%d]", pSetting->type);
			break;
	}
}


void SmsPluginSetting::getConfigData(MSG_SETTING_S *pSetting)
{
	MSG_DEBUG("Setting Type : [%d]", pSetting->type);

	switch (pSetting->type)
	{
		case MSG_SMSC_LIST :
			getParamList(&pSetting->option.smscList);
		break;

		case MSG_CBMSG_OPT :
			getCbConfig(&pSetting->option.cbMsgOpt);
		break;

		default :
			THROW(MsgException::SMS_PLG_ERROR, "The Setting type is not supported. [%d]", pSetting->type);
		break;
	}
}


msg_error_t SmsPluginSetting::addSMSCList(MSG_SMSC_LIST_S *pSmscList)
{
	msg_error_t err = MSG_SUCCESS;

	MSG_DEBUG("total_count[%d]", pSmscList->totalCnt);
	MSG_DEBUG("selected index[%d]", pSmscList->selected);

	for (int i = 0; i < pSmscList->totalCnt; i++)
	{
		MSG_DEBUG("pid[%d]", pSmscList->smscData[i].pid);
		MSG_DEBUG("val_period[%d]", pSmscList->smscData[i].valPeriod);
		MSG_DEBUG("name[%s]", pSmscList->smscData[i].name);

		MSG_DEBUG("ton[%d]", pSmscList->smscData[i].smscAddr.ton);
		MSG_DEBUG("npi[%d]", pSmscList->smscData[i].smscAddr.npi);
		MSG_DEBUG("address[%s]", pSmscList->smscData[i].smscAddr.address);
	}

	char keyName[128];

	if (MsgSettingSetInt(SMSC_TOTAL_COUNT, pSmscList->totalCnt) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", SMSC_TOTAL_COUNT);
		return MSG_ERR_SET_SETTING;
	}

	if (MsgSettingSetInt(SMSC_SELECTED, pSmscList->selected) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", SMSC_SELECTED);
		return MSG_ERR_SET_SETTING;
	}

	for (int i = 0; i < pSmscList->totalCnt; i++) {
		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", SMSC_PID, i);

		if ((err = MsgSettingSetInt(keyName, (int)pSmscList->smscData[i].pid)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", SMSC_VAL_PERIOD, i);

		if ((err = MsgSettingSetInt(keyName, (int)pSmscList->smscData[i].valPeriod)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", SMSC_NAME, i);

		if ((err = MsgSettingSetString(keyName, pSmscList->smscData[i].name)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", SMSC_TON, i);

		if (pSmscList->smscData[i].smscAddr.address[0] == '+')
			pSmscList->smscData[i].smscAddr.ton = MSG_TON_INTERNATIONAL;
		else
			pSmscList->smscData[i].smscAddr.ton = MSG_TON_NATIONAL;

		if ((err = MsgSettingSetInt(keyName, (int)pSmscList->smscData[i].smscAddr.ton)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", SMSC_NPI, i);

		pSmscList->smscData[i].smscAddr.npi = MSG_NPI_ISDN; // app cannot set this value

		if ((err = MsgSettingSetInt(keyName, (int)pSmscList->smscData[i].smscAddr.npi)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", SMSC_ADDRESS, i);

		if ((err = MsgSettingSetString(keyName, pSmscList->smscData[i].smscAddr.address)) != MSG_SUCCESS)
			break;
	}

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", keyName);
	}

	return err;
}


msg_error_t SmsPluginSetting::addCbOpt(MSG_CBMSG_OPT_S *pCbOpt)
{
	msg_error_t err = MSG_SUCCESS;

	MSG_DEBUG("Receive [%d], Max SIM Count [%d]", pCbOpt->bReceive, pCbOpt->maxSimCnt);

	MSG_DEBUG("Channel Count [%d]", pCbOpt->channelData.channelCnt);

	for (int i = 0; i < pCbOpt->channelData.channelCnt; i++)
	{
		MSG_DEBUG("Channel FROM [%d], Channel TO [%d]", pCbOpt->channelData.channelInfo[i].from, pCbOpt->channelData.channelInfo[i].to);
	}

	// Set Setting Data into Vconf
	if (MsgSettingSetBool(CB_RECEIVE, pCbOpt->bReceive) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", CB_RECEIVE);
		return MSG_ERR_SET_SETTING;
	}

	if (MsgSettingSetInt(CB_MAX_SIM_COUNT, pCbOpt->maxSimCnt) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", CB_MAX_SIM_COUNT);
		return MSG_ERR_SET_SETTING;
	}

	if (MsgSettingSetInt(CB_CHANNEL_COUNT, pCbOpt->channelData.channelCnt) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", CB_CHANNEL_COUNT);
		return MSG_ERR_SET_SETTING;
	}

	char keyName[128];

	for (int i = 0; i < pCbOpt->channelData.channelCnt; i++) {
		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ACTIVATE, i);

		if ((err = MsgSettingSetBool(keyName, pCbOpt->channelData.channelInfo[i].bActivate)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ID_FROM, i);

		if ((err = MsgSettingSetInt(keyName, pCbOpt->channelData.channelInfo[i].from)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ID_TO, i);

		if ((err = MsgSettingSetInt(keyName, pCbOpt->channelData.channelInfo[i].to)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_NAME, i);

		if ((err = MsgSettingSetString(keyName, pCbOpt->channelData.channelInfo[i].name)) != MSG_SUCCESS)
			break;
	}

	return err;
}


void SmsPluginSetting::getCbOpt(MSG_SETTING_S *pSetting)
{
	char keyName[128];
	char *tmpValue = NULL;

	memset(&(pSetting->option.cbMsgOpt), 0x00, sizeof(MSG_CBMSG_OPT_S));

	MsgSettingGetBool(CB_RECEIVE, &pSetting->option.cbMsgOpt.bReceive);

	pSetting->option.cbMsgOpt.maxSimCnt = MsgSettingGetInt(CB_MAX_SIM_COUNT);

	pSetting->option.cbMsgOpt.channelData.channelCnt = MsgSettingGetInt(CB_CHANNEL_COUNT);

	for (int i = 0; i < pSetting->option.cbMsgOpt.channelData.channelCnt; i++) {
		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ACTIVATE, i);

		MsgSettingGetBool(keyName, &pSetting->option.cbMsgOpt.channelData.channelInfo[i].bActivate);

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ID_FROM, i);

		pSetting->option.cbMsgOpt.channelData.channelInfo[i].from = MsgSettingGetInt(keyName);

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ID_TO, i);

		pSetting->option.cbMsgOpt.channelData.channelInfo[i].to = MsgSettingGetInt(keyName);

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_NAME, i);

		tmpValue = MsgSettingGetString(keyName);

		if (tmpValue != NULL)
		{
			strncpy(pSetting->option.cbMsgOpt.channelData.channelInfo[i].name, tmpValue, CB_CHANNEL_NAME_MAX);
			free(tmpValue);
			tmpValue = NULL;
		}
	}

	for (int i = MSG_CBLANG_TYPE_ALL; i < MSG_CBLANG_TYPE_MAX; i++) {
		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_LANGUAGE, i);

		MsgSettingGetBool(keyName, &pSetting->option.cbMsgOpt.bLanguage[i]);
	}

}


void SmsPluginSetting::setParamList(const MSG_SMSC_LIST_S *pSMSCList)
{
	MSG_BEGIN();

	TelSmsParams_t smsParam = {0};

	int ret = TAPI_API_SUCCESS;

	for (int index = 0; index < pSMSCList->totalCnt; index++) {
		/*Setting the SMSP Record index value*/
		smsParam.RecordIndex = (unsigned char)index;

		/*Setting the SMSP Record Length value = 28 + alphaId_len*/
		smsParam.RecordLen = 28 + strlen(pSMSCList->smscData[index].name);

		/*Setting the SMSP Alpha ID value*/
		smsParam.AlphaIdLen = strlen(pSMSCList->smscData[index].name);
		MSG_DEBUG("AlphaIdLen = %ld", smsParam.AlphaIdLen);

		if (smsParam.AlphaIdLen > 0 &&  smsParam.AlphaIdLen <= SMSC_NAME_MAX) {
			memcpy(smsParam.szAlphaId, pSMSCList->smscData[index].name, smsParam.AlphaIdLen);
			smsParam.szAlphaId[smsParam.AlphaIdLen] = '\0';
			MSG_DEBUG("szAlphaId = %s", smsParam.szAlphaId);
		}

		smsParam.ParamIndicator = 0x00;

		if (strlen(pSMSCList->smscData[index].smscAddr.address) > 0) {
			smsParam.ParamIndicator |= 0x02 ;  //enable 2nd Bit
			MSG_DEBUG("ParamIndicator = [%02x]", smsParam.ParamIndicator);

			if (pSMSCList->smscData[index].smscAddr.address[0] == '+')
				smsParam.TpSvcCntrAddr.Ton = TAPI_SIM_TON_INTERNATIONAL;
			else
				smsParam.TpSvcCntrAddr.Ton = TAPI_SIM_TON_NATIONAL;

			smsParam.TpSvcCntrAddr.Npi = TAPI_SIM_NPI_ISDN_TEL; // app cannot set this value

			MSG_DEBUG("SMSC TON = [%d] NPI = [%d]", smsParam.TpSvcCntrAddr.Ton, smsParam.TpSvcCntrAddr.Npi);

			MSG_DEBUG("address = %s", pSMSCList->smscData[index].smscAddr.address);

			smsParam.TpSvcCntrAddr.DialNumLen = SmsPluginParamCodec::encodeSMSC(pSMSCList->smscData[index].smscAddr.address, smsParam.TpSvcCntrAddr.szDiallingNum);
		} else {
			MSG_DEBUG("SMSC Addr is not present");
		}

		/*Setting the PID value*/
		smsParam.ParamIndicator |= 0x04 ;  //enable 3nd Bit
		MSG_DEBUG("ParamIndicator = [%02x]", smsParam.ParamIndicator);

		smsParam.TpProtocolId = (unsigned short)convertPid(pSMSCList->smscData[index].pid);

		/*Setting the ValidityPeriod value*/
		smsParam.ParamIndicator |= 0x10 ;  //enable 5nd Bit
		MSG_DEBUG("ParamIndicator = [%02x]", smsParam.ParamIndicator);

		smsParam.TpValidityPeriod = (unsigned short)pSMSCList->smscData[index].valPeriod;

		smsParam.ParamIndicator = ~(smsParam.ParamIndicator);
		MSG_DEBUG("ParamIndicator = [%02x]", smsParam.ParamIndicator);

		ret = tel_set_sms_parameters(pTapiHandle, (const TelSmsParams_t*)&smsParam, TapiEventSetConfigData, NULL);

		if (ret != TAPI_API_SUCCESS)
			THROW(MsgException::SMS_PLG_ERROR, "tel_set_sms_parameters() Error. [%d]", ret);

		if (!getResultFromSim())
			THROW(MsgException::SMS_PLG_ERROR, "tel_set_sms_parameters() Result Error.");
	}

	MSG_END();
}


void SmsPluginSetting::getParamList(MSG_SMSC_LIST_S *pSMSCList)
{
	MSG_BEGIN();

	int paramCnt = 0;

	paramCnt = getParamCount();

	MSG_DEBUG("Parameter Count [%d]", paramCnt);

	int ret = TAPI_API_SUCCESS;

	MSG_SMSC_DATA_S tmpSmscData = {};

	for (int index = 0; index < paramCnt; index++) {
		ret = tel_get_sms_parameters(pTapiHandle, index, TapiEventGetParam, NULL);

		if (ret == TAPI_API_SUCCESS) {
			MSG_DEBUG("######## tel_get_sms_parameters() Success !!! #######");
		} else {
			THROW(MsgException::SMS_PLG_ERROR, "######## tel_get_sms_parameters() Fail !!! return : %d #######", ret);
		}

		if (getParamEvent(&tmpSmscData) == true) {
			MSG_DEBUG("######## Get Parameter was Successful !!! #######");
		} else {
		 	THROW(MsgException::SMS_PLG_ERROR, "######## Get Parameter was Failed !!! #######");
		}

		memcpy(&(pSMSCList->smscData[index]), &tmpSmscData, sizeof(MSG_SMSC_DATA_S));

		MSG_DEBUG("pid[%d]", pSMSCList->smscData[index].pid);
		MSG_DEBUG("val_period[%d]", pSMSCList->smscData[index].valPeriod);
		MSG_DEBUG("name[%s]", pSMSCList->smscData[index].name);

		MSG_DEBUG("ton[%d]", pSMSCList->smscData[index].smscAddr.ton);
		MSG_DEBUG("npi[%d]", pSMSCList->smscData[index].smscAddr.npi);
		MSG_DEBUG("address[%s]", pSMSCList->smscData[index].smscAddr.address);
	}

	pSMSCList->totalCnt = paramCnt;
	pSMSCList->selected = selectedParam;

	MSG_DEBUG("total_count[%d]", pSMSCList->totalCnt);

	MSG_END();
}


int SmsPluginSetting::getParamCount()
{
	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sms_parameter_count(pTapiHandle, TapiEventGetParamCnt, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_get_sms_parameter_count() Success !!! #######");
	} else {
		THROW(MsgException::SMS_PLG_ERROR, "tel_get_sms_parameter_count() Error. [%d]", ret);
	}

	return getParamCntEvent();
}


bool SmsPluginSetting::getParam(int Index, MSG_SMSC_DATA_S *pSmscData)
{
	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sms_parameters(pTapiHandle, Index, TapiEventGetParam, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_get_sms_parameters() Success !!! #######");
	} else {
		MSG_DEBUG("######## tel_get_sms_parameters() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getParamEvent(pSmscData) == true) {
		MSG_DEBUG("######## Get Parameter was Successful !!! #######");
	} else {
	 	MSG_DEBUG("######## Get Parameter was Failed !!! #######");
		return false;
	}

	return true;
}


bool SmsPluginSetting::setCbConfig(const MSG_CBMSG_OPT_S *pCbOpt)
{
	int ret = TAPI_API_SUCCESS;

	TelSmsCbConfig_t cbConfig = {};

	cbConfig.CBEnabled = (int)pCbOpt->bReceive;
	cbConfig.Net3gppType = TAPI_NETTEXT_NETTYPE_3GPP;
	cbConfig.MsgIdMaxCount = pCbOpt->maxSimCnt;
	cbConfig.MsgIdRangeCount = pCbOpt->channelData.channelCnt;

	for (int i = 0; i < cbConfig.MsgIdRangeCount; i++) {
		cbConfig.MsgIDs[i].Net3gpp.Selected = (unsigned short)pCbOpt->channelData.channelInfo[i].bActivate;
		cbConfig.MsgIDs[i].Net3gpp.FromMsgId = (unsigned short)pCbOpt->channelData.channelInfo[i].from;
		cbConfig.MsgIDs[i].Net3gpp.ToMsgId = (unsigned short)pCbOpt->channelData.channelInfo[i].to;

		MSG_DEBUG("FROM: %d, TO: %d", cbConfig.MsgIDs[i].Net3gpp.FromMsgId, cbConfig.MsgIDs[i].Net3gpp.ToMsgId);
	}
	MSG_DEBUG("CBEnabled: %d, range_count: %d", cbConfig.CBEnabled, cbConfig.MsgIdRangeCount);

	ret = tel_set_sms_cb_config(pTapiHandle, &cbConfig, TapiEventSetConfigData, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_set_sms_cb_config() Success !!! #######");
	} else {
		MSG_DEBUG("######## tel_set_sms_cb_config() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getResultFromSim() == true) {
		MSG_DEBUG("######## Set Cb Config was Successful !!! #######");
	} else {
	 	MSG_DEBUG("######## Set Cb Config was Failed !!! #######");
		return false;
	}

	return true;
}


bool SmsPluginSetting::getCbConfig(MSG_CBMSG_OPT_S *pCbOpt)
{
	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sms_cb_config(pTapiHandle, TapiEventGetCBConfig, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_get_sms_cb_config() Success !!! #######");
	} else {
		MSG_DEBUG("######## tel_get_sms_cb_config() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getCbConfigEvent(pCbOpt) == true) {
		MSG_DEBUG("######## Get Cb Config was Successful !!! #######");
	} else {
	 	MSG_DEBUG("######## Get Cb Config was Failed !!! #######");
		return false;
	}

	return true;
}

void SmsPluginSetting::setVoiceMailInfo(const MSG_VOICEMAIL_OPT_S *pVoiceOpt)
{
	int ret = TAPI_API_SUCCESS;

	TelSimMailBoxNumber_t mailboxInfo = {0,};

	for (int i = 0; i < simMailboxList.count; i++) {
		if (simMailboxList.list[i].mb_type == TAPI_SIM_MAILBOX_VOICE) {
			memset(&simMailboxList.list[i].num, 0x00, sizeof(simMailboxList.list[i].num));
			snprintf(simMailboxList.list[i].num, sizeof(simMailboxList.list[i].num), "%s", pVoiceOpt->mailNumber);
			MSG_DEBUG("Mailbox number config [%s]", simMailboxList.list[i].num);

			mailboxInfo.b_cphs = simMailboxList.list[i].b_cphs;
			mailboxInfo.alpha_id_max_len = simMailboxList.list[i].alpha_id_max_len;
			mailboxInfo.mb_type = (TelSimMailboxType_t)simMailboxList.list[i].mb_type;
			mailboxInfo.profile_num = simMailboxList.list[i].profile_num;
			mailboxInfo.rec_index = simMailboxList.list[i].rec_index;
			mailboxInfo.ton = (TelSimTypeOfNum_t)simMailboxList.list[i].ton;
			mailboxInfo.npi = (TelSimNumberingPlanIdentity_t)simMailboxList.list[i].npi;
			snprintf(mailboxInfo.alpha_id, sizeof(mailboxInfo.alpha_id), "%s", simMailboxList.list[i].alpha_id);
			snprintf(mailboxInfo.num, sizeof(mailboxInfo.num), "%s", simMailboxList.list[i].num);
			mailboxInfo.cc_id = simMailboxList.list[i].cc_id;
			mailboxInfo.ext1_id = simMailboxList.list[i].ext1_id;

			break;
		}
	}

	ret = tel_set_sim_mailbox_info(pTapiHandle, &mailboxInfo, TapiEventSetMailboxInfo, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_set_sim_mailbox_info() Success !!! #######");
	} else {
		MSG_DEBUG("######## tel_set_sim_mailbox_info() Fail !!! return : %d #######", ret);
	}

	if (getResultFromSim() == true) {
		MSG_DEBUG("######## Set mailbox info Success !!! #######");
	} else {
		THROW(MsgException::SMS_PLG_ERROR, "########  Set mailbox info Failed !!!#######");
	}

	return;
}

bool SmsPluginSetting::getVoiceMailInfo(MSG_VOICEMAIL_OPT_S *pVoiceOpt)
{
	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sim_mailbox_info(pTapiHandle, TapiEventGetMailboxInfo, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_get_sim_mailbox_info() Success !!! #######");
	} else {
		MSG_DEBUG("######## tel_get_sim_mailbox_info() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getMailboxInfoEvent(pVoiceOpt) == true) {
		MSG_DEBUG("######## Get MWI info was Successful !!! #######");
	} else {
	 	MSG_DEBUG("######## Get MWI info was Failed !!! #######");
		return false;
	}

	return true;
}

void SmsPluginSetting::setMwiInfo(MSG_SUB_TYPE_T type, int count)
{
	if (type < MSG_MWI_VOICE_SMS || type > MSG_MWI_OTHER_SMS) {
		MSG_DEBUG("Invalid parameter");
		return;
	}

	int ret = TAPI_API_SUCCESS;

	TelSimMessageWaitingReq_t mwReq = {0,};

	if (simMwiInfo.b_cphs) {
		if (type == MSG_MWI_VOICE_SMS)
			simMwiInfo.cphs_mwi.b_voice1 = true;
		else if (type == MSG_MWI_FAX_SMS)
			simMwiInfo.cphs_mwi.b_fax = true;
		else
			MSG_DEBUG("There is no type [%d] in CPHS.", type);

		mwReq.mw_data_u.cphs_mw.b_voice1 = simMwiInfo.cphs_mwi.b_voice1;
		mwReq.mw_data_u.cphs_mw.b_voice2 = simMwiInfo.cphs_mwi.b_voice2;
		mwReq.mw_data_u.cphs_mw.b_fax = simMwiInfo.cphs_mwi.b_fax;
		mwReq.mw_data_u.cphs_mw.b_data = simMwiInfo.cphs_mwi.b_data;
	} else {
		if (type == MSG_MWI_VOICE_SMS)
			simMwiInfo.mwi_list.mw_info[0].voice_count = count;
		else if (type == MSG_MWI_FAX_SMS)
			simMwiInfo.mwi_list.mw_info[0].fax_count = count;
		else if (type == MSG_MWI_EMAIL_SMS)
			simMwiInfo.mwi_list.mw_info[0].email_count = count;
		else // MSG_MWI_OTHER_SMS
			simMwiInfo.mwi_list.mw_info[0].other_count = count;

		mwReq.mw_data_u.mw.rec_index = simMwiInfo.mwi_list.mw_info[0].rec_index;
		mwReq.mw_data_u.mw.indicator_status = simMwiInfo.mwi_list.mw_info[0].indicator_status;
		mwReq.mw_data_u.mw.voice_count = simMwiInfo.mwi_list.mw_info[0].voice_count;
		mwReq.mw_data_u.mw.fax_count = simMwiInfo.mwi_list.mw_info[0].fax_count;
		mwReq.mw_data_u.mw.email_count = simMwiInfo.mwi_list.mw_info[0].email_count;
		mwReq.mw_data_u.mw.other_count = simMwiInfo.mwi_list.mw_info[0].other_count;
		mwReq.mw_data_u.mw.video_count = simMwiInfo.mwi_list.mw_info[0].video_count;
	}

	mwReq.b_cphs = simMwiInfo.b_cphs;

	ret = tel_set_sim_messagewaiting_info(pTapiHandle, &mwReq, TapiEventSetMwiInfo, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_set_sim_messagewaiting_info() Success !!! #######");
	} else {
		MSG_DEBUG("######## tel_set_sim_messagewaiting_info() Fail !!! return : %d #######", ret);
	}

	return;
}


bool SmsPluginSetting::getMwiInfo(void)
{
	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sim_messagewaiting_info(pTapiHandle, TapiEventGetMwiInfo, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_get_sim_messagewaiting_info() Success !!! #######");
	} else {
		MSG_DEBUG("######## tel_get_sim_messagewaiting_info() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getResultFromSim() == true) {
		MSG_DEBUG("######## Get Mainbox info was Successful !!! #######");
	} else {
	 	MSG_DEBUG("######## Get Mainbox info was Failed !!! #######");
		return false;
	}

	return true;
}


bool SmsPluginSetting::getMsisdnInfo(void)
{
	int ret = TAPI_API_SUCCESS;
	bool result = true;

	ret = tel_get_sim_msisdn(pTapiHandle, TapiEventGetMsisdnInfo, NULL);

	if (ret == TAPI_API_SUCCESS) {
		result = true;
		MSG_DEBUG("######## tel_get_sim_msisdn() Success !!! #######");
	} else {
		result = false;
		MSG_DEBUG("######## tel_get_sim_msisdn() Fail !!! return : %d #######", ret);
	}

	return result;
}


void SmsPluginSetting::setParamCntEvent(int ParamCnt)
{
	mx.lock();

	paramCnt = ParamCnt;

	cv.signal();

	mx.unlock();
}


int SmsPluginSetting::getParamCntEvent()
{
	int ret = 0;

	mx.lock();

	ret = cv.timedwait(mx.pMutex(), 10);

	mx.unlock();

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return 0;
	}

	return paramCnt;
}


void SmsPluginSetting::setParamEvent(const MSG_SMSC_DATA_S *pSmscData, int RecordIdx, bool bSuccess)
{
	mx.lock();

	bTapiResult = bSuccess;

	memset(&smscData, 0x00, sizeof(MSG_SMSC_DATA_S));

	if (bTapiResult == true) {
		MSG_DEBUG("Success to get parameter data");

		selectedParam = RecordIdx;

		memcpy(&smscData, pSmscData, sizeof(MSG_SMSC_DATA_S));
	}

	cv.signal();

	mx.unlock();
}


bool SmsPluginSetting::getParamEvent(MSG_SMSC_DATA_S *pSmscData)
{
	int ret = 0;

	mx.lock();

	bTapiResult = false;
	ret = cv.timedwait(mx.pMutex(), 10);

	mx.unlock();

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	memset(pSmscData, 0x00, sizeof(MSG_SMSC_DATA_S));

	if (bTapiResult == true) {
		memcpy(pSmscData, &smscData, sizeof(MSG_SMSC_DATA_S));
	}

	return bTapiResult;
}


void SmsPluginSetting::setCbConfigEvent(const MSG_CBMSG_OPT_S *pCbOpt, bool bSuccess)
{
	mx.lock();

	bTapiResult = bSuccess;

	memset(&cbOpt, 0x00, sizeof(MSG_CBMSG_OPT_S));

	if (bTapiResult == true) {
		MSG_DEBUG("Success to get cb config data");

		memcpy(&cbOpt, pCbOpt, sizeof(MSG_CBMSG_OPT_S));
	}

	cv.signal();

	mx.unlock();
}


bool SmsPluginSetting::getCbConfigEvent(MSG_CBMSG_OPT_S *pCbOpt)
{
	int ret = 0;

	mx.lock();

	bTapiResult = false;
	ret = cv.timedwait(mx.pMutex(), 10);

	mx.unlock();

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	memset(pCbOpt, 0x00, sizeof(MSG_CBMSG_OPT_S));

	if (bTapiResult == true) {
		memcpy(pCbOpt, &cbOpt, sizeof(MSG_CBMSG_OPT_S));
	}

	return bTapiResult;
}


void SmsPluginSetting::setMailboxInfoEvent(SMS_SIM_MAILBOX_LIST_S *pMailboxList, bool bSuccess)
{
	mx.lock();

	bTapiResult = bSuccess;

	memset(&simMailboxList, 0x00, sizeof(SMS_SIM_MAILBOX_LIST_S));

	if (bTapiResult == true) {
		int i = 0;

		if (pMailboxList && pMailboxList->count > 0) {
			memcpy(&simMailboxList, pMailboxList, sizeof(SMS_SIM_MAILBOX_LIST_S));

			/* Temp :: Save voicemail number with VOICE1 line number */
			for (i = 0; i < pMailboxList->count ; i++) {
				MSG_DEBUG("Mailbox list[%d] type=[%d] address = [%s]", i, pMailboxList->list[i].mb_type, pMailboxList->list[i].num);
				if (pMailboxList->list[i].mb_type == TAPI_SIM_MAILBOX_VOICE) {
					char mailNumber[MAX_PHONE_NUMBER_LEN+1];
					memset(mailNumber, 0x00 , sizeof(mailNumber));
					if (simMailboxList.list[i].ton == MSG_TON_INTERNATIONAL && simMailboxList.list[i].num[0] != '+') {
						snprintf(mailNumber, MAX_PHONE_NUMBER_LEN, "+%s", simMailboxList.list[i].num);
						MSG_DEBUG("MSG_TON_INTERNATIONAL [%s]", mailNumber);
					} else {
						snprintf(mailNumber, MAX_PHONE_NUMBER_LEN, "%s", simMailboxList.list[i].num);
						MSG_DEBUG("[%s]", mailNumber);
					}

					if (mailNumber[0] != '\0') {
						if (MsgSettingSetString(VOICEMAIL_NUMBER, mailNumber) != MSG_SUCCESS)
							MSG_DEBUG("MsgSettingSetString is failed!!");
					}
					break;
				}
			}
		}
	}

	cv.signal();

	mx.unlock();
}

bool SmsPluginSetting::getMailboxInfoEvent(MSG_VOICEMAIL_OPT_S *pVoiceOpt)
{
	int ret = 0;

	mx.lock();

	bTapiResult = false;
	ret = cv.timedwait(mx.pMutex(), 10);

	mx.unlock();

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	memset(pVoiceOpt, 0x00, sizeof(MSG_VOICEMAIL_OPT_S));

	if (bTapiResult == true) {
		for(int i = 0; i < simMailboxList.count; i++) {
			if (simMailboxList.list[i].mb_type == MSG_SIM_MAILBOX_VOICE) {
				if (simMailboxList.list[i].ton == MSG_TON_INTERNATIONAL && simMailboxList.list[i].num[0] != '+') {
					snprintf(pVoiceOpt->mailNumber, sizeof(pVoiceOpt->mailNumber), "+%s", simMailboxList.list[i].num);
				} else {
					snprintf(pVoiceOpt->mailNumber, sizeof(pVoiceOpt->mailNumber), "%s", simMailboxList.list[i].num);
				}

				break;
			}
		}
	}

	return bTapiResult;
}

void SmsPluginSetting::setMwiInfoEvent(SMS_SIM_MWI_INFO_S *pMwiInfo, bool bSuccess)
{
	mx.lock();

	bTapiResult = bSuccess;

	memset(&simMwiInfo, 0x00, sizeof(SMS_SIM_MWI_INFO_S));

	if (bTapiResult == true) {
		int mwi_cnt = 0;
		int index = 0;

		memcpy(&simMwiInfo, pMwiInfo, sizeof(SMS_SIM_MWI_INFO_S));

		/* Save MW count with VOICE line number */
		for(int i = 0; i < simMailboxList.count; i++) {

			if (simMailboxList.list[i].mb_type == MSG_SIM_MAILBOX_VOICE) {

				index = simMailboxList.list[i].profile_num - 1;
				if (index < 0) {
					MSG_DEBUG("SIM profile number is invalid.");
					break;
				}

				MSG_DEBUG("SIM MWI profile number=[%d], index=[%d]", simMailboxList.list[i].profile_num, index);

				if (simMwiInfo.b_cphs == false) { // Normal case
					mwi_cnt = simMwiInfo.mwi_list.mw_info[index].voice_count;
				} else { // CPHS case
					/* For CPHS case, mwi_cnt value is boolean */
					mwi_cnt = simMwiInfo.cphs_mwi.b_voice1;
				}

				if (MsgSettingSetInt(VOICEMAIL_COUNT, mwi_cnt) != MSG_SUCCESS)
					MSG_DEBUG("MsgSettingSetInt is failed!!");

				if (mwi_cnt > 0) {
					MSG_MESSAGE_INFO_S msgInfo = {0,};

					msgInfo.displayTime = time(NULL);
					snprintf(msgInfo.addressList[0].addressVal, sizeof(msgInfo.addressList[0].addressVal), \
							"%s", simMailboxList.list[i].num);
					memset(msgInfo.addressList[0].displayName, 0x00, sizeof(msgInfo.addressList[0].displayName));
					msgInfo.msgType.mainType = MSG_SMS_TYPE;
					msgInfo.msgType.subType = MSG_MWI_VOICE_SMS;

					snprintf(msgInfo.msgText, sizeof(msgInfo.msgText), "%d new voice message", mwi_cnt);

					MsgSoundPlayStart(false);
					MsgInsertNoti(&msgInfo);
				}
				break;
			}
		}
	}

	cv.signal();

	mx.unlock();
}

void SmsPluginSetting::setResultFromSim(bool bResult)
{
	mx.lock();

	bTapiResult = bResult;

	cv.signal();

	mx.unlock();
}


bool SmsPluginSetting::getResultFromSim()
{
	int ret = 0;

	mx.lock();

	ret = cv.timedwait(mx.pMutex(), 10);

	mx.unlock();

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	return bTapiResult;
}


SMS_PID_T SmsPluginSetting::convertPid(MSG_SMS_PID_T pid)
{
	SMS_PID_T retPid;

	switch (pid)
	{
		case MSG_PID_TEXT :
			retPid = SMS_PID_NORMAL;
		break;
		case MSG_PID_VOICE :
			retPid = SMS_PID_VOICE;
		break;
		case MSG_PID_FAX :
			retPid = SMS_PID_TELEX;
		break;
		case MSG_PID_X400 :
			retPid = SMS_PID_x400;
		break;
		case MSG_PID_ERMES :
			retPid = SMS_PID_ERMES;
		break;
		case MSG_PID_EMAIL :
			retPid = SMS_PID_EMAIL;
		break;
		default :
			retPid = SMS_PID_NORMAL;
		break;
	}

	return retPid;
}

