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

