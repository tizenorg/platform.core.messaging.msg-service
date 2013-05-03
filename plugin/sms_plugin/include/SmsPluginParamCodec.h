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
