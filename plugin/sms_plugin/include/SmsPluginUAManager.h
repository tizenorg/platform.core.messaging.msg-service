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

	/* virtual void start(); */

	void addReqEntity(SMS_REQUEST_INFO_S *request);

private:
	SmsPluginUAManager();
	~SmsPluginUAManager();
	void lock()	{ mx.lock(); };
	void unlock()	{ mx.unlock(); };
	void wait()	{ cv.wait(mx.pMsgMutex()); };
	void signal()	{ cv.signal(); };

	virtual void run();

	static SmsPluginUAManager* pInstance;

	MsgSimpleQ <SMS_REQUEST_INFO_S> smsTranQ;

	MsgMutex mx;
	MsgCndVar cv;
};

#endif /* SMS_PLUGIN_UA_MANAGER_H */

