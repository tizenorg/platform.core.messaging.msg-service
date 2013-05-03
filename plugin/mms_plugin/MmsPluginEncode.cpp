/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
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
#include "MsgUtilFile.h"
#include "MmsPluginDebug.h"
#include "MmsPluginEncode.h"
#include "MmsPluginCodecTypes.h"
#include "MmsPluginCodecCommon.h"
#include "MmsPluginMIME.h"
#include "MmsPluginUtil.h"


/**  Sending message related variables  ------------------------ */
static	char gszMmsEncodeBuf[MSG_MMS_ENCODE_BUFFER_MAX] = {0, };
static	int gCurMmsEncodeBuffPos = 0;	/* number of characters on gpMmsEncodeBuf */
static	int	gMmsEncodeMaxLen = 0;
static	int	gMmsEncodeCurOffset = 0;	/* offset in file */
static	char *gpMmsEncodeBuf = NULL;

/* Acknowledge.ind & NotifyResp.ind related variables ------------------------ */
static	char gszMmsEncodeBuf2[MSG_MMS_ENCODE_BUFFER_MAX] = {0, };
static	int	gCurMmsEncodeBuffPos2 = 0;	/* number of characters on gpMmsEncodeBuf */
static	int	gMmsEncodeMaxLen2 = 0;
static	int	gMmsEncodeCurOffset2 = 0;	/* offset in file */
static	char *gpMmsEncodeBuf2 = NULL;

static int __MmsGetEncodeOffset(void);
static void __MmsRegisterEncodeBuffer2(char *pInBuff, int maxLen);
static void __MmsUnregisterEncodeBuffer2(void);

static	int __MmsBinaryEncodeUintvarLen(UINT32 integer);
static	bool __MmsBinaryEncodeUintvar(FILE *pFile, UINT32 integer, int length);
static	int __MmsBinaryEncodeValueLengthLen(UINT32 integer);
static	bool __MmsBinaryEncodeValueLength(FILE *pFile, UINT32 integer, int length);
static	int	__MmsBinaryEncodeIntegerLen(UINT32 integer);
static	bool __MmsBinaryEncodeInteger(FILE *pFile, UINT32 integer, int length);
static	int	__MmsBinaryEncodeLongIntegerLen(UINT32 integer);
static	bool __MmsBinaryEncodeLongInteger(FILE *pFile, UINT32 integer, int length);
static	int	__MmsBinaryEncodeTextStringLen(UINT8 *source);
static	bool __MmsBinaryEncodeTextString(FILE *pFile, UINT8 *source, int length);
static	int	__MmsBinaryEncodeQuotedStringLen(UINT8 *pSrc);
static	bool __MmsBinaryEncodeQuotedString(FILE *pFile, UINT8 *source, int length);
static	int	__MmsBinaryEncodeEncodedStringLen(UINT8 *source);
static	bool __MmsBinaryEncodeEncodedString(FILE *pFile, UINT8 *source, int length);

static int __MmsBinaryEncodeContentTypeLen(MsgType *pType);
static bool __MmsBinaryEncodeContentType(FILE *pFile, MsgType *pType, int typeLength);
static int __MmsBinaryEncodeContentHeaderLen(MimeType contentType, MsgType *pType, bool bMultipart);
static bool __MmsBinaryEncodeContentHeader(FILE *pFile, MimeType contentType, MsgType *pType, bool bMultipart);

static bool __MmsBinaryEncodeContentBody(FILE *pFile, MsgBody *pBody);
static bool __MmsBinaryEncodeMsgPart(FILE *pFile, int contentType, MsgType *pType, MsgBody *pBody);

static	bool __MmsBinaryEncodeMmsVersion(FILE *pFile);
static	bool __MmsBinaryEncodeTrID(FILE *pFile, char *szTrID, int bufLen);
static	bool __MmsBinaryEncodeMsgID(FILE *pFile, const char *szMsgID);	/** 2005-05-24, added for read-reply PDU 1.2 */
static	bool __MmsBinaryEncodeFrom(FILE *pFile);
static	bool __MmsBinaryEncodeTime(FILE *pFile, MmsFieldCode fieldCode, MmsTimeStruct time);
static	bool __MmsBinaryEncodeDate(FILE *pFile);
static	bool __MmsBinaryEncodeOneAddress(FILE *pFile, MmsFieldCode addrType, char *szAddrStr);
static	bool __MmsBinaryEncodeAddress(FILE *pFile, MmsFieldCode addrType, char *szAddr);

static	bool __MmsBinaryEncodeFieldCodeAndValue(FILE *pFile, UINT8 fieldCode, UINT8 fieldValue);
static bool __MmsBinaryEncodeSendReqHdr(FILE *pFile, MmsMsg *pMsg);
static bool __MmsBinaryEncodeAckIndHdr(FILE *pFile, char *pTrID, bool bReportAllowed);
static bool __MmsBinaryEncodeNotiRespIndHdr(FILE* pFile, char *pTrID, msg_delivery_report_status_t iStatus, bool bReportAllowed);
static bool __MmsBinaryEncodeReadReport10Hdr(FILE *pFile, MmsMsg *pMsg, msg_read_report_status_t mmsReadStatus);
static bool __MmsBinaryEncodeReadReport11Hdr(FILE *pFile, MmsMsg *pMsg, msg_read_report_status_t mmsReadStatus);

static bool __MmsBinaryEncodeMsgBody(FILE *pFile, MsgType *pType, MsgBody *pBody, int nPartCount, bool bTemplate);//NEW_TEMPLATE
static bool __MmsEncodeSendReq(FILE *pFile, MmsMsg *pMsg, bool bIncludeSendReqHeader);

/* Functions for Acknowledge.ind & NotifyResp.ind  ------------------------ */
static	bool __MmsBinaryEncodeTextString2(FILE *pFile, UINT8 *source, int length);
static	bool __MmsBinaryEncodeMmsVersion2(FILE *pFile);
static	bool __MmsBinaryEncodeFieldCodeAndValue2(FILE *pFile, UINT8 fieldCode, UINT8 fieldValue);

/** -----------------------------------------------------------------
 *					M   M   S       E   N   C   O   D   E
 * * -----------------------------------------------------------------*/

static	void __MmsCleanEncodeBuff(void)
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

static	void __MmsCleanEncodeBuff2(void)
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
	MSG_DEBUG("MmsBinaryEncodeTextString2: \n");

	/**
	 * make text string
	 * Text-string = [Quote] *TEXT End-of-string
	 * If the 1st char in the TEXT is in the range of 128-255, a Quote char must precede it.
	 * Otherwise the Quote char must be omitted.
	 * The Quote is not part of the contents.
	 * Quote = <Octet 127>
	 */

	if (pFile == NULL || source == NULL) {
		MSG_DEBUG("MmsBinaryEncodeTextString2: source == NULL \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen2 - gCurMmsEncodeBuffPos2) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
											gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
			MSG_DEBUG("MmsBinaryEncodeTextString2: MsgWriteDataFromEncodeBuffer fail \n");
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

static	bool __MmsBinaryEncodeFieldCodeAndValue2(FILE *pFile, UINT8 fieldCode, UINT8 fieldValue)
{
	if ((gMmsEncodeMaxLen2 - gCurMmsEncodeBuffPos2) < 2) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
											gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
			MSG_DEBUG("__MmsBinaryEncodeFieldCodeAndValue: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	if (fieldCode == 0xff) {
		MSG_DEBUG("__MmsBinaryEncodeFieldCodeAndValue: invalid fieldCode \n");
		goto __CATCH;
	}

	if (fieldValue == 0xff) {
		MSG_DEBUG("__MmsBinaryEncodeFieldCodeAndValue: invalid fieldValue \n");
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

	MSG_DEBUG("__MmsBinaryEncodeMmsVersion2: \n");

	if (pFile == NULL) {
		MSG_DEBUG("__MmsBinaryEncodeMmsVersion2: invalid input file \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen2 - gCurMmsEncodeBuffPos2) < 2) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
											gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
			MSG_DEBUG("__MmsBinaryEncodeMmsVersion2: MsgWriteDataFromEncodeBuffer fail \n");
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

	MSG_DEBUG("_MmsBinaryEncodeAckIndHdr: szTrID = %s\n", szTrID);
	MSG_DEBUG("_MmsBinaryEncodeAckIndHdr: bReportAllowed = %d\n", bReportAllowed);

	__MmsCleanEncodeBuff2();

	/* msgType */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGTYPE) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeMsgType, MMS_MSGTYPE_ACKNOWLEDGE_IND) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue2(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("_MmsBinaryEncodeAckIndHdr: msgType error\n");
		goto __CATCH;
	}

	/* trID (other type of message) */
	length = __MmsBinaryEncodeTextStringLen((UINT8*)szTrID);
	if (length == -1) {
		MSG_DEBUG("_MmsBinaryEncodeAckIndHdr: MmsBinaryEncodeTextStringLen fail \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen2 - gCurMmsEncodeBuffPos2) < (length + 1)) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
											gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
			MSG_DEBUG("_MmsBinaryEncodeAckIndHdr: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf2[gCurMmsEncodeBuffPos2++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_TRID) | 0x80;

	if (__MmsBinaryEncodeTextString2(pFile, (UINT8*)szTrID, length) == false) {
		MSG_DEBUG("_MmsBinaryEncodeAckIndHdr: MmsBinaryEncodeTextString fail\n");
		goto __CATCH;
	}


	if (__MmsBinaryEncodeMmsVersion2(pFile) == false) {
		MSG_DEBUG("_MmsBinaryEncodeAckIndHdr: __MmsBinaryEncodeMmsVersion error\n");
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
		MSG_DEBUG("_MmsBinaryEncodeAckIndHdr: Report Allowed error\n");
		goto __CATCH;
	}

	/* flush remained data on encoding file */
	if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
										gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
		MSG_DEBUG("_MmsBinaryEncodeAckIndHdr: remained data MsgWriteDataFromEncodeBuffer fail \n");
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

	MSG_DEBUG("_MmsBinaryEncodeNotiRespIndHdr: szTrID = %s\n", szTrID);
	MSG_DEBUG("_MmsBinaryEncodeNotiRespIndHdr: iStatus = %d\n", iStatus);
	MSG_DEBUG("_MmsBinaryEncodeNotiRespIndHdr: bReportAllowed = %d\n", bReportAllowed);

	__MmsCleanEncodeBuff2();

	/* msgType */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGTYPE) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeMsgType, MMS_MSGTYPE_NOTIFYRESP_IND) | 0x80;

	if (__MmsBinaryEncodeFieldCodeAndValue2(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("_MmsBinaryEncodeNotiRespIndHdr: msgType error\n");
		goto __CATCH;
	}


	/* trID (other type of message) */
	length = __MmsBinaryEncodeTextStringLen((UINT8*)szTrID);
	if (length == -1) {
		MSG_DEBUG("_MmsBinaryEncodeNotiRespIndHdr: MmsBinaryEncodeTextStringLen fail \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen2 - gCurMmsEncodeBuffPos2) < (length + 1)) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
											gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
			MSG_DEBUG("_MmsBinaryEncodeNotiRespIndHdr: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf2[gCurMmsEncodeBuffPos2++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_TRID) | 0x80;
	if (__MmsBinaryEncodeTextString2(pFile, (UINT8*)szTrID, length) == false) {
		MSG_DEBUG("_MmsBinaryEncodeNotiRespIndHdr: MmsBinaryEncodeTextString fail\n");
		goto __CATCH;
	}


	if (__MmsBinaryEncodeMmsVersion2(pFile) == false) {
		MSG_DEBUG("_MmsBinaryEncodeNotiRespIndHdr: __MmsBinaryEncodeMmsVersion error\n");
		goto __CATCH;
	}


	/* MsgStatus */
	MSG_DEBUG("_MmsBinaryEncodeNotiRespIndHdr: MsgStatus = %d\n", iStatus);

	if (iStatus != MSG_DELIVERY_REPORT_NONE) {
		fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGSTATUS) | 0x80;
		fieldValue = MmsGetBinaryValue(MmsCodeMsgStatus, iStatus) | 0x80;
		if (__MmsBinaryEncodeFieldCodeAndValue2(pFile, fieldCode, fieldValue) == false) {
			MSG_DEBUG("_MmsBinaryEncodeNotiRespIndHdr: MsgStatus error\n");
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
		MSG_DEBUG("_MmsBinaryEncodeNotiRespIndHdr: Report Allowed error\n");
		goto __CATCH;
	}

	/* flush remained data on encoding file */
	if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf2, &gCurMmsEncodeBuffPos2,
										gMmsEncodeMaxLen2, &gMmsEncodeCurOffset2) == false) {
		MSG_DEBUG("_MmsBinaryEncodeNotiRespIndHdr: remained data MsgWriteDataFromEncodeBuffer fail \n");
		goto __CATCH;
	}

	return true;

__CATCH:

	return false;
}

bool MmsEncodeAckInd(FILE *pFile, char *pTrID, bool bReportAllowed)
{
	__MmsRegisterEncodeBuffer2(gszMmsEncodeBuf2, MSG_MMS_ENCODE_BUFFER_MAX);

	MSG_DEBUG("_MmsEncodeAckInd: Start Binary Encoding now ============= \n");

	if (__MmsBinaryEncodeAckIndHdr(pFile, pTrID, bReportAllowed) == false) {
		MSG_DEBUG("_MmsEncodeAckInd: SendReq Binary encoding fail \n");
		goto __CATCH;
	}

	__MmsUnregisterEncodeBuffer2();

	return true;

__CATCH:

	MSG_DEBUG("## _MmsEncodeAckInd: failed");
	__MmsUnregisterEncodeBuffer2();

	return false;
}

bool MmsEncodeNotiRespInd(FILE *pFile, char *pTrID, msg_delivery_report_status_t iStatus, bool bReportAllowed)
{
	__MmsRegisterEncodeBuffer2(gszMmsEncodeBuf2, MSG_MMS_ENCODE_BUFFER_MAX);

	MSG_DEBUG("_MmsEncodeNotiRespInd: Start Binary Encoding now ============= \n");

	if (__MmsBinaryEncodeNotiRespIndHdr(pFile, pTrID, iStatus, bReportAllowed) == false) {
		MSG_DEBUG("_MmsEncodeNotiRespInd: SendReq Binary encoding fail \n");
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
	MsgBody msgBody;

	char *pszReportMsg = NULL;
	int	maxLen = 0;

	struct	tm	*dateTime = NULL;
	time_t	RawTime = 0;
	time_t	dateSec = 0;

	MmsRegisterEncodeBuffer(gszMmsEncodeBuf, MSG_MMS_ENCODE_BUFFER_MAX);

	MSG_DEBUG("_MmsEncodeMsg: Start Binary Encoding now V1.0============= \n");

	if (__MmsBinaryEncodeReadReport10Hdr(pFile, pMsg, mmsReadStatus) == false) {
		MSG_DEBUG("_MmsEncodeReadReport10: SendReq Binary encoding fail \n");
		goto __CATCH;
	}

	memset(&msgType, 0, sizeof(MsgType));
	memset(&msgBody, 0, sizeof(MsgBody));

	pText = (char *)malloc(MSG_STDSTR_LONG);
	if (pText == NULL) {
		MSG_DEBUG("__MmsSendReadReportV10: text body malloc fail \n");
		goto __CATCH;
	}

	memset(pText, 0, MSG_STDSTR_LONG);

	time(&RawTime);
	dateTime = localtime(&RawTime);
	dateSec = mktime(dateTime);

	// get report message
	if (mmsReadStatus == MSG_READ_REPORT_IS_DELETED) {
		pszReportMsg = (char*)"Your message has been deleted " ;
	} else {
		pszReportMsg = (char*)"Your message has been read " ;
	}

	// make report body ..
	maxLen = strlen (pszReportMsg) +16 /* date string */ + 8 /* enter chars */ ;

	if (maxLen > MSG_STDSTR_LONG) {
		snprintf (pText, MSG_STDSTR_LONG, "%s\n", pszReportMsg);
	} else {
		snprintf(pText, MSG_STDSTR_LONG, "%s\r\n\r\n%.4d/%.2d/%.2d %.2d:%.2d\r\n",
						pszReportMsg, dateTime->tm_year+1900, dateTime->tm_mon+1, dateTime->tm_mday, dateTime->tm_hour, dateTime->tm_min);
	}

	// make header
	msgType.type = MIME_APPLICATION_VND_WAP_MULTIPART_MIXED;
	msgType.contentSize = strlen(pText);
	msgType.param.charset = MSG_CHARSET_UNKNOWN;

	// make body
	if ((pPart = MmsAllocMultipart()) == NULL) {
		MSG_DEBUG("__MmsSendReadReportV10: MsgAllocMultipart Fail \n");
		goto __CATCH;
	}


	pPart->type.type = MIME_TEXT_PLAIN;
	pPart->type.contentSize = strlen(pText);
	pPart->type.param.charset = MSG_CHARSET_UTF8;

	if (pPart->pBody == NULL) {
		MSG_DEBUG("__MmsSendReadReportV10: pPart->pBody is NULL \n");
		goto __CATCH;
	}

	pPart->pBody->size = strlen(pText);
	pPart->pBody->body.pText = pText;

	msgBody.body.pMultipart = pPart;

	if (__MmsBinaryEncodeMsgBody(pFile, &msgType, &msgBody, 1, false) == false) {
		MSG_DEBUG("__MmsSendReadReportV10: MmsBinaryEncodeMsgBody fail \n");
		goto __CATCH;
	}

	MSG_DEBUG("__MmsSendReadReportV10:  Send To RM ReadReport Msg \n");

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

	MSG_DEBUG("_MmsEncodeMsg: Start Binary Encoding now V1.1============= \n");

	if (__MmsBinaryEncodeReadReport11Hdr(pFile, pMsg, mmsReadStatus) == false) {
		MSG_DEBUG("_MmsEncodeMsg: SendReq Binary encoding fail \n");
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
		MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: msgType error\n");
		goto __CATCH;
	}

	/* trID (other type of message) */
	if (__MmsBinaryEncodeTrID(pFile, pMsg->szTrID, MMS_TR_ID_LEN + 1) == false) {
		MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: trID error\n");
		goto __CATCH;
	}

	MSG_DEBUG("_MmsBinaryEncodeSendReqHdr:                pMsg->szTrID = %s\n", pMsg->szTrID);

	if (__MmsBinaryEncodeMmsVersion(pFile) == false) {
		MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: __MmsBinaryEncodeMmsVersion error\n");
		goto __CATCH;
	}

	/* From : Insert Token mode */
	if (__MmsBinaryEncodeFrom(pFile) == false) {
		MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: __MmsBinaryEncodeFrom fail\n");
		goto __CATCH;
	}


	/* To = Encoded-string-value */
	if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_TO, pMsg->mmsAttrib.szTo) == false)
	{
		MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: To __MmsBinaryEncodeAddress fail\n");
		goto __CATCH;
	}


	/* Cc = Encoded-string-value */
	if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_CC, pMsg->mmsAttrib.szCc) == false) {
		MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: Cc __MmsBinaryEncodeAddress fail\n");
		goto __CATCH;
	}


	/* Bcc = Encoded-string-value */

	if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_BCC, pMsg->mmsAttrib.szBcc) == false) {
		MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: Bcc __MmsBinaryEncodeAddress fail\n");
		goto __CATCH;
	}


	MSG_DEBUG("_MmsBinaryEnocdeSendReqHdr() pMsg->mmsAttrib.szSubject =%s\n",pMsg->mmsAttrib.szSubject);

	/* Subject = Encoded-string-value */
	if (pMsg->mmsAttrib.szSubject[0]) {
		length = __MmsBinaryEncodeEncodedStringLen((UINT8*)pMsg->mmsAttrib.szSubject);
		if (length == -1) {
			MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: subject MmsBinaryEncodeEncodedStringLen fail \n");
			goto __CATCH;
		}

		if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length + 1) {
			if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
												gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
				MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: subject MsgWriteDataFromEncodeBuffer fail \n");
				goto __CATCH;
			}
		}

		fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_SUBJECT) | 0x80;
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = fieldCode;
		if (__MmsBinaryEncodeEncodedString(pFile, (UINT8*)pMsg->mmsAttrib.szSubject, length) == false) {
			MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: subject MmsBinaryEncodeEncodedString fail \n");
			goto __CATCH;
		}
	}

	/* MMS-1.3-con-739 */
	/* Msg class */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGCLASS) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeMsgClass,  (int)pMsg->mmsAttrib.msgClass) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: msgClass error\n");
		goto __CATCH;
	}
	/* MMS-1.3-con-739 */
	/* MMS-1.3-con-733 */
	/* Date = Long-integer */
	if (!__MmsBinaryEncodeDate(pFile)) {
			MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: __MmsBinaryEncodeDate error\n");
			goto __CATCH;
	}
	/* MMS-1.3-con-733 */

	/* Expiry Time  : Value-length Absolute-token Date-value */
	if (pMsg->mmsAttrib.bUseExpiryCustomTime == true) {
		if (__MmsBinaryEncodeTime(pFile, MMS_CODE_EXPIRYTIME, pMsg->mmsAttrib.expiryCustomTime) == false) {
			MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: expiryTime __MmsBinaryEncodeTime fail\n");
			goto __CATCH;
		}
	} else if (pMsg->mmsAttrib.expiryTime.type != MMS_TIMETYPE_NONE) { 	// for avoiding the creation of the expiry field in case the user selects the maximum
		if (__MmsBinaryEncodeTime(pFile, MMS_CODE_EXPIRYTIME, pMsg->mmsAttrib.expiryTime) == false) {
			MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: expiryTime __MmsBinaryEncodeTime fail\n");
			goto __CATCH;
		}
	}

	/* Use Custom time for Delivery Time */
	if (pMsg->mmsAttrib.bUseDeliveryCustomTime == true) {
		if (__MmsBinaryEncodeTime(pFile, MMS_CODE_DELIVERYTIME, pMsg->mmsAttrib.deliveryCustomTime) == false) {
			MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: deliveryTime __MmsBinaryEncodeTime fail\n");
			goto __CATCH;
		}
	} else {
		if (__MmsBinaryEncodeTime(pFile, MMS_CODE_DELIVERYTIME, pMsg->mmsAttrib.deliveryTime) == false) {
			MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: deliveryTime __MmsBinaryEncodeTime fail\n");
			goto __CATCH;
		}
	}

	/* Priority */
	if (pMsg->mmsAttrib.priority!= MMS_PRIORITY_ERROR && pMsg->mmsAttrib.priority!= MMS_PRIORITY_NORMAL) {
		/* MMS_PRIORITY_NORMAL is default : don't send optional field */

		fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_PRIORITY)|0x80;
		fieldValue = MmsGetBinaryValue(MmsCodePriority, pMsg->mmsAttrib.priority)|0x80;

		if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
			MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: priority error\n");
			goto __CATCH;
		}
	}

	/* Sender Visible (hide | show)	*/
	if (pMsg->mmsAttrib.bHideAddress == true) {
		fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_SENDERVISIBILLITY) | MSB;
		fieldValue = MmsGetBinaryValue(MmsCodeSenderVisibility, MMS_SENDER_HIDE) | 0x80;

		if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
			MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: sender visibility error\n");
			goto __CATCH;
		}
	}

	/* Delivery Report (yes | no) */
	fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_DELIVERYREPORT)|0x80;

	if (pMsg->mmsAttrib.bAskDeliveryReport) {
		fieldValue = MmsGetBinaryValue(MmsCodeDeliveryReport, MMS_REPORT_YES) | 0x80;
	} else {
		fieldValue = MmsGetBinaryValue(MmsCodeDeliveryReport, MMS_REPORT_NO) | 0x80;
	}

	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: sender visibility error\n");
		goto __CATCH;
	}

	/* Read Reply (Yes | no) */
	fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_READREPLY)|0x80;
	if (pMsg->mmsAttrib.bAskReadReply) {
		fieldValue = MmsGetBinaryValue(MmsCodeReadReply, MMS_REPORT_YES)|0x80;
	} else {
		fieldValue = MmsGetBinaryValue(MmsCodeReadReply, MMS_REPORT_NO)|0x80;
	}

	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: read reply error\n");
		goto __CATCH;
	}

	if ((pMsg->mmsAttrib.replyCharge.chargeType == MMS_REPLY_REQUESTED) ||
		(pMsg->mmsAttrib.replyCharge.chargeType == MMS_REPLY_REQUESTED_TEXT_ONLY)) {

		// reply charging
		fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_REPLYCHARGING)|0x80;
		fieldValue = MmsGetBinaryValue(MmsCodeReadReply, pMsg->mmsAttrib.replyCharge.chargeType)|0x80;

		if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
			MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: replyCharging error\n");
			goto __CATCH;
		}

		/** fixme: Reply-charging-deadline */
		if (pMsg->mmsAttrib.replyCharge.deadLine.time > 0) {
			if (__MmsBinaryEncodeTime(pFile, MMS_CODE_REPLYCHARGINGDEADLINE, pMsg->mmsAttrib.replyCharge.deadLine) == false) {
				MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: replyCharging __MmsBinaryEncodeTime fail\n");
				goto __CATCH;
			}
		}

		/** fixme: Reply-charging-size */
		if (pMsg->mmsAttrib.replyCharge.chargeSize > 0) {
			length = __MmsBinaryEncodeIntegerLen(pMsg->mmsAttrib.replyCharge.chargeSize);

			if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < (length + 1)) {
				if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
												gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
					MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: MsgWriteDataFromEncodeBuffer fail \n");
					goto __CATCH;
				}
			}

			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_REPLYCHARGINGSIZE) | 0x80;

			if (__MmsBinaryEncodeInteger(pFile, pMsg->mmsAttrib.replyCharge.chargeSize, length) == false)	{
				MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: replyChargingSize MmsBinaryEncodeInteger error\n");
				goto __CATCH;
			}
		}

		/** fixme: Reply-charging-ID  ----> used only when reply message  */
		if (pMsg->mmsAttrib.replyCharge.szChargeID[0]) {
			length = __MmsBinaryEncodeTextStringLen((UINT8*)pMsg->mmsAttrib.replyCharge.szChargeID);
			if (length == -1) {
				MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: szReplyChargingID MmsBinaryEncodeTextStringLen fail\n");
				goto __CATCH;
			}

			if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < (length + 1)) {
				if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
												gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
					MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: MsgWriteDataFromEncodeBuffer fail \n");
					goto __CATCH;
				}
			}

			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_REPLYCHARGINGID) | 0x80;

			if (__MmsBinaryEncodeTextString(pFile, (UINT8*)pMsg->mmsAttrib.replyCharge.szChargeID, length) == false) {
				MSG_DEBUG("szContentLocation MmsBinaryEncodeTextString fail\n");
				goto __CATCH;
			}
		}
	}

	/* flush remained data on encoding file */
	if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos, gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
		MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: remained data MsgWriteDataFromEncodeBuffer fail \n");
		goto __CATCH;
	}

	return true;

__CATCH:

	MSG_DEBUG("## _MmsBinaryEncodeSendReqHdr: failed");
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

	MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: mmsReadStatus = %d\n", mmsReadStatus);

	__MmsCleanEncodeBuff();

	/* msgType */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGTYPE) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeMsgType, MMS_MSGTYPE_SEND_REQ) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: msgType error\n");
		goto __CATCH;
	}

	/* trID (other type of message) */
	if (__MmsBinaryEncodeTrID(pFile, NULL, 0) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: __MmsBinaryEncodeTrID error\n");
		goto __CATCH;
	}

	if (__MmsBinaryEncodeMmsVersion(pFile) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: __MmsBinaryEncodeMmsVersion error\n");
		goto __CATCH;
	}

	/* Date = Long-integer */
	if (!__MmsBinaryEncodeDate(pFile)) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: date __MmsBinaryEncodeDate error\n");
		goto __CATCH;
	}

	/* From : Insert Token mode */
	if (__MmsBinaryEncodeFrom(pFile) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: __MmsBinaryEncodeFrom fail\n");
		goto __CATCH;
	}

	MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: To = %s\n", pMsg->mmsAttrib.szFrom);

	/* To = Encoded-string */
	if (pMsg && (strchr(pMsg->mmsAttrib.szFrom, '/') == NULL)) {
		length = strlen(pMsg->mmsAttrib.szFrom);
		szTo = (char *)malloc(length + 11);
		if (szTo == NULL) {
			MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: szTo alloc fail\n");
			goto __CATCH;
		}

		snprintf(szTo, length + 11, "%s/TYPE=PLMN", pMsg->mmsAttrib.szFrom);
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: To = %s\n", szTo);

		if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_TO, szTo) == false) {
			MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: To __MmsBinaryEncodeAddress fail\n");
			goto __CATCH;
		}

		if (szTo) {
			free(szTo);
			szTo = NULL;
		}
	} else {
		if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_TO, pMsg->mmsAttrib.szFrom) == false) {
			MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: To __MmsBinaryEncodeAddress fail\n");
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
			snprintf(szSubject, MSG_LOCALE_SUBJ_LEN + 8, "%s", MMS_READ_REPORT_STRING_DELETED );
		}
	}

	length = __MmsBinaryEncodeEncodedStringLen((UINT8*)szSubject);
	if (length == -1) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: subject MmsBinaryEncodeEncodedStringLen fail \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length + 1) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf, &gCurMmsEncodeBuffPos, gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: subject MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_SUBJECT) | 0x80;
	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = fieldCode;
	if (__MmsBinaryEncodeEncodedString(pFile, (UINT8*)szSubject, length) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: subject MmsBinaryEncodeEncodedString fail \n");
		goto __CATCH;
	}

	/* Msg class */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_MSGCLASS) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeMsgClass, MMS_MSGCLASS_AUTO) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: msgClass error\n");
		goto __CATCH;
	}


	/* Delivery Report (yes | no) */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_DELIVERYREPORT) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeDeliveryReport, MMS_REPORT_NO) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: Delivery Report error\n");
		goto __CATCH;
	}


	/* Read Reply (Yes | no) */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_READREPLY) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeReadReply, MMS_REPORT_NO) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: Read Reply error\n");
		goto __CATCH;
	}


	/* Sender Visible (hide | show)	*/
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_SENDERVISIBILLITY) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeSenderVisibility, MMS_SENDER_SHOW) | 0x80;
	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: Sender Visible error\n");
		goto __CATCH;
	}

	/* fixme: MimeType */
	/* fixme: msgHeader */
	/* fixme: msgBody */

	/* flush remained data on encoding file */
	if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
										gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: remained data MsgWriteDataFromEncodeBuffer fail \n");
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
		MSG_DEBUG("_MmsBinaryEncodeReadReport11Hdr: msgType error\n");
		goto __CATCH;
	}

	/* MMS version */
	if (__MmsBinaryEncodeMmsVersion(pFile) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport11Hdr: __MmsBinaryEncodeMmsVersion error\n");
		goto __CATCH;
	}

	if (__MmsBinaryEncodeMsgID(pFile, pMsg->szMsgID) == false)
		goto __CATCH;

	/* To = Encoded-string */
	if (strchr(pMsg->mmsAttrib.szFrom, '/') == NULL) {
		int length = 0;
		length = strlen(pMsg->mmsAttrib.szFrom);
		szTo = (char *)malloc(length + 11);

		if (szTo == NULL) {
			MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: szTo alloc fail\n");
			goto __CATCH;
		}

		snprintf(szTo, length + 11,"%s/TYPE=PLMN", pMsg->mmsAttrib.szFrom);
		MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: To = %s\n", szTo);

		if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_TO, szTo) == false) {
			MSG_DEBUG("_MmsBinaryEncodeReadReport10Hdr: To __MmsBinaryEncodeAddress fail\n");
			goto __CATCH;
		}

		if (szTo) {
			free(szTo);
			szTo = NULL;
		}
	} else {
		if (__MmsBinaryEncodeAddress(pFile, MMS_CODE_TO, pMsg->mmsAttrib.szFrom) == false) {
			MSG_DEBUG("_MmsBinaryEncodeReadReport11Hdr: To __MmsBinaryEncodeAddress fail\n");
			goto __CATCH;
		}
	}

	/* From : Insert Token mode */
	if (__MmsBinaryEncodeFrom(pFile) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport11Hdr: __MmsBinaryEncodeFrom fail\n");
		goto __CATCH;
	}

	/* Date = Long-integer */
	if (!__MmsBinaryEncodeDate(pFile)) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport11Hdr: __MmsBinaryEncodeDate error\n");
		goto __CATCH;
	}

	/* Read Status (Yes | no) */
	fieldCode  = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_READSTATUS) | 0x80;
	fieldValue = MmsGetBinaryValue(MmsCodeReadStatus, mmsReadStatus) | 0x80;

	if (__MmsBinaryEncodeFieldCodeAndValue(pFile, fieldCode, fieldValue) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport11Hdr: Read Status error\n");
		goto __CATCH;
	};

	if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
										gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
		MSG_DEBUG("_MmsBinaryEncodeReadReport11Hdr: remained data MsgWriteDataFromEncodeBuffer fail \n");
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
	MsgMultipart *pMultipart = NULL;

	MSG_DEBUG("MmsBinaryEncodeMsgBody: nPartCount = %d\n", nPartCount);

	if (pFile == NULL || pType == NULL) {
		MSG_DEBUG("MmsBinaryEncodeMsgBody: invalid file handle\n");
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
			MSG_DEBUG("MmsBinaryEncodeMsgBody: MmsBinaryEncodeContentTypeLen fail \n");
			goto __CATCH;
		}
		if (bTemplate == false)
			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_CONTENTTYPE) | 0x80;

		if (__MmsBinaryEncodeContentType(pFile, pType, length) == false) {
			MSG_DEBUG("MmsBinaryEncodeMsgBody: MmsBinaryEncodeContentType fail \n");
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
				MSG_DEBUG("MmsBinaryEncodeMsgBody: nEntries MmsBinaryEncodeUintvarLen fail \n");
				goto __CATCH;
			}
			if (__MmsBinaryEncodeUintvar(pFile, nEntries, length) == false) {
				MSG_DEBUG("MmsBinaryEncodeMsgBody: nEntries MmsBinaryEncodeUintvar fail \n");
				goto __CATCH;
			}

			pType->size = __MmsGetEncodeOffset() - pType->offset;
		}

		if (nEntries > 0) {
			if (nEntries && pBody->pPresentationBody) {
				if (__MmsBinaryEncodeMsgPart(pFile, pType->type, &pBody->presentationType, pBody->pPresentationBody) == false) {
					MSG_DEBUG("MmsBinaryEncodeMsgBody: __MmsBinaryEncodeMsgPart fail \n");
					goto __CATCH;
				}

				nEntries--;
			}

			pMultipart = pBody->body.pMultipart;
			while (nEntries && pMultipart) {
				if (__MmsBinaryEncodeMsgPart(pFile, pType->type, &pMultipart->type, pMultipart->pBody) == false) {
					MSG_DEBUG("MmsBinaryEncodeMsgBody: __MmsBinaryEncodeMsgPart fail \n");
					goto __CATCH;
				}
				pMultipart = pMultipart->pNext;
				nEntries--;
			}
		} else {
			if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
												gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
				MSG_DEBUG("MmsBinaryEncodeMsgBody: Empty message body MsgWriteDataFromEncodeBuffer fail \n");
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
			MSG_DEBUG("MmsBinaryEncodeMsgBody: Singlepart MmsBinaryEncodeContentTypeLen fail \n");
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
	MSG_DEBUG("## MmsBinaryEncodeMsgBody: failed\n");

	return false;
}

static int __MmsBinaryEncodeContentTypeLen(MsgType *pType)
{
	int length = 0;
	int	totalLength = 0;
	UINT16 fieldValue = 0xffff;
	const char *szTextType = NULL;
	int	contentType = MIME_UNKNOWN;

	MSG_DEBUG("MSmsBinaryEncodeContentTypeLen:  type\n");

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
	if (fieldValue == UNDEFINED_BINARY || fieldValue == 0x0049)	{
		/* Extension-media type */
		szTextType = MmsGetTextValue(MmsCodeContentType, contentType);
		if (szTextType != NULL) {
			length  = __MmsBinaryEncodeTextStringLen((UINT8*)szTextType);
			if (length == -1) {
				MSG_DEBUG("MSmsBinaryEncodeContentTypeLen: szTextType MmsBinaryEncodeTextStringLen fail \n");
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

	MSG_DEBUG("MSmsBinaryEncodeContentTypeLen:  parameters    \n\n");

	/* Well-known-charset = Any-charset | Integer-value ----------------------- */

	if (pType->param.charset != MSG_CHARSET_UNKNOWN) {
		fieldValue = MmsGetBinaryValue(MmsCodeCharSet, pType->param.charset);
		length	   = __MmsBinaryEncodeIntegerLen(fieldValue);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeContentTypeLen: charSet MmsBinaryEncodeIntegerLen fail \n");
			goto __CATCH;
		}
		totalLength += (length + 1);
	} else {
		if (MmsIsText(contentType)) {	// Any-charset
			if (!MmsIsVitemContent (contentType, pType->param.szName))
				totalLength += 2;
		}
	}


	/* Name = Text-string ------------------------------------------------------ */

	if (pType->param.szName[0]) {
		char* pszName = NULL;

		if (MsgIsASCII (pType->param.szName)) {
			MSG_DEBUG("MmsBinaryEncodeContentTypeLen: szName is consisted of ascii char-set chars. \n");

			pszName = (char *)malloc(strlen(pType->param.szName) +1);
			memset(pszName, 0, (strlen(pType->param.szName)+1));
			strcpy(pszName, pType->param.szName);
		} else {
			MSG_DEBUG("MmsBinaryEncodeContentTypeLen: szName is not consisted of ascii char-set chars. \n");
			MSG_DEBUG("MmsBinaryEncodeContentTypeLen: pType->param.szName : %s\n", pType->param.szName);

			/* if msg-server can't support non ascii, then convert non-ascii to '_' by using _MsgReplaceNonAscii*/
			int filelen = strlen(pType->param.szName);

			pszName = (char *)malloc(filelen + 1);
			memset(pszName, 0, (filelen + 1));
			strncpy(pszName, pType->param.szName, filelen);

			MSG_DEBUG("MmsBinaryEncodeContentTypeLen: pszName : %s\n", pszName);
		}

		//change empty space to '_' in the file name
		if (MsgIsSpace(pszName)) {
			char *pszTempName = NULL;

			MSG_DEBUG("MmsBinaryEncodeContentTypeLen: szName has space(' '). \n");

			MsgReplaceSpecialChar(pszName, &pszTempName, ' ');

			if (pszTempName) {
				free(pszName);
				pszName = pszTempName;
			}
			MSG_DEBUG("MmsBinaryEncodeContentTypeLen: pszName : %s\n", pszName);
		}

		length = __MmsBinaryEncodeTextStringLen((UINT8*)pszName);
		free(pszName);

		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeContentTypeLen: szName MmsBinaryEncodeIntegerLen fail \n");
			goto __CATCH;
		}

		totalLength += (length + 1);
	}

#ifdef FEATURE_JAVA_MMS
	if (pType->param.szApplicationID) {
		length = __MmsBinaryEncodeTextStringLen((UINT8*)pType->param.szApplicationID);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeContentTypeLen: szApplicationID MmsBinaryEncodeTextStrinLen fail \n");
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
			MSG_DEBUG("MmsBinaryEncodeContentTypeLen: szApplicationID MmsBinaryEncodeTextStrinLen fail \n");
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
					MSG_DEBUG("MmsBinaryEncodeContentTypeLen: type param MmsBinaryEncodeTextStringLen fail \n");
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
				MSG_DEBUG("MmsBinaryEncodeContentType: szStart MmsBinaryEncodeTextStringLen fail \n");
				goto __CATCH;
			}

			totalLength += (length + 1);
		}


		/* startInfo = Text-string -------------------- */
		if (pType->param.szStartInfo[0]) {
			/* StartInfo (with multipart/related) = Text-string */
			length  = __MmsBinaryEncodeTextStringLen((UINT8*)pType->param.szStartInfo);
			if (length == -1) {
				MSG_DEBUG("MmsBinaryEncodeContentType: szStartInfo MmsBinaryEncodeTextStringLen fail \n");
				goto __CATCH;
			}

			totalLength += (length + 1);
		}
	}

	return totalLength;

__CATCH:

	MSG_DEBUG("## MmsBinaryEncodeContentTypeLen: failed");
	return -1;
}

static bool __MmsBinaryEncodeContentType(FILE *pFile, MsgType *pType, int typeLength)
{
	int length = 0;
	UINT16 fieldValue = 0xffff;
	const char *szTextType = NULL;
	int contentType = MIME_UNKNOWN;

#ifdef FEATURE_JAVA_MMS
	const char *szParameter = NULL;
#endif

	MSG_DEBUG("************************************************************************************\n");
	MSG_DEBUG("MmsBinaryEncodeContentType:  C O N T E N T     T Y P E    \n\n");

	/*
	 * Content-type-value = Constrained-media | Content-general-form
	 * Constrained-media  = Constrained-encoding
	 *						Constrained-encoding = Extension-Media | Short-integer
	 *						Extension-media = *TEXT End-of-string
	 * Content-general-form = Value-length Media-type
	 *						Media-type = (Well-known-media | Extension-Media) *(Parameter)
	 */

	if (pFile == NULL) {
		MSG_DEBUG("MmsBinaryEncodeContentType: invalid file handle\n");
		goto __CATCH;
	}


	/* Content-Type = Content-general-form ------------------------------- */

	length = __MmsBinaryEncodeValueLengthLen(typeLength);
	if (length == -1) {
		MSG_DEBUG("MSmsBinaryEncodeContentType : MmsBinaryEncodeValueLengthLen fail.\n");
		goto __CATCH;
	}

	if (__MmsBinaryEncodeValueLength(pFile, typeLength, length) == false) {
		MSG_DEBUG("MSmsBinaryEncodeContentType : MmsBinaryEncodeValueLength fail.\n");
		goto __CATCH;
	}

	contentType = pType->type;

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
			MSG_DEBUG("MSmsBinaryEncodeContentType: szTextType MmsBinaryEncodeTextStringLen fail \n");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeTextString(pFile, (UINT8*)szTextType, length) == false) {
			MSG_DEBUG("MmsBinaryEncodeContentType: szTextType MmsBinaryEncodeTextString fail \n");
			goto __CATCH;
		}
	}

	/* Parameters -------------------------------------------------------------- */

	MSG_DEBUG("MmsBinaryEncodeContentType:  P A R M E T E R S    \n\n");

	/* Name = Text-string ------------------------------------------------------ */

	if (pType->param.szName[0]) {
		char* pszName = NULL;

		if (MsgIsASCII (pType->param.szName)) {

			MSG_DEBUG("MmsBinaryEncodeContentType: szName is consisted of ascii char-set chars. \n");

			pszName = (char *)malloc(strlen(pType->param.szName) +1);
			memset(pszName, 0, (strlen(pType->param.szName)+1));
			strcpy(pszName, pType->param.szName);
		} else {
			MSG_DEBUG("MmsBinaryEncodeContentTypeLen: szName is not consisted of ascii char-set chars. \n");

			/* if msg-server can't support non ascii, then convert non-ascii to '_' by using _MsgReplaceNonAscii*/
			int filelen = strlen(pType->param.szName);

			pszName = (char *)malloc(filelen + 1);
			memset(pszName, 0, (filelen + 1));
			strncpy(pszName, pType->param.szName, filelen);

			MSG_DEBUG("MmsBinaryEncodeContentType: pszName : %s\n", pszName);
		}

		//change empty space to '_' in the file name
		if (MsgIsSpace(pszName)) {
			char*	pszTempName = NULL;

			MSG_DEBUG("MmsBinaryEncodeContentType: szName has space(' '). \n");

			MsgReplaceSpecialChar(pszName, &pszTempName, ' ');

			if (pszTempName) {
				free(pszName);
				pszName = pszTempName;
			}
			MSG_DEBUG("MmsBinaryEncodeContentType: pszName : %s\n", pszName);
		}

		length = __MmsBinaryEncodeTextStringLen((UINT8*)pszName);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeContentType: szName MmsBinaryEncodeIntegerLen fail \n");
			free(pszName);
			goto __CATCH;
		}
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_NAME) | 0x80;

		if (__MmsBinaryEncodeTextString(pFile, (UINT8*)pszName, length) == false) {
			MSG_DEBUG("MmsBinaryEncodeContentType: szName MmsBinaryEncodeTextString fail\n");
			free(pszName);
			goto __CATCH;
		}
		free(pszName);
	}
#ifdef FEATURE_JAVA_MMS
	MSG_DEBUG(" MmsBinaryEncodeContentType: Application-ID \n");

	/* Application-ID: Text-string */
	if (pType->param.szApplicationID) {
		length = __MmsBinaryEncodeTextStringLen((UINT8*) pType->param.szApplicationID);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeContentType: szApplicationID MmsBinaryEncodeIntegerLen Fail \n");
			goto __CATCH;
		}

		fieldValue = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_APPLICATION_ID);

		if (fieldValue == UNDEFINED_BINARY)
			szParameter = MmsGetTextValue(MmsCodeParameterCode, MSG_PARAM_APPLICATION_ID);

		if (szParameter == NULL) {
			MSG_DEBUG("MmsBinaryEncodeContentType: szParameter is NULL \n");
			goto __CATCH;
		}

		strncpy(gpMmsEncodeBuf + gCurMmsEncodeBuffPos, (char*)szParameter, strlen(szParameter));
		gCurMmsEncodeBuffPos += strlen(szParameter);
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] =(UINT8)NULL;

		if (__MmsBinaryEncodeTextString(pFile, (UINT8*) pType->param.szApplicationID, length) == false) {
			MSG_DEBUG(" MmsBinaryEncodeContentType: szApplicationID MmsBinaryEncodeTextString fail\n");
			goto __CATCH;
		}

	}

	/* ReplyToApplicationID: Text-string */
	if (pType->param.szReplyToApplicationID) {
		length = __MmsBinaryEncodeTextStringLen((UINT8*) pType->param.szReplyToApplicationID);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeContentType: szReplyToApplicationID MmsBinaryEncodeIntegerLen Fail \n");
			goto __CATCH;
		}

		fieldValue = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_REPLY_TO_APPLICATION_ID);

		if (fieldValue == UNDEFINED_BINARY)
			szParameter = MmsGetTextValue(MmsCodeParameterCode, MSG_PARAM_REPLY_TO_APPLICATION_ID);

		if (szParameter == NULL) {
			MSG_DEBUG("MmsBinaryEncodeContentType: szParameter is NULL \n");
			goto __CATCH;
		}

		strncpy(gpMmsEncodeBuf + gCurMmsEncodeBuffPos, (char*)szParameter, strlen(szParameter));
		gCurMmsEncodeBuffPos += strlen(szParameter);
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] =(UINT8)NULL;

		if (__MmsBinaryEncodeTextString(pFile, (UINT8*) pType->param.szReplyToApplicationID, length) == false) {
			MSG_DEBUG(" MmsBinaryEncodeContentType: szApplicationID MmsBinaryEncodeTextString fail\n");
			goto __CATCH;
		}
	}
#endif

	/* Well-known-charset = Any-charset | Integer-value ----------------------- */

	if (pType->param.charset != MSG_CHARSET_UNKNOWN) {
		fieldValue = MmsGetBinaryValue(MmsCodeCharSet, pType->param.charset);
		length = __MmsBinaryEncodeIntegerLen(fieldValue);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeContentType: charSet MmsBinaryEncodeIntegerLen fail \n");
			goto __CATCH;
		}

		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_CHARSET) | 0x80;
		if (__MmsBinaryEncodeInteger(pFile, fieldValue, length) == false) {
			MSG_DEBUG("MmsBinaryEncodeContentType: charSet MmsBinaryEncodeInteger fail\n");
			goto __CATCH;
		}
	} else {
		/* Any-charset */
		if (MmsIsText(contentType)) {
			if (!MmsIsVitemContent (contentType, pType->param.szName)) {
				gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_CHARSET) | 0x80;
				fieldValue = 0x0000;	//laconic_warning, just to remove warning message
				gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)fieldValue | 0x80;
			}
		}
	}

	/* type, start, & startInfo : multipart/related only parameters -------------- */
	if (contentType == MIME_MULTIPART_RELATED || contentType == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {
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
				MSG_DEBUG("MmsBinaryEncodeContentType: type param MmsBinaryEncodeTextStringLen fail \n");
				goto __CATCH;
			}

			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_TYPE) | 0x80;
			if (__MmsBinaryEncodeTextString(pFile, (UINT8*)szTextType, length) == false) {
				MSG_DEBUG("MmsBinaryEncodeContentType: type param MmsBinaryEncodeTextString fail \n");
				goto __CATCH;
			}
		}

		/* start = Text-string ----------------------- */
		if (pType->param.szStart[0]) {
			/* start = Text-string */
			length  = __MmsBinaryEncodeTextStringLen((UINT8*)pType->param.szStart);
			if (length == -1) {
				MSG_DEBUG("MmsBinaryEncodeContentType: szStart MmsBinaryEncodeTextStringLen fail \n");
				goto __CATCH;
			}

			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeParameterCode,
																		 MSG_PARAM_START) | 0x80;
			if (__MmsBinaryEncodeTextString(pFile, (UINT8*)pType->param.szStart, length) == false) {
				MSG_DEBUG("MmsBinaryEncodeContentType: szStart MmsBinaryEncodeTextString fail \n");
				goto __CATCH;
			}
		}

		/* startInfo = Text-string -------------------- */
		if (pType->param.szStartInfo[0]) {
			/* StartInfo (with multipart/related) = Text-string */
			length  = __MmsBinaryEncodeTextStringLen((UINT8*)pType->param.szStartInfo);
			if (length == -1) {
				MSG_DEBUG("MmsBinaryEncodeContentType: szStartInfo MmsBinaryEncodeTextStringLen fail \n");
				goto __CATCH;
			}

			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeParameterCode, MSG_PARAM_START_INFO) | 0x80;
			if (__MmsBinaryEncodeTextString(pFile, (UINT8*)pType->param.szStartInfo, length) == false) {
				MSG_DEBUG("MmsBinaryEncodeContentType: szStartInfo MmsBinaryEncodeTextString fail \n");
				goto __CATCH;
			}
		}
	}

	return true;

__CATCH:
	MSG_DEBUG("## MmsBinaryEncodeContentType: failed");
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
			MSG_DEBUG("MmsBinaryEncodeMsgPart: 1. headerLeng MmsBinaryEncodeUintvarLen fail \n");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeUintvar(pFile, pType->size, length) == false) {
			MSG_DEBUG("MmsBinaryEncodeMsgPart: 1. eaderLeng fail \n");
			goto __CATCH;
		}

		length = __MmsBinaryEncodeUintvarLen(pBody->size);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeMsgPart: 1. bodyLeng MmsBinaryEncodeUintvarLen fail \n");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeUintvar(pFile, pBody->size, length) == false) {
			MSG_DEBUG("MmsBinaryEncodeMsgPart: 1. bodyLeng fail \n");
			goto __CATCH;
		}

		pFile2 = MsgOpenFile(pType->szOrgFilePath, "rb");
		if (pFile != NULL) {
			pData = (char *)malloc(pType->size);
			if (pData == NULL)
				goto __CATCH;

			if (MsgFseek(pFile2, pType->offset, SEEK_SET) < 0) {
				MSG_DEBUG("MmsBinaryEncodeMsgPart: MsgFseek fail \n");
				goto __CATCH;
			}

			ULONG nRead = 0;

			if ((nRead = MsgReadFile(pData, sizeof(char), pType->size, pFile2)) == 0) {
				if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
													gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
					MSG_DEBUG("MmsBinaryEncodeMsgPart: 1. header MsgWriteDataFromEncodeBuffer fail \n");
					goto __CATCH;
				}
				pType->offset = __MmsGetEncodeOffset();
				if(MsgWriteFile(pData, sizeof(char), nRead, pFile) != (size_t)nRead) {
					MSG_DEBUG("MsgWriteFile failed");
					goto __CATCH;
				}
				gMmsEncodeCurOffset = MsgFtell(pFile);
				if(gMmsEncodeCurOffset < 0) {
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
			MSG_DEBUG("MmsBinaryEncodeMsgPart: headerLeng calculation fail \n");
			goto __CATCH;
		}

		headerLeng = contentTypeLen + contentHdrLen + length;
		length = __MmsBinaryEncodeUintvarLen(headerLeng);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeMsgPart: headerLeng MmsBinaryEncodeUintvarLen fail \n");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeUintvar(pFile, headerLeng, length) == false) {
			MSG_DEBUG("MmsBinaryEncodeMsgPart: headerLeng fail \n");
			goto __CATCH;
		}

		length = __MmsBinaryEncodeUintvarLen(pBody->size);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeMsgPart: bodyLeng MmsBinaryEncodeUintvarLen fail \n");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeUintvar(pFile, pBody->size, length) == false) {
			MSG_DEBUG("MmsBinaryEncodeMsgPart: bodyLeng fail \n");
			goto __CATCH;
		}

		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
							 				gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MmsBinaryEncodeMsgPart: 2. header MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}

		/* content-type & header --------------------------- */
		pType->offset = __MmsGetEncodeOffset();

		if (__MmsBinaryEncodeContentType(pFile, pType, contentTypeLen) == false) {
			MSG_DEBUG("MmsBinaryEncodeMsgPart: MmsBinaryEncodeContentType fail \n");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeContentHeader(pFile, (MimeType)contentType, pType, true) == false)	{
			MSG_DEBUG("MmsBinaryEncodeMsgPart: MmsBinaryEncodeContentHeader fail \n");
			goto __CATCH;
		}

		pType->size = __MmsGetEncodeOffset() - pType->offset;
	}

	if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos, gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
		MSG_DEBUG("MmsBinaryEncodeMsgPart: contentBody MsgWriteDataFromEncodeBuffer fail \n");
		goto __CATCH;
	}

	/* content-body --------------------------- */
	if (__MmsBinaryEncodeContentBody(pFile, pBody) == false) {
		MSG_DEBUG("MmsBinaryEncodeMsgPart: __MmsBinaryEncodeContentBody fail \n");
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

	MSG_DEBUG("## MmsBinaryEncodeMsgPart: failed\n");
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


	MSG_DEBUG("MmsBinaryEncodeContentHeaderLen: S T A R T \n\n");

	/* content-id ------------------------------------------------- */
	if (pType->szContentID[0]) {
		if (bMultipart) { //Binary Encoding
			totalLength++;
		} else {
			/* content-id = Quoted-string */
			length = __MmsBinaryEncodeTextStringLen((UINT8*)"Content-ID");
			if (length == -1) {
				MSG_DEBUG("MmsBinaryEncodeContentHeaderLen: Content-ID MmsBinaryEncodeTextStringLen fail.\n");
				goto __CATCH;
			}

			totalLength += length;
		}

		length = __MmsBinaryEncodeQuotedStringLen((UINT8*)pType->szContentID);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeContentHeader: pType->szContentID MmsBinaryEncodeQuotedStringLen fail.\n");
			goto __CATCH;
		}
		totalLength += length;
	}


	if (pType->szContentLocation[0]) {
		if (bMultipart) { //Binary Encoding
			totalLength++;
		} else {
			/* content-location = Quoted-string */
			length = __MmsBinaryEncodeTextStringLen((UINT8*)"Content-Location");
			if (length == -1) {
				MSG_DEBUG("MmsBinaryEncodeContentHeader: Content-Location MmsBinaryEncodeTextStringLen fail.\n");
				goto __CATCH;
			}

			totalLength += length;
		}

		length = __MmsBinaryEncodeTextStringLen((UINT8*)pType->szContentLocation);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeContentHeader: pType->szContentLocation MmsBinaryEncodeTextStringLen fail.\n");
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

		if (bMultipart) { //Binary Encoding
			totalLength += 3;
		} else {
			/* content-disposition = Quoted-string */
			szTextValue = MmsGetTextValue(MmsCodeMsgDisposition, pType->disposition);

			if (szTextValue) {
				length = __MmsBinaryEncodeTextStringLen((UINT8*)"Content-Disposition");
				if (length == -1) {
					MSG_DEBUG("MmsBinaryEncodeContentHeader: Content-Disposition MmsBinaryEncodeTextStringLen fail.\n");
					goto __CATCH;
				}

				totalLength += length;

				length = __MmsBinaryEncodeTextStringLen((UINT8*)szTextValue);
				if (length == -1) {
					MSG_DEBUG("MmsBinaryEncodeContentHeader: Content-Disposition MmsBinaryEncodeTextStringLen fail.\n");
					goto __CATCH;
				}
				totalLength += length;
			}
		}
	}

	return totalLength;

__CATCH:
	MSG_DEBUG("## MmsBinaryEncodeContentHeadeLen: failed");

	return -1;
}

static bool __MmsBinaryEncodeContentHeader(FILE *pFile, MimeType contentType, MsgType *pType, bool bMultipart)
{
	int	length = 0;
	const char *szTextValue = NULL;

	MSG_DEBUG("MmsBinaryEncodeContentHeader: S T A R T \n\n");

	/* content-id ------------------------------------------------- */
	if (pType->szContentID[0]) {
		if (bMultipart) { //Binary Encoding
			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeMsgBodyHeaderCode, MMS_BODYHDR_CONTENTID) | 0x80;
		} else {
			/* content-id = Quoted-string */
			length = __MmsBinaryEncodeTextStringLen((UINT8*)"Content-ID");
			if (length == -1) {
				MSG_DEBUG("MmsBinaryEncodeContentHeader: Content-ID MmsBinaryEncodeTextStringLen fail.\n");
				goto __CATCH;
			}

			if (__MmsBinaryEncodeTextString(pFile, (UINT8*)"Content-ID", length) == false) {
				MSG_DEBUG("MmsBinaryEncodeContentHeader: Content-ID MmsBinaryEncodeTextString fail.\n");
				goto __CATCH;
			}
		}

		length = __MmsBinaryEncodeQuotedStringLen((UINT8*)pType->szContentID);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeContentHeader: pType->szContentID MmsBinaryEncodeQuotedStringLen fail.\n");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeQuotedString(pFile, (UINT8*)pType->szContentID, length) == false) {
			MSG_DEBUG("MmsBinaryEncodeContentHeader: pType->szContentID MmsBinaryEncodeQuotedString fail.\n");
			goto __CATCH;
		}
	}

	if (pType->szContentLocation[0]) {
		if (bMultipart) { //Binary Encoding
			gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeMsgBodyHeaderCode, MMS_BODYHDR_CONTENTLOCATION) | 0x80;
		} else {
			/* content-location = Quoted-string */
			length = __MmsBinaryEncodeTextStringLen((UINT8*)"Content-Location");
			if (length == -1) {
				MSG_DEBUG("MmsBinaryEncodeContentHeader: Content-Location MmsBinaryEncodeTextStringLen fail.\n");
				goto __CATCH;
			}

			if (__MmsBinaryEncodeTextString(pFile, (UINT8*)"Content-Location", length) == false) {
				MSG_DEBUG("MmsBinaryEncodeContentHeader: Content-Location MmsBinaryEncodeTextString fail.\n");
				goto __CATCH;
			}
		}

		length = __MmsBinaryEncodeTextStringLen((UINT8*)pType->szContentLocation);
		if (length == -1) {
			MSG_DEBUG("MmsBinaryEncodeContentHeader: pType->szContentLocation MmsBinaryEncodeTextStringLen fail.\n");
			goto __CATCH;
		}

		if (__MmsBinaryEncodeTextString(pFile, (UINT8*)pType->szContentLocation, length) == false) {
			MSG_DEBUG("MmsBinaryEncodeContentHeader: pType->szContentLocation MmsBinaryEncodeTextString fail.\n");
			goto __CATCH;
		}
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

		if (bMultipart) {//Binary Encoding

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
					MSG_DEBUG("MmsBinaryEncodeContentHeader: Content-Disposition MmsBinaryEncodeTextStringLen fail.\n");
					goto __CATCH;
				}

				if (__MmsBinaryEncodeTextString(pFile, (UINT8*)"Content-Disposition", length) == false) {
					MSG_DEBUG("MmsBinaryEncodeContentHeader: Content-Disposition MmsBinaryEncodeTextString fail.\n");
					goto __CATCH;
				}

				length = __MmsBinaryEncodeTextStringLen((UINT8*)szTextValue);
				if (length == -1) {
					MSG_DEBUG("MmsBinaryEncodeContentHeader: Content-Disposition MmsBinaryEncodeTextStringLen fail.\n");
					goto __CATCH;
				}

				if (__MmsBinaryEncodeTextString(pFile, (UINT8*)szTextValue, length) == false) {
					MSG_DEBUG("MmsBinaryEncodeContentHeader: Content-Disposition MmsBinaryEncodeTextString fail.\n");
					goto __CATCH;
				}
			}
		}
	}

	return true;

__CATCH:
	MSG_DEBUG("## MmsBinaryEncodeContentHeader: failed");
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
		if(MsgWriteFile(pData, sizeof(char), nRead, pFile) != (size_t)nRead) {
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
		if (MsgWriteFile(pBody->body.pText, sizeof(char),(size_t)pBody->size, pFile) != (size_t)pBody->size) {
			MSG_DEBUG("MsgWriteFile failed");
			goto __CATCH;
		}
		gMmsEncodeCurOffset = MsgFtell(pFile);

		if(gMmsEncodeCurOffset < 0) {
			MSG_DEBUG("MsgFtell returns negative value [%ld]", gMmsEncodeCurOffset);
			goto __CATCH;
		}
	}

	return true;

__CATCH:
	MSG_DEBUG("## __MmsBinaryEncodeContentBody: failed\n");
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

		length++;	// + Short-length

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
		MSG_DEBUG("MmsBinaryEncodeInteger: source == NULL \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MmsBinaryEncodeInteger: MsgWriteDataFromEncodeBuffer fail \n");
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
		length--;					// - "Short-length"

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

	length++;	// + Short-length

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
		MSG_DEBUG("MmsBinaryEncodeLongInteger: source == NULL \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MmsBinaryEncodeLongInteger: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	changer.integer = integer;
	length--;					// - "Short-length"

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

	MSG_DEBUG("MmsBinaryEncodeTextStringLen: \n");

	if (source == NULL) {
		MSG_DEBUG("MmsBinaryEncodeTextStringLen: source == NULL \n");
		return -1;
	}

	length = (int)strlen((char*)source);
	if (source[0] > 0x7F) {
		length += 2;			// + NULL
	} else {
		length++;				// + NULL
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

	MSG_DEBUG("MmsBinaryEncodeTextString: \n");

	/*
	 * make text string
	 * Text-string = [Quote] *TEXT End-of-string
	 * If the 1st char in the TEXT is in the range of 128-255, a Quote char must precede it.
	 * Otherwise the Quote char must be omitted.
	 * The Quote is not part of the contents.
	 * Quote = <Octet 127>
	 */

	if (pFile == NULL || source == NULL) {
		MSG_DEBUG("MmsBinaryEncodeTextString: source == NULL \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MmsBinaryEncodeTextString: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	if (source[0] > 0x7F) {
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = QUOTE;
		length--;
	}

	strncpy(gpMmsEncodeBuf + gCurMmsEncodeBuffPos, (char*)source, (length - 1));	// except NULL
	gCurMmsEncodeBuffPos += (length - 1);			// except NULL
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
const UINT32 UINTVAR_LENGTH_1 =  0x0000007f;		//7bit
const UINT32 UINTVAR_LENGTH_2 =  0x00003fff;		//14bit
const UINT32 UINTVAR_LENGTH_3 =  0x001fffff;		//21bit


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
		MSG_DEBUG("MmsBinaryEncodeUintvar: pFile == INVALID_HBOJ \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MmsBinaryEncodeUintvar: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	source.integer = integer;
	memset(szReverse, 0, MSG_STDSTR_LONG);

	/* Seperate integer to 4 1 byte integer */
	szReverse[3] = source.bytes[3] & 0x0f;
	szReverse[0] = source.bytes[0];
	szReverse[0] = szReverse[0] & 0x7f;

	while (length >= i) { // initially, i = 2
		/* Move integer bit to proper position */
		source.integer = source.integer << 1;
		source.integer = source.integer >> 8;
		source.bytes[3] = ZERO;

		/* Retrive 1 encode uintvar */
		szReverse[i-1] = source.bytes[0];
		szReverse[i-1] = szReverse[i-1] | 0x80;
		i++;
	}

	for(i=0; i < length; i++)
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
		length = __MmsBinaryEncodeUintvarLen(integer) + 1;		//LENGTH_QUOTE
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
		MSG_DEBUG("MmsBinaryEncodeValueLength: pFile == INVALID_HBOJ \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MmsBinaryEncodeValueLength: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	if (integer < 0x1F) {
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)integer;
	} else {
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)LENGTH_QUOTE;
		if (__MmsBinaryEncodeUintvar(pFile, integer, length - 1) == false) {	// LENGTH_QUOTE
			MSG_DEBUG("MmsBinaryEncodeValueLength: MmsBinaryEncodeUintvar fail\n");
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
		MSG_DEBUG("MmsBinaryEncodeQuotedStringLen: invalid file\n");
		goto __CATCH;
	}

	return (strlen((char*)pSrc) + 2);	// QUOTE + NULL

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
		MSG_DEBUG("MmsBinaryEncodeQuotedString: invalid file\n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MmsBinaryEncodeQuotedString: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = '\"';
	strncpy(gpMmsEncodeBuf + gCurMmsEncodeBuffPos, (char*)source, length - 2);		// except '\"' & NULL
	gCurMmsEncodeBuffPos += (length - 2);
	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = (UINT8)NULL;

	return true;

__CATCH:
	return false;
}

static int __MmsBinaryEncodeEncodedStringLen(UINT8 *source)
{
	UINT32 charset = 0x6A;		// default = utf-8
	int charLeng		= 0;
	int textLeng		= 0;
	int valueLengthLen	= 0;


	MSG_DEBUG("MmsBinaryEncodeEncodedStringLen: \n");

	/* value-length charSet text-string */
	/* Estimate charset value length and text string length */
	charLeng = __MmsBinaryEncodeIntegerLen(charset);
	if (charLeng == -1) {
		MSG_DEBUG("MmsBinaryEncodeEncodedStringLen : charLeng MmsBinaryEncodeTextStringLen fail.\n");
		goto __CATCH;;
	}

	textLeng = __MmsBinaryEncodeTextStringLen((UINT8*)source);
	if (textLeng == -1)	{
		MSG_DEBUG("MmsBinaryEncodeEncodedStringLen : textLeng MmsBinaryEncodeTextStringLen fail.\n");
		goto __CATCH;;
	}

	valueLengthLen = __MmsBinaryEncodeValueLengthLen(charLeng + textLeng);
	if (valueLengthLen == -1) {
		MSG_DEBUG("MmsBinaryEncodeEncodedStringLen : valLengthLen MmsBinaryEncodeTextStringLen fail.\n");
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
	UINT32 charset = 0x6A;		// default = utf-8
	int charLeng = 0;
	int textLeng = 0;
	int valLengthLen = 0;


	MSG_DEBUG("MmsBinaryEncodeEncodedString: \n");

	/* value-length charSet text-string */

	if (pFile == NULL || source == NULL) {
		MSG_DEBUG("MmsBinaryEncodeEncodedString: invalid input parameter\n");
		goto __CATCH;
	}

	/* Estimate charset value length and text string length */
	charLeng = __MmsBinaryEncodeIntegerLen(charset);
	if (charLeng == -1) {
		MSG_DEBUG("MmsBinaryEncodeEncodedString : charLeng MmsBinaryEncodeTextStringLen fail.\n");
		goto __CATCH;;
	}

	textLeng = __MmsBinaryEncodeTextStringLen((UINT8*)source);
	if (textLeng == -1) {
		MSG_DEBUG("MmsBinaryEncodeEncodedString : textLeng MmsBinaryEncodeTextStringLen fail.\n");
		goto __CATCH;;
	}

	valLengthLen = __MmsBinaryEncodeValueLengthLen(charLeng + textLeng);
	if (valLengthLen == -1) {
		MSG_DEBUG("MmsBinaryEncodeEncodedString : MmsBinaryEncodeValueLengthLen fail.\n");
		goto __CATCH;
	}

	if (length != (charLeng + textLeng + valLengthLen))
	{
		MSG_DEBUG("MmsBinaryEncodeEncodedString: invalid length\n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("MmsBinaryEncodeEncodedString: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	/* Value length of charset value and text string */
	if (__MmsBinaryEncodeValueLength(pFile, charLeng + textLeng, valLengthLen) == false) {
		MSG_DEBUG("MmsBinaryEncodeEncodedString : MmsBinaryEncodeValueLength fail.\n");
		goto __CATCH;
	}

	/* fixme: Write charset on buffer -> integer value not long-integer */
	if (__MmsBinaryEncodeInteger(pFile, charset, charLeng) == false) {
		MSG_DEBUG("MmsBinaryEncodeEncodedString : MmsBinaryEncodeInteger fail.\n");
		goto __CATCH;
	}


	/* Write text string on buffer */
	if (__MmsBinaryEncodeTextString(pFile, (UINT8*)source, textLeng) == false) {
		MSG_DEBUG("MmsBinaryEncodeEncodedString : MmsBinaryEncodeTextString fail.\n");
		goto __CATCH;
	}

	return true;

__CATCH:
	return false;
}

static	bool __MmsBinaryEncodeFieldCodeAndValue(FILE *pFile, UINT8 fieldCode, UINT8 fieldValue)
{
	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < 2) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
										gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("__MmsBinaryEncodeFieldCodeAndValue: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	if (fieldCode == 0xff) {
		MSG_DEBUG("__MmsBinaryEncodeFieldCodeAndValue: invalid fieldCode \n");
		goto __CATCH;
	}

	if (fieldValue == 0xff) {
		MSG_DEBUG("__MmsBinaryEncodeFieldCodeAndValue: invalid fieldValue \n");
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
	struct	tm	*dateTime = NULL;
	time_t	RawTime = 0;
	time_t	dateSec = 0;

	time(&RawTime);
	dateTime = localtime(&RawTime);
	dateSec = mktime(dateTime);


	fieldCode = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_TRID) | 0x80;
	if (fieldCode == 0xff) {
		MSG_DEBUG("__MmsBinaryEncodeTrID: invalid fieldCode \n");
		goto __CATCH;
	}

	snprintf(szBuff, MMS_TR_ID_LEN + 1, "%lu.%lu", dateSec, (unsigned long)random());
	MSG_DEBUG("__MmsBinaryEncodeTrID: 2. szBuff = %s\n", szBuff);

	length = __MmsBinaryEncodeTextStringLen((UINT8*)szBuff);
	if (length == -1) {
		MSG_DEBUG("__MmsBinaryEncodeTrID: MmsBinaryEncodeTextStringLen fail \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < (length + 1)) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("__MmsBinaryEncodeTrID: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}


	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = fieldCode;
	if (__MmsBinaryEncodeTextString(pFile, (UINT8*)szBuff, length) == false) {
		MSG_DEBUG("__MmsBinaryEncodeTrID: MmsBinaryEncodeTextString fail\n");
		goto __CATCH;
	}

	if (szTrID) {
		memset(szTrID, 0, bufLen);
		strncpy(szTrID, szBuff, bufLen - 1);
	}

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
		MSG_DEBUG("__MmsBinaryEncodeTrID: invalid fieldCode \n");
		goto __CATCH;
	}

	MSG_DEBUG("__MmsBinaryEncodeMsgID: 2. szBuff = %s\n", szMsgID);

	length = __MmsBinaryEncodeTextStringLen((UINT8*)szMsgID);
	if (length == -1) {
		MSG_DEBUG("__MmsBinaryEncodeMsgID: MmsBinaryEncodeTextStringLen fail \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < (length + 1)) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
										gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("__MmsBinaryEncodeTrID: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = fieldCode;
	if (__MmsBinaryEncodeTextString(pFile, (UINT8*)szMsgID, length) == false)	{
		MSG_DEBUG("__MmsBinaryEncodeTrID: MmsBinaryEncodeTextString fail\n");
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

	MSG_DEBUG("__MmsBinaryEncodeMmsVersion: \n");

	if (pFile == NULL) {
		MSG_DEBUG("__MmsBinaryEncodeMmsVersion: invalid input file \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < 2) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
										gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("__MmsBinaryEncodeMmsVersion: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_VERSION) | 0x80;

	MSG_DEBUG("__MmsBinaryEncodeMmsVersion: major version (%d)\n", majorVer);
	MSG_DEBUG("__MmsBinaryEncodeMmsVersion: minor version (%d)\n", minorVer);

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos] = (majorVer << 4) | (minorVer & 0x0f) | MSB;

	if (gpMmsEncodeBuf[gCurMmsEncodeBuffPos] < 0x80) {
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] |= 0x80;
	} else {
		gCurMmsEncodeBuffPos++;
	}

	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeDate(FILE *pFile)
{
	struct	tm	*dateTime = NULL;
	time_t	dateSec = 0;

	dateSec = time(NULL);
	dateTime = localtime(&dateSec);

	MSG_DEBUG("%d - %d - %d, %d : %d (SYSTEM)", dateTime->tm_year + 1900, dateTime->tm_mon + 1, dateTime->tm_mday
		, dateTime->tm_hour, dateTime->tm_min);

	if (dateSec > 0) {
		int	length	= 0;
		length = __MmsBinaryEncodeLongIntegerLen(dateSec);

		if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < (length + 1)) {	// + fieldCode
			if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
												gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
				MSG_DEBUG("__MmsBinaryEncodeDate: MsgWriteDataFromEncodeBuffer fail \n");
				goto __CATCH;
			}
		}

		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_DATE) | 0x80;

		if (__MmsBinaryEncodeLongInteger(pFile, dateSec, length) == false) {
			MSG_DEBUG("__MmsBinaryEncodeDate: date MmsBinaryEncodeLongInteger error\n");
			goto __CATCH;
		}
	} else {
		MSG_DEBUG("__MmsBinaryEncodeDate: date has a negative value (%d) \n", dateSec);
		goto __CATCH;
	}
	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeFrom(FILE *pFile)
{
	if (pFile == NULL) {
		MSG_DEBUG("__MmsBinaryEncodeFrom: invalid input file \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < 3) {
		if (MsgWriteDataFromEncodeBuffer(pFile, gpMmsEncodeBuf, &gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("__MmsBinaryEncodeFrom: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, MMS_CODE_FROM) | 0x80;
	/* length of MMS_INSERT_ADDRESS_TOKEN value */
	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = 0x01;
	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeAddressType, MMS_INSERT_ADDRESS_TOKEN) | 0x80;
	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeOneAddress(FILE *pFile, MmsFieldCode addrType, char *szAddrStr)
{
	int length = 0;

	if (pFile == NULL) {
		MSG_DEBUG("__MmsBinaryEncodeOneAddress: invalid input file \n");
		goto __CATCH;
	}

	/* EncodedString */
	length = __MmsBinaryEncodeEncodedStringLen((UINT8*)szAddrStr);

	if (length == -1) {
		MSG_DEBUG("__MmsBinaryEncodeOneAddress: MmsBinaryEncodeEncodedStringLen fail \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < length + 1) {
		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("__MmsBinaryEncodeOneAddress: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, addrType) | 0x80;


	if (__MmsBinaryEncodeEncodedString(pFile, (UINT8*)szAddrStr, length) == false) {
		MSG_DEBUG("__MmsBinaryEncodeOneAddress: MmsBinaryEncodeEncodedString fail \n");
		goto __CATCH;
	}

	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryEncodeAddress(FILE *pFile, MmsFieldCode addrType, char *szAddr)
{
	char *pSingleAddr = NULL;


	if (pFile == NULL) {
		MSG_DEBUG("__MmsBinaryEncodeAddress: invalid input file \n");
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
				MSG_DEBUG("__MmsBinaryEncodeAddress: __MmsBinaryEncodeAddress fail\n");
				goto __CATCH;
			}
			*pSingleAddr = MSG_CH_SEMICOLON;

			szAddr		 = pSingleAddr + 1;
			pSingleAddr  = NULL;
		} else {
			if (__MmsBinaryEncodeOneAddress(pFile, addrType, szAddr) == false) {
				MSG_DEBUG("__MmsBinaryEncodeAddress: __MmsBinaryEncodeAddress fail\n");
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
		MSG_DEBUG("__MmsBinaryEncodeTime: invalid input file \n");
		goto __CATCH;
	}

	if (time.time == 0 ||
		(fieldCode != MMS_CODE_EXPIRYTIME && fieldCode != MMS_CODE_DELIVERYTIME && fieldCode != MMS_CODE_REPLYCHARGINGDEADLINE) ||
		(time.type != MMS_TIMETYPE_ABSOLUTE && time.type != MMS_TIMETYPE_RELATIVE)) {
		MSG_DEBUG("__MmsBinaryEncodeTime: time.type = %d \n", time.type);
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
		MSG_DEBUG("__MmsBinaryEncodeTime: MmsBinaryEncodeLongIntegerLen fail \n");
		goto __CATCH;
	}

	length = __MmsBinaryEncodeValueLengthLen(timeLen + 1);	//time length + time type token
	if (length == -1) {
		MSG_DEBUG("__MmsBinaryEncodeTime: MmsBinaryEncodeValueLengthLen fail \n");
		goto __CATCH;
	}

	if ((gMmsEncodeMaxLen - gCurMmsEncodeBuffPos) < (length + timeLen + 2)) {	// + fieldCode + timeType

		if (MsgWriteDataFromEncodeBuffer(pFile,	gpMmsEncodeBuf,	&gCurMmsEncodeBuffPos,
											gMmsEncodeMaxLen, &gMmsEncodeCurOffset) == false) {
			MSG_DEBUG("_MmsBinaryEncodeSendReqHdr: MsgWriteDataFromEncodeBuffer fail \n");
			goto __CATCH;
		}
	}

	/* fieldCode */
	gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeFieldCode, fieldCode) | 0x80;

	/* value length */
	if (__MmsBinaryEncodeValueLength(pFile, timeLen + 1, length) == false) {
		MSG_DEBUG("__MmsBinaryEncodeTime: MmsBinaryEncodeValueLength fail \n");
		goto __CATCH;
	}

	/* time type & value */
	if (time.type == MMS_TIMETYPE_RELATIVE) {
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeTimeType, MMS_TIMETYPE_RELATIVE) | 0x80;
		if (__MmsBinaryEncodeInteger(pFile, time.time, timeLen) == false)	{
			MSG_DEBUG("__MmsBinaryEncodeTime: MmsBinaryEncodeLongInteger fail \n");
			goto __CATCH;
		}
	} else {
		gpMmsEncodeBuf[gCurMmsEncodeBuffPos++] = MmsGetBinaryValue(MmsCodeTimeType, MMS_TIMETYPE_ABSOLUTE) | 0x80;
		if (__MmsBinaryEncodeLongInteger(pFile, time.time, timeLen) == false) {
			MSG_DEBUG("__MmsBinaryEncodeTime: MmsBinaryEncodeLongInteger fail \n");
			goto __CATCH;
		}
	}

	return true;

__CATCH:
	return false;
}
