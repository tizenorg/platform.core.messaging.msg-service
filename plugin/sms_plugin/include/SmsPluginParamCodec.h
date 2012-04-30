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

#ifndef SMS_PLUGIN_PARAMCODEC_H
#define SMS_PLUGIN_PARAMCODEC_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "SmsPluginTypes.h"


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginParamCodec
{
public:
	SmsPluginParamCodec();
	virtual ~SmsPluginParamCodec();

	static int encodeAddress(const SMS_ADDRESS_S *pAddress, char **ppParam);
	static int encodeTime(const SMS_TIMESTAMP_S *pTimeStamp, char **ppParam);
	static int encodeDCS(const SMS_DCS_S *pDCS, char **ppParam);
	static int encodeSMSC(const char *pAddress, unsigned char *pEncodeAddr);
	static int encodeSMSC(const SMS_ADDRESS_S *pAddress, unsigned char *pSMSC);

	static int decodeAddress(const unsigned char *pTpdu, SMS_ADDRESS_S *pAddress);
	static int decodeTime(const unsigned char *pTpdu, SMS_TIMESTAMP_S *pTimeStamp);
	static int decodeDCS(const unsigned char *pTpdu, SMS_DCS_S *pDCS);
	static void decodeSMSC(unsigned char* pAddress, int AddrLen, MSG_SMS_TON_T ton, char *pDecodeAddr);

private:
	static int convertDigitToBcd(char *pDigit, int DigitLen, unsigned char *pBcd);
	static int convertBcdToDigit(const unsigned char *pBcd, int BcdLen, char *pDigit);
};

#endif //SMS_PLUGIN_PARAMCODEC_H
