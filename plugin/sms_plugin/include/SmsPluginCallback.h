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

#ifndef SMS_PLUGIN_CALLBACK_H
#define SMS_PLUGIN_CALLBACK_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <map>
#include <vector>

extern "C"
{
	#include <TapiEvent.h>
}

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
int TapiEventDeviceReady(const TelTapiEvent_t *pEvent, void*);

int TapiEventSentStatus(const TelTapiEvent_t *pEvent, void*);
int TapiEventMsgIncoming(const TelTapiEvent_t *pEvent, void*);
int TapiEventCbMsgIncoming(const TelTapiEvent_t *pEvent, void*);
int TapiEventDeliveryReportCNF(const TelTapiEvent_t *pEvent, void*);

int TapiEventGetSimMsgCnt(const TelTapiEvent_t *pEvent, void*);
int TapiEventGetSimMsg(const TelTapiEvent_t *pEvent, void*);
int TapiEventSaveSimMsg(const TelTapiEvent_t *pEvent, void*);
int TapiEventDeleteSimMsg(const TelTapiEvent_t *pEvent, void*);

int TapiEventSetConfigData(const TelTapiEvent_t *pEvent, void*);

int TapiEventGetCBConfig(const TelTapiEvent_t *pEvent, void*);

int TapiEventGetParamCnt(const TelTapiEvent_t *pEvent, void*);
int TapiEventGetParam(const TelTapiEvent_t *pEvent, void*);

int TapiEventSatSmsRefresh(const TelTapiEvent_t *pEvent, void*);
int TapiEventSatSendSms(const TelTapiEvent_t *pEvent, void*);
int TapiEventSatMoSmsCtrl(const TelTapiEvent_t *pEvent, void*);

int TapiEventSimFileInfo(const TelTapiEvent_t *pEvent, void*);
int TapiEventSimReadFile(const TelTapiEvent_t *pEvent, void*);

int TapiEventFactoryDftSms(const TelTapiEvent_t *pEvent, void*);


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginCallback
{
public:
	static SmsPluginCallback* instance();

	void registerEvent();
	void deRegisterEvent();

private:
	SmsPluginCallback();
	~SmsPluginCallback();

	static SmsPluginCallback* pInstance;
};

#endif //SMS_PLUGIN_CALLBACK_H

