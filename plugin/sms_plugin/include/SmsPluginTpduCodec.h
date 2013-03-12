/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
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

