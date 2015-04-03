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

#ifndef SMS_CDMA_PLUGIN_STORAGE_H
#define SMS_CDMA_PLUGIN_STORAGE_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgMutex.h"

#include "MsgStorageTypes.h"
#include "SmsCdmaPluginTypes.h"
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

	msg_error_t insertMsgRef(MSG_MESSAGE_INFO_S *pMsg, unsigned char msgRef, int index);
	msg_error_t updateMsgDeliverStatus(MSG_MESSAGE_INFO_S *pMsgInfo, unsigned char msgRef);

	msg_error_t updateSentMsg(MSG_MESSAGE_INFO_S *pMsgInfo, msg_network_status_t Status);

	msg_error_t checkMessage(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t addSmsMessage(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t deleteSmsMessage(msg_message_id_t msgId);
	msg_error_t addSmsSendOption(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo);

	msg_error_t checkStorageStatus(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t getRegisteredPushEvent(char* pPushHeader, int *count, char *app_id, int app_id_len, char *content_type, int content_type_len);
	msg_error_t getnthPushEvent(int index, int *appcode);
	msg_error_t releasePushEvent();

private:
	SmsPluginStorage();
	~SmsPluginStorage();

	static SmsPluginStorage* pInstance;

	MSG_MESSAGE_INFO_S msgInfo;
	MSG_ADDRESS_INFO_S addrInfo;

	std::list<PUSH_APPLICATION_INFO_S> pushAppInfoList;
};

#endif //SMS_CDMA_PLUGIN_STORAGE_H

