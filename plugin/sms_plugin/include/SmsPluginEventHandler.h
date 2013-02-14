/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef SMS_PLUGIN_EVENT_HANDLER_H
#define SMS_PLUGIN_EVENT_HANDLER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgMutex.h"
#include "MsgTextConvert.h"
#include "MsgPluginInterface.h"
#include "SmsPluginTypes.h"


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginEventHandler
{
public:
	static SmsPluginEventHandler* instance();

	void registerListener(MSG_PLUGIN_LISTENER_S *pListener);
	void handleSentStatus(msg_network_status_t NetStatus);
	void handleMsgIncoming(SMS_TPDU_S *pTpdu);
	void handleSyncMLMsgIncoming(msg_syncml_message_type_t msgType, char* pPushBody, int PushBodyLen, char* pWspHeader, int WspHeaderLen);
	void handleLBSMsgIncoming(char* pPushHeader, char* pPushBody, int pushBodyLen);
	void handlePushMsgIncoming(char* pPushHeader, char* pPushBody, int pushBodyLen, char *app_id, char *content_type);

	msg_error_t callbackMsgIncoming(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t callbackCBMsgIncoming(MSG_CB_MSG_S *pCbMsg);
	msg_error_t callbackInitSimBySat();
	msg_error_t callbackStorageChange(msg_storage_change_type_t storageChangeType, MSG_MESSAGE_INFO_S *pMsgInfo);

	void SetSentInfo(SMS_SENT_INFO_S *pSentInfo);

	void setDeviceStatus();
	bool getDeviceStatus();

	void convertTpduToMsginfo(SMS_TPDU_S *pTpdu, MSG_MESSAGE_INFO_S *msgInfo); // temp

private:
	SmsPluginEventHandler();
	virtual ~SmsPluginEventHandler();

	void convertSubmitTpduToMsginfo(const SMS_SUBMIT_S *pTpdu, MSG_MESSAGE_INFO_S *msgInfo);
	void convertDeliverTpduToMsginfo(const SMS_DELIVER_S *pTpdu, MSG_MESSAGE_INFO_S *msgInfo);
	void convertStatusRepTpduToMsginfo(const SMS_STATUS_REPORT_S *pTpdu, MSG_MESSAGE_INFO_S *msgInfo);
	MSG_SUB_TYPE_T convertMsgSubType(SMS_PID_T pid);

	static SmsPluginEventHandler* pInstance;

	MSG_PLUGIN_LISTENER_S listener;
	MSG_SIM_COUNT_S* pSimCnt;
	SMS_SENT_INFO_S sentInfo;

	bool devStatus;

	Mutex mx;
	CndVar cv;

	MsgTextConvert textCvt;
};

#endif //SMS_PLUGIN_EVENT_HANDLER_H

