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

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#include "MsgCppTypes.h"
#include "MsgUtilFile.h"
#include "MsgUtilMime.h"

#include "MmsPluginDebug.h"
#include "MmsPluginEncode.h"
#include "MmsPluginCodecTypes.h"
#include "MmsPluginCodecCommon.h"
#include "MmsPluginUtil.h"

using namespace std;

/**  Sending message related variables  ------------------------ */
static char gszMmsEncodeBuf[MSG_MMS_ENCODE_BUFFER_MAX] = {0, };
static int gCurMmsEncodeBuffPos = 0;	/* number of characters on gpMmsEncodeBuf */
static int	gMmsEncodeMaxLen = 0;
static int	gMmsEncodeCurOffset = 0;	/* offset in file */
static char *gpMmsEncodeBuf = NULL;

/* Acknowledge.ind & NotifyResp.ind related variables ------------------------ */
static char gszMmsEncodeBuf2[MSG_MMS_ENCODE_BUFFER_MAX] = {0, };
static int	gCurMmsEncodeBuffPos2 = 0;	/* number of characters on gpMmsEncodeBuf */
static int	gMmsEncodeMaxLen2 = 0;
static int	gMmsEncodeCurOffset2 = 0;	/* offset in file */
static char *gpMmsEncodeBuf2 = NULL;

static int __MmsGetEncodeOffset(void);
static void __MmsRegisterEncodeBuffer2(char *pInBuff, int maxLen);
static void __MmsUnregisterEncodeBuffer2(void);

static int __MmsBinaryEncodeUintvarLen(UINT32 integer);
static bool __MmsBinaryEncodeUintvar(FILE *pFile, UINT32 integer, int length);
static int __MmsBinaryEncodeValueLengthLen(UINT32 integer);
static bool __MmsBinaryEncodeValueLength(FILE *pFile, UINT32 integer, int length);
static int	__MmsBinaryEncodeIntegerLen(UINT32 integer);
static bool __MmsBinaryEncodeInteger(FILE *pFile, UINT32 integer, int length);
static int	__MmsBinaryEncodeLongIntegerLen(UINT32 integer);
static bool __MmsBinaryEncodeLongInteger(FILE *pFile, UINT32 integer, int length);
static int	__MmsBinaryEncodeTextStringLen(UINT8 *source);
static bool __MmsBinaryEncodeTextString(FILE *pFile, UINT8 *source, int length);
static int	__MmsBinaryEncodeQuotedStringLen(UINT8 *pSrc);
static bool __MmsBinaryEncodeQuotedString(FILE *pFile, UINT8 *source, int length);
static int	__MmsBinaryEncodeEncodedStringLen(UINT8 *source);
static bool __MmsBinaryEncodeEncodedString(FILE *pFile, UINT8 *source, int length);

static int __MmsBinaryEncodeContentTypeLen(MsgType *pType);
static bool __MmsBinaryEncodeContentType(FILE *pFile, MsgType *pType, int typeLength);
static int __MmsBinaryEncodeContentHeaderLen(MimeType contentType, MsgType *pType, bool bMultipart);
static bool __MmsBinaryEncodeContentHeader(FILE *pFile, MimeType contentType, MsgType *pType, bool bMultipart);

static bool __MmsBinaryEncodeContentBody(FILE *pFile, MsgBody *pBody);
static bool __MmsBinaryEncodeMsgPart(FILE *pFile, int contentType, MsgType *pType, MsgBody *pBody);

static bool __MmsBinaryEncodeMmsVersion(FILE *pFile);
static bool __MmsBinaryEncodeTrID(FILE *pFile, char *szTrID, int bufLen);
static bool __MmsBinaryEncodeMsgID(FILE *pFile, const char *szMsgID);	/** 2005-05-24, added for read-reply PDU 1.2 */
static bool __MmsBinaryEncodeFrom(FILE *pFile);
static bool __MmsBinaryEncodeTime(FILE *pFile, MmsFieldCode fieldCode, MmsTimeStruct time);
static bool __MmsBinaryEncodeDate(FILE *pFile, time_t inpDateSec);
static bool __MmsBinaryEncodeOneAddress(FILE *pFile, MmsFieldCode addrType, char *szAddrStr);
static bool __MmsBinaryEncodeAddress(FILE *pFile, MmsFieldCode addrType, char *szAddr);

static bool __MmsBinaryEncodeFieldCodeAndValue(FILE *pFile, UINT8 fieldCode, UINT8 fieldValue);
static bool __MmsBinaryEncodeSendReqHdr(FILE *pFile, MmsMsg *pMsg);
static bool __MmsBinaryEncodeAckIndHdr(FILE *pFile, char *pTrID, bool bReportAllowed);
static bool __MmsBinaryEncodeNotiRespIndHdr(FILE* pFile, char *pTrID, msg_delivery_report_status_t iStatus, bool bReportAllowed);
static bool __MmsBinaryEncodeReadReport10Hdr(FILE *pFile, MmsMsg *pMsg, msg_read_report_status_t mmsReadStatus);
static bool __MmsBinaryEncodeReadReport11Hdr(FILE *pFile, MmsMsg *pMsg, msg_read_report_status_t mmsReadStatus);

static bool __MmsBinaryEncodeMsgBody(FILE *pFile, MsgType *pType, MsgBody *pBody, int nPartCount, bool bTemplate); /* NEW_TEMPLATE */
static bool __MmsEncodeSendReq(FILE *pFile, MmsMsg *pMsg, bool bIncludeSendReqHeader);

/* Functions for Acknowledge.ind & NotifyResp.ind  ------------------------ */
static bool __MmsBinaryEncodeTextString2(FILE *pFile, UINT8 *source, int length);
static bool __MmsBinaryEncodeMmsVersion2(FILE *pFile);
static bool __MmsBinaryEncodeFieldCodeAndValue2(FILE *pFile, UINT8 fieldCode, UINT8 fieldValue);

static char *_MsgSkipWS3(char *s);

/** -----------------------------------------------------------------
 *					M   M   S       E   N   C   O   D   E
 * * -----------------------------------------------------------------*/

static void __MmsCleanEncodeBuff(void)
{
	memset(gpMmsEncodeBuf, 0, MSG_MMS_ENCODE_BUFFER_MAX);
	gCurMmsEncodeBuffPos = 0;
}


void MmsRegisterEncodeBuffer(char *pInBuff, int maxLen)
{
	gpMmsEncodeBuf = pInBuff;
	gCurMmsEncodeBuffPos = 0;
	gMmsEncodeMaxLen = maxLen;
	gMmsEncodeCurOffset = 0;
}

void MmsUnregisterEncodeBuffer(void)
{
	gpMmsEncodeBuf = NULL;
	gCurMmsEncodeBuffPos = 0;
	gMmsEncodeMaxLen = 0;
	gMmsEncodeCurOffset = 0;
}

static int __MmsGetEncodeOffset(void)
{
	return (gMmsEncodeCurOffset + gCurMmsEncodeBuffPos);
}

bool MmsEncodeSendReq(FILE *pFile, MmsMsg *pMsg)
{
	return __MmsEncodeSendReq(pFile, pMsg, true);
}

bool MmsEncodeTemplate(FILE *pFile, MmsMsg *pMsg)
{
	return __MmsEncodeSendReq(pFile, pMsg, false);
}

static bool __MmsEncodeSendReq(FILE* pFile, MmsMsg* pMsg, bool bIncludeSendReqHeader)
{
	MmsRegisterEncodeBuffer(gszMmsEncodeBuf, MSG_MMS_ENCODE_BUFFER_MAX);

	if (bIncludeSendReqHeader) {
		if (__MmsBinaryEncodeSendReqHdr(pFile, pMsg) == false) {
			MmsUnregisterEncodeBuffer();
			return false;
		}
	}

	if (__MmsBinaryEncodeMsgBody(pFile, &pMsg->msgType, &pMsg->msgBody, pMsg->nPartCount, !bIncludeSendReqHeader) == false) {
		MmsUnregisterEncodeBuffer();
		return false;
	}

	MmsUnregisterEncodeBuffer();

	return true;
}

/* Functions for Acknowledge.ind & NotifyResp.ind ------------------------ */

static void __MmsCleanEncodeBuff2(void)
{
	memset(gpMmsEncodeBuf2, 0, MSG_MMS_ENCODE_BUFFER_MAX);
	gCurMmsEncodeBuffPos2 = 0;
}

static void __MmsRegisterEncodeBuffer2(char *pInBuff, int maxLen)
{
	gpMmsEncodeBuf2 = pInBuff;
	gCurMmsEncodeBuffPos2 = 0;
	gMmsEncodeMaxLen2 = maxLen;
	gMmsEncodeCurOffset2 = 0;
}

static void __MmsUnregisterEncodeBuffer2(void)
{
	gpMmsEncodeBuf2 = NULL;
	gCurMmsEncodeBuffPos2 = 0;
	gMmsEncodeMaxLen2 = 0;
	gMmsEncodeCurOffset2 = 0;
}

/**
 * @param 	source [in] originam string
 * @param	length [in] gotten from MmsBinaryEncodeTextStringLen()
 * @param	dest [in] buffer to store quted string
 * @return 	changed string length
*/
static bool __MmsBinaryEncodeTextString2(FILE *pFile, UINT8 *source, int length)
{
	/**
	 * make text string
	 * Text-string = [Quote] *TEXT End-of-string
	 * If the 1st char in the TEXT is in the range of 128-255, a Quote char must precede it.
	 * Otherwise the Quote char must be omitted.
	 * The Quote is not part of the contents.
	 * Quote = <Octet 127>
	 */

	if (pFile == NULL || source == NULL) {
		MSG_DEBUG("source == NULL");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen2 - gCurMmsEncodeBuffPos2) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
											gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	if (source[0] > 0x7F) {
		gpMmsEncodeBuf2[gCurMmsEncodeBuffPos2++] = QUOTE;
		length--;
	}

	strncpy(gpMmsEncodeBuf2 + gCurMmsEncodeBuffPos2, (char*)source, (length - 1));	/** except NULL */
	gCurMmsEncodeBuffPos2 += (length - 1);	/** except NULL */
	gpMmsEncodeBuf2[gCurMmsEncodeBuffPos2++] = (UINT8)NULL;

	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeFieldCodeAndValue2(FILE *pFile, UINT8 fieldCode, UINT8 fieldValue)
{
	if ((gMmsEncodeMaxLen2 - gCurMmsEncodeBuffPos2) < 2) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
											gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	if (fieldCode == 0xff) {
		MSG_DEBUG("invalid fieldCode");
		goto __CATCH;
	}

	if (fieldValue == 0xff) {
		MSG_DEBUG("invalid fieldValue");
		return true;
	}

	gpMmsEncodeBuf2[gCurMmsEncodeBuffPos2++] = fieldCode;
	gpMmsEncodeBuf2[gCurMmsEncodeBuffPos2++] = fieldValue;

	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeMmsVersion2(FILE *pFile)
{
	UINT8 majorVer = MMS_MAJOR_VERSION;
	UINT8 minorVer = MMS_MINOR_VERSION;

	if (pFile == NULL) {
		MSG_DEBUG("invalid input file");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen2 - gCurMmsEncodeBuffPos2) < 2) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
											gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf2[gCurMmsEncodeBuffPos2++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_VERSION) | 0x80;
	gpMmsEncodeBuf2[gCurMmsEncodeBuffPos2] = (majorVer << 4) | (minorVer & 0x0f) | MSB;

	if (gpMmsEncodeBuf2[gCurMmsEncodeBuffPos2] < 0x80) {
		gpMmsEncodeBuf2[gCurMmsEncodeBuffPos2++] |= 0x80;
	} else {
		gCurMmsEncodeBuffPos2++;
	}

	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeAckIndHdr(FILE *pFile, char *szTrID, bool bReportAllowed)
{
	int length = 0;
	UINT8 fieldCode = 0xff;
	UINT8 fieldValue = 0xff;

	MSG_DEBUG("szTrID = %s", szTrID);
	MSG_DEBUG("bReportAllowed = %d", bReportAllowed);

	__MmsCleanEncodeBuff2();

	/* msgType */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGTYPE) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeMsgType, MMS_MSGTYPE_ACKNOWLEDGE_IND) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue2(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("msgType error");
		goto __CATCH;
	}

	/* trID (other type of message) */
	length = __MmsBinaryEncodeTextStringLen((UINT8*)szTrID);
	if (length == -1) {
		MSG_DEBUG("MmsBinaryEncodeTextStringLen fail");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen2 - gCurMmsEncodeBuffPos2) < (length + 1)) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
											gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf2[gCurMmsEncodeBuffPos2++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_TRID) | 0x80;

	if (__MmsBinaryEncodeTextString2(pFile, (UINT8*)szTrID, length) == false) {
		MSG_DEBUG("MmsBinaryEncodeTextString fail");
		goto __CATCH;
	}


	if (__MmsBinaryEncodeMmsVersion2(pFile) == false) {
		MSG_DEBUG("__MmsBinaryEncodeMmsVersion error");
		goto __CATCH;
	}


	/* Report Allowed */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_REPORTALLOWED) | 0x80;

	if (bReportAllowed) {
		fieldValue = MmsGetBinaryValue(MmsCodeReportAllowed, MMS_REPORTALLOWED_YES) | 0x80;
	} else {
		fieldValue = MmsGetBinaryValue(MmsCodeReportAllowed, MMS_REPORTALLOWED_NO) | 0x80;
	}

	if (__MmsBinaryEncodeFieldCodeAndValue2(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("Report Allowed error");
		goto __CATCH;
	}

	/* flush remained data on encoding file */
	if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
										gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
		MSG_DEBUG("remained data MsgWriteDataFromEncodeBuffer fail");
		goto __CATCH;
	}

	return true;

__CATCH:

	return false;
}

static bool __MmsBinaryEncodeNotiRespIndHdr(FILE *pFile, char *szTrID, msg_delivery_report_status_t iStatus, bool bReportAllowed)
{
	int length = 0;
	UINT8 fieldCode = 0xff;
	UINT8 fieldValue = 0xff;

	MSG_DEBUG("szTrID = %s", szTrID);
	MSG_DEBUG("iStatus = %d", iStatus);
	MSG_DEBUG("bReportAllowed = %d", bReportAllowed);

	__MmsCleanEncodeBuff2();

	/* msgType */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGTYPE) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeMsgType, MMS_MSGTYPE_NOTIFYRESP_IND) | 0x80;

	if (__MmsBinaryEncodeFieldCodeAndValue2(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("msgType error");
		goto __CATCH;
	}


	/* trID (other type of message) */
	length = __MmsBinaryEncodeTextStringLen((UINT8*)szTrID);
	if (length == -1) {
		MSG_DEBUG("MmsBinaryEncodeTextStringLen fail");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen2 - gCurMmsEncodeBuffPos2) < (length + 1)) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
											gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf2[gCurMmsEncodeBuffPos2++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_TRID) | 0x80;
	if (__MmsBinaryEncodeTextString2(pFile, (UINT8*)szTrID, length) == false) {
		MSG_DEBUG("MmsBinaryEncodeTextString fail");
		goto __CATCH;
	}


	if (__MmsBinaryEncodeMmsVersion2(pFile) == false) {
		MSG_DEBUG("__MmsBinaryEncodeMmsVersion error");
		goto __CATCH;
	}


	/* MsgStatus */
	MSG_DEBUG("MsgStatus = %d", iStatus);

	if (iStatus != MSG_DELIVERY_REPORT_NONE) {
		fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGSTATUS) | 0x80;
		fieldValue = MmsGetBinaryValue(MmsCodeMsgStatus, iStatus) | 0x80;
		if (__MmsBinaryEncodeFieldCodeAndValue2(pFile, fieldCode, fieldValue) == false) {
			MSG_DEBUG("MsgStatus error");
			goto __CATCH;
		}
	}


	/* Report Allowed */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_REPORTALLOWED) | 0x80;

	if (bReportAllowed) {
		fieldValue = MmsGetBinaryValue(MmsCodeReportAllowed, MMS_REPORTALLOWED_YES) | 0x80;
	} else {
		fieldValue = MmsGetBinaryValue(MmsCodeReportAllowed, MMS_REPORTALLOWED_NO) | 0x80;
	}

	if (__MmsBinaryEncodeFieldCodeAndValue2(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("Report Allowed error");
		goto __CATCH;
	}

	/* flush remained data on encoding file */
	if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
										gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
		MSG_DEBUG("remained data MsgWriteDataFromEncodeBuffer fail");
		goto __CATCH;
	}

	return true;

__CATCH:

	return false;
}

bool MmsEncodeAckInd(FILE *pFile, char *pTrID, bool bReportAllowed)
{
	__MmsRegisterEncodeBuffer2(gszMmsEncodeBuf2, MSG_MMS_ENCODE_BUFFER_MAX);

	MSG_DEBUG("Start Binary Encoding now =============");

	if (__MmsBinaryEncodeAckIndHdr(pFile, pTrID, bReportAllowed) == false) {
		MSG_DEBUG("SendReq Binary encoding fail");
		goto __CATCH;
	}

	__MmsUnregisterEncodeBuffer2();

	return true;

__CATCH:

	MSG_DEBUG("Failed");
	__MmsUnregisterEncodeBuffer2();

	return false;
}

bool MmsEncodeNotiRespInd(FILE *pFile, char *pTrID, msg_delivery_report_status_t iStatus, bool bReportAllowed)
{
	__MmsRegisterEncodeBuffer2(gszMmsEncodeBuf2, MSG_MMS_ENCODE_BUFFER_MAX);

	MSG_DEBUG("Start Binary Encoding now =============");

	if (__MmsBinaryEncodeNotiRespIndHdr(pFile, pTrID, iStatus, bReportAllowed) == false) {
		MSG_DEBUG("SendReq Binary encoding fail");
		goto __CATCH;
	}

	__MmsUnregisterEncodeBuffer2();

	return true;

__CATCH:

	return false;
}

/* Functions for Acknowledge.ind & NotifyResp.ind  (END) ------------------------ */
bool MmsEncodeReadReport10(FILE *pFile, MmsMsg *pMsg, msg_read_report_status_t mmsReadStatus)
{
	char *pText = NULL;
	MsgMultipart *pPart = NULL;
	MsgType msgType;

	MsgBody *msgBody = NULL;
	unique_ptr<MsgBody*, void(*)(MsgBody**)> buf(&msgBody, unique_ptr_deleter);
	msgBody = (MsgBody *)new char[sizeof(MsgBody)];
	memset(msgBody, 0x00, sizeof(MsgBody));

	char *pszReportMsg = NULL;
	int maxLen = 0;

	struct tm dateTime;
	time_t RawTime = 0;

	MmsRegisterEncodeBuffer(gszMmsEncodeBuf, MSG_MMS_ENCODE_BUFFER_MAX);

	MSG_DEBUG("Start Binary Encoding now V1.0=============");

	if (__MmsBinaryEncodeReadReport10Hdr(pFile, pMsg, mmsReadStatus) == false) {
		MSG_DEBUG("SendReq Binary encoding fail");
		goto __CATCH;
	}

	memset(&msgType, 0, sizeof(MsgType));

	pText = (char *)calloc(1, MSG_STDSTR_LONG);
	if (pText == NULL) {
		MSG_DEBUG("text body calloc fail");
		goto __CATCH;
	}

	memset(pText, 0, MSG_STDSTR_LONG);

	time(&RawTime);
	localtime_r(&RawTime, &dateTime);

	/* get report message */
	if (mmsReadStatus == MSG_READ_REPORT_IS_DELETED) {
		pszReportMsg = (char*)"Your message has been deleted " ;
	} else {
		pszReportMsg = (char*)"Your message has been read " ;
	}

	/* make report body .. */
	maxLen = strlen(pszReportMsg) + 16 /* date string */ + 8 /* enter chars */ ;

	if (maxLen > MSG_STDSTR_LONG) {
		snprintf(pText, MSG_STDSTR_LONG, "%s", pszReportMsg);
	} else {
		snprintf(pText, MSG_STDSTR_LONG, "%s\r\n\r\n%.4d/%.2d/%.2d %.2d:%.2d\r\n",
						pszReportMsg, dateTime.tm_year+1900, dateTime.tm_mon+1, dateTime.tm_mday, dateTime.tm_hour, dateTime.tm_min);
	}

	/* make header */
	msgType.type = MIME_APPLICATION_VND_WAP_MULTIPART_MIXED;
	msgType.contentSize = strlen(pText);
	msgType.param.charset = MSG_CHARSET_UNKNOWN;

	/* make body */
	if ((pPart = MmsAllocMultipart()) == NULL) {
		MSG_DEBUG("MsgAllocMultipart Fail");
		goto __CATCH;
	}


	pPart->type.type = MIME_TEXT_PLAIN;
	pPart->type.contentSize = strlen(pText);
	pPart->type.param.charset = MSG_CHARSET_UTF8;

	if (pPart->pBody == NULL) {
		MSG_DEBUG("pPart->pBody is NULL");
		goto __CATCH;
	}

	pPart->pBody->size = strlen(pText);
	pPart->pBody->body.pText = pText;

	msgBody->body.pMultipart = pPart;

	if (__MmsBinaryEncodeMsgBody(pFile, &msgType, msgBody, 1, false) == false) {
		MSG_DEBUG("MmsBinaryEncodeMsgBody fail");
		goto __CATCH;
	}

	MSG_DEBUG(" Send To RM ReadReport Msg");

	if (pText) {
		free(pText);
		pText = NULL;
	}

	if (pPart) {
		if (pPart->pBody) {
			free(pPart->pBody);
			pPart->pBody = NULL;
		}
		free(pPart);
		pPart = NULL;
	}

	MmsUnregisterEncodeBuffer();

	return true;

__CATCH:

	if (pText) {
		free(pText);
		pText = NULL;
	}

	if (pPart) {
		if (pPart->pBody) {
			free(pPart->pBody);
			pPart->pBody = NULL;
		}
		free(pPart);
		pPart = NULL;
	}

	MmsUnregisterEncodeBuffer();

	return false;
}

bool MmsEncodeReadReport11(FILE *pFile, MmsMsg *pMsg, msg_read_report_status_t mmsReadStatus)
{
	MmsRegisterEncodeBuffer(gszMmsEncodeBuf, MSG_MMS_ENCODE_BUFFER_MAX);

	MSG_DEBUG("Start Binary Encoding now V1.1=============");

	if (__MmsBinaryEncodeReadReport11Hdr(pFile, pMsg, mmsReadStatus) == false) {
		MSG_DEBUG("SendReq Binary encoding fail");
		goto __CATCH;
	}

	MmsUnregisterEncodeBuffer();

	return true;

__CATCH:
	MmsUnregisterEncodeBuffer();

	return false;
}

/* ==========================================================

					B  I  N  A  R  Y         E  N  C  O  D  I  N  G

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
static bool __MmsBinaryEncodeSendReqHdr(FILE *pFile, MmsMsg *pMsg)
{
	UINT8 fieldCode	= 0xff;
	UINT8 fieldValue	= 0xff;
	int length		= 0;

	__MmsCleanEncodeBuff();

	/* msgType */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGTYPE) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeMsgType, MMS_MSGTYPE_SEND_REQ) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("msgType error");
		goto __CATCH;
	}

	MSG_SEC_INFO("X-Mms-Message-Type = [%s]", MmsDebugGetMsgType(MMS_MSGTYPE_SEND_REQ));

	/* trID (other type of message) */
	if (__MmsBinaryEncodeTrID(pFile, pMsg->szTrID, MMS_TR_ID_LEN + 1) == false) {
		MSG_DEBUG("trID error");
		goto __CATCH;
	}

	if (__MmsBinaryEncodeMmsVersion(pFile) == false) {
		MSG_DEBUG("__MmsBinaryEncodeMmsVersion error");
		goto __CATCH;
	}

	/* From : Insert Token mode */
	if (__MmsBinaryEncodeFrom(pFile) == false) {
		MSG_DEBUG("__MmsBinaryEncodeFrom fail");
		goto __CATCH;
	}


	/* To = Encoded-string-value */
	if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_TO, pMsg->mmsAttrib.szTo) == false) {
		MSG_DEBUG("To __MmsBinaryEncodeAddress fail");
		goto __CATCH;
	}


	/* Cc = Encoded-string-value */
	if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_CC, pMsg->mmsAttrib.szCc) == false) {
		MSG_DEBUG("Cc __MmsBinaryEncodeAddress fail");
		goto __CATCH;
	}


	/* Bcc = Encoded-string-value */
	if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_BCC, pMsg->mmsAttrib.szBcc) == false) {
		MSG_DEBUG("Bcc __MmsBinaryEncodeAddress fail");
		goto __CATCH;
	}

	MSG_SEC_INFO("Subject = [%s]", pMsg->mmsAttrib.szSubject);

	/* Subject = Encoded-string-value */
	if (pMsg->mmsAttrib.szSubject[0]) {
		length = __MmsBinaryEncodeEncodedStringLen((UINT8*)pMsg->mmsAttrib.szSubject);
		if (length == -1) {
			MSG_DEBUG("subject MmsBinaryEncodeEncodedStringLen fail");
			goto __CATCH;
		}

		if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length + 1) {
			if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
												gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
				MSG_DEBUG("subject MsgWriteDataFromEncodeBuffer fail");
				goto __CATCH;
			}
		}

		fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_SUBJECT) | 0x80;
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = fieldCode;
		if (__MmsBinaryEncodeEncodedString(pFile, (UINT8*)pMsg->mmsAttrib.szSubject, length) == false) {
			MSG_DEBUG("subject MmsBinaryEncodeEncodedString fail");
			goto __CATCH;
		}
	}

	/* MMS-1.3-con-739 */
	/* Msg class */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGCLASS) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeMsgClass,  (int)pMsg->mmsAttrib.msgClass) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("msgClass error");
		goto __CATCH;
	}

	MSG_SEC_INFO("X-Mms-Message-Class = [%s]", MmsDebugGetMsgClass(pMsg->mmsAttrib.msgClass));

	/* MMS-1.3-con-739 */
	/* MMS-1.3-con-733 */
	/* Date = Long-integer */
	if (!__MmsBinaryEncodeDate(pFile, 0)) {
		MSG_DEBUG("__MmsBinaryEncodeDate error");
		goto __CATCH;
	}
	/* MMS-1.3-con-733 */

	/* Expiry Time  : Value-length Absolute-token Date-value */
	if (pMsg->mmsAttrib.expiryTime.type != MMS_TIMETYPE_NONE) {
		/* for avoiding the creation of the expiry field in case the user selects the maximum */
		if (__MmsBinaryEncodeTime(pFile, MMS_CODE_EXPIRYTIME, pMsg->mmsAttrib.expiryTime) == false) {
			MSG_DEBUG("expiryTime __MmsBinaryEncodeTime fail");
			goto __CATCH;
		}

		MSG_DEBUG("X-Mms-Expiry : type = [%d], time = [%u]", pMsg->mmsAttrib.expiryTime.type, pMsg->mmsAttrib.expiryTime.time);
	}

	if (pMsg->mmsAttrib.deliveryTime.type != MMS_TIMETYPE_NONE) {
		if (__MmsBinaryEncodeTime(pFile, MMS_CODE_DELIVERYTIME, pMsg->mmsAttrib.deliveryTime) == false) {
			MSG_DEBUG("deliveryTime __MmsBinaryEncodeTime fail");
			goto __CATCH;
		}

		MSG_DEBUG("X-Mms-Delivery-Time : type = [%d], time = [%u]", pMsg->mmsAttrib.deliveryTime.type, pMsg->mmsAttrib.deliveryTime.time);
	}

	/* Priority */
	if (pMsg->mmsAttrib.priority!= MMS_PRIORITY_ERROR && pMsg->mmsAttrib.priority!= MMS_PRIORITY_NORMAL) {
		/* MMS_PRIORITY_NORMAL is default : don't send optional field */

		fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_PRIORITY)|0x80;
		fieldValue = MmsGetBinaryValue(MmsCodePriority, pMsg->mmsAttrib.priority)|0x80;

		if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
			MSG_DEBUG("priority error");
			goto __CATCH;
		}

		MSG_SEC_INFO("X-Mms-Priority = [%d]", pMsg->mmsAttrib.priority);
	}

	/* Sender Visible (hide | show)	*/
	if (pMsg->mmsAttrib.bHideAddress == true) {
		fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_SENDERVISIBILLITY) | MSB;
		fieldValue = MmsGetBinaryValue(MmsCodeSenderVisibility, MMS_SENDER_HIDE) | 0x80;

		if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
			MSG_DEBUG("sender visibility error");
			goto __CATCH;
		}
	}

	MSG_SEC_INFO("X-Mms-Sender-Visibility = [%d]", pMsg->mmsAttrib.bHideAddress);

	/* Delivery Report (yes | no) */
	fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_DELIVERYREPORT)|0x80;

	if (pMsg->mmsAttrib.bAskDeliveryReport) {
		fieldValue = MmsGetBinaryValue(MmsCodeDeliveryReport, MMS_REPORT_YES) | 0x80;
	} else {
		fieldValue = MmsGetBinaryValue(MmsCodeDeliveryReport, MMS_REPORT_NO) | 0x80;
	}

	MSG_SEC_INFO("X-Mms-Delivery-Report = [%d]", pMsg->mmsAttrib.bAskDeliveryReport);

	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("sender visibility error");
		goto __CATCH;
	}

	/* Read Reply (Yes | no) */
	fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_READREPLY)|0x80;
	if (pMsg->mmsAttrib.bAskReadReply) {
		fieldValue = MmsGetBinaryValue(MmsCodeReadReply, MMS_REPORT_YES)|0x80;
	} else {
		fieldValue = MmsGetBinaryValue(MmsCodeReadReply, MMS_REPORT_NO)|0x80;
	}

	MSG_SEC_INFO("X-Mms-Read-Report = [%d]", pMsg->mmsAttrib.bAskReadReply);

	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("read reply error");
		goto __CATCH;
	}

	if ((pMsg->mmsAttrib.replyCharge.chargeType == MMS_REPLY_REQUESTED) ||
		(pMsg->mmsAttrib.replyCharge.chargeType == MMS_REPLY_REQUESTED_TEXT_ONLY)) {
		/* reply charging */
		fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_REPLYCHARGING)|0x80;
		fieldValue = MmsGetBinaryValue(MmsCodeReadReply, pMsg->mmsAttrib.replyCharge.chargeType)|0x80;

		if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
			MSG_DEBUG("replyCharging error");
			goto __CATCH;
		}

		/** fixme: Reply-charging-deadline */
		if (pMsg->mmsAttrib.replyCharge.deadLine.time > 0) {
			if (__MmsBinaryEncodeTime(pFile, MMS_CODE_REPLYCHARGINGDEADLINE, pMsg->mmsAttrib.replyCharge.deadLine) == false) {
				MSG_DEBUG("replyCharging __MmsBinaryEncodeTime fail");
				goto __CATCH;
			}
		}

		/** fixme: Reply-charging-size */
		if (pMsg->mmsAttrib.replyCharge.chargeSize > 0) {
			length = __MmsBinaryEncodeIntegerLen(pMsg->mmsAttrib.replyCharge.chargeSize);

			if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < (length + 1)) {
				if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
												gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
					MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
					goto __CATCH;
				}
			}

			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_REPLYCHARGINGSIZE) | 0x80;

			if (__MmsBinaryEncodeInteger(pFile, pMsg->mmsAttrib.replyCharge.chargeSize, length) == false) {
				MSG_DEBUG("replyChargingSize MmsBinaryEncodeInteger error");
				goto __CATCH;
			}
		}

		/** fixme: Reply-charging-ID  ----> used only when reply message  */
		if (pMsg->mmsAttrib.replyCharge.szChargeID[0]) {
			length = __MmsBinaryEncodeTextStringLen((UINT8*)pMsg->mmsAttrib.replyCharge.szChargeID);
			if (length == -1) {
				MSG_DEBUG("szReplyChargingID MmsBinaryEncodeTextStringLen fail");
				goto __CATCH;
			}

			if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < (length + 1)) {
				if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
												gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
					MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
					goto __CATCH;
				}
			}

			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_REPLYCHARGINGID) | 0x80;

			if (__MmsBinaryEncodeTextString(pFile, (UINT8*)pMsg->mmsAttrib.replyCharge.szChargeID, length) == false) {
				MSG_DEBUG("szContentLocation MmsBinaryEncodeTextString fail");
				goto __CATCH;
			}
		}
	}

	/* flush remained data on encoding file */
	if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos, gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
		MSG_DEBUG("remained data MsgWriteDataFromEncodeBuffer fail");
		goto __CATCH;
	}

	return true;

__CATCH:

	MSG_DEBUG("Failed");
	return false;
}

static bool __MmsBinaryEncodeReadReport10Hdr(FILE *pFile, MmsMsg *pMsg, msg_read_report_status_t mmsReadStatus)
{
	int length	= 0;
	char *szTo	= NULL;
	UINT8 fieldCode = 0xff;
	UINT8 fieldValue = 0xff;
	char szSubject[MSG_LOCALE_SUBJ_LEN + 8] = {0, };

	if (pMsg == NULL) {
		MSG_DEBUG("pMsg is NULL");
		goto __CATCH;
	}

	MSG_DEBUG("mmsReadStatus = %d", mmsReadStatus);

	__MmsCleanEncodeBuff();

	/* msgType */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGTYPE) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeMsgType, MMS_MSGTYPE_SEND_REQ) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("msgType error");
		goto __CATCH;
	}

	/* trID (other type of message) */
	if (__MmsBinaryEncodeTrID(pFile, NULL, 0) == false) {
		MSG_DEBUG("__MmsBinaryEncodeTrID error");
		goto __CATCH;
	}

	if (__MmsBinaryEncodeMmsVersion(pFile) == false) {
		MSG_DEBUG("__MmsBinaryEncodeMmsVersion error");
		goto __CATCH;
	}

	/* Date = Long-integer */
	if (!__MmsBinaryEncodeDate(pFile, 0)) {
		MSG_DEBUG("date __MmsBinaryEncodeDate error");
		goto __CATCH;
	}

	/* From : Insert Token mode */
	if (__MmsBinaryEncodeFrom(pFile) == false) {
		MSG_DEBUG("__MmsBinaryEncodeFrom fail");
		goto __CATCH;
	}

	MSG_DEBUG("To = %s", pMsg->mmsAttrib.szFrom);

	/* To = Encoded-string */
	if (pMsg && (strchr(pMsg->mmsAttrib.szFrom, '/') == NULL)) {
		length = strlen(pMsg->mmsAttrib.szFrom);
		szTo = (char *)calloc(1, length + 11);
		if (szTo == NULL) {
			MSG_DEBUG("szTo alloc fail");
			goto __CATCH;
		}

		snprintf(szTo, length + 11, "%s/TYPE=PLMN", pMsg->mmsAttrib.szFrom);
		MSG_DEBUG("To = %s", szTo);

		if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_TO, szTo) == false) {
			MSG_DEBUG("To __MmsBinaryEncodeAddress fail");
			goto __CATCH;
		}

		if (szTo) {
			free(szTo);
			szTo = NULL;
		}
	} else {
		if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_TO, pMsg->mmsAttrib.szFrom) == false) {
			MSG_DEBUG("To __MmsBinaryEncodeAddress fail");
			goto __CATCH;
		}
	}

	/* Subject = Encoded-string-value */
	if (pMsg && pMsg->mmsAttrib.szSubject[0]) {
		if (mmsReadStatus == MSG_READ_REPORT_IS_READ) {
			snprintf(szSubject, MSG_LOCALE_SUBJ_LEN + 8, "%s%s", MMS_READ_REPORT_STRING_READ, pMsg->mmsAttrib.szSubject);
		} else {
			snprintf(szSubject, MSG_LOCALE_SUBJ_LEN + 8, "%s%s", MMS_READ_REPORT_STRING_DELETED, pMsg->mmsAttrib.szSubject);
		}
	} else {
		if (mmsReadStatus == MSG_READ_REPORT_IS_READ) {
			snprintf(szSubject, MSG_LOCALE_SUBJ_LEN + 8, "%s", MMS_READ_REPORT_STRING_READ);

		} else {
			snprintf(szSubject, MSG_LOCALE_SUBJ_LEN + 8, "%s", MMS_READ_REPORT_STRING_DELETED);
		}
	}

	length = __MmsBinaryEncodeEncodedStringLen((UINT8*)szSubject);
	if (length == -1) {
		MSG_DEBUG("subject MmsBinaryEncodeEncodedStringLen fail");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length + 1) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf, &gCurMmsEncodeBuffPos, gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("subject MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_SUBJECT) | 0x80;
	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = fieldCode;
	if (__MmsBinaryEncodeEncodedString(pFile, (UINT8*)szSubject, length) == false) {
		MSG_DEBUG("subject MmsBinaryEncodeEncodedString fail");
		goto __CATCH;
	}

	/* Msg class */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGCLASS) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeMsgClass, MMS_MSGCLASS_AUTO) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("msgClass error");
		goto __CATCH;
	}


	/* Delivery Report (yes | no) */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_DELIVERYREPORT) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeDeliveryReport, MMS_REPORT_NO) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("Delivery Report error");
		goto __CATCH;
	}


	/* Read Reply (Yes | no) */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_READREPLY) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeReadReply, MMS_REPORT_NO) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("Read Reply error");
		goto __CATCH;
	}


	/* Sender Visible (hide | show)	*/
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_SENDERVISIBILLITY) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeSenderVisibility, MMS_SENDER_SHOW) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("Sender Visible error");
		goto __CATCH;
	}

	/* fixme: MimeType */
	/* fixme: msgHeader */
	/* fixme: msgBody */

	/* flush remained data on encoding file */
	if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
										gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
		MSG_DEBUG("remained data MsgWriteDataFromEncodeBuffer fail");
		goto __CATCH;
	}

	return true;

__CATCH:
	if (szTo) {
		free(szTo);
		szTo = NULL;
	}

	return false;
}

static bool __MmsBinaryEncodeReadReport11Hdr(FILE *pFile, MmsMsg *pMsg, msg_read_report_status_t mmsReadStatus)
{
	UINT8 fieldCode = 0xff;
	UINT8 fieldValue = 0xff;
	char *szTo = NULL;

	__MmsCleanEncodeBuff();

	/* msgType */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGTYPE) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeMsgType, MMS_MSGTYPE_READREC_IND) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("msgType error");
		goto __CATCH;
	}

	/* MMS version */
	if (__MmsBinaryEncodeMmsVersion(pFile) == false) {
		MSG_DEBUG("__MmsBinaryEncodeMmsVersion error");
		goto __CATCH;
	}

	if (__MmsBinaryEncodeMsgID(pFile, pMsg->szMsgID) == false)
		goto __CATCH;

	/* To = Encoded-string */
	if (strchr(pMsg->mmsAttrib.szFrom, '/') == NULL) {
		int length = 0;
		length = strlen(pMsg->mmsAttrib.szFrom);
		szTo = (char *)calloc(1, length + 11);

		if (szTo == NULL) {
			MSG_DEBUG("szTo alloc fail");
			goto __CATCH;
		}

		snprintf(szTo, length + 11, "%s/TYPE=PLMN", pMsg->mmsAttrib.szFrom);
		MSG_DEBUG("To = %s", szTo);

		if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_TO, szTo) == false) {
			MSG_DEBUG("To __MmsBinaryEncodeAddress fail");
			goto __CATCH;
		}

		if (szTo) {
			free(szTo);
			szTo = NULL;
		}
	} else {
		if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_TO, pMsg->mmsAttrib.szFrom) == false) {
			MSG_DEBUG("To __MmsBinaryEncodeAddress fail");
			goto __CATCH;
		}
	}

	/* From : Insert Token mode */
	if (__MmsBinaryEncodeFrom(pFile) == false) {
		MSG_DEBUG("__MmsBinaryEncodeFrom fail");
		goto __CATCH;
	}

	/* Date = Long-integer */
	if (!__MmsBinaryEncodeDate(pFile, 0)) {
		MSG_DEBUG("__MmsBinaryEncodeDate error");
		goto __CATCH;
	}

	/* Read Status (Yes | no) */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_READSTATUS) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeReadStatus, mmsReadStatus) | 0x80;

	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("Read Status error");
		goto __CATCH;
	};

	if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
										gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
		MSG_DEBUG("remained data MsgWriteDataFromEncodeBuffer fail");
		goto __CATCH;
	}

	return true;

__CATCH:
	if (szTo) {
		free(szTo);
		szTo = NULL;
	}

	return false;
}

static bool __MmsBinaryEncodeMsgBody(FILE *pFile, MsgType *pType, MsgBody *pBody, int nPartCount, bool bTemplate)
{
	int length = 0;
	int index = 0;
	MsgMultipart *pMultipart = NULL;

	MSG_DEBUG("nPartCount = %d", nPartCount);

	if (pFile == NULL || pType == NULL) {
		MSG_DEBUG("invalid file handle");
		goto __CATCH;
	}

	if (MmsIsMultipart(pType->type)) {
		int	nEntries   = 0;
		/* ---------------------------
		 *       Multipart message
		 * ---------------------------*/

		pType->offset = __MmsGetEncodeOffset();

		/* Content type */
		length = __MmsBinaryEncodeContentTypeLen(pType);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeContentTypeLen fail");
			goto __CATCH;
		}
		if (bTemplate == false)
			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_CONTENTTYPE) | 0x80;

		if (__MmsBinaryEncodeContentType(pFile, pType, length) == false) {
			MSG_DEBUG("MmsBinaryEncodeContentType fail");
			goto __CATCH;
		}

		pBody->offset = __MmsGetEncodeOffset();

		/* nEntries */
		if (pBody->pPresentationBody) {
			nEntries = nPartCount + 1;
		} else {
			nEntries = nPartCount;
		}

		if (nEntries >= 0) {
			length = __MmsBinaryEncodeUintvarLen(nEntries);
			if (length == -1) {
				MSG_DEBUG("nEntries MmsBinaryEncodeUintvarLen fail");
				goto __CATCH;
			}
			if (__MmsBinaryEncodeUintvar(pFile, nEntries, length) == false) {
				MSG_DEBUG("nEntries MmsBinaryEncodeUintvar fail");
				goto __CATCH;
			}

			pType->size = __MmsGetEncodeOffset() - pType->offset;
		}

		if (nEntries > 0) {
			if (nEntries && pBody->pPresentationBody) {
				if (__MmsBinaryEncodeMsgPart(pFile, pType->type, &pBody->presentationType, pBody->pPresentationBody) == false) {
					MSG_DEBUG("__MmsBinaryEncodeMsgPart fail");
					goto __CATCH;
				}

				nEntries--;
			}

			pMultipart = pBody->body.pMultipart;
			while (nEntries && pMultipart) {
				if (__MmsBinaryEncodeMsgPart(pFile, pType->type, &pMultipart->type, pMultipart->pBody) == false) {
					MSG_DEBUG("__MmsBinaryEncodeMsgPart fail");
					goto __CATCH;
				}

				MmsPrintMulitpart(pMultipart, index++);

				pMultipart = pMultipart->pNext;
				nEntries--;
			}
		} else {
			if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
												gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
				MSG_DEBUG("Empty message body MsgWriteDataFromEncodeBuffer fail");
				goto __CATCH;
			}
		}

		pBody->size = __MmsGetEncodeOffset() - pBody->offset;
	} else {
		/* ---------------------------
		 *       Singlepart message
		 * ---------------------------*/
		pType->offset = __MmsGetEncodeOffset();

		if (__MmsBinaryEncodeContentHeader(pFile, (MimeType)pType->type, pType, false) == false)
			goto __CATCH;

		length = __MmsBinaryEncodeContentTypeLen(pType);
		if (length == -1) {
			MSG_DEBUG("Singlepart MmsBinaryEncodeContentTypeLen fail");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeContentType(pFile, pType, length) == false)
			goto __CATCH;

		pType->size = __MmsGetEncodeOffset() - pType->offset;

		if (__MmsBinaryEncodeContentBody(pFile, pBody) == false)
			goto __CATCH;
	}

	return true;

__CATCH:
	MSG_DEBUG("Failed");

	return false;
}

static int __MmsBinaryEncodeContentTypeLen(MsgType *pType)
{
	int length = 0;
	int	totalLength = 0;
	UINT16 fieldValue = 0xffff;
	const char *szTextType = NULL;
	int	contentType = MIME_UNKNOWN;

	bool isAscii = true;
	unsigned  long tmpLength = 0;
	unsigned char tmpFileName[MSG_LOCALE_FILENAME_LEN_MAX+1];

	/*
	 * Content-type-value = Constrained-media | Content-general-form
	 * Constrained-media  = Constrained-encoding
	 *						Constrained-encoding = Extension-Media | Short-integer
	 *						Extension-media = *TEXT End-of-string
	 * Content-general-form = Value-length Media-type
	 *						Media-type = (Well-known-media | Extension-Media) *(Parameter)
	 */

	/* Content-type-value = Content-general-form ------------------------------- */
	/* Content-Type */

	contentType = pType->type;

	fieldValue = MmsGetBinaryValue(MmsCodeContentType, contentType);
	if (fieldValue == UNDEFINED_BINARY || fieldValue == 0x0049) {
		/* Extension-media type */
		szTextType = MmsGetTextValue(MmsCodeContentType, contentType);
		if (szTextType != NULL) {
			length  = __MmsBinaryEncodeTextStringLen((UINT8*)szTextType);
			if (length == -1) {
				MSG_DEBUG("szTextType MmsBinaryEncodeTextStringLen fail");
				goto __CATCH;
			}
			totalLength += length;
		} else {
			totalLength++;
		}
	} else {
		totalLength++;
	}

	/* Parameters -------------------------------------------------------------- */

	/* Well-known-charset = Any-charset | Integer-value ----------------------- */

	if (pType->param.charset != MSG_CHARSET_UNKNOWN) {
		fieldValue = MmsGetBinaryValue(MmsCodeCharSet, pType->param.charset);
		length	   = __MmsBinaryEncodeIntegerLen(fieldValue);
		if (length == -1) {
			MSG_DEBUG("charSet MmsBinaryEncodeIntegerLen fail");
			goto __CATCH;
		}
		totalLength += (length + 1);
	} else {
		if (MmsIsTextType(contentType)) {	/* Any-charset */
			if (!MmsIsVitemContent (contentType, pType->param.szName))
				totalLength += 2;
		}
	}


	/* Name = Text-string ------------------------------------------------------ */

	if (pType->param.szName[0]) {
		char* pszName = NULL;
		if (MmsIsAsciiString(pType->param.szName)) {
			MSG_DEBUG("Name is consisted of ascii char-set chars.");
		} else {
			isAscii = false;
			MSG_DEBUG("Name is NOT consisted of ascii char-set chars.");
		}

		pszName = g_strdup(pType->param.szName);

		if (pszName) {
			/* change empty space to '_' in the file name */
			MmsReplaceSpaceChar(pszName);

			if (isAscii == false) {
				if (MsgEncode2Base64(pszName, strlen(pszName), &tmpLength, tmpFileName) == true) {
					g_free(pszName);
					pszName = g_strdup_printf("=?UTF-8?B?%s?=", tmpFileName);
					MSG_DEBUG("base64 encode filename=[%s]", pszName);
				}
			}

			length = __MmsBinaryEncodeTextStringLen((UINT8*)pszName);

			if (length == -1) {
				MSG_DEBUG("szName MmsBinaryEncodeIntegerLen fail");
				goto __CATCH;
			}

			g_free(pszName);

			totalLength += (length + 1);
		}
	}

#ifdef FEATURE_JAVA_MMS
	if (pType->param.szApplicationID) {
		length = __MmsBinaryEncodeTextStringLen((UINT8*)pType->param.szApplicationID);
		if (length == -1) {
			MSG_DEBUG("szApplicationID MmsBinaryEncodeTextStrinLen fail");
			goto __CATCH;
		}

		totalLength += (length);
		if (MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_APPLICATION_ID) == UNDEFINED_BINARY) {
			totalLength += strlen(MmsGetTextValue(MmsCodeParameterCode, MSG_PARAM_APPLICATION_ID)) + 1; /* NULL */
		} else {
			totalLength++;
		}
	}

	if (pType->param.szReplyToApplicationID) {
		length = __MmsBinaryEncodeTextStringLen((UINT8*)pType->param.szReplyToApplicationID);
		if (length == -1) {
			MSG_DEBUG("szApplicationID MmsBinaryEncodeTextStrinLen fail");
			goto __CATCH;
		}

		totalLength += (length);
		if (MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_REPLY_TO_APPLICATION_ID) == UNDEFINED_BINARY) {
			totalLength += strlen(MmsGetTextValue(MmsCodeParameterCode, MSG_PARAM_REPLY_TO_APPLICATION_ID)) + 1; /* NULL */
		} else {
			totalLength++;
		}
	}
#endif

	/* type, start, & startInfo : multipart/related only parameters -------------- */
	if (contentType == MIME_MULTIPART_RELATED || contentType == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {
		/* type ------------------------------------- */
		fieldValue = MmsGetBinaryValue(MmsCodeContentType, pType->param.type);
		if (fieldValue == UNDEFINED_BINARY || fieldValue == 0x0049) {
			/* Extension-media type */
			szTextType = MmsGetTextValue(MmsCodeContentType, pType->param.type);
			if (szTextType != NULL) {
				length  = __MmsBinaryEncodeTextStringLen((UINT8*)szTextType);
				if (length == -1) {
					MSG_DEBUG("type param MmsBinaryEncodeTextStringLen fail");
					goto __CATCH;
				}
				totalLength += (length + 1);
			} else {
				totalLength += 2;
			}
		} else {
			totalLength += 2;
		}

		/* start = Text-string ----------------------- */
		if (pType->param.szStart[0]) {
			/* start = Text-string */
			length  = __MmsBinaryEncodeTextStringLen((UINT8*)pType->param.szStart);
			if (length == -1) {
				MSG_DEBUG("szStart MmsBinaryEncodeTextStringLen fail");
				goto __CATCH;
			}

			totalLength += (length + 1);
		}


		/* startInfo = Text-string -------------------- */
		if (pType->param.szStartInfo[0]) {
			/* StartInfo (with multipart/related) = Text-string */
			length  = __MmsBinaryEncodeTextStringLen((UINT8*)pType->param.szStartInfo);
			if (length == -1) {
				MSG_DEBUG("szStartInfo MmsBinaryEncodeTextStringLen fail");
				goto __CATCH;
			}

			totalLength += (length + 1);
		}
	}

	return totalLength;

__CATCH:

	MSG_DEBUG("Failed");
	return -1;
}

static bool __MmsBinaryEncodeContentType(FILE *pFile, MsgType *pType, int typeLength)
{
	int length = 0;
	UINT16 fieldValue = 0xffff;
	const char *szTextType = NULL;
	int contentType = MIME_UNKNOWN;

	bool isAscii = true;
	unsigned  long tmpLength = 0;
	unsigned char tmpFileName[MSG_LOCALE_FILENAME_LEN_MAX+1];

#ifdef FEATURE_JAVA_MMS
	const char *szParameter = NULL;
#endif
	/*
	 * Content-type-value = Constrained-media | Content-general-form
	 * Constrained-media  = Constrained-encoding
	 *						Constrained-encoding = Extension-Media | Short-integer
	 *						Extension-media = *TEXT End-of-string
	 * Content-general-form = Value-length Media-type
	 *						Media-type = (Well-known-media | Extension-Media) *(Parameter)
	 */

	if (pFile == NULL) {
		MSG_DEBUG("invalid file handle");
		goto __CATCH;
	}


	/* Content-Type = Content-general-form ------------------------------- */

	length = __MmsBinaryEncodeValueLengthLen(typeLength);
	if (length == -1) {
		MSG_DEBUG("MmsBinaryEncodeValueLengthLen fail.");
		goto __CATCH;
	}

	if (__MmsBinaryEncodeValueLength(pFile, typeLength, length) == false) {
		MSG_DEBUG("MmsBinaryEncodeValueLength fail.");
		goto __CATCH;
	}

	contentType = pType->type;

	MSG_SEC_DEBUG("Content Type : %s", MmsGetTextValue(MmsCodeContentType, (int)contentType));

	fieldValue = MmsGetBinaryValue(MmsCodeContentType, (int)contentType);
	if (fieldValue == UNDEFINED_BINARY || fieldValue == 0x0049) {
		/* Extension-media type */
		szTextType = MmsGetTextValue(MmsCodeContentType, (int)contentType);
		if (szTextType == NULL)
			fieldValue = 0x00;
	}

	if (szTextType == NULL) {
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)fieldValue | 0x80;
	} else {
		length  = __MmsBinaryEncodeTextStringLen((UINT8*)szTextType);
		if (length == -1) {
			MSG_DEBUG("szTextType MmsBinaryEncodeTextStringLen fail");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeTextString(pFile, (UINT8*)szTextType, length) == false) {
			MSG_DEBUG("szTextType MmsBinaryEncodeTextString fail");
			goto __CATCH;
		}
	}

	/* Name = Text-string ------------------------------------------------------ */
	if (pType->param.szName[0]) {
		MSG_SEC_DEBUG("Parameter Name : %s", pType->param.szName);
		char *pszName = NULL;
		if (MmsIsAsciiString(pType->param.szName)) {
			MSG_DEBUG("Name is consisted of ascii char-set chars.");
		} else {
			isAscii = false;
			MSG_DEBUG("Name is NOT consisted of ascii char-set chars.");
		}

		pszName = g_strdup(pType->param.szName);

		/* change empty space to '_' in the file name */
		MmsReplaceSpaceChar(pszName);

		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_NAME) | 0x80;

		if (isAscii == false) {
			if (MsgEncode2Base64(pszName, strlen(pszName), &tmpLength, tmpFileName) == true) {
				g_free(pszName);
				pszName = g_strdup_printf("=?UTF-8?B?%s?=", tmpFileName);
				MSG_DEBUG("base64 encode filename=[%s]", pszName);
			}
		}

		length = __MmsBinaryEncodeTextStringLen((UINT8*)pszName);

		if (length == -1) {
			MSG_DEBUG("szName MmsBinaryEncodeIntegerLen fail");
			free(pszName);
			goto __CATCH;
		}

		if (__MmsBinaryEncodeTextString(pFile, (UINT8*)pszName, length) == false) {
			MSG_DEBUG("szName MmsBinaryEncodeTextString fail");
			free(pszName);
			goto __CATCH;
		}
		free(pszName);
	}
#ifdef FEATURE_JAVA_MMS

	/* Application-ID: Text-string */
	if (pType->param.szApplicationID) {
		MSG_SEC_DEBUG("Parameter Application ID : %s", pType->param.szApplicationID);
		length = __MmsBinaryEncodeTextStringLen((UINT8*) pType->param.szApplicationID);
		if (length == -1) {
			MSG_DEBUG("szApplicationID MmsBinaryEncodeIntegerLen Fail");
			goto __CATCH;
		}

		fieldValue = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_APPLICATION_ID);

		if (fieldValue == UNDEFINED_BINARY)
			szParameter = MmsGetTextValue(MmsCodeParameterCode, MSG_PARAM_APPLICATION_ID);

		if (szParameter == NULL) {
			MSG_DEBUG("szParameter is NULL");
			goto __CATCH;
		}

		strncpy(gpMmsEncodeBuf + gCurMmsEncodeBuffPos, (char*)szParameter, strlen(szParameter));
		gCurMmsEncodeBuffPos += strlen(szParameter);
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] =(UINT8)NULL;

		if (__MmsBinaryEncodeTextString(pFile, (UINT8*) pType->param.szApplicationID, length) == false) {
			MSG_DEBUG(" szApplicationID MmsBinaryEncodeTextString fail");
			goto __CATCH;
		}
	}

	/* ReplyToApplicationID: Text-string */
	if (pType->param.szReplyToApplicationID) {
		MSG_SEC_DEBUG("Parameter Reply To Application-ID : %s", pType->param.szReplyToApplicationID);
		length = __MmsBinaryEncodeTextStringLen((UINT8*) pType->param.szReplyToApplicationID);
		if (length == -1) {
			MSG_DEBUG("szReplyToApplicationID MmsBinaryEncodeIntegerLen Fail");
			goto __CATCH;
		}

		fieldValue = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_REPLY_TO_APPLICATION_ID);

		if (fieldValue == UNDEFINED_BINARY)
			szParameter = MmsGetTextValue(MmsCodeParameterCode, MSG_PARAM_REPLY_TO_APPLICATION_ID);

		if (szParameter == NULL) {
			MSG_DEBUG("szParameter is NULL");
			goto __CATCH;
		}

		strncpy(gpMmsEncodeBuf + gCurMmsEncodeBuffPos, (char*)szParameter, strlen(szParameter));
		gCurMmsEncodeBuffPos += strlen(szParameter);
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] =(UINT8)NULL;

		if (__MmsBinaryEncodeTextString(pFile, (UINT8*) pType->param.szReplyToApplicationID, length) == false) {
			MSG_DEBUG("szApplicationID MmsBinaryEncodeTextString fail");
			goto __CATCH;
		}
	}
#endif

	/* Well-known-charset = Any-charset | Integer-value ----------------------- */

	if (pType->param.charset != MSG_CHARSET_UNKNOWN) {
		MSG_DEBUG("Parameter Charset : %d", pType->param.charset);
		fieldValue = MmsGetBinaryValue(MmsCodeCharSet, pType->param.charset);
		length = __MmsBinaryEncodeIntegerLen(fieldValue);
		if (length == -1) {
			MSG_DEBUG("charSet MmsBinaryEncodeIntegerLen fail");
			goto __CATCH;
		}

		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_CHARSET) | 0x80;
		if (__MmsBinaryEncodeInteger(pFile, fieldValue, length) == false) {
			MSG_DEBUG("charSet MmsBinaryEncodeInteger fail");
			goto __CATCH;
		}
	} else {
		/* Any-charset */
		if (MmsIsTextType(contentType)) {
			if (!MmsIsVitemContent(contentType, pType->param.szName)) {
				gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_CHARSET) | 0x80;
				fieldValue = 0x0000;	/* laconic_warning, just to remove warning message */
				gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)fieldValue | 0x80;
			}
		}
	}

	/* type, start, & startInfo : multipart/related only parameters -------------- */
	if (contentType == MIME_MULTIPART_RELATED || contentType == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {
		MSG_SEC_DEBUG("Parameter Content Type : %s", MmsGetTextValue(MmsCodeContentType, pType->param.type));
		/* type ------------------------------------- */
		fieldValue = MmsGetBinaryValue(MmsCodeContentType, pType->param.type);
		if (fieldValue == UNDEFINED_BINARY || fieldValue == 0x0049) {
			/* Extension-media type */
			szTextType = MmsGetTextValue(MmsCodeContentType, pType->param.type);
			if (szTextType == NULL)
				fieldValue = 0x00;
		}

		if (szTextType == NULL) {
			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)fieldValue | 0x80;
			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_TYPE) | 0x80;
		} else {
			length  = __MmsBinaryEncodeTextStringLen((UINT8*)szTextType);
			if (length == -1) {
				MSG_DEBUG("type param MmsBinaryEncodeTextStringLen fail");
				goto __CATCH;
			}

			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_TYPE) | 0x80;
			if (__MmsBinaryEncodeTextString(pFile, (UINT8*)szTextType, length) == false) {
				MSG_DEBUG("type param MmsBinaryEncodeTextString fail");
				goto __CATCH;
			}
		}

		/* start = Text-string ----------------------- */
		if (pType->param.szStart[0]) {
			/* start = Text-string */
			MSG_DEBUG("Parameter Start: %s", pType->param.szStart);
			length  = __MmsBinaryEncodeTextStringLen((UINT8*)pType->param.szStart);
			if (length == -1) {
				MSG_DEBUG("szStart MmsBinaryEncodeTextStringLen fail");
				goto __CATCH;
			}

			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeParameterCode,
																		 MSG_PARAM_START) | 0x80;
			if (__MmsBinaryEncodeTextString(pFile, (UINT8*)pType->param.szStart, length) == false) {
				MSG_DEBUG("szStart MmsBinaryEncodeTextString fail");
				goto __CATCH;
			}
		}

		/* startInfo = Text-string -------------------- */
		if (pType->param.szStartInfo[0]) {
			/* StartInfo (with multipart/related) = Text-string */
			MSG_DEBUG("Parameter StartInfo: %s", pType->param.szStartInfo);
			length  = __MmsBinaryEncodeTextStringLen((UINT8*)pType->param.szStartInfo);
			if (length == -1) {
				MSG_DEBUG("szStartInfo MmsBinaryEncodeTextStringLen fail");
				goto __CATCH;
			}

			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_START_INFO) | 0x80;
			if (__MmsBinaryEncodeTextString(pFile, (UINT8*)pType->param.szStartInfo, length) == false) {
				MSG_DEBUG("szStartInfo MmsBinaryEncodeTextString fail");
				goto __CATCH;
			}
		}
	}

	return true;

__CATCH:
	MSG_DEBUG("Failed");
	return false;
}

static bool __MmsBinaryEncodeMsgPart(FILE *pFile, int contentType, MsgType *pType, MsgBody *pBody)
{
	FILE *pFile2 = NULL;
	char *pData = NULL;
	int	length = 0;


	if (pType->offset && pType->size) {
		/* header length & body length --------------------------- */

		length = __MmsBinaryEncodeUintvarLen(pType->size);
		if (length == -1) {
			MSG_DEBUG("1. headerLeng MmsBinaryEncodeUintvarLen fail");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeUintvar(pFile, pType->size, length) == false) {
			MSG_DEBUG("1. eaderLeng fail");
			goto __CATCH;
		}

		length = __MmsBinaryEncodeUintvarLen(pBody->size);
		if (length == -1) {
			MSG_DEBUG("1. bodyLeng MmsBinaryEncodeUintvarLen fail");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeUintvar(pFile, pBody->size, length) == false) {
			MSG_DEBUG("1. bodyLeng fail");
			goto __CATCH;
		}

		pFile2 = MsgOpenFile(pType->szOrgFilePath, "rb");
		if (pFile != NULL) {
			pData = (char *)calloc(1, pType->size);
			if (pData == NULL)
				goto __CATCH;

			if (MsgFseek(pFile2, pType->offset, SEEK_SET) < 0) {
				MSG_DEBUG("MsgFseek fail");
				goto __CATCH;
			}

			ULONG nRead = 0;

			if ((nRead = MsgReadFile(pData, sizeof(char), pType->size, pFile2)) == 0) {
				if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
													gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
					MSG_DEBUG("1. header MsgWriteDataFromEncodeBuffer fail");
					goto __CATCH;
				}
				pType->offset = __MmsGetEncodeOffset();
				if (MsgWriteFile(pData, sizeof(char), nRead, pFile) != (size_t)nRead) {
					MSG_DEBUG("MsgWriteFile failed");
					goto __CATCH;
				}
				gMmsEncodeCurOffset = MsgFtell(pFile);
				if (gMmsEncodeCurOffset < 0) {
					MSG_DEBUG("MsgFtell returns negative value [%ld]", gMmsEncodeCurOffset);
					goto __CATCH;
				}
			}

			MsgCloseFile(pFile2);
			pFile2 = NULL;
		}
	} else {
		int headerLeng	   = 0;
		int contentTypeLen = 0;
		int contentHdrLen  = 0;

		/* header length & body length --------------------------- */
		contentTypeLen = __MmsBinaryEncodeContentTypeLen(pType);
		length = __MmsBinaryEncodeValueLengthLen(contentTypeLen);
		contentHdrLen  = __MmsBinaryEncodeContentHeaderLen((MimeType)contentType, pType, true);

		if (contentTypeLen == -1 || length == -1 || contentHdrLen == -1) {
			MSG_DEBUG("headerLeng calculation fail");
			goto __CATCH;
		}

		headerLeng = contentTypeLen + contentHdrLen + length;
		length = __MmsBinaryEncodeUintvarLen(headerLeng);
		if (length == -1) {
			MSG_DEBUG("headerLeng MmsBinaryEncodeUintvarLen fail");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeUintvar(pFile, headerLeng, length) == false) {
			MSG_DEBUG("headerLeng fail");
			goto __CATCH;
		}

		length = __MmsBinaryEncodeUintvarLen(pBody->size);
		if (length == -1) {
			MSG_DEBUG("bodyLeng MmsBinaryEncodeUintvarLen fail");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeUintvar(pFile, pBody->size, length) == false) {
			MSG_DEBUG("bodyLeng fail");
			goto __CATCH;
		}

		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
							 				gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("2. header MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}

		/* content-type & header --------------------------- */
		pType->offset = __MmsGetEncodeOffset();

		if (__MmsBinaryEncodeContentType(pFile, pType, contentTypeLen) == false) {
			MSG_DEBUG("MmsBinaryEncodeContentType fail");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeContentHeader(pFile, (MimeType)contentType, pType, true) == false) {
			MSG_DEBUG("MmsBinaryEncodeContentHeader fail");
			goto __CATCH;
		}

		pType->size = __MmsGetEncodeOffset() - pType->offset;
	}

	if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos, gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
		MSG_DEBUG("contentBody MsgWriteDataFromEncodeBuffer fail");
		goto __CATCH;
	}

	/* content-body --------------------------- */
	if (__MmsBinaryEncodeContentBody(pFile, pBody) == false) {
		MSG_DEBUG("__MmsBinaryEncodeContentBody fail");
		goto __CATCH;
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

	MSG_DEBUG("Failed");
	if (pFile2) {
		MsgCloseFile(pFile2);
		pFile2 = NULL;
	}

	return false;
}

static int __MmsBinaryEncodeContentHeaderLen(MimeType contentType, MsgType *pType, bool bMultipart)
{
	int	length = 0;
	int	totalLength = 0;
	const char *szTextValue = NULL;

	/* content-id ------------------------------------------------- */
	if (pType->szContentID[0]) {
		if (bMultipart) { /* Binary Encoding */
			totalLength++;
		} else {
			/* content-id = Quoted-string */
			length = __MmsBinaryEncodeTextStringLen((UINT8*)"Content-ID");
			if (length == -1) {
				MSG_DEBUG("Content-ID MmsBinaryEncodeTextStringLen fail.");
				goto __CATCH;
			}

			totalLength += length;
		}

		length = __MmsBinaryEncodeQuotedStringLen((UINT8*)pType->szContentID);
		if (length == -1) {
			MSG_DEBUG("pType->szContentID MmsBinaryEncodeQuotedStringLen fail.");
			goto __CATCH;
		}
		totalLength += length;
	}


	if (pType->szContentLocation[0]) {
		if (bMultipart) { /* Binary Encoding */
			totalLength++;
		} else {
			/* content-location = Quoted-string */
			length = __MmsBinaryEncodeTextStringLen((UINT8*)"Content-Location");
			if (length == -1) {
				MSG_DEBUG("Content-Location MmsBinaryEncodeTextStringLen fail.");
				goto __CATCH;
			}

			totalLength += length;
		}

		length = __MmsBinaryEncodeTextStringLen((UINT8*)pType->szContentLocation);
		if (length == -1) {
			MSG_DEBUG("pType->szContentLocation MmsBinaryEncodeTextStringLen fail.");
			goto __CATCH;
		}

		totalLength += length;
	}


	/* MIME_APPLICATION_VND_WAP_MULTIPART_RELATEDrequires always "inline" */

	if (contentType != MIME_APPLICATION_VND_WAP_MULTIPART_RELATED &&
		contentType != MIME_MULTIPART_RELATED &&
		pType->disposition != INVALID_VALUE) {
		/*
		 * Content-disposition-value = Value-length Disposition *(Parameter)
		 * Disposition = Form-data | Attachment | Inline | Token-text
		 * Form-data = <Octet 128> : 0x80
		 * Attachment = <Octet 129> : 0x81
		 * Inline = <Octet 130> : 0x82
		 */

		if (bMultipart) { /* Binary Encoding */
			totalLength += 3;
		} else {
			/* content-disposition = Quoted-string */
			szTextValue = MmsGetTextValue(MmsCodeMsgDisposition, pType->disposition);

			if (szTextValue) {
				length = __MmsBinaryEncodeTextStringLen((UINT8*)"Content-Disposition");
				if (length == -1) {
					MSG_DEBUG("Content-Disposition MmsBinaryEncodeTextStringLen fail.");
					goto __CATCH;
				}

				totalLength += length;

				length = __MmsBinaryEncodeTextStringLen((UINT8*)szTextValue);
				if (length == -1) {
					MSG_DEBUG("Content-Disposition MmsBinaryEncodeTextStringLen fail.");
					goto __CATCH;
				}
				totalLength += length;
			}
		}
	}

	return totalLength;

__CATCH:
	MSG_DEBUG("Failed");
	return -1;
}

static bool __MmsBinaryEncodeContentHeader(FILE *pFile, MimeType contentType, MsgType *pType, bool bMultipart)
{
	int	length = 0;
	const char *szTextValue = NULL;
	bool isAscii = true;
	unsigned long tmpLength = 0;
	unsigned char tmpFileName[MSG_LOCALE_FILENAME_LEN_MAX+1];

	/* content-id ------------------------------------------------- */
	if (pType->szContentID[0]) {
		MSG_SEC_DEBUG("Content ID : %s", pType->szContentID);

		if (bMultipart) { /* Binary Encoding */
			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeMsgBodyHeaderCode, MMS_BODYHDR_CONTENTID) | 0x80;
		} else {
			/* content-id = Quoted-string */
			length = __MmsBinaryEncodeTextStringLen((UINT8*)"Content-ID");
			if (length == -1) {
				MSG_DEBUG("Content-ID MmsBinaryEncodeTextStringLen fail.");
				goto __CATCH;
			}

			if (__MmsBinaryEncodeTextString(pFile, (UINT8*)"Content-ID", length) == false) {
				MSG_DEBUG("Content-ID MmsBinaryEncodeTextString fail.");
				goto __CATCH;
			}
		}

		length = __MmsBinaryEncodeQuotedStringLen((UINT8*)pType->szContentID);
		if (length == -1) {
			MSG_DEBUG("pType->szContentID MmsBinaryEncodeQuotedStringLen fail.");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeQuotedString(pFile, (UINT8*)pType->szContentID, length) == false) {
			MSG_DEBUG("pType->szContentID MmsBinaryEncodeQuotedString fail.");
			goto __CATCH;
		}
	}

	if (pType->szContentLocation[0]) {
		MSG_SEC_DEBUG("Content Location : %s", pType->szContentLocation);

		if (MmsIsAsciiString(pType->szContentLocation)) {
			MSG_DEBUG("Name is consisted of ascii char-set chars.");
		} else {
			isAscii = false;
			MSG_DEBUG("Name is NOT consisted of ascii char-set chars.");
		}

		if (bMultipart) { /* Binary Encoding */
			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeMsgBodyHeaderCode, MMS_BODYHDR_CONTENTLOCATION) | 0x80;
		} else {
			/* content-location = Quoted-string */
			length = __MmsBinaryEncodeTextStringLen((UINT8*)"Content-Location");
			if (length == -1) {
				MSG_DEBUG("Content-Location MmsBinaryEncodeTextStringLen fail.");
				goto __CATCH;
			}

			if (__MmsBinaryEncodeTextString(pFile, (UINT8*)"Content-Location", length) == false) {
				MSG_DEBUG("Content-Location MmsBinaryEncodeTextString fail.");
				goto __CATCH;
			}
		}

		char* pszName = g_strdup(pType->szContentLocation);
		MmsReplaceSpaceChar(pszName);

		if (isAscii == false) {
			if (MsgEncode2Base64(pszName, strlen(pszName), &tmpLength, tmpFileName) == true) {
				g_free(pszName);
				pszName = g_strdup_printf("=?UTF-8?B?%s?=", tmpFileName);
				MSG_DEBUG("base64 encode filename=[%s]", pszName);
			}
		}

		length = __MmsBinaryEncodeTextStringLen((UINT8*)pszName);
		if (length == -1) {
			MSG_DEBUG("pType->szContentLocation MmsBinaryEncodeTextStringLen fail.");
			g_free(pszName);
			goto __CATCH;
		}

		if (__MmsBinaryEncodeTextString(pFile, (UINT8*)pszName, length) == false) {
			MSG_DEBUG("pType->szContentLocation MmsBinaryEncodeTextString fail.");
			g_free(pszName);
			goto __CATCH;
		}

		g_free(pszName);
	}

	/* MIME_APPLICATION_VND_WAP_MULTIPART_RELATED requires always "inline" */

	if (contentType != MIME_APPLICATION_VND_WAP_MULTIPART_RELATED &&
		contentType != MIME_MULTIPART_RELATED &&
		pType->disposition != INVALID_VALUE) {
		MSG_DEBUG("Content Disposition : %d", pType->disposition);
		/*
		 * Content-disposition-value = Value-length Disposition *(Parameter)
		 * Disposition = Form-data | Attachment | Inline | Token-text
		 * Form-data = <Octet 128> : 0x80
		 * Attachment = <Octet 129> : 0x81
		 * Inline = <Octet 130> : 0x82
		 */

		if (bMultipart) { /* Binary Encoding */
			UINT8 fieldValue  = 0xff;
			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeMsgBodyHeaderCode, MMS_BODYHDR_DISPOSITION) | 0x80;

			fieldValue = MmsGetBinaryValue(MmsCodeMsgDisposition, pType->disposition) | 0x80;

			if (fieldValue == 0xff)
				fieldValue = 0x81;

			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = 0x01;
			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = fieldValue;
		} else {
			/* content-disposition = Disposition (no support parameter) */

			szTextValue = MmsGetTextValue(MmsCodeMsgDisposition, pType->disposition);

			if (szTextValue) {
				length = __MmsBinaryEncodeTextStringLen((UINT8*)"Content-Disposition");
				if (length == -1) {
					MSG_DEBUG("Content-Disposition MmsBinaryEncodeTextStringLen fail.");
					goto __CATCH;
				}

				if (__MmsBinaryEncodeTextString(pFile, (UINT8*)"Content-Disposition", length) == false) {
					MSG_DEBUG("Content-Disposition MmsBinaryEncodeTextString fail.");
					goto __CATCH;
				}

				length = __MmsBinaryEncodeTextStringLen((UINT8*)szTextValue);
				if (length == -1) {
					MSG_DEBUG("Content-Disposition MmsBinaryEncodeTextStringLen fail.");
					goto __CATCH;
				}

				if (__MmsBinaryEncodeTextString(pFile, (UINT8*)szTextValue, length) == false) {
					MSG_DEBUG("Content-Disposition MmsBinaryEncodeTextString fail.");
					goto __CATCH;
				}
			}
		}
	}

	return true;

__CATCH:
	MSG_DEBUG("Failed");
	return false;
}

static bool __MmsBinaryEncodeContentBody(FILE *pFile, MsgBody *pBody)
{
	int nRead = 0;
	char *pData = NULL;


	if (pFile == NULL)
		goto __CATCH;

	if (pBody == NULL)
		return true;

	if (pBody->szOrgFilePath[0]) {
		pData = MsgOpenAndReadMmsFile(pBody->szOrgFilePath, pBody->offset, pBody->size, &nRead);
		if (pData == NULL)
			goto __CATCH;

		pBody->offset = __MmsGetEncodeOffset();
		if (MsgWriteFile(pData, sizeof(char), nRead, pFile) != (size_t)nRead) {
			MSG_DEBUG("MsgWriteFile failed");
			goto __CATCH;
		}
		gMmsEncodeCurOffset = MsgFtell(pFile);

		if (gMmsEncodeCurOffset < 0) {
			MSG_DEBUG("MsgFtell returns negative value [%ld]", gMmsEncodeCurOffset);
			goto __CATCH;
		}

		if (pData) {
			free(pData);
			pData = NULL;
		}
	} else if (pBody->body.pText && pBody->size) {
		pBody->offset = __MmsGetEncodeOffset();
		if (MsgWriteFile(pBody->body.pText, sizeof(char), (size_t)pBody->size, pFile) != (size_t)pBody->size) {
			MSG_DEBUG("MsgWriteFile failed");
			goto __CATCH;
		}
		gMmsEncodeCurOffset = MsgFtell(pFile);

		if (gMmsEncodeCurOffset < 0) {
			MSG_DEBUG("MsgFtell returns negative value [%ld]", gMmsEncodeCurOffset);
			goto __CATCH;
		}
	}

	return true;

__CATCH:
	MSG_DEBUG("##failed");
	if (pData) {
		free(pData);
		pData = NULL;
	}

	return false;
}

/* =========================================================================

   B  I  N  A  R  Y       E  N  C  O  D  I  N  G      U  T  I  L  I  T  Y

   =========================================================================*/
static int __MmsBinaryEncodeIntegerLen(UINT32 integer)
{
	if (integer < 0x80) {
		/* short-integer */
		return 1;
	} else {
		int	length = 0;
		/*
		 * Long-integer = Short-length Multi-octet-integer
		 * The Short-length indicates the length of the Multi-octet-integer
		 */

		while (integer) {
			length++;
			integer = (integer >> 8);
		}

		length++;	/* + Short-length */

		return length;
	}
}


/*
 * This makes value-length by specified integer value
 *
 * @param 	length [in] gotten from MmsBinaryEncodeIntegerLen()
 */
static bool __MmsBinaryEncodeInteger(FILE *pFile, UINT32 integer, int length)
{
	union {
		UINT32 integer;
		UINT8 seg[4];
	} changer;


	if (pFile == NULL) {
		MSG_DEBUG("source == NULL");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	if (integer < 0x80) {
		/* short-integer */
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)integer | 0x80;
	} else {
		/*
		 * Long-integer = Short-length Multi-octet-integer
		 * The Short-length indicates the length of the Multi-octet-integer
		 */
		changer.integer = integer;
		length--;					/* - "Short-length" */

		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)length;

		int i;
		for(i = 0; i < length; i++)
			gpMmsEncodeBuf[gCurMmsEncodeBuffPos + i] = changer.seg[length - (i + 1)];

		gCurMmsEncodeBuffPos += length;
	}

	return true;

__CATCH:
	return false;
}

static int __MmsBinaryEncodeLongIntegerLen(UINT32 integer)
{
	int length = 0;

	/*
	 * Long-integer = Short-length Multi-octet-integer
	 * The Short-length indicates the length of the Multi-octet-integer
	 */

	if (integer == 0)
		return 2;

	while (integer) {
		length++;
		integer = (integer >> 8);
	}

	length++;	/* + Short-length */

	return length;
}

/*
 * This makes value-length by specified integer value
 *
 * @param 	length [in] gotten from MmsBinaryEncodeIntegerLen()
 */
static bool __MmsBinaryEncodeLongInteger(FILE *pFile, UINT32 integer, int length)
{
	int i  = 0;
	union {
		UINT32 integer;
		UINT8 seg[4];
	}changer;


	/*
	 * Long-integer = Short-length Multi-octet-integer
	 * The Short-length indicates the length of the Multi-octet-integer
	 */

	if (pFile == NULL) {
		MSG_DEBUG("pFile == NULL");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	changer.integer = integer;
	length--;					/* - "Short-length" */

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)length;

	for(i = 0; i < length; i++)
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos + i] = changer.seg[length - (i + 1)];

	gCurMmsEncodeBuffPos += length;

	return true;

__CATCH:
	return false;
}

static int __MmsBinaryEncodeTextStringLen(UINT8 *source)
{
	int	 length = 0;

	if (source == NULL) {
		MSG_DEBUG("source == NULL");
		return -1;
	}

	length = (int)strlen((char*)source);
	if (source[0] > 0x7F) {
		length += 2;			/* + NULL */
	} else {
		length++;				/* + NULL */
	}

	return length;
}

/*
 * @param 	source [in] originam string
 * @param	length [in] gotten from MmsBinaryEncodeTextStringLen()
 * @param	dest [in] buffer to store quted string
 * @return 	changed string length
*/
static bool __MmsBinaryEncodeTextString(FILE *pFile, UINT8 *source, int length)
{
	/*
	 * make text string
	 * Text-string = [Quote] *TEXT End-of-string
	 * If the 1st char in the TEXT is in the range of 128-255, a Quote char must precede it.
	 * Otherwise the Quote char must be omitted.
	 * The Quote is not part of the contents.
	 * Quote = <Octet 127>
	 */

	if (pFile == NULL || source == NULL) {
		MSG_DEBUG("pFile == %p, source = %p", pFile, source);
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	if (source[0] > 0x7F) {
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = QUOTE;
		length--;
	}

	strncpy(gpMmsEncodeBuf + gCurMmsEncodeBuffPos, (char*)source, (length - 1));	/* except NULL */
	gCurMmsEncodeBuffPos += (length - 1);			/* except NULL */
	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)NULL;

	return true;

__CATCH:
	return false;
}

/*
 * Encode 28bit unsigned integer(Maximum) to uintvar
 *
 * @param 	interger [in] integer to be encoded
 * @return 	encoded UINTVAR stream
*/
const UINT32 UINTVAR_LENGTH_1 =  0x0000007f;		/* 7bit */
const UINT32 UINTVAR_LENGTH_2 =  0x00003fff;		/* 14bit */
const UINT32 UINTVAR_LENGTH_3 =  0x001fffff;		/* 21bit */


static int __MmsBinaryEncodeUintvarLen(UINT32 integer)
{
	UINT32		length	= 0;

	/* Find encoded unitvar length */
	if (integer  <= UINTVAR_LENGTH_1) {
		length = 1;
	} else {
		if (integer <= UINTVAR_LENGTH_2) {
			length = 2;
		} else {
			if (integer <= UINTVAR_LENGTH_3) {
				length = 3;
			} else {
				length = 4;
			}
		}
	}

	return length;
}

static bool __MmsBinaryEncodeUintvar(FILE *pFile, UINT32 integer, int length)
{
	const char ZERO	= 0x00;
	int i = 2;
	char szReverse[MSG_STDSTR_LONG] = {0, };

	union {
		UINT32	integer;
		char	bytes[4];
	} source;

	if (pFile == NULL) {
		MSG_DEBUG("pFile == INVALID_HBOJ");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	source.integer = integer;
	memset(szReverse, 0, MSG_STDSTR_LONG);

	/* Seperate integer to 4 1 byte integer */
	szReverse[3] = source.bytes[3] & 0x0f;
	szReverse[0] = source.bytes[0];
	szReverse[0] = szReverse[0] & 0x7f;

	while (length >= i) { /* initially, i = 2 */
		/* Move integer bit to proper position */
		source.integer = source.integer << 1;
		source.integer = source.integer >> 8;
		source.bytes[3] = ZERO;

		/* Retrive 1 encode uintvar */
		szReverse[i-1] = source.bytes[0];
		szReverse[i-1] = szReverse[i-1] | 0x80;
		i++;
	}

	for (i = 0; i < length; i++)
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = szReverse[length - i - 1];

	return true;

__CATCH:
	return false;
}

static int __MmsBinaryEncodeValueLengthLen(UINT32 integer)
{
	int length = 0;

	if (integer < 0x1f) {
		length = 1;
	} else {
		length = __MmsBinaryEncodeUintvarLen(integer) + 1;		/* LENGTH_QUOTE */
	}

	return length;
}

/*
 * This makes value-length by specified integer value
 *
 * @param 	length [in] from MmsBinaryEncodeValueLengthLen()
 * @return 	encoded value-length
 */
static bool __MmsBinaryEncodeValueLength(FILE *pFile, UINT32 integer, int length)
{
	/*
	 * Value-length = Short-length | (Length-quote Length)
	 * ; Value length is used to indicate the length of the value to follow
	 * Short-length = <Any octet 0-30>
	 * Length-quote = <Octet 31>
	 * Length = Uintvar-integer
	 */

	if (pFile == NULL) {
		MSG_DEBUG("pFile == INVALID_HBOJ");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	if (integer < 0x1F) {
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)integer;
	} else {
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)LENGTH_QUOTE;
		if (__MmsBinaryEncodeUintvar(pFile, integer, length - 1) == false) {	/* LENGTH_QUOTE */
			MSG_DEBUG("MmsBinaryEncodeUintvar fail");
			goto __CATCH;
		}
	}

	return true;

__CATCH:
	return false;
}

static int __MmsBinaryEncodeQuotedStringLen(UINT8 *pSrc)
{
	if (pSrc == NULL) {
		MSG_DEBUG("invalid file");
		goto __CATCH;
	}

	return (strlen((char*)pSrc) + 2);	/* QUOTE + NULL */

__CATCH:
	return -1;
}

/*
 * make quoted string
 * Quoted-string = <Octet 34> *TEXT End-of-string
 *
 * @param 	source [in] original string
 * @param	length [in] length (in bytes) of data
 * @param	dest [out] buffer to store quted string
 * @return 	changed string length
*/
static bool __MmsBinaryEncodeQuotedString(FILE *pFile, UINT8 *source, int length)
{
	if (source == NULL || pFile == NULL) {
		MSG_DEBUG("invalid file");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = '\"';
	strncpy(gpMmsEncodeBuf + gCurMmsEncodeBuffPos, (char*)source, length - 2);		/* except '\"' & NULL */
	gCurMmsEncodeBuffPos += (length - 2);
	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)NULL;

	return true;

__CATCH:
	return false;
}

static int __MmsBinaryEncodeEncodedStringLen(UINT8 *source)
{
	UINT32 charset = 0x6A;		/* default = utf-8 */
	int charLeng		= 0;
	int textLeng		= 0;
	int valueLengthLen	= 0;

	/* value-length charSet text-string */
	/* Estimate charset value length and text string length */
	charLeng = __MmsBinaryEncodeIntegerLen(charset);
	if (charLeng == -1) {
		MSG_DEBUG(" charLeng MmsBinaryEncodeTextStringLen fail.");
		goto __CATCH;
	}

	textLeng = __MmsBinaryEncodeTextStringLen((UINT8*)source);
	if (textLeng == -1) {
		MSG_DEBUG(" textLeng MmsBinaryEncodeTextStringLen fail.");
		goto __CATCH;
	}

	valueLengthLen = __MmsBinaryEncodeValueLengthLen(charLeng + textLeng);
	if (valueLengthLen == -1) {
		MSG_DEBUG(" valLengthLen MmsBinaryEncodeTextStringLen fail.");
		goto __CATCH;
	}

	return (charLeng + textLeng + valueLengthLen);

__CATCH:
	return -1;
}

/*
 * This makes value-length by specified integer value
 *
 * @param 	length [in] from MmsBinaryEncodeEncodedStringLen()
 * @return 	encoded encoded-string
 */
static bool __MmsBinaryEncodeEncodedString(FILE *pFile, UINT8 *source, int length)
{
	UINT32 charset = 0x6A;		/* default = utf-8 */
	int charLeng = 0;
	int textLeng = 0;
	int valLengthLen = 0;

	/* value-length charSet text-string */
	if (pFile == NULL || source == NULL) {
		MSG_DEBUG("invalid input parameter");
		goto __CATCH;
	}

	/* Estimate charset value length and text string length */
	charLeng = __MmsBinaryEncodeIntegerLen(charset);
	if (charLeng == -1) {
		MSG_DEBUG("charLeng MmsBinaryEncodeTextStringLen fail.");
		goto __CATCH;
	}

	textLeng = __MmsBinaryEncodeTextStringLen((UINT8*)source);
	if (textLeng == -1) {
		MSG_DEBUG("textLeng MmsBinaryEncodeTextStringLen fail.");
		goto __CATCH;
	}

	valLengthLen = __MmsBinaryEncodeValueLengthLen(charLeng + textLeng);
	if (valLengthLen == -1) {
		MSG_DEBUG("MmsBinaryEncodeValueLengthLen fail.");
		goto __CATCH;
	}

	if (length != (charLeng + textLeng + valLengthLen)) {
		MSG_DEBUG("invalid length");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	/* Value length of charset value and text string */
	if (__MmsBinaryEncodeValueLength(pFile, charLeng + textLeng, valLengthLen) == false) {
		MSG_DEBUG("MmsBinaryEncodeValueLength fail.");
		goto __CATCH;
	}

	/* fixme: Write charset on buffer -> integer value not long-integer */
	if (__MmsBinaryEncodeInteger(pFile, charset, charLeng) == false) {
		MSG_DEBUG("MmsBinaryEncodeInteger fail.");
		goto __CATCH;
	}

	/* Write text string on buffer */
	if (__MmsBinaryEncodeTextString(pFile, (UINT8*)source, textLeng) == false) {
		MSG_DEBUG("MmsBinaryEncodeTextString fail.");
		goto __CATCH;
	}

	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeFieldCodeAndValue(FILE *pFile, UINT8 fieldCode, UINT8 fieldValue)
{
	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < 2) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
										gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	if (fieldCode == 0xff) {
		MSG_DEBUG("invalid fieldCode");
		goto __CATCH;
	}

	if (fieldValue == 0xff) {
		MSG_DEBUG("invalid fieldValue");
		return true;
	}

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = fieldCode;
	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = fieldValue;

	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeTrID(FILE *pFile, char *szTrID, int bufLen)
{
	int	length	  = 0;
	UINT8 fieldCode = 0xff;
	char szBuff[MMS_TR_ID_LEN + 1] = {0, };
	struct tm dateTime;
	time_t RawTime = 0;
	time_t dateSec = 0;

	time(&RawTime);
	localtime_r(&RawTime, &dateTime);

	dateSec = mktime(&dateTime);

	fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_TRID) | 0x80;
	if (fieldCode == 0xff) {
		MSG_DEBUG("Invalid fieldCode[0x%02x]", fieldCode & 0x7F);
		goto __CATCH;
	}

	if (szTrID && strlen(szTrID)) {
		snprintf(szBuff, MMS_TR_ID_LEN + 1, "%s", szTrID);
	} else {
		snprintf(szBuff, MMS_TR_ID_LEN + 1, "%lu.%lu", dateSec, (unsigned long)random());
		MSG_SEC_DEBUG("Generated Transaction ID = %s", szBuff);
	}

	length = __MmsBinaryEncodeTextStringLen((UINT8*)szBuff);
	if (length == -1) {
		MSG_DEBUG("MmsBinaryEncodeTextStringLen fail");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < (length + 1)) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = fieldCode;
	if (__MmsBinaryEncodeTextString(pFile, (UINT8*)szBuff, length) == false) {
		MSG_DEBUG("MmsBinaryEncodeTextString fail");
		goto __CATCH;
	}

	if (szTrID) {
		memset(szTrID, 0, bufLen);
		strncpy(szTrID, szBuff, bufLen - 1);
	}

	MSG_SEC_INFO("X-Mms-Transaction-Id = [%s]", szBuff);
	return true;

__CATCH:

	return false;
}

static bool __MmsBinaryEncodeMsgID(FILE *pFile, const char *szMsgID)
{
	int	length = 0;
	UINT8 fieldCode = 0xff;

	fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGID) | 0x80;
	if (fieldCode == 0xff) {
		MSG_DEBUG("invalid fieldCode");
		goto __CATCH;
	}

	MSG_DEBUG("2. szBuff = %s", szMsgID);

	length = __MmsBinaryEncodeTextStringLen((UINT8*)szMsgID);
	if (length == -1) {
		MSG_DEBUG("MmsBinaryEncodeTextStringLen fail");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < (length + 1)) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
										gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = fieldCode;
	if (__MmsBinaryEncodeTextString(pFile, (UINT8*)szMsgID, length) == false) {
		MSG_DEBUG("MmsBinaryEncodeTextString fail");
		goto __CATCH;
	}

	return true;

__CATCH:

	return false;
}

static bool __MmsBinaryEncodeMmsVersion(FILE *pFile)
{
	UINT8 majorVer = MMS_MAJOR_VERSION;
	UINT8 minorVer = MMS_MINOR_VERSION;

	if (pFile == NULL) {
		MSG_DEBUG("invalid input file");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < 2) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
										gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_VERSION) | 0x80;

	MSG_DEBUG("major version (%d)", majorVer);
	MSG_DEBUG("minor version (%d)", minorVer);

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos] = (majorVer << 4) | (minorVer & 0x0f) | MSB;

	MSG_SEC_INFO("X-Mms-MMS-Version = [0x%02x]", gpMmsEncodeBuf[gCurMmsEncodeBuffPos]);

	if (gpMmsEncodeBuf[gCurMmsEncodeBuffPos] < 0x80) {
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] |= 0x80;
	} else {
		gCurMmsEncodeBuffPos++;
	}

	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeDate(FILE *pFile, time_t inpDateSec)
{
	struct tm dateTime;
	time_t dateSec = 0;

	if (inpDateSec > 0)
		dateSec = inpDateSec;
	else
		dateSec = time(NULL);

	localtime_r(&dateSec, &dateTime);

	MSG_SEC_INFO("%d - %d - %d, %d : %d (SYSTEM)", dateTime.tm_year + 1900, dateTime.tm_mon + 1, dateTime.tm_mday
		, dateTime.tm_hour, dateTime.tm_min);

	if (dateSec > 0) {
		int	length	= 0;
		length = __MmsBinaryEncodeLongIntegerLen(dateSec);

		if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < (length + 1)) {	/* + fieldCode */
			if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
												gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
				MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
				goto __CATCH;
			}
		}

		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_DATE) | 0x80;

		if (__MmsBinaryEncodeLongInteger(pFile, dateSec, length) == false) {
			MSG_DEBUG("date MmsBinaryEncodeLongInteger error");
			goto __CATCH;
		}

	} else {
		MSG_DEBUG("date has a negative value (%d)", dateSec);
		goto __CATCH;
	}
	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeFrom(FILE *pFile)
{
	if (pFile == NULL) {
		MSG_DEBUG("invalid input file");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < 3) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_FROM) | 0x80;
	/* length of MMS_INSERT_ADDRESS_TOKEN value */
	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = 0x01;
	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeAddressType, MMS_INSERT_ADDRESS_TOKEN) | 0x80;

	MSG_SEC_INFO("From  = [INSERT_ADDRESS_TOKEN]");
	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeOneAddress(FILE *pFile, MmsFieldCode addrType, char *szAddrStr)
{
	int length = 0;

	if (pFile == NULL) {
		MSG_DEBUG("invalid input file");
		goto __CATCH;
	}

	/* EncodedString */
	length = __MmsBinaryEncodeEncodedStringLen((UINT8*)szAddrStr);

	if (length == -1) {
		MSG_DEBUG("MmsBinaryEncodeEncodedStringLen fail");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length + 1) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, addrType) | 0x80;


	if (__MmsBinaryEncodeEncodedString(pFile, (UINT8*)szAddrStr, length) == false) {
		MSG_DEBUG("MmsBinaryEncodeEncodedString fail");
		goto __CATCH;
	}

	if (addrType == MMS_CODE_TO )
		MSG_SEC_INFO("To = [%s]", szAddrStr);
	else if (addrType == MMS_CODE_CC)
		MSG_SEC_INFO("CC = [%s]", szAddrStr);
	else if (addrType == MMS_CODE_BCC)
		MSG_SEC_INFO("Bcc = [%s]", szAddrStr);


	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeAddress(FILE *pFile, MmsFieldCode addrType, char *szAddr)
{
	char *pSingleAddr = NULL;


	if (pFile == NULL) {
		MSG_DEBUG("invalid input file");
		goto __CATCH;
	}

	while (szAddr && szAddr[0]) {
		szAddr = _MsgSkipWS3(szAddr);
		if (szAddr == NULL)
			break;

		pSingleAddr = strchr(szAddr, MSG_CH_SEMICOLON);
		if (pSingleAddr) {
			*pSingleAddr = MSG_CH_NULL;
			if (__MmsBinaryEncodeOneAddress(pFile, addrType, szAddr) == false) {
				MSG_DEBUG("__MmsBinaryEncodeAddress fail");
				goto __CATCH;
			}
			*pSingleAddr = MSG_CH_SEMICOLON;

			szAddr		 = pSingleAddr + 1;
			pSingleAddr  = NULL;
		} else {
			if (__MmsBinaryEncodeOneAddress(pFile, addrType, szAddr) == false) {
				MSG_DEBUG("__MmsBinaryEncodeAddress fail");
				goto __CATCH;
			}

			szAddr = NULL;
		}
	}

	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeTime(FILE *pFile, MmsFieldCode fieldCode, MmsTimeStruct time)
{
	int		timeLen	 = 0;
	int		length	 = 0;

	if (pFile == NULL) {
		MSG_DEBUG("invalid input file");
		goto __CATCH;
	}

	if (time.time == 0 ||
		(fieldCode != MMS_CODE_EXPIRYTIME && fieldCode != MMS_CODE_DELIVERYTIME && fieldCode != MMS_CODE_REPLYCHARGINGDEADLINE) ||
		(time.type != MMS_TIMETYPE_ABSOLUTE && time.type != MMS_TIMETYPE_RELATIVE)) {
		MSG_DEBUG("time.type = %d", time.type);
		return true;
	}

	/*
	 * use temporary buffer to estimate value length
	 * and copy it to pData buffer later.
	 */

	if (time.type == MMS_TIMETYPE_RELATIVE) {
		timeLen = __MmsBinaryEncodeIntegerLen(time.time);
	} else {
		timeLen = __MmsBinaryEncodeLongIntegerLen(time.time);
	}

	if (timeLen <= 0) {
		MSG_DEBUG("MmsBinaryEncodeLongIntegerLen fail");
		goto __CATCH;
	}

	length = __MmsBinaryEncodeValueLengthLen(timeLen + 1);	/* time length + time type token */
	if (length == -1) {
		MSG_DEBUG("MmsBinaryEncodeValueLengthLen fail");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < (length + timeLen + 2)) {	/* + fieldCode + timeType */
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MsgWriteDataFromEncodeBuffer fail");
			goto __CATCH;
		}
	}

	/* fieldCode */
	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, fieldCode) | 0x80;

	/* value length */
	if (__MmsBinaryEncodeValueLength(pFile, timeLen + 1, length) == false) {
		MSG_DEBUG("MmsBinaryEncodeValueLength fail");
		goto __CATCH;
	}

	/* time type & value */
	if (time.type == MMS_TIMETYPE_RELATIVE) {
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeTimeType, MMS_TIMETYPE_RELATIVE) | 0x80;
		if (__MmsBinaryEncodeInteger(pFile, time.time, timeLen) == false) {
			MSG_DEBUG("MmsBinaryEncodeLongInteger fail");
			goto __CATCH;
		}
	} else {
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeTimeType, MMS_TIMETYPE_ABSOLUTE) | 0x80;
		if (__MmsBinaryEncodeLongInteger(pFile, time.time, timeLen) == false) {
			MSG_DEBUG("MmsBinaryEncodeLongInteger fail");
			goto __CATCH;
		}
	}

	return true;

__CATCH:
	return false;
}

char *_MsgSkipWS3(char *s)
{
	while (true) {
		if ((*s == MSG_CH_CR) ||
			(*s == MSG_CH_LF) ||
			(*s == MSG_CH_SP) ||
			(*s == MSG_CH_TAB))
			++s;
		else
			return s;
	}
}

static bool __EncodeMmsMessage(MmsMsg *pMmsMsg, const char *raw_filepath)
{
	bool encode_ret = false;
	mode_t file_mode = (S_IRUSR | S_IWUSR);

	if (pMmsMsg == NULL || raw_filepath == NULL) {
		MSG_DEBUG("Invalid Parameter pMmsMsg = %p , raw_filepath = %p", pMmsMsg, raw_filepath);
		return false;
	}

	FILE *pFile = MsgOpenFile(raw_filepath, "wb+");

	if (pFile == NULL) {
		MSG_FATAL("File Open Error: %s", g_strerror(errno));
		goto __CATCH;
	}

	if (MsgFseek(pFile, 0L, SEEK_CUR) < 0) {
		MSG_DEBUG("File Fseek Error: %s", g_strerror(errno));
		goto __CATCH;
	}

	if (fchmod(fileno(pFile), file_mode) < 0) {
		MSG_DEBUG("File chmod Error: %s", g_strerror(errno));
		goto __CATCH;
	}

	switch(pMmsMsg->mmsAttrib.msgType) {
		case MMS_MSGTYPE_SEND_REQ:
		case MMS_MSGTYPE_SEND_CONF:
			encode_ret = MmsEncodeSendReq(pFile, pMmsMsg);
			if (encode_ret == false) {
				MSG_DEBUG("Fail to MmsEncodeSendReq");
				goto __CATCH;
			}
		break;
		default:
			MSG_DEBUG("Not Support msg type : %d", pMmsMsg->mmsAttrib.msgType);
			goto __CATCH;
		break;
	}

	MsgFsync(pFile);	/* file is written to device immediately, it prevents missing file data from unexpected power off */
	MsgCloseFile(pFile);

	return true;

__CATCH:
	if (pFile) {
		MsgCloseFile(pFile);
	}

	return false;
}

MmsPluginEncoder *MmsPluginEncoder::pInstance = NULL;

MmsPluginEncoder *MmsPluginEncoder::instance()
{
	if (!MmsPluginEncoder::pInstance)
		MmsPluginEncoder::pInstance = new MmsPluginEncoder();

	return MmsPluginEncoder::pInstance;
}

MmsPluginEncoder::MmsPluginEncoder() {}
MmsPluginEncoder::~MmsPluginEncoder() {}

void MmsPluginEncoder::encodeMmsPdu(MMS_DATA_S *pMmsData, msg_message_id_t msgID, const char *pduFilePath)
{
	MmsMsg *pMmsMsg = (MmsMsg *)calloc(1, sizeof(MmsMsg));

	if (pMmsMsg) {
		if (MmsConvertMmsMsg(pMmsMsg, pMmsData) != true) {
			MSG_DEBUG("Fail to Compose MMS Message");
			goto __CATCH;
		}

		encodeMmsPdu(pMmsMsg, msgID, pduFilePath);
	}

__CATCH:
	MmsReleaseMmsMsg(pMmsMsg);
	MSG_FREE(pMmsMsg);
}

void MmsPluginEncoder::encodeMmsPdu(MmsMsg *pMmsMsg, msg_message_id_t msgID, const char *pduFilePath)
{
	pMmsMsg->msgID = msgID;

	__EncodeMmsMessage(pMmsMsg, pduFilePath);
}
