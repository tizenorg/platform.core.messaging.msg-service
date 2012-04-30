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

