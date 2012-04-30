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

