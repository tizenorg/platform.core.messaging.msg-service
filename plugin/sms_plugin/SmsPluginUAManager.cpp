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


void SmsPluginUAManager::addReqEntity(SMS_REQUEST_INFO_S request)
{
	smsTranQ.push_back(request);
	cv.signal();
}

