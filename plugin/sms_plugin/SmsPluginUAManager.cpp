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
		if (smsTranQ.empty())
			cv.wait(mx.pMutex());

		SMS_REQUEST_INFO_S request;
		smsTranQ.front(&request);

		try
		{
			SmsPluginTransport::instance()->submitRequest(&request);
		}
		catch (MsgException& e)
		{
			MSG_FATAL("%s", e.what());

			smsTranQ.pop_front();
			continue;
		}
		catch (exception& e)
		{
			MSG_FATAL("%s", e.what());

			smsTranQ.pop_front();
			continue;
		}

		smsTranQ.pop_front();
	}
}


void SmsPluginUAManager::addReqEntity(SMS_REQUEST_INFO_S *request)
{
	SMS_REQUEST_INFO_S reqTmp = {0,};

	memcpy(&reqTmp, request, sizeof(SMS_REQUEST_INFO_S));
	smsTranQ.push_back(reqTmp);
	cv.signal();
}

