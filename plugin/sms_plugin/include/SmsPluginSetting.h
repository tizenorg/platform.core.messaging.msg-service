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

#ifndef SMS_PLUGIN_SETTING_H
#define SMS_PLUGIN_SETTING_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgMutex.h"
#include "MsgSettingTypes.h"
#include <list>

extern "C"
{
	#include <tapi_common.h>
}

/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginSetting
{
public:
	static SmsPluginSetting* instance();

	void setConfigData(const MSG_SETTING_S *pSetting);
	void getConfigData(MSG_SETTING_S *pSetting);
	void getMeImei(char *pImei);

	void setParamCntEvent(int ParamCnt);
	void setParamEvent(struct tapi_handle *handle, const MSG_SMSC_DATA_S *pSmscData, int RecordIdx, bool bSuccess);
	void setCbConfigEvent(struct tapi_handle *handle, const MSG_CBMSG_OPT_S *pCbOpt, bool bSuccess);
	void setMailboxInfoEvent(struct tapi_handle *handle, SMS_SIM_MAILBOX_LIST_S *pVoiceOpt, bool bSuccess, bool bMbdn);
	void setMwiInfo(int simIndex, MSG_SUB_TYPE_T type, int count);
	void setMwiInfoEvent(struct tapi_handle *handle, SMS_SIM_MWI_INFO_S *pMwiInfo, bool bSuccess);
	void setResultFromSim(bool bResult);
	void setResultImei(bool bResult, char *pImei);

	void setSmscInfo(const MSG_SMSC_LIST_S *pSmscList);

	void setSimChangeStatus(struct tapi_handle *handle, bool bInitializing);
	void getSmscListInfo(int simIndex, MSG_SMSC_LIST_S *pSmscList);
	void SimRefreshCb(struct tapi_handle *handle);

private:
	SmsPluginSetting();
	~SmsPluginSetting();

	void updateSimStatus(struct tapi_handle *handle);

	void initConfigData(struct tapi_handle *handle);
	static void* init_config_data(void *data);
	static void* initSimInfo(void *data);
	void* processInitSimInfo(void *data);

	void addSMSCList(MSG_SMSC_LIST_S *pSmscList);

	msg_error_t addCbOpt(MSG_CBMSG_OPT_S *pCbOpt);
	void getCbOpt(MSG_SETTING_S *pSetting, int simIndex);

	void setParamList(const MSG_SMSC_LIST_S *pSMSCList);
	void getParamList(MSG_SMSC_LIST_S *pSMSCList);

	int getParamCount(struct tapi_handle *handle);
	bool getParam(struct tapi_handle *handle, int Index, MSG_SMSC_DATA_S *pSmscData);

	bool setCbConfig(const MSG_CBMSG_OPT_S *pCbOpt);
	bool getCbConfig(MSG_CBMSG_OPT_S *pCbOpt);

	void setVoiceMailInfo(const MSG_VOICEMAIL_OPT_S *pVoiceOpt);
	bool getVoiceMailInfo(struct tapi_handle *handle);

	bool getMwiInfo(struct tapi_handle *handle);
	bool getMsisdnInfo(struct tapi_handle *handle);
	bool getSimServiceTable(struct tapi_handle *handle);
	bool getResultImei(char *pImei);

	int getParamCntEvent();
	bool getParamEvent(struct tapi_handle *handle, MSG_SMSC_DATA_S *pSmscData);
	bool getCbConfigEvent(MSG_CBMSG_OPT_S *pCbOpt);

	bool getMailboxInfoEvent();

	bool getResultFromSim();

	SMS_PID_T convertPid(MSG_SMS_PID_T pid);
	void deliverVoiceMsgNoti(int simIndex, int mwiCnt);

	static SmsPluginSetting* pInstance;
	std::list<TapiHandle *> tel_handle_list;

	/* Setting values for keeping in setting instance */
	MSG_SMSC_LIST_S		smscList[MAX_TELEPHONY_HANDLE_CNT+1];
	MSG_SIM_STATUS_T 	simStatus[MAX_TELEPHONY_HANDLE_CNT+1];
	MSG_SMSC_DATA_S		smscData[MAX_TELEPHONY_HANDLE_CNT+1];
	SMS_SIM_MAILBOX_LIST_S	simMailboxList[MAX_TELEPHONY_HANDLE_CNT+1];
	SMS_SIM_MWI_INFO_S	simMwiInfo[MAX_TELEPHONY_HANDLE_CNT+1];
	MSG_CBMSG_OPT_S	cbOpt[MAX_TELEPHONY_HANDLE_CNT+1];

	/* Local values for getting from SIM(TAPI) */
	bool	bTapiResult;
	int		paramCnt;
	int		selectedParam;
	int 	selectedSimIndex;
	char 	meImei[MAX_ME_IMEI_LEN + 1];

	bool 	bMbdnEnable[MAX_TELEPHONY_HANDLE_CNT];

	Mutex mx;
	CndVar cv;
};

#endif //SMS_PLUGIN_SETTING_H

