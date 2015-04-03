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

#ifndef SMS_CDMA_PLUGIN_PARAMCODEC_H
#define SMS_CDMA_PLUGIN_PARAMCODEC_H


/*==================================================================================================
										INCLUDE FILES
==================================================================================================*/
#include "SmsCdmaPluginTypes.h"


/*==================================================================================================
										CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginParamCodec
{
public:

	static SmsPluginParamCodec* instance();

	static int convertDigitToBcd(char *pDigit, int DigitLen, unsigned char *pBcd);
	static int convertBcdToDigit(const unsigned char *pBcd, int BcdLen, char *pDigit);
	static int convertDigitToDTMF(const char *pDigit, int DigitLen, int startBit, unsigned char *pDtmf);
	static int convertDTMFToDigit(const unsigned char *pDtmf, int DtmfLen, int startBit, char *pDigit);

private:
	SmsPluginParamCodec();
	virtual ~SmsPluginParamCodec();

	static SmsPluginParamCodec* pInstance;

	static bool isDtmfNumber(const char *pDigit, int DigitLen);
};

#endif /*SMS_CDMA_PLUGIN_PARAMCODEC_H*/
