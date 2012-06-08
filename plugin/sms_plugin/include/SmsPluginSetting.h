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

#ifndef SMS_PLUGIN_SETTING_H
#define SMS_PLUGIN_SETTING_H


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

	void initConfigData(MSG_SIM_STATUS_T SimStatus);

	void setConfigData(const MSG_SETTING_S *pSetting);
	void getConfigData(MSG_SETTING_S *pSetting);

	void setParamCntEvent(int ParamCnt);
	void setParamEvent(const MSG_SMSC_DATA_S *pSmscData, int RecordIdx, bool bSuccess);
	void setCbConfigEvent(const MSG_CBMSG_OPT_S *pCbOpt, bool bSuccess);
	void setResultFromSim(bool bResult);

private:
	SmsPluginSetting();
	~SmsPluginSetting();

	MSG_ERROR_T addSMSCList(MSG_SMSC_LIST_S *pSmscList);

	MSG_ERROR_T addCbOpt(MSG_CBMSG_OPT_S *pCbOpt);
	void getCbOpt(MSG_SETTING_S *pSetting);

	void setParamList(const MSG_SMSC_LIST_S *pSMSCList);
	void getParamList(MSG_SMSC_LIST_S *pSMSCList);

	int getParamCount();
	bool getParam(int Index, MSG_SMSC_DATA_S *pSmscData);

	bool setCbConfig(const MSG_CBMSG_OPT_S *pCbOpt);
	bool getCbConfig(MSG_CBMSG_OPT_S *pCbOpt);

	int getParamCntEvent();
	bool getParamEvent(MSG_SMSC_DATA_S *pSmscData);
	bool getCbConfigEvent(MSG_CBMSG_OPT_S *pCbOpt);
	bool getResultFromSim();

	SMS_PID_T convertPid(MSG_SMS_PID_T pid);

	static SmsPluginSetting* pInstance;

	MSG_SMSC_DATA_S		smscData;
	MSG_CBMSG_OPT_S 	cbOpt;

	bool		bTapiResult;
	int		paramCnt;
	int		selectedParam;

	Mutex mx;
	CndVar cv;
};

#endif //SMS_PLUGIN_SETTING_H

