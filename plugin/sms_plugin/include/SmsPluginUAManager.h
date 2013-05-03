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

#ifndef SMS_PLUGIN_UA_MANAGER_H
#define SMS_PLUGIN_UA_MANAGER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgMutex.h"
#include "MsgQueue.h"
#include "MsgThread.h"
#include "SmsPluginTypes.h"


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginUAManager : public MsgThread
{
public:
	static SmsPluginUAManager* instance();

	//virtual void start();

	void addReqEntity(SMS_REQUEST_INFO_S *request);

private:
	SmsPluginUAManager();
	~SmsPluginUAManager();

	virtual void run();

	static SmsPluginUAManager* pInstance;

	bool bRunning;

	MsgThdSafeQ <SMS_REQUEST_INFO_S> smsTranQ;

	Mutex mx;
	CndVar cv;
};

#endif //SMS_PLUGIN_UA_MANAGER_H

