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

#ifndef SMS_PLUGIN_SAT_HANDLER_H
#define SMS_PLUGIN_SAT_HANDLER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "SmsPluginTypes.h"

/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginSatHandler
{
public:
	static SmsPluginSatHandler* instance();

	void refreshSms(void *pData);
	void sendSms(void *pData);
	void ctrlSms(void *pData);
	void ctrlSms(MSG_NETWORK_STATUS_T netStatus);

	void finishSimMsgInit(MSG_ERROR_T Err);

private:
	SmsPluginSatHandler();
	virtual ~SmsPluginSatHandler();

	void initSim();

	void	initSMSCList();
	void	initCBConfig();

	int handleSatTpdu(unsigned char *pTpdu, unsigned char TpduLen, int bIsPackingRequired);

	void sendResult(SMS_SAT_CMD_TYPE_T CmdType, int ResultType);

	static SmsPluginSatHandler* pInstance;

	int commandId;

	bool bSendSms;
	bool bInitSim;
	bool bSMSPChanged;
	bool bCBMIChanged;
};

#endif //SMS_PLUGIN_SAT_HANDLER_H

