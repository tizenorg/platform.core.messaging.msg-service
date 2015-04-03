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

#ifndef SMS_PLUGIN_UDCODEC_H
#define SMS_PLUGIN_UDCODEC_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "SmsPluginTypes.h"


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginUDCodec
{
public:
	SmsPluginUDCodec();
	virtual ~SmsPluginUDCodec();

	static int encodeUserData(const SMS_USERDATA_S *pUserData, SMS_CODING_SCHEME_T CodingScheme, char *pEncodeData);
	static int decodeUserData(const unsigned char *pTpdu, const int tpduLen, bool bHeaderInd, SMS_CODING_SCHEME_T CodingScheme, SMS_USERDATA_S *pUserData);
	static int decodeUserData(const unsigned char *pTpdu, const int tpduLen, bool bHeaderInd, SMS_CODING_SCHEME_T CodingScheme, SMS_USERDATA_S *pUserData, SMS_TPUD_S *pTPUD);

	static int pack7bitChar(const unsigned char *pUserData, int dataLen, int fillBits, char *pPackData);
	static int unpack7bitChar(const unsigned char *pTpdu, unsigned char dataLen, int fillBits, char *pUnpackData);

private:
	static int encodeGSMData(const SMS_USERDATA_S *pUserData, char *pEncodeData);
	static int encode8bitData(const SMS_USERDATA_S *pUserData, char *pEncodeData);
	static int encodeUCS2Data(const SMS_USERDATA_S *pUserData, char *pEncodeData);

	static int decodeGSMData(const unsigned char *pTpdu, const int tpduLen, bool bHeaderInd, SMS_USERDATA_S *pUserData, SMS_TPUD_S *pTPUD);
	static int decode8bitData(const unsigned char *pTpdu, bool bHeaderInd, SMS_USERDATA_S *pUserData, SMS_TPUD_S *pTPUD);
	static int decodeUCS2Data(const unsigned char *pTpdu, const int tpduLen, bool bHeaderInd, SMS_USERDATA_S *pUserData, SMS_TPUD_S *pTPUD);

	static int encodeHeader(const SMS_UDH_S header, char *pEncodeHeader);
	static int decodeHeader(const unsigned char *pTpdu, SMS_UDH_S *pHeader);
};

#endif

