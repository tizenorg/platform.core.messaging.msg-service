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

extern "C"
{
#ifndef _TAPI_NETTEXT_H_
	#include "ITapiNetText.h"
	#include "ITapiSim.h"
#endif
}

/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginStorage
{
public:
	static SmsPluginStorage* instance();

	MSG_ERROR_T updateSentMsg(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_NETWORK_STATUS_T Status);

	MSG_ERROR_T addSimMessage(MSG_MESSAGE_INFO_S *pSimMsgInfo);

	MSG_ERROR_T addMessage(MSG_MESSAGE_INFO_S *pMsgInfo);
	MSG_ERROR_T addSmsMessage(MSG_MESSAGE_INFO_S *pMsgInfo);

	MSG_ERROR_T deleteSmsMessage(MSG_MESSAGE_ID_T MsgId);

private:
	SmsPluginStorage();
	~SmsPluginStorage();

	MSG_ERROR_T updateSmsMessage(MSG_MESSAGE_INFO_S *pMsgInfo);

	MSG_ERROR_T addCbMessage(MSG_MESSAGE_INFO_S *pMsgInfo);
	MSG_ERROR_T addReplaceTypeMsg(MSG_MESSAGE_INFO_S *pMsgInfo);
	MSG_ERROR_T addWAPMessage(MSG_MESSAGE_INFO_S *pMsgInfo);
	MSG_ERROR_T handleCOWAPMessage(MSG_MESSAGE_INFO_S *pMsgInfo);
	MSG_ERROR_T checkPushMsgValidation(MSG_PUSH_MESSAGE_S *pPushMsg, bool *pbProceed);

	MSG_ERROR_T checkStorageStatus(MSG_MESSAGE_INFO_S *pMsgInfo);
	MSG_ERROR_T updateAllAddress();

	static SmsPluginStorage* pInstance;

	MsgDbHandler dbHandle;

};

#endif //SMS_PLUGIN_STORAGE_H

