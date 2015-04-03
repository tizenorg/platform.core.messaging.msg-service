/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef SMS_PLUGIN_DS_HANDLER_H
#define SMS_PLUGIN_DS_HANDLER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "SmsPluginTypes.h"


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginDSHandler
{
public:
	static SmsPluginDSHandler* instance();
	int initTelHandle();
	void deinitTelHandle();
	struct tapi_handle *getTelHandle(int sim_idx);
	int getTelHandleCount();
	int getActiveSimCount();
	int getSimIndex(struct tapi_handle *handle);
	void getDefaultNetworkSimId(int *simId);

	int getSubscriberId(unsigned int simIndex, char **subscriber_id);

private:
	static SmsPluginDSHandler* pInstance;
	SmsPluginDSHandler();
	virtual ~SmsPluginDSHandler();
	char **cp_list;
	SMS_TELEPHONY_HANDLE_LIST_S handle_list;
};

#endif //SMS_PLUGIN_DS_HANDLER_H

