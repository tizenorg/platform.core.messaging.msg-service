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
#include <pthread.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgGconfWrapper.h"
#include "MsgNotificationWrapper.h"
#include "MsgSoundPlayer.h"
#include "MsgContact.h"
#include "MsgUtilStorage.h"
#include "MsgTextConvert.h"
#include "MsgDevicedWrapper.h"

#include "SmsPluginParamCodec.h"
#include "SmsPluginCallback.h"
#include "SmsPluginEventHandler.h"
#include "SmsPluginSimMsg.h"
#include "SmsPluginMain.h"
#include "SmsPluginSetting.h"
#include "SmsPluginDSHandler.h"

extern "C"
{
	#include <tapi_common.h>
	#include <TelSms.h>
	#include <TapiUtility.h>
	#include <ITapiNetText.h>
	#include <ITapiSim.h>
	#include <ITapiModem.h>
}

/*==================================================================================================
                                  INTERNAL FUNCTION
==================================================================================================*/


/*==================================================================================================
                   IMPLEMENTATION OF SmsPluginSetting - Member Functions
==================================================================================================*/
SmsPluginSetting* SmsPluginSetting::pInstance = NULL;


SmsPluginSetting::SmsPluginSetting()
{
	// Initialize member variables
	for (int i = 0; i <= MAX_TELEPHONY_HANDLE_CNT; i++) {
		memset(&smscList[i], 0x00, sizeof(MSG_SMSC_LIST_S));
		memset(&smscData[i], 0x00, sizeof(MSG_SMSC_DATA_S));
		memset(&cbOpt[i], 0x00, sizeof(MSG_CBMSG_OPT_S));
		memset(&simMailboxList[i], 0x00, sizeof(SMS_SIM_MAILBOX_LIST_S));
		memset(&simMwiInfo[i], 0x00, sizeof(SMS_SIM_MWI_INFO_S));
		simStatus[i] = MSG_SIM_STATUS_NOT_FOUND;
	}
	memset(&meImei, 0x00, sizeof(meImei));

	bTapiResult = false;
	paramCnt = 0;
	selectedParam = 0;

	for (int i = 0; i < MAX_TELEPHONY_HANDLE_CNT; i++)
		bMbdnEnable[i] = false;
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


void* SmsPluginSetting::initSimInfo(void *data)
{
	static Mutex mm;
	MutexLocker lock(mm);

	SmsPluginSetting::instance()->processInitSimInfo(data);

	return NULL;
}

void* SmsPluginSetting::processInitSimInfo(void *data)
{
	MSG_BEGIN();

	//Handle sim info initialization separately
	struct tapi_handle *handle = (struct tapi_handle *)data;
	instance()->updateSimStatus(handle);

	MSG_END();
	return NULL;
}


void SmsPluginSetting::updateSimStatus(struct tapi_handle *handle)
{
	MSG_BEGIN();

	if (!handle) {
		MSG_DEBUG("handle is NULL.");
		return;
	}

	int status = 0;
	int tapiRet = TAPI_API_SUCCESS;

	char keyName[MAX_VCONFKEY_NAME_LEN];
	memset(keyName, 0x00, sizeof(keyName));

	int simIndex = SmsPluginDSHandler::instance()->getSimIndex(handle);

	// Get IMSI
	TelSimImsiInfo_t imsiInfo;
	memset(&imsiInfo, 0x00, sizeof(TelSimImsiInfo_t));

	tapiRet = tel_get_sim_imsi(handle, &imsiInfo);
	if (tapiRet != TAPI_API_SUCCESS) {
		MSG_DEBUG("tel_get_sim_imsi() Error![%d]", tapiRet);
		snprintf(keyName, sizeof(keyName), "%s/%d", MSG_NATIONAL_SIM, simIndex);
		MsgSettingSetBool(keyName, false);
	}

	/* Save Subcriber ID */
	char *subscriberId = NULL;
	memset(keyName, 0x00, sizeof(keyName));

	if (SmsPluginDSHandler::instance()->getSubscriberId(simIndex, &subscriberId) != MSG_SUCCESS) {
		MSG_DEBUG("getSubscriberId() is failed");
	} else {
		snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_SUBS_ID, simIndex);
		MsgSettingSetString(keyName, subscriberId);
	}

	g_free(subscriberId); subscriberId = NULL;

	/* Check device status */
	tapiRet = tel_check_sms_device_status(handle, &status);

	if (tapiRet != TAPI_API_SUCCESS) {
		MSG_DEBUG("tel_check_sms_device_status() Error! [%d], Status [%d]", tapiRet, status);
		return;
	}

	if (status == TAPI_NETTEXT_READY_STATUS_3GPP) {
		MSG_DEBUG("Device Is Ready");
	} else {
		MSG_DEBUG("Device Is Not Ready.. Waiting For Ready Callback");

		if (SmsPluginEventHandler::instance()->getDeviceStatus(handle) == true) {
			MSG_DEBUG("Device Is Ready");
		} else {
			MSG_DEBUG("Device Is Not Ready.");
			return;
		}
	}

	// init config data.
	initConfigData(handle);

	try
	{
		// init sim messages.
		SmsPluginSimMsg::instance()->initSimMessage(handle);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return;
	}


	MSG_END();

	return;
}


void SmsPluginSetting::initConfigData(struct tapi_handle *handle)
{
	MSG_BEGIN();

	char keyName[MAX_VCONFKEY_NAME_LEN];

	int sim_idx = SmsPluginDSHandler::instance()->getSimIndex(handle);
	/*==================== SMSC setting ====================*/
	// Init SMS Parameter
	int paramCnt = 0;
	int failCnt = 0;
	bool bSelectedFound = false;
	bool bAPReceive = false;

	paramCnt = getParamCount(handle);

	MSG_INFO("Parameter Count [%d]", paramCnt);

	MSG_SMSC_DATA_S tmpSmscData = {};
	MSG_SMSC_LIST_S tmpSmscList = {0,};

	for (int index = 0; index < paramCnt; index++)
	{
		memset(&tmpSmscData, 0x00, sizeof(MSG_SMSC_DATA_S));
		if (getParam(handle, index, &tmpSmscData) == false) {
			failCnt++;
			continue;
		}

		memcpy(&(tmpSmscList.smscData[index]), &tmpSmscData, sizeof(MSG_SMSC_DATA_S));

		MSG_DEBUG("pid[%d]", tmpSmscList.smscData[index].pid);
		MSG_DEBUG("val_period[%d]", tmpSmscList.smscData[index].valPeriod);
		MSG_SEC_DEBUG("name[%s]", tmpSmscList.smscData[index].name);

		MSG_DEBUG("ton[%d]", tmpSmscList.smscData[index].smscAddr.ton);
		MSG_DEBUG("npi[%d]", tmpSmscList.smscData[index].smscAddr.npi);
		MSG_SEC_DEBUG("address[%s]", tmpSmscList.smscData[index].smscAddr.address);

		//First smsc is selected index
		if (!bSelectedFound) {
			tmpSmscList.selected = selectedParam;
			bSelectedFound = !bSelectedFound;
		}
	}

	tmpSmscList.totalCnt = (paramCnt - failCnt);
	tmpSmscList.simIndex = sim_idx;

	if (paramCnt > 0) {
		MSG_DEBUG("########  Add SMSC ist #######");
		addSMSCList(&tmpSmscList);
	}

	/*==================== CB configuration ====================*/
	if (simStatus[sim_idx] != MSG_SIM_STATUS_NOT_FOUND)
	{
		MSG_DEBUG("simStatus == [%d]", simStatus[sim_idx]);

		MSG_CBMSG_OPT_S cbMsgOpt = {0,};
		cbMsgOpt.simIndex = sim_idx;

		if (getCbConfig(&cbMsgOpt) == true) {
			cbMsgOpt.simIndex = sim_idx;

			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, sizeof(keyName), "%s/%d", CB_RECEIVE, sim_idx);
			MsgSettingGetBool(keyName, &bAPReceive);

			if (cbMsgOpt.bReceive == false && bAPReceive == false) {
				MSG_DEBUG("CB is off in CP and in AP. No need to send CB request to CP. ");
			}
			else {
				MSG_DEBUG("########  Add CB Option Success !!! #######");
				MSG_SETTING_S cbSetting;
				getCbOpt(&cbSetting, sim_idx);
				setCbConfig(&(cbSetting.option.cbMsgOpt));
			}
		} else {
			MSG_WARN("########  getCbConfig Fail !!! #######");

#if 0
			// CSC doesn't support CB Info any longer
			if (MsgCscGetCBInfo(&cbMsgOpt) == true) {
				err = addCbOpt(&cbMsgOpt);

				if (err == MSG_SUCCESS) {
					MSG_DEBUG("########  Add CB Option From CSC Success !!! #######");
				} else {
					MSG_DEBUG("########  Add CB Option from CSC Fail !!! return : %d #######", err);
				}
			} else {
				MSG_DEBUG("########  MsgCscGetCBInfo Fail !!! #######");
			}
#endif
		}

		/*==================== Default Voice mail Setting ====================*/
		if (simStatus[sim_idx] == MSG_SIM_STATUS_CHANGED) {
			char keyName[MAX_VCONFKEY_NAME_LEN];

			MSG_DEBUG("=================SIM CHANGED===================");

			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_NUMBER, sim_idx);
			if (MsgSettingSetString(keyName, VOICEMAIL_DEFAULT_NUMBER) != MSG_SUCCESS)
				MSG_DEBUG("MsgSettingSetString is failed!!");

			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_COUNT, sim_idx);
			if (MsgSettingSetInt(keyName, 0) != MSG_SUCCESS)
				MSG_DEBUG("MsgSettingSetInt is failed!!");

			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_ALPHA_ID, sim_idx);
			if (MsgSettingSetString(keyName, VOICEMAIL_DEFAULT_ALPHA_ID) != MSG_SUCCESS)
				MSG_DEBUG("MsgSettingSetString is failed!!");

			MsgDeleteNoti(MSG_NOTI_TYPE_VOICE_1, sim_idx);
			MsgDeleteNoti(MSG_NOTI_TYPE_VOICE_2, sim_idx);
		}


		/*==================== Voice mail information update ====================*/
		if (getVoiceMailInfo(handle) == true) {
			MSG_DEBUG("########  getVoiceMailInfo Success !!! #######");
		} else {
			MSG_WARN("########  getVoiceMailInfo Fail !!! #######");
		}

		/*==================== Voice mail count update ====================*/
		if (getMwiInfo(handle) == true) {
			MSG_DEBUG("########  getMwiInfo Success !!! #######");
		} else {
			MSG_WARN("########  getMwiInfo Fail !!! #######");

			// Get MWI from vconf and insert notification
			int mwiCnt = 0;
			char keyName[MAX_VCONFKEY_NAME_LEN];
			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_COUNT, sim_idx);
			if ((mwiCnt = MsgSettingGetInt(keyName)) > 0)
				deliverVoiceMsgNoti(sim_idx, mwiCnt);
		}

		/*==================== MSISDN update ====================*/
		if (getMsisdnInfo(handle) == true) {
			MSG_DEBUG("########  getMsisdnInfo Success !!! #######");
		} else {
			MSG_WARN("########  getMsisdnInfo Fail !!! #######");
		}

		/*==================== SST(SIM Service Table) update ====================*/
		if (getSimServiceTable(handle) == true) {
			MSG_DEBUG("########  getSimServiceTable Success !!! #######");
		} else {
			MSG_WARN("########  getSimServiceTable Fail !!! #######");
		}
	}

	MSG_END();
}


void SmsPluginSetting::SimRefreshCb(struct tapi_handle *handle)
{
	pthread_t thd;

	if (pthread_create(&thd, NULL, &init_config_data, handle) < 0) {
		MSG_DEBUG("pthread_create() error");
	}

	pthread_detach(thd);

}


void* SmsPluginSetting::init_config_data(void *data)
{
	struct tapi_handle *handle = (struct tapi_handle *)data;

	instance()->initConfigData(handle);
	return NULL;
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
			addSMSCList((MSG_SMSC_LIST_S *)(&pSetting->option.smscList));
//			setSmscInfo(&pSetting->option.smscList);
			break;
		case MSG_CBMSG_OPT :
			if (setCbConfig(&pSetting->option.cbMsgOpt) == false) {
				THROW(MsgException::SMS_PLG_ERROR, "Failed to set config.");
			}
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
//			getParamList(&pSetting->option.smscList);
			getSmscListInfo(pSetting->option.smscList.simIndex, &(pSetting->option.smscList));
		break;

		case MSG_CBMSG_OPT :
			if (getCbConfig(&pSetting->option.cbMsgOpt) == false) {
				THROW(MsgException::SMS_PLG_ERROR, "Get CB config option failed.");
			}
		break;

		default :
			THROW(MsgException::SMS_PLG_ERROR, "The Setting type is not supported. [%d]", pSetting->type);
		break;
	}
}


void SmsPluginSetting::addSMSCList(MSG_SMSC_LIST_S *pSmscList)
{
	MSG_BEGIN();

	int sim_index = -1;

	MSG_DEBUG("SIM index[%d]", pSmscList->simIndex);
	MSG_DEBUG("total_count[%d]", pSmscList->totalCnt);
	MSG_DEBUG("selected index[%d]", pSmscList->selected);

	for (int i = 0; i < pSmscList->totalCnt; i++)
	{
		MSG_DEBUG("pid[%d]", pSmscList->smscData[i].pid);
		MSG_DEBUG("val_period[%d]", pSmscList->smscData[i].valPeriod);
		MSG_SEC_DEBUG("name[%s]", pSmscList->smscData[i].name);

		MSG_DEBUG("ton[%d]", pSmscList->smscData[i].smscAddr.ton);
		MSG_DEBUG("npi[%d]", pSmscList->smscData[i].smscAddr.npi);
		MSG_SEC_DEBUG("address[%s]", pSmscList->smscData[i].smscAddr.address);
	}

	sim_index = pSmscList->simIndex;

	smscList[sim_index].simIndex = pSmscList->simIndex;
	smscList[sim_index].totalCnt = pSmscList->totalCnt;
	smscList[sim_index].selected = pSmscList->selected;

	for (int i = 0; i < pSmscList->totalCnt; i++) {

		smscList[sim_index].smscData[i].pid = pSmscList->smscData[i].pid;
		smscList[sim_index].smscData[i].valPeriod = pSmscList->smscData[i].valPeriod;

		memset(smscList[sim_index].smscData[i].name, 0x00, SMSC_NAME_MAX+1);
		memcpy(smscList[sim_index].smscData[i].name, pSmscList->smscData[i].name, SMSC_NAME_MAX);

		memset(smscList[sim_index].smscData[i].smscAddr.address, 0x00, SMSC_ADDR_MAX+1);
		memcpy(smscList[sim_index].smscData[i].smscAddr.address, pSmscList->smscData[i].smscAddr.address, SMSC_ADDR_MAX);

		if (pSmscList->smscData[i].smscAddr.address[0] == '+')
			smscList[sim_index].smscData[i].smscAddr.ton = MSG_TON_INTERNATIONAL;
		else
			smscList[sim_index].smscData[i].smscAddr.ton = MSG_TON_NATIONAL;

		smscList[sim_index].smscData[i].smscAddr.npi = MSG_NPI_ISDN; // app cannot set this value
	}

	MSG_END();
}


void SmsPluginSetting::getSmscListInfo(int simIndex, MSG_SMSC_LIST_S *pSmscList)
{
	if (pSmscList == NULL) {
		MSG_DEBUG("pSmscList is NULL!");
		return;
	}

	if (simIndex <= 0) {
		struct tapi_handle *handle = SmsPluginDSHandler::instance()->getTelHandle(simIndex);
		simIndex = SmsPluginDSHandler::instance()->getSimIndex(handle);
	}

	if (simIndex == -1)
		memset(pSmscList, 0x00, sizeof(MSG_SMSC_LIST_S));
	else
		memcpy(pSmscList, &smscList[simIndex], sizeof(MSG_SMSC_LIST_S));

	return;
}


msg_error_t SmsPluginSetting::addCbOpt(MSG_CBMSG_OPT_S *pCbOpt)
{
	msg_error_t err = MSG_SUCCESS;
	char keyName[MAX_VCONFKEY_NAME_LEN];

	MSG_DEBUG("Receive [%d], Max SIM Count [%d]", pCbOpt->bReceive, pCbOpt->maxSimCnt);

	MSG_DEBUG("Channel Count [%d]", pCbOpt->channelData.channelCnt);

	for (int i = 0; i < pCbOpt->channelData.channelCnt; i++)
	{
		MSG_DEBUG("Channel FROM [%d], Channel TO [%d]", pCbOpt->channelData.channelInfo[i].from, pCbOpt->channelData.channelInfo[i].to);
	}

#if 0
	// Set Setting Data into Vconf
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", CB_RECEIVE, pCbOpt->simIndex);
	if (MsgSettingSetBool(keyName, pCbOpt->bReceive) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", keyName);
		return MSG_ERR_SET_SETTING;
	}
#endif

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", CB_MAX_SIM_COUNT, pCbOpt->simIndex);
	if (MsgSettingSetInt(keyName, pCbOpt->maxSimCnt) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", keyName);
		return MSG_ERR_SET_SETTING;
	}

#if 0
	MsgDbHandler *dbHandle = getDbHandle();
	err = MsgStoAddCBChannelInfo(dbHandle, &pCbOpt->channelData);
	if (err != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoGetCBChannelInfo is failed [%d]", err);
		return MSG_ERR_SET_SETTING;
	}
#endif

	return err;
}


void SmsPluginSetting::getCbOpt(MSG_SETTING_S *pSetting, int simIndex)
{
	char keyName[MAX_VCONFKEY_NAME_LEN];

	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();

	memset(&(pSetting->option.cbMsgOpt), 0x00, sizeof(MSG_CBMSG_OPT_S));

	pSetting->type = MSG_CBMSG_OPT;
	pSetting->option.cbMsgOpt.simIndex = simIndex;

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", CB_RECEIVE, simIndex);
	MsgSettingGetBool(keyName, &pSetting->option.cbMsgOpt.bReceive);

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", CB_MAX_SIM_COUNT, simIndex);
	pSetting->option.cbMsgOpt.maxSimCnt = MsgSettingGetInt(keyName);

	err = MsgStoGetCBChannelInfo(dbHandle, &pSetting->option.cbMsgOpt.channelData, simIndex);
	if (err != MSG_SUCCESS)
		MSG_ERR("MsgStoGetCBChannelInfo : err=[%d]", err);

	for (int i = MSG_CBLANG_TYPE_ALL; i < MSG_CBLANG_TYPE_MAX; i++) {
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", CB_LANGUAGE, i);

		MsgSettingGetBool(keyName, &pSetting->option.cbMsgOpt.bLanguage[i]);
	}

}


void SmsPluginSetting::setParamList(const MSG_SMSC_LIST_S *pSMSCList)
{
	MSG_BEGIN();

	MutexLocker lock(mx);

	TelSmsParams_t smsParam = {0};

	int ret = TAPI_API_SUCCESS;

	int index = pSMSCList->index;
	MSG_DEBUG("SMSC Index to be set in SIM = %d", index);

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

		MSG_SEC_DEBUG("address = %s", pSMSCList->smscData[index].smscAddr.address);

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

	/* Get TAPI handle */
	struct tapi_handle *handle = SmsPluginDSHandler::instance()->getTelHandle(pSMSCList->simIndex);

	ret = tel_set_sms_parameters(handle, (const TelSmsParams_t*)&smsParam, TapiEventSetConfigData, NULL);

	if (ret != TAPI_API_SUCCESS)
		THROW(MsgException::SMS_PLG_ERROR, "tel_set_sms_parameters() Error. [%d]", ret);

	if (!getResultFromSim())
		THROW(MsgException::SMS_PLG_ERROR, "tel_set_sms_parameters() Result Error.");

	MSG_END();
}


void SmsPluginSetting::getParamList(MSG_SMSC_LIST_S *pSMSCList)
{
	MSG_BEGIN();

	int paramCnt = 0;

	MSG_SEC_DEBUG("SIM index [%d]", pSMSCList->simIndex);
	struct tapi_handle *handle = SmsPluginDSHandler::instance()->getTelHandle(pSMSCList->simIndex);
	paramCnt = getParamCount(handle);

	MSG_DEBUG("Parameter Count [%d]", paramCnt);

	int ret = TAPI_API_SUCCESS;

	MSG_SMSC_DATA_S tmpSmscData = {};

	for (int index = 0; index < paramCnt; index++) {
		ret = tel_get_sms_parameters(handle, index, TapiEventGetParam, NULL);

		if (ret == TAPI_API_SUCCESS) {
			MSG_DEBUG("######## tel_get_sms_parameters() Success !!! #######");
		} else {
			THROW(MsgException::SMS_PLG_ERROR, "######## tel_get_sms_parameters() Fail !!! return : %d #######", ret);
		}

		if (getParamEvent(handle, &tmpSmscData) == true) {
			MSG_DEBUG("######## Get Parameter was Successful !!! #######");

			memcpy(&(pSMSCList->smscData[index]), &tmpSmscData, sizeof(MSG_SMSC_DATA_S));

			MSG_DEBUG("pid[%d]", pSMSCList->smscData[index].pid);
			MSG_DEBUG("val_period[%d]", pSMSCList->smscData[index].valPeriod);
			MSG_SEC_DEBUG("name[%s]", pSMSCList->smscData[index].name);

			MSG_DEBUG("ton[%d]", pSMSCList->smscData[index].smscAddr.ton);
			MSG_DEBUG("npi[%d]", pSMSCList->smscData[index].smscAddr.npi);
			MSG_SEC_DEBUG("address[%s]", pSMSCList->smscData[index].smscAddr.address);
		} else {
		 	MSG_DEBUG("######## Get Parameter was Failed !!! #######");
		}
	}

	pSMSCList->totalCnt = paramCnt;
	pSMSCList->selected = selectedParam;

	MSG_DEBUG("total_count[%d]", pSMSCList->totalCnt);

	MSG_END();
}


int SmsPluginSetting::getParamCount(struct tapi_handle *handle)
{
	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sms_parameter_count(handle, TapiEventGetParamCnt, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_get_sms_parameter_count() Success !!! #######");
	} else {
		THROW(MsgException::SMS_PLG_ERROR, "tel_get_sms_parameter_count() Error. [%d]", ret);
	}

	return getParamCntEvent();
}


bool SmsPluginSetting::getParam(struct tapi_handle *handle, int Index, MSG_SMSC_DATA_S *pSmscData)
{
	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sms_parameters(handle, Index, TapiEventGetParam, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_get_sms_parameters() Success !!! #######");
	} else {
		MSG_ERR("######## tel_get_sms_parameters() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getParamEvent(handle, pSmscData) == true) {
		MSG_DEBUG("######## Get Parameter was Successful !!! #######");
	} else {
	 	MSG_ERR("######## Get Parameter was Failed !!! #######");
		return false;
	}

	return true;
}


void SmsPluginSetting::setSmscInfo(const MSG_SMSC_LIST_S *pSmscList)
{
	MSG_BEGIN();

	MutexLocker lock(mx);

	int ret = TAPI_API_SUCCESS;

	struct tapi_handle *handle = NULL;

	handle = SmsPluginDSHandler::instance()->getTelHandle(pSmscList->simIndex);

	int select_id = pSmscList->selected;
	const MSG_SMSC_DATA_S *pSmscData = (const MSG_SMSC_DATA_S *)&(pSmscList->smscData[select_id]);

	MSG_DEBUG("Select SMSC id = [%d]", select_id);

	TelSmsAddressInfo_t sca;
	memset(&sca, 0x00, sizeof(TelSmsAddressInfo_t));

	if (strlen(pSmscData->smscAddr.address) > 0) {

		if (pSmscData->smscAddr.address[0] == '+')
			sca.Ton = TAPI_SIM_TON_INTERNATIONAL;
		else
			sca.Ton = TAPI_SIM_TON_NATIONAL;

		sca.Npi = TAPI_SIM_NPI_ISDN_TEL; // app cannot set this value

		MSG_SEC_DEBUG("SMSC TON = [%d], NPI = [%d], Address = [%s]", sca.Ton, sca.Npi, pSmscData->smscAddr.address);

		sca.DialNumLen = SmsPluginParamCodec::encodeSMSC(pSmscData->smscAddr.address, sca.szDiallingNum);
	} else {
		MSG_DEBUG("SMSC Addr is not present");
	}

	ret = tel_set_sms_sca(handle, (const TelSmsAddressInfo_t *)&sca, 0, TapiEventSetSmscInfo, NULL);

	if (ret != TAPI_API_SUCCESS)
		THROW(MsgException::SMS_PLG_ERROR, "tel_set_sms_sca() Error. [%d]", ret);

	if (!getResultFromSim())
		THROW(MsgException::SMS_PLG_ERROR, "tel_set_sms_sca() Result Error.");

	MSG_END();
}


bool SmsPluginSetting::setCbConfig(const MSG_CBMSG_OPT_S *pCbOpt)
{
	struct tapi_handle *handle = NULL;
	int simCnt = SmsPluginDSHandler::instance()->getTelHandleCount();

	TelSmsCbConfig_t cbConfig = {};

	int cbEnabled = 0;
	int ret = TAPI_API_SUCCESS;

	if (pCbOpt->bReceive == true)
		cbEnabled = 2;// Need to get a Enumeration from TAPI, currently it is not available

	MSG_DEBUG("simIndex:%d, cbEnabled:%d", pCbOpt->simIndex, cbEnabled);

	if (pCbOpt->simIndex == 0) {
		char keyName[MAX_VCONFKEY_NAME_LEN];
		memset(keyName, 0x00, sizeof(keyName));
		MSG_SIM_STATUS_T simStatus = MSG_SIM_STATUS_NOT_FOUND;

		for (int i = 1; i <= simCnt; i++) {
			MutexLocker lock(mx);

 			//if (i == pCbOpt->simIndex)
			//	continue;
			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_CHANGED, i);
			simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(keyName);

			if (simStatus == MSG_SIM_STATUS_NOT_FOUND) {
				MSG_DEBUG("SIM %d is not present..", i);
				continue;
			}

			handle = SmsPluginDSHandler::instance()->getTelHandle(i);
			memset(&cbConfig, 0x00, sizeof(TelSmsCbConfig_t));

			MSG_SETTING_S cbSetting;
			getCbOpt(&cbSetting, i);

			cbConfig.CBEnabled = cbEnabled;
			cbConfig.Net3gppType = TAPI_NETTEXT_NETTYPE_3GPP;

			/*cbConfig.CBEnabled = (int)pCbOpt->bReceive;
			cbConfig.Net3gppType = TAPI_NETTEXT_NETTYPE_3GPP;
			cbConfig.MsgIdMaxCount = cbSetting.option.cbMsgOpt.maxSimCnt;
			cbConfig.MsgIdRangeCount = cbSetting.option.cbMsgOpt.channelData.channelCnt;

			for (int i = 0; i < cbConfig.MsgIdRangeCount; i++) {
				cbConfig.MsgIDs[i].Net3gpp.Selected = (unsigned short)cbSetting.option.cbMsgOpt.channelData.channelInfo[i].bActivate;
				cbConfig.MsgIDs[i].Net3gpp.FromMsgId = (unsigned short)cbSetting.option.cbMsgOpt.channelData.channelInfo[i].from;
				cbConfig.MsgIDs[i].Net3gpp.ToMsgId = (unsigned short)cbSetting.option.cbMsgOpt.channelData.channelInfo[i].to;
			}*/

			ret = tel_set_sms_cb_config(handle, &cbConfig, TapiEventSetConfigData, NULL);

			if (ret == TAPI_API_SUCCESS) {
				MSG_DEBUG("######## tel_set_sms_cb_config() Success !!! #######");
			} else {
				MSG_ERR("######## tel_set_sms_cb_config() Fail !!! return : %d #######", ret);
				return false;
			}

			if (getResultFromSim() == true) {
				MSG_DEBUG("######## Set Cb Config was Successful !!! #######");
			} else {
				MSG_ERR("######## Set Cb Config was Failed !!! #######");
				return false;
			}

			msg_error_t err = MSG_SUCCESS;
			MsgDbHandler *dbHandle = getDbHandle();
			err = MsgStoAddCBChannelInfo(dbHandle, const_cast<MSG_CB_CHANNEL_S*>(&pCbOpt->channelData),i);
			if (err != MSG_SUCCESS) {
				MSG_DEBUG("MsgStoAddCBChannelInfo is failed [%d]", err);
				return MSG_ERR_SET_SETTING;
			}
		}

		MSG_DEBUG("SIM Index = [0], Set CB Receive is done");
		return true;
	} else {
		MutexLocker lock(mx);

		handle = SmsPluginDSHandler::instance()->getTelHandle(pCbOpt->simIndex);

		memset(&cbConfig, 0x00, sizeof(TelSmsCbConfig_t));
		cbConfig.CBEnabled = cbEnabled;
		cbConfig.Net3gppType = TAPI_NETTEXT_NETTYPE_3GPP;
	/*	cbConfig.CBEnabled = (int)pCbOpt->bReceive;
		cbConfig.Net3gppType = TAPI_NETTEXT_NETTYPE_3GPP;
		cbConfig.MsgIdMaxCount = pCbOpt->maxSimCnt;
		cbConfig.MsgIdRangeCount = pCbOpt->channelData.channelCnt;

		for (int i = 0; i < cbConfig.MsgIdRangeCount; i++) {
			cbConfig.MsgIDs[i].Net3gpp.Selected = (unsigned short)pCbOpt->channelData.channelInfo[i].bActivate;
			cbConfig.MsgIDs[i].Net3gpp.FromMsgId = (unsigned short)pCbOpt->channelData.channelInfo[i].from;
			cbConfig.MsgIDs[i].Net3gpp.ToMsgId = (unsigned short)pCbOpt->channelData.channelInfo[i].to;

			MSG_DEBUG("FROM: %d, TO: %d", cbConfig.MsgIDs[i].Net3gpp.FromMsgId, cbConfig.MsgIDs[i].Net3gpp.ToMsgId);
		}
		MSG_DEBUG("CBEnabled: %d, range_count: %d", cbConfig.CBEnabled, cbConfig.MsgIdRangeCount);*/

		ret = tel_set_sms_cb_config(handle, &cbConfig, TapiEventSetConfigData, NULL);

		if (ret == TAPI_API_SUCCESS) {
			MSG_DEBUG("######## tel_set_sms_cb_config() Success !!! #######");
		} else {
			MSG_ERR("######## tel_set_sms_cb_config() Fail !!! return : %d #######", ret);
			return false;
		}

		if (getResultFromSim() == true) {
			MSG_DEBUG("######## Set Cb Config was Successful !!! #######");
		} else {
		 	MSG_ERR("######## Set Cb Config was Failed !!! #######");
			return false;
		}
	}

	return true;
}


bool SmsPluginSetting::getCbConfig(MSG_CBMSG_OPT_S *pCbOpt)
{
	int ret = TAPI_API_SUCCESS;

	struct tapi_handle *handle = NULL;

	if (pCbOpt->simIndex == 0) {
		MSG_DEBUG("SIM Index for getCBConfig = 0, CANNOT get cbconfig from SIM 0");
		return false;
	}

	handle = SmsPluginDSHandler::instance()->getTelHandle(pCbOpt->simIndex);

	ret = tel_get_sms_cb_config(handle, TapiEventGetCBConfig, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_get_sms_cb_config() Success !!! #######");
	} else {
		MSG_ERR("######## tel_get_sms_cb_config() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getCbConfigEvent(pCbOpt) == true) {
		MSG_DEBUG("######## Get Cb Config was Successful !!! #######");
	} else {
	 	MSG_ERR("######## Get Cb Config was Failed !!! #######");
		return false;
	}

	return true;
}


void SmsPluginSetting::setVoiceMailInfo(const MSG_VOICEMAIL_OPT_S *pVoiceOpt)
{
	MutexLocker lock(mx);

	int ret = TAPI_API_SUCCESS;
	bool *bShowError = NULL; //When invalid voicemail data exists on SIM, update error should not be handled.

	int simIndex = pVoiceOpt->simIndex;

	if (bMbdnEnable[simIndex] == false) {
		MSG_DEBUG("MBDN service is disable.");
		return;
	}

	TelSimMailBoxNumber_t mailboxInfo = {0,};
	bool bExistVoicetype = false;
	int i = 0;

	if (simMailboxList[simIndex].count < 0) { /* Not available */
		return;
	}

	bShowError = (bool*)calloc(1, sizeof(bool));
	if (!bShowError)
		return;

	if (simMailboxList[simIndex].count == 0) {
		char num[MAX_PHONE_NUMBER_LEN + 1] = {0,};

		mailboxInfo.mb_type = TAPI_SIM_MAILBOX_VOICE;
		mailboxInfo.rec_index = 1;
		mailboxInfo.ton = TAPI_SIM_TON_UNKNOWN;

		snprintf(num, sizeof(num), "%s", pVoiceOpt->mailNumber);
		MSG_DEBUG("Mailbox number config [%s]", num);

		if (num[0] == '+') {
			snprintf(mailboxInfo.num, sizeof(mailboxInfo.num), "%s", &(num[1]));
			mailboxInfo.ton = TAPI_SIM_TON_INTERNATIONAL;
		} else {
			snprintf(mailboxInfo.num, sizeof(mailboxInfo.num), "%s", num);
		}

		MSG_SEC_DEBUG("Mailbox number to save sim [%s]", mailboxInfo.num);

		*bShowError = false;

	} else {
		for (i = 0; i < simMailboxList[simIndex].count; i++) {
			if (simMailboxList[simIndex].list[i].mb_type == TAPI_SIM_MAILBOX_VOICE) {
				bExistVoicetype = true;
				break;
			}
		}

		if (bExistVoicetype == false) {
			*bShowError = false;
			/* There is no mailbox information for voicemail type on SIM. */
			for (i = 0; i < simMailboxList[simIndex].count; i++) {
				if (simMailboxList[simIndex].list[i].mb_type < TAPI_SIM_MAILBOX_VOICE || simMailboxList[simIndex].list[i].mb_type > TAPI_SIM_MAILBOX_OTHER) {
					simMailboxList[simIndex].list[i].mb_type = TAPI_SIM_MAILBOX_VOICE;
					break;
				}
			}
		}

		/* if strlen of voicemail number retrieved from SIM is zero, error is not shown */
		if(simMailboxList[simIndex].list[i].num_len == 0) {
			MSG_DEBUG("In SIM voicemail does not exist");
			*bShowError = false;
		}

		MSG_INFO("bShowError = %d", *bShowError);

		memset(&simMailboxList[simIndex].list[i].num, 0x00, sizeof(simMailboxList[simIndex].list[i].num));
		snprintf(simMailboxList[simIndex].list[i].num, sizeof(simMailboxList[simIndex].list[i].num), "%s", pVoiceOpt->mailNumber);
		MSG_DEBUG("Mailbox number config [%s]", simMailboxList[simIndex].list[i].num);

		mailboxInfo.b_cphs = simMailboxList[simIndex].list[i].b_cphs;
		mailboxInfo.alpha_id_max_len = simMailboxList[simIndex].list[i].alpha_id_max_len;
		mailboxInfo.mb_type = (TelSimMailboxType_t)simMailboxList[simIndex].list[i].mb_type;
		mailboxInfo.profile_num = simMailboxList[simIndex].list[i].profile_num;
		mailboxInfo.rec_index = (simMailboxList[simIndex].list[i].rec_index == 0) ? 1 : simMailboxList[simIndex].list[i].rec_index;
		mailboxInfo.ton = (TelSimTypeOfNum_t)simMailboxList[simIndex].list[i].ton;
		mailboxInfo.npi = (TelSimNumberingPlanIdentity_t)simMailboxList[simIndex].list[i].npi;
		snprintf(mailboxInfo.alpha_id, sizeof(mailboxInfo.alpha_id), "%s", simMailboxList[simIndex].list[i].alpha_id);

		if (simMailboxList[simIndex].list[i].num[0] == '+') {
			snprintf(mailboxInfo.num, sizeof(mailboxInfo.num), "%s", &(simMailboxList[simIndex].list[i].num[1]));
			mailboxInfo.ton = TAPI_SIM_TON_INTERNATIONAL;
		} else {
			snprintf(mailboxInfo.num, sizeof(mailboxInfo.num), "%s", simMailboxList[simIndex].list[i].num);
		}
		MSG_DEBUG("Mailbox number to save sim [%s]", mailboxInfo.num);

		mailboxInfo.cc_id = simMailboxList[simIndex].list[i].cc_id;
		mailboxInfo.ext1_id = simMailboxList[simIndex].list[i].ext1_id;
	}

	struct tapi_handle *handle = SmsPluginDSHandler::instance()->getTelHandle(simIndex);

	ret = tel_set_sim_mailbox_info(handle, &mailboxInfo, TapiEventSetMailboxInfo, (void*)bShowError);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_set_sim_mailbox_info() Success !!! #######");
	} else {
		MSG_ERR("######## tel_set_sim_mailbox_info() Fail !!! return : %d #######", ret);
	}

	if (getResultFromSim() == true) {
		MSG_DEBUG("######## Set mailbox info Success !!! #######");
	} else {
		if(bShowError)
			free(bShowError);
		THROW(MsgException::SMS_PLG_ERROR, "########  Set mailbox info Failed !!!#######");
		MSG_ERR("######## Set mailbox info Failed !!! #######");
	}

	if(bShowError)
		free(bShowError);

	return;
}


bool SmsPluginSetting::getVoiceMailInfo(struct tapi_handle *handle)
{
	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sim_mailbox_info(handle, TapiEventGetMailboxInfo, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_get_sim_mailbox_info() Success !!! #######");
	} else {
		MSG_ERR("######## tel_get_sim_mailbox_info() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getMailboxInfoEvent() == true) {
		MSG_DEBUG("######## Get mailbox info was Successful !!! #######");
	} else {
	 	MSG_ERR("######## Get mailbox info was Failed !!! #######");
		return false;
	}

	return true;
}


void SmsPluginSetting::getMeImei(char *pImei)
{
	int ret = TAPI_API_SUCCESS;

	struct tapi_handle *handle = NULL;
	handle = SmsPluginDSHandler::instance()->getTelHandle(1);

	if (handle == NULL) {
		MSG_DEBUG("Tapi Handle is NULL!");
		return;
	}

	ret = tel_get_misc_me_imei(handle, TapiEventGetMeImei, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_SEC_DEBUG("######## tel_get_misc_me_imei() Success !!! #######");

		if (getResultImei(pImei) == true) {
			MSG_SEC_DEBUG("######## Get ME IMEI was Successful !!! #######");
		} else {
			MSG_SEC_DEBUG("######## Get ME IMEI was Failed !!! #######");
		}
	} else {
		MSG_SEC_DEBUG("######## tel_get_misc_me_imei() Fail !!! return : %d #######", ret);
	}
}


void SmsPluginSetting::setMwiInfo(int simIndex, MSG_SUB_TYPE_T type, int count)
{
	MSG_DEBUG("SET MWI INFO, type=[%d]", type);
	MSG_DEBUG("SET MWI INFO, count=[%d]", count);

	char keyName[MAX_VCONFKEY_NAME_LEN];

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_COUNT, simIndex);

	if (MsgSettingSetInt(keyName, count) != MSG_SUCCESS)
		MSG_DEBUG("MsgSettingSetInt is failed!!");

	if (count <= 0) {
		if (type == MSG_MWI_VOICE_SMS)
			MsgDeleteNoti(MSG_NOTI_TYPE_VOICE_1, simIndex);
		else if (type == MSG_MWI_VOICE2_SMS)
			MsgDeleteNoti(MSG_NOTI_TYPE_VOICE_2, simIndex);
	}

	if (bMbdnEnable[simIndex] == false) {
		MSG_DEBUG("MBDN service is disable.");
		return;
	}

	int ret = TAPI_API_SUCCESS;
	TelSimMessageWaitingReq_t mwReq = {0,};

	MSG_DEBUG("SET MWI INFO, CPHS? [%s]", simMwiInfo[simIndex].b_cphs?"Yes":"No");

	if (simMwiInfo[simIndex].b_cphs) {
		if (type == MSG_MWI_VOICE_SMS)
			simMwiInfo[simIndex].cphs_mwi.b_voice1 = (count > 0 ? 1:0);
		else if (type == MSG_MWI_VOICE2_SMS)
			simMwiInfo[simIndex].cphs_mwi.b_voice2 = (count > 0 ? 1:0);
		else if (type == MSG_MWI_FAX_SMS)
			simMwiInfo[simIndex].cphs_mwi.b_fax = (count > 0 ? 1:0);
		else {
			MSG_DEBUG("There is no type [%d] in CPHS.", type);
			return;
		}

		mwReq.mw_data_u.cphs_mw.b_voice1 = simMwiInfo[simIndex].cphs_mwi.b_voice1;
		mwReq.mw_data_u.cphs_mw.b_voice2 = simMwiInfo[simIndex].cphs_mwi.b_voice2;
		mwReq.mw_data_u.cphs_mw.b_fax = simMwiInfo[simIndex].cphs_mwi.b_fax;
		mwReq.mw_data_u.cphs_mw.b_data = simMwiInfo[simIndex].cphs_mwi.b_data;

		MSG_DEBUG("MWI voice 1 = [%d]", mwReq.mw_data_u.cphs_mw.b_voice1);
		MSG_DEBUG("MWI voice 2 = [%d]", mwReq.mw_data_u.cphs_mw.b_voice2);
		MSG_DEBUG("MWI fax = [%d]", mwReq.mw_data_u.cphs_mw.b_fax);
		MSG_DEBUG("MWI data = [%d]", mwReq.mw_data_u.cphs_mw.b_data);

	} else {
		if (type == MSG_MWI_VOICE_SMS)
			simMwiInfo[simIndex].mwi_list.mw_info[0].voice_count = count;
		else if (type == MSG_MWI_FAX_SMS)
			simMwiInfo[simIndex].mwi_list.mw_info[0].fax_count = count;
		else if (type == MSG_MWI_EMAIL_SMS)
			simMwiInfo[simIndex].mwi_list.mw_info[0].email_count = count;
		else // MSG_MWI_OTHER_SMS
			simMwiInfo[simIndex].mwi_list.mw_info[0].other_count = count;

		mwReq.mw_data_u.mw.rec_index = simMwiInfo[simIndex].mwi_list.mw_info[0].rec_index;
		mwReq.mw_data_u.mw.indicator_status = simMwiInfo[simIndex].mwi_list.mw_info[0].indicator_status;
		mwReq.mw_data_u.mw.voice_count = simMwiInfo[simIndex].mwi_list.mw_info[0].voice_count;
		mwReq.mw_data_u.mw.fax_count = simMwiInfo[simIndex].mwi_list.mw_info[0].fax_count;
		mwReq.mw_data_u.mw.email_count = simMwiInfo[simIndex].mwi_list.mw_info[0].email_count;
		mwReq.mw_data_u.mw.other_count = simMwiInfo[simIndex].mwi_list.mw_info[0].other_count;
		mwReq.mw_data_u.mw.video_count = simMwiInfo[simIndex].mwi_list.mw_info[0].video_count;

		MSG_DEBUG("MWI record index = [%d]", mwReq.mw_data_u.mw.rec_index);
		MSG_DEBUG("MWI ind status = [%d]", mwReq.mw_data_u.mw.indicator_status);
		MSG_DEBUG("MWI voice = [%d]", mwReq.mw_data_u.mw.voice_count);
		MSG_DEBUG("MWI fax = [%d]", mwReq.mw_data_u.mw.fax_count);
		MSG_DEBUG("MWI email = [%d]", mwReq.mw_data_u.mw.email_count);
		MSG_DEBUG("MWI other = [%d]", mwReq.mw_data_u.mw.other_count);
		MSG_DEBUG("MWI video = [%d]", mwReq.mw_data_u.mw.video_count);
	}

	mwReq.b_cphs = simMwiInfo[simIndex].b_cphs;

	struct tapi_handle *handle = SmsPluginDSHandler::instance()->getTelHandle(simIndex);

	ret = tel_set_sim_messagewaiting_info(handle, &mwReq, TapiEventSetMwiInfo, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_set_sim_messagewaiting_info() Success !!! #######");
	} else {
		MSG_DEBUG("######## tel_set_sim_messagewaiting_info() Fail !!! return : %d #######", ret);
	}

	return;
}


bool SmsPluginSetting::getMwiInfo(struct tapi_handle *handle)
{
	MutexLocker lock(mx);

	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sim_messagewaiting_info(handle, TapiEventGetMwiInfo, NULL);

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


bool SmsPluginSetting::getMsisdnInfo(struct tapi_handle *handle)
{
	MutexLocker lock(mx);

	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sim_msisdn(handle, TapiEventGetMsisdnInfo, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_get_sim_msisdn() Success !!! #######");
	} else {
		MSG_DEBUG("######## tel_get_sim_msisdn() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getResultFromSim() == true) {
		MSG_DEBUG("######## Get Sim msisdn was Successful !!! #######");
	} else {
	 	MSG_DEBUG("######## Get Sim msisdn was Failed !!! #######");
		return false;
	}

	return true;
}


bool SmsPluginSetting::getSimServiceTable(struct tapi_handle *handle)
{
	MutexLocker lock(mx);

	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sim_service_table(handle, TapiEventGetSimServiceTable, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_get_sim_service_table() Success !!! #######");
	} else {
		MSG_DEBUG("######## tel_get_sim_service_table() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getResultFromSim() == true) {
		MSG_DEBUG("######## Get SST info was Successful !!! #######");
	} else {
	 	MSG_DEBUG("######## Get SST info was Failed !!! #######");
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

	ret = cv.timedwait(mx.pMutex(), MAX_TAPI_SIM_API_TIMEOUT);

	mx.unlock();

	if (ret == ETIMEDOUT) {
		MSG_ERR("WARNING: TAPI callback TIME-OUT");
		return 0;
	}

	return paramCnt;
}


void SmsPluginSetting::setParamEvent(struct tapi_handle *handle, const MSG_SMSC_DATA_S *pSmscData, int RecordIdx, bool bSuccess)
{
	mx.lock();

	bTapiResult = bSuccess;

	int sim_idx = SmsPluginDSHandler::instance()->getSimIndex(handle);

	memset(&smscData[sim_idx], 0x00, sizeof(MSG_SMSC_DATA_S));

	if (bTapiResult == true) {
		MSG_DEBUG("Success to get parameter data");

		selectedParam = RecordIdx;

		memcpy(&smscData[sim_idx], pSmscData, sizeof(MSG_SMSC_DATA_S));
	}

	cv.signal();

	mx.unlock();
}


bool SmsPluginSetting::getParamEvent(struct tapi_handle *handle, MSG_SMSC_DATA_S *pSmscData)
{
	int ret = 0;

	mx.lock();

	bTapiResult = false;
	ret = cv.timedwait(mx.pMutex(), MAX_TAPI_SIM_API_TIMEOUT);

	mx.unlock();

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	memset(pSmscData, 0x00, sizeof(MSG_SMSC_DATA_S));

	if (bTapiResult == true) {
		int index = SmsPluginDSHandler::instance()->getSimIndex(handle);
		memcpy(pSmscData, &smscData[index], sizeof(MSG_SMSC_DATA_S));
	}

	return bTapiResult;
}


void SmsPluginSetting::setCbConfigEvent(struct tapi_handle *handle, const MSG_CBMSG_OPT_S *pCbOpt, bool bSuccess)
{
	mx.lock();

	char keyName[MAX_VCONFKEY_NAME_LEN];

	bTapiResult = bSuccess;

	int simIndex = SmsPluginDSHandler::instance()->getSimIndex(handle);
	memset(&cbOpt[simIndex], 0x00, sizeof(MSG_CBMSG_OPT_S));

	if (bTapiResult == true) {
		MSG_DEBUG("Success to get cb config data");

		memcpy(&cbOpt[simIndex], pCbOpt, sizeof(MSG_CBMSG_OPT_S));

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", CB_MAX_SIM_COUNT, simIndex);
		if (MsgSettingSetInt(keyName, pCbOpt->maxSimCnt) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", keyName);
		}
	}

	cv.signal();

	mx.unlock();
}


bool SmsPluginSetting::getCbConfigEvent(MSG_CBMSG_OPT_S *pCbOpt)
{
	int ret = 0;

	mx.lock();

	bTapiResult = false;
	ret = cv.timedwait(mx.pMutex(), MAX_TAPI_SIM_API_TIMEOUT);

	mx.unlock();

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	int simIndex = pCbOpt->simIndex;

	memset(pCbOpt, 0x00, sizeof(MSG_CBMSG_OPT_S));

	if (bTapiResult == true) {
		memcpy(pCbOpt, &cbOpt[simIndex], sizeof(MSG_CBMSG_OPT_S));
	}

	return bTapiResult;
}


void SmsPluginSetting::setMailboxInfoEvent(struct tapi_handle *handle, SMS_SIM_MAILBOX_LIST_S *pMailboxList, bool bSuccess, bool bMbdn)
{
	mx.lock();

	bTapiResult = bSuccess;

	int simIndex = SmsPluginDSHandler::instance()->getSimIndex(handle);

	bMbdnEnable[simIndex] = bMbdn;

	memset(&simMailboxList[simIndex], 0x00, sizeof(SMS_SIM_MAILBOX_LIST_S));

	if (bTapiResult == true) {
		int i = 0;
		bool bExistMailboxType = false;
		char keyName[MAX_VCONFKEY_NAME_LEN];

		if (pMailboxList && pMailboxList->count > 0) {
			memcpy(&simMailboxList[simIndex], pMailboxList, sizeof(SMS_SIM_MAILBOX_LIST_S));

			/* Temp :: Save voicemail number with VOICE1 line number */
			for (i = 0; i < pMailboxList->count ; i++) {
				MSG_SEC_DEBUG("Mailbox list[%d] type=[%d]", i, pMailboxList->list[i].mb_type);

				if (pMailboxList->list[i].mb_type == TAPI_SIM_MAILBOX_VOICE) {
					bExistMailboxType = true;
					break;
				}
			}

			if (bExistMailboxType == false) {
				MSG_DEBUG("There is no voice mailbox type.");
				for (i = 0; i < simMailboxList[simIndex].count; i++) {
					if (pMailboxList->list[i].mb_type < TAPI_SIM_MAILBOX_VOICE || pMailboxList->list[i].mb_type > TAPI_SIM_MAILBOX_OTHER) {
						pMailboxList->list[i].mb_type = TAPI_SIM_MAILBOX_VOICE;
						break;
					}
				}
			}

			char mailNumber[MAX_PHONE_NUMBER_LEN+1];
			memset(mailNumber, 0x00 , sizeof(mailNumber));

			MSG_SEC_DEBUG("Mailbox list[%d] ton=[%d], address=[%s], alpha_id=[%s]", \
					i, simMailboxList[simIndex].list[i].ton, simMailboxList[simIndex].list[i].num, \
					simMailboxList[simIndex].list[i].alpha_id);

			if (simMailboxList[simIndex].list[i].ton == MSG_TON_INTERNATIONAL && simMailboxList[simIndex].list[i].num[0] != '+') {
				snprintf(mailNumber, MAX_PHONE_NUMBER_LEN, "+%s", simMailboxList[simIndex].list[i].num);
				MSG_DEBUG("MSG_TON_INTERNATIONAL [%s]", mailNumber);
			} else {
				snprintf(mailNumber, MAX_PHONE_NUMBER_LEN, "%s", simMailboxList[simIndex].list[i].num);
				MSG_DEBUG("[%s]", mailNumber);
			}

			if (mailNumber[0] != '\0') {
				memset(keyName, 0x00, sizeof(keyName));
				snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_NUMBER, simIndex);
				if (MsgSettingSetString(keyName, mailNumber) != MSG_SUCCESS)
					MSG_DEBUG("MsgSettingSetString is failed!!");
			}

			if (simMailboxList[simIndex].list[i].alpha_id[0] != '\0') {
				char unpackAlphaId[MAX_SIM_XDN_ALPHA_ID_LEN+8];
				int tmpLen = 0;
				MSG_LANG_INFO_S langInfo = {0,};

				memset(unpackAlphaId, 0x00, sizeof(unpackAlphaId));

				langInfo.bSingleShift = false;
				langInfo.bLockingShift = false;

				tmpLen = strlen(simMailboxList[simIndex].list[i].alpha_id);

				MsgTextConvert *textCvt = MsgTextConvert::instance();
				textCvt->convertGSM7bitToUTF8((unsigned char*)unpackAlphaId, sizeof(unpackAlphaId), (unsigned char*)simMailboxList[simIndex].list[i].alpha_id, tmpLen, &langInfo);

				MSG_DEBUG("UTF8 ALPHA_ID = [%s]", unpackAlphaId);

				memset(keyName, 0x00, sizeof(keyName));
				snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_ALPHA_ID, simIndex);
				if (MsgSettingSetString(keyName, unpackAlphaId) != MSG_SUCCESS)
					MSG_DEBUG("MsgSettingSetString is failed!!");
			}
		}
	}

	cv.signal();

	mx.unlock();
}

bool SmsPluginSetting::getMailboxInfoEvent()
{
	int ret = 0;

	mx.lock();

	bTapiResult = false;
	ret = cv.timedwait(mx.pMutex(), MAX_TAPI_SIM_API_TIMEOUT);

	mx.unlock();

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	return bTapiResult;
}

void SmsPluginSetting::setMwiInfoEvent(struct tapi_handle *handle, SMS_SIM_MWI_INFO_S *pMwiInfo, bool bSuccess)
{
	MSG_BEGIN();

	mx.lock();

	bTapiResult = bSuccess;

	int index = SmsPluginDSHandler::instance()->getSimIndex(handle);

	memset(&simMwiInfo[index], 0x00, sizeof(SMS_SIM_MWI_INFO_S));

	if (bTapiResult == true) {
		int mwiCnt = 0;
		char keyName[MAX_VCONFKEY_NAME_LEN];

		memcpy(&simMwiInfo[index], pMwiInfo, sizeof(SMS_SIM_MWI_INFO_S));

		/* Save MW count with VOICE line1 number */
		if (simMwiInfo[index].b_cphs == true) {
			mwiCnt = simMwiInfo[index].cphs_mwi.b_voice1;
		} else {
			mwiCnt = simMwiInfo[index].mwi_list.mw_info[0].voice_count; // Normal case
		}
		// TODO :: Add operation for voice mail of line 2

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_COUNT, index);
		if (MsgSettingSetInt(keyName, mwiCnt) != MSG_SUCCESS)
			MSG_DEBUG("MsgSettingSetInt is failed!!");

		MSG_DEBUG("MWI count = [%d]", mwiCnt);

		if (mwiCnt > 0) {
			deliverVoiceMsgNoti(index, mwiCnt);
		}
	}

	cv.signal();

	mx.unlock();

	MSG_END();
}


void SmsPluginSetting::setResultImei(bool bResult, char *pImei)
{
	mx.lock();

	bTapiResult = bResult;

	memset(&meImei, 0x00, sizeof(meImei));

	if (bTapiResult == true && pImei) {
		snprintf(meImei, sizeof(meImei), "%s", pImei);
	}

	cv.signal();

	mx.unlock();
}


bool SmsPluginSetting::getResultImei(char *pImei)
{
	int ret = 0;

	mx.lock();

	ret = cv.timedwait(mx.pMutex(), MAX_TAPI_SIM_API_TIMEOUT);

	mx.unlock();

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	if (bTapiResult == true && pImei) {
		snprintf(pImei, sizeof(meImei), "%s", meImei);
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

	MSG_DEBUG("getResultFromSim() is called .");

	//mx.lock(); //  Caller of this function MUST acquire the mutex before calling the TAPI API

	ret = cv.timedwait(mx.pMutex(), MAX_TAPI_SIM_API_TIMEOUT);

	//mx.unlock();

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


void SmsPluginSetting::deliverVoiceMsgNoti(int simIndex, int mwiCnt)
{
	MSG_BEGIN();

	MSG_MESSAGE_INFO_S msgInfo = {0,};

	msgInfo.addressList = NULL;
	AutoPtr<MSG_ADDRESS_INFO_S> addressListBuf(&msgInfo.addressList);

	msgInfo.addressList = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S)];
	memset(msgInfo.addressList, 0x00, sizeof(MSG_ADDRESS_INFO_S));

	msgInfo.nAddressCnt = 1;

	msgInfo.displayTime = time(NULL);

	char keyName[MAX_VCONFKEY_NAME_LEN];
	char *voiceNum = NULL;
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_NUMBER, simIndex);
	voiceNum = MsgSettingGetString(keyName);

	if (voiceNum) {
		snprintf(msgInfo.addressList[0].addressVal, sizeof(msgInfo.addressList[0].addressVal),
				"%s", voiceNum);
		free(voiceNum);
		voiceNum = NULL;
	}
	memset(msgInfo.addressList[0].displayName, 0x00, sizeof(msgInfo.addressList[0].displayName));
	msgInfo.msgType.mainType = MSG_SMS_TYPE;
	msgInfo.msgType.subType = MSG_MWI_VOICE_SMS;
	msgInfo.sim_idx = simIndex;

#if 0
	if (simMwiInfo.b_cphs == false) { // Normal case
		snprintf(msgInfo.msgText, sizeof(msgInfo.msgText), "%d new voice message", mwiCnt);
	} else {
		snprintf(msgInfo.msgText, sizeof(msgInfo.msgText), "New voice message");
	}
#else
	snprintf(msgInfo.msgText, sizeof(msgInfo.msgText), "%d", mwiCnt);
#endif

#if 0
	if (SmsPluginEventHandler::instance()->callbackMsgIncoming(&msgInfo) != MSG_SUCCESS)
		MSG_DEBUG("callbackIncoming is failed.");
#else
	MsgInsertNotification(&msgInfo);
	MsgChangePmState();
#endif

	MSG_END();
}


void SmsPluginSetting::setSimChangeStatus(struct tapi_handle *handle, bool bInitializing)
{
	MSG_BEGIN();

	int tapiRet = TAPI_API_SUCCESS;
	TelSimCardStatus_t status = TAPI_SIM_STATUS_CARD_ERROR;

	int cardChanged = 0;
	int simIndex = SmsPluginDSHandler::instance()->getSimIndex(handle);

	pthread_t thd;
	char keyName[MAX_VCONFKEY_NAME_LEN] = {0,};

	tapiRet = tel_get_sim_init_info(handle, &status, &cardChanged);
	MSG_INFO("Tapi Ret=[%d], SIM index [%d], SIM status [%d], CardChanged [%d]", tapiRet, simIndex, status, cardChanged);

	if (status == TAPI_SIM_STATUS_SIM_INIT_COMPLETED) {
		if (simStatus[simIndex] == MSG_SIM_STATUS_NOT_FOUND) {
			if (cardChanged == 1) {
				snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_CHANGED, simIndex);
				MsgSettingSetInt(keyName, MSG_SIM_STATUS_CHANGED);
				simStatus[simIndex] = MSG_SIM_STATUS_CHANGED;
			} else {
				snprintf(keyName, sizeof(keyName),"%s/%d", MSG_SIM_CHANGED, simIndex);
				MsgSettingSetInt(keyName, MSG_SIM_STATUS_NORMAL);
				simStatus[simIndex] = MSG_SIM_STATUS_NORMAL;
			}
//			tel_handle_list.push_back(handle);
			// Modified to call initSimInfo for SIM separately
			MSG_DEBUG("calling initSimInfo");
			if (pthread_create(&thd, NULL, &initSimInfo, handle) < 0) {
				MSG_DEBUG("pthread_create() error");
			}
			pthread_detach(thd);

		} else {
			MSG_DEBUG("SIM init was already done!");
		}
	} else {
		MSG_DEBUG("It doesn't initialize yet!!");
		snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_CHANGED, simIndex);
		MSG_DEBUG("Set MSG_SIM_CHANGED to MSG_SIM_STATUS_NOT_FOUND");
		if (MsgSettingSetInt(keyName, MSG_SIM_STATUS_NOT_FOUND) != MSG_SUCCESS)
			MSG_DEBUG("Fail to set MSG_SIM_CHANGED to MSG_SIM_STATUS_NOT_FOUND");

		simStatus[simIndex] = MSG_SIM_STATUS_NOT_FOUND;

	}

	MSG_END();
}
