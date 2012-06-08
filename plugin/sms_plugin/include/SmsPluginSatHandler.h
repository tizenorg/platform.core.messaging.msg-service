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

#ifndef SMS_PLUGIN_SAT_HANDLER_H
#define SMS_PLUGIN_SAT_HANDLER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "SmsPluginTypes.h"

/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginSatHandler
{
public:
	static SmsPluginSatHandler* instance();

	void refreshSms(void *pData);
	void sendSms(void *pData);
	void ctrlSms(void *pData);
	void ctrlSms(MSG_NETWORK_STATUS_T netStatus);

	void finishSimMsgInit(MSG_ERROR_T Err);

private:
	SmsPluginSatHandler();
	virtual ~SmsPluginSatHandler();

	void initSim();

	void	initSMSCList();
	void	initCBConfig();

	int handleSatTpdu(unsigned char *pTpdu, unsigned char TpduLen, int bIsPackingRequired);

	void sendResult(SMS_SAT_CMD_TYPE_T CmdType, int ResultType);

	static SmsPluginSatHandler* pInstance;

	int commandId;

	bool bSendSms;
	bool bInitSim;
	bool bSMSPChanged;
	bool bCBMIChanged;
};

#endif //SMS_PLUGIN_SAT_HANDLER_H

