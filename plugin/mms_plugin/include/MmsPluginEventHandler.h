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

#ifndef MMS_PLUGIN_EVENT_HANDLER_H
#define MMS_PLUGIN_EVENT_HANDLER_H


/*==================================================================================================
							INCLUDE FILES
==================================================================================================*/
#include "MsgPluginInterface.h"
#include "MmsPluginTypes.h"
#include "MmsPluginMessage.h"

/*==================================================================================================
							CLASS DEFINITIONS
==================================================================================================*/
class MmsPluginEventHandler
{
public:
	static MmsPluginEventHandler *instance();

	void registerListener(MSG_PLUGIN_LISTENER_S *pListener);
	void handleSentStatus(int TapiReqId, MSG_NETWORK_STATUS_T NetStatus);
	void handleMmsReceivedData(mmsTranQEntity *pRequest, char *pRetrivedFilePath);
	void handleMmsError(mmsTranQEntity *pRequest);

private:
	MmsPluginEventHandler();
	virtual ~MmsPluginEventHandler();

	static MmsPluginEventHandler *pInstance;

	MSG_PLUGIN_LISTENER_S listener;
};

#endif //MMS_PLUGIN_EVENT_HANDLER_H

