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
	void handleSentStatus(int TapiReqId, msg_network_status_t NetStatus);
	void handleMmsReceivedData(mmsTranQEntity *pRequest, char *pRetrivedFilePath);
	void handleMmsError(mmsTranQEntity *pRequest);

private:
	MmsPluginEventHandler();
	virtual ~MmsPluginEventHandler();

	static MmsPluginEventHandler *pInstance;

	MSG_PLUGIN_LISTENER_S listener;
};

#endif //MMS_PLUGIN_EVENT_HANDLER_H

