/*
* Copyright 2012  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.tizenopensource.org/license
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "SmsPluginParamCodec.h"
#include "SmsPluginUDCodec.h"


/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginUDCodec - Member Functions
==================================================================================================*/
SmsPluginUDCodec::SmsPluginUDCodec()
{

}


SmsPluginUDCodec::~SmsPluginUDCodec()
{

}


int SmsPluginUDCodec::encodeUserData(const SMS_USERDATA_S *pUserData, SMS_CODING_SCHEME_T CodingScheme, char *pEncodeData)
{
	int encodeSize = 0;

	switch (CodingScheme)
	{
		case SMS_CHARSET_7BIT:
			encodeSize = encodeGSMData(pUserData, pEncodeData);
			break;
		case SMS_CHARSET_8BIT:
			encodeSize = encode8bitData(pUserData, pEncodeData);
			break;
		case SMS_CHARSET_UCS2:
			encodeSize = encodeUCS2Data(pUserData, pEncodeData);
			break;
	}

	return encodeSize;
}


int SmsPluginUDCodec::decodeUserData(const unsigned char *pTpdu, const int tpduLen, bool bHeaderInd, SMS_CODING_SCHEME_T CodingScheme, SMS_USERDATA_S *pUserData)
{
	int decodeSize = 0;

	memset(pUserData, 0x00, sizeof(SMS_USERDATA_S));

	switch (CodingScheme)
	{
		case SMS_CHARSET_7BIT:
			decodeSize = decodeGSMData(pTpdu, tpduLen, bHeaderInd, pUserData, NULL);
			break;
		case SMS_CHARSET_8BIT:
			decodeSize = decode8bitData(pTpdu, bHeaderInd, pUserData, NULL);
			break;
		case SMS_CHARSET_UCS2:
			decodeSize = decodeUCS2Data(pTpdu, tpduLen, bHeaderInd, pUserData, NULL);
			break;
	}

	return decodeSize;
}


int SmsPluginUDCodec::decodeUserData(const unsigned char *pTpdu, const int tpduLen, bool bHeaderInd, SMS_CODING_SCHEME_T CodingScheme, SMS_USERDATA_S *pUserData, SMS_TPUD_S *pTPUD)
{
	int decodeSize = 0;

	memset(pUserData, 0x00, sizeof(SMS_USERDATA_S));

	switch (CodingScheme)
	{
		case SMS_CHARSET_7BIT:
			decodeSize = decodeGSMData(pTpdu, tpduLen, bHeaderInd, pUserData, pTPUD);
			break;
		case SMS_CHARSET_8BIT:
			decodeSize = decode8bitData(pTpdu, bHeaderInd, pUserData, pTPUD);
			break;
		case SMS_CHARSET_UCS2:
			decodeSize = decodeUCS2Data(pTpdu, tpduLen, bHeaderInd, pUserData, pTPUD);
			break;
		case SMS_CHARSET_EUCKR:
			decodeSize = decodeUCS2Data(pTpdu, tpduLen, bHeaderInd, pUserData, pTPUD);
			break;
	}

	return decodeSize;
}


int SmsPluginUDCodec::encodeGSMData(const SMS_USERDATA_S *pUserData, char *pEncodeData)
{
	int headerLen = 0, offset = 0, fillBits = 0, packSize = 0, encodeLen = 0;
	unsigned char udhl = 0x00;

	if (pUserData->headerCnt > 0)
		offset = 2;
	else
		offset = 1;

MSG_DEBUG("pUserData->headerCnt [%d]", pUserData->headerCnt);

	// Encode User Data Header
	for (int i = 0; i < pUserData->headerCnt; i++)
	{
		headerLen = encodeHeader(pUserData->header[i], &(pEncodeData[offset]));

MSG_DEBUG("headerLen [%d]", headerLen);

		udhl += headerLen;
		offset += headerLen;
	}

MSG_DEBUG("udhl [%d]", udhl);

	if (udhl > 0)
		fillBits = ((udhl+1)*8)%7; // + UDHL

	if (fillBits > 0)
		fillBits = 7 - fillBits;

MSG_DEBUG("fillBits [%d]", fillBits);
MSG_DEBUG("dataLen [%d]", pUserData->length);

	// Set UDL, UDHL
	if (udhl > 0)
	{
		pEncodeData[0] = (((udhl+1)*8) + fillBits + (pUserData->length*7)) / 7;
		pEncodeData[1] =  udhl;
	}
	else
	{
		pEncodeData[0] = (char)pUserData->length;
	}

	packSize = pack7bitChar((unsigned char*)pUserData->data, pUserData->length, fillBits, &(pEncodeData[offset]));

	encodeLen = offset + packSize;

MSG_DEBUG("packSize [%d]", packSize);
MSG_DEBUG("encodeLen [%d]", encodeLen);

	return encodeLen;
}


int SmsPluginUDCodec::encode8bitData(const SMS_USERDATA_S *pUserData, char *pEncodeData)
{
	int headerLen = 0, offset = 2, fillBits = 0, encodeLen = 0;
	unsigned char udhl = 0x00;

	if (pUserData->headerCnt > 0)
		offset = 2;
	else
		offset = 1;

	// Encode User Data Header
	for (int i = 0; i < pUserData->headerCnt; i++)
	{
		headerLen = encodeHeader(pUserData->header[i], &(pEncodeData[offset]));

		udhl += headerLen;
		offset += headerLen;
	}

MSG_DEBUG("fillBits [%d]", fillBits);
MSG_DEBUG("dataLen [%d]", pUserData->length);

	// Set UDL, UDHL
	if (udhl > 0)
	{
		pEncodeData[0] = (udhl+1) + fillBits + pUserData->length;
		pEncodeData[1] =  udhl;
	}
	else
	{
		pEncodeData[0] = (char)pUserData->length;
	}

	memcpy(&(pEncodeData[offset]), pUserData->data, pUserData->length);

	encodeLen = offset + pUserData->length;

	return encodeLen;
}


int SmsPluginUDCodec::encodeUCS2Data(const SMS_USERDATA_S *pUserData, char *pEncodeData)
{
	int headerLen = 0, offset = 2, fillBits = 0, encodeLen = 0;
	unsigned char udhl = 0x00;

	if (pUserData->headerCnt > 0)
		offset = 2;
	else
		offset = 1;

	// Encode User Data Header
	for (int i = 0; i < pUserData->headerCnt; i++)
	{
		headerLen = encodeHeader(pUserData->header[i], &(pEncodeData[offset]));

		udhl += headerLen;
		offset += headerLen;
	}

MSG_DEBUG("fillBits [%d]", fillBits);
MSG_DEBUG("dataLen [%d]", pUserData->length);

	// Set UDL, UDHL
	if (udhl > 0)
	{
		pEncodeData[0] = (udhl+1) + fillBits + pUserData->length;
		pEncodeData[1] =  udhl;
	}
	else
	{
		pEncodeData[0] = (char)pUserData->length;
	}

	memcpy(&(pEncodeData[offset]), pUserData->data, pUserData->length);

	encodeLen = offset + pUserData->length;

	return encodeLen;
}


int SmsPluginUDCodec::decodeGSMData(const unsigned char *pTpdu, const int tpduLen, bool bHeaderInd, SMS_USERDATA_S *pUserData, SMS_TPUD_S *pTPUD)
{
	int offset = 0, udl = 0, udhl = 0, headerLen = 0, fillBits = 0, octetUdl = 0;

	// UDL
	udl = pTpdu[offset++];
	octetUdl = (udl*7)/8;

MSG_DEBUG("udl = %d, tpdulen = %d, octetUdl = %d.", udl, tpduLen, octetUdl);
MSG_DEBUG("bHeaderInd = %d", bHeaderInd);

	if (udl > MAX_GSM_7BIT_DATA_LEN || octetUdl > tpduLen)
	{
		pUserData->length = 0;
		pUserData->headerCnt = 0;
		return 0;
	}

	// Setting for Wap Push
	if (pTPUD != NULL)
	{
		pTPUD->udl = udl;

		memcpy(pTPUD->ud, &(pTpdu[offset]), udl);
		pTPUD->ud[udl] = '\0';
	}

	// Decode User Data Header
	if (bHeaderInd == true)
	{
		// UDHL
		udhl = pTpdu[offset++];

		MSG_DEBUG("udhl = %d", udhl);

		pUserData->headerCnt = 0;

		for (int i = 0; offset < udhl; i++)
		{
			headerLen = decodeHeader(&(pTpdu[offset]), &(pUserData->header[i]));

			if (headerLen <= 0) {
				MSG_DEBUG("Error to decode User Data Header. headerLen [%d]", headerLen);

				pUserData->length = 0;
				memset(pUserData->data, 0x00, sizeof(pUserData->data));

				return 0;
			}

			offset += headerLen;

			if (offset > (udhl+2)) {
				MSG_DEBUG("Error to decode User Data Header. offset [%d] > (udhl [%d] + 2)", offset, udhl);

				pUserData->length = 0;
				memset(pUserData->data, 0x00, sizeof(pUserData->data));

				return 0;
			}

			pUserData->headerCnt++;
		}
	}
	else
		pUserData->headerCnt = 0;

	MSG_DEBUG("headerCnt = %d", pUserData->headerCnt);

	if (udhl > 0)
	{
		fillBits = ((udl*7) - ((udhl+1)*8)) % 7;
		udl = ((udl*7) - ((udhl+1)*8)) / 7;
	}

MSG_DEBUG("fillBits = %d", fillBits);
MSG_DEBUG("udhl = %d", udhl);
MSG_DEBUG("udl = %d", udl);

MSG_DEBUG("offset = %d", offset);

	pUserData->length = unpack7bitChar(&(pTpdu[offset]), udl, fillBits, pUserData->data);

	//MSG_DEBUG("data = [%s]", pUserData->data);
	//MSG_DEBUG("length = [%d]", pUserData->length);


	return pUserData->length;
}


int SmsPluginUDCodec::decode8bitData(const unsigned char *pTpdu, bool bHeaderInd, SMS_USERDATA_S *pUserData, SMS_TPUD_S *pTPUD)
{
	int offset = 0, udl = 0, udhl = 0, headerLen = 0;

	// UDL
	udl = pTpdu[offset++];

	if (udl > MAX_UCS2_DATA_LEN)
	{
		pUserData->length = 0;
		return 0;
	}

	// Setting for Wap Push
	if (pTPUD != NULL)
	{
		pTPUD->udl = udl;

		memcpy(pTPUD->ud, &(pTpdu[offset]), udl);
		pTPUD->ud[udl] = '\0';
	}

MSG_DEBUG("udl = %d", udl);
MSG_DEBUG("bHeaderInd = %d", bHeaderInd);

	// Decode User Data Header
	if (bHeaderInd == true)
	{
		// UDHL
		udhl = pTpdu[offset++];

		MSG_DEBUG("udhl = %d", udhl);

		pUserData->headerCnt = 0;

		for (int i = 0; offset < udhl; i++)
		{
			headerLen = decodeHeader(&(pTpdu[offset]), &(pUserData->header[i]));

			if (headerLen <= 0) {
				MSG_DEBUG("Error to decode User Data Header. headerLen [%d]", headerLen);

				pUserData->length = 0;
				memset(pUserData->data, 0x00, sizeof(pUserData->data));

				return 0;
			}

			offset += headerLen;

			if (offset > (udhl+2)) {
				MSG_DEBUG("Error to decode User Data Header. offset [%d] > (udhl [%d] + 2)", offset, udhl);

				pUserData->length = 0;
				memset(pUserData->data, 0x00, sizeof(pUserData->data));

				return 0;
			}

			pUserData->headerCnt++;
		}
	}
	else
		pUserData->headerCnt = 0;

MSG_DEBUG("headerCnt = %d", pUserData->headerCnt);

	if (udhl > 0)
		pUserData->length = (udl) - (udhl+1);
	else
		pUserData->length = udl;

MSG_DEBUG("pUserData->length = %d", pUserData->length);
MSG_DEBUG("offset = %d", offset);

	memcpy(pUserData->data, &(pTpdu[offset]), pUserData->length);

	return pUserData->length;
}


int SmsPluginUDCodec::decodeUCS2Data(const unsigned char *pTpdu, const int tpduLen, bool bHeaderInd, SMS_USERDATA_S *pUserData, SMS_TPUD_S *pTPUD)
{
	int offset = 0, udl = 0, udhl = 0, headerLen = 0;

	// UDL
	udl = pTpdu[offset++];

MSG_DEBUG("udl = %d, tpdulen = %d.", udl, tpduLen);
MSG_DEBUG("bHeaderInd = %d", bHeaderInd);

	if (udl > MAX_UCS2_DATA_LEN || udl > tpduLen)
	{
		pUserData->length = 0;
		pUserData->headerCnt = 0;
		return 0;
	}

	// Setting for Wap Push
	if (pTPUD != NULL)
	{
		pTPUD->udl = udl;

		memcpy(pTPUD->ud, &(pTpdu[offset]), udl);
		pTPUD->ud[udl] = '\0';
	}

	// Decode User Data Header
	if (bHeaderInd == true)
	{
		// UDHL
		udhl = pTpdu[offset++];

		MSG_DEBUG("udhl = %d", udhl);

		pUserData->headerCnt = 0;

		for (int i = 0; offset < udhl; i++)
		{
			headerLen = decodeHeader(&(pTpdu[offset]), &(pUserData->header[i]));

			if (headerLen <= 0) {
				MSG_DEBUG("Error to decode User Data Header. headerLen [%d]", headerLen);

				pUserData->length = 0;
				memset(pUserData->data, 0x00, sizeof(pUserData->data));

				return 0;
			}

			offset += headerLen;

			if (offset > (udhl+2)) {
				MSG_DEBUG("Error to decode User Data Header. offset [%d] > (udhl [%d] + 2)", offset, udhl);

				pUserData->length = 0;
				memset(pUserData->data, 0x00, sizeof(pUserData->data));

				return 0;
			}

			pUserData->headerCnt++;
		}
	}
	else
		pUserData->headerCnt = 0;

	if (udhl > 0)
		pUserData->length = (udl) - (udhl+1);
	else
		pUserData->length = udl;

MSG_DEBUG("pUserData->length = %d", pUserData->length);
MSG_DEBUG("offset = %d", offset);

	memcpy(pUserData->data, &(pTpdu[offset]), pUserData->length);
	pUserData->data[pUserData->length] = 0;

	return pUserData->length;
}


int SmsPluginUDCodec::encodeHeader(const SMS_UDH_S header, char *pEncodeHeader)
{
	int offset = 0, addrLen = 0;

	char* encodedAddr = NULL;
	AutoPtr<char> addressBuf(&encodedAddr);

	switch (header.udhType)
	{
		case SMS_UDH_CONCAT_8BIT :
			// IEI
			pEncodeHeader[offset++] = SMS_UDH_CONCAT_8BIT;

			// IEDL
			pEncodeHeader[offset++] = 0x03;

			// Reference Number
			pEncodeHeader[offset++] = header.udh.concat8bit.msgRef;

			// Number of Segments
			pEncodeHeader[offset++] = header.udh.concat8bit.totalSeg;

			// Sequence Number
			pEncodeHeader[offset++] = header.udh.concat8bit.seqNum;
		break;

		case SMS_UDH_CONCAT_16BIT :
			// IEI
			pEncodeHeader[offset++] = SMS_UDH_CONCAT_16BIT;

			// IEDL
			pEncodeHeader[offset++] = 0x04;

			// Reference Number
			pEncodeHeader[offset++] = (char)(header.udh.concat16bit.msgRef >> 8);
			pEncodeHeader[offset++] = header.udh.concat16bit.msgRef & 0x00FF;

			// Number of Segments
			pEncodeHeader[offset++] = header.udh.concat16bit.totalSeg;

			// Sequence Number
			pEncodeHeader[offset++] = header.udh.concat16bit.seqNum;
		break;

		case SMS_UDH_APP_PORT_8BIT :
			// IEI
			pEncodeHeader[offset++] = SMS_UDH_APP_PORT_8BIT;

			// IEDL
			pEncodeHeader[offset++] = 0x02;

			// Dest Port
			pEncodeHeader[offset++] = header.udh.appPort8bit.destPort;

			// Origin Port
			pEncodeHeader[offset++] = header.udh.appPort8bit.originPort;
		break;

		case SMS_UDH_APP_PORT_16BIT :
			// IEI
			pEncodeHeader[offset++] = SMS_UDH_APP_PORT_16BIT;

			// IEDL
			pEncodeHeader[offset++] = 0x04;

			// Dest Port
			pEncodeHeader[offset++] = (char)(header.udh.appPort16bit.destPort >> 8);
			pEncodeHeader[offset++] = header.udh.appPort16bit.destPort & 0x00FF;

			// Origin Port
			pEncodeHeader[offset++] = (char)(header.udh.appPort16bit.originPort >> 8);
			pEncodeHeader[offset++] = header.udh.appPort16bit.originPort & 0x00FF;
		break;

		case SMS_UDH_ALTERNATE_REPLY_ADDRESS :
			// IEI
			pEncodeHeader[offset++] = SMS_UDH_ALTERNATE_REPLY_ADDRESS;

			addrLen = SmsPluginParamCodec::encodeAddress(&(header.udh.alternateAddress), &encodedAddr);

			// IEDL
			pEncodeHeader[offset++] = addrLen;

			// Alternate Reply Address
			memcpy(&pEncodeHeader[offset], encodedAddr, addrLen);

			offset += addrLen;
		break;

		case SMS_UDH_SINGLE_SHIFT :
			// IEI
			pEncodeHeader[offset++] = SMS_UDH_SINGLE_SHIFT;

			// IEDL
			pEncodeHeader[offset++] = 0x01;

			// National Language Identifier
			pEncodeHeader[offset++] = header.udh.singleShift.langId;
		break;

		case SMS_UDH_LOCKING_SHIFT :
			// IEI
			pEncodeHeader[offset++] = SMS_UDH_LOCKING_SHIFT;

			// IEDL
			pEncodeHeader[offset++] = 0x01;

			// National Language Identifier
			pEncodeHeader[offset++] = header.udh.lockingShift.langId;
		break;

		case SMS_UDH_NONE :
		default :
		break;
	}

	return offset;
}


int SmsPluginUDCodec::decodeHeader(const unsigned char *pTpdu, SMS_UDH_S *pHeader)
{
	int offset = 0;
	unsigned char IEDL = 0;

	pHeader->udhType = pTpdu[offset++];

	switch (pHeader->udhType)
	{
		case SMS_UDH_CONCAT_8BIT :
		{
			IEDL = pTpdu[offset++];

			if (IEDL == 0) return 0;

			pHeader->udh.concat8bit.msgRef = pTpdu[offset++];
			pHeader->udh.concat8bit.totalSeg = pTpdu[offset++];
			pHeader->udh.concat8bit.seqNum = pTpdu[offset++];

MSG_DEBUG("concat8bit.msgRef [%02x]", pHeader->udh.concat8bit.msgRef);
MSG_DEBUG("concat8bit.totalSeg [%02x]", pHeader->udh.concat8bit.totalSeg);
MSG_DEBUG("concat8bit.seqNum [%02x]", pHeader->udh.concat8bit.seqNum);
		}
		break;

		case SMS_UDH_CONCAT_16BIT :
		{
			IEDL = pTpdu[offset++];

			if (IEDL == 0) return 0;

			pHeader->udh.concat16bit.msgRef = pTpdu[offset++];
			pHeader->udh.concat16bit.msgRef = (unsigned short)((pHeader->udh.concat16bit.msgRef << 8) | pTpdu[offset++]);
			pHeader->udh.concat16bit.totalSeg = pTpdu[offset++];
			pHeader->udh.concat16bit.seqNum = pTpdu[offset++];

MSG_DEBUG("concat16bit.msgRef [%04x]", pHeader->udh.concat16bit.msgRef);
MSG_DEBUG("concat16bit.totalSeg [%02x]", pHeader->udh.concat16bit.totalSeg);
MSG_DEBUG("concat16bit.seqNum [%02x]", pHeader->udh.concat16bit.seqNum);
		}
		break;

		case SMS_UDH_APP_PORT_8BIT :
		{
			IEDL = pTpdu[offset++];

			if (IEDL == 0) return 0;

			pHeader->udh.appPort8bit.destPort = pTpdu[offset++];
			pHeader->udh.appPort8bit.originPort = pTpdu[offset++];

MSG_DEBUG("appPort8bit.destPort [%02x]", pHeader->udh.appPort8bit.destPort);
MSG_DEBUG("appPort8bit.originPort [%02x]", pHeader->udh.appPort8bit.originPort);
		}
		break;

		case SMS_UDH_APP_PORT_16BIT :
		{
			IEDL = pTpdu[offset++];

			if (IEDL == 0) return 0;

			pHeader->udh.appPort16bit.destPort = pTpdu[offset++];
			pHeader->udh.appPort16bit.destPort = (unsigned short)((pHeader->udh.appPort16bit.destPort << 8) | pTpdu[offset++]);
			pHeader->udh.appPort16bit.originPort = pTpdu[offset++];
			pHeader->udh.appPort16bit.originPort = (unsigned short)((pHeader->udh.appPort16bit.originPort << 8) | pTpdu[offset++]);

MSG_DEBUG("appPort16bit.destPort [%04x]", pHeader->udh.appPort16bit.destPort);
MSG_DEBUG("appPort16bit.originPort [%04x]", pHeader->udh.appPort16bit.originPort);
		}
		break;

		case SMS_UDH_SPECIAL_SMS :
		{
			IEDL = pTpdu[offset++];

			if (IEDL != 2) return 0;
MSG_DEBUG("Decoding special sms udh.");

			pHeader->udh.specialInd.bStore = (bool) (pTpdu[offset] & 0x80);
			pHeader->udh.specialInd.msgInd = (unsigned short) (pTpdu[offset++] & 0x7F);
			pHeader->udh.specialInd.waitMsgNum = (unsigned short) pTpdu[offset];
		}
		break;

		case SMS_UDH_ALTERNATE_REPLY_ADDRESS :
		{
			IEDL = pTpdu[offset++];

			if (IEDL == 0) return 0;

			offset += SmsPluginParamCodec::decodeAddress(&pTpdu[offset], &(pHeader->udh.alternateAddress));

MSG_DEBUG("alternate reply address [%s]", pHeader->udh.alternateAddress.address);
		}
		break;

		case SMS_UDH_SINGLE_SHIFT :
		{
			IEDL = pTpdu[offset++];

			if (IEDL == 0) return 0;

			pHeader->udh.singleShift.langId = pTpdu[offset++];

MSG_DEBUG("singleShift.langId [%02x]", pHeader->udh.singleShift.langId);
		}
		break;

		case SMS_UDH_LOCKING_SHIFT :
		{
			IEDL = pTpdu[offset++];

			if (IEDL == 0) return 0;

			pHeader->udh.lockingShift.langId = pTpdu[offset++];

MSG_DEBUG("lockingShift.langId [%02x]", pHeader->udh.lockingShift.langId);
		}
		break;

		default :
		{
			MSG_DEBUG("Not Supported Header Type [%02x]", pHeader->udhType);

			IEDL = pTpdu[offset++];

			MSG_DEBUG("IEDL [%d]", IEDL);
			return (offset + IEDL);
		}
		break;
	}

	return offset;
}


int SmsPluginUDCodec::pack7bitChar(const unsigned char *pUserData, int dataLen, int fillBits, char *pPackData)
{
	int srcIdx = 0, dstIdx = 0, shift = fillBits;

//MSG_DEBUG("dataLen = %d", dataLen);

	if (shift > 0)
		dstIdx = 1;

	while (srcIdx < dataLen)
	{
		if (shift == 0)
		{
//			if (srcIdx > 0) srcIdx++;

//MSG_DEBUG("pUserData [%02x]", pUserData[srcIdx]);
//MSG_DEBUG("shift = %d", shift);

			pPackData[dstIdx] = pUserData[srcIdx];

//MSG_DEBUG("pPackData [%02x]", pPackData[dstIdx]);
			if (srcIdx >= dataLen) break;

			shift = 7;
			srcIdx++;
			dstIdx++;
		}

		if (shift > 1)
		{
//MSG_DEBUG("pUserData [%02x]", pUserData[srcIdx]);

//MSG_DEBUG("shift = %d", shift);

			pPackData[dstIdx-1] |= pUserData[srcIdx] << shift;
			pPackData[dstIdx] = pUserData[srcIdx] >> (8-shift);
			shift--;
//MSG_DEBUG("pPackData [%02x]", pPackData[dstIdx]);

			srcIdx++;
			dstIdx++;
		}
		else if (shift == 1)
		{
//MSG_DEBUG("pUserData [%02x]", pUserData[srcIdx]);
//MSG_DEBUG("shift = %d", shift);
			pPackData[dstIdx-1] |= pUserData[srcIdx] << shift;

//MSG_DEBUG("pPackData [%02x]", pPackData[dstIdx-1]);

			srcIdx++;

			shift--;
		}
	}

//MSG_DEBUG("dstIdx = %d", dstIdx);

	return dstIdx;
}


int SmsPluginUDCodec::unpack7bitChar(const unsigned char *pTpdu, unsigned char dataLen, int fillBits, char *pUnpackData)
{
	int srcIdx = 0, dstIdx = 0, shift = fillBits;

MSG_DEBUG("dataLen = %d", dataLen);

	if (shift > 0)
		srcIdx = 1;

	for (; dstIdx < dataLen; dstIdx++)
	{
		if (shift == 0)
		{
//MSG_DEBUG("shift = %d", shift);

			pUnpackData[dstIdx] = pTpdu[srcIdx] & 0x7F;

//MSG_DEBUG("UserData[%d] = %02x", dstIdx, pUnpackData[dstIdx]);
			shift = 7;
			srcIdx++;
			dstIdx++;

			if (dstIdx >= dataLen) break;
		}

		if (shift > 0)
		{
//MSG_DEBUG("shift = %d", shift);

			pUnpackData[dstIdx] = (pTpdu[srcIdx-1] >> shift) + (pTpdu[srcIdx] << (8 - shift));

			pUnpackData[dstIdx] &= 0x7F;

//MSG_DEBUG("UserData[%d] = %02x", dstIdx, pUnpackData[dstIdx]);

			shift--;

			if (shift > 0) srcIdx++;
		}
	}

	return dstIdx;
}

