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

#ifndef SMS_PLUGIN_EVENT_HANDLER_H
#define SMS_PLUGIN_EVENT_HANDLER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgMutex.h"
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
	void handleSentStatus(int TapiReqId, MSG_NETWORK_STATUS_T NetStatus);
	void handleMsgIncoming(SMS_TPDU_S *pTpdu);
	void handleSyncMLMsgIncoming(MSG_SYNCML_MESSAGE_TYPE_T msgType, char* pPushBody, int PushBodyLen, char* pWspHeader, int WspHeaderLen);
	void handleLBSMsgIncoming(char* pPushHeader, char* pPushBody, int pushBodyLen);
	void handleDftSms(MSG_FOLDER_ID_T FolderId, char* pNumber, char* pData);

	MSG_ERROR_T callbackMsgIncoming(MSG_MESSAGE_INFO_S *pMsgInfo);
	MSG_ERROR_T callbackInitSimBySat();
	MSG_ERROR_T callbackStorageChange(MSG_STORAGE_CHANGE_TYPE_T storageChangeType, MSG_MESSAGE_INFO_S *pMsgInfo);

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
};

#endif //SMS_PLUGIN_EVENT_HANDLER_H

