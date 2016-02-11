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
#include "MsgTextConvert.h"
#include "SmsPluginUDCodec.h"
#include "SmsPluginParamCodec.h"


/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginParamCodec - Member Functions
==================================================================================================*/
SmsPluginParamCodec::SmsPluginParamCodec()
{
}


SmsPluginParamCodec::~SmsPluginParamCodec()
{
}


/*==================================================================================================
                                     Encode Functions
==================================================================================================*/
int SmsPluginParamCodec::encodeAddress(const SMS_ADDRESS_S *pAddress, char **ppParam)
{
	int offset = 0, length = 0;
	char *temp = (char *)pAddress->address;

	SMS_TON_T ton;

	*ppParam = new char[MAX_ADD_PARAM_LEN];

	/* Set Address Length */
	if (temp[0] == '+') {
		(*ppParam)[offset++] = strlen(temp) - 1;
		temp++;

		ton = SMS_TON_INTERNATIONAL;
	} else {
		(*ppParam)[offset++] = strlen(temp);

		ton = pAddress->ton;
	}

	/* Set TON, NPI */
	(*ppParam)[offset++] = 0x80 + (ton << 4) + pAddress->npi;

	MSG_DEBUG("Address length is %d.", (*ppParam)[0]);
	MSG_DEBUG("pAddress->ton : %d.", ton);
	MSG_DEBUG("pAddress->npi : %d.", pAddress->npi);

	length = convertDigitToBcd(temp, strlen(temp), (unsigned char *) &((*ppParam)[offset]));

	offset += length;

	return offset ;
}


int SmsPluginParamCodec::encodeTime(const SMS_TIMESTAMP_S *pTimeStamp, char **ppParam)
{
	int offset = 0;

	if (pTimeStamp->format == SMS_TIME_ABSOLUTE) {
		int timeZone = pTimeStamp->time.absolute.timeZone;
		*ppParam = new char[MAX_ABS_TIME_PARAM_LEN];

		(*ppParam)[offset++] = ((pTimeStamp->time.absolute.year % 10)  << 4) + (pTimeStamp->time.absolute.year / 10);
		(*ppParam)[offset++] = ((pTimeStamp->time.absolute.month % 10) << 4) + (pTimeStamp->time.absolute.month / 10);
		(*ppParam)[offset++] = ((pTimeStamp->time.absolute.day % 10) << 4) + (pTimeStamp->time.absolute.day / 10);
		(*ppParam)[offset++] = ((pTimeStamp->time.absolute.hour % 10) << 4) + (pTimeStamp->time.absolute.hour / 10);
		(*ppParam)[offset++] = ((pTimeStamp->time.absolute.minute % 10) << 4) + (pTimeStamp->time.absolute.minute / 10);
		(*ppParam)[offset++] = ((pTimeStamp->time.absolute.second % 10) << 4) + (pTimeStamp->time.absolute.second / 10);

		if (timeZone < 0) {
			timeZone *= -1;
			(*ppParam)[offset] = 0x08;
		}
		(*ppParam)[offset++] += ((pTimeStamp->time.absolute.timeZone % 10) << 4) + (pTimeStamp->time.absolute.timeZone / 10);


		return offset;
	} else if (pTimeStamp->format == SMS_TIME_RELATIVE) {
		*ppParam = new char[MAX_REL_TIME_PARAM_LEN+1];

		memcpy(*ppParam, &(pTimeStamp->time.relative.time), MAX_REL_TIME_PARAM_LEN);

		return MAX_REL_TIME_PARAM_LEN;
	}

	return offset;
}


int SmsPluginParamCodec::encodeDCS(const SMS_DCS_S *pDCS, char **ppParam)
{
	*ppParam = new char[MAX_DCS_PARAM_LEN];

	**ppParam = 0x00;

	switch (pDCS->codingGroup) {
	case SMS_GROUP_GENERAL: {
		if (pDCS->msgClass != SMS_MSG_CLASS_NONE)
			**ppParam = 0x10 + pDCS->msgClass;

		if (pDCS->bCompressed)
			**ppParam |= 0x20;
	}
		break;

	case SMS_GROUP_CODING_CLASS: {
		**ppParam = 0xF0 + pDCS->msgClass;
	}
		break;

	case SMS_GROUP_DELETION:
		/* not supported */
		break;

	case SMS_GROUP_DISCARD:
		/* not supported */
		break;

	case SMS_GROUP_STORE:
		/* not supported */
		break;

	default:
		return 0;
	}

	switch (pDCS->codingScheme) {
	case SMS_CHARSET_7BIT:

		break;

	case SMS_CHARSET_8BIT:
		**ppParam |= 0x04;
		break;

	case SMS_CHARSET_UCS2:
		**ppParam |= 0x08;
		break;

	default:
		return 0;
	}

	return MAX_DCS_PARAM_LEN;
}


int SmsPluginParamCodec::encodeSMSC(const char *pAddress, unsigned char *pEncodeAddr)
{
	char newAddr[MAX_SMSC_LEN+1];
	memset(newAddr, 0x00, sizeof(newAddr));

/*	MSG_DEBUG("SMSC [%s]", pAddress);
*/
	if (pAddress[0] == '+')
		strncpy(newAddr, pAddress+1, MAX_SMSC_LEN);
	else
		strncpy(newAddr, pAddress, MAX_SMSC_LEN);

	/* Set Address */
	int encodeLen = convertDigitToBcd(newAddr, strlen(newAddr), pEncodeAddr);

	pEncodeAddr[encodeLen] = '\0';

	return encodeLen;
}


int SmsPluginParamCodec::encodeSMSC(const SMS_ADDRESS_S *pAddress, unsigned char *pSMSC)
{
	char newAddr[MAX_SMSC_LEN+1];
	memset(newAddr, 0x00, sizeof(newAddr));

	int dataSize = 0, addrLen = 0;

	if (pAddress->address[0] == '+')
		memcpy(newAddr, pAddress->address+1, strlen(pAddress->address)-1);
	else
		memcpy(newAddr, pAddress->address, strlen(pAddress->address));

	addrLen = strlen(newAddr);

	if (addrLen % 2 == 0)
		dataSize = 2 + (addrLen/2);
	else
		dataSize = 2 + (addrLen/2) + 1;

	if (dataSize > MAX_SMSC_LEN) {
		MSG_DEBUG("addrLen is too long [%d]", addrLen);
		MSG_DEBUG("dataSize is too long [%d]", dataSize);

		return 0;
	}

	/* Set Address Length
	   Check IPC 4.0 -> addrLen/2 */
	pSMSC[0] = addrLen;

	/* Set TON, NPI */
	pSMSC[1] = 0x80 + (pAddress->ton << 4) + pAddress->npi;

	/* Set Address */
	convertDigitToBcd(newAddr, addrLen, &(pSMSC[2]));

	pSMSC[dataSize] = '\0';

	return dataSize;
}


/*==================================================================================================
                                     Decode Functions
==================================================================================================*/
int SmsPluginParamCodec::decodeAddress(const unsigned char *pTpdu, SMS_ADDRESS_S *pAddress)
{
	int offset = 0, addrLen = 0, bcdLen = 0;
	MsgTextConvert *textCvt = MsgTextConvert::instance();
	memset(pAddress->address, 0x00, sizeof(pAddress->address));

	addrLen = (int)pTpdu[offset++];

	if (addrLen % 2 == 0)
		bcdLen = addrLen/2;
	else
		bcdLen = addrLen/2 + 1;

	pAddress->ton = (pTpdu[offset] & 0x70) >> 4;
	pAddress->npi = pTpdu[offset++] & 0x0F;

	MSG_DEBUG("ton [%d]", pAddress->ton);
	MSG_DEBUG("npi [%d]", pAddress->npi);

	if (pAddress->ton == SMS_TON_ALPHANUMERIC) {
		MSG_DEBUG("Alphanumeric address");

		char* tmpAddress = new char[MAX_ADDRESS_LEN];
		int tmplength = 0;

		tmplength = SmsPluginUDCodec::unpack7bitChar(&(pTpdu[offset]), (addrLen*4)/7, 0, tmpAddress);

		MSG_LANG_INFO_S langInfo = {0, };

		langInfo.bSingleShift = false;
		langInfo.bLockingShift = false;

		textCvt->convertGSM7bitToUTF8((unsigned char*)pAddress->address, MAX_ADDRESS_LEN, (unsigned char*)tmpAddress, tmplength, &langInfo);

		if (tmpAddress)
			delete[] tmpAddress;
	} else if (pAddress->ton == SMS_TON_INTERNATIONAL) {
		convertBcdToDigit(&(pTpdu[offset]), bcdLen, &((pAddress->address)[1]));
		if (pAddress->address[1] != '\0')
			pAddress->address[0] = '+';
	} else {
		convertBcdToDigit(&(pTpdu[offset]), bcdLen, &((pAddress->address)[0]));
	}

	offset += 	bcdLen;

/*	MSG_DEBUG("address [%s]", pAddress->address);
*/
	return offset;
}


int SmsPluginParamCodec::decodeTime(const unsigned char *pTpdu, SMS_TIMESTAMP_S *pTimeStamp)
{
	int offset = 0;

	/* decode in ABSOLUTE time type. */
	pTimeStamp->format = SMS_TIME_ABSOLUTE;

	pTimeStamp->time.absolute.year = (pTpdu[offset] & 0x0F)*10 + ((pTpdu[offset] & 0xF0) >> 4);
	offset++;

	pTimeStamp->time.absolute.month = (pTpdu[offset] & 0x0F)*10 + ((pTpdu[offset] & 0xF0) >> 4);
	offset++;

	pTimeStamp->time.absolute.day = (pTpdu[offset] & 0x0F)*10 + ((pTpdu[offset] & 0xF0) >> 4);
	offset++;

	pTimeStamp->time.absolute.hour = (pTpdu[offset] & 0x0F)*10 + ((pTpdu[offset] & 0xF0) >> 4);
	offset++;

	pTimeStamp->time.absolute.minute = (pTpdu[offset] & 0x0F)*10 + ((pTpdu[offset] & 0xF0) >> 4);
	offset++;

	pTimeStamp->time.absolute.second = (pTpdu[offset] & 0x0F)*10 + ((pTpdu[offset] & 0xF0) >> 4);
	offset++;

	pTimeStamp->time.absolute.timeZone = (pTpdu[offset] & 0x07)*10 + ((pTpdu[offset] & 0xF0) >> 4);

	if (pTpdu[offset] & 0x08)
		pTimeStamp->time.absolute.timeZone *= (-1);

	offset++;

	return offset;
}


int SmsPluginParamCodec::decodeDCS(const unsigned char *pTpdu, SMS_DCS_S *pDCS)
{
	int offset = 0;
	char dcs = pTpdu[offset++];

	pDCS->bMWI = false;
	pDCS->bIndActive = false;
	pDCS->indType = SMS_OTHER_INDICATOR;

	if (((dcs & 0xC0) >> 6) == 0) {
		pDCS->codingGroup = SMS_GROUP_GENERAL;
		pDCS->bCompressed = (dcs & 0x20) >> 5;
		pDCS->codingScheme = (dcs & 0x0C) >> 2;

		if (((dcs & 0x10) >> 4) == 0)
			pDCS->msgClass = SMS_MSG_CLASS_NONE;
		else
			pDCS->msgClass = dcs & 0x03;
	} else if (((dcs & 0xF0) >> 4) == 0x0F) {
		pDCS->codingGroup = SMS_GROUP_CODING_CLASS;
		pDCS->bCompressed = false;
		pDCS->codingScheme = (dcs & 0x0C) >> 2;

		pDCS->msgClass = dcs & 0x03;
	} else if (((dcs & 0xC0) >> 6) == 1) {
		pDCS->codingGroup = SMS_GROUP_DELETION;
		pDCS->bCompressed = false;
		pDCS->msgClass = SMS_MSG_CLASS_NONE;

		/* TODO: finish here. ?? */
	} else if (((dcs & 0xF0) >> 4) == 0x0C) {
		pDCS->codingGroup = SMS_GROUP_DISCARD;
		pDCS->bCompressed = false;
		pDCS->msgClass = SMS_MSG_CLASS_NONE;

		pDCS->bMWI = true;
		pDCS->bIndActive = (((dcs & 0x08) >> 3) == 1)? true:false;
		pDCS->indType = (SMS_INDICATOR_TYPE_T)(dcs & 0x03);
	} else if (((dcs & 0xF0) >> 4) == 0x0D) {
		pDCS->codingGroup = SMS_GROUP_STORE;
		pDCS->codingScheme = SMS_CHARSET_7BIT;
		pDCS->bCompressed = false;
		pDCS->msgClass = SMS_MSG_CLASS_NONE;

		pDCS->bMWI = true;
		pDCS->bIndActive = (((dcs & 0x08) >> 3) == 1)? true:false;
		pDCS->indType = (SMS_INDICATOR_TYPE_T)(dcs & 0x03);
	} else if (((dcs & 0xF0) >> 4) == 0x0E) {
		pDCS->codingGroup = SMS_GROUP_STORE;
		pDCS->codingScheme = SMS_CHARSET_UCS2;
		pDCS->bCompressed = false;
		pDCS->msgClass = SMS_MSG_CLASS_NONE;

		pDCS->bMWI = true;
		pDCS->bIndActive = (((dcs & 0x08) >> 3) == 1)? true:false;
		pDCS->indType = (SMS_INDICATOR_TYPE_T)(dcs & 0x03);
	} else {
		pDCS->codingGroup = SMS_GROUP_UNKNOWN;

		pDCS->bCompressed = (dcs & 0x20) >> 5;
		pDCS->codingScheme = (dcs & 0x0C) >> 2;

		pDCS->msgClass = SMS_MSG_CLASS_NONE;
	}

	return offset;
}


void SmsPluginParamCodec::decodeSMSC(unsigned char* pAddress, int AddrLen, MSG_SMS_TON_T ton, char *pDecodeAddr)
{
	if (pAddress == NULL || AddrLen == 0)
		return;

	if (ton == SMS_TON_INTERNATIONAL) {
		pDecodeAddr[0] = '+';
		convertBcdToDigit(pAddress, AddrLen, &(pDecodeAddr[1]));
	} else {
		convertBcdToDigit(pAddress, AddrLen, pDecodeAddr);
	}
}


/*==================================================================================================
                                     Util Functions
==================================================================================================*/
int SmsPluginParamCodec::convertDigitToBcd(char *pDigit, int DigitLen, unsigned char *pBcd)
{
	int offset = 0;
	unsigned char temp;

/*	MSG_DEBUG("DigitLen [%d]", DigitLen);
	MSG_DEBUG("pDigit [%s]", pDigit); */

	for (int i = 0; i < DigitLen; i++) {
		if (pDigit[i] == '*')
			temp = 0x0A;
		else if (pDigit[i] == '#')
			temp = 0x0B;
		else if (pDigit[i] == 'P' || pDigit[i] == 'p')
			temp = 0x0C;
		else
			temp = pDigit[i] - '0';

		if ((i%2) == 0)
			pBcd[offset] = temp & 0x0F;
		else
			pBcd[offset++] |= ((temp & 0x0F) << 4);
	}

	if ((DigitLen%2) == 1)
		pBcd[offset++] |= 0xF0;

	return offset;
}


int SmsPluginParamCodec::convertBcdToDigit(const unsigned char *pBcd, int BcdLen, char *pDigit)
{
	int offset = 0;
	unsigned char temp;

	for (int i = 0; i < BcdLen; i++) {
		temp = pBcd[i] & 0x0F;

		if (temp == 0x0A)
			pDigit[offset++] = '*';
		else if (temp == 0x0B)
			pDigit[offset++] = '#';
		else if (temp == 0x0C)
			pDigit[offset++] = 'P';
		else
			pDigit[offset++] = temp + '0';

		temp = (pBcd[i] & 0xF0) >> 4;

		if (temp == 0x0F) {
			pDigit[offset] = '\0';
			return offset;
		}

		if (temp == 0x0A)
			pDigit[offset++] = '*';
		else if (temp == 0x0B)
			pDigit[offset++] = '#';
		else if (temp == 0x0C)
			pDigit[offset++] = 'P';
		else
			pDigit[offset++] = temp + '0';
	}

	pDigit[offset] = '\0';

	return offset;
}

bool SmsPluginParamCodec::checkCphsVmiMsg(const unsigned char *pTpdu, int *setType, int *indType)
{
	bool ret = false;

	int offset = 0;
	int addrLen = 0;

	addrLen = (int)pTpdu[offset++];

	if (addrLen == 0x04 && pTpdu[offset++] == 0xD0) {
		if (pTpdu[offset] == 0x11 || pTpdu[offset] == 0x10) {
			MSG_DEBUG("####### VMI msg ######");
			*setType = (int)(pTpdu[offset] & 0x01); /* 0 : clear, 1 : set */

			*indType = (int)(pTpdu[offset+1] & 0x01); /* 0 : indicator 1, 1 : indicator 2 */

			ret = true;
		}
	}

	return ret;
}

time_t SmsPluginParamCodec::convertTime(const SMS_TIMESTAMP_S *time_stamp)
{
	time_t rawtime;

	if (time_stamp->format == SMS_TIME_ABSOLUTE) {
		MSG_DEBUG("year : %d", time_stamp->time.absolute.year);
		MSG_DEBUG("month : %d", time_stamp->time.absolute.month);
		MSG_DEBUG("day : %d", time_stamp->time.absolute.day);
		MSG_DEBUG("hour : %d", time_stamp->time.absolute.hour);
		MSG_DEBUG("minute : %d", time_stamp->time.absolute.minute);
		MSG_DEBUG("second : %d", time_stamp->time.absolute.second);
		MSG_DEBUG("timezone : %d", time_stamp->time.absolute.timeZone);

		char displayTime[32];
		struct tm * timeTM;

		struct tm timeinfo;
		memset(&timeinfo, 0x00, sizeof(tm));

		timeinfo.tm_year = (time_stamp->time.absolute.year + 100);
		timeinfo.tm_mon = (time_stamp->time.absolute.month - 1);
		timeinfo.tm_mday = time_stamp->time.absolute.day;
		timeinfo.tm_hour = time_stamp->time.absolute.hour;
		timeinfo.tm_min = time_stamp->time.absolute.minute;
		timeinfo.tm_sec = time_stamp->time.absolute.second;
		timeinfo.tm_isdst = 0;

		rawtime = mktime(&timeinfo);

		MSG_DEBUG("tzname[0] [%s]", tzname[0]);
		MSG_DEBUG("tzname[1] [%s]", tzname[1]);
		MSG_DEBUG("timezone [%d]", timezone);
		MSG_DEBUG("daylight [%d]", daylight);

		memset(displayTime, 0x00, sizeof(displayTime));
		strftime(displayTime, 32, "%Y-%02m-%02d %T %z", &timeinfo);
		MSG_DEBUG("displayTime [%s]", displayTime);

		rawtime -= (time_stamp->time.absolute.timeZone * (3600/4));

		timeTM = localtime(&rawtime);
		memset(displayTime, 0x00, sizeof(displayTime));
		strftime(displayTime, 32, "%Y-%02m-%02d %T %z", timeTM);
		MSG_DEBUG("displayTime [%s]", displayTime);

/* timezone value is tiemzone + daylight. So should not add daylight */
#ifdef __MSG_DAYLIGHT_APPLIED__
		rawtime -= (timezone - daylight*3600);
#else
		rawtime -= timezone;
#endif

		timeTM = localtime(&rawtime);
		memset(displayTime, 0x00, sizeof(displayTime));
		strftime(displayTime, 32, "%Y-%02m-%02d %T %z", timeTM);
		MSG_DEBUG("displayTime [%s]", displayTime);
	} else {
		rawtime = time(NULL);
	}

	return rawtime;
}
