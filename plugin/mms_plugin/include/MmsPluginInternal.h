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

#ifndef MMS_PLUGIN_INTERNAL_H
#define MMS_PLUGIN_INTERNAL_H

#include "MmsPluginTypes.h"
#include "MmsPluginCodecTypes.h"

class MmsPluginInternal
{
	public:
		static MmsPluginInternal *instance();

		void processReceivedInd(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S *pRequest, bool *bReject);
		void processSendConf(MSG_MESSAGE_INFO_S *pMsgInfo, mmsTranQEntity *pRequest);
		void processRetrieveConf(MSG_MESSAGE_INFO_S *pMsgInfo, mmsTranQEntity *pRequest, char *pRetrivedFilePath);
		void processForwardConf(MSG_MESSAGE_INFO_S *pMsgInfo, mmsTranQEntity *pRequest);
		bool encodeNotifyRespInd(char *szTrID, msg_delivery_report_status_t iStatus, bool bReportAllowed, char *pSendFilePath);
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
		const char *getMmsDeliveryStatus(msg_delivery_report_status_t deliveryStatus);
		const char *getMmsReadStatus(msg_read_report_status_t readStatus);
};

#endif //MMS_PLUGIN_INTERNAL_H
