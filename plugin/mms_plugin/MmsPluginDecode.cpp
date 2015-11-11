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
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

#include "MsgUtilFile.h"
#include "MsgUtilMime.h"
#include "MsgSmil.h"
#include "MsgMmsMessage.h"

#include "MmsPluginDebug.h"
#include "MmsPluginDecode.h"
#include "MmsPluginCodecCommon.h"
#include "MmsPluginStorage.h"
#include "MmsPluginDebug.h"
#include "MmsPluginTextConvert.h"
#include "MmsPluginUtil.h"

#include "MmsPluginDrm.h"
#include "MsgDrmWrapper.h"

/*Decode wsp*/
static int __MmsGetDecodeOffset(void);
static bool __MmsDecodeInitialize(void);
static void __MmsCleanDecodeBuff(void);
static bool __MmsBinaryDecodeMovePointer(FILE *pFile, int offset, int totalLength);
static bool	__MmsBinaryDecodeCheckAndDecreaseLength(int *pLength, int valueLength);

static bool __MmsBinaryDecodeGetBytes(FILE *pFile, char *szBuff, int bufLen, int totalLength);		/* bufLen < gMmsDecodeMaxLen */
static bool __MmsBinaryDecodeGetLongBytes(FILE *pFile, char *szBuff, int bufLen, int totalLength);	/* no bufLen limit */
static bool __MmsBinaryDecodeGetOneByte(FILE *pFile, UINT8 *pOneByte, int totalLength);

static UINT32 __MmsHeaderDecodeIntegerByLength(FILE *pFile, UINT32 length, int totalLength);

static bool __MmsBinaryDecodeInteger(FILE *pFile, UINT32 *pInteger, int *pIntLen, int totalLength);
static bool __MmsDecodeLongInteger(FILE *pFile, UINT32 *pLongInteger, int totalLength);
static int __MmsBinaryDecodeUintvar(FILE *pFile, UINT32 *pUintVar, int totalLength);
static int __MmsDecodeValueLength(FILE *pFile, UINT32 *pValueLength, int totalLength);
static int __MmsDecodeValueLength2(FILE *pFile, UINT32 *pValueLength, int totalLength);
static int __MmsBinaryDecodeText(FILE *pFile, char *szBuff, int bufLen, int totalLength);
static char *__MmsBinaryDecodeText2(FILE *pFile, int totalLength, int *pLength);
static int __MmsBinaryDecodeQuotedString(FILE *pFile, char *szBuff, int bufLen, int totalLength);
static bool __MmsBinaryDecodeEncodedString(FILE *pFile, char *szBuff, int bufLen, int totalLength);

static bool __MmsBinaryDecodeCharset(FILE *pFile, UINT32 *nCharSet, int *pCharSetLen, int totalLength);
static int __MmsDecodeGetFilename(FILE *pFile, char *szBuff, int bufLen, int totalLength);
static MsgHeaderAddress *__MmsDecodeEncodedAddress(FILE *pFile, int totalLength);

static bool __MmsBinaryDecodeMultipart(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, int totalLength);
static bool __MmsBinaryDecodeEachPart(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, int totalLength);
static bool __MmsBinaryDecodePartHeader(FILE *pFile, MsgType *pMsgType, int headerLen, int totalLength);
static bool __MmsBinaryDecodePartBody(FILE *pFile, UINT32 bodyLength, int totalLength);
static bool __MmsBinaryDecodeEntries(FILE *pFile, UINT32 *npEntries, int totalLength);
static bool __MmsBinaryDecodeParameter(FILE *pFile, MsgType *pMsgType, int valueLength, int totalLength);

static int __MmsBinaryDecodeContentType(FILE *pFile, MsgType *pMsgType, int totalLength);


/* util funcion */
static void __MsgRemoveFilePath(char *pSrc);
static bool __MsgChangeSpace(char *pOrg, char **ppNew);
static void __MsgConfirmPresentationPart(MsgType *pMsgType, MsgBody *pMsgBody, MsgPresentaionInfo *pPresentationInfo);
static MsgPresentationFactor __MsgIsPresentationEx(MsgType *multipartType, char* szStart, MimeType typeParam);
static bool __MsgLoadDataToDecodeBuffer(FILE *pFile, char **ppBuf, int *pPtr, int *pOffset, char *pInBuf1, char *pInBuf2, int maxLen, int *npRead, int endOfFile);
static bool __MsgFreeHeaderAddress(MsgHeaderAddress *pAddr);
static bool __MsgCheckFileNameHasInvalidChar(char *szName);

static bool __MsgReplaceInvalidFileNameChar(char *szInText, char replaceChar);
static char *__MsgGetStringUntilDelimiter(char *pszString, char delimiter);
static bool __MsgParseParameter(MsgType *pType, char *pSrc);
static char *__MsgSkipWS(char *s);
static char *__MsgSkipComment(char *s, long trim);

static char *__MsgConvertLatin2UTF8FileName(char *pSrc);

/* static bool __MsgIsPercentSign(char *pSrc); */
static bool __MsgIsMultipartRelated(int type);
static bool __MsgIsPresentablePart(int type);
static bool __MsgResolveNestedMultipart(MsgType *pPartType, MsgBody *pPartBody);
static bool __MsgIsHexChar(char *pSrc);
static char __MsgConvertHexValue(char *pSrc);
static int __MsgConvertCharToInt(char ch);
static bool __MsgCopyNestedMsgType(MsgType *pMsgType1, MsgType *pMsgType2);
static bool __MsgCopyNestedMsgParam(MsgContentParam *pParam1, MsgContentParam *pParam2);
static bool __MsgIsMultipartMixed(int type);

static bool __MsgIsInvalidFileNameChar(char ch);

static int __MsgGetLatin2UTFCodeSize(unsigned char *szSrc, int nChar);
static int __MsgLatin2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar);
static int __MsgCutUTFString(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar);
static void __MsgMIMERemoveQuote(char *szSrc);
static bool __MmsMultipartSaveAsTempFile(MsgType *pPartType, MsgBody *pPartBody, char *pszMailboxPath, char *pszMsgFilename, int index, bool bSave);

static bool __MmsGetMediaPartData(MsgType *pPartType, MsgBody *pPartBody, FILE *pFile);
static char *__MmsGetBinaryUTF8Data(char *pData, int nRead, int msgEncodingValue, int msgTypeValue, int msgCharsetValue, int *npRead);

static bool __MsgMakeFileName(int iMsgType, char *szFileName, MsgDrmType drmType, int nUntitleIndex, char *outBuf, int outBufLen);


__thread char gszMmsLoadBuf1[MSG_MMS_DECODE_BUFFER_MAX + 1] = {0, };
__thread char gszMmsLoadBuf2[MSG_MMS_DECODE_BUFFER_MAX + 1] = {0, };

__thread char *gpCurMmsDecodeBuff = NULL;
__thread int gCurMmsDecodeBuffPos = 0;	/* next decoding position in gpCurMmsDecodeBuff  */
__thread int gMmsDecodeMaxLen = 0;
__thread int gMmsDecodeCurOffset = 0;	/* current offset in file (last read) */
__thread int gMmsDecodeBufLen = 0;		/* number of last read characters */

__thread char *gpMmsDecodeBuf1 = NULL;
__thread char *gpMmsDecodeBuf2 = NULL;

__thread MmsHeader mmsHeader =
{
	(MmsMsgType)MMS_MSGTYPE_ERROR,			/* MmsMsgType			iType; */
	"",										/* char[]				szTrID; */
	0,										/* short int				version; */
	0,										/* UINT32				date; */

	NULL,									/* MsgHeaderAddress*		pFrom; */
	NULL,									/* MsgHeaderAddress*		pTo; */
	NULL,									/* MsgHeaderAddress*		pCc; */
	NULL,									/* MsgHeaderAddress*		pBcc; */
	"",										/* char[]				szSubject; */
	(MmsResponseStatus)MMS_RESPSTATUS_OK,	/* MmsResponseStatus		iResponseStatus; */
	(MmsRetrieveStatus)MMS_RETRSTATUS_OK,	/* MmsRetrieveStatus		iRetrieveStatus; */
	"",										/* char[]				szResponseText; */
	"",										/* char[]				szRetrieveText; */


	/* has default value in specification */

	(MmsMsgClass)MMS_MSGCLASS_PERSONAL,		/* MmsMsgClass			msgClass; */
	{MMS_TIMETYPE_RELATIVE, 0},				/* MmsTimeStruct			expiryTime; */
	{MMS_TIMETYPE_RELATIVE, 0},				/* MmsTimeStruct			deliveryTime; */
	(MmsPriority)MMS_PRIORITY_NORMAL,		/* MmsPriority			priority; */ /* Refer [OMA-MMS-ENC-v1_2-20030915-C] */
	(MmsSenderVisible)MMS_SENDER_SHOW,		/* MmsSenderVisible		senderVisible; */
	(MmsReport)MMS_REPORT_NO,				/* MmsReport				deliveryReport; */
	(MmsReport)MMS_REPORT_NO,				/* MmsReport				readReply; */
	(MmsReportAllowed)MMS_REPORTALLOWED_NO, /* MmsReportAllowed		iReportAllowed; */
	"",										/* char[]				szContentLocation; */


	/* there is no right default value */

	(msg_delivery_report_status_t)MSG_DELIVERY_REPORT_NONE,		/* MmsMsgStatus			iMsgStatus; */
	(msg_read_report_status_t)MSG_READ_REPORT_NONE,		/* MmsReadStatus			readStatus; */

	/* MMS v1.1 ReplyCharge */
	{
		(MmsReplyChargeType)MMS_REPLY_NONE,	/* MmsReplyChargeType	chargeType; */
		{MMS_TIMETYPE_RELATIVE, 0},			/* MmsTimeStruct			deadLine; */
		0,									/* int					chargeSize; */
		"" ,								/* char					szChargeID; */
	},

	"",										/* char[]				szMsgID; */
	0,										/* UINT32				msgSize; */
};

#define	MMS_DRM2_CONVERT_BUFFER_MAX	4*1024
const UINT32 MMS_UINTVAR_LENGTH_1 =  0x0000007f;		/* 7bit */
const UINT32 MMS_UINTVAR_LENGTH_2 =  0x00003fff;		/* 14bit */
const UINT32 MMS_UINTVAR_LENGTH_3 =  0x001fffff;		/* 21bit */

static bool __MmsDecodeInitialize(void)
{
	MmsInitMsgType(&mmsHeader.msgType);
	MmsInitMsgBody(&mmsHeader.msgBody);
	return true;
}

void MmsInitHeader()
{
	mmsHeader.type = MMS_MSGTYPE_ERROR;

	memset(mmsHeader.szTrID, 0, MMS_TR_ID_LEN + 1);
	mmsHeader.version = MMS_VERSION;
	mmsHeader.date = 0;

	__MsgFreeHeaderAddress(mmsHeader.pFrom);
	__MsgFreeHeaderAddress(mmsHeader.pTo);
	__MsgFreeHeaderAddress(mmsHeader.pCc);
	__MsgFreeHeaderAddress(mmsHeader.pBcc);

	mmsHeader.pFrom	= NULL;
	mmsHeader.pTo = NULL;
	mmsHeader.pCc = NULL;
	mmsHeader.pBcc = NULL;

	memset(mmsHeader.szSubject, 0, MSG_LOCALE_SUBJ_LEN + 1);

	mmsHeader.responseStatus = (MmsResponseStatus)MMS_RESPSTATUS_OK;
	mmsHeader.retrieveStatus = (MmsRetrieveStatus)MMS_RETRSTATUS_OK;
	memset(mmsHeader.szResponseText, 0, MMS_LOCALE_RESP_TEXT_LEN + 1);
	memset(mmsHeader.szRetrieveText, 0, MMS_LOCALE_RESP_TEXT_LEN + 1);


	mmsHeader.msgClass = (MmsMsgClass)MMS_MSGCLASS_PERSONAL;
	mmsHeader.expiryTime.type = MMS_TIMETYPE_RELATIVE;
	mmsHeader.expiryTime.time = 0;
	mmsHeader.deliveryTime.type	= MMS_TIMETYPE_RELATIVE;
	mmsHeader.deliveryTime.time	= 0;
	mmsHeader.priority = (MmsPriority)MMS_PRIORITY_NORMAL;	/* Refer [OMA-MMS-ENC-v1_2-20030915-C] */
	mmsHeader.hideAddress =(MmsSenderVisible)MMS_SENDER_SHOW;
	mmsHeader.deliveryReport = (MmsReport)MMS_REPORT_NO;
	mmsHeader.readReply = (MmsReport)MMS_REPORT_NO;
	mmsHeader.reportAllowed = (MmsReportAllowed)MMS_REPORTALLOWED_YES;
	memset(mmsHeader.szContentLocation, 0, MMS_LOCATION_LEN + 1);


	mmsHeader.msgStatus = (msg_delivery_report_status_t)MSG_DELIVERY_REPORT_NONE;
	mmsHeader.readStatus = (msg_read_report_status_t)MSG_READ_REPORT_NONE;

	mmsHeader.replyCharge.chargeType = (MmsReplyChargeType)MMS_REPLY_NONE;
	mmsHeader.replyCharge.chargeSize = 0;
	mmsHeader.replyCharge.deadLine.type = MMS_TIMETYPE_RELATIVE;
	mmsHeader.replyCharge.deadLine.time = 0;
	memset(mmsHeader.replyCharge.szChargeID, 0, MMS_MSG_ID_LEN + 1);


	memset(mmsHeader.szMsgID, 0, MMS_MSG_ID_LEN + 1);
	mmsHeader.msgSize = 0;
	mmsHeader.drmType = MSG_DRM_TYPE_NONE;

	__MmsDecodeInitialize();
}

void MmsReleaseHeader(MmsHeader *mms)
{
	__MsgFreeHeaderAddress(mms->pFrom);
	__MsgFreeHeaderAddress(mms->pTo);
	__MsgFreeHeaderAddress(mms->pCc);
	__MsgFreeHeaderAddress(mms->pBcc);

	mmsHeader.pFrom = NULL;
	mmsHeader.pTo = NULL;
	mmsHeader.pCc = NULL;
	mmsHeader.pBcc = NULL;

}

static void __MmsCleanDecodeBuff(void)
{
	memset(gpMmsDecodeBuf1, 0, gMmsDecodeMaxLen + 1);
	memset(gpMmsDecodeBuf2, 0, gMmsDecodeMaxLen + 1);
	gpCurMmsDecodeBuff = NULL;
	gCurMmsDecodeBuffPos = 0;
	gMmsDecodeBufLen = 0;
}

void MmsRegisterDecodeBuffer()
{
	gpMmsDecodeBuf1 = gszMmsLoadBuf1;
	gpMmsDecodeBuf2 = gszMmsLoadBuf2;
	gpCurMmsDecodeBuff = NULL;
	gCurMmsDecodeBuffPos = 0;
	gMmsDecodeMaxLen = MSG_MMS_DECODE_BUFFER_MAX;
	gMmsDecodeCurOffset = 0;
	gMmsDecodeBufLen = 0;
}

void MmsUnregisterDecodeBuffer(void)
{
	gpMmsDecodeBuf1 = NULL;
	gpMmsDecodeBuf2 = NULL;
	gpCurMmsDecodeBuff = NULL;
	gCurMmsDecodeBuffPos = 0;
	gMmsDecodeMaxLen = 0;
	gMmsDecodeCurOffset = 0;
	gMmsDecodeBufLen = 0;
}


static int __MmsGetDecodeOffset(void)
{
	return (gMmsDecodeCurOffset - gMmsDecodeBufLen + gCurMmsDecodeBuffPos);
}

static bool __MmsBinaryDecodeCheckAndDecreaseLength(int *pLength, int valueLength)
{
	if (*pLength <= valueLength) {
		gCurMmsDecodeBuffPos -= valueLength;
		gCurMmsDecodeBuffPos += *pLength;
		return false;
	}

	*pLength -= valueLength;

	return true;
}


/* ==========================================================

	   B  I  N  A  R  Y         D  E  C  O  D  I  N  G

   ==========================================================*/

/*
 * Binary Encoded Message Format
 *
 *		< Single Part Body Message >
 * -----------------------------------
 * |		Header Fields			 |
 * -----------------------------------
 * | Content Type:start=xxx;type=xxx |	->(ex) Text/Plain, Text/Html, ....
 * -----------------------------------
 * |		Single Part Body		 |
 * -----------------------------------
 *
 *		< Multi Part Body Message >
 * -----------------------------------
 * |		Header Fields			 |
 * -----------------------------------
 * | Content Type:start=xxx;type=xxx |	-> (ex) Application/vnd.wap.multipart.mixed(related), multipart/mixed(related)
 * -----------------------------------
 * |	# of Entries (body parts)	 |
 * -----------------------------------				< Each Entry >
 * |			Entry 1				 |	->	-----------------------------
 * -----------------------------------		|		header Length		|
 * |			Entry 2				 |		-----------------------------
 * -----------------------------------		|		Data Length			|
 * |			......				 |		-----------------------------  -
 * -----------------------------------		|		Content-Type		|  |
 * |			Entry n				 |		-----------------------------  | Header Length
 * -----------------------------------		|			Header			|  |
 *											-----------------------------  -
 *											|			Data			|  | Data Length
 *											-----------------------------  -
 */
bool MmsBinaryDecodeMsgHeader(FILE *pFile, int totalLength)
{
	MSG_BEGIN();
	UINT16 fieldCode = 0xffff;
	UINT16 fieldValue = 0xffff;
	UINT8 oneByte = 0xff;

	MsgHeaderAddress *pAddr = NULL;
	MsgHeaderAddress *pLastTo = NULL;
	MsgHeaderAddress *pLastCc = NULL;
	MsgHeaderAddress *pLastBcc = NULL;

	UINT32 valueLength = 0;
	UINT32 tmpInteger = 0;
	int	tmpIntLen = 0;

	int offset = 0;

	char szGarbageBuff[MSG_STDSTR_LONG]	= {0, };
	char *pLimitData = NULL;
	int nRead = 0;

	MSG_DEBUG("pFile ptr : [%p], total len = [%d]", pFile, totalLength);

	__MmsCleanDecodeBuff();

	if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos,
								   &gMmsDecodeCurOffset, gpMmsDecodeBuf1, gpMmsDecodeBuf2,
								   gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_FATAL("fail to load to buffer");
		goto __CATCH;
	}

	while (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength)) {
		fieldCode = oneByte & 0x7f;

		switch (MmsGetBinaryType(MmsCodeFieldCode, fieldCode)) {
		case MMS_CODE_RESPONSESTATUS: {
			MmsResponseStatus resposeStatus = MMS_RESPSTATUS_ERROR;

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("responseStatus GetOneByte fail");
				goto __CATCH;
			}

			fieldValue = oneByte;

			/* range 197 to 223 as it does to the value 192 (Error-transient-failure). */
			/* range 236 to 255 as it does to the value 224 (Error-permanent-failure). */
			if (fieldValue >= 197 && fieldValue <= 223) {
				fieldValue = 192;
			} else if (fieldValue >= 236 && fieldValue <= 255) {
				fieldValue = 224;
			}

			resposeStatus = (MmsResponseStatus)MmsGetBinaryType(MmsCodeResponseStatus, (UINT16)(fieldValue & 0x7F));

			mmsHeader.responseStatus = (MmsResponseStatus)resposeStatus;

			MSG_SEC_INFO("X-Mms-Response-Status = [0x%02x][0x%02x][%s]", oneByte, fieldValue, MmsDebugGetResponseStatus(mmsHeader.responseStatus));
			break;
		}
		case MMS_CODE_RETRIEVESTATUS: {
			MmsRetrieveStatus RetrieveStatus = MMS_RETRSTATUS_ERROR;

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("retrieveStatus GetOneByte fail");
				goto __CATCH;
			}

			fieldValue = oneByte;

			/* 195 to 223 as it does to the value 192 (Error-transient-failure). */
			/* 228 to 255 as it does to the value 224 (Error-permanent-failure). */
			if (fieldValue >= 195 && fieldValue <= 223) {
				fieldValue = 192; /* 192; Error-transient-failure */
			} else if (fieldValue >= 228 && fieldValue <= 255) {
				fieldValue = 224; /* 224; Error-permanent-failure */
			}

			RetrieveStatus = (MmsRetrieveStatus)MmsGetBinaryType(MmsCodeRetrieveStatus, (UINT16)(fieldValue & 0x7F));

			mmsHeader.retrieveStatus = (MmsRetrieveStatus)RetrieveStatus;

			MSG_SEC_INFO("X-Mms-Retrieve-Status = [0x%02x][0x%02x][%s]", oneByte, fieldValue, MmsDebugGetRetrieveStatus(mmsHeader.retrieveStatus));

			break;
		}
		case MMS_CODE_RESPONSETEXT:

			if (__MmsBinaryDecodeEncodedString(pFile, mmsHeader.szResponseText, MMS_LOCALE_RESP_TEXT_LEN + 1, totalLength) == false) {
				MSG_DEBUG("invalid MMS_CODE_RESPONSETEXT");
				goto __CATCH;
			}

			MSG_SEC_INFO("X-Mms-Response-Text = [%s]", mmsHeader.szResponseText);
			break;

		case MMS_CODE_RETRIEVETEXT:

			if (__MmsBinaryDecodeEncodedString(pFile, mmsHeader.szRetrieveText, MMS_LOCALE_RESP_TEXT_LEN + 1, totalLength) == false) {
				MSG_DEBUG("invalid MMS_CODE_RETRIEVETEXT");
				goto __CATCH;
			}

			MSG_SEC_INFO("X-Mms-Retrieve-Text = [%s]", mmsHeader.szRetrieveText);
			break;

		case MMS_CODE_MSGID:

			if (__MmsBinaryDecodeText(pFile, mmsHeader.szMsgID, MMS_MSG_ID_LEN + 1, totalLength) < 0) {
				MSG_DEBUG("MMS_CODE_MSGID is invalid");
				goto __CATCH;
			}

			MSG_SEC_INFO("Message-ID =[%s]", mmsHeader.szMsgID);

			if (strlen(mmsHeader.szMsgID) > 2)
				__MsgMIMERemoveQuote (mmsHeader.szMsgID);

			break;

		case MMS_CODE_SUBJECT:

			if (__MmsBinaryDecodeEncodedString(pFile, mmsHeader.szSubject, MSG_LOCALE_SUBJ_LEN + 1, totalLength) == false) {
				MSG_DEBUG("invalid MMS_CODE_SUBJECT");
				goto __CATCH;
			}

			pLimitData = (char *)calloc(1, MSG_LOCALE_SUBJ_LEN + 1);

			if (pLimitData == NULL) {
				MSG_DEBUG("pLimitData calloc fail");
				goto __CATCH;
			}

			nRead = __MsgCutUTFString((unsigned char*)pLimitData, MSG_LOCALE_SUBJ_LEN + 1, (unsigned char*)mmsHeader.szSubject, MSG_SUBJ_LEN);
			MSG_DEBUG("Subject edit..");

			if (nRead > MSG_LOCALE_SUBJ_LEN) {
				memset(mmsHeader.szSubject, 0 , sizeof(mmsHeader.szSubject));
				strncpy(mmsHeader.szSubject, pLimitData, MSG_SUBJ_LEN);
			} else {
				memset(mmsHeader.szSubject, 0 , sizeof(mmsHeader.szSubject));
				strncpy(mmsHeader.szSubject, pLimitData, MSG_LOCALE_SUBJ_LEN);
			}

			if (pLimitData) {
				free(pLimitData);
				pLimitData = NULL;
			}

			MSG_SEC_INFO("Subject = [%s]", mmsHeader.szSubject);
			break;

		case MMS_CODE_FROM:

			/* Value-length (Address-present-token Encoded-string-value | Insert-address-token) */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("MMS_CODE_FROM is invalid");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MMS_CODE_FROM GetOneByte fail");
				goto __CATCH;
			}

			/* DRM_TEMPLATE - start */

			valueLength--;

			if (oneByte == (MmsGetBinaryValue(MmsCodeAddressType, MMS_ADDRESS_PRESENT_TOKEN)|0x80)) {
				if (valueLength > 0) {
					mmsHeader.pFrom = __MmsDecodeEncodedAddress(pFile, totalLength);
					if (mmsHeader.pFrom == NULL) {
						MSG_DEBUG("MMS_CODE_FROM __MmsDecodeEncodedAddress fail");
						goto __CATCH;
					}
				} else {
					mmsHeader.pFrom = (MsgHeaderAddress *)calloc(1, sizeof(MsgHeaderAddress));
					if (mmsHeader.pFrom == NULL)
						goto __CATCH;

					mmsHeader.pFrom->szAddr = (char *)calloc(1, 1);
					if (mmsHeader.pFrom->szAddr == NULL) {
						free(mmsHeader.pFrom);
						mmsHeader.pFrom = NULL;
						goto __CATCH;
					}

					mmsHeader.pFrom->szAddr[0] = '\0';
					mmsHeader.pFrom->pNext = NULL;
				}

				MSG_SEC_INFO("From = [%s]", mmsHeader.pFrom->szAddr);
				/* DRM_TEMPLATE - end */
			} else if (oneByte == (MmsGetBinaryValue(MmsCodeAddressType, MMS_INSERT_ADDRESS_TOKEN)|0x80)) {
				/* Present Token only */
				MSG_SEC_INFO("From = [insert token]");
			} else {
				/* from data broken */
				MSG_WARN("from addr broken");
				gCurMmsDecodeBuffPos--;
				goto __CATCH;
			}
			break;

		case MMS_CODE_TO:

			pAddr = __MmsDecodeEncodedAddress(pFile, totalLength);
			if (pAddr == NULL) {
				MSG_DEBUG("MMS_CODE_TO __MmsDecodeEncodedAddress fail");
				goto __CATCH;
			}

			if (mmsHeader.pTo == NULL) {
				/* the first TO */
				pLastTo = mmsHeader.pTo = pAddr;
			} else {
				if (pLastTo)
					pLastTo->pNext = pAddr;
				pLastTo = pAddr;
			}

			MSG_SEC_INFO("To = [%s]", pAddr->szAddr);
			break;

		case MMS_CODE_BCC:

			pAddr = __MmsDecodeEncodedAddress(pFile, totalLength);
			if (pAddr == NULL) {
				MSG_DEBUG("MMS_CODE_BCC __MmsDecodeEncodedAddress fail");
				goto __CATCH;
			}

			if (mmsHeader.pBcc == NULL) {
				/* the first Bcc */
				pLastBcc = mmsHeader.pBcc = pAddr;
			} else {
				if (pLastBcc)
					pLastBcc->pNext = pAddr;
				pLastBcc = pAddr;
			}

			MSG_SEC_INFO("Bcc = [%s]", pAddr->szAddr);
			break;

		case MMS_CODE_CC:

			pAddr = __MmsDecodeEncodedAddress(pFile, totalLength);
			if (pAddr == NULL) {
				MSG_DEBUG("MMS_CODE_CC __MmsDecodeEncodedAddress fail");
				goto __CATCH;
			}

			if (mmsHeader.pCc == NULL) {
				/* the first Cc */
				pLastCc = mmsHeader.pCc = pAddr;
			} else {
				if (pLastCc)
					pLastCc->pNext = pAddr;
				pLastCc	= pAddr;
			}
			MSG_SEC_INFO("Cc = [%s]", pAddr->szAddr);
			break;

		case MMS_CODE_CONTENTLOCATION:

			if (__MmsBinaryDecodeText(pFile, mmsHeader.szContentLocation, MMS_LOCATION_LEN + 1, totalLength) < 0) {
				MSG_DEBUG("MMS_CODE_CONTENTLOCATION is invalid");
				goto __CATCH;
			}
			MSG_SEC_DEBUG("X-Mms-Content-Location = [%s]", mmsHeader.szContentLocation);
			break;

		case MMS_CODE_DATE:

			if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.date, totalLength) == false) {
				MSG_DEBUG("MMS_CODE_DATE is invalid");
				goto __CATCH;
			}

			MSG_SEC_INFO("Date = [%u][%d]", mmsHeader.date, (const time_t *)&mmsHeader.date);
			break;

		case MMS_CODE_DELIVERYREPORT:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("deliveryReport GetOneByte fail");
				goto __CATCH;
			}

			fieldValue = MmsGetBinaryType(MmsCodeDeliveryReport, (UINT16)(oneByte & 0x7F));

			if (fieldValue == 0xFFFF) {
				MSG_DEBUG("deliveryReport error");
				goto __CATCH;
			}

			mmsHeader.deliveryReport = (MmsReport)fieldValue;

			MSG_SEC_INFO("X-Mms-Delivery-Report =[0x%02x][%s]", oneByte, MmsDebugGetMmsReport(mmsHeader.deliveryReport));
			break;

		case MMS_CODE_DELIVERYTIME:

			/* value_length (absolute-token Long-integer | Relative-token Long-integer) */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("invalid MMS_CODE_DELIVERYTIME");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("delivery time GetOneByte fail");
				goto __CATCH;
			}

			/* DRM_TEMPLATE - start */
			valueLength--;

			if (oneByte == (MmsGetBinaryValue(MmsCodeTimeType, MMS_TIMETYPE_ABSOLUTE)|0x80)) {
				mmsHeader.deliveryTime.type = MMS_TIMETYPE_ABSOLUTE;

				if (valueLength > 0) {
					if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.deliveryTime.time, totalLength) == false)	{
						MSG_DEBUG("invalid MMS_CODE_DELIVERYTIME");
						goto __CATCH;
					}
				}
			/* DRM_TEMPLATE - end */
			} else {
				mmsHeader.deliveryTime.type = MMS_TIMETYPE_RELATIVE;

				if (__MmsBinaryDecodeInteger(pFile, (UINT32*)&mmsHeader.deliveryTime.time, &tmpIntLen, totalLength) == false) {
					MSG_DEBUG("__MmsBinaryDecodeInteger fail...");
					goto __CATCH;
				}
			}
			MSG_SEC_INFO("X-Mms-Delivery-Time : type = [%d], time= [%u]", mmsHeader.deliveryTime.type, mmsHeader.deliveryTime.time);
			break;

		case MMS_CODE_EXPIRYTIME:

			/* value_length(absolute-token Long-integer | Relative-token Long-integer) */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("invalid MMS_CODE_EXPIRYTIME");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("expiry time GetOneByte fail");
				goto __CATCH;
			}

			/* DRM_TEMPLATE - start */
			valueLength--;

			if (oneByte == (MmsGetBinaryValue(MmsCodeTimeType, MMS_TIMETYPE_ABSOLUTE)|0x80)) {
				mmsHeader.expiryTime.type = MMS_TIMETYPE_ABSOLUTE;

				if (valueLength > 0) {
					if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.expiryTime.time, totalLength) == false) {
						MSG_DEBUG("MMS_CODE_EXPIRYTIME is invalid");
						goto __CATCH;
					}
				}
			/* DRM_TEMPLATE - end */
			} else {
				mmsHeader.expiryTime.type = MMS_TIMETYPE_RELATIVE;

				if (__MmsBinaryDecodeInteger(pFile, (UINT32*)&mmsHeader.expiryTime.time, &tmpIntLen, totalLength) == false) {
					MSG_INFO("__MmsBinaryDecodeInteger fail...");
					goto __CATCH;
				}
			}

			MSG_DEBUG("X-Mms-Expiry : type = [%d], time = [%u]", mmsHeader.expiryTime.type, mmsHeader.expiryTime.time);
			break;

		case MMS_CODE_MSGCLASS:

			/* Class-value = Class-identifier | Token Text */

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgClass GetOneByte fail");
				goto __CATCH;
			}

			if (oneByte > 0x7f) {
				/* Class-identifier */
				mmsHeader.msgClass = (MmsMsgClass)MmsGetBinaryType(MmsCodeMsgClass, (UINT16)(oneByte & 0x7F));
			} else {
				if (__MmsBinaryDecodeText(pFile, szGarbageBuff, MSG_STDSTR_LONG, totalLength) < 0) {
					MSG_DEBUG("1. __MmsBinaryDecodeText fail. (class)");
					goto __CATCH;
				}
			}

			MSG_SEC_INFO("X-Mms-Message-Class =[%s]", MmsDebugGetMsgClass(mmsHeader.msgClass));
			break;

		case MMS_CODE_MSGSIZE:

			if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.msgSize, totalLength) == false) {
				MSG_DEBUG("MMS_CODE_MSGSIZE is invalid");
				goto __CATCH;
			}

			MSG_SEC_INFO("X-Mms-Message-Size = [%d]", mmsHeader.msgSize);
			break;

		case MMS_CODE_MSGSTATUS:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail");
				goto __CATCH;
			}

			mmsHeader.msgStatus =  (msg_delivery_report_status_t)MmsGetBinaryType(MmsCodeMsgStatus, (UINT16)(oneByte & 0x7F));
			MSG_SEC_INFO("X-Mms-Status = [%s]", MmsDebugGetMsgStatus(mmsHeader.msgStatus));
			break;

		case MMS_CODE_MSGTYPE:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail");
				goto __CATCH;
			}

			mmsHeader.type = (MmsMsgType)MmsGetBinaryType(MmsCodeMsgType, (UINT16)(oneByte & 0x7F));
			MSG_SEC_INFO("X-Mms-Message-Type = [%s]", MmsDebugGetMsgType(mmsHeader.type));
			break;

		case MMS_CODE_PRIORITY:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail");
				goto __CATCH;
			}
			mmsHeader.priority = (MmsPriority)MmsGetBinaryType(MmsCodePriority, (UINT16)(oneByte & 0x7F));
			MSG_SEC_INFO("X-Mms-Priority = [%d]", mmsHeader.priority);
			break;

		case MMS_CODE_READREPLY:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail");
				goto __CATCH;
			}
			mmsHeader.readReply = (MmsReport)MmsGetBinaryType(MmsCodeReadReply, (UINT16)(oneByte & 0x7F));
			MSG_SEC_INFO("X-Mms-Read-Report = [0x%02x][%s]", oneByte, MmsDebugGetMmsReport(mmsHeader.readReply));
			break;

		case MMS_CODE_REPORTALLOWED:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail");
				goto __CATCH;
			}
			mmsHeader.reportAllowed =  (MmsReportAllowed)MmsGetBinaryType(MmsCodeReportAllowed, (UINT16)(oneByte & 0x7F));
			MSG_SEC_INFO("X-Mms-Report-Allowed = [%d]", MmsDebugGetMmsReportAllowed(mmsHeader.reportAllowed));
			break;

		case MMS_CODE_SENDERVISIBILLITY:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail");
				goto __CATCH;
			}
			mmsHeader.hideAddress= (MmsSenderVisible)!(MmsGetBinaryType(MmsCodeSenderVisibility, (UINT16)(oneByte &0x7F)));
			MSG_SEC_INFO("X-Mms-Sender-Visibility = [%d]", mmsHeader.hideAddress);
			break;

		case MMS_CODE_TRID:

			if (__MmsBinaryDecodeText(pFile, mmsHeader.szTrID, MMS_TR_ID_LEN + 1, totalLength) < 0) {
				MSG_DEBUG("Transaction ID Too Long");
				goto __CATCH;
			}
			MSG_SEC_INFO("X-Mms-Transaction-Id = [%s]", mmsHeader.szTrID);
			break;

		case MMS_CODE_VERSION:
			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail");
				goto __CATCH;
			}
			mmsHeader.version = oneByte;

			MSG_SEC_INFO("X-Mms-MMS-Version = [0x%02x]", mmsHeader.version);
			break;

		case MMS_CODE_CONTENTTYPE:

			/*
			 * Content-type is the last header field of SendRequest and RetrieveConf.
			 * It's required to decrease pointer by one and return,
			 * to parse this field in MmsBinaryDecodeContentType().
			 */
			goto __RETURN;


		/* ----------- Add by MMSENC v1.1 ----------- */

		case MMS_CODE_READSTATUS:

			/* Read-status-value = Read | Deleted without being read */

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail");
				goto __CATCH;
			}

			mmsHeader.readStatus =  (msg_read_report_status_t)MmsGetBinaryType(MmsCodeReadStatus, (UINT16)(oneByte & 0x7F));
			MSG_SEC_INFO("X-Mms-Read-Status = [%s]", MmsDebugGetMmsReadStatus(mmsHeader.readStatus));
			break;

		case MMS_CODE_REPLYCHARGING:

			/* Reply-charging-value = Requested | Requested text only | Accepted | Accepted text only */

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail");
				goto __CATCH;
			}

			mmsHeader.replyCharge.chargeType =  (MmsReplyChargeType)MmsGetBinaryType(MmsCodeReplyCharging, (UINT16)(oneByte & 0x7F));
			MSG_SEC_INFO("X-Mms-Reply-Charging = [%d]", mmsHeader.replyCharge.chargeType);
			break;

		case MMS_CODE_REPLYCHARGINGDEADLINE:

			/* Reply-charging-deadline-value = Value-length (Absolute-token Date-value | Relative-token Delta-seconds-value) */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("invalid MMS_CODE_REPLYCHARGINGDEADLINE");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail");
				goto __CATCH;
			}

			if (oneByte == (MmsGetBinaryValue(MmsCodeTimeType, MMS_TIMETYPE_ABSOLUTE) | 0x80)) {
				mmsHeader.replyCharge.deadLine.type = MMS_TIMETYPE_ABSOLUTE;
			} else {
				mmsHeader.replyCharge.deadLine.type = MMS_TIMETYPE_RELATIVE;
			}

			/* DRM_TEMPLATE - start */
			valueLength--;

			if (valueLength > 0) {
				if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.replyCharge.deadLine.time, totalLength) == false) {
					MSG_DEBUG("MMS_CODE_REPLYCHARGINGDEADLINE is invalid");
					goto __CATCH;
				}
			}

			MSG_SEC_INFO("X-Mms-Reply-Charging-Deadline : type = [%d], time = [%u]", mmsHeader.replyCharge.deadLine.type, mmsHeader.replyCharge.deadLine.time);
			/* DRM_TEMPLATE - end */
			break;

		case MMS_CODE_REPLYCHARGINGID:

			/* Reply-charging-ID-value = Text-string */
			if (__MmsBinaryDecodeText(pFile, mmsHeader.replyCharge.szChargeID, MMS_MSG_ID_LEN + 1, totalLength) < 0) {
				MSG_DEBUG("1. __MmsBinaryDecodeText fail. (szReplyChargingID)");
				goto __CATCH;
			}
			SECURE_SLOGD("X-Mms-Reply-Charging-ID = [%s]", mmsHeader.replyCharge.szChargeID);
			break;

		case MMS_CODE_REPLYCHARGINGSIZE:

			/* Reply-charging-size-value = Long-integer */
			if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.replyCharge.chargeSize, totalLength) == false) {
				MSG_DEBUG("MMS_CODE_REPLYCHARGINGSIZE is invalid");
				goto __CATCH;
			}
			MSG_SEC_INFO("X-Mms-Reply-Charging-Size = [%d]", mmsHeader.replyCharge.chargeSize);
			break;

		case MMS_CODE_PREVIOUSLYSENTBY:

			/*
			 * Previously-sent-by-value = Value-length Forwarded-count-value Encoded-string-value
			 * Forwarded-count-value = Integer-value
			 * MMS_CODE_PREVIOUSLYSENTBY shall be a pair with MMS_CODE_PREVIOUSLYSENTDATE
			 */

			/*
			 * fixme: There is no proper field to store this information.
			 * Just increase pointer now.
			 */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("1. invalid MMS_CODE_PREVIOUSLYSENTBY");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeInteger(pFile, &tmpInteger, &tmpIntLen, totalLength) == false) {
				MSG_DEBUG("2. invalid MMS_CODE_PREVIOUSLYSENTBY");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeEncodedString(pFile, szGarbageBuff, MSG_STDSTR_LONG, totalLength) == false) {
				MSG_DEBUG("invalid MMS_CODE_RETRIEVETEXT");
				goto __CATCH;
			}
			break;

		case MMS_CODE_PREVIOUSLYSENTDATE:

			/*
			 * Previously-sent-date-value = Value-length Forwarded-count-value Date-value
			 * Forwarded-count-value = Integer-value
			 * MMS_CODE_PREVIOUSLYSENTDATE shall be a pair with MMS_CODE_PREVIOUSLYSENTBY
			 */

			/*
			 * fixme: There is no proper field to store this information.
			 * Just increase pointer now.
			 */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("1. invalid MMS_CODE_PREVIOUSLYSENTDATE");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeInteger(pFile, &tmpInteger, &tmpIntLen, totalLength) == false) {
				MSG_DEBUG("2. invalid MS_CODE_PREVIOUSLYSENTDATE");
				goto __CATCH;
			}

			if (__MmsDecodeLongInteger(pFile, (UINT32*)&tmpInteger, totalLength) == false) {
				MSG_DEBUG("3. invalid MMS_CODE_PREVIOUSLYSENTDATE");
				goto __CATCH;
			}
			break;

		default: {

			/*
			 * Application-header		  = Token-text Application-specific-value
			 * Token-text				  = Token End-of-string
			 * Application-specific-value = Text -string
			 *
			 * OR unknown header field - Just ignore these fields.
			 *
			 * Read one byte and check the value >= 0x80
			 * (check these value can be field code)
			 */

			int remainLength = 0;

			oneByte = 0x00;

			offset = __MmsGetDecodeOffset();
			if (offset >= totalLength)
				goto __RETURN;

			remainLength = totalLength - offset;

			while ((oneByte < 0x80) && (remainLength > 0)) {
				if (__MmsBinaryDecodeCheckAndDecreaseLength(&remainLength, 1) == false) {
					MSG_DEBUG("__MmsBinaryDecodeCheckAndDecreaseLength fail");
					goto __CATCH;
				}

				if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
					MSG_DEBUG("responseStatus GetOneByte fail");
					goto __CATCH;
				}
			}

			gCurMmsDecodeBuffPos--;
			break;
		}
		}	/* switch */

		offset = __MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

	}	/* while */


__RETURN:

	if (mmsHeader.pTo == NULL && pLastTo) {
		free(pLastTo);
	}

	if (mmsHeader.pCc == NULL && pLastCc) {
		free(pLastCc);
	}

	if (mmsHeader.pBcc == NULL && pLastBcc) {
		free(pLastBcc);
	}

	MSG_INFO("## Decode Header Success ##");
	MSG_END();
	return true;


__CATCH:

	if (mmsHeader.pTo == NULL && pLastTo) {
		free(pLastTo);
	}

	if (mmsHeader.pCc == NULL && pLastCc) {
		free(pLastCc);
	}

	if (mmsHeader.pBcc == NULL && pLastBcc) {
		free(pLastBcc);
	}

	MSG_FATAL("## Decode Header Fail ##");
	MSG_END();
	return false;
}

bool MmsBinaryDecodeMsgBody(FILE *pFile, char *szFilePath, int totalLength)
{
	MSG_BEGIN();

	int length = 0;
	int offset = 0;

	if (szFilePath != NULL)
		snprintf(mmsHeader.msgType.szOrgFilePath, sizeof(mmsHeader.msgType.szOrgFilePath), "%s", szFilePath);

	mmsHeader.msgType.offset = __MmsGetDecodeOffset() - 1;		/* + Content-Type code value */

	/* read data(2K) from msg file(/User/Msg/Inbox/5) to gpCurMmsDecodeBuff for decoding */
	if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos,
									&gMmsDecodeCurOffset, gpMmsDecodeBuf1, gpMmsDecodeBuf2,
									gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("fail to load to buffer");
		goto __CATCH;
	}

	/* msg's type [ex] related, mixed, single part (jpg, amr and etc) */
	length = __MmsBinaryDecodeContentType(pFile, &mmsHeader.msgType, totalLength);
	if (length == -1) {
		MSG_DEBUG("MMS_CODE_CONTENTTYPE is fail");
		goto __CATCH;
	}

	mmsHeader.msgType.size	 = length + 1; /* + Content-Type code value */
	mmsHeader.msgBody.offset = __MmsGetDecodeOffset();

	switch (mmsHeader.msgType.type) {
	case MIME_APPLICATION_VND_WAP_MULTIPART_MIXED:
	case MIME_APPLICATION_VND_WAP_MULTIPART_RELATED:
	case MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC:
	case MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE:
	case MIME_MULTIPART_REPORT:
	case MIME_MULTIPART_MIXED:
	case MIME_MULTIPART_RELATED:
	case MIME_MULTIPART_ALTERNATIVE:
	case MIME_APPLICATION_VND_OMA_DRM_MESSAGE:
	case MIME_APPLICATION_VND_OMA_DRM_CONTENT:

		MSG_DEBUG("Decode Multipart");

		offset = __MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		if (__MmsBinaryDecodeMultipart(pFile, szFilePath, &mmsHeader.msgType, &mmsHeader.msgBody, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodeMultipart is fail.");
			goto __CATCH;
		}
		break;

	default:

		/* Single part message ---------------------------------------------- */
		MSG_DEBUG("Decode Singlepart");

		offset = __MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		if (__MmsBinaryDecodePartBody(pFile, totalLength - mmsHeader.msgBody.offset, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodePartBody is fail.(Single Part)");
			goto __CATCH;
		}

		mmsHeader.msgBody.size = totalLength - mmsHeader.msgBody.offset;
		mmsHeader.msgType.contentSize = totalLength - mmsHeader.msgBody.offset;

		break;
	}

__RETURN:
	MSG_END();
	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryDecodeParameter(FILE *pFile, MsgType *pMsgType, int valueLength, int totalLength)
{
	MSG_BEGIN();
	UINT8 oneByte = 0;
	int charSetLen = 0;
	char *szTypeString = NULL;
	char *szTypeValue = NULL;
	UINT8 paramCode = 0xff;
	UINT32 integer = 0;
	int intLen = 0;
	int length = 0;
	int textLength = 0;

	/*
	 * Parameter = Typed-parameter | Untyped-parameter
	 * WAP-230-WSP-20010118-p, Proposed Version 18 January 2001 (pp.107)
	 */

	while (valueLength > 0) {
		if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
			MSG_DEBUG("paramCode _MmsBinaryDecodeGetOneByte fail");
			goto __CATCH;
		}

		paramCode = oneByte;
		valueLength--;

		switch (paramCode) {
		case 0x81: /* charset */

			if (__MmsBinaryDecodeCharset(pFile, (UINT32*)&(pMsgType->param.charset), &charSetLen, totalLength) == false) {
				MSG_DEBUG("__MmsBinaryDecodeCharset fail.");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, charSetLen) == false)
				goto __RETURN;

			break;

		case 0x85: /* name = Text-string */
		case 0x97: /* name = Text-value  = No-value | Token-text | Quoted-string */
			memset(pMsgType->param.szName, 0, sizeof(pMsgType->param.szName));
			length = __MmsDecodeGetFilename(pFile,  pMsgType->param.szName,
											 MSG_FILENAME_LEN_MAX -5,		/* MSG_LOCALE_FILENAME_LEN_MAX + 1, :  change @ 110(Ui code have to change for this instead of DM) */
											 totalLength);
			if (length < 0) {
				MSG_DEBUG("__MmsDecodeGetFilename fail. (name parameter)");
				goto __CATCH;
			}

			if (__MsgCheckFileNameHasInvalidChar(pMsgType->param.szName)) {
				__MsgReplaceInvalidFileNameChar(pMsgType->param.szName, '_');
			}

			if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, length) == false)
				goto __RETURN;

			break;

		case 0x86: /* filename = Text-string */
		case 0x98: /* filename = Text-value  = No-value | Token-text | Quoted-string */
			memset(pMsgType->param.szFileName, 0, sizeof(pMsgType->param.szFileName));
			length = __MmsDecodeGetFilename(pFile, pMsgType->param.szFileName, MSG_FILENAME_LEN_MAX -5, totalLength);
			if (length < 0) {
				MSG_DEBUG("__MmsDecodeGetFilename fail. (filename parameter)");
				goto __CATCH;
			}

			if (__MsgCheckFileNameHasInvalidChar(pMsgType->param.szFileName)) {
				__MsgReplaceInvalidFileNameChar(pMsgType->param.szFileName, '_');
			}

			if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, length) == false)
				goto __RETURN;

			break;

		case 0x89: /* type = Constrained-encoding = Extension-Media | Short-integer */

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("type _MmsBinaryDecodeGetOneByte fail");
				goto __CATCH;
			}

			if (oneByte > 0x7f) {
				pMsgType->param.type = MimeGetMimeIntFromBi((UINT16)(oneByte & 0x7f));
				/* MmsGetBinaryType(MmsCodeContentType,(UINT16)(oneByte & 0x7f)); */
				if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, 1) == false)
					goto __RETURN;
			} else {
				gCurMmsDecodeBuffPos--;

				textLength = 0;
				szTypeString = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
				pMsgType->param.type = MimeGetMimeIntFromMimeString(szTypeString);
				if (szTypeString) {
					free(szTypeString);
					szTypeString = NULL;
				}

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, textLength) == false)
					goto __RETURN;
			}

			break;

		case 0x8A: /* start encoding version 1.2 */
		case 0x99: /* start encoding version 1.4 */

			textLength	 = 0;
			szTypeString = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
			if (szTypeString) {
				memset(pMsgType->param.szStart, 0, MMS_CONTENT_ID_LEN + 1);
				strncpy(pMsgType->param.szStart, szTypeString, MMS_CONTENT_ID_LEN);
				free(szTypeString);
				szTypeString = NULL;

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, textLength) == false)
					goto __RETURN;
			}

			break;

		case 0x8B: /* startInfo encoding version 1.2 */
		case 0x9A: /* startInfo encoding version 1.4 */

			textLength	 = 0;
			szTypeString = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);

			if (szTypeString) {
				memset(pMsgType->param.szStartInfo, 0, MMS_CONTENT_ID_LEN + 1);
				strncpy(pMsgType->param.szStartInfo, szTypeString, MMS_CONTENT_ID_LEN);

				free(szTypeString);
				szTypeString = NULL;

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, textLength) == false)
					goto __RETURN;
			}

			break;

		default:

			if (paramCode > 0x7F) {
				MSG_DEBUG("Unsupported parameter");

				/* In case of the last byte of Parameter field, it should be returned without decreasing the gCurMmsDecodeBuffPos value. */
				valueLength++;
				if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, 1) == false)
					goto __RETURN;
			} else {
				/*
				 * Untyped Parameter = Token-text Untyped-value
				 * Token-text		 = Token End-of-string
				 * Untyped-value	 = Integer-value | Text-value
				 * Text-value		 = No-value | Token-text | Quoted-string
				 *
				 * Just increase pointer!!!
				 */


				/* Token-text */

				gCurMmsDecodeBuffPos--;
				valueLength++;

				textLength	  = 0;
				szTypeString  = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
				if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, textLength) == false)
					goto __RETURN;


				/* Text-value */

				if (__MmsBinaryDecodeInteger(pFile, &integer, &intLen, totalLength) == true) {
					MSG_DEBUG("Unsupported parameter(%d)\n", integer);
					if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, intLen) == false)
						goto __RETURN;
				} else {
					textLength	  = 0;
					szTypeValue  = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);

					if (szTypeValue) {
						/* checkMe:  forwardLock needs boudary string */
						if (strcasecmp(szTypeString, "boundary") == 0) {
							memset(pMsgType->param.szBoundary, 0, MSG_BOUNDARY_LEN + 1);
							strncpy(pMsgType->param.szBoundary, szTypeValue, MSG_BOUNDARY_LEN);
#ifdef FEATURE_JAVA_MMS
						} else if (strcasecmp(szTypeString, "Application-ID") == 0) {
							pMsgType->param.szApplicationID = (char*) calloc(1, textLength + 1);
							if (pMsgType->param.szApplicationID) {
								memset(pMsgType->param.szApplicationID,  0,  textLength + 1);
								strncpy(pMsgType->param.szApplicationID, szTypeValue, textLength);
								MSG_SEC_DEBUG("Application-ID:%s",pMsgType->param.szApplicationID);
							}
						} else if (strcasecmp(szTypeString,"Reply-To-Application-ID") == 0) {
							pMsgType->param.szReplyToApplicationID = (char*) calloc(1, textLength + 1);
							if (pMsgType->param.szReplyToApplicationID) {
								memset(pMsgType->param.szReplyToApplicationID, 0, textLength + 1);
								strncpy(pMsgType->param.szReplyToApplicationID, szTypeValue, textLength);
								MSG_SEC_DEBUG("ReplyToApplication-ID:%s",pMsgType->param.szReplyToApplicationID);
							}
#endif
						}

						MSG_DEBUG("Unsupported parameter(%s)\n", szTypeValue);
						free(szTypeValue);
						szTypeValue = NULL;

						if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, textLength) == false)
							goto __RETURN;
					}
				}

				if (szTypeString) {
					MSG_DEBUG("Unsupported parameter(%s)\n", szTypeString);
					free(szTypeString);
					szTypeString = NULL;
				}
			}

			break;
		}
	}	/*end of while loop*/


__RETURN:

	if (szTypeString) {
		free(szTypeString);
		szTypeString = NULL;
	}

	MSG_END();
	return true;

__CATCH:
	MSG_END();
	return false;
}

/**
 * Decode Encoded Content type
 *
 * @param 	pEncodedData	[in] ContentType encoded data
 * @param	pMsgType		[out] Decoded MsgType
 * @return 	Decoded address list
 */
static int __MmsBinaryDecodeContentType(FILE *pFile, MsgType *pMsgType, int totalLength)
{
	MSG_BEGIN();
	UINT8 oneByte = 0;
	char *szTypeString = NULL;
	int valueLength = 0;
	int length = 0;
	int textLength = 0;


	/*
	 * Content-type-value		  : [WAPWSP 8.4.2.24]
	 * Preassigned content-types  : [WAPWSP Appendix A, Table 40]
	 * The use of start-parameter : [RFC2387] and SHOULD be encoded according to [WAPWSP].
	 *
	 * Content-type-value	= Constrained-media | Content-general-form
	 * Content-general-form = Value-length Media-type
	 * Media-type			= (Well-known-media | Extension-Media) *(Parameter)
	 */

	length = __MmsDecodeValueLength(pFile, (UINT32*)&valueLength, totalLength);
	if (length <= 0) {
		/*
		 * Constrained-media or Single part message
		 * Constrained-media = Constrained-encoding = Extension-Media | Short-integer
		 * Extension-media   = *TEXT End-of-string
		 * Short-integer     = OCTET(1xxx xxxx)
		 */

		if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
			MSG_DEBUG("Constrained-media _MmsBinaryDecodeGetOneByte fail");
			goto __CATCH;
		}

		if (oneByte > 0x7F) {
			/* Short-integer */
			pMsgType->type = MimeGetMimeIntFromBi((UINT16)(oneByte & 0x7f));

			MSG_SEC_DEBUG("Constrained-media : Short-integer : Content Type = [0x%04x], MimeType = [0x%04x]", oneByte, pMsgType->type);

			length = 1;
		} else {
			char *pszTemp = NULL;

			/* Extension-Media */
			gCurMmsDecodeBuffPos--;

			textLength = 0;
			szTypeString = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);

			if (szTypeString && (strchr(szTypeString, ';')) != NULL) {

				MSG_SEC_DEBUG("Constrained-media : Extension-Media : Content Type with delimiter = [%s]", szTypeString);

				pszTemp = __MsgGetStringUntilDelimiter(szTypeString, ';');
				if (pszTemp) {
					free(szTypeString);
					szTypeString = pszTemp;
				}
			}

			pMsgType->type = MimeGetMimeIntFromMimeString(szTypeString);

			MSG_SEC_DEBUG("Constrained-media : Extension-Media : Content Type = [%s], MimeType = [0x%04x]", szTypeString, pMsgType->type);

			length = textLength;

			if (szTypeString) {
				free(szTypeString);
				szTypeString = NULL;
			}
		}
	} else {
		/*
		 * Content-general-form = Value-length Media-type
		 * Media-type			= (Well-known-media | Extension-Media)*(Parameter)
		 */

		length += valueLength;

		if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
			MSG_DEBUG("Well-known-media _MmsBinaryDecodeGetOneByte fail");
			goto __CATCH;
		}

		if (oneByte > 0x7F) {
			/* Well-known-media */
			pMsgType->type = MimeGetMimeIntFromBi((UINT16)(oneByte & 0x7f));
			MSG_SEC_DEBUG("Content-general-form : Well-known-media : Content Type = [0x%04x], MimeType = [0x%04x]", oneByte, pMsgType->type);
			valueLength--;
		} else {
			/* Extension-Media */
			gCurMmsDecodeBuffPos--;

			textLength = 0;
			szTypeString = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);

			pMsgType->type = MimeGetMimeIntFromMimeString(szTypeString);

			MSG_SEC_DEBUG("Content-general-form : Extension-Media : Content Type = [%s], MimeType = [0x%04x]", szTypeString, pMsgType->type);

			valueLength -= textLength;

			if (szTypeString) {
				free(szTypeString);
				szTypeString = NULL;
			}
		}

		MSG_SEC_DEBUG("Content-Type = [%s]", MmsDebugGetMimeType((MimeType)pMsgType->type));

		if (__MmsBinaryDecodeParameter(pFile, pMsgType, valueLength, totalLength) == false) {
			MSG_DEBUG("Content-Type parameter fail");
			goto __CATCH;
		}
	}

	MSG_END();
	return length;

__CATCH:
	MSG_END();
	return -1;
}

static bool __MmsBinaryDecodePartHeader(FILE *pFile, MsgType *pMsgType, int headerLen, int totalLength)
{
	UINT8 fieldCode	= 0xff;
	int length = 0;
	UINT32 valueLength = 0;
	char *pCode = NULL;
	char *pValue = NULL;
	char *pParam = NULL;
	char ch	= '\0';
	UINT8 oneByte = 0;
	int	textLength = 0;
	int tmpInt = 0;
	int tmpIntLen = 0;
	char *pLatinBuff = NULL;
	char *szSrc = NULL;
	char *szTemp = NULL;

	MSG_INFO("headerLen[%d] totalLength[%d]", headerLen, totalLength);

	if (pFile == NULL || pMsgType == NULL)
		return false;

	/*
	 * Message-header			  = Well-known-header | Application-header
	 * Well-known-header		  = Well-known-field-name Wap-value
	 * Application-header		  = Token-text Application-specific-value
	 * Well-known-field-name	  = Short-integer
	 * Application-specific-value = Text-string
	 */

	while (headerLen > 0) {
		if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
			MSG_DEBUG("field code GetOneByte fail");
			goto __CATCH;
		}

		if (0x80 <= oneByte && oneByte <= 0xC7) {
			/* Well-known-header = Well-known-field-name Wap-value (0x00 ~ 0x47) */

			if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, 1) == false)
				goto __RETURN;

			fieldCode = oneByte & 0x7f;

			switch (fieldCode) {
			case 0x0E:	/* Content-Location */
			case 0x04:	/* Content-Location */
			{
				pLatinBuff = (char *)calloc(1, MMS_CONTENT_ID_LEN + 1);
				if (pLatinBuff == NULL)
					goto __CATCH;

				length = __MmsBinaryDecodeText(pFile, pLatinBuff, MMS_CONTENT_ID_LEN + 1, totalLength);
				if (length == -1) {
					MSG_DEBUG("__MmsBinaryDecodeQuotedString fail.");
					goto __CATCH;
				}

				szSrc = __MsgConvertLatin2UTF8FileName(pLatinBuff);
				if (szSrc) {
					snprintf(pMsgType->szContentLocation, sizeof(pMsgType->szContentLocation), "%s", szSrc);
					MSG_SEC_DEBUG("Content Location : [%s]", pMsgType->szContentLocation);
					free(szSrc);
					szSrc = NULL;
				}

				free(pLatinBuff);
				pLatinBuff = NULL;

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, length) == false)
					goto __RETURN;
			}
				break;

			case 0x40:	/* Content-ID */
			{
				char szContentID[MMS_CONTENT_ID_LEN + 1] = {0, };

				pLatinBuff = (char *)calloc(1, MMS_CONTENT_ID_LEN + 1);
				if (pLatinBuff == NULL)
					goto __CATCH;

				length = __MmsBinaryDecodeQuotedString(pFile, pLatinBuff, MMS_CONTENT_ID_LEN + 1, totalLength);

				if (length == -1) {
					MSG_DEBUG("Content-ID __MmsBinaryDecodeQuotedString fail.");
					goto __CATCH;
				}

				szSrc = __MsgConvertLatin2UTF8FileName(pLatinBuff);
				if (szSrc) {
					snprintf(szContentID, sizeof(szContentID), "%s", szSrc);
					MSG_SEC_DEBUG("Content ID : [%s]", szContentID);
					free(szSrc);
					szSrc = NULL;
				}

				free(pLatinBuff);
				pLatinBuff = NULL;

				MmsRemoveLessGreaterChar(szContentID, pMsgType->szContentID, sizeof(pMsgType->szContentID)); /* remove "< >" */

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, length) == false)
					goto __RETURN;
			}
				break;

			case 0x2E:	/* Content-Disposition */
			case 0x45:	/* Content-Disposition */

				/*
				 * Content-disposition-value = Value-length Disposition *(Parameter)
				 * Disposition = Form-data | Attachment | Inline | Token-text
				 *				 Form-data = <Octet 128> : 0x80
				 *				 Attachment = <Octet 129> : 0x81
				 *				 Inline = <Octet 130> : 0x82
				 */

				length = __MmsDecodeValueLength2(pFile, &valueLength, totalLength);

				if (length > 0) {
					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, length) == false)
						goto __RETURN;

				}

				if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
					MSG_DEBUG("Disposition value GetOneByte fail");
					goto __CATCH;
				}

				if (length > 0)
					valueLength--;

				if (oneByte >= 0x80) {
					pMsgType->disposition = MmsGetBinaryType(MmsCodeMsgDisposition, (UINT16)(oneByte & 0x7F));

					if (pMsgType->disposition == -1) {
						MSG_DEBUG("Content-Disposition MmsGetBinaryType fail.");
						pMsgType->disposition = MSG_DISPOSITION_ATTACHMENT;		/* default */
					}

					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, 1) == false)
						goto __RETURN;

					if (__MmsBinaryDecodeParameter(pFile, pMsgType, valueLength, totalLength) == false) {
						MSG_DEBUG("Disposition parameter fail");
						goto __CATCH;
					}

					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, valueLength) == false)
						goto __RETURN;
				} else {

					gCurMmsDecodeBuffPos--;
					valueLength++;

					pLatinBuff = (char *)calloc(1, MSG_FILENAME_LEN_MAX);
					if (pLatinBuff == NULL)
						goto __CATCH;
					memset(pLatinBuff, 0, MSG_FILENAME_LEN_MAX);

					textLength = __MmsBinaryDecodeText(pFile, pLatinBuff, MSG_FILENAME_LEN_MAX-1, totalLength);


					if (textLength < 0) {
						MSG_DEBUG("Content-Disposition decodingfail.");
						goto __CATCH;
					}
					free(pLatinBuff);
					pLatinBuff = NULL;

					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, textLength) == false)
						goto __RETURN;

					valueLength -= textLength;

					if (__MmsBinaryDecodeParameter(pFile, pMsgType, valueLength, totalLength) == false)
					{
						MSG_DEBUG("Disposition parameter fail");
						goto __CATCH;
					}

					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, valueLength) == false)
						goto __RETURN;

				}

				break;

			case 0x0B:	/* Content-Encoding */

				if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
					MSG_DEBUG("Disposition value GetOneByte fail");
					goto __CATCH;
				}

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, 1) == false)
					goto __RETURN;

				break;

			case 0x0C:	/* Content-Language */

				if (__MmsBinaryDecodeInteger(pFile, (UINT32*)&tmpInt, &tmpIntLen, totalLength) == true) {
					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, tmpIntLen) == false)
						goto __RETURN;
				} else {
					char* cTemp = NULL;

					cTemp = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);

					if (cTemp == NULL) {
						MSG_DEBUG("__MmsBinaryDecodeText2 fail...");
						goto __CATCH;
					}

					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, textLength) == false) {
						if (cTemp) {
							free(cTemp);
						}
						goto __RETURN;
					}

					if (cTemp)
						free(cTemp);
				}

				break;

			case 0x0D:	/* Content-Length */

				if (__MmsBinaryDecodeInteger(pFile, (UINT32*)&tmpInt, &tmpIntLen, totalLength) == false) {
					MSG_DEBUG("__MmsBinaryDecodeInteger fail...");
					goto __CATCH;
				}

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, tmpIntLen) == false)
					goto __RETURN;

				break;

			case 0x1C:	/* Location */

				pLatinBuff = (char *)calloc(1, MMS_LOCATION_URL_LEN + 1);
				if (pLatinBuff == NULL)
					goto __CATCH;

				length = __MmsBinaryDecodeText(pFile, pLatinBuff, MMS_LOCATION_URL_LEN, totalLength);
				if (length == -1) {
					MSG_DEBUG("__MmsBinaryDecodeQuotedString fail.");
					goto __CATCH;
				}

				szSrc = __MsgConvertLatin2UTF8FileName(pLatinBuff);
				if (szSrc) {
					snprintf(pMsgType->szLocation, sizeof(pMsgType->szLocation), "%s", szSrc);
					MSG_INFO("Location : [%s]", pMsgType->szLocation);
					free(szSrc);
					szSrc = NULL;
				}

				free(pLatinBuff);
				pLatinBuff = NULL;

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, length) == false)
					goto __RETURN;

				break;

			case 0x30:	/* X-Wap-Content-URI skip this value */

				MSG_DEBUG("X-Wap-Content-URI header.");
				pLatinBuff = (char *)calloc(1, MMS_TEXT_LEN);
				if (pLatinBuff == NULL)
					goto __CATCH;

				length = __MmsBinaryDecodeText(pFile, pLatinBuff, MMS_TEXT_LEN,	totalLength);

				if (length == -1) {
					MSG_DEBUG(" __MmsBinaryDecodeQuotedString fail.");
					goto __CATCH;
				}

				MSG_DEBUG("X-Wap-Content-URI header decoded. Value length %d\n", length);
				free(pLatinBuff);
				pLatinBuff = NULL;

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, length) == false)
					goto __RETURN;

				MSG_DEBUG("X-Wap-Content-URI header skipped.");

				break;

			case 0x01:	/* Accept-charset */
/*				if (NvGetInt(NV_SI_ADM_GCF_STATE) == 1) */
				{
					/*	WAP-230-WSP-200010705-a.pdf
						8.4.2.8 Accept charset field
						The following rules are used to encode accept character set values.
						Accept-charset-value = Constrained-charset | Accept-charset-general-form
						Accept-charset-general-form = Value-length (Well-known-charset | Token-text) [Q-value]
						Constrained-charset = Any-charset | Constrained-encoding
						Well-known-charset = Any-charset | Integer-value
						; Both are encoded using values from Character Set Assignments table in Assigned Numbers
						Any-charset = <Octet 128>
						; Equivalent to the special RFC2616 charset value ��*��
					*/

					int	charset = 0;
					int charSetLen = 0;

					MSG_DEBUG("Accept-charset.");

					length = __MmsDecodeValueLength(pFile, &valueLength, totalLength);
					if (length > 0) {
						if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, length) == false)
							goto __RETURN;

					}

					if (__MmsBinaryDecodeInteger(pFile, (UINT32*)&charset, &charSetLen, totalLength) == false) {
						/* We only support the well-known-charset format */
						MSG_DEBUG("__MmsBinaryDecodeInteger fail...");
						goto __CATCH;
					}

					if (charset > 0)
						MmsGetBinaryType(MmsCodeCharSet, (UINT16)charset);

					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, charSetLen) == false)
						goto __RETURN;

					break;
				}

			default:

				/* Other Content-xxx headers : Have valueLength */

				MSG_WARN("unknown Value = 0x%x\n", oneByte);

				length = __MmsDecodeValueLength(pFile, &valueLength, totalLength);
				if (length <= 0) {
					MSG_WARN("invalid MMS_CODE_PREVIOUSLYSENTDATE");
					goto __CATCH;
				}

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, length) == false)
					goto __RETURN;

				szTemp = (char *)calloc(1, valueLength);
				if (szTemp == NULL)
					goto __CATCH;

				if (__MmsBinaryDecodeGetBytes(pFile, szTemp, valueLength, totalLength) == false) {
					MSG_WARN("default _MmsBinaryDecodeGetBytes() fail");
					if (szTemp) {
						free(szTemp);
						szTemp = NULL;
					}
					goto __CATCH;
				}

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, valueLength) == false)
					goto __RETURN;

				break;
			}
		} else {
			/*
			 * Application-header  = Token-text Application-specific-value
			 * Application-specific-value = Text-string
			 */

			MSG_DEBUG(" Application-header = Token-text Application-specific-value");

			gCurMmsDecodeBuffPos--;

			/* Token-text */

			textLength = 0;
			pCode = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
			if (pCode == NULL) {
				MSG_DEBUG("pCode is null");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, textLength) == false)
				goto __RETURN;

			MSG_DEBUG(" Token-text (%s) \n", pCode);


			/* Application-specific-value */

			textLength = 0;
			pValue = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
			if (pValue == NULL) {
				MSG_DEBUG("pValue is null");
				goto __CATCH;
			}

			MSG_DEBUG(" Application-specific-value (%s) \n", pValue);


			pParam = strchr(pValue, MSG_CH_ADDR_DELIMETER);
			if (pParam) {
				ch = *pParam;
				*pParam = '\0';
			}

			switch (MmsGetTextType(MmsCodeMsgBodyHeaderCode, pCode)) {
			case MMS_BODYHDR_TRANSFERENCODING:		/* Content-Transfer-Encoding */
				pMsgType->encoding = MmsGetTextType(MmsCodeContentTransferEncoding, pValue);
				break;

			case MMS_BODYHDR_CONTENTID:				/* Content-ID */
			{
				char szContentID[MMS_CONTENT_ID_LEN + 1];

				pLatinBuff = (char *)calloc(1, MMS_CONTENT_ID_LEN + 1);
				if (pLatinBuff == NULL)
				{
					goto __CATCH;
				}

				__MsgMIMERemoveQuote (pValue);
				strncpy(pLatinBuff, pValue, MMS_MSG_ID_LEN);

				length = strlen(pLatinBuff);
				if (__MsgLatin2UTF ((unsigned char*)szContentID, MMS_CONTENT_ID_LEN + 1, (unsigned char*)pLatinBuff, length) < 0)
				{
					MSG_DEBUG("MsgLatin2UTF fail");
					goto __CATCH;
				}

				MmsRemoveLessGreaterChar(szContentID, pMsgType->szContentID, sizeof(pMsgType->szContentID)); /* remove "< >" */

				free(pLatinBuff);
				pLatinBuff = NULL;
				break;
			}
			case MMS_BODYHDR_CONTENTLOCATION:		/* Content-Location */

				pLatinBuff = (char *)calloc(1, MMS_CONTENT_ID_LEN + 1);
				if (pLatinBuff == NULL)
					goto __CATCH;

				strncpy(pLatinBuff, pValue, MMS_MSG_ID_LEN);

				length = strlen(pLatinBuff);
				if (__MsgLatin2UTF ((unsigned char*)pMsgType->szContentLocation, MMS_CONTENT_ID_LEN + 1, (unsigned char*)pLatinBuff, length) < 0) {
					MSG_DEBUG("MsgLatin2UTF fail");
					goto __CATCH;
				}

				free(pLatinBuff);
				pLatinBuff = NULL;
				break;

			case MMS_BODYHDR_DISPOSITION:			/* Content-Disposition */
				pMsgType->disposition = MmsGetTextType(MmsCodeMsgDisposition, pValue);
				break;

			case MMS_BODYHDR_X_OMA_DRM_SEPARATE_DELIVERY:	/* DRM RO WAITING */
					break;

			default:
				MSG_DEBUG("Unknown Field : %s, Value: %s\n", pCode, pValue);
				break;
			}

			if (pParam) {
				__MsgParseParameter(pMsgType, pParam + 1);
				*pParam = ch;
			}
			if (pCode) {
				free(pCode);
				pCode = NULL;
			}
			if (pValue) {
				free(pValue);
				pValue = NULL;
			}

			if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, textLength) == false)
				goto __RETURN;

		}
	} /* while */

__RETURN:

	if (szTemp) {
		free(szTemp);
		szTemp = NULL;
	}

	if (pCode) {
		free(pCode);
		pCode = NULL;
	}

	return true;

__CATCH:

	if (pLatinBuff) {
		free(pLatinBuff);
		pLatinBuff = NULL;
	}
	if (pCode) {
		free(pCode);
		pCode = NULL;
	}
	if (pValue) {
		free(pValue);
		pValue = NULL;
	}

	if (szTemp) {
		free(szTemp);
		szTemp = NULL;
	}

	return false;
}

static bool __MmsBinaryDecodeEntries(FILE *pFile, UINT32 *npEntries, int totalLength)
{
	int length = 0;

	length = __MmsBinaryDecodeUintvar(pFile, npEntries, totalLength);
	if (length <= 0) {
		goto __CATCH;
	}

	MSG_INFO("Number of Entries = [%d]", *npEntries);

	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryDecodePartBody(FILE *pFile, UINT32 bodyLength, int totalLength)
{
	int offset = 0;

	/*
	 * Currently, offset and size is
	 * the only information used with msgBody.
	 * If you need, add here more information
	 */
	MSG_BEGIN();

	offset = __MmsGetDecodeOffset();
	offset += bodyLength;

	if (MsgFseek(pFile, offset, SEEK_SET) < 0) {
		MSG_DEBUG("fail to seek file pointer");
		goto __CATCH;
	}

	__MmsCleanDecodeBuff();

	gMmsDecodeCurOffset = offset;

	if (offset >= totalLength)
		goto __RETURN;

	if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
								   gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("fail to load to buffer");
		goto __CATCH;
	}

	return true;

__RETURN:
	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryDecodeMovePointer(FILE *pFile, int offset, int totalLength)
{
	if (offset > totalLength)
		goto __RETURN;

	if (MsgFseek(pFile, offset, SEEK_SET) < 0) {
		MSG_DEBUG("fail to seek file pointer");
		goto __CATCH;
	}

	__MmsCleanDecodeBuff();

	gMmsDecodeCurOffset = offset;

	if (offset == totalLength)
		goto __RETURN;

	if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
									gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("fail to load to buffer");
		goto __CATCH;
	}

__RETURN:
	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryDecodeMultipart(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, int totalLength)
{
	UINT32 nEntries = 0;
	MsgMultipart *pMultipart = NULL;
	MsgMultipart *pLastMultipart = NULL;
	int offset = 0;
	int	index = 0;

	MsgPresentationFactor factor = MSG_PRESENTATION_NONE;
	MsgPresentaionInfo presentationInfo;

	MSG_DEBUG("pdu length = [%d]", totalLength);

	presentationInfo.factor = MSG_PRESENTATION_NONE;
	presentationInfo.pCurPresentation = NULL;
	presentationInfo.pPrevPart = NULL;

	if (__MmsBinaryDecodeEntries(pFile, &nEntries, totalLength) == false) {
		MSG_DEBUG("MmsBinaryDecodeEntries is fail.");
		goto __CATCH;
	}

	if (pMsgBody->body.pMultipart != NULL) {
		pLastMultipart = pMsgBody->body.pMultipart;
		MSG_DEBUG("previous multipart exist [%p]", pMsgBody->body.pMultipart);
	} else {
		MSG_DEBUG("first multipart");
	}

	while (nEntries) {
		MSG_DEBUG("decoding [%d]th multipart", index);

		offset = __MmsGetDecodeOffset();
		if (offset >= totalLength) {
			MSG_DEBUG("offset is over totalLength");
			break;
		}

		if ((pMultipart = MmsAllocMultipart()) == NULL) {
			MSG_DEBUG("MsgAllocMultipart Fail");
			goto __CATCH;
		}

		if (__MmsBinaryDecodeEachPart(pFile, szFilePath, &(pMultipart->type), pMultipart->pBody, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodeEachPart is fail.(nEntries = %d)\n", nEntries);
			goto __CATCH;
		}

		if (pMultipart->type.type == MIME_APPLICATION_SMIL) {
			/* P151019-00477 : received mms message is type of multipart mixed, but among multiparts if there's smil part then set it multipart related */
			if (pMsgType->type == MIME_APPLICATION_VND_WAP_MULTIPART_MIXED)
				pMsgType->type = MIME_APPLICATION_VND_WAP_MULTIPART_RELATED;

			factor = __MsgIsPresentationEx(&(pMultipart->type), pMsgType->param.szStart, (MimeType)pMsgType->param.type);
			if (factor == MSG_PRESENTATION_NONE) {
				factor = MSG_PRESENTATION_TYPE_BASE;
			}
		} else {
			factor = MSG_PRESENTATION_NONE;
		}

		/* priority 1 : content type match, 2: content location, 3: type */
		if (presentationInfo.factor < factor) {
			/* Presentation part */
			presentationInfo.factor = factor;
			presentationInfo.pPrevPart = pLastMultipart;
			presentationInfo.pCurPresentation = pMultipart;
		}

		/* first multipart */
		if (pLastMultipart == NULL) {
			pMsgBody->body.pMultipart = pMultipart;
			pLastMultipart = pMultipart;
		} else {
			pLastMultipart->pNext = pMultipart;
			pLastMultipart = pMultipart;
		}

		pMsgType->contentSize += pMultipart->pBody->size;

		nEntries--;

		MmsPrintMulitpart(pMultipart, index++);

	}

	pMsgBody->size = totalLength - pMsgBody->offset;

	__MsgConfirmPresentationPart(pMsgType, pMsgBody, &presentationInfo);

	if (__MsgResolveNestedMultipart(pMsgType, pMsgBody) == false) {
		MSG_DEBUG("MsgResolveNestedMultipart failed");
		goto __CATCH;
	}

	return true;

__CATCH:
	if (pMultipart) {
		if (pMultipart->pBody) {
			free(pMultipart->pBody);
			pMultipart->pBody = NULL;
		}

		free(pMultipart);
		pMultipart = NULL;
	}

	return false;
}

static bool __MmsBinaryDecodeEachPart(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, int totalLength)
{
	int	length = 0;
	bool bSuccess = false;
	UINT32 headerLength = 0;
	UINT32 bodyLength = 0;
	int offset = 0;

	MSG_DEBUG("pdu length = [%d]", totalLength);

	/* header length */
	if (__MmsBinaryDecodeUintvar(pFile, &headerLength, totalLength) <= 0) {
		MSG_DEBUG("Get header length fail");
		goto __CATCH;
	}

	offset = __MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;

	/* body length */
	if (__MmsBinaryDecodeUintvar(pFile, &bodyLength, totalLength) <= 0) {
		MSG_DEBUG("Get body length fail");
		goto __CATCH;
	}

	offset = __MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;

	/* Content Type */
	if (szFilePath != NULL)
		snprintf(pMsgType->szOrgFilePath, sizeof(pMsgType->szOrgFilePath), "%s", szFilePath);

	pMsgType->offset = __MmsGetDecodeOffset();
	pMsgType->size = headerLength;
	pMsgType->contentSize = bodyLength;

	if (pMsgType->offset > totalLength)
		goto __RETURN;

	length = __MmsBinaryDecodeContentType(pFile, pMsgType, totalLength);
	if (length <= 0) {
		MSG_DEBUG("Decode contentType Fail");
		goto __CATCH;
	}

	offset = __MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;


	/* Part Header */

	if (__MmsBinaryDecodePartHeader(pFile, pMsgType, headerLength - length, totalLength) == false) {
		MSG_DEBUG("Decode contentHeader Fail");
		goto __CATCH;
	}

	offset = __MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;

	/* Part Body */

	if (szFilePath != NULL)
		snprintf(pMsgBody->szOrgFilePath, sizeof(pMsgBody->szOrgFilePath), "%s", szFilePath);

	pMsgBody->offset = __MmsGetDecodeOffset();
	pMsgBody->size	 = bodyLength;

	if (pMsgBody->offset > totalLength)
		goto __RETURN;

	switch (pMsgType->type) {
	case MIME_APPLICATION_VND_WAP_MULTIPART_MIXED:
	case MIME_APPLICATION_VND_WAP_MULTIPART_RELATED:
	case MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC:
	case MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE:
	case MIME_MULTIPART_REPORT:
	case MIME_MULTIPART_MIXED:
	case MIME_MULTIPART_RELATED:
	case MIME_MULTIPART_ALTERNATIVE:

		MSG_DEBUG("Multipart");
		if (__MmsBinaryDecodeMultipart(pFile, szFilePath, pMsgType, pMsgBody, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodeMultipart is fail");
			goto __CATCH;
		}

		offset = __MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		break;

	default:
		MSG_DEBUG("Normal Part");

		bSuccess = __MmsBinaryDecodePartBody(pFile, bodyLength, totalLength);
		if (bSuccess == false) {
			MSG_DEBUG("Decode contentBody Fail");
			goto __CATCH;
		}

		offset = __MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		break;
	}

	MSG_END();
	return true;

__RETURN:
	MSG_END();
	return true;

__CATCH:
	MSG_END();
	return false;
}

/* --------------------------------------------------------------------
 *
 *     B  I  N  A  R  Y       D  E  C  D  E      U  T  I  L  I  T  Y
 *
 * --------------------------------------------------------------------*/

bool __MmsBinaryDecodeGetOneByte(FILE *pFile, UINT8 *pOneByte, int totalLength)
{
	int length = gMmsDecodeMaxLen - gCurMmsDecodeBuffPos;

	if (pFile == NULL || pOneByte == NULL)
	{
		MSG_DEBUG("invalid file or buffer");
		goto __CATCH;
	}

	if (length < 1) {
		if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
									   gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
			MSG_DEBUG("fail to load to buffer");
			goto __CATCH;
		}
	}

	*pOneByte = gpCurMmsDecodeBuff[gCurMmsDecodeBuffPos++];

	return true;

__CATCH:
	return false;
}

/*
 * @remark: bufLen < gMmsDecodeMaxLen
 */
bool __MmsBinaryDecodeGetBytes(FILE *pFile, char *szBuff, int bufLen, int totalLength)
{
	int length = gMmsDecodeMaxLen - gCurMmsDecodeBuffPos;
	int i = 0;


	if (pFile == NULL || szBuff == NULL || bufLen == 0 || bufLen > gMmsDecodeMaxLen)
		goto __CATCH;

	memset(szBuff, 0, bufLen);

	if (length < bufLen) {
		if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
									   gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
			MSG_DEBUG("fail to load to buffer");
			goto __CATCH;
		}
	}

	for (i = 0; i < bufLen - 1; i++) {
		szBuff[i] = gpCurMmsDecodeBuff[gCurMmsDecodeBuffPos++];
	}

	gCurMmsDecodeBuffPos++;	/* NULL */

	return true;

__CATCH:
	return false;
}

bool __MmsBinaryDecodeGetLongBytes(FILE *pFile, char *szBuff, int bufLen, int totalLength)
{
	int iPos = 0;

	if (pFile == NULL || szBuff == NULL || bufLen == 0)
		goto __CATCH;

	memset(szBuff, 0, bufLen);

	if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
								   gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("fail to load to buffer");
		goto __CATCH;
	}

	while ((bufLen - iPos) >= gMmsDecodeMaxLen) {
		if (__MmsBinaryDecodeGetBytes(pFile, szBuff + iPos, gMmsDecodeMaxLen, totalLength) == false) {
			MSG_DEBUG("__MmsBinaryDecodeGetBytes fail");
			goto __CATCH;
		}

		iPos += gMmsDecodeMaxLen;
	}

	if ((bufLen - iPos) > 0) {
		if (__MmsBinaryDecodeGetBytes(pFile, szBuff + iPos, (bufLen - iPos), totalLength) == false) {
			MSG_DEBUG("__MmsBinaryDecodeGetBytes fail");
			goto __CATCH;
		}

		iPos += (bufLen - iPos);
	}

	return true;

__CATCH:
	return false;
}

/**
 * Decode uintvar to 32bit unsigned integer
 *
 * @param 	pEncodedData    [in] encoded data
 * @param	pUintVar		[out] Decode uintvar (32bit unsigned integer)
 * @return	The length of uintvar (-1, if cannot be converted to a uintvar)
 *
 * 0 XXXXXXX -> 0-bit: continue bit & 1~7bit: integer value
 * - -------
 */
static const UINT32 uintvarDecodeTable[] = { 0x00000001, 0x00000080, 0x00004000, 0x00100000, 0x08000000 };

static int __MmsBinaryDecodeUintvar(FILE *pFile, UINT32 *pUintVar, int totalLength)
{
	UINT8 count = 0;
	UINT8 oneByte = 0;
	UINT32 decodedUintvar = 0;
	UINT8 iBuff[5] = {0};
	int length = MSG_MMS_DECODE_BUFFER_MAX - gCurMmsDecodeBuffPos;


	if (pFile == NULL || pUintVar == NULL)
		return -1;

	if (length < 5) {
		if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
									   gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
			MSG_DEBUG("fail to load to buffer");
			goto __CATCH;
		}
	}

	while (true) {
		if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false)
			goto __CATCH;

		if (oneByte > 0x7f)	{
			iBuff[count++] = oneByte;
		} else {
			iBuff[count++] = oneByte;
			break;
		}

		if (count > 4) {
			MSG_DEBUG("legnth is too long");
			goto __CATCH;
		}
	}

	for (int i = 0; i < count; i++)
		decodedUintvar += (uintvarDecodeTable[i] * (iBuff[count-(i+1)]&0x7f));

	*pUintVar = decodedUintvar;

	return count;

__CATCH:
	gCurMmsDecodeBuffPos -= count;
	return -1;
}

/**
 * Decode uintvar to 32bit unsigned integer by uintvar length
 *
 * @param 	pEncodedData [in] uintvar encoded data
 * @param 	length		 [in] length of integer value
 * @return 	unsigned integer value
 */
static UINT32 __MmsHeaderDecodeIntegerByLength(FILE *pFile, UINT32 length, int totalLength)
{
	UINT32 i = 0;
	UINT8 oneByte = 0;
	char *pData = NULL;
	union {
		UINT32	integer;
		UINT8	seg[4];
	} returner;

	returner.integer = 0;

	if (length > 4)
		length = 4;

	if (length == 1)
	{
		if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
			MSG_DEBUG("_MmsBinaryDecodeGetOneByte fail");
			return oneByte;
		}

		if (oneByte > 0x7f) {
			return (oneByte & 0x7f);
		} else {
			return oneByte;
		}
	}

	if (length == 0)
		return 0;

	pData = (char *)calloc(1, length + 1);
	if (pData == NULL) {
		MSG_DEBUG("pData alloc fail");
		goto __CATCH;
	}
	memset(pData, 0, length + 1);

	if (__MmsBinaryDecodeGetBytes(pFile, pData, length + 1, totalLength) == false) {
		MSG_DEBUG("_MmsBinaryDecodeGetOneByte fail");
		goto __CATCH;
	}

	gCurMmsDecodeBuffPos--;	/* - NULL */

	for (i= 0; i < length; i++)
		returner.seg[length - (i+1)] = pData[i];

	if (pData) {
		free(pData);
		pData = NULL;
	}

	return returner.integer;

__CATCH:

	if (pData) {
		free(pData);
		pData = NULL;
	}

	return returner.integer;
}

/**
 * Decode uintvar to 32bit unsigned integer by uintvar length
 *
 * @param 	pEncodedData	[in]  uintvar encoded data
 * @param	pInteger		[out] Decode integer value (long/short)
 * @return 	unsigned integer value (-1, if cannot be converted to unsigned integer value)
 */
static bool __MmsBinaryDecodeInteger(FILE *pFile, UINT32 *pInteger, int *pIntLen, int totalLength)
{
	UINT8 oneByte	= 0;
	char *pData	= NULL;
	union {
		UINT32	integer;
		UINT8	seg[4];
	} returner;


	if (pInteger == NULL)
		return false;

	returner.integer = 0;
	*pIntLen		 = 0;

	if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
		MSG_DEBUG("GetOneByte fail");
		return false;
	}

	if (oneByte < 0x1F)				/* long integer : WAP-230-WSP-20010118-p, Proposed Version 18 January 2001 (pp.86) */
	{
		pData = (char *)calloc(1, oneByte + 1);
		if (pData == NULL) {
			MSG_DEBUG("pData calloc fail");
			goto __CATCH;
		}
		memset(pData, 0, oneByte + 1);

		/* Even NULL is copied in the _MmsBinaryDecodeGetBytes */
		if (__MmsBinaryDecodeGetBytes(pFile, pData, oneByte + 1, totalLength) == false) {
			MSG_DEBUG("GetBytes fail");
			goto __CATCH;
		}

		gCurMmsDecodeBuffPos--;	/* - NULL */

		int		length	= 0;
		if (oneByte > 4) {
			length = 4;
		} else {
			length = oneByte;
		}

		int		i = 0;
		for (i = 0; i < length; i++)
			returner.seg[length - (i+1)] = pData[i];

		*pInteger = returner.integer;
		*pIntLen  = oneByte + 1;
	} else if (oneByte >= 0x80)	{
		/* short integer : WAP-230-WSP-20010118-p, Proposed Version 18 January 2001 (pp.86) */
		*pInteger = oneByte & 0x7f;
		*pIntLen  = 1;
	} else {
		goto __CATCH;
	}

	if (pData) {
		free(pData);
		pData = NULL;
	}

	return true;

__CATCH:

	gCurMmsDecodeBuffPos--;

	if (pData) {
		free(pData);
		pData = NULL;
	}

	return false;
}

/**
 * Decode uintvar to 32bit unsigned integer by uintvar length
 *
 * @return 	1  : Success
 *			0  : This is not Value Length type data
 *			-1 : Requires System error report
 */
static int __MmsDecodeValueLength(FILE *pFile, UINT32 *pValueLength, int totalLength)
{
	int length = 0;
	UINT32 uintvar = 0;
	UINT8 oneByte = 0;


	/*
	 * value-length = short-length | (Length-quote Length)
	 *				= 0~30		   | 31 + Uintvar-length
	 */

	if (pFile == NULL || pValueLength == NULL)
		goto __CATCH;

	*pValueLength = 0;

	if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
		gCurMmsDecodeBuffPos--;
		goto __CATCH;
	}

	if (0x00 < oneByte && oneByte < 0x1F) {
		/* short-length */

		*pValueLength = oneByte;
		length = 1;
	} else if (oneByte == 0x1F) {
		/* Length-quote = 0x1F */

		length = __MmsBinaryDecodeUintvar(pFile, &uintvar, totalLength);
		if (length == -1) {
			MSG_DEBUG(" __MmsBinaryDecodeUintvar fail..");
			goto __CATCH;
		}
		length ++;					/* + length-quote */
		*pValueLength = uintvar;
	} else {
		MSG_DEBUG("not a value length type data");
		gCurMmsDecodeBuffPos--;
		return 0;
	}

	return length;

__CATCH:
	MSG_DEBUG("getting data fail");
	return -1;
}

/**
 * Decode uintvar to 32bit unsigned integer by uintvar length
 *
 * @return 	1  : Success
 *			0  : This is not Value Length type data
 *			-1 : Requires System error report
 * @ defference : if there is not length-quote, consider it as short length.
 */
static int __MmsDecodeValueLength2(FILE *pFile, UINT32 *pValueLength, int totalLength)
{
	int length	= 0;
	UINT32 uintvar = 0;
	UINT8 oneByte = 0;


	/*
	 * value-length = short-length | (Length-quote Length)
	 *				= 0~30		   | 31 + Uintvar-length
	 */

	if (pFile == NULL || pValueLength == NULL)
		goto __CATCH;

	*pValueLength = 0;

	if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
		gCurMmsDecodeBuffPos--;
		goto __CATCH;
	}

	if (0x00 < oneByte && oneByte < 0x1F) {
		/* short-length */

		*pValueLength = oneByte;
		length = 1;
	} else if (oneByte == 0x1F) {
		/* Length-quote = 0x1F */

		length = __MmsBinaryDecodeUintvar(pFile, &uintvar, totalLength);
		if (length == -1) {
			MSG_DEBUG("__MmsBinaryDecodeUintvar fail..");
			goto __CATCH;
		}
		length ++;					/* + length-quote */
		*pValueLength = uintvar;
	} else {
		MSG_DEBUG("there is not length-quote, consider it as short length.");
		*pValueLength = oneByte;
		length = 1;
	}

	return length;

__CATCH:
	MSG_DEBUG("getting data fail");
	return -1;
}

/**
 * Decode QuotedString
 *
 * @param 	pEncodedData	[in] QuotedString encoded data
 * @param	szBuff			[out] Decoded quoted string
 * @param	bufLen			[out] Buffer length
 * @return 	length of quoted string
 */
static int __MmsBinaryDecodeQuotedString(FILE *pFile, char *szBuff, int bufLen, int totalLength)
{
	int iPos = 0;
	int length = 0;
	int readBytes = 0;
	char *pData = NULL;
	int returnLength = 0;

	/*
	 * Quoted-string = <Octet 34> *TEXT End-of-string
	 * The TEXT encodes an RFC2616 Quoted-string with the enclosing quotation-marks <"> removed
	 */

	if (pFile == NULL || szBuff == NULL || bufLen <= 0)
		return -1;

	memset(szBuff, 0, bufLen);

	if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
						  			 gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("fail to load to buffer");
		goto __CATCH;
	}

	length = strlen(gpCurMmsDecodeBuff) + 1;	/* + NULL */

	if (length == 0)
		goto __RETURN;

	while (length > gMmsDecodeBufLen) {
		if (gMmsDecodeBufLen <= 0) {
			MSG_DEBUG("gMmsDecodeBufLen <= 0");
			MSG_DEBUG("%x %x %x %x %x",
										gpCurMmsDecodeBuff[0], gpCurMmsDecodeBuff[1], gpCurMmsDecodeBuff[2],
										gpCurMmsDecodeBuff[3], gpCurMmsDecodeBuff[4]);
			MSG_DEBUG("%x %x %x %x %x",
										gpCurMmsDecodeBuff[5], gpCurMmsDecodeBuff[6], gpCurMmsDecodeBuff[7],
										gpCurMmsDecodeBuff[8], gpCurMmsDecodeBuff[9]);
			MSG_DEBUG("%x %x %x %x %x",
										gpCurMmsDecodeBuff[10], gpCurMmsDecodeBuff[11], gpCurMmsDecodeBuff[12],
										gpCurMmsDecodeBuff[13], gpCurMmsDecodeBuff[14]);
			MSG_DEBUG("%x %x %x %x %x",
										gpCurMmsDecodeBuff[15], gpCurMmsDecodeBuff[16], gpCurMmsDecodeBuff[17],
										gpCurMmsDecodeBuff[18], gpCurMmsDecodeBuff[19]);
			goto __CATCH;
		}

		pData = (char *)calloc(1, gMmsDecodeBufLen + 1);
		if (pData == NULL)
			goto __CATCH;

		memset(pData, 0, gMmsDecodeBufLen + 1);

		if (__MmsBinaryDecodeGetBytes(pFile, pData, gMmsDecodeBufLen, totalLength) == false)
			goto __CATCH;

		returnLength += gMmsDecodeBufLen;

		if ((bufLen - iPos) > 0) {
			readBytes = (gMmsDecodeBufLen < (bufLen - iPos)) ? gMmsDecodeBufLen : (bufLen - iPos);
			if (iPos == 0 && (pData[0] == MARK)) {
				/* MARK: check first time only */

				strncpy(szBuff + iPos, (char*)pData + 1, readBytes - 1);
				iPos += (readBytes - 1);
			} else {
				strncpy(szBuff + iPos, (char*)pData, readBytes);
				iPos += readBytes;
			}
		}

		if (pData) {
			free(pData);
			pData = NULL;
		}

		if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
							 			gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
			MSG_DEBUG("fail to load to buffer");
			goto __CATCH;
		}
		length = strlen(gpCurMmsDecodeBuff) + 1;	/* + NULL */
	}	/* while */

	if (length > 0) {
		pData = (char *)calloc(1, length);
		if (pData == NULL)
			goto __CATCH;

		if (__MmsBinaryDecodeGetBytes(pFile, pData, length, totalLength) == false)
			goto __CATCH;

		returnLength += length;

		if ((bufLen - iPos) > 0) {
			/* read until NULL from raw data, and copy only string */
			readBytes = (length < (bufLen - iPos)) ? length : (bufLen - iPos);
			if (iPos == 0 && (pData[0] == MARK)) {
				/* MARK: check first time only */
				strncpy(szBuff + iPos, (char*)pData + 1, readBytes - 1);
				iPos += (readBytes - 1);
			} else {
				strncpy(szBuff + iPos, (char*)pData, readBytes - 1);	/* + NULL */
				iPos += readBytes;
			}
		}

		if (pData) {
			free(pData);
			pData = NULL;
		}
	}

	szBuff[bufLen - 1] = '\0';

	return returnLength;

__RETURN:

	return length;

__CATCH:

	if (pData) {
		free(pData);
		pData = NULL;
	}

	return -1;
}

/**
 * Decode Text
 *
 * @param 	pEncodedData	[in] QuotedString encoded data
 * @param	szBuff			[out] Decoded quoted string
 * @param	bufLen			[out] Buffer length
 * @return 	length of decode text string
 */
static int __MmsBinaryDecodeText(FILE *pFile, char *szBuff, int bufLen, int totalLength)
{
	int length = 0;
	int readBytes = 0;
	int iPos = 0;
	int returnLength = 0;
	char *pData = NULL;
	bool bQuote = false;
	int offset = 0;

	/*
	 * Text-String = [QUOTE]*TEXT end_of_string
	 *				 [QUOTE]*(128~255)\0
	 *				 *(32~126)\0
	 */

	if (pFile == NULL || szBuff == NULL || bufLen <= 0)
		return -1;

	offset = __MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;

	memset(szBuff, 0, bufLen);

	if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
						  			gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("fail to load to buffer");
		goto __CATCH;
	}

	length = strlen(gpCurMmsDecodeBuff) + 1;	/* + NULL */

	if (length == 0)
		goto __RETURN;

	while (length > gMmsDecodeBufLen) {
		if (gMmsDecodeBufLen <= 0) {
			MSG_DEBUG("gMmsDecodeBufLen <= 0");
			MSG_DEBUG("%x %x %x %x %x", gpCurMmsDecodeBuff[0], gpCurMmsDecodeBuff[1], gpCurMmsDecodeBuff[2],
										gpCurMmsDecodeBuff[3], gpCurMmsDecodeBuff[4]);
			MSG_DEBUG("%x %x %x %x %x", gpCurMmsDecodeBuff[5], gpCurMmsDecodeBuff[6], gpCurMmsDecodeBuff[7],
										gpCurMmsDecodeBuff[8], gpCurMmsDecodeBuff[9]);
			MSG_DEBUG("%x %x %x %x %x", gpCurMmsDecodeBuff[10], gpCurMmsDecodeBuff[11], gpCurMmsDecodeBuff[12],
										gpCurMmsDecodeBuff[13], gpCurMmsDecodeBuff[14]);
			MSG_DEBUG("%x %x %x %x %x", gpCurMmsDecodeBuff[15], gpCurMmsDecodeBuff[16], gpCurMmsDecodeBuff[17],
										gpCurMmsDecodeBuff[18], gpCurMmsDecodeBuff[19]);
			goto __CATCH;
		}

		pData = (char *)calloc(1, gMmsDecodeBufLen + 1);
		if (pData == NULL)
			goto __CATCH;

		memset(pData, 0, gMmsDecodeBufLen + 1);

		if (__MmsBinaryDecodeGetBytes(pFile, pData, gMmsDecodeBufLen, totalLength) == false)
			goto __CATCH;

		if ((bufLen - iPos) > 0) {
			readBytes = (gMmsDecodeBufLen < (bufLen - iPos)) ? gMmsDecodeBufLen : (bufLen - iPos);
			if (iPos == 0 && (pData[0] == QUOTE) && (bQuote == false)) {
				/* QUOTE: check first time only */

				strncpy(szBuff + iPos, (char*)pData + 1, readBytes - 1);
				iPos += (readBytes - 1);
				bQuote = true;
			} else {
				strncpy(szBuff + iPos, (char*)pData, readBytes);
				iPos += readBytes;
			}
		}

		if (pData) {
			free(pData);
			pData = NULL;
		}

		returnLength += gMmsDecodeBufLen;

		if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
									   gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
			MSG_DEBUG("fail to load to buffer");
			goto __CATCH;
		}
		length = strlen(gpCurMmsDecodeBuff) + 1;	/* + NULL */
	}	/* while */

	if (length > 0) {
		pData = (char *)calloc(1, length);
		if (pData == NULL)
			goto __CATCH;

		memset(pData, 0, length);

		if (__MmsBinaryDecodeGetBytes(pFile, pData, length, totalLength) == false)
			goto __CATCH;

		if ((bufLen - iPos) > 0) {
			readBytes = (length < (bufLen - iPos)) ? length : (bufLen - iPos);
			if (iPos == 0 && (pData[0] == QUOTE) && (bQuote == false)) {
				/* QUOTE: check first time only */

				strncpy(szBuff + iPos, (char*)pData + 1, readBytes - 1);
				iPos += (readBytes - 1);
				bQuote = true;
			} else {
				strncpy(szBuff + iPos, (char*)pData, readBytes - 1);	/* + NULL */
				iPos += readBytes;
			}
		}

		if (pData) {
			free(pData);
			pData = NULL;
		}

		returnLength += length;		/* + NULL */
	}

	szBuff[bufLen - 1] = '\0';

	return returnLength;

__RETURN:

	szBuff[0] = '\0';
	length = 0;

	__MmsBinaryDecodeMovePointer(pFile, offset, totalLength);

	return length;

__CATCH:

	if (pData) {
		free(pData);
		pData = NULL;
	}

	return -1;
}

static char* __MmsBinaryDecodeText2(FILE *pFile, int totalLength, int *pLength)
{
	int length = 0;
	int curLen = 0;
	char *pData = NULL;
	char *szBuff = NULL;
	char *szTempPtr = NULL;
	bool bQuote = false;
	int offset = 0;

	/*
	 * Text-String = [QUOTE]*TEXT end_of_string
	 *				 [QUOTE]*(128~255)\0
	 *				 *(32~126)\0
	 */

	if (pFile == NULL || pLength == NULL)
		goto __CATCH;

	*pLength = 0;
	offset = __MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;

	if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
								   gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("fail to load to buffer");
		goto __CATCH;
	}

	length = strlen(gpCurMmsDecodeBuff) + 1;

	if (length == 0)
		goto __CATCH;

	while (length > gMmsDecodeBufLen) {
		if (gMmsDecodeBufLen <= 0) {
			MSG_DEBUG("gMmsDecodeBufLen <= 0");
			MSG_DEBUG("%x %x %x %x %x",
										gpCurMmsDecodeBuff[0], gpCurMmsDecodeBuff[1], gpCurMmsDecodeBuff[2],
										gpCurMmsDecodeBuff[3], gpCurMmsDecodeBuff[4]);
			MSG_DEBUG("%x %x %x %x %x",
										gpCurMmsDecodeBuff[5], gpCurMmsDecodeBuff[6], gpCurMmsDecodeBuff[7],
										gpCurMmsDecodeBuff[8], gpCurMmsDecodeBuff[9]);
			MSG_DEBUG("%x %x %x %x %x",
										gpCurMmsDecodeBuff[10], gpCurMmsDecodeBuff[11], gpCurMmsDecodeBuff[12],
										gpCurMmsDecodeBuff[13], gpCurMmsDecodeBuff[14]);
			MSG_DEBUG("%x %x %x %x %x\n",
										gpCurMmsDecodeBuff[15], gpCurMmsDecodeBuff[16], gpCurMmsDecodeBuff[17],
										gpCurMmsDecodeBuff[18], gpCurMmsDecodeBuff[19]);
			goto __CATCH;
		}

		pData = (char *)calloc(1, gMmsDecodeBufLen + 1);
		if (pData == NULL)
			goto __CATCH;

		memset(pData, 0, gMmsDecodeBufLen + 1);

		if (__MmsBinaryDecodeGetBytes(pFile, pData, gMmsDecodeBufLen, totalLength) == false)
			goto __CATCH;

		if (szBuff == NULL)	{
			szBuff = (char *)calloc(1, gMmsDecodeBufLen + 1);
		} else {
			szTempPtr = (char *)realloc(szBuff, curLen + gMmsDecodeBufLen + 1);

			/* NULL pointer check for realloc */
			if (szTempPtr == NULL) {
				goto __CATCH;
			} else {
				szBuff = szTempPtr;
			}
		}
		if (szBuff == NULL)
			goto __CATCH;

		memset(szBuff + curLen, 0, gMmsDecodeBufLen + 1);

		if (curLen == 0 && (pData[0] == QUOTE) && (bQuote == false)) {
			/* QUOTE: check first time only */

			strncpy(szBuff + curLen, (char*)pData + 1, gMmsDecodeBufLen - 1);
			curLen += (gMmsDecodeBufLen - 1);
			bQuote = true;
		} else {
			strncpy(szBuff + curLen, (char*)pData, gMmsDecodeBufLen);
			curLen += gMmsDecodeBufLen;
		}

		if (pData) {
			free(pData);
			pData = NULL;
		}

		*pLength += gMmsDecodeBufLen;

		if (__MsgLoadDataToDecodeBuffer(pFile,
							   &gpCurMmsDecodeBuff,
							   &gCurMmsDecodeBuffPos,
							   &gMmsDecodeCurOffset,
							   gpMmsDecodeBuf1,
							   gpMmsDecodeBuf2,
							   gMmsDecodeMaxLen,
							   &gMmsDecodeBufLen,
							   totalLength) == false) {
			MSG_DEBUG("fail to load to buffer");
			goto __CATCH;
		}
		length = strlen(gpCurMmsDecodeBuff) + 1;
	}	/* while */

	if (length > 0)	{
		pData = (char *)calloc(1, length);
		if (pData == NULL) {
			goto __CATCH;
		}

		if (__MmsBinaryDecodeGetBytes(pFile, pData, length, totalLength) == false) {
			goto __CATCH;
		}

		if (szBuff == NULL) {
			szBuff = (char *)calloc(1, length);
		} else {
			szTempPtr = (char *)realloc(szBuff, curLen + length);

			/* NULL pointer check for realloc */
			if (szTempPtr == NULL)
				goto __CATCH;
			else
				szBuff = szTempPtr;
		}

		if (szBuff == NULL) {
			goto __CATCH;
		}

		memset(szBuff + curLen, 0, length);

		if (curLen == 0 && (pData[0] == QUOTE)  && (bQuote == false)) {
			/* QUOTE: check first time only */

			strncpy(szBuff + curLen, (char*)pData + 1, length - 2);
			curLen += (length - 1);
			bQuote = true;
		} else {
			strncpy(szBuff + curLen, (char*)pData, length - 1);
			curLen += length;
		}

		if (pData) {
			free(pData);
			pData = NULL;
		}

		*pLength += length;		/* + NULL */
	}

	return szBuff;

__RETURN:

	*pLength = 1;

	__MmsBinaryDecodeMovePointer(pFile, offset, totalLength);

	return szBuff;

__CATCH:

	if (szBuff) {
		free(szBuff);
		szBuff = NULL;
	}

	if (pData) {
		free(pData);
		pData = NULL;
	}

	return NULL;
}

/**
 * Decode Charset
 *
 * @param 	pEncodedData	[in] QuotedString encoded data
 * @param	nCharSet		[out] Decoded character set
 * @return 	length of charset value
 */
static bool __MmsBinaryDecodeCharset(FILE *pFile, UINT32 *nCharSet, int *pCharSetLen, int totalLength)
{
	UINT32 integer = 0;

	/*
	 * Charset v1.1 0x01 Well-known-charset
	 *					 Well-known-charset = Any-charset | Integer-value
	 *						; Both are encoded using values from
	 *						  Character Set Assignments table in Assigned Numbers
	 *					 Any-charset = <Octet 128>
	 *						; Equivalent to the special RFC2616 charset value ��*��
	 */

	if (pFile == NULL || nCharSet == NULL || pCharSetLen == NULL)
		return false;

	if (__MmsBinaryDecodeInteger(pFile, &integer, pCharSetLen, totalLength) == false) {
		MSG_DEBUG("__MmsBinaryDecodeInteger fail...");
		goto __CATCH;
	}

	if (integer == 0) {
		/* AnyCharSet : return MSG_CHARSET_UTF8 */
		*nCharSet = MSG_CHARSET_UTF8;
		return true;
	}

	*nCharSet = MmsGetBinaryType(MmsCodeCharSet, (UINT16)integer);
	MSG_DEBUG("Decoded charset MIBenum = [%d], charset enum = [%d]", integer, *nCharSet);
	if (*nCharSet == MIME_UNKNOWN) {
		MSG_DEBUG("MmsGetBinaryType fail..");
		*nCharSet = MSG_CHARSET_UNKNOWN;
	}

	return true;

__CATCH:
	return false;
}

/**
 * Decode EncodedString
 *
 * @param 	pEncodedData	[in] QuotedString encoded data
 * @param	szBuff			[out] Decoded string buffer
 * @param	bufLen			[in]  Decoded buffer length
 * @return 	length of decoded string length
 */
static bool __MmsBinaryDecodeEncodedString(FILE *pFile, char *szBuff, int bufLen, int totalLength)
{
	UINT32 valueLength = 0;
	UINT32 charSet = 0;
	int charSetLen = 0;
	int nTemp = 0;
	char *pData = NULL;

	MSG_DEBUG(" decode string..");

	if (pFile == NULL || szBuff == NULL || bufLen <= 0) {
		MSG_DEBUG("invalid file or buffer");
		goto __CATCH;
	}

	/*
	 * Encoded_string_value = Text-string | Value-length Char-set Text-String
	 *						  Text-string	 = [Quote]*TEXT End-of-string
	 *						  Value-length	 = 0 ~ 31
	 */

	memset(szBuff, 0, bufLen);

	switch (__MmsDecodeValueLength(pFile, &valueLength, totalLength)) {
	case -1:
		goto __CATCH;

	case 0:

		/* Text-string = [Quote]*TEXT End-of-string */

		if (__MmsBinaryDecodeText(pFile, szBuff, bufLen, totalLength) < 0) {
			MSG_DEBUG("__MmsBinaryDecodeText fail.");
			goto __CATCH;
		}
		break;

	default:

		/* Value-length Charset Text_string */

		if (__MmsBinaryDecodeCharset(pFile, &charSet, &charSetLen, totalLength) == false) {
			MSG_DEBUG(" __MmsBinaryDecodeCharset error");
			goto __CATCH;			/* (valueLength + valueLengthLen) */
		}

		nTemp = __MmsBinaryDecodeText(pFile, szBuff, bufLen, totalLength);

		if (nTemp < 0) {
			/* There can be some error in data - no NULL -> try again with value length */

			pData = (char *)calloc(1, valueLength - charSetLen);
			if (pData == NULL) {
				MSG_DEBUG("pData alloc fail.");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeGetLongBytes(pFile, pData, valueLength - charSetLen, totalLength) == false) {
				MSG_DEBUG("_MmsBinaryDecodeGetLongBytes fail.");
				goto __CATCH;
			}

			strncpy(szBuff, pData, bufLen - 1);
		}

		{ /* temp brace */

			nTemp = strlen(szBuff);

			const char *pToCharSet = "UTF-8";

			UINT16 charset_code =  MmsGetBinaryValue(MmsCodeCharSet, charSet);

			const char *pFromCharSet = MmsPluginTextConvertGetCharSet(charset_code);
			if (pFromCharSet == NULL || !strcmp(pFromCharSet, pToCharSet)) {
				if (pData) {
					free(pData);
					pData = NULL;
				}
				return true;
			}

			char *pDest = NULL;
			int destLen = 0;

			if (MmsPluginTextConvert(pToCharSet, pFromCharSet, szBuff, nTemp, &pDest, &destLen) == false) {
				MSG_DEBUG("MmsPluginTextConvert Fail");

			} else {
				memset(szBuff, 0x00, bufLen);
				snprintf(szBuff, destLen+1, "%s", pDest);
			}

			if (pDest) {
				free(pDest);
				pDest = NULL;
			}
		}
		break;
	}

	if (pData) {
		free(pData);
		pData = NULL;
	}

	return true;

__CATCH:

	if (pData) {
		free(pData);
		pData = NULL;
	}

	return false;
}



/**
 * Decode Encoded Addresses
 *
 * @param 	pEncodedData	[in] QuotedString encoded data
 * @param	pAddrLength		[out] Decoded address length
 * @return 	Decoded address list
 */
MsgHeaderAddress *__MmsDecodeEncodedAddress(FILE *pFile, int totalLength)
{
	UINT32 valueLength	= 0;
	UINT32 charSet		= 0;
	int charSetLen	= 0;
	int textLength	= 0;
	char *pAddrStr	= NULL;
	MsgHeaderAddress *pAddr = NULL;

	MSG_DEBUG("decoding address..");

	if (pFile == NULL) {
		MSG_DEBUG("invalid file or buffer");
		goto __CATCH;
	}

	/*
	 * Encoded_string_value = Text-string | Value-length Char-set Text-String
	 *						  Text-string	 = [Quote]*TEXT End-of-string
	 *						  Value-length	 = 0 ~ 31
	 */

	switch (__MmsDecodeValueLength(pFile, &valueLength, totalLength)) {
	case -1:
		goto __CATCH;

	case 0:

		/* Text-string = [Quote]*TEXT End-of-string */

		textLength = 0;
		pAddrStr   = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
		if (pAddrStr == NULL) {
			MSG_DEBUG(" __MmsBinaryDecodeText2 fail.");
			goto __CATCH;
		}
		break;

	default:

		/* Value-length Charset Text_string */

		if (__MmsBinaryDecodeCharset(pFile, &charSet, &charSetLen, totalLength) == false) {
			MSG_DEBUG(" __MmsBinaryDecodeCharset error");
			goto __CATCH;
		}

		textLength = 0;
		pAddrStr   = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
		if (pAddrStr == NULL) {
			/* There can be some error in data - no NULL -> try again with value length */

			pAddrStr = (char *)calloc(1, valueLength - charSetLen);
			if (pAddrStr == NULL) {
				MSG_DEBUG("pData alloc fail.");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeGetLongBytes(pFile, pAddrStr, valueLength - charSetLen, totalLength) == false) {
				MSG_DEBUG(" _MmsBinaryDecodeGetLongBytes fail.");
				goto __CATCH;
			}
		}

		/* fixme: charset transformation */

		break;
	}

	pAddr = (MsgHeaderAddress *)calloc(1, sizeof(MsgHeaderAddress));
	if (pAddr == NULL)
		goto __CATCH;

	memset(pAddr, 0, sizeof(MsgHeaderAddress));
	pAddr->szAddr = pAddrStr;

	return pAddr;

__CATCH:

	if (pAddrStr) {
		free(pAddrStr);
		pAddrStr = NULL;
	}

	return NULL;
}


/**
 * Decode Encoded Pointer String
 *
 * @param 	pEncodedData	[in] Long integer encoded data
 * @param	pLongInteger	[out] Decoded long integer
 * @return 	Decoded address list
 */
static bool __MmsDecodeLongInteger(FILE *pFile, UINT32 *pLongInteger, int totalLength)
{
	UINT8 oneByte = 0;

	/*
	 * Long-integer = Short-length Multi-octet-integer
	 *				  Short-length = 0~30
	 *				  Multi-octet-integer
	 */

	if (pFile == NULL || pLongInteger == NULL)
		return false;

	*pLongInteger = 0;

	if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false)
		goto __CATCH;

	if (oneByte > 31)
		goto __CATCH;

	*pLongInteger = __MmsHeaderDecodeIntegerByLength(pFile, oneByte, totalLength);

	return true;

__CATCH:
	return false;
}


/*
 * @param	pEncodedData	[in] filename encoded data
 * @param	szBuff			[out] filename output buffer
 * @param	fullLength		[in] full filename length
 * @param	bufLen			[in] buffer length
 * CAUTION: bufLen - 1
 */
static int __MmsDecodeGetFilename(FILE *pFile, char *szBuff, int bufLen, int totalLength)
{
	char *pUTF8Buff = NULL;
	char *pLatinBuff = NULL;
	char *pExt = NULL;
	char *szSrc = NULL;
	char *szSrc2 = NULL;
	int length = 0;
	int textLength = 0;

	char *pTmpBuff = NULL;

	memset (szBuff, 0, bufLen);

	textLength = 0;
	pLatinBuff  = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);

	/* remove "" */
	if (pLatinBuff) {
		szSrc = MsgRemoveQuoteFromFilename(pLatinBuff);
		if (szSrc) {
			strncpy(pLatinBuff, szSrc, textLength);
			free(szSrc);
			szSrc = NULL;
		}

		szSrc2 = MsgChangeHexString(pLatinBuff);
		if (szSrc2) {
			strncpy(pLatinBuff, szSrc2, textLength);
			free(szSrc2);
			szSrc2 = NULL;
		}

		if (MmsIsUtf8String((unsigned char*)pLatinBuff, strlen(pLatinBuff)) == false) {
			length = strlen(pLatinBuff);

			int		utf8BufSize = 0;
			utf8BufSize = __MsgGetLatin2UTFCodeSize((unsigned char*)pLatinBuff, length);
			if (utf8BufSize < 3)
				utf8BufSize = 3; /* min value */

			pUTF8Buff = (char *)calloc(1, utf8BufSize + 1);
			if (pUTF8Buff == NULL) {
				MSG_DEBUG("pUTF8Buff alloc fail");
				goto __CATCH;
			}

			if (__MsgLatin2UTF ((unsigned char*)pUTF8Buff, utf8BufSize + 1, (unsigned char*)pLatinBuff, length) < 0) {
				MSG_DEBUG("MsgLatin2UTF fail");
				goto __CATCH;
			}
			free(pLatinBuff);
			pLatinBuff = NULL;
		} else {
			pTmpBuff = MsgDecodeText(pLatinBuff);
			pUTF8Buff = pTmpBuff;
			free (pLatinBuff);
			pLatinBuff = NULL;
		}
	}

	if (pUTF8Buff) {
		/*
		 * keeping extension
		 * it should be kept extention even if the file name is shorten
		 */

		length = strlen(pUTF8Buff);
		if ((pExt = strrchr(pUTF8Buff, '.')) != NULL) {
			int nameLength = 0;
			nameLength = (length < bufLen) ? (length - strlen(pExt)) : (bufLen - strlen(pExt));
			strncpy(szBuff, pUTF8Buff, nameLength);
			g_strlcat(szBuff, pExt,(gsize)bufLen);

		} else {
			strncpy(szBuff, pUTF8Buff, bufLen - 1);
		}

		free(pUTF8Buff);
		pUTF8Buff = NULL;

		return textLength;
	}

__CATCH:

	if (pLatinBuff) {
		free(pLatinBuff);
		pLatinBuff = NULL;
	}

	if (pUTF8Buff) {
		free(pUTF8Buff);
		pUTF8Buff = NULL;
	}

	return -1;
}

/* ==========================================================

	   M  M  S        D  E  C  O  D  I  N  G

   ==========================================================*/

/* to get message body this function should be modified from message raw file. */
bool MmsReadMsgBody(msg_message_id_t msgID, bool bSavePartsAsTempFiles, bool bRetrieved, char *retrievedPath)
{
	FILE *pFile	= NULL;
	MmsMsg *pMsg = NULL;
	MsgMultipart *pMultipart = NULL;
	int nSize = 0;
	char szFullPath[MSG_FILEPATH_LEN_MAX] = {0, };
	char szTempMediaDir[MSG_FILEPATH_LEN_MAX] = {0, };

	MSG_BEGIN();

	MmsPluginStorage::instance()->getMmsMessage(&pMsg);
	memset(pMsg, 0, sizeof(MmsMsg));

	MmsInitHeader();

	if (bRetrieved && (retrievedPath != NULL)) {
		strncpy(szFullPath, retrievedPath, (strlen(retrievedPath) > MSG_FILEPATH_LEN_MAX ? MSG_FILEPATH_LEN_MAX:strlen(retrievedPath)));
	} else {
		MmsPluginStorage::instance()->getMmsRawFilePath(msgID, szFullPath, sizeof(szFullPath));
	}

	pMsg->msgID = msgID;

	/*	read from MMS raw file	*/
	strncpy(pMsg->szFileName, szFullPath + strlen(MSG_DATA_PATH), strlen(szFullPath + strlen(MSG_DATA_PATH)));

	MSG_SEC_DEBUG("msg_id = [%d]", msgID);
	MSG_SEC_DEBUG("raw file path = [%s]", szFullPath);

	if (MsgGetFileSize(szFullPath, &nSize) == false) {
		MSG_FATAL("Fail MsgGetFileSize");
		goto __CATCH;
	}

	pFile = MsgOpenFile(szFullPath, "rb");
	if (pFile == NULL) {
		MSG_SEC_DEBUG("Fail MsgOpenFile [%s]", szFullPath);
		goto __CATCH;
	}

	MmsRegisterDecodeBuffer();

	if (MmsBinaryDecodeMsgHeader(pFile, nSize) == false) {
		MSG_FATAL("Fail to MmsBinaryDecodeMsgHeader");
		goto __CATCH;
	}

	if (MmsBinaryDecodeMsgBody(pFile, szFullPath, nSize) == false) {
		MSG_FATAL("Fail to MmsBinaryDecodeMsgBody");
		goto __CATCH;
	}

	/* Set mmsHeader.msgType & msgBody to pMsg ----------- */

	memcpy(&(pMsg->msgType), &(mmsHeader.msgType), sizeof(MsgType));
	memcpy(&(pMsg->msgBody), &(mmsHeader.msgBody), sizeof(MsgBody));

{ /* attribute convert mmsHeader -> mmsAttribute */

	pMsg->mmsAttrib.contentType = (MimeType)mmsHeader.msgType.type;

	pMsg->mmsAttrib.date = mmsHeader.date;

	if (mmsHeader.deliveryReport == MMS_REPORT_YES) {
		pMsg->mmsAttrib.bAskDeliveryReport = true;
	}

	memcpy(&pMsg->mmsAttrib.deliveryTime, &mmsHeader.deliveryTime, sizeof(MmsTimeStruct));

	memcpy(&pMsg->mmsAttrib.expiryTime, &mmsHeader.expiryTime, sizeof(MmsTimeStruct));

	pMsg->mmsAttrib.msgClass = mmsHeader.msgClass;

	snprintf(pMsg->szMsgID, sizeof(pMsg->szMsgID), "%s", mmsHeader.szMsgID);

	pMsg->mmsAttrib.msgType = mmsHeader.type;

	pMsg->mmsAttrib.version = mmsHeader.version;

	pMsg->mmsAttrib.msgSize = mmsHeader.msgSize;

	pMsg->mmsAttrib.priority = mmsHeader.priority;

	if (mmsHeader.readReply == MMS_REPORT_YES) {
		pMsg->mmsAttrib.bAskReadReply = true;
	}

	snprintf(pMsg->mmsAttrib.szSubject, sizeof(pMsg->mmsAttrib.szSubject), "%s", mmsHeader.szSubject);

	snprintf(pMsg->szTrID, sizeof(pMsg->szTrID), "%s", mmsHeader.szTrID);

	pMsg->mmsAttrib.retrieveStatus = mmsHeader.retrieveStatus;

	/* FIXME:: mmsHeader will release after delete global mmsHeader */
	/* memset(&(mmsHeader.msgBody), 0x00, sizeof(MsgBody)); */ /* After copy to MmsMsg */
}
	if (pMsg->msgBody.pPresentationBody) {
		if (MsgFseek(pFile, pMsg->msgBody.pPresentationBody->offset, SEEK_SET) < 0)
			goto __CATCH;

		pMsg->msgBody.pPresentationBody->body.pText = (char *)calloc(1, pMsg->msgBody.pPresentationBody->size + 1);
		if (pMsg->msgBody.pPresentationBody->body.pText == NULL)
			goto __CATCH;

		memset(pMsg->msgBody.pPresentationBody->body.pText, 0, pMsg->msgBody.pPresentationBody->size + 1);

		ULONG nRead = 0;
		nRead = MsgReadFile(pMsg->msgBody.pPresentationBody->body.pText, sizeof(char), pMsg->msgBody.pPresentationBody->size, pFile);
		if (nRead == 0)
			goto __CATCH;

	}

	MsgCloseFile(pFile);
	pFile = NULL;
	/* nPartCount */
	pMsg->nPartCount = 0;

	if (MsgIsMultipart(mmsHeader.msgType.type) == true) {
		pMultipart = pMsg->msgBody.body.pMultipart;
		while (pMultipart) {
			pMsg->nPartCount++;
			pMultipart = pMultipart->pNext;
		}
	} else {
		if (pMsg->msgBody.size > 0)
			pMsg->nPartCount++;
	}

	/* 	make temporary	*/
	snprintf(szTempMediaDir, MSG_FILEPATH_LEN_MAX, "%s%s.dir", MSG_DATA_PATH, pMsg->szFileName);

	if (MsgIsMultipart(pMsg->msgType.type) == true) {
		int partIndex = 0;
		pMultipart = pMsg->msgBody.body.pMultipart;

		if (bSavePartsAsTempFiles) {
			if (mkdir(szTempMediaDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
				if (errno == EEXIST) {
					MSG_SEC_DEBUG("exist dir : [%s]", szTempMediaDir);
				} else {
					MSG_SEC_DEBUG("Fail to Create Dir [%s]", szTempMediaDir);
					goto __CATCH;
				}
			} else {
				MSG_SEC_DEBUG("make dir : [%s]", szTempMediaDir);
			}
		}

		if (pMsg->msgBody.pPresentationBody) {
			if (__MmsMultipartSaveAsTempFile(&pMsg->msgBody.presentationType, pMsg->msgBody.pPresentationBody,
												(char*)MSG_DATA_PATH, pMsg->szFileName, 0, bSavePartsAsTempFiles) == false)
				goto __CATCH;
		}

		while (pMultipart) {

			if (__MmsMultipartSaveAsTempFile(&pMultipart->type, pMultipart->pBody,
											(char*)MSG_DATA_PATH, pMsg->szFileName, partIndex, bSavePartsAsTempFiles) == false)
				goto __CATCH;

			MmsPrintMulitpart(pMultipart, partIndex);

			pMultipart = pMultipart->pNext;
			partIndex ++;
		}

	} else { /* single part */
		if (pMsg->nPartCount > 0) {

			if (bSavePartsAsTempFiles) {
				if (mkdir(szTempMediaDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
					if (errno == EEXIST) {
						MSG_DEBUG("exist dir : [%s]", szTempMediaDir);
					} else {
						MSG_DEBUG("Fail to Create Dir [%s]", szTempMediaDir);
						goto __CATCH;
					}
				} else {
					MSG_DEBUG("make dir : [%s]", szTempMediaDir);
				}
			}

			if (__MmsMultipartSaveAsTempFile(&pMsg->msgType, &pMsg->msgBody,
											(char*)MSG_DATA_PATH, pMsg->szFileName, 0, bSavePartsAsTempFiles) == false)
				goto __CATCH;
		}
	}
	MSG_DEBUG("### Success ###");
	MSG_END();
	return true;

__CATCH:

	MmsInitHeader();
	MmsUnregisterDecodeBuffer();

	if (pFile != NULL) {
		MsgCloseFile(pFile);
		pFile = NULL;
	}


	MmsReleaseMsgDRMInfo(&pMsg->msgType.drmInfo);

	MmsReleaseMsgBody(&pMsg->msgBody, pMsg->msgType.type);

	MSG_DEBUG("### Fail ###");
	MSG_END();
	return false;
}

static bool __MsgFreeHeaderAddress(MsgHeaderAddress *pAddr)
{
	MsgHeaderAddress *pTempAddr = NULL;

	while (pAddr != NULL) {
		pTempAddr = pAddr;
		pAddr = pAddr->pNext;

		if (pTempAddr->szAddr) {
			free(pTempAddr->szAddr);
			pTempAddr->szAddr = NULL;
		}

		free(pTempAddr);
		pTempAddr = NULL;
	}

	return true;
}

static bool __MsgCheckFileNameHasInvalidChar(char *szName)
{
	int	strLen = 0;
	int i = 0;

	strLen = strlen(szName);

	for (i=0; i<strLen; i++) {
		if (__MsgIsInvalidFileNameChar(szName[i]))
			return true;
	}

	return false;
}

static bool __MsgReplaceInvalidFileNameChar(char *szInText, char replaceChar)
{
	int nCount = 0;
	int totalLength = 0;

	totalLength = strlen(szInText);

	while ((*(szInText+nCount) != '\0') && (nCount < totalLength)) {
		if (0x0001 <= *(szInText+nCount) && *(szInText+nCount) <= 0x007F) {
			if (__MsgIsInvalidFileNameChar(szInText[nCount]))
				*(szInText+nCount) = replaceChar;

			nCount += 1;
		} else {
			nCount += 2;
		}
	}

	return true;
}

static char *__MsgGetStringUntilDelimiter(char *pszString, char delimiter)
{
	char *pszBuffer = NULL;
	char *pszStrDelimiter = NULL;
	int	bufLength = 0;

	if (!pszString)	{
		MSG_DEBUG("pszString == NULL");
		return NULL;
	}

	if ((pszStrDelimiter = strchr(pszString, delimiter)) == NULL) {
		MSG_DEBUG("There is no %c in %s. \n", delimiter, pszString);
		return NULL;
	}

	bufLength = pszStrDelimiter - pszString;

	if ((pszBuffer = (char*)calloc (1, bufLength + 1)) == NULL) {
		MSG_DEBUG("calloc is failed");
		return NULL;
	}
	memset(pszBuffer, 0, bufLength + 1) ;

	strncat(pszBuffer, pszString, bufLength);

	return pszBuffer;
}

char *MsgChangeHexString(char *pOrg)
{
	char *pNew = NULL;
	char szBuf[10] = {0,};
	char OneChar;
	int cLen = 0;
	int cIndex =0;
	int index = 0;

	if (pOrg == NULL)
		return false;

	cLen = strlen(pOrg);

	pNew = (char *)calloc(1, cLen + 1);
	if (pNew == NULL)
		return NULL;

	memset(pNew, 0, cLen + 1);

	for (cIndex = 0; cIndex< cLen ; cIndex++) {
		if (pOrg[cIndex] == '%') {
			if (pOrg[cIndex+1] != 0 && pOrg[cIndex+2] != 0) {
				snprintf(szBuf, sizeof(szBuf), "%c%c", pOrg[cIndex+1], pOrg[cIndex+2]); /* read two chars after '%' */

				if (__MsgIsHexChar(szBuf) == true) { /* check the two character is between  0 ~ F */
					OneChar = __MsgConvertHexValue(szBuf);

					pNew[index] = OneChar;
					index++;
					cIndex+= 2;
					continue;
				}
			}
		}
		pNew[index++] = pOrg[cIndex];
	}
	return pNew;
}

static bool __MsgParseParameter(MsgType *pType, char *pSrc)
{
	char *pName = NULL;
	char *pValue = NULL;
	char *pDec = NULL;
	char *pTest = NULL;
	char *pNextParam = NULL;
	char *pExt = NULL;
	int nameLen = 0;
	int count;
	char *pTempNextParam = NULL;
	char *pCh = NULL;
	char *szSrc = NULL;
	char *pUTF8Buff	= NULL;

	while (pSrc != NULL) {
		pSrc = __MsgSkipWS(pSrc);
		if (pSrc == NULL) {
			/* End of parse parameter */
			return true;
		}

		pNextParam = NULL;
		pTempNextParam = strchr(pSrc, MSG_CH_SEMICOLON);
		pCh = pSrc;

		if (*pCh == MSG_CH_QUOT) {
			count = 1;
		} else {
			count = 0;
		}

		pCh++;
		for (; pCh<=pTempNextParam ; pCh++) {
			if (*pCh == MSG_CH_QUOT)
				if (*(pCh - 1) != '\\')
					count++;
		}

		if (count%2 == 0)
			pNextParam = pTempNextParam;

		if (pNextParam)
			*pNextParam++ = MSG_CH_NULL;

		if ((pName = strchr(pSrc, MSG_CH_EQUAL)) != NULL) {
			*pName++ = MSG_CH_NULL;

			if ((pValue = strchr(pName, MSG_CH_QUOT))!= NULL) {
				*pValue++ = MSG_CH_NULL;

				if ((pTest = strchr(pValue, MSG_CH_QUOT)) != NULL)
					*pTest = MSG_CH_NULL;

				pDec = MsgDecodeText(pValue);		/* Api is to long, consider Add to another file (MsgMIMECodec.c) */
			} else {
				pDec = MsgDecodeText(pName);
			}

			if (pDec) {
				switch (MmsGetTextType(MmsCodeParameterCode, pSrc)) {
				case MSG_PARAM_BOUNDARY:

					/* RFC 822: boundary := 0*69<bchars> bcharsnospace */

					memset (pType->param.szBoundary, 0, MSG_BOUNDARY_LEN + 1);
					strncpy(pType->param.szBoundary, pDec, MSG_BOUNDARY_LEN);
					MSG_SEC_INFO("szBoundary = [%s]", pType->param.szBoundary);
					break;

				case MSG_PARAM_CHARSET:
					pType->param.charset = MmsGetTextType(MmsCodeParameterCode, pDec);

					if (pType->param.charset == -1)
						pType->param.charset = MSG_CHARSET_UNKNOWN;

					MSG_SEC_INFO("type = %d    [charset] = %d", pType->type, pType->param.charset);
					break;

				case MSG_PARAM_NAME:

					memset (pType->param.szName, 0, MSG_LOCALE_FILENAME_LEN_MAX + 1);

					pUTF8Buff = __MsgConvertLatin2UTF8FileName(pDec);

					if (pUTF8Buff) {
						if ((pExt = strrchr(pUTF8Buff, '.')) != NULL) {
							if ((MSG_LOCALE_FILENAME_LEN_MAX-1) < strlen(pUTF8Buff)) {
								nameLen = (MSG_LOCALE_FILENAME_LEN_MAX-1) - strlen(pExt);
							} else {
								nameLen = strlen(pUTF8Buff) - strlen(pExt);
							}

							strncpy(pType->param.szName, pUTF8Buff, nameLen);
							g_strlcat(pType->param.szName, pExt, sizeof(pType->param.szName));
						} else {
							strncpy(pType->param.szName, pUTF8Buff, (MSG_LOCALE_FILENAME_LEN_MAX-1));
						}
						free(pUTF8Buff);
						pUTF8Buff = NULL;

						if (__MsgChangeSpace(pType->param.szName, &szSrc) == true) {
							if (szSrc)
								strncpy(pType->param.szName, szSrc , strlen(szSrc));
						}

						if (szSrc) {
							free(szSrc);
							szSrc = NULL;
						}

						/* Remvoe '/', ex) Content-Type: image/gif; name="images/vf7.gif" */
						__MsgRemoveFilePath(pType->param.szName);
					} else {
						MSG_SEC_DEBUG("MsgConvertLatin2UTF8FileName(%s) return NULL", pDec);
					}

					MSG_SEC_INFO("szName = %s", pType->param.szName);
					break;

				case MSG_PARAM_FILENAME:

					memset (pType->param.szFileName, 0, MSG_FILENAME_LEN_MAX+1);

					pUTF8Buff = __MsgConvertLatin2UTF8FileName(pDec);

					if (pUTF8Buff) {
						if ((pExt = strrchr(pUTF8Buff, '.')) != NULL) {
							if ((MSG_FILENAME_LEN_MAX-1) < strlen(pUTF8Buff)) {
								nameLen = (MSG_FILENAME_LEN_MAX-1) - strlen(pExt);
							} else {
								nameLen = strlen(pUTF8Buff) - strlen(pExt);
							}

							strncpy(pType->param.szFileName, pUTF8Buff, nameLen);
							g_strlcat (pType->param.szFileName, pExt, sizeof(pType->param.szFileName));
						} else {
							strncpy(pType->param.szFileName, pUTF8Buff, (MSG_FILENAME_LEN_MAX-1));
						}
						free(pUTF8Buff);
						pUTF8Buff = NULL;

						if (__MsgChangeSpace(pType->param.szFileName, &szSrc) == true) {
							snprintf(pType->param.szFileName, sizeof(pType->param.szFileName), "%s", szSrc);
						}

						if (szSrc) {
							free(szSrc);
							szSrc = NULL;
						}

						/* Remvoe '/', ex) Content-Type: image/gif; name="images/vf7.gif" */
						__MsgRemoveFilePath(pType->param.szFileName);
					} else {
						MSG_SEC_DEBUG("MsgConvertLatin2UTF8FileName(%s) return NULL", pDec);
					}

					MSG_SEC_INFO("szFileName = %s", pType->param.szFileName);

					break;

				case MSG_PARAM_TYPE:

					/* type/subtype of root. Only if content-type is multipart/related */

					pType->param.type = MimeGetMimeIntFromMimeString(pDec);
					MSG_SEC_INFO("type = %d", pType->param.type);

					break;

				case MSG_PARAM_START:

					/* Content-id. Only if content-type is multipart/related */

					memset (pType->param.szStart, 0, MSG_MSG_ID_LEN + 1);
					strncpy(pType->param.szStart, pDec, MSG_MSG_ID_LEN);

					MSG_SEC_INFO("szStart = %s", pType->param.szStart);

					break;

				case MSG_PARAM_START_INFO:

					/* Only if content-type is multipart/related */

					memset (pType->param.szStartInfo, 0, MSG_MSG_ID_LEN + 1);
					strncpy(pType->param.szStartInfo, pDec, MSG_MSG_ID_LEN);

					MSG_SEC_INFO("szStartInfo = %s", pType->param.szStartInfo);

					break;

				case MSG_PARAM_REPORT_TYPE:

					/* only used as parameter of Content-Type: multipart/report; report-type=delivery-status; */

					if (strcasecmp(pDec, "delivery-status") == 0) {
						pType->param.reportType = MSG_PARAM_REPORT_TYPE_DELIVERY_STATUS;
					} else {
						pType->param.reportType = MSG_PARAM_REPORT_TYPE_UNKNOWN;
					}

					MSG_SEC_INFO("reportType = %s", pDec);
					break;

				default:

					MSG_DEBUG("Unknown paremeter (%s)", pDec);
					break;
				}

				free(pDec);
				pDec = NULL;
			}
		}
		pSrc = pNextParam;
	}
	return true;
}

static char *__MsgSkipWS(char *s)
{
	while (true) {
		if ((*s == MSG_CH_CR) || (*s == MSG_CH_LF) || (*s == MSG_CH_SP)	|| (*s == MSG_CH_TAB)) {
			++s;
		} else if ((*s != '(') || (__MsgSkipComment(s,(long)NULL)==NULL)) {
			return s;
		}
	}
}

static char *__MsgSkipComment (char *s,long trim)
{

	char *ret;
	char *s1 = s;
	char *t = NULL;

	/* ignore empty space */
	for (ret = ++s1; *ret == ' '; ret++)
		;

	/* handle '(', ')', '\',  '\0' */
	do {
		switch (*s1) {
		case '(':
			if (!__MsgSkipComment (s1,(long)NULL))
				goto __NULL_RETURN;
			t = --s1;
			break;
		case ')':
			s = ++s1;
			if (trim) {
				if (t) {
					t[1] = '\0';
				} else {
					*ret = '\0';
				}
			}
			return ret;
		case '\\':
			if (*++s1)
				break;
		case '\0':
			*s = '\0';
			goto __NULL_RETURN;
		case ' ':
			break;
		default:
			t = s1;
			break;
		}
	} while (s1++);

__NULL_RETURN:
	return NULL;
}

static char *__MsgConvertLatin2UTF8FileName(char *pSrc)
{
	char *pUTF8Buff  = NULL;
/*	char *pData = NULL; */


	/* convert utf8 string */
	if (MmsIsUtf8String((unsigned char*)pSrc, strlen(pSrc)) == false) {
		int length  = 0;
		int utf8BufSize = 0;

		length = strlen(pSrc);
		utf8BufSize = __MsgGetLatin2UTFCodeSize((unsigned char*)pSrc, length);
		if (utf8BufSize < 3)
			utf8BufSize = 3; /* min value */

		pUTF8Buff = (char *)calloc(1, utf8BufSize + 1);

		if (pUTF8Buff == NULL) {
			MSG_DEBUG("pUTF8Buff alloc fail");
			goto __CATCH;
		}

		if (__MsgLatin2UTF ((unsigned char*)pUTF8Buff, utf8BufSize + 1, (unsigned char*)pSrc, length) < 0) {
			MSG_DEBUG("MsgLatin2UTF fail");
			goto __CATCH;
		}
	} else {
		int length = strlen(pSrc);
		pUTF8Buff = (char *)calloc(1, length+1);

		if (pUTF8Buff == NULL) {
			MSG_DEBUG("pUTF8Buff alloc fail");
			goto __CATCH;
		}

		memcpy(pUTF8Buff, pSrc, length);
	}

	/* convert hex string */
/*
	if (__MsgIsPercentSign(pUTF8Buff) == true) {
		pData = MsgChangeHexString(pUTF8Buff);
		if (pData) {
			free(pUTF8Buff);
			pUTF8Buff = pData;
		}
	}
*/

	return pUTF8Buff;

__CATCH:

	if (pUTF8Buff) {
		free(pUTF8Buff);
		pUTF8Buff = NULL;
	}

	return NULL;
}

static bool __MsgChangeSpace(char *pOrg, char **ppNew)
{
	char *pNew = NULL;
	int cLen = 0;
	int cIndex =0;
	int index = 0;

	if (pOrg == NULL)
		return false;

	cLen = strlen(pOrg);

	pNew = (char *)calloc(1, cLen + 1);
	if (pNew == NULL)
		return false;

	memset(pNew, 0, cLen + 1);

	for (cIndex=0; cIndex<cLen;cIndex++) {
		if (pOrg[cIndex] == '%' && pOrg[cIndex+1] == '2' && pOrg[cIndex+2] == '0') {
			pNew[index] = ' ';
			index++;
			cIndex+= 2;
			continue;
		}
		pNew[index++] = pOrg[cIndex];
	}

	*ppNew = pNew;

	return true;
}

static void __MsgRemoveFilePath(char *pSrc)
{
	/* Remvoe '/', ex) Content-Type: image/gif; name="images/vf7.gif" */
	char *pTemp = NULL;
	char *tmp_name = NULL;

	tmp_name = MsgGetFileName(pSrc);
	if (tmp_name) {
		snprintf(pSrc, strlen(tmp_name), "%s", tmp_name);
		g_free(tmp_name);
		tmp_name = NULL;
	}

	/* Remove additional file information
	 * ex) Content-type: application/octet-stream; name="060728gibson_210.jpg?size=s"
	 * if "?size=" exist, insert NULL char. */
	{
		pTemp = strcasestr(pSrc, "?size=");
		if (pTemp != NULL)
			*pTemp = '\0';
	}
}

#if 0
static bool __MsgIsPercentSign(char *pSrc)
{
	char *pCh = NULL;
	bool bRet = false;

	pCh = strchr(pSrc , '%');

	if (pCh != NULL) {
		bRet = true;
	} else {
		bRet = false;
	}

	return bRet;
}
#endif

static MsgPresentationFactor __MsgIsPresentationEx(MsgType *multipartType, char* szStart, MimeType typeParam)
{
	char szTmpStart[MSG_MSG_ID_LEN + 3] = { 0, };
	char szTmpContentID[MSG_MSG_ID_LEN + 3] = { 0, };
	char szTmpContentLO[MSG_MSG_ID_LEN + 3] = { 0, };
	int strLen = 0;

	/* remove '<' and '>' in Start Param : contentID ex] <0_1.jpg> or <1233445> */
	if (szStart && szStart[0]) {
		int startLen = 0;
		startLen = strlen(szStart);
		if (szStart[0] == '<' && szStart[startLen - 1] == '>') {
			strncpy(szTmpStart, &szStart[1], startLen - 2);
		} else {
			strncpy(szTmpStart, szStart, startLen);
		}
	}

	/* remove '<' and '>' in ContentID : contentID ex] <0_1.jpg> or <1233445> */
	if (multipartType->szContentID[0]) 	{
		strLen = strlen(multipartType->szContentID);
		if (multipartType->szContentID[0] == '<' && multipartType->szContentID[strLen - 1] == '>') {
			strncpy(szTmpContentID, &(multipartType->szContentID[1]), strLen - 2);
		} else {
			strncpy(szTmpContentID, multipartType->szContentID, strLen);
		}
	}

	/* remove '<' and '>' in ContentLocation : contentID ex] <0_1.jpg> or <1233445> */
	if (multipartType->szContentLocation[0]) {
		strLen = strlen(multipartType->szContentLocation);
		if (multipartType->szContentLocation[0] == '<' && multipartType->szContentLocation[strLen - 1] == '>') {
			strncpy(szTmpContentLO, &multipartType->szContentLocation[1], strLen - 2);
		} else {
			strncpy(szTmpContentLO, multipartType->szContentLocation, strLen);
		}
	}

	if ((szTmpContentID[0] == '\0') && (szTmpContentLO[0] == '\0') && (multipartType->type == MIME_UNKNOWN))
		return  MSG_PRESENTATION_NONE;

	/* exception handling */
	if (szTmpStart[0] != '\0') {
		/* presentation part : 1.compare with contentID 2.compare with content Location 3. compare with type */
		if (strcmp(szTmpStart, szTmpContentID) == 0) {
			return MSG_PRESENTATION_ID;
		} else if (strcmp(szTmpStart, szTmpContentLO) == 0) {
			return   MSG_PRESENTATION_LOCATION;
		} else if (multipartType->type == typeParam) {
			return   MSG_PRESENTATION_TYPE_BASE;
		} else {
			return   MSG_PRESENTATION_NONE;
		}
	} else {
		if (multipartType->type == typeParam && typeParam != MIME_UNKNOWN) {
			return   MSG_PRESENTATION_TYPE_BASE;
		} else {
			return   MSG_PRESENTATION_NONE;
		}
	}
}

static void __MsgConfirmPresentationPart(MsgType *pMsgType, MsgBody *pMsgBody, MsgPresentaionInfo *pPresentationInfo)
{
	MSG_BEGIN();
	MsgMultipart *pNextPart = NULL;
	MsgMultipart *pRemovePart = NULL;

	if (__MsgIsMultipartRelated(pMsgType->type)) {
		/* assign the multipart to presentation part */
		/* remove the multipart(pCurPresentation) which is presentation part from the linked list. */
		/* if there is no presentation part -> assign first multipart to presentation part by force. */
		if (pPresentationInfo->pCurPresentation == NULL) {
			pPresentationInfo->pCurPresentation	= pMsgBody->body.pMultipart;
			pPresentationInfo->pPrevPart		= NULL;
			pPresentationInfo->factor			= MSG_PRESENTATION_NONE;
		}

		if (pPresentationInfo->pCurPresentation != NULL && __MsgIsPresentablePart(pPresentationInfo->pCurPresentation->type.type)) {
			/* Presentable Part is some MARK-UP page, such as SMIL, HTML, WML, XHTML.
			 * In this case, COPY the Presentation part and leave other multiparts.
			 */
			memcpy(&pMsgBody->presentationType, &pPresentationInfo->pCurPresentation->type, sizeof(MsgType));
			pMsgBody->pPresentationBody = pPresentationInfo->pCurPresentation->pBody;

			/* remove pCurPresentation from multipart linked list */
			if ((pPresentationInfo->factor == MSG_PRESENTATION_NONE)||(pPresentationInfo->pPrevPart == NULL)) {
				/* first part */
				pMsgBody->body.pMultipart = pPresentationInfo->pCurPresentation->pNext;
				pMsgType->contentSize -= pPresentationInfo->pCurPresentation->pBody->size;
				pMsgBody->size -= pPresentationInfo->pCurPresentation->pBody->size;
				if (pPresentationInfo->pCurPresentation) {

					MmsReleaseMsgDRMInfo(&pPresentationInfo->pCurPresentation->type.drmInfo);

					free(pPresentationInfo->pCurPresentation);
					pPresentationInfo->pCurPresentation = NULL;
				}
			} else {
				/* not a first part */
				pPresentationInfo->pPrevPart->pNext = pPresentationInfo->pCurPresentation->pNext;
				pMsgType->contentSize -= pPresentationInfo->pCurPresentation->pBody->size;
				pMsgBody->size -= pPresentationInfo->pCurPresentation->pBody->size;
				if (pPresentationInfo->pCurPresentation) {
					free(pPresentationInfo->pCurPresentation);
					pPresentationInfo->pCurPresentation = NULL;
				}
			}
		} else if (pPresentationInfo->pCurPresentation != NULL && MmsIsTextType(pPresentationInfo->pCurPresentation->type.type)) {
			/* NON-Presentable Part is some PLAIN part such as, text/plain, multipart/alternative.
			 * In this case, leave the Presentation part as a multipart and remove other multiparts.
			 */

			/* Backup the multipart link information */
			pNextPart = pMsgBody->body.pMultipart;

			/* Copy presentation part as a main part */
			memcpy(pMsgType, &pPresentationInfo->pCurPresentation->type, sizeof(MsgType));
			memcpy(pMsgBody, pPresentationInfo->pCurPresentation->pBody, sizeof(MsgBody));

			/* Remove multipart linked list */
			while (pNextPart) {
				pRemovePart = pNextPart;
				pNextPart = pNextPart->pNext;

				if (pRemovePart->pBody) {
					MmsReleaseMsgBody(pRemovePart->pBody, pRemovePart->type.type);
					free(pRemovePart->pBody);
					pRemovePart->pBody = NULL;
				}

				free(pRemovePart);
				pRemovePart = NULL;
			}
		} else {


			MmsReleaseMsgDRMInfo(&pMsgBody->presentationType.drmInfo);

			MmsInitMsgType(&pMsgBody->presentationType);
			pMsgBody->pPresentationBody = NULL;
		}
	}
	MSG_END();
}

static bool __MsgIsMultipartRelated(int type)
{
	if (type == MIME_MULTIPART_RELATED || type == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {
		return true;
	} else {
		return false;
	}
}

static bool __MsgIsPresentablePart(int type)
{
	if (type == MIME_TEXT_HTML || type == MIME_TEXT_VND_WAP_WML || type == MIME_APPLICATION_SMIL) {
		return true;
	} else {
		return false;
	}
}

static bool __MsgResolveNestedMultipart(MsgType *pPartType, MsgBody *pPartBody)
{
	MSG_BEGIN();
	MsgMultipart *pTmpMultipart = NULL;
	MsgMultipart *pSelectedPart = NULL;
	MsgMultipart *pPrevPart = NULL;
	MsgMultipart *pFirstPart = NULL;
	MsgMultipart *pLastPart = NULL;
	MsgMultipart *pRemoveList = NULL;
	MsgMultipart *pNextRemovePart = NULL;

	switch (pPartType->type) {
	case MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE:
	case MIME_MULTIPART_ALTERNATIVE:

		/* fixme:
		 * Policy: multipart/alternative
		 * multipart/alternative message has only several parts of media.
		 * You should choose one of them and make the alternative part
		 * to the selected media part.
		 */

		MSG_DEBUG("MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE");

		pSelectedPart = pPartBody->body.pMultipart;

		/* NULL Pointer check!! */
		if (pSelectedPart == NULL) {
			MSG_DEBUG("multipart(ALTERNATIVE) does not exist");
			break;
		}

		pTmpMultipart = pPartBody->body.pMultipart->pNext;

		while (pTmpMultipart) {
			if (pSelectedPart->type.type <= pTmpMultipart->type.type)
				pSelectedPart = pTmpMultipart;

			pTmpMultipart = pTmpMultipart->pNext;
		}

		pTmpMultipart = pPartBody->body.pMultipart;
		pPrevPart = NULL;

		while (pTmpMultipart) {
			if (pSelectedPart == pTmpMultipart)
				break;

			pPrevPart = pTmpMultipart;
			pTmpMultipart = pTmpMultipart->pNext;
		}

		if (pPrevPart == NULL) {
			/* selected part is the first part */
			pRemoveList = pSelectedPart->pNext;
		} else {
			pPrevPart->pNext = pSelectedPart->pNext;
			pRemoveList = pPartBody->body.pMultipart;
			pPartBody->body.pMultipart = pSelectedPart;
		}

		pSelectedPart->pNext = NULL;

		if (pRemoveList) {

			MmsReleaseMsgDRMInfo(&pRemoveList->type.drmInfo);

			MmsReleaseMsgBody(pRemoveList->pBody, pRemoveList->type.type);

			free(pRemoveList->pBody);
			free(pRemoveList);
		}

		if (__MsgCopyNestedMsgType(pPartType, &(pSelectedPart->type)) == false) {
			MSG_DEBUG("MsgPriorityCopyMsgType failed");
			goto __CATCH;
		}

		if (pSelectedPart->pBody != NULL)
			memcpy(pPartBody, pSelectedPart->pBody, sizeof(MsgBody));

		if (pSelectedPart != NULL) {

			MmsReleaseMsgDRMInfo(&pSelectedPart->type.drmInfo);

			if (pSelectedPart->pBody != NULL) {
				free(pSelectedPart->pBody);
				pSelectedPart->pBody = NULL;
			}
			free(pSelectedPart);
			pSelectedPart = NULL;
		}

		break;

	case MIME_APPLICATION_VND_WAP_MULTIPART_RELATED:
	case MIME_MULTIPART_RELATED:

		MSG_DEBUG("MIME_APPLICATION_VND_WAP_MULTIPART_RELATED");

		pSelectedPart = pPartBody->body.pMultipart;

		while (pSelectedPart) {
			if (__MsgIsMultipartMixed(pSelectedPart->type.type)) {

				if (pSelectedPart->pBody == NULL) {
					MSG_DEBUG("pSelectedPart->pBody(1) is NULL");
					break;
				}

				pFirstPart = pSelectedPart->pBody->body.pMultipart;

				if (pFirstPart == NULL) {
					MSG_DEBUG("multipart(RELATED) does not exist");
					break;
				}

				if (pFirstPart->pNext) {
					pLastPart = pFirstPart->pNext;
					while (pLastPart->pNext)
						pLastPart = pLastPart->pNext;
				} else {
					pLastPart = pFirstPart;
				}

				if (pPrevPart == NULL) {
					/* the first part */
					pTmpMultipart = pPartBody->body.pMultipart->pNext;
					pPartBody->body.pMultipart = pFirstPart;
					pLastPart->pNext = pTmpMultipart;
				} else {
					pTmpMultipart = pSelectedPart->pNext;
					pPrevPart->pNext = pFirstPart;
					pLastPart->pNext = pTmpMultipart;
				}

				if (pSelectedPart) {

					MmsReleaseMsgDRMInfo(&pSelectedPart->type.drmInfo);

					free(pSelectedPart->pBody);
					free(pSelectedPart);
				}
				pSelectedPart = pTmpMultipart;
			} else if (__MsgIsMultipartRelated(pSelectedPart->type.type) && pPrevPart != NULL) {
				pPrevPart->pNext = pTmpMultipart = pSelectedPart->pNext;
				MmsReleaseMsgBody(pSelectedPart->pBody, pSelectedPart->type.type);

				free(pSelectedPart->pBody);
				free(pSelectedPart);
				pSelectedPart = pTmpMultipart;
			} else {
				pPrevPart = pSelectedPart;
				pSelectedPart = pSelectedPart->pNext;
			}
		}

		break;


	case MIME_APPLICATION_VND_WAP_MULTIPART_MIXED:
	case MIME_MULTIPART_MIXED:

		MSG_DEBUG("MIME_APPLICATION_VND_WAP_MULTIPART_MIXED");

		pPrevPart = NULL;
		pSelectedPart = pPartBody->body.pMultipart;

		while (pSelectedPart) {
			if (MsgIsMultipart(pSelectedPart->type.type)) {
				if (pSelectedPart->pBody == NULL) {
					MSG_DEBUG("pSelectedPart->pBody(2) is NULL");
					break;
				}

				pFirstPart = pSelectedPart->pBody->body.pMultipart;

				/* NULL Pointer check!! */
				if (pFirstPart == NULL) {
					MSG_DEBUG("multipart does not exist");
					break;
				}

				if (pFirstPart->pNext) {
					pLastPart = pFirstPart->pNext;
					while (pLastPart->pNext)
						pLastPart = pLastPart->pNext;
				} else {
					pLastPart = pFirstPart;
				}

				if (pPrevPart == NULL) {
					/* the first part */
					pTmpMultipart = pPartBody->body.pMultipart->pNext;
					pPartBody->body.pMultipart = pFirstPart;
					pLastPart->pNext = pTmpMultipart;
				} else {
					pTmpMultipart = pSelectedPart->pNext;
					pPrevPart->pNext = pFirstPart;
					pLastPart->pNext = pTmpMultipart;
				}

				if (pSelectedPart->pBody->pPresentationBody)
					pPartBody->pPresentationBody = pSelectedPart->pBody->pPresentationBody;

				memcpy(&pPartBody->presentationType,
						  &pSelectedPart->pBody->presentationType, sizeof(MsgType));

				pPartType->type = pSelectedPart->type.type;

				MmsReleaseMsgDRMInfo(&pSelectedPart->type.drmInfo);

				free(pSelectedPart->pBody);
				free(pSelectedPart);

				pSelectedPart = pTmpMultipart;
			} else {
				pPrevPart = pSelectedPart;
				pSelectedPart = pSelectedPart->pNext;
			}
		}

		break;

	case MIME_MULTIPART_REPORT:

		MSG_DEBUG("MIME_MULTIPART_REPORT");

		pTmpMultipart = pPartBody->body.pMultipart;
		pPrevPart = NULL;

		if (pTmpMultipart == NULL) {
			MSG_DEBUG("pTmpMultipart == NULL");
			return false;
		}

		while (pTmpMultipart) {
			if (pTmpMultipart->type.type == MIME_TEXT_PLAIN) {
				pSelectedPart = pTmpMultipart;
				break;
			}

			pPrevPart = pTmpMultipart;
			pTmpMultipart = pTmpMultipart->pNext;
		}

		if (pSelectedPart == NULL) {
			MSG_DEBUG("MIME_MULTIPART_REPORT [no selected part]");

			pRemoveList = pPartBody->body.pMultipart->pNext;
			if (pPartBody->body.pMultipart != NULL) {
				pSelectedPart = pPartBody->body.pMultipart;
				pSelectedPart->pNext = NULL;
			}
		} else {
			if (pPrevPart == NULL) {
				/* first part is selected */
				pRemoveList = pPartBody->body.pMultipart->pNext;
			} else {
				pRemoveList = pPartBody->body.pMultipart->pNext;
				pPrevPart->pNext = pSelectedPart->pNext;
			}

			pSelectedPart->pNext = NULL;
			pPartBody->body.pMultipart = pSelectedPart;
		}

		pTmpMultipart = pRemoveList;

		while (pTmpMultipart) {

			MmsReleaseMsgDRMInfo(&pTmpMultipart->type.drmInfo);

			MmsReleaseMsgBody(pTmpMultipart->pBody, pTmpMultipart->type.type);
			pNextRemovePart = pTmpMultipart->pNext;

			free(pTmpMultipart->pBody);
			free(pTmpMultipart);
			pTmpMultipart = pNextRemovePart;
		}

		if (__MsgCopyNestedMsgType(pPartType, &(pSelectedPart->type)) == false) {
			MSG_DEBUG("MsgPriorityCopyMsgType failed");
			goto __CATCH;
		}

		if (pSelectedPart != NULL) {

			if (pSelectedPart->pBody != NULL)
				memcpy(pPartBody, pSelectedPart->pBody, sizeof(MsgBody));

			MmsReleaseMsgDRMInfo(&pSelectedPart->type.drmInfo);

			if (pSelectedPart->pBody != NULL) {
				free(pSelectedPart->pBody);
				pSelectedPart->pBody = NULL;
			}
			free(pSelectedPart);
			pSelectedPart = NULL;
		}

		break;

	default:
		break;
	}
	MSG_END();

	return true;

__CATCH:
	return false;

}

char *MsgResolveContentURI(char *szSrc)
{
	char *szTemp = NULL;
	char *szReturn = NULL;
	int length = 0;

	if (szSrc == NULL) {
		goto __CATCH;
	}

	if (szSrc[0] == '\0')
		goto __CATCH;


	if (!strncasecmp(szSrc, "cid:", 4)) {
		length = strlen(szSrc) - 3;
		szSrc += 4;
	} else {
		length = strlen(szSrc) + 1;
	}

	szTemp = (char *)calloc(1, length);
	if (szTemp == NULL) {
		MSG_DEBUG("memory full");
		goto __CATCH;
	}

	memset(szTemp, 0, length);

	strncpy(szTemp, szSrc, length - 1);

	szReturn = MsgChangeHexString(szTemp);

	if (szTemp) {
		free(szTemp);
		szTemp = NULL;
	}

	return szReturn;

__CATCH:

	return NULL;
}

char *MsgRemoveQuoteFromFilename(char *pSrc)
{
	int cLen = 0;	/* length of pBuff */
	char *pBuff = NULL;

	if (pSrc == NULL) {
		MSG_DEBUG("pSrc is Null");
		return NULL;
	}

	cLen = strlen(pSrc);

	pBuff = (char *)calloc(1, cLen + 1);

	if (pBuff == NULL) {
		MSG_DEBUG("pBuff mem alloc fail!");
		return NULL;
	}
	memset(pBuff, 0 , sizeof(char)*(cLen + 1));

	/* remove front quote */
	if (pSrc[0] == MSG_CH_QUOT) {
		cLen--;
		strncpy(pBuff, &pSrc[1], cLen);
		pBuff[cLen] = '\0';
	} else if (pSrc[0] == MSG_CH_LF) {
		cLen--;
		strncpy(pBuff, &pSrc[1], cLen);
	} else {
		strncpy(pBuff, pSrc, cLen);
	}

	/* remove last qoute */
	if (pBuff[cLen-1] == MSG_CH_QUOT) {
		pBuff[cLen-1] = '\0';
	}

	return pBuff;
}

bool MsgIsMultipart(int type)
{
	if (type == MIME_MULTIPART_RELATED || type == MIME_APPLICATION_VND_WAP_MULTIPART_MIXED ||
		type == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED || type == MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC ||
		type == MIME_MULTIPART_MIXED || type == MIME_MULTIPART_REPORT) {
		return true;
	} else {
		return false;
	}
}


static bool __MsgIsHexChar(char *pSrc)
{
	int cIndex = 0;
	int cLen = 0;
	bool bRet = false;

	cLen = strlen(pSrc);

	for (cIndex = 0; cIndex < cLen ; cIndex++) {
		if ((pSrc[cIndex] >= '0' && pSrc[cIndex] <= '9') || (pSrc[cIndex] >= 'A'&& pSrc[cIndex] <= 'F') ||
			(pSrc[cIndex] >= 'a' && pSrc[cIndex] <= 'f')) {
			bRet = true;
		} else {
			return false;
		}
	}

	return bRet;
}

static char __MsgConvertHexValue(char *pSrc)
{
	int ch = 0;
	int cIndex = 0;
	int cLen = 0;
	char ResultChar;
	unsigned char uCh[2] = {0,};

	cLen = strlen(pSrc);

	for (cIndex = 0; cIndex < cLen ; cIndex += 2) {
		uCh[0] = __MsgConvertCharToInt(pSrc[cIndex]);
		uCh[1] = __MsgConvertCharToInt(pSrc[cIndex+1]);
		ch = (int)uCh[0]<<4|uCh[1];
	}

	ResultChar = (char)ch;

	return ResultChar;
}

static int __MsgConvertCharToInt(char ch)
{
	if (ch>='0' && ch<='9') {
		return ch - '0';
	} else if (ch>='a'&& ch <='f') {
		return ch -'a'+10;
	} else if (ch>='A'&& ch <='F') {
		return ch -'A'+10;
	} else {
		return 0;
	}
}

static bool __MsgCopyNestedMsgType(MsgType *pMsgType1, MsgType *pMsgType2)
{
	if (!pMsgType1 || !pMsgType2)
		return false;

/*	if (pMsgType1->section == INVALID_HOBJ) */
		pMsgType1->section = pMsgType2->section;

	int length = 0;

	if (pMsgType1->drmInfo.drmType == MSG_DRM_TYPE_NONE)
		pMsgType1->drmInfo.drmType = pMsgType2->drmInfo.drmType;


	if (pMsgType1->szContentID[0] == '\0') {
		snprintf(pMsgType1->szContentID, sizeof(pMsgType1->szContentID), "%s", pMsgType2->szContentID);
	}

	if (pMsgType1->szContentID[0] != '\0') {

		length = strlen(pMsgType1->szContentID);
		if (pMsgType1->szContentID[0] == '<' && pMsgType1->szContentID[length - 1] == '>') {
			char szTempString[MSG_MSG_ID_LEN + 1];
			MmsRemoveLessGreaterChar(pMsgType1->szContentID, szTempString, sizeof(szTempString));
			pMsgType1->drmInfo.szContentURI = g_strdup(szTempString);
		} else {
			pMsgType1->drmInfo.szContentURI = g_strdup(pMsgType1->szContentID);
		}
	}

	if (pMsgType1->szContentLocation[0] == '\0') {
		strncpy(pMsgType1->szContentLocation, pMsgType2->szContentLocation, MSG_MSG_ID_LEN);
	}

	/* Copy informations - we shoud open the pMsgType2's orgFile
	 * concerning its offset and size.
	 */
	if (pMsgType2->szOrgFilePath[0] != '\0') {
		strncpy(pMsgType1->szOrgFilePath, pMsgType2->szOrgFilePath, MSG_FILEPATH_LEN_MAX-1);
	}

	if (pMsgType2->disposition != -1)
		pMsgType1->disposition = pMsgType2->disposition;

	if ((pMsgType1->type != MIME_APPLICATION_VND_OMA_DRM_MESSAGE && pMsgType1->type != MIME_APPLICATION_VND_OMA_DRM_CONTENT) &&
		 pMsgType2->encoding != -1)
		pMsgType1->encoding = pMsgType2->encoding;

	pMsgType1->contentSize = pMsgType2->contentSize;
	pMsgType1->offset = pMsgType2->offset;
	pMsgType1->size = pMsgType2->size;
	pMsgType1->type = pMsgType2->type;

	__MsgCopyNestedMsgParam(&(pMsgType1->param), &(pMsgType2->param));

	if (pMsgType1->param.szName[0]) {
		pMsgType1->drmInfo.szContentName = g_strdup(pMsgType2->param.szName);
	}

	return true;
}

static bool __MsgCopyNestedMsgParam(MsgContentParam *pParam1, MsgContentParam *pParam2)
{
	if (pParam1->charset == MSG_CHARSET_UNKNOWN)
		pParam1->charset = pParam2->charset;

	if (pParam1->type == MIME_UNKNOWN)
		pParam1->type = pParam2->type;

	/* Don't copy pParam2->pPresentation */

	/* For alternative: copy the boundary string */
	if (pParam2->szBoundary[0] !='\0') {
		strncpy(pParam1->szBoundary, pParam2->szBoundary, MSG_BOUNDARY_LEN);
	}

	if (pParam1->szFileName[0] =='\0') {
		strncpy(pParam1->szFileName, pParam2->szFileName, MSG_FILENAME_LEN_MAX);
	}

	if (pParam1->szName[0] =='\0') {
		strncpy(pParam1->szName, pParam2->szName, MSG_LOCALE_FILENAME_LEN_MAX);
	}

	if (pParam1->szStart[0] =='\0') {
		strncpy(pParam1->szStart, pParam2->szStart, MSG_MSG_ID_LEN);
	}

	if (pParam1->szStartInfo[0] =='\0') {
		strncpy(pParam1->szStartInfo, pParam2->szStartInfo, MSG_MSG_ID_LEN);
	}
	return true;
}

static bool __MsgIsMultipartMixed(int type)
{
	if (type == MIME_APPLICATION_VND_WAP_MULTIPART_MIXED || type == MIME_MULTIPART_MIXED) {
		return true;
	} else {
		return false;
	}
}

static bool __MsgIsInvalidFileNameChar(char ch)
{
	if ((ch == 0x5C /* \ */) ||
		(ch == 0x2F /* / */) ||
		(ch == 0x3A /* : */) ||
		(ch == 0x2A /* * */) ||
		(ch == 0x3F /* ? */) ||
		(ch == 0x22 /* " */) ||
		(ch == 0x3C /* < */) ||
		(ch == 0x3E /* > */) ||
		(ch == 0x7C /* | */))
		return true;

	return false;
}

static int __MsgGetLatin2UTFCodeSize(unsigned char *szSrc, int nChar)
{
	int nCount = 0;

	MSG_DEBUG("---------------");

	if ((szSrc == NULL) || (nChar <= 0)) {
		MSG_DEBUG("szSrc is NULL !!!! ---------------");
		return 0;
	}

	while ((nChar > 0) && (*szSrc != '\0')) {
		if (0x01 <= *szSrc && *szSrc <= 0x7F) {
			nCount += 1;
			szSrc++;
			nChar--;
		} else {
			nCount += 2;
			szSrc++;
			nChar--;
		}
	}

	return nCount;
}

static int __MsgLatin2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar)
{
	unsigned char*	org;
	unsigned char	t1, t2;

	MSG_DEBUG("---------------");

	org = des;
	outBufSize--;			/* NULL character */

	while ((nChar > 0) && (*szSrc != '\0')) {
		if (0x01 <= *szSrc && *szSrc <= 0x7F) {
			/* check outbuffer's room for this UTF8 character */

			outBufSize --;
			if (outBufSize < 0)
				goto __RETURN;

			*des = (unsigned char) (*szSrc & 0x007F);

			des++;
			szSrc++;
			nChar--;
		} else {
			/* check outbuffer's room for this UTF8 character */

			outBufSize -= 2;
			if (outBufSize < 0)
				goto __RETURN;

			t2 = (unsigned char) (*szSrc & 0x003F);				/* right most 6 bit */
			t1 = (unsigned char) ((*szSrc & 0xC0) >> 6);		/* right most 2 bit */

			*des = 0xC0 | (t1 & 0x1F);
			*(des + 1) = 0x80 | (t2 & 0x3F);

			des += 2;
			szSrc += 1;
			nChar -= 1;
		}
	}

__RETURN:

	*des = 0;
	return (des - org);
}

bool MmsAddrUtilCheckEmailAddress(char *pszAddr)
{
	if (!pszAddr || pszAddr[0] == 0)
		return false;

	if (!strchr (pszAddr, MSG_MMS_CH_EMAIL_AT))
		return false;

	return true;
}

bool MmsAddrUtilRemovePlmnString(char *pszAddr)
{
	char *pszAddrCopy = NULL;
	char *pszStrStart = NULL;
	char *pszStrTemp = NULL;
	int strLen = 0;

	if ((!pszAddr) || (pszAddr[0] == 0)) {
		MSG_DEBUG("pszAddr is null or zero");
		return false;
	}

	strLen = strlen(pszAddr);

	pszAddrCopy = (char*)calloc(1,strLen + 1);
	if (!pszAddrCopy) {
		MSG_DEBUG("pszAddrCopy is NULL, mem alloc failed");
		return false;
	}

	strncpy(pszAddrCopy, pszAddr, strLen);

	pszAddr[0] = 0;
	pszStrStart = pszAddrCopy;

	while (true) {
		char*	pszStrEnd = NULL;
		int	addressLen = 0;

		if (MmsAddrUtilCheckEmailAddress(pszAddrCopy))
			pszStrEnd = strstr(pszStrStart, "/TYPE=PLMN");
		else
			pszStrEnd = strstr(pszStrStart, "/");

		if (!pszStrEnd) {
			char *pszStart = NULL;
			char *pszEnd = NULL;
			/* "/TYPE=PLMN" not found */

			int remainedLen = strlen(pszStrStart);

			if (remainedLen <= 0)
				break;

			/* Email address can occur with Sender Name<email-address> format */
			/* remove the Sender name and only retain the email address. */
			pszStart = strstr(pszStrStart, "<");
			if (pszStart) {
				pszEnd = strstr(pszStrStart, ">");

				if (pszEnd) {
					pszStart++; /* skip "<" */
					g_strlcat(pszAddr, pszStart, pszEnd - pszStart + 1);
					break;
				}
			}

			g_strlcat(pszAddr, pszStrStart, strLen + 1);
			break;
		}

		/* Get one address length */
		addressLen = pszStrEnd - pszStrStart;

		strncat(pszAddr, pszStrStart, addressLen);

		/* Find next address */
		pszStrStart = pszStrEnd;

		pszStrTemp = strstr(pszStrStart, MSG_MMS_STR_ADDR_DELIMETER);

		if (pszStrTemp) {
			addressLen = pszStrTemp - pszStrEnd;
			pszStrStart += addressLen;
		} else {
			pszStrStart += strlen(pszStrEnd);
		}

		if (pszStrStart[0] == 0)	/* end of string */
			break;


		g_strlcat(pszAddr, MSG_MMS_STR_ADDR_DELIMETER, strLen + 1);	/* add ';' */
		pszStrStart++;	/* remove ';' */
	}

	if (pszAddr[0] == 0)
		strncpy(pszAddr, pszAddrCopy, strLen);

	free(pszAddrCopy);

	return true;
}

static int __MsgCutUTFString(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar)
{
	unsigned char *org;

	MSG_DEBUG("---------------");

	org = des;
	outBufSize--;			/* NULL character */

	while ((nChar > 0) && (*szSrc != '\0')) {
		if (*szSrc < 0x80) {
			outBufSize --;
			if (outBufSize < 0)
				goto __RETURN;

			*des = *szSrc;
			des++;
			szSrc++;
		} else if  (((0xC0 <= *szSrc) && (*szSrc < 0xE0)) && (*(szSrc+1) >= 0x80)) {
			outBufSize -= 2;
			if (outBufSize < 0)
				goto __RETURN;

			*des = *szSrc;
			*(des + 1) = *(szSrc + 1);

			des += 2;
			szSrc += 2;
		} else if  ((*szSrc >= 0xE0) && (*(szSrc+1) >= 0x80) && (*(szSrc+2) >= 0x80)) {
			outBufSize -= 3;
			if (outBufSize < 0)
				goto __RETURN;

			*des = *szSrc;
			*(des + 1) = *(szSrc + 1);
			*(des + 2) = *(szSrc + 2);

			des += 3;
			szSrc += 3;
		} else {
			outBufSize --;
			if (outBufSize < 0)
				goto __RETURN;

			*des = *szSrc;
			des++;
			szSrc++;
			MSG_DEBUG("utf8 incorrect range!");
		}

		nChar--;
	}

__RETURN:

	*des = 0;
	return (des - org);
}

static void __MsgMIMERemoveQuote(char *szSrc)
{
	int length = 0;

	length = strlen(szSrc);
	if (szSrc[0] == MSG_CH_QUOT && szSrc[length-1] == MSG_CH_QUOT) {
		int index = 0;

		for (index = 0; index < length-2; index++)
			szSrc[index] = szSrc[index+1];
		szSrc[index] = '\0';
	}
}

static bool __MsgLoadDataToDecodeBuffer(FILE *pFile, char **ppBuf, int *pPtr, int *pOffset, char *pInBuf1, char *pInBuf2, int maxLen, int *pBufLen, int endOfFile)
{
	MSG_BEGIN();
	int nRead = 0;
	int length= 0;

	if (pFile == NULL) {
		MSG_DEBUG("Error");

		*pBufLen = 0;
		return false;
	}

	if (pPtr == NULL || pInBuf1 == NULL || pInBuf2 == NULL) {
		MSG_DEBUG("Error");

		*pBufLen = 0;
		return false;
	}

	if (*pBufLen == 0) {
		length = maxLen - (*pPtr);
	} else {
		length = (*pBufLen) - (*pPtr);
	}

	if (length < 0)
		length = 0;

	if ((*ppBuf) == NULL) {
		memset(pInBuf1, 0, maxLen);
		(*ppBuf) = pInBuf1;
	} else if ((*ppBuf) == pInBuf1) {
		memset(pInBuf2, 0, maxLen);
		if (length)
			memcpy(pInBuf2, pInBuf1 + (*pPtr), length);
		(*ppBuf) = pInBuf2;
	} else {
		memset(pInBuf1, 0, maxLen);
		if (length)
			memcpy(pInBuf1, pInBuf2 + (*pPtr), length);
		(*ppBuf) = pInBuf1;
	}

	(*pPtr) = 0;

	if (*pOffset == endOfFile) {
		*pBufLen = length;
		return true;
	}

	if (maxLen == length) {
		/* (*pPtr) was 0 */
		if (MsgReadFileForDecode(pFile, (*ppBuf), maxLen, &nRead) == false)
			return false;

		*pBufLen = nRead;
	} else {
		if (MsgReadFileForDecode(pFile, (*ppBuf) + length, maxLen - length, &nRead) == false)
			return false;

		*pBufLen = length + nRead;
	}

	if ((*pOffset = MsgFtell(pFile)) == -1L) {
		MSG_DEBUG("MsgFtell Error");
		return false;
	}

	MSG_END();

	return true;
}

/*
 *	This function write media data from raw data to file.
 *	@param	pMsg
 *	@param	pPartBody
 *	@param	pszMailboxPath	: path of mailbox
 *	@param	pszMsgFilename	: name of msg file
 *	@param	index			: used for file naming
 *	@param	bSave			: if true, file will be save otherwise just filename will be stored.
 */
static bool __MmsMultipartSaveAsTempFile(MsgType *pPartType, MsgBody *pPartBody, char *pszMailboxPath, char *pszMsgFilename, int index, bool bSave)
{
	FILE *pFile = NULL;
	char szFileName[MSG_FILENAME_LEN_MAX+1] = {0, };	/* file name of temp file */
	char szFullPath[MSG_FILEPATH_LEN_MAX] = {0, }; /* full absolute path of temp file. */
	bool bFileExist = false;
	MSG_BEGIN();

	if (!pPartType) {
		MSG_DEBUG("pPartType is NULL");
		return false;
	}

	if (pPartType->type == MIME_APPLICATION_SMIL) {
		snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%s", "smil.txt");
	} else {

		if (pPartType->param.szName[0] != '\0') {
			snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%s", pPartType->param.szName);
		} else if (pPartType->param.szFileName[0] != '\0') {
			snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%s", pPartType->param.szFileName);
		} else if (pPartType->szContentLocation[0] != '\0') {
			snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%s", pPartType->szContentLocation);
		} else {
			snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%lu", (unsigned long)index);
		}
	}

	/* make full path for save */
	__MsgMakeFileName(pPartType->type, szFileName, pPartType->drmInfo.drmType, 0, szFileName, sizeof(szFileName));	/* FL & CD -> extension(.dm) SD -> extension(.dcf) */

	snprintf(szFullPath, MSG_FILEPATH_LEN_MAX, "%s%s.dir/%s", pszMailboxPath, pszMsgFilename, szFileName);	/* get absolute path of each temp file of each part */

	if (pPartType->type == MIME_APPLICATION_OCTET_STREAM)
		MsgGetMimeTypeFromFileName(MIME_MAINTYPE_UNKNOWN, szFullPath, (MimeType *)&pPartType->type, NULL);

	/* save file */
	bFileExist = MsgAccessFile(szFullPath, F_OK);

	MSG_SEC_DEBUG("save flag  [%d],  filepath [%s], file exist [%d]", bSave, szFullPath, bFileExist);

	if (bSave == true && bFileExist == false) {

		if ((pFile = MsgOpenFile(szFullPath, "wb+")) == NULL) {
			MSG_DEBUG("MsgOpenFile failed");
			goto __CATCH;
		}

		if (__MmsGetMediaPartData(pPartType, pPartBody, pFile) == false) {
			MSG_DEBUG("MmsGetMediaPartData fail [index:%d]\n", index);
			goto __CATCH;
		}

		MsgCloseFile(pFile);
		pFile = NULL;

		snprintf(pPartBody->szOrgFilePath, sizeof(pPartBody->szOrgFilePath), "%s", szFullPath);

		/* IF DRM type Convert to dcf */
		if (pPartType->type == MIME_APPLICATION_VND_OMA_DRM_MESSAGE
			|| pPartType->type == MIME_APPLICATION_VND_OMA_DRM_CONTENT)
		{
			char destDrmPath[MSG_FILEPATH_LEN_MAX] = {0,};

			if (MsgDrmConvertDmtoDcfType(pPartBody->szOrgFilePath, destDrmPath) == true) {
				MSG_INFO("Success Convert to Dcf");

				bFileExist = MsgAccessFile(destDrmPath, F_OK);

				if (bFileExist) {
					snprintf(pPartBody->szOrgFilePath, sizeof(pPartBody->szOrgFilePath), "%s", destDrmPath);
					MsgGetFileName(pPartBody->szOrgFilePath, szFileName, MSG_FILENAME_LEN_MAX);
				}

			} else {
				MSG_INFO("Fail Convert to Dcf");
			}

			if (MsgDrmIsDrmFile(pPartBody->szOrgFilePath) == true)
				MmsPluginDrmGetInfo(pPartBody->szOrgFilePath, pPartType);

			MSG_SEC_INFO("Drm File Path [%s] isdrm [%d]", pPartBody->szOrgFilePath, MsgDrmIsDrmFile(pPartBody->szOrgFilePath));
		}

		pPartBody->offset = 0;
		pPartBody->size = MsgGetFileSize(pPartBody->szOrgFilePath);

		if (pPartType->drmInfo.drmType != MSG_DRM_TYPE_NONE) {
			MsgDrmRegisterFile(MSG_MODE_FILE, pPartBody->szOrgFilePath, strlen(pPartBody->szOrgFilePath));

			/* change szDrm2FullPath as current content path*/
			if (pPartType->drmInfo.szDrm2FullPath) {
				free(pPartType->drmInfo.szDrm2FullPath);
				pPartType->drmInfo.szDrm2FullPath = g_strdup(pPartBody->szOrgFilePath);
			}
		}

		MSG_SEC_DEBUG("Save Part File to [%s]", pPartBody->szOrgFilePath);

	} else {
		snprintf(pPartBody->szOrgFilePath, sizeof(pPartBody->szOrgFilePath), "%s", szFullPath);

		/* IF DRM type check dcf exist */
		if (pPartType->type == MIME_APPLICATION_VND_OMA_DRM_MESSAGE
			|| pPartType->type == MIME_APPLICATION_VND_OMA_DRM_CONTENT)
		{
			char destDrmPath[MSG_FILEPATH_LEN_MAX] = {0,};

			MsgGetFileNameWithoutExtension(destDrmPath, pPartBody->szOrgFilePath);
			strncat(destDrmPath, ".dcf", 4);

			bFileExist = MsgAccessFile(destDrmPath, F_OK);

			if (bFileExist) {
				snprintf(pPartBody->szOrgFilePath, sizeof(pPartBody->szOrgFilePath), "%s", destDrmPath);
				MsgGetFileName(pPartBody->szOrgFilePath, szFileName, MSG_FILENAME_LEN_MAX);
			}

			if (MsgDrmIsDrmFile(pPartBody->szOrgFilePath) == true)
				MmsPluginDrmGetInfo(pPartBody->szOrgFilePath, pPartType);
		}

		pPartBody->offset = 0;
		pPartBody->size = MsgGetFileSize(pPartBody->szOrgFilePath);

		MSG_SEC_DEBUG("Set Part File to [%s]", pPartBody->szOrgFilePath);
	}

	/* file name fix */
	if (szFileName[0]  != '\0') {
		snprintf(pPartType->param.szFileName, MSG_FILENAME_LEN_MAX+1, "%s.dir/%s", pszMsgFilename, szFileName); /* store relative path of each temp file of each part including sub folder. */
		snprintf(pPartType->param.szName, MSG_LOCALE_FILENAME_LEN_MAX+1, "%s", szFileName);
		MSG_SEC_DEBUG("Set Name : %s", pPartType->param.szName);
	}

	MSG_END();
	return true;

__CATCH:

	if (pFile != NULL) {
		MsgCloseFile(pFile);
		pFile = NULL;
	}
	MSG_END();
	return false;
}

bool __MmsGetMediaPartData(MsgType *pPartType, MsgBody *pPartBody, FILE* pFile)
{
	int nRead = 0;
	int nRead2 = 0;
	char *pData = NULL;
	char *pNewData = NULL;
	char *pTempData = NULL;
	int msgEncodingValue = 0;
	int msgTypeValue = 0;
	int msgCharsetValue	= 0;

	int offset = 0;
	int size = 0;

	msgEncodingValue = pPartType->encoding;
	msgTypeValue = pPartType->type;
	msgCharsetValue = pPartType->param.charset;

	offset = pPartBody->offset;
	size = pPartBody->size;

	if (pPartBody->szOrgFilePath[0]) {
		pTempData = MsgOpenAndReadMmsFile(pPartBody->szOrgFilePath, offset, size, &nRead);

		if (pTempData == NULL) {
			MSG_DEBUG("pTempData read fail");
			goto __CATCH;
		}

		pData = pTempData;
	} else if (pPartBody->body.pText) {
		pData = pPartBody->body.pText;
		nRead = pPartBody->size;
	}

	if (pData == NULL) {
		MSG_DEBUG("there is no data");
		goto __RETURN;
	}

	pNewData = __MmsGetBinaryUTF8Data(pData, nRead, msgEncodingValue, msgTypeValue, msgCharsetValue, &nRead2);

	if (pNewData) {
		pPartType->encoding = MSG_ENCODING_BINARY;

		if (MmsIsTextType(msgTypeValue))
			pPartType->param.charset = MSG_CHARSET_UTF8;

		if (MsgWriteFile(pNewData, sizeof(char), nRead2,  pFile) != (size_t)nRead2) {
			MSG_DEBUG("file writing fail");
			goto __CATCH;
		}

	} else {
		if (MsgWriteFile(pData, sizeof(char), nRead,  pFile) != (size_t)nRead) {
			MSG_DEBUG("file writing fail");
			goto __CATCH;
		}
	}

__RETURN:

	if (pNewData) {
		free(pNewData);
		pNewData = NULL;
	}

	if (pTempData) {
		free(pTempData);
		pTempData = NULL;
	}

	return true;

__CATCH:

	if (pNewData) {
		free(pNewData);
		pNewData = NULL;
	}

	if (pTempData) {
		free(pTempData);
		pTempData = NULL;
	}

	return false;
}

char *__MmsGetBinaryUTF8Data(char *pData, int nRead, int msgEncodingValue, int msgTypeValue, int msgCharsetValue, int *npRead)
{
	int nByte = 0;
	int nTemp = 0;
	char *pTemp = NULL;

	char *pConvertedStr	= NULL;
	char *pConvertedData = NULL;
	char *pNewData = NULL;
	char *pReturnData = NULL;

	const char *pToCodeSet = "UTF-8";
	const char *pFromCodeSet = NULL;

	switch (msgEncodingValue) {
	case MSG_ENCODING_BASE64:

		pConvertedData = (char*)MsgDecodeBase64((UCHAR*)pData, (ULONG)nRead, (ULONG*)&nByte);
		MSG_DEBUG("MSG_ENCODING_BASE64 bodyLength [%d]", nByte);

		pTemp = pConvertedData;
		nTemp = nByte;

		break;

	case MSG_ENCODING_QUOTE_PRINTABLE:

		pConvertedData = (char*)MsgDecodeQuotePrintable((UCHAR*)pData, (ULONG)nRead, (ULONG*)&nByte);
		MSG_DEBUG("MSG_ENCODING_QUOTE_PRINTABLE bodyLength [%d]", nByte);

		pTemp = pConvertedData;
		nTemp = nByte;

		break;

	default:

		MSG_DEBUG("encoding val [%d] bodyLength [%d]", msgEncodingValue, nRead);
		pTemp = pData;
		nTemp = nRead;

		break;
	}

	if (MmsIsTextType(msgTypeValue)) {

		if (msgCharsetValue == MSG_CHARSET_US_ASCII) {
			pNewData = pTemp;
			*npRead = nTemp;
		} else if (msgCharsetValue == MSG_CHARSET_UTF8) {

			/* skip BOM (Byte Order Mark) bytes .. (Please refer to the http://www.unicode.org/faq/utf_bom.html#BOM) */
			if (nTemp >= 3) {
				if (((UINT8)pTemp[0]) == 0xEF && ((UINT8)pTemp[1]) == 0xBB && ((UINT8)pTemp[2]) == 0xBF) {
					pTemp += 3;
					nTemp -= 3;
				}
			}

			pNewData = pTemp;
			*npRead = nTemp;
		} else {

			UINT16 MIBenum = MmsGetBinaryValue(MmsCodeCharSet, msgCharsetValue);

			pFromCodeSet = MmsGetTextByCode(MmsCodeCharSet, MIBenum);

			MSG_DEBUG("char set enum = [%d], MIBenum = [%d], str = [%s]", msgCharsetValue, MIBenum, pFromCodeSet);

			if (pFromCodeSet) {
				MSG_DEBUG("Convert to UTF-8");

				if (MmsPluginTextConvert(pToCodeSet, pFromCodeSet, pTemp, nTemp, &pConvertedStr, npRead) == true) {
					pNewData = pConvertedStr;
				} else {
					MSG_DEBUG("Failed MmsPluginTextConvert");
					pNewData = pTemp;
					*npRead = nTemp;
				}

			} else { /* unsupported charset */
				MSG_DEBUG("unsupported charset");
				pNewData = pTemp;
				*npRead = nTemp;
			}
		}

	} else {
		pNewData = pTemp;
		*npRead = nTemp;
	}

	pReturnData = (char *)calloc(1, *npRead);
	if (pReturnData == NULL) {
		MSG_DEBUG("pReturnData alloc fail.");
		goto __CATCH;
	}

	if (pNewData != NULL) {
		memset(pReturnData, 0, *npRead);
		memcpy(pReturnData, pNewData, *npRead);
	}

	if (pConvertedData) {
		free(pConvertedData);
		pConvertedData = NULL;
	}

	if (pConvertedStr) {
		free(pConvertedStr);
		pConvertedStr = NULL;
	}

	return pReturnData;

__CATCH:

	if (pConvertedData) {
		free(pConvertedData);
		pConvertedData = NULL;
	}

	if (pConvertedStr) {
		free(pConvertedStr);
		pConvertedStr = NULL;
	}

	return NULL;
}

static bool __MsgMakeFileName(int iMsgType, char *szFileName, MsgDrmType drmType, int nUntitleIndex, char *outBuf, int outBufLen)
{
	char szTemp[MSG_FILENAME_LEN_MAX+1]={0,};
	char szTempFileName[MSG_FILENAME_LEN_MAX+1]={0,};
	const char *pExt = NULL;

	MSG_SEC_DEBUG("Input : type  [0x%x], drmType [%d], filename [%s]", iMsgType, drmType, szFileName);

	if (szFileName == NULL)
		return false;

	/* Filename */
	int inp_len = strlen(szFileName);
	if (inp_len > 0) {

		pExt = strrchr(szFileName, '.');

		if (pExt != NULL && *(pExt + 1) != '\0') {
			pExt = pExt +1;
		} else {
			pExt = NULL;
		}

		MsgGetFileNameWithoutExtension(szTempFileName, szFileName);
	} else {
		if (nUntitleIndex >= 1) {
			snprintf(szTempFileName, sizeof(szTempFileName), "%s_%d", "untitled", nUntitleIndex);
		} else {
			snprintf(szTempFileName, sizeof(szTempFileName), "%s", "untitled");
		}
	}

	/* extension */
	if (iMsgType == MIME_APPLICATION_VND_OMA_DRM_MESSAGE)
		pExt = "dm";
	else if (iMsgType == MIME_APPLICATION_VND_OMA_DRM_CONTENT)
		pExt = "dcf";

	if (pExt == NULL) { /* find ext from content type */

		if (iMsgType == MIME_APPLICATION_OCTET_STREAM || iMsgType == MIME_UNKNOWN) {
			MSG_DEBUG("unsupported MsgType [%d]", iMsgType);
			goto __CATCH;
		}

		pExt = MimeGetExtFromMimeInt((MimeType)iMsgType);
	}

	/* Filename + extension */
	if (pExt) {
		snprintf(szTemp, sizeof(szTemp), "%s.%s", szTempFileName, pExt);
	} else {
		MSG_DEBUG("Failed to get extension of that mime data file.");
		goto __CATCH;
	}

	snprintf(outBuf, outBufLen, "%s", szTemp);
	MSG_SEC_DEBUG("Result : filename [%s]", outBuf);
	return true;

__CATCH:
	return false;
}

bool MsgGetFileNameWithoutExtension (char *szOutputName, char *szName)
{
	char *pszExt = NULL;

	if (szOutputName == NULL) {
		MSG_DEBUG("szOutputName is NULL");
		return false;
	}

	strncpy(szOutputName, szName, strlen(szName));

	if ((pszExt = strrchr(szOutputName, '.'))) {
		if (pszExt[0] == '.')
			pszExt[0] = '\0';
	}

	return true;
}

bool MsgGetFileName(char *szFilePath, char *szFileName, int size)
{
	char *filename = NULL;
	if (szFilePath) {
		filename = strrchr(szFilePath, '/');
		if (filename != NULL) {
			snprintf(szFileName, size, "%s", filename + 1);
		} else {
			snprintf(szFileName, size, "%s", szFilePath);
		}
	} else {
		return false;
	}

	return true;
}

bool MmsGetMediaPartHeader(int index, MsgType *pHeader)
{
	MmsMsg *pMsg = NULL;
	MsgMultipart *pPart = NULL;

	if (pHeader == NULL) {
		MSG_DEBUG("Invalid pHeader input. It's null");
		return false;
	}

	MmsPluginStorage::instance()->getMmsMessage(&pMsg);

	MmsInitMsgType(pHeader);


	/* Requires header of non-presentation */
	if (MsgIsMultipart(pMsg->msgType.type)) {
		MSG_DEBUG("Multipart header [index = %d] \n", index);

		pPart = pMsg->msgBody.body.pMultipart;

		while (pPart && index--)
			pPart = pPart->pNext;

		if (pPart == NULL) {
			MSG_DEBUG("There is no such msg part.");
			return false;
		}

		memcpy(pHeader, &pPart->type, sizeof(MsgType));
	} else {
		MSG_DEBUG("Requires singlepart header");
		memcpy(pHeader, &pMsg->msgType, sizeof(MsgType));
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////

MmsPluginDecoder *MmsPluginDecoder::pInstance = NULL;

MmsPluginDecoder *MmsPluginDecoder::instance()
{
	if (!MmsPluginDecoder::pInstance)
		MmsPluginDecoder::pInstance = new MmsPluginDecoder();

	return MmsPluginDecoder::pInstance;
}

MmsPluginDecoder::MmsPluginDecoder(){}
MmsPluginDecoder::~MmsPluginDecoder(){}

void MmsPluginDecoder::decodeMmsPdu(MmsMsg *pMsg, msg_message_id_t msgID, const char *pduFilePath)
{
	MSG_BEGIN();

	FILE *pFile	= NULL;
	MsgMultipart *pMultipart = NULL;
	int nSize = 0;
	char szFullPath[MSG_FILEPATH_LEN_MAX] = {0, };
	char szTempMediaDir[MSG_FILEPATH_LEN_MAX] = {0, };

	MmsInitHeader();

	pMsg->msgID = msgID;

	snprintf(szFullPath, sizeof(szFullPath), "%s", pduFilePath);

	MsgGetFileName(szFullPath, pMsg->szFileName, sizeof(pMsg->szFileName));

	if (MsgGetFileSize(szFullPath, &nSize) == false) {
		MSG_FATAL("Fail MsgGetFileSize");
		goto __CATCH;
	}

	pFile = MsgOpenFile(szFullPath, "rb");
	if (pFile == NULL) {
		MSG_SEC_DEBUG("Fail MsgOpenFile [%s]", szFullPath);
		goto __CATCH;
	}

	MmsRegisterDecodeBuffer();

	if (MmsBinaryDecodeMsgHeader(pFile, nSize) == false) {
		MSG_FATAL("Fail to MmsBinaryDecodeMsgHeader");
		goto __CATCH;
	}

	if (MmsBinaryDecodeMsgBody(pFile, szFullPath, nSize) == false) {
		MSG_FATAL("Fail to MmsBinaryDecodeMsgBody");
		goto __CATCH;
	}

	/* Set mmsHeader.msgType & msgBody to pMsg ----------- */

	memcpy(&(pMsg->msgType), &(mmsHeader.msgType), sizeof(MsgType));
	memcpy(&(pMsg->msgBody), &(mmsHeader.msgBody), sizeof(MsgBody));

{ /* attribute convert mmsHeader -> mmsAttribute */

	pMsg->mmsAttrib.contentType = (MimeType)mmsHeader.msgType.type;

	pMsg->mmsAttrib.date = mmsHeader.date;

	if (mmsHeader.deliveryReport == MMS_REPORT_YES) {
		pMsg->mmsAttrib.bAskDeliveryReport = true;
	}

	memcpy(&pMsg->mmsAttrib.deliveryTime, &mmsHeader.deliveryTime, sizeof(MmsTimeStruct));

	memcpy(&pMsg->mmsAttrib.expiryTime, &mmsHeader.expiryTime, sizeof(MmsTimeStruct));

	MSG_DEBUG("@@@@@pMsg->mmsAttrib.deliveryTime=[%d]", pMsg->mmsAttrib.deliveryTime);

	pMsg->mmsAttrib.msgClass = mmsHeader.msgClass;

	snprintf(pMsg->szMsgID, sizeof(pMsg->szMsgID), "%s", mmsHeader.szMsgID);

	pMsg->mmsAttrib.msgType = mmsHeader.type;

	pMsg->mmsAttrib.version = mmsHeader.version;

	pMsg->mmsAttrib.msgSize = mmsHeader.msgSize;

	pMsg->mmsAttrib.priority = mmsHeader.priority;

	if (mmsHeader.readReply == MMS_REPORT_YES) {
		pMsg->mmsAttrib.bAskReadReply = true;
	}

	snprintf(pMsg->mmsAttrib.szSubject, sizeof(pMsg->mmsAttrib.szSubject), "%s", mmsHeader.szSubject);

	snprintf(pMsg->szTrID, sizeof(pMsg->szTrID), "%s", mmsHeader.szTrID);

	pMsg->mmsAttrib.retrieveStatus = mmsHeader.retrieveStatus;

	/* FIXME:: mmsHeader will release after delete global mmsHeader */
	/* memset(&(mmsHeader.msgBody), 0x00, sizeof(MsgBody)); */ /* After copy to MmsMsg */
}
	if (pMsg->msgBody.pPresentationBody) {
		if (MsgFseek(pFile, pMsg->msgBody.pPresentationBody->offset, SEEK_SET) < 0)
			goto __CATCH;

		pMsg->msgBody.pPresentationBody->body.pText = (char *)calloc(1, pMsg->msgBody.pPresentationBody->size + 1);
		if (pMsg->msgBody.pPresentationBody->body.pText == NULL)
			goto __CATCH;

		memset(pMsg->msgBody.pPresentationBody->body.pText, 0, pMsg->msgBody.pPresentationBody->size + 1);

		ULONG nRead = 0;
		nRead = MsgReadFile(pMsg->msgBody.pPresentationBody->body.pText, sizeof(char), pMsg->msgBody.pPresentationBody->size, pFile);
		if (nRead == 0)
			goto __CATCH;

	}

	MsgCloseFile(pFile);
	pFile = NULL;
	/* nPartCount */
	pMsg->nPartCount = 0;

	if (MsgIsMultipart(mmsHeader.msgType.type) == true) {
		pMultipart = pMsg->msgBody.body.pMultipart;
		while (pMultipart) {
			pMsg->nPartCount++;
			pMultipart = pMultipart->pNext;
		}
	} else {
		if (pMsg->msgBody.size > 0)
			pMsg->nPartCount++;
	}

	/* 	make temporary	*/
	snprintf(szTempMediaDir, MSG_FILEPATH_LEN_MAX, "%s%s.dir", MSG_DATA_PATH, pMsg->szFileName);

	if (MsgIsMultipart(pMsg->msgType.type) == true) {
		int partIndex = 0;
		pMultipart = pMsg->msgBody.body.pMultipart;

		if (mkdir(szTempMediaDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
			if (errno == EEXIST) {
				MSG_SEC_DEBUG("exist dir : [%s]", szTempMediaDir);
			} else {
				MSG_SEC_DEBUG("Fail to Create Dir [%s]", szTempMediaDir);
				goto __CATCH;
			}
		} else {
			MSG_SEC_DEBUG("make dir : [%s]", szTempMediaDir);
		}

		if (pMsg->msgBody.pPresentationBody) {
			if (__MmsMultipartSaveAsTempFile(&pMsg->msgBody.presentationType, pMsg->msgBody.pPresentationBody,
												(char*)MSG_DATA_PATH, pMsg->szFileName, 0, true) == false)
				goto __CATCH;
		}

		while (pMultipart) {

			if (__MmsMultipartSaveAsTempFile(&pMultipart->type, pMultipart->pBody,
											(char*)MSG_DATA_PATH, pMsg->szFileName, partIndex, true) == false)
				goto __CATCH;

			MmsPrintMulitpart(pMultipart, partIndex);

			pMultipart = pMultipart->pNext;
			partIndex ++;
		}

	} else { /* single part */
		if (pMsg->nPartCount > 0) {

			if (mkdir(szTempMediaDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
				if (errno == EEXIST) {
					MSG_DEBUG("exist dir : [%s]", szTempMediaDir);
				} else {
					MSG_DEBUG("Fail to Create Dir [%s]", szTempMediaDir);
					goto __CATCH;
				}
			} else {
				MSG_DEBUG("make dir : [%s]", szTempMediaDir);
			}

			if (__MmsMultipartSaveAsTempFile(&pMsg->msgType, &pMsg->msgBody,
											(char*)MSG_DATA_PATH, pMsg->szFileName, 0, true) == false)
				goto __CATCH;
		}
	}
	MSG_DEBUG("### Success ###");
	MSG_END();
	return;

__CATCH:

	MmsInitHeader();
	MmsUnregisterDecodeBuffer();

	if (pFile != NULL) {
		MsgCloseFile(pFile);
		pFile = NULL;
	}


	MmsReleaseMsgDRMInfo(&pMsg->msgType.drmInfo);

	MmsReleaseMsgBody(&pMsg->msgBody, pMsg->msgType.type);

	MSG_DEBUG("### Fail ###");
	MSG_END();
	return;

}

/* CID 41989: Removed function decodeMmsPdu which is unused. */
#if 0
void MmsPluginDecoder::decodeMmsPdu(MMS_DATA_S *pMmsData, const char *pduFilePath)
{
	MSG_BEGIN();

	FILE *pFile	= NULL;
	MsgMultipart * iter_multipart = NULL;
	int nSize = 0;
	char szFullPath[MSG_FILEPATH_LEN_MAX] = {0, };

	MmsInitHeader();

	//pMsg->msgID = msgID;

	snprintf(szFullPath, sizeof(szFullPath), "%s", pduFilePath);

	//MsgGetFileName(szFullPath, pMsg->szFileName, sizeof(pMsg->szFileName));

	if (MsgGetFileSize(szFullPath, &nSize) == false) {
		MSG_FATAL("Fail MsgGetFileSize");
		goto __CATCH;
	}

	pFile = MsgOpenFile(szFullPath, "rb");
	if (pFile == NULL) {
		MSG_SEC_DEBUG("Fail MsgOpenFile [%s]", szFullPath);
		goto __CATCH;
	}

	MmsRegisterDecodeBuffer();

	if (MmsBinaryDecodeMsgHeader(pFile, nSize) == false) {
		MSG_FATAL("Fail to MmsBinaryDecodeMsgHeader");
		goto __CATCH;
	}

	if (MmsBinaryDecodeMsgBody(pFile, szFullPath, nSize) == false) {
		MSG_FATAL("Fail to MmsBinaryDecodeMsgBody");
		goto __CATCH;
	}

	//set header
	if (pMmsData->header == NULL) {
		pMmsData->header = MsgMmsCreateHeader();
	}

	pMmsData->header->messageType = mmsHeader.type;

	pMmsData->header->mmsVersion = mmsHeader.version;

	pMmsData->header->contentType = mmsHeader.msgType.type;

	pMmsData->header->date = mmsHeader.date;

	pMmsData->header->messageSize  = mmsHeader.msgSize;

	pMmsData->header->mmsPriority = mmsHeader.priority;

	pMmsData->header->messageClass = mmsHeader.msgClass;

	if (mmsHeader.deliveryReport == MMS_REPORT_YES) {
		pMmsData->header->bDeliveryReport = true;
	}

	if (mmsHeader.readReply == MMS_REPORT_YES) {
		pMmsData->header->bReadReport = true;
	}

	memcpy(&pMmsData->header->delivery, &mmsHeader.deliveryTime, sizeof(MmsTimeStruct));

	memcpy(&pMmsData->header->expiry, &mmsHeader.expiryTime, sizeof(MmsTimeStruct));


	snprintf(pMmsData->header->messageID, sizeof(pMmsData->header->messageID), "%s", mmsHeader.szMsgID);

	snprintf(pMmsData->header->szSubject, sizeof(pMmsData->header->szSubject), "%s", mmsHeader.szSubject);

	snprintf(pMmsData->header->trID, sizeof(pMmsData->header->trID), "%s", mmsHeader.szTrID);

	//CID 41989: Moving assignment of iter_multipart before its de-referencing.
	iter_multipart = mmsHeader.msgBody.body.pMultipart;
	//set multipart
	if (pMmsData->header->contentType == MIME_MULTIPART_RELATED || pMmsData->header->contentType == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {

		MMS_MULTIPART_DATA_S *pMultipart = MsgMmsCreateMultipart();

		pMultipart->type = MIME_APPLICATION_SMIL;
		snprintf(pMultipart->szContentType, sizeof(pMultipart->szContentType), "%s", "application/smil");
		snprintf(pMultipart->szContentID, sizeof(pMultipart->szContentID), "%s", mmsHeader.msgBody.presentationType.szContentID);
		snprintf(pMultipart->szContentLocation, sizeof(pMultipart->szContentLocation), "%s", mmsHeader.msgBody.presentationType.szContentLocation);
		snprintf(pMultipart->szFileName, sizeof(pMultipart->szFileName), "%s", mmsHeader.msgBody.presentationType.param.szName);

		//snprintf(pMultipart->szFilePath, sizeof(pMultipart->szFilePath), MSG_DATA_PATH"%s", mmsHeader.msgBody.presentationType.param.szFileName);

		MsgBody *pBody = iter_multipart->pBody;
		pMultipart->pMultipartData = MsgOpenAndReadMmsFile(pBody->szOrgFilePath, pBody->offset, pBody->size, (int*)&pMultipart->nMultipartDataLen);

		pMmsData->smil = pMultipart;
	}


	while (iter_multipart) {

		MMS_MULTIPART_DATA_S *pMultipart = MsgMmsCreateMultipart();

		pMultipart->type = (MimeType)iter_multipart->type.type;

		snprintf(pMultipart->szContentType, sizeof(pMultipart->szContentType), "%s",MimeGetMimeStringFromMimeInt(iter_multipart->type.type));
		snprintf(pMultipart->szContentID, sizeof(pMultipart->szContentID), "%s", iter_multipart->type.szContentID);
		snprintf(pMultipart->szContentLocation, sizeof(pMultipart->szContentLocation), "%s", iter_multipart->type.szContentLocation);
		snprintf(pMultipart->szFileName, sizeof(pMultipart->szFileName), "%s", iter_multipart->type.param.szName);

		//snprintf(pMultipart->szFilePath, sizeof(pMultipart->szFilePath), "%s", iter_multipart->pBody->szOrgFilePath);

		MsgBody *pBody = iter_multipart->pBody;
		pMultipart->pMultipartData = MsgOpenAndReadMmsFile(pBody->szOrgFilePath, pBody->offset, pBody->size, (int*)&pMultipart->nMultipartDataLen);


#ifdef __SUPPORT_DRM__
		if (iter_multipart->type.drmInfo.drmType != MSG_DRM_TYPE_NONE) {
			pMultipart->drmType = iter_multipart->type.drmInfo.drmType;
		}
#endif

		pMmsData->multipartlist = g_list_append(pMmsData->multipartlist, pMultipart);
		iter_multipart = iter_multipart->pNext;
	}


	MSG_DEBUG("### SUCCESS ###");
	MSG_END();
	return;
__CATCH:

	MmsInitHeader();
	MmsUnregisterDecodeBuffer();

	if (pFile != NULL) {
		MsgCloseFile(pFile);
		pFile = NULL;
	}

	MSG_DEBUG("### Fail ###");
	MSG_END();
	return;
}
#endif
