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

#ifndef SMS_PLUGIN_TPDU_CODEC_H
#define SMS_PLUGIN_TPDU_CODEC_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "SmsPluginTypes.h"


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginTpduCodec
{
public:
	SmsPluginTpduCodec();
	virtual ~SmsPluginTpduCodec();

	static int encodeTpdu(const SMS_TPDU_S *pSmsTpdu, char *pTpdu);
	static int decodeTpdu(const unsigned char *pTpdu, int TpduLen, SMS_TPDU_S *pSmsTpdu);

private:
	static int encodeSubmit(const SMS_SUBMIT_S *pSubmit, char *pTpdu);
	static int encodeDeliver(const SMS_DELIVER_S *pDeliver, char *pTpdu);
	static int encodeDeliverReport(const SMS_DELIVER_REPORT_S *pDeliverRep, char *pTpdu);
	static int encodeStatusReport(const SMS_STATUS_REPORT_S *pStatusRep, char *pTpdu);

	static int decodeSubmit(const unsigned char *pTpdu, int TpduLen, SMS_SUBMIT_S *pSubmit);
	static int decodeDeliver(const unsigned char *pTpdu, int TpduLen, SMS_DELIVER_S *pDeliver);
	static int decodeStatusReport(const unsigned char *pTpdu, int TpduLen, SMS_STATUS_REPORT_S *pStatusRep);
};

#endif //SMS_PLUGIN_TPDU_CODEC_H

