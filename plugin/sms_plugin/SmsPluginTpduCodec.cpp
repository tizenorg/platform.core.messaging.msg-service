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

#include <stdio.h>
#include <string.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgGconfWrapper.h"

#include "SmsPluginTpduCodec.h"
#include "SmsPluginParamCodec.h"
#include "SmsPluginUDCodec.h"


/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginTpduCodec - Member Functions
==================================================================================================*/
SmsPluginTpduCodec::SmsPluginTpduCodec()
{


}


SmsPluginTpduCodec::~SmsPluginTpduCodec()
{


}


int SmsPluginTpduCodec::encodeTpdu(const SMS_TPDU_S *pSmsTpdu, char *pTpdu)
{
	int tpduLen = 0;

	switch (pSmsTpdu->tpduType)
	{
		case SMS_TPDU_SUBMIT:
			tpduLen = encodeSubmit(&(pSmsTpdu->data.submit), pTpdu);
		break;

		case SMS_TPDU_DELIVER:
			tpduLen = encodeDeliver(&(pSmsTpdu->data.deliver), pTpdu);
		break;

		case SMS_TPDU_DELIVER_REP:
			tpduLen = encodeDeliverReport(&(pSmsTpdu->data.deliverRep), pTpdu);
		break;

		case SMS_TPDU_STATUS_REP:
			tpduLen = encodeStatusReport(&(pSmsTpdu->data.statusRep), pTpdu);
		break;
	}

	return tpduLen;
}


int SmsPluginTpduCodec::decodeTpdu(const unsigned char *pTpdu, int TpduLen, SMS_TPDU_S *pSmsTpdu)
{
	int decodeLen = 0;

	char mti = pTpdu[0] & 0x03;

	switch (mti)
	{
		case 0x00:
			pSmsTpdu->tpduType = SMS_TPDU_DELIVER;
			decodeLen = decodeDeliver(pTpdu, TpduLen, &(pSmsTpdu->data.deliver));
		break;

		case 0x01:
			pSmsTpdu->tpduType = SMS_TPDU_SUBMIT;
			decodeLen = decodeSubmit(pTpdu, TpduLen, &(pSmsTpdu->data.submit));
		break;

		case 0x02:
			pSmsTpdu->tpduType = SMS_TPDU_STATUS_REP;
			decodeLen = decodeStatusReport(pTpdu, TpduLen, &(pSmsTpdu->data.statusRep));
		break;
	}

	return decodeLen;
}


int SmsPluginTpduCodec::encodeSubmit(const SMS_SUBMIT_S *pSubmit, char *pTpdu)
{
	int offset = 0, length = 0, encodeSize = 0;

	char* address = NULL;
	AutoPtr<char> addressBuf(&address);

	char* dcs = NULL;
	AutoPtr<char> dcsBuf(&dcs);

	char* vpTime = NULL;
	AutoPtr<char> vpBuf(&vpTime);

	//TP-MTI
	pTpdu[offset] = 0x01;

	//TP-RD
	if(pSubmit->bRejectDup == true)
		pTpdu[offset] |= 0x04;

	//TP-VPF
	switch (pSubmit->vpf)
	{
		case SMS_VPF_NOT_PRESENT:
			break;
		case SMS_VPF_ENHANCED:
			pTpdu[offset] |= 0x08;
			break;
		case SMS_VPF_RELATIVE:
			pTpdu[offset] |= 0x10;
			break;
		case SMS_VPF_ABSOLUTE:
			pTpdu[offset] |= 0x18;
			break;
		default:
			break;
	}

	//TP-SRR
	if (pSubmit->bStatusReport == true)
		pTpdu[offset] |= 0x20;

	MSG_DEBUG("TP-SRR pSubmit->bStatusReport : %d.", pSubmit->bStatusReport);

	//TP-UDHI
	if (pSubmit->bHeaderInd == true)
		pTpdu[offset] |= 0x40;

	//TP-RP
	if (pSubmit->bReplyPath == true)
		pTpdu[offset] |= 0x80;

	offset++;

	//TP-MR
	pTpdu[offset++] = pSubmit->msgRef;

	MSG_DEBUG("TP-MR pSubmit->msgRef : %d.", pSubmit->msgRef);

	//TP-DA
	length = SmsPluginParamCodec::encodeAddress(&pSubmit->destAddress, &address);
	memcpy(&(pTpdu[offset]), address, length);
	offset += length;

	MSG_DEBUG("TP-DA length : %d.", length);

	//TP-PID
	pTpdu[offset++] = pSubmit->pid;

	MSG_DEBUG("TP-PID pSubmit->pid : %d.", pSubmit->pid);

	//TP-DCS
	length = SmsPluginParamCodec::encodeDCS(&pSubmit->dcs, &dcs);
	memcpy(&(pTpdu[offset]), dcs, length);
	offset += length;

	MSG_DEBUG("TP-DCS length : %d.", length);

	//TP-VP
	if (pSubmit->vpf != SMS_VPF_NOT_PRESENT)
	{
		length = SmsPluginParamCodec::encodeTime(&pSubmit->validityPeriod, &vpTime);

		if (length > 0)
		{
			memcpy(&(pTpdu[offset]), vpTime, length);
			offset += length;
		}
	}

	encodeSize = SmsPluginUDCodec::encodeUserData(&(pSubmit->userData), pSubmit->dcs.codingScheme, &(pTpdu[offset]));

MSG_DEBUG("encodeSize : %d", encodeSize);

	offset += encodeSize;

#if 0
	printf("\n\n[encodeSubmit] pTpdu data.\n");
	for (int i = 0; i < offset; i++)
	{
		printf(" [%02x]", pTpdu[i]);
	}
	printf("\n\n");
#endif

	return offset;
}


int SmsPluginTpduCodec::encodeDeliver(const SMS_DELIVER_S *pDeliver, char *pTpdu)
{
	int offset = 0, length = 0, encodeSize = 0;

	char* address = NULL;
	AutoPtr<char> addressBuf(&address);

	char* dcs = NULL;
	AutoPtr<char> dcsBuf(&dcs);

	char* scts = NULL;
	AutoPtr<char> timeBuf(&scts);

	// TP-MTI : 00
	pTpdu[offset] = 0x00;

	// TP-MMS
	if (pDeliver->bMoreMsg == false)
		pTpdu[offset] |= 0x04;

	// TP-SRI
	if (pDeliver->bStatusReport == true)
		pTpdu[offset] |= 0x20;

	// TP-UDHI
	if (pDeliver->bHeaderInd == true)
		pTpdu[offset] |= 0x40;

	// TP-RP
	if (pDeliver->bReplyPath == true)
		pTpdu[offset] |= 0x80;

	offset++;

	// TP-OA
	length = SmsPluginParamCodec::encodeAddress(&pDeliver->originAddress, &address);
	memcpy(&(pTpdu[offset]), address, length);
	offset += length;

	// TP-PID
	pTpdu[offset++] = pDeliver->pid;

	// TP-DCS
	length = SmsPluginParamCodec::encodeDCS(&pDeliver->dcs, &dcs);
	memcpy(&(pTpdu[offset]), dcs, length);
	offset += length;

	// TP-SCTS
	length = SmsPluginParamCodec::encodeTime(&pDeliver->timeStamp, &scts);
	memcpy(&(pTpdu[offset]), scts, length);
	offset += length;

	// TP-UDL & TP-UD
	encodeSize = SmsPluginUDCodec::encodeUserData(&(pDeliver->userData), pDeliver->dcs.codingScheme, &(pTpdu[offset]));

	MSG_DEBUG("encodeSize : %d", encodeSize);

	offset += encodeSize;

	return offset;
}


int SmsPluginTpduCodec::encodeDeliverReport(const SMS_DELIVER_REPORT_S *pDeliverRep, char *pTpdu)
{
	int offset = 0;

	// TP-MTI : 00
	pTpdu[offset] = 0x00;

	// TP-UDHI
	if (pDeliverRep->bHeaderInd == true)
		pTpdu[offset] |= 0x40;

	offset++;

	// TP-FCS
	if (pDeliverRep->reportType == SMS_REPORT_NEGATIVE) {
		pTpdu[offset++] = pDeliverRep->failCause;
		MSG_DEBUG("Delivery report : fail cause = [%02x]", pDeliverRep->failCause);
	}

	// TP-PI
	pTpdu[offset++] = pDeliverRep->paramInd;

	// TP-PID
	if (pDeliverRep->paramInd & 0x01)
		pTpdu[offset++] = pDeliverRep->pid;

	// TP-DCS
	if (pDeliverRep->paramInd & 0x02)
	{
		int length = 0;

		char* dcs = NULL;
		AutoPtr<char> dcsBuf(&dcs);

		length = SmsPluginParamCodec::encodeDCS(&pDeliverRep->dcs, &dcs);
		memcpy(&(pTpdu[offset]), dcs, length);

		offset += length;
	}

	// TP-UDL & TP-UD
	if (pDeliverRep->paramInd & 0x04)
	{
		int encodeSize = 0;

		encodeSize = SmsPluginUDCodec::encodeUserData(&(pDeliverRep->userData), pDeliverRep->dcs.codingScheme, &(pTpdu[offset]));

		MSG_DEBUG("encodeSize : %d", encodeSize);

		offset += encodeSize;
	}

	pTpdu[offset] = '\0';

	return offset;
}


int SmsPluginTpduCodec::encodeStatusReport(const SMS_STATUS_REPORT_S *pStatusRep, char *pTpdu)
{
	int offset = 0, length = 0;

	char* address = NULL;
	AutoPtr<char> addressBuf(&address);

	char* scts = NULL;
	AutoPtr<char> sctsBuf(&scts);

	char* dt = NULL;
	AutoPtr<char> dtBuf(&dt);

	// TP-MTI : 10
	pTpdu[offset] = 0x02;

	// TP-MMS
	if (pStatusRep->bMoreMsg == true)
		pTpdu[offset] |= 0x04;

	// TP-SRQ
	if (pStatusRep->bStatusReport == true)
		pTpdu[offset] |= 0x20;

	// TP-UDHI
	if (pStatusRep->bHeaderInd == true)
		pTpdu[offset] |= 0x40;

	offset++;

	// TP-MR
	pTpdu[offset++] = pStatusRep->msgRef;

	// TP-RA
	length = SmsPluginParamCodec::encodeAddress(&pStatusRep->recipAddress, &address);
	memcpy(&(pTpdu[offset]), address, length);
	offset += length;

	// TP-SCTS
	length = SmsPluginParamCodec::encodeTime(&pStatusRep->timeStamp, &scts);
	memcpy(&(pTpdu[offset]), scts, length);
	offset += length;

	// TP-DT
	length = SmsPluginParamCodec::encodeTime(&pStatusRep->dischargeTime, &dt);
	memcpy(&(pTpdu[offset]), dt, length);
	offset += length;

	// TP-Status
	pTpdu[offset++] = pStatusRep->status;

	// TP-PI
	pTpdu[offset++] = pStatusRep->paramInd;

	// TP-PID
	if (pStatusRep->paramInd & 0x01)
		pTpdu[offset++] = pStatusRep->pid;

	// TP-DCS
	if (pStatusRep->paramInd & 0x02)
	{
		int length = 0;

		char* dcs = NULL;
		AutoPtr<char> dcsBuf(&dcs);

		length = SmsPluginParamCodec::encodeDCS(&pStatusRep->dcs, &dcs);
		memcpy(&(pTpdu[offset]), dcs, length);

		offset += length;
	}

	// TP-UDL & TP-UD
	if (pStatusRep->paramInd & 0x04)
	{
		int encodeSize = 0;

		encodeSize = SmsPluginUDCodec::encodeUserData(&(pStatusRep->userData), pStatusRep->dcs.codingScheme, &(pTpdu[offset]));

		MSG_DEBUG("encodeSize : %d", encodeSize);

		offset += encodeSize;
	}

	pTpdu[offset] = '\0';

	return offset;
}


int SmsPluginTpduCodec::decodeSubmit(const unsigned char *pTpdu, int TpduLen, SMS_SUBMIT_S *pSubmit)
{
	int offset = 0, udLen = 0;

	// TP-RD
	if (pTpdu[offset] & 0x04)
		pSubmit->bRejectDup = false;
	else
		pSubmit->bRejectDup = true;

	// TP-VPF
	pSubmit->vpf = (SMS_VPF_T)(pTpdu[offset] & 0x18);

	// TP-SRR
	if (pTpdu[offset] & 0x20)
		pSubmit->bStatusReport = true;
	else
		pSubmit->bStatusReport = false;

	// TP-UDHI
	if (pTpdu[offset] & 0x40)
		pSubmit->bHeaderInd = true;
	else
		pSubmit->bHeaderInd = false;

	// TP-RP
	if (pTpdu[offset] & 0x80)
		pSubmit->bReplyPath = true;
	else
		pSubmit->bReplyPath = false;

	offset++;

	// TP-MR
	pSubmit->msgRef = pTpdu[offset++];

	// TP-DA
	offset += SmsPluginParamCodec::decodeAddress(pTpdu+offset, &(pSubmit->destAddress));

	// TP-PID
	pSubmit->pid = pTpdu[offset++];

	// TP-DCS
	offset += SmsPluginParamCodec::decodeDCS(pTpdu+offset, &(pSubmit->dcs));

	// TP-VP
	if (pSubmit->vpf != SMS_VPF_NOT_PRESENT)
	{
		// Decode VP
	}

	// TP-UDL & TP-UD
	udLen = SmsPluginUDCodec::decodeUserData(pTpdu+offset, TpduLen, pSubmit->bHeaderInd, pSubmit->dcs.codingScheme, &(pSubmit->userData));

	return udLen;
}


int SmsPluginTpduCodec::decodeDeliver(const unsigned char *pTpdu, int TpduLen, SMS_DELIVER_S *pDeliver)
{
	int offset = 0, udLen = 0, tmpOffset = 0;


	char tpduTmp[(TpduLen*2)+1];
	memset(tpduTmp, 0x00, sizeof(tpduTmp));
	for (int i = 0; i < TpduLen; i++) {
		snprintf(tpduTmp+(i*2), sizeof(tpduTmp)-(i*2), "%02X", pTpdu[i]);
	}
	MSG_DEBUG("Deliver TPDU.");
	MSG_INFO("[%s]", tpduTmp);


	// TP-MMS
	if (pTpdu[offset] & 0x04)
		pDeliver->bMoreMsg = false;
	else
		pDeliver->bMoreMsg = true;

	// TP-SRI
	if (pTpdu[offset] & 0x20)
		pDeliver->bStatusReport = true;
	else
		pDeliver->bStatusReport = false;

	// TP-UDHI
	if (pTpdu[offset] & 0x40)
		pDeliver->bHeaderInd = true;
	else
		pDeliver->bHeaderInd = false;

	// TP-RP
	if (pTpdu[offset] & 0x80)
		pDeliver->bReplyPath = true;
	else
		pDeliver->bReplyPath = false;

	offset++;

	tmpOffset = offset;
#if 1
	// TP-OA
	offset += SmsPluginParamCodec::decodeAddress(&pTpdu[offset], &(pDeliver->originAddress));

	// TP-PID
	pDeliver->pid = pTpdu[offset++];

	// TP-DCS
	offset += SmsPluginParamCodec::decodeDCS(&pTpdu[offset], &(pDeliver->dcs));

	// Support KSC5601 :: Coding group bits == 0x84
	if (pTpdu[offset-1] == 0x84) {
		pDeliver->dcs.codingScheme = SMS_CHARSET_EUCKR;
	}

#else
	//For alphanumeric address test

	offset += SmsPluginParamCodec::decodeAddress(&pTpdu[offset], &(pDeliver->originAddress));

	char* address = new char[15];
	address[0] = 0x04;
	address[1] = 0xd0;
	address[2] = 0x11;
	address[3] = 0x00;
	address[4] = 0x20;
	address[5] = 0xF2;
	address[6] = 0x01;
	address[7] = 0x01;
	address[8] = 0x11;
	address[9] = 0x61;
	address[10] = 0x40;
	address[11] = 0x82;
	address[12] = 0x2b;
	address[13] = 0x01;
	address[14] = 0x20;

	SmsPluginParamCodec::decodeAddress((unsigned char*)address, &(pDeliver->originAddress));

	pDeliver->pid = 0x20;
	offset++;
	offset += SmsPluginParamCodec::decodeDCS((unsigned char*)address, &(pDeliver->dcs));
	// end test
#endif

	if (pDeliver->pid == 0x20 && pDeliver->originAddress.ton == SMS_TON_ALPHANUMERIC) {
		int setType = -1;
		int indType = -1;

		bool bVmi = SmsPluginParamCodec::checkCphsVmiMsg(&pTpdu[tmpOffset], &setType, &indType);

		MSG_DEBUG("bVmi = [%d], setType=[%d], indType=[%d]", bVmi, setType, indType);

		if (bVmi) {
			pDeliver->dcs.bMWI = true;

			if (setType == 0) {
				pDeliver->dcs.bIndActive = false;
			} else {
				pDeliver->dcs.bIndActive = true;
			}

			if (indType == 0)
				pDeliver->dcs.indType = SMS_VOICE_INDICATOR;
			else if (indType == 1)
				pDeliver->dcs.indType = SMS_VOICE2_INDICATOR;
		}
	}

	// TP-SCTS
	offset += SmsPluginParamCodec::decodeTime(&pTpdu[offset], &(pDeliver->timeStamp));

	// TP-UD
	udLen = SmsPluginUDCodec::decodeUserData(&pTpdu[offset], TpduLen, pDeliver->bHeaderInd, pDeliver->dcs.codingScheme, &(pDeliver->userData), &(pDeliver->udData));

	return udLen;
}


int SmsPluginTpduCodec::decodeStatusReport(const unsigned char *pTpdu, int TpduLen, SMS_STATUS_REPORT_S *pStatusRep)
{
#ifdef LOG_ENABLE
	printf("\n\n[decodeStatusReport] pTpdu data - Length [%d]\n", TpduLen);

	for (int i = 0; i < TpduLen; i++)
	{
		printf(" [%02x]", pTpdu[i]);
	}
	printf("\n\n");
#endif

	int offset = 0, udLen = 0;

	char* address = NULL;
	AutoPtr<char> addressBuf(&address);

	char* scts = NULL;
	AutoPtr<char> sctsBuf(&scts);

	char* dt = NULL;
	AutoPtr<char> dtBuf(&dt);

	// TP-MMS
	if (pTpdu[offset] & 0x04)
		pStatusRep->bMoreMsg = false;
	else
		pStatusRep->bMoreMsg = true;

	// TP-SRQ
	if (pTpdu[offset] & 0x20)
		pStatusRep->bStatusReport = true;
	else
		pStatusRep->bStatusReport = false;

	// TP-UDHI
	if (pTpdu[offset] & 0x40)
		pStatusRep->bHeaderInd = true;
	else
		pStatusRep->bHeaderInd = false;

	offset++;

	// TP-MR
	pStatusRep->msgRef = pTpdu[offset++];

	// TP-RA
	offset += SmsPluginParamCodec::decodeAddress(&pTpdu[offset], &(pStatusRep->recipAddress));

	// TP-SCTS
	// Decode timestamp
	offset += SmsPluginParamCodec::decodeTime(&pTpdu[offset], &(pStatusRep->timeStamp));

	// TP-DT
	// Decode timestamp
	offset += SmsPluginParamCodec::decodeTime(&pTpdu[offset], &(pStatusRep->dischargeTime));

	// TP-Status
	pStatusRep->status = pTpdu[offset++];

	// TP-PI
	pStatusRep->paramInd = pTpdu[offset++];

	// No Parameters
	if (pStatusRep->paramInd == 0)
	{
		pStatusRep->pid = SMS_PID_NORMAL;

		pStatusRep->dcs.bCompressed = false;
		pStatusRep->dcs.bMWI = false;
		pStatusRep->dcs.bIndActive = false;

		pStatusRep->dcs.msgClass = MSG_CLASS_NONE;
		pStatusRep->dcs.codingScheme = SMS_CHARSET_7BIT;
		pStatusRep->dcs.codingGroup = SMS_GROUP_GENERAL;
		pStatusRep->dcs.indType = SMS_OTHER_INDICATOR;

		pStatusRep->userData.headerCnt = 0;
		pStatusRep->userData.length = 0;
		memset(pStatusRep->userData.data, 0x00, MAX_USER_DATA_LEN+1);
	}

	// TP-PID
	if (pStatusRep->paramInd & 0x01)
		pStatusRep->pid = pTpdu[offset++];

	// TP-DCS
	if (pStatusRep->paramInd & 0x02)
	{
		offset += SmsPluginParamCodec::decodeDCS(&pTpdu[offset], &(pStatusRep->dcs));
	}

	// TP-UDL & TP-UD
	if (pStatusRep->paramInd & 0x04)
	{
		// Decode User Data
		udLen = SmsPluginUDCodec::decodeUserData(&pTpdu[offset], TpduLen, pStatusRep->bHeaderInd, pStatusRep->dcs.codingScheme, &(pStatusRep->userData));
	}

	return udLen;
}

