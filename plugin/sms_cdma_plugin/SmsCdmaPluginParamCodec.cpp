/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved
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

#include "SmsCdmaPluginParamCodec.h"


/*==================================================================================================
				IMPLEMENTATION OF SmsCdmaPluginParamCodec - Member Functions
==================================================================================================*/
SmsPluginParamCodec* SmsPluginParamCodec::pInstance = NULL;


SmsPluginParamCodec::SmsPluginParamCodec()
{


}


SmsPluginParamCodec::~SmsPluginParamCodec()
{


}


SmsPluginParamCodec* SmsPluginParamCodec::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginParamCodec();

	return pInstance;
}



/*==================================================================================================
                                     Util Functions
==================================================================================================*/
int SmsPluginParamCodec::convertDigitToBcd(char *pDigit, int DigitLen, unsigned char *pBcd)
{
	int offset = 0;
	unsigned char temp;

	for (int i = 0; i < DigitLen; i++) {
		if (pDigit[i] == '*')
			temp = 0x0A;
		else if (pDigit[i] == '#')
			temp = 0x0B;
		else if (pDigit[i] == 'P' || pDigit[i] == 'p')
			temp = 0x0C;
		else
			temp = pDigit[i] - '0';

		if ((i % 2) == 0)
			pBcd[offset] = temp & 0x0F;
		else
			pBcd[offset++] |= ((temp & 0x0F) << 4);
	}

	if ((DigitLen % 2) == 1) {
		pBcd[offset++] |= 0xF0;
	}

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


int SmsPluginParamCodec::convertDigitToDTMF(const char *pDigit, int DigitLen, int startBit, unsigned char *pDtmf)
{
	int shift = startBit;
	int offset = 0;
	int srcIdx = 0;
	unsigned char temp;

	if (shift > 7) {
		MSG_DEBUG("Invalid Param value shift : %d", shift);
		return 0;
	}
	/* shift 1 and shift 2 are supported in this spec. */
	if (shift >= 4) {
		MSG_DEBUG("Invalid Param value shift : %d", shift);
		return 0;
	}

	for (int i = 0; i < DigitLen; i++) {
		if (pDigit[srcIdx] == '*')
			temp = 0x0B;
		else if (pDigit[srcIdx] == '#')
			temp = 0x0C;
		else if (pDigit[srcIdx] == '0')
			temp = 0x0A;
		else
			temp = pDigit[srcIdx] - '0';

		temp &= 0x0F;

		if (shift == 0) {
			if (i % 2 == 1) {
				pDtmf[offset] |= temp;
				offset++;
			} else {
				pDtmf[offset] |= temp << 4;
			}
		} else if (shift >= 1 && shift < 4) {
			if (i % 2 == 1) {
				pDtmf[offset] |= (temp >> shift);
				pDtmf[offset + 1] = temp << (8 - shift);
				offset++;
			} else {
				pDtmf[offset] |= (temp << (8 - shift - 4));
			}
		}

		srcIdx++;
	}

	return offset;
}


int SmsPluginParamCodec::convertDTMFToDigit(const unsigned char *pDtmf, int DtmfLen, int startBit, char *pDigit)
{
	int shift = startBit;
	int offset = 0;
	int srcIdx = 0;
	unsigned char temp = 0;

	if (shift > 7) {
		MSG_DEBUG("Invalid Param value shift : %d", shift);
		return 0;
	}
	/* shift 1 and shift 2 are supported in this spec. */
	if (shift >= 4) {
		MSG_DEBUG("Invalid Param value shift : %d", shift);
		return 0;
	}

	for (int i = 0; i < DtmfLen; i++) {
		if (shift == 0) {
			if (i % 2 == 1) {
				temp = pDtmf[srcIdx] & 0x0F;
				srcIdx++;
			} else {
				temp = (pDtmf[srcIdx] >> 4) & 0x0F;
			}
		} else if (shift >= 1 && shift < 4) {
			if (i % 2 == 1) {
				temp = ((pDtmf[srcIdx] << shift) & 0x0F) + (pDtmf[srcIdx + 1] >> (8 - shift));
				srcIdx++;
			} else {
				temp = (pDtmf[srcIdx] >> (8 - shift - 4)) & 0x0F;
			}
		}

		if (temp == 0x0A)
			pDigit[offset++] = '0';
		else if (temp == 0x0B)
			pDigit[offset++] = '*';
		else if (temp == 0x0C)
			pDigit[offset++] = '#';
		else
			pDigit[offset++] = temp + '0';
	}

	pDigit[offset] = '\0';

	return offset;
}


bool SmsPluginParamCodec::isDtmfNumber(const char *pDigit, int DigitLen)
{
	bool isDtmf = true;

	for (int i = 0; i < DigitLen; i++) {
		if (!((pDigit[i] >= '0' && pDigit[i] <= '9') ||
			pDigit[i] == '*' ||
			pDigit[i] == '#')) {
			isDtmf = false;
			break;
		}
	}

	return isDtmf;
}
