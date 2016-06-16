/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved
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

#ifndef SMS_CDMA_PLUGIN_SETTING_H
#define SMS_CDMA_PLUGIN_SETTING_H


/*==================================================================================================
										INCLUDE FILES
==================================================================================================*/
#include "MsgMutex.h"
#include "MsgSettingTypes.h"


/*==================================================================================================
										CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginSetting
{
public:
	static SmsPluginSetting* instance();

	void setSimChangeStatus();

	void setConfigData(const MSG_SETTING_S *pSetting);
	void getConfigData(MSG_SETTING_S *pSetting);
	void getMeImei(char *pImei);

	void setCbConfigEvent(const MSG_CBMSG_OPT_S *pCbOpt, bool bSuccess);

	void setResultImei(bool bResult, char *pImei);
	void setResultFromEvent(bool bResult);
	void setResultFromSim(bool bResult);

	void setMwiInfo(MSG_SUB_TYPE_T type, int count);
	void SimRefreshCb();

	bool getUpdateVoicemailByMdn();

private:
	SmsPluginSetting();
	~SmsPluginSetting();

	void updateSimStatus();

	void initConfigData();
	static void* init_config_data(void *data);
	static void* initSimInfo(void *data);

	msg_error_t addCbOpt(MSG_CBMSG_OPT_S *pCbOpt);
	void getCbOpt(MSG_SETTING_S *pSetting);

	void setVoiceMailInfo(const MSG_VOICEMAIL_OPT_S *pVoiceOpt);
	bool setCbConfig(const MSG_CBMSG_OPT_S *pCbOpt);
	bool getCbConfig(MSG_CBMSG_OPT_S *pCbOpt);

	bool getMsisdnInfo(void);

	bool getResultImei(char *pImei);

	bool getCbConfigEvent(MSG_CBMSG_OPT_S *pCbOpt);

	bool getResultFromSim();

	static SmsPluginSetting* pInstance;

	MSG_SMSC_DATA_S		smscData;
	MSG_CBMSG_OPT_S	cbOpt;

	bool		bTapiResult;
	bool		bUpdateVoicemailByMdn;

	char	meImei[MAX_ME_IMEI_LEN + 1];

	MsgMutex mx;
	MsgCndVar cv;
};

#endif /* SMS_CDMA_PLUGIN_SETTING_H */
