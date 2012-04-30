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

#ifndef MMS_PLUGIN_INTERNAL_H
#define MMS_PLUGIN_INTERNAL_H

/*==================================================================================================
							INCLUDE FILES
==================================================================================================*/
#include "MsgStorageTypes.h"
#include "MsgTypes.h"
#include "MsgTransportTypes.h"
#include "MmsPluginMessage.h"
#include "MmsPluginTypes.h"

/*==================================================================================================
							CLASS DEFINITIONS
==================================================================================================*/
class MmsPluginInternal
{
	public:
		static MmsPluginInternal *instance();

		void processReceivedInd(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S *pRequest, bool *bReject);
		void processSendConf(MSG_MESSAGE_INFO_S *pMsgInfo, mmsTranQEntity *pRequest);
		void processRetrieveConf(MSG_MESSAGE_INFO_S *pMsgInfo, mmsTranQEntity *pRequest, char *pRetrivedFilePath);
		void processForwardConf(MSG_MESSAGE_INFO_S *pMsgInfo, mmsTranQEntity *pRequest);
		bool encodeNotifyRespInd(char *szTrID, MSG_DELIVERY_REPORT_STATUS_T iStatus, bool bReportAllowed, char *pSendFilePath);
		bool encodeAckInd(char *szTrID, bool bReportAllowed, char *pSendFilePath);

	private:
		static MmsPluginInternal *pInstance;

		MmsPluginInternal();
		~MmsPluginInternal();

		bool processNotiInd(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S *pRequest);
		void processDeliveryInd(MSG_MESSAGE_INFO_S *pMsgInfo);
		void processReadOrgInd(MSG_MESSAGE_INFO_S *pMsgInfo);
		bool checkRejectNotiInd(int roamState, bool bReportAllowed, char *pSendFilePath);
		bool getMmsReport(MmsReport mmsReport);
		const char *getMmsDeliveryStatus(MSG_DELIVERY_REPORT_STATUS_T deliveryStatus);
		const char *getMmsReadStatus(MSG_READ_REPORT_STATUS_T readStatus);
};
#endif
