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

#ifndef SMS_PLUGIN_SAT_HANDLER_H
#define SMS_PLUGIN_SAT_HANDLER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "SmsPluginTypes.h"
#include "MsgMutex.h"

extern "C"
{
	#include <TelSat.h>
}

/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginSatHandler
{
public:
	static SmsPluginSatHandler* instance();

	void refreshSms(TapiHandle *handle, void *pData);
	void sendSms(TapiHandle *handle, void *pData);
	void ctrlSms(TapiHandle *handle, void *pData);
	void ctrlSms(TapiHandle *handle, SMS_NETWORK_STATUS_T netStatus);

	void finishSimMsgInit(msg_error_t Err);

private:
	SmsPluginSatHandler();
	virtual ~SmsPluginSatHandler();

	void initSim();

	void	initSMSCList();
	void	initCBConfig();

	int handleSatTpdu(unsigned char *pTpdu, unsigned char TpduLen, int bIsPackingRequired);

	void sendResult(TapiHandle *handle, SMS_SAT_CMD_TYPE_T CmdType, int ResultType);

	static SmsPluginSatHandler* pInstance;

	int commandId;

	bool bInitSim;
	bool bSMSPChanged;
	bool bCBMIChanged;

	MsgMutex mx;
	MsgCndVar cv;
};

#endif /* SMS_PLUGIN_SAT_HANDLER_H */

