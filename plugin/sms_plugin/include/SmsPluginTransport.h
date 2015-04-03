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

#ifndef SMS_PLUGIN_TRANSPORT_H
#define SMS_PLUGIN_TRANSPORT_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgMutex.h"
#include "MsgTextConvert.h"
#include "MsgTransportTypes.h"
#include "MsgSettingTypes.h"
#include "SmsPluginTypes.h"
#include "MsgInternalTypes.h"

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

	void submitRequest(SMS_REQUEST_INFO_S *pReqInfo);
	void sendDeliverReport(struct tapi_handle *handle, msg_error_t err);
	void sendClass0DeliverReport(struct tapi_handle *handle, msg_error_t err);

	void setSmscOptions(int simIndex, SMS_ADDRESS_S *pSmsc);

	void msgInfoToSubmitData(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SUBMIT_DATA_S *pData, SMS_CODING_SCHEME_T *pCharType, int addrIndex);
	void setConcatHeader(SMS_UDH_S *pSrcHeader, SMS_UDH_S *pDstHeader);

	void setNetStatus(SMS_NETWORK_STATUS_T sentStatus);
	SMS_NETWORK_STATUS_T getNetStatus();

	void setMoCtrlStatus(TelSatMoSmCtrlIndData_t *moCtrlData);
	int getMoCtrlStatus();

	unsigned char getMsgRef();

private:
	SmsPluginTransport();
	~SmsPluginTransport();

	void getSmsSendOption(int simIndex, SMS_SUBMIT_S *pSubmit);
	void setSmsSendOption(SMS_REQUEST_INFO_S *pReqInfo, SMS_TPDU_S* pSmsTpdu);
	void setSmsDcsOption(SMS_REQUEST_INFO_S *pReqInfo, SMS_TPDU_S* pSmsTpdu);
	void setSmsReportOption(SMS_REQUEST_INFO_S *pReqInfo, SMS_TPDU_S* pSmsTpdu);

	int getSegmentSize(SMS_CODING_SCHEME_T CodingScheme, int DataLen, bool bPortNum, MSG_LANGUAGE_ID_T LangId, int ReplyAddrLen);
	SMS_PID_T convertPid(MSG_SMS_PID_T pid);

	static SmsPluginTransport* pInstance;

	unsigned char 		msgRef;

	unsigned char 		msgRef8bit;
	unsigned short 	msgRef16bit;

	SMS_NETWORK_STATUS_T curStatus;
	TelSatMoSmCtrlIndData_t curMoCtrlData;

	Mutex mx;
	CndVar cv;
};

#endif //SMS_PLUGIN_TRANSPORT_H

