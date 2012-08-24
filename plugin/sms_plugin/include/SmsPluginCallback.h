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
	#include <tapi_common.h>
	#include <TelSms.h>
	#include <TapiUtility.h>
	#include <ITapiNetText.h>
}

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
void TapiEventDeviceReady(TapiHandle *handle, int result, void *data, void *user_data);

void TapiEventSentStatus(TapiHandle *handle, int result, void *data, void *user_data);
void TapiEventMsgIncoming(TapiHandle *handle, const char *noti_id, void *data, void *user_data);
void TapiEventCbMsgIncoming(TapiHandle *handle, const char *noti_id, void *data, void *user_data);
void TapiEventDeliveryReportCNF(TapiHandle *handle, int result, void *data, void *user_data);

void TapiEventGetSimMsgCnt(TapiHandle *handle, int result, void *data, void *user_data);
void TapiEventGetSimMsg(TapiHandle *handle, int result, void *data, void *user_data);
void TapiEventSaveSimMsg(TapiHandle *handle, int result, void *data, void *user_data);
void TapiEventDeleteSimMsg(TapiHandle *handle, int result, void *data, void *user_data);

void TapiEventSetConfigData(TapiHandle *handle, int result, void *data, void *user_data);

void TapiEventGetCBConfig(TapiHandle *handle, int result, void *data, void *user_data);

void TapiEventGetParamCnt(TapiHandle *handle, int result, void *data, void *user_data);
void TapiEventGetParam(TapiHandle *handle, int result, void *data, void *user_data);

void TapiEventSatSmsRefresh(TapiHandle *handle, int result, void *data, void *user_data);
void TapiEventSatSendSms(TapiHandle *handle, const char *noti_id, void *data, void *user_data);
void TapiEventSatMoSmsCtrl(TapiHandle *handle, int result, void *data, void *user_data);

void TapiEventSimFileInfo(TapiHandle *handle, int result, void *data, void *user_data);
void TapiEventSimReadFile(TapiHandle *handle, int result, void *data, void *user_data);


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

