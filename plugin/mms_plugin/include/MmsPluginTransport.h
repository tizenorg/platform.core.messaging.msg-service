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

#ifndef MMS_PLUGIN_TRANSPORT_H
#define MMS_PLUGIN_TRANSPORT_H


/*==================================================================================================
							INCLUDE FILES
==================================================================================================*/
#include <map>

#include "MsgTransportTypes.h"
#include "MmsPluginTypes.h"


/*==================================================================================================
							CLASS DEFINITIONS
==================================================================================================*/
class MmsPluginTransport
{
	public:
		static MmsPluginTransport *instance();

		void submitRequest(const MSG_REQUEST_INFO_S *pReqInfo);
		void cancelRequest(MSG_REQUEST_ID_T reqId);

	private:
		MmsPluginTransport();
		~MmsPluginTransport();

		void submitProcess(mmsTranQEntity req);
		MMS_NET_ERROR_T submitHandler(mmsTranQEntity *qEntity);
		bool processReceivedData(int msgId, char *pRcvdBody, int rcvdBodyLen, char *retrievedFilePath);

		static MmsPluginTransport *pInstance;
};

#endif //MMS_PLUGIN_TRANSPORT_H

