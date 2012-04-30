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

