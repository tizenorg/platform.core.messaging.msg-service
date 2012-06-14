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

