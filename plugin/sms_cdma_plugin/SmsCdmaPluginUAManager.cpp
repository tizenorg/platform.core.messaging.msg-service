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

#include "MsgException.h"
#include "MsgCppTypes.h"
#include "SmsCdmaPluginUAManager.h"
#include "SmsCdmaPluginTransport.h"

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
	while (1) {
		lock();
		while (smsTranQ.empty()) {
			wait();
		}
		sms_request_info_s request;
		smsTranQ.front(&request);
		unlock();

		request.msgInfo.addressList = NULL;
		unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&request.msgInfo.addressList, unique_ptr_deleter);

		try {
			SmsPluginTransport::instance()->submitRequest(&request);
		} catch (MsgException& e) {
			MSG_FATAL("%s", e.what());

			lock();
			smsTranQ.pop_front();
			unlock();
			continue;
		} catch (exception& e) {
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


void SmsPluginUAManager::addReqEntity(sms_request_info_s *request)
{
	sms_request_info_s reqTmp = {0, };

	memcpy(&reqTmp, request, sizeof(sms_request_info_s));

	lock();
	smsTranQ.push_back(reqTmp);
	cv.signal();
	unlock();
}
