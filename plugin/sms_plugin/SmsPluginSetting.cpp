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

#include <errno.h>

#include "MsgDebug.h"
#include "MsgException.h"
#include "MsgGconfWrapper.h"
#include "SmsPluginParamCodec.h"
#include "SmsPluginSetting.h"


extern "C"
{
#ifndef _TAPI_NETTEXT_H_
	#include "ITapiNetText.h"
	#include "ITapiSim.h"
#endif
}


/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginSetting - Member Functions
==================================================================================================*/
SmsPluginSetting* SmsPluginSetting::pInstance = NULL;


SmsPluginSetting::SmsPluginSetting()
{
	// Initialize member variables
	memset(&smscData, 0x00, sizeof(MSG_SMSC_DATA_S));
	memset(&cbOpt, 0x00, sizeof(MSG_CBMSG_OPT_S));

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

	MSG_ERROR_T	err = MSG_SUCCESS;

	// Init SMS Parameter
	int paramCnt = 0;
	int failCnt = 0;

	paramCnt = getParamCount();

	MSG_DEBUG("Parameter Count [%d]", paramCnt);

	MSG_SMSC_DATA_S tmpSmscData = {};
	MSG_SMSC_LIST_S tmpSmscList = {};

	for (int index = 0; index < paramCnt; index++)
	{
		if (getParam(index, &tmpSmscData) == false)
		{
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
	tmpSmscList.selected = selectedParam;

	if (paramCnt > 0)
	{
		err = addSMSCList(&tmpSmscList);

		if (err == MSG_SUCCESS)
		{
			MSG_DEBUG("########  Add SMSC List Success !!! #######");
		}
		else
		{
			MSG_DEBUG("########  Add SMSC List Fail !!! return : %d #######", err);
		}
	}

	// Init CB Config
	if (SimStatus == MSG_SIM_STATUS_CHANGED)
	{
		MSG_DEBUG("simStatus == MSG_SIM_STATUS_CHANGED");

		MSG_CBMSG_OPT_S cbMsgOpt = {};

		if (getCbConfig(&cbMsgOpt) == true)
		{
			err = addCbOpt(&cbMsgOpt);

			if (err == MSG_SUCCESS)
			{
				MSG_DEBUG("########  Add CB Option Success !!! #######");
			}
			else
			{
				MSG_DEBUG("########  Add CB Option Fail !!! return : %d #######", err);
			}
		}
	}
	else if (SimStatus == MSG_SIM_STATUS_NORMAL)
	{
		MSG_DEBUG("simStatus == MSG_SIM_STATUS_NORMAL");

		// Set CB Data into SIM in case of same SIM
		MSG_SETTING_S cbSetting;
		cbSetting.type = MSG_CBMSG_OPT;

		getCbOpt(&cbSetting);

		setCbConfig(&(cbSetting.option.cbMsgOpt));
	}

	MSG_END();
}


void SmsPluginSetting::setConfigData(const MSG_SETTING_S *pSetting)
{
	MSG_DEBUG("Setting Type : [%d]", pSetting->type);

	switch (pSetting->type)
	{
		case MSG_SMSC_LIST :
			setParamList(&pSetting->option.smscList);
		break;

		case MSG_CBMSG_OPT :
			setCbConfig(&pSetting->option.cbMsgOpt);
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


MSG_ERROR_T SmsPluginSetting::addSMSCList(MSG_SMSC_LIST_S *pSmscList)
{
	MSG_ERROR_T err = MSG_SUCCESS;

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

	if (MsgSettingSetInt(SMSC_TOTAL_COUNT, pSmscList->totalCnt) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", SMSC_TOTAL_COUNT);
		return MSG_ERR_SET_SETTING;
	}

	if (MsgSettingSetInt(SMSC_SELECTED, pSmscList->selected) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", SMSC_SELECTED);
		return MSG_ERR_SET_SETTING;
	}

	for (int i = 0; i < pSmscList->totalCnt; i++)
	{
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

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", keyName);
	}

	return err;
}


MSG_ERROR_T SmsPluginSetting::addCbOpt(MSG_CBMSG_OPT_S *pCbOpt)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_DEBUG("Receive [%d], All Channel [%d], Max SIM Count [%d]", pCbOpt->bReceive, pCbOpt->bAllChannel, pCbOpt->maxSimCnt);

	MSG_DEBUG("Channel Count [%d]", pCbOpt->channelData.channelCnt);

	for (int i = 0; i < pCbOpt->channelData.channelCnt; i++)
	{
		MSG_DEBUG("Channel ID [%d]", pCbOpt->channelData.channelInfo[i].id);
	}

	// Set Setting Data into Vconf
	if (MsgSettingSetBool(CB_RECEIVE, pCbOpt->bReceive) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", CB_RECEIVE);
		return MSG_ERR_SET_SETTING;
	}

	if (MsgSettingSetBool(CB_ALL_CHANNEL, pCbOpt->bAllChannel) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", CB_ALL_CHANNEL);
		return MSG_ERR_SET_SETTING;
	}

	if (MsgSettingSetInt(CB_MAX_SIM_COUNT, pCbOpt->maxSimCnt) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", CB_MAX_SIM_COUNT);
		return MSG_ERR_SET_SETTING;
	}

	if (MsgSettingSetInt(CB_CHANNEL_COUNT, pCbOpt->channelData.channelCnt) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", CB_CHANNEL_COUNT);
		return MSG_ERR_SET_SETTING;
	}

	char keyName[128];

	for (int i = 0; i < pCbOpt->channelData.channelCnt; i++)
	{
		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ACTIVATE, i);

		if ((err = MsgSettingSetBool(keyName, pCbOpt->channelData.channelInfo[i].bActivate)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ID, i);

		if ((err = MsgSettingSetInt(keyName, pCbOpt->channelData.channelInfo[i].id)) != MSG_SUCCESS)
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

	MsgSettingGetBool(CB_ALL_CHANNEL, &pSetting->option.cbMsgOpt.bAllChannel);

	pSetting->option.cbMsgOpt.maxSimCnt = MsgSettingGetInt(CB_MAX_SIM_COUNT);

	pSetting->option.cbMsgOpt.channelData.channelCnt = MsgSettingGetInt(CB_CHANNEL_COUNT);

	for (int i = 0; i < pSetting->option.cbMsgOpt.channelData.channelCnt; i++)
	{
		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ACTIVATE, i);

		MsgSettingGetBool(keyName, &pSetting->option.cbMsgOpt.channelData.channelInfo[i].bActivate);

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ID, i);

		pSetting->option.cbMsgOpt.channelData.channelInfo[i].id = MsgSettingGetInt(keyName);

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

	for (int i = MSG_CBLANG_TYPE_ALL; i < MSG_CBLANG_TYPE_MAX; i++)
	{
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
	int reqId = 0;

	for (int index = 0; index < pSMSCList->totalCnt; index++)
	{
		/*Setting the SMSP Record index value*/
		smsParam.RecordIndex = (unsigned char)index;

		/*Setting the SMSP Record Length value = 28 + alphaId_len*/
		smsParam.RecordLen = 28 + strlen(pSMSCList->smscData[index].name);

		/*Setting the SMSP Alpha ID value*/
		smsParam.AlphaIdLen = strlen(pSMSCList->smscData[index].name);
		MSG_DEBUG("AlphaIdLen = %ld", smsParam.AlphaIdLen);

		if (smsParam.AlphaIdLen > 0 &&  smsParam.AlphaIdLen <= SMSC_NAME_MAX)
		{
			memcpy(smsParam.szAlphaId, pSMSCList->smscData[index].name, smsParam.AlphaIdLen);
			smsParam.szAlphaId[smsParam.AlphaIdLen] = '\0';
			MSG_DEBUG("szAlphaId = %s", smsParam.szAlphaId);
		}

		smsParam.ParamIndicator = 0x00;

		if (strlen(pSMSCList->smscData[index].smscAddr.address) > 0)
		{
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
		}
		else
		{
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

		ret = tel_set_sms_parameters((const TelSmsParams_t*)&smsParam, &reqId);

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
	int reqId = 0;

	MSG_SMSC_DATA_S tmpSmscData = {};

	for (int index = 0; index < paramCnt; index++)
	{
		ret = tel_get_sms_parameters(index, &reqId);

		if (ret == TAPI_API_SUCCESS)
		{
			MSG_DEBUG("######## tel_get_sms_parameters() Success !!! #######");
		}
		else
		{
			THROW(MsgException::SMS_PLG_ERROR, "######## tel_get_sms_parameters() Fail !!! return : %d #######", ret);
		}

		if (getParamEvent(&tmpSmscData) == true)
		{
			MSG_DEBUG("######## Get Parameter was Successful !!! #######");
		}
		else
		{
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
	int reqId = 0;

	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sms_parameter_count(&reqId);

	if (ret == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("######## tel_get_sms_parameter_count() Success !!! #######");
	}
	else
	{
		THROW(MsgException::SMS_PLG_ERROR, "tel_get_sms_parameter_count() Error. [%d]", ret);
	}

	return getParamCntEvent();
}


bool SmsPluginSetting::getParam(int Index, MSG_SMSC_DATA_S *pSmscData)
{
	int reqId = 0;

	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sms_parameters(Index, &reqId);

	if (ret == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("######## tel_get_sms_parameters() Success !!! #######");
	}
	else
	{
		MSG_DEBUG("######## tel_get_sms_parameters() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getParamEvent(pSmscData) == true)
	{
		MSG_DEBUG("######## Get Parameter was Successful !!! #######");
	}
	else
	{
	 	MSG_DEBUG("######## Get Parameter was Failed !!! #######");
		return false;
	}

	return true;
}


bool SmsPluginSetting::setCbConfig(const MSG_CBMSG_OPT_S *pCbOpt)
{
	int reqId = 0;

	int ret = TAPI_API_SUCCESS;

	TelSmsCbConfig_t cbConfig = {};

	cbConfig.bCBEnabled = (int)pCbOpt->bReceive;

	if (pCbOpt->bAllChannel == true)
		cbConfig.SelectedId = 0x01;
	else
		cbConfig.SelectedId = 0x02;

	cbConfig.MsgIdCount = pCbOpt->channelData.channelCnt;

	for (int i = 0; i < cbConfig.MsgIdCount; i++)
	{
		cbConfig.MsgIDs[i] = (unsigned short)pCbOpt->channelData.channelInfo[i].id;
	}

	ret = tel_set_sms_cb_config(&cbConfig, &reqId);

	if (ret == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("######## tel_set_sms_cb_config() Success !!! #######");
	}
	else
	{
		MSG_DEBUG("######## tel_set_sms_cb_config() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getResultFromSim() == true)
	{
		MSG_DEBUG("######## Set Cb Config was Successful !!! #######");
	}
	else
	{
	 	MSG_DEBUG("######## Set Cb Config was Failed !!! #######");
		return false;
	}

	return true;
}


bool SmsPluginSetting::getCbConfig(MSG_CBMSG_OPT_S *pCbOpt)
{
	int reqId = 0;

	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sms_cb_config(&reqId);

	if (ret == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("######## tel_get_sms_cb_config() Success !!! #######");
	}
	else
	{
		MSG_DEBUG("######## tel_get_sms_cb_config() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getCbConfigEvent(pCbOpt) == true)
	{
		MSG_DEBUG("######## Get Cb Config was Successful !!! #######");
	}
	else
	{
	 	MSG_DEBUG("######## Get Cb Config was Failed !!! #######");
		return false;
	}

	return true;
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

	if (ret == ETIMEDOUT)
	{
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

	if (bTapiResult == true)
	{
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

	bTapiResult = false;

	mx.lock();

	ret = cv.timedwait(mx.pMutex(), 10);

	mx.unlock();

	if (ret == ETIMEDOUT)
	{
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	memset(pSmscData, 0x00, sizeof(MSG_SMSC_DATA_S));

	if (bTapiResult == true)
	{
		memcpy(pSmscData, &smscData, sizeof(MSG_SMSC_DATA_S));
	}

	return bTapiResult;
}


void SmsPluginSetting::setCbConfigEvent(const MSG_CBMSG_OPT_S *pCbOpt, bool bSuccess)
{
	mx.lock();

	bTapiResult = bSuccess;

	memset(&cbOpt, 0x00, sizeof(MSG_CBMSG_OPT_S));

	if (bTapiResult == true)
	{
		MSG_DEBUG("Success to get cb config data");

		memcpy(&cbOpt, pCbOpt, sizeof(MSG_CBMSG_OPT_S));
	}

	cv.signal();

	mx.unlock();
}


bool SmsPluginSetting::getCbConfigEvent(MSG_CBMSG_OPT_S *pCbOpt)
{
	int ret = 0;

	bTapiResult = false;

	mx.lock();

	ret = cv.timedwait(mx.pMutex(), 10);

	mx.unlock();

	if (ret == ETIMEDOUT)
	{
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	memset(pCbOpt, 0x00, sizeof(MSG_CBMSG_OPT_S));

	if (bTapiResult == true)
	{
		memcpy(pCbOpt, &cbOpt, sizeof(MSG_CBMSG_OPT_S));
	}

	return bTapiResult;
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

	if (ret == ETIMEDOUT)
	{
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

