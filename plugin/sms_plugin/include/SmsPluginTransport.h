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

#ifndef SMS_PLUGIN_TRANSPORT_H
#define SMS_PLUGIN_TRANSPORT_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgMutex.h"
#include "MsgTransportTypes.h"
#include "MsgSettingTypes.h"
#include "SmsPluginTypes.h"
#include "MsgInternalTypes.h"

/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginTransport
{
public:
	static SmsPluginTransport* instance();

	void submitRequest(SMS_REQUEST_INFO_S *pReqInfo);
	void sendDeliverReport(MSG_ERROR_T err);

	void setSmsSendOptions(SMS_SUBMIT_S *pSubmit);
	void setSmscOptions(SMS_ADDRESS_S *pSmsc);

	void msgInfoToSubmitData(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SUBMIT_DATA_S *pData, SMS_CODING_SCHEME_T *pCharType);
	void setConcatHeader(SMS_UDH_S *pSrcHeader, SMS_UDH_S *pDstHeader);

	void setNetStatus(MSG_NETWORK_STATUS_T netStatus);
	MSG_NETWORK_STATUS_T getNetStatus();

	unsigned char getMsgRef();

private:
	SmsPluginTransport();
	~SmsPluginTransport();

	int getSegmentSize(SMS_CODING_SCHEME_T CodingScheme, int DataLen, bool bPortNum, SMS_LANGUAGE_ID_T LangId, int ReplyAddrLen);
	SMS_PID_T convertPid(MSG_SMS_PID_T pid);

	static SmsPluginTransport* pInstance;

	unsigned char 		msgRef;

	unsigned char 		msgRef8bit;
	unsigned short 	msgRef16bit;

	MSG_NETWORK_STATUS_T curStatus;

	Mutex mx;
	CndVar cv;
};

#endif //SMS_PLUGIN_TRANSPORT_H

