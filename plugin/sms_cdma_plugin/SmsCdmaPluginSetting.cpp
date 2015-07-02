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

#include "SmsCdmaPluginParamCodec.h"
#include "SmsCdmaPluginCallback.h"
#include "SmsCdmaPluginEventHandler.h"
#include "SmsCdmaPluginMain.h"
#include "SmsCdmaPluginSetting.h"


extern "C"
{
	#include <tapi_common.h>
	#include <TelSms.h>
	#include <TapiUtility.h>
	#include <ITapiNetText.h>
	#include <ITapiSim.h>
	#include <ITapiModem.h>
}

extern struct tapi_handle *pTapiHandle;

/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginSetting - Member Functions
==================================================================================================*/
SmsPluginSetting* SmsPluginSetting::pInstance = NULL;


SmsPluginSetting::SmsPluginSetting()
{
	// Initialize member variables
	memset(&cbOpt, 0x00, sizeof(MSG_CBMSG_OPT_S));
	memset(&meImei, 0x00, sizeof(meImei));

	bTapiResult = false;
	bUpdateVoicemailByMdn = false;
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
	MSG_BEGIN();

	int tapiRet = TAPI_API_SUCCESS;

	// Get IMSI
	char imsi[17];
	memset(imsi, 0x00, sizeof(imsi));

	// Get IMSI
	TelSimImsiInfo_t imsiInfo;
	memset(&imsiInfo, 0x00, sizeof(TelSimImsiInfo_t));

	tapiRet = tel_get_sim_imsi(pTapiHandle, &imsiInfo);

	if (tapiRet == TAPI_API_SUCCESS) {
		MSG_SEC_DEBUG("tel_get_sim_imsi() Success - MCC [%s], MNC [%s], MSIN [%s]", imsiInfo.szMcc, imsiInfo.szMnc, imsiInfo.szMsin);
		snprintf(imsi, sizeof(imsi), "%03d%03d%s", atoi(imsiInfo.szMcc), atoi(imsiInfo.szMnc), imsiInfo.szMsin);
		MSG_SEC_DEBUG("IMSI [%s]", imsi);
	} else {
		MSG_DEBUG("tel_get_sim_imsi() Error![%d]", tapiRet);
	}

	MsgSettingSetString(MSG_SIM_IMSI, imsi);

	instance()->updateSimStatus();

	MSG_END();
	return NULL;
}

void SmsPluginSetting::updateSimStatus()
{
	MSG_BEGIN();

	if (!pTapiHandle) {
		MSG_DEBUG("pTapiHandle is NULL.");
		return;
	}

	int status = 0;
	int tapiRet = TAPI_API_SUCCESS;

	tapiRet = tel_check_sms_device_status(pTapiHandle, &status);

	if (tapiRet != TAPI_API_SUCCESS) {
		MSG_DEBUG("tel_check_sms_device_status() Error! [%d], Status [%d]", tapiRet, status);
		return;
	}

	if (status == 1 || status == 2) {
		MSG_DEBUG("Device Is Ready, status = %d", status);
		SmsPluginEventHandler::instance()->setNeedInitConfig(false);
	} else if (status == 0) {
		MSG_DEBUG("Device Is Not Ready.. Waiting For Ready Callback");

		if (SmsPluginEventHandler::instance()->getDeviceStatus() == true) {
			MSG_DEBUG("Device Is Ready");
		} else {
			MSG_DEBUG("Device Is Not Ready.");
			return;
		}
	}

	// init config data.
	initConfigData();

	MSG_END();

	return;
}


void SmsPluginSetting::setSimChangeStatus()
{
	MSG_BEGIN();

	pthread_t thd;

	if(pthread_create(&thd, NULL, &initSimInfo, NULL) < 0) {
		MSG_DEBUG("pthread_create() error");
	}

	pthread_detach(thd);

	MSG_END();
}



void SmsPluginSetting::initConfigData()
{
	MSG_BEGIN();

	msg_error_t	err = MSG_SUCCESS;

#if 1
	/*==================== CB configuration ====================*/
//	if (simStatus != MSG_SIM_STATUS_NOT_FOUND)
//	{
//		MSG_DEBUG("simStatus == [%d]", simStatus);

		MSG_CBMSG_OPT_S cbMsgOpt = {0,};

		if (getCbConfig(&cbMsgOpt) == true) {
			err = addCbOpt(&cbMsgOpt);

			if (err == MSG_SUCCESS) {
				MSG_DEBUG("########  Add CB Option Success !!! #######");
				MSG_SETTING_S cbSetting;
				cbSetting.type = MSG_CBMSG_OPT;
				getCbOpt(&cbSetting);
				setCbConfig(&(cbSetting.option.cbMsgOpt));
			} else {
				MSG_DEBUG("########  Add CB Option Fail !!! return : %d #######", err);
			}
		} else {
			MSG_DEBUG("########  getCbConfig Fail !!! #######");
#endif

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

		/*==================== MSISDN update ====================*/
		if (getMsisdnInfo() == true) {
			MSG_DEBUG("########  getMsisdnInfo Success !!! #######");
		} else {
			MSG_DEBUG("########  getMsisdnInfo Fail !!! #######");
		}
#if 1
		/*==================== Default Voice mail Setting ====================*/
		char *num = MsgSettingGetString(VOICEMAIL_DEFAULT_NUMBER);

		if (num) {
			MSG_DEBUG("Voicemail Default Number [%s]", num);
			if (MsgSettingSetString(VOICEMAIL_NUMBER, num) != MSG_SUCCESS)
				MSG_DEBUG("MsgSettingSetInt is failed!!");
			free(num);
			num = NULL;
		}
		else {
			MSG_DEBUG("Voicemail Default Number is NULL");
			if (MsgSettingSetString(VOICEMAIL_NUMBER, "") != MSG_SUCCESS)
				MSG_DEBUG("MsgSettingSetInt is failed!!");
		}

		char *voiceNumber = MsgSettingGetString(VOICEMAIL_NUMBER);

		if (!voiceNumber || (voiceNumber && voiceNumber[0] == '\0')) {
			MSG_DEBUG("Voice Number is Empty");
		}

		if (voiceNumber) {
			free(voiceNumber);
			voiceNumber = NULL;
		}

		if (MsgSettingSetString(VOICEMAIL_ALPHA_ID, VOICEMAIL_DEFAULT_ALPHA_ID) != MSG_SUCCESS)
			MSG_DEBUG("MsgSettingSetString is failed!!");
#endif

	MSG_END();
}


void SmsPluginSetting::SimRefreshCb()
{
	pthread_t thd;

	if(pthread_create(&thd, NULL, &init_config_data, NULL) < 0) {
		MSG_DEBUG("pthread_create() error");
	}

	pthread_detach(thd);

}


void* SmsPluginSetting::init_config_data(void *data)
{
	instance()->initConfigData();
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
		case MSG_SMSC_LIST :
			setParamList(&pSetting->option.smscList);
			break;
#endif
		case MSG_VOICEMAIL_OPT:
			setVoiceMailInfo(&pSetting->option.voiceMailOpt);
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
#if 0
		case MSG_SMSC_LIST :
			getParamList(&pSetting->option.smscList);
		break;
#endif
		case MSG_CBMSG_OPT :
			getCbConfig(&pSetting->option.cbMsgOpt);
		break;

		default :
			THROW(MsgException::SMS_PLG_ERROR, "The Setting type is not supported. [%d]", pSetting->type);
		break;
	}
}


msg_error_t SmsPluginSetting::addCbOpt(MSG_CBMSG_OPT_S *pCbOpt)
{
	msg_error_t err = MSG_SUCCESS;

//	MSG_DEBUG("Receive [%d], Max SIM Count [%d]", pCbOpt->bReceive, pCbOpt->maxSimCnt);

	MSG_DEBUG("Receive [%d], Channel Count [%d]", pCbOpt->bReceive, pCbOpt->channelData.channelCnt);

	for (int i = 0; i < pCbOpt->channelData.channelCnt; i++)
	{
		MSG_DEBUG("Channel Category [%d], Channel Language [%d]", pCbOpt->channelData.channelInfo[i].ctg, pCbOpt->channelData.channelInfo[i].lang);
	}

#if 0
	// Set Setting Data into Vconf
	if (MsgSettingSetBool(CB_RECEIVE, pCbOpt->bReceive) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", CB_RECEIVE);
		return MSG_ERR_SET_SETTING;
	}
#endif

#if 0
	if (MsgSettingSetInt(CB_MAX_SIM_COUNT, pCbOpt->maxSimCnt) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", CB_MAX_SIM_COUNT);
		return MSG_ERR_SET_SETTING;
	}
#endif

#if 0
	MsgDbHandler dbHandle;
	err = MsgStoAddCBChannelInfo(&dbHandle, &pCbOpt->channelData);
	if (err != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoGetCBChannelInfo is failed [%d]", err);
		return MSG_ERR_SET_SETTING;
	}
#endif

	return err;
}


void SmsPluginSetting::getCbOpt(MSG_SETTING_S *pSetting)
{
	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler dbHandle;

	memset(&(pSetting->option.cbMsgOpt), 0x00, sizeof(MSG_CBMSG_OPT_S));

	MsgSettingGetBool(CB_RECEIVE, &pSetting->option.cbMsgOpt.bReceive);

//	pSetting->option.cbMsgOpt.maxSimCnt = MsgSettingGetInt(CB_MAX_SIM_COUNT);

	err = MsgStoGetCBChannelInfo(&dbHandle, &pSetting->option.cbMsgOpt.channelData);
	MSG_DEBUG("MsgStoAddCBChannelInfo : err=[%d]", err);

#if 0
	char keyName[128];

	for (int i = MSG_CBLANG_TYPE_ALL; i < MSG_CBLANG_TYPE_MAX; i++) {
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", CB_LANGUAGE, i);

		MsgSettingGetBool(keyName, &pSetting->option.cbMsgOpt.bLanguage[i]);
	}
#endif

}

void SmsPluginSetting::setVoiceMailInfo(const MSG_VOICEMAIL_OPT_S *pVoiceOpt)
{
	bUpdateVoicemailByMdn = false;

	return;
}

bool SmsPluginSetting::setCbConfig(const MSG_CBMSG_OPT_S *pCbOpt)
{
	int ret = TAPI_API_SUCCESS;

#if 1
	TelSmsCbConfig_t cbConfig = {};

	cbConfig.CBEnabled = (int)pCbOpt->bReceive;
	cbConfig.Net3gppType = TAPI_NETTEXT_NETTYPE_3GPP2;
//	cbConfig.MsgIdMaxCount = pCbOpt->maxSimCnt;
	cbConfig.MsgIdRangeCount = pCbOpt->channelData.channelCnt;

	for (int i = 0; i < cbConfig.MsgIdRangeCount; i++) {
		cbConfig.MsgIDs[i].Net3gpp2.Selected = (unsigned short)pCbOpt->channelData.channelInfo[i].bActivate;
		cbConfig.MsgIDs[i].Net3gpp2.CBCategory = (unsigned short)pCbOpt->channelData.channelInfo[i].ctg;
		cbConfig.MsgIDs[i].Net3gpp2.CBLanguage = (unsigned short)pCbOpt->channelData.channelInfo[i].lang;

		MSG_DEBUG("Category: %d, Language: %d", cbConfig.MsgIDs[i].Net3gpp2.CBCategory, cbConfig.MsgIDs[i].Net3gpp2.CBLanguage);
	}
	MSG_DEBUG("CBEnabled: %d, range_count: %d", cbConfig.CBEnabled, cbConfig.MsgIdRangeCount);

	ret = tel_set_sms_cb_config(pTapiHandle, &cbConfig, TapiEventSetConfigData, NULL);

	if (ret == TAPI_API_SUCCESS) {
		MSG_DEBUG("######## tel_set_sms_cb_config() Success !!! #######");
	} else {
		MSG_DEBUG("######## tel_set_sms_cb_config() Fail !!! return : %d #######", ret);
		return false;
	}

#if 0
	if (getResultFromSim() == true) {
		MSG_DEBUG("######## Set Cb Config was Successful !!! #######");
	} else {
	 	MSG_DEBUG("######## Set Cb Config was Failed !!! #######");
		return false;
	}
#endif
#endif
	return true;
}


bool SmsPluginSetting::getCbConfig(MSG_CBMSG_OPT_S *pCbOpt)
{
	int ret = TAPI_API_SUCCESS;
#if 1
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
#endif
	return true;
}


void SmsPluginSetting::getMeImei(char *pImei)
{
#if 0
	int ret = TAPI_API_SUCCESS;
	ret = tel_get_misc_me_imei(pTapiHandle, TapiEventGetMeImei, NULL);

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
#endif
}

bool SmsPluginSetting::getUpdateVoicemailByMdn()
{
	return bUpdateVoicemailByMdn;
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
	ret = cv.timedwait(mx.pMutex(), 25);

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

	ret = cv.timedwait(mx.pMutex(), 25);

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


void SmsPluginSetting::setResultFromEvent(bool bResult)
{
	mx.lock();

	bTapiResult = bResult;

	cv.signal();

	mx.unlock();
}


void SmsPluginSetting::setMwiInfo(MSG_SUB_TYPE_T type, int count)
{
	MSG_DEBUG("SET MWI INFO, type=[%d]", type);
	MSG_DEBUG("SET MWI INFO, count=[%d]", count);

	if (MsgSettingSetInt(VOICEMAIL_COUNT, count) != MSG_SUCCESS)
		MSG_DEBUG("MsgSettingSetInt is failed!!");

//	if (count == 0) {
//		MsgStoClearUniquenessTable();
//	}

//	if(count <= 0) {
//		if (type == MSG_MWI_VOICE_SMS)
//			MsgCleanAndResetNotification(MSG_NOTI_TYPE_VOICE_1);
//		else if (type == MSG_MWI_VOICE2_SMS)
//			MsgCleanAndResetNotification(MSG_NOTI_TYPE_VOICE_2);
//	}

//	if (bMbdnEnable == false) {
//		MSG_DEBUG("MBDN service is disable.");
//		return;
//	}

	return;
}


bool SmsPluginSetting::getMsisdnInfo(void)
{
	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sim_msisdn(pTapiHandle, TapiEventGetMsisdnInfo, NULL);

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

	ret = cv.timedwait(mx.pMutex(), 25);

	mx.unlock();

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	return bTapiResult;
}

