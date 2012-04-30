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

