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

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "SmsPluginEventHandler.h"
#include "SmsPluginWapPushHandler.h"
#include "SmsPluginConcatHandler.h"
#include "SmsPluginTransport.h"
#include "SmsPluginUAManager.h"


/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginUAManager - Member Functions
==================================================================================================*/
SmsPluginUAManager* SmsPluginUAManager::pInstance = NULL;


SmsPluginUAManager::SmsPluginUAManager() : mx(), cv()
{
	start();
}


SmsPluginUAManager::~SmsPluginUAManager()
{

}


SmsPluginUAManager* SmsPluginUAManager::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginUAManager();

	return pInstance;
}


void SmsPluginUAManager::run()
{
	while (1)
	{
		lock();
		while (smsTranQ.empty()) {
			wait();
		}
		SMS_REQUEST_INFO_S request;
		smsTranQ.front(&request);
		unlock();

		request.msgInfo.addressList = NULL;
		unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&request.msgInfo.addressList, unique_ptr_deleter);

		try
		{
			SmsPluginTransport::instance()->submitRequest(&request);
		}
		catch (MsgException& e)
		{
			MSG_FATAL("%s", e.what());

			lock();
			smsTranQ.pop_front();
			unlock();
			continue;
		}
		catch (exception& e)
		{
			MSG_FATAL("%s", e.what());
			lock();
			smsTranQ.pop_front();
			unlock();
			continue;
		}

		lock();
		smsTranQ.pop_front();
		unlock();
	}
}


void SmsPluginUAManager::addReqEntity(SMS_REQUEST_INFO_S *request)
{
	SMS_REQUEST_INFO_S reqTmp = {0,};

	memcpy(&reqTmp, request, sizeof(SMS_REQUEST_INFO_S));

	lock();

	smsTranQ.push_back(reqTmp);
	cv.signal();

	unlock();
}

