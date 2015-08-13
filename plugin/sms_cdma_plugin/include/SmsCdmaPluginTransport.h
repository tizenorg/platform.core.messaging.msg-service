/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved
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

#ifndef SMS_CDMA_PLUGIN_TRANSPORT_H
#define SMS_CDMA_PLUGIN_TRANSPORT_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgInternalTypes.h"
#include "MsgMutex.h"
#include "SmsCdmaPluginTypes.h"

extern "C"
{
	#include <TelSat.h>
}

/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginTransport
{
public:
	static SmsPluginTransport* instance();

	void submitRequest(sms_request_info_s *pReqInfo);
	void sendDeliverReport(msg_error_t err, sms_trans_p2p_msg_s *p_p2p_msg);

	void setNetStatus(sms_network_status_t sentStatus);

private:
	SmsPluginTransport();
	~SmsPluginTransport();

	static SmsPluginTransport* pInstance;

	unsigned char getMsgRef();
	unsigned char getSeqNum();
	unsigned char getSubmitMsgId();

	sms_network_status_t getNetStatus();

	void convertMsgInfoToTelesvcMsg(const MSG_MESSAGE_INFO_S *pMsgInfo, sms_trans_msg_s *pMsg);
	void convertMsgInfoToPtp(const MSG_MESSAGE_INFO_S *pMsgInfo, sms_trans_p2p_msg_s *pPtpMsg);
	void convertMsgInfoToSubmit(const MSG_MESSAGE_INFO_S *pMsgInfo, sms_telesvc_submit_s *pSubmit);

	unsigned char 		msgRef;
	unsigned char		msgSeqNum;
	unsigned char		msgSubmitId;

	unsigned char 		msgRef8bit;
	unsigned short 		msgRef16bit;

	sms_network_status_t	curStatus;

	Mutex mx;
	CndVar cv;
};

#endif //SMS_PLUGIN_TRANSPORT_H
