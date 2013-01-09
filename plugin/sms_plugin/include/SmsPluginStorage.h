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

#ifndef SMS_PLUGIN_STORAGE_H
#define SMS_PLUGIN_STORAGE_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgMutex.h"

#include "MsgStorageTypes.h"
#include "SmsPluginTypes.h"
#include "MsgInternalTypes.h"
#include "MsgSqliteWrapper.h"
#include <list>

extern "C"
{
	#include <tapi_common.h>
	#include <TelSms.h>
	#include <TapiUtility.h>
	#include <ITapiNetText.h>
}

/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginStorage
{
public:
	static SmsPluginStorage* instance();

	msg_error_t updateSentMsg(MSG_MESSAGE_INFO_S *pMsgInfo, msg_network_status_t Status);

	msg_error_t addSimMessage(MSG_MESSAGE_INFO_S *pSimMsgInfo);

	msg_error_t addMessage(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t addSmsMessage(MSG_MESSAGE_INFO_S *pMsgInfo);

	msg_error_t addSmsSendOption(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo);

	msg_error_t deleteSmsMessage(msg_message_id_t MsgId);

	msg_error_t getRegisteredPushEvent(char* pPushHeader, int *count, char *app_id, char *content_type);
	msg_error_t getnthPushEvent(int index, int *appcode);
	msg_error_t releasePushEvent();
private:
	SmsPluginStorage();
	~SmsPluginStorage();

	msg_error_t updateSmsMessage(MSG_MESSAGE_INFO_S *pMsgInfo);

	msg_error_t addCbMessage(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t addReplaceTypeMsg(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t addWAPMessage(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t handleCOWAPMessage(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t checkPushMsgValidation(MSG_PUSH_MESSAGE_S *pPushMsg, bool *pbProceed);

	msg_error_t checkStorageStatus(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t updateAllAddress();

	static SmsPluginStorage* pInstance;

	MsgDbHandler dbHandle;
	std::list<PUSH_APPLICATION_INFO_S> pushAppInfoList;
//	unsigned char tmpMsgRef;
};

#endif //SMS_PLUGIN_STORAGE_H

