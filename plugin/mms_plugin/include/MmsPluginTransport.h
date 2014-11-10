/*
 * msg-service
 *
 * Copyright (c) 2000 - 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef MMS_PLUGIN_TRANSPORT_H
#define MMS_PLUGIN_TRANSPORT_H

#include "MmsPluginTypes.h"

class MmsPluginTransport
{
	public:
		static MmsPluginTransport *instance();

		void submitRequest(const MSG_REQUEST_INFO_S *pReqInfo);
		void cancelRequest(msg_request_id_t reqId);

	private:
		MmsPluginTransport();
		~MmsPluginTransport();

		void submitProcess(mmsTranQEntity req);
		MMS_NET_ERROR_T submitHandler(mmsTranQEntity *qEntity);
		bool processReceivedData(int msgId, char *pRcvdBody, int rcvdBodyLen, char *retrievedFilePath);

		static MmsPluginTransport *pInstance;
};

#endif //MMS_PLUGIN_TRANSPORT_H
