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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

#include "MsgUtilFile.h"

#include "MmsPluginDebug.h"
#include "MmsPluginDecode.h"
#include "MmsPluginCodecCommon.h"
#include "MmsPluginStorage.h"
#include "MmsPluginDebug.h"
#include "MmsPluginMIME.h"
#include "MmsPluginAvCodec.h"
#include "MmsPluginSmil.h"
#include "MmsPluginTextConvert.h"
#include "MmsPluginUtil.h"

#ifdef __SUPPORT_DRM__
#include "MmsPluginDrm.h"
#include "MsgDrmWrapper.h"
#endif


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

#ifdef __SUPPORT_DRM__
static bool __MmsBinaryDecodeDRMContent(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, unsigned int bodyLength, int totalLength);
#endif

//util funcion
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
static MsgMultipart *__MsgAllocMultipart(void);

static char *__MsgConvertLatin2UTF8FileName(char *pSrc);
static bool __MsgIsUTF8String(unsigned char *szSrc, int nChar);
static bool __MsgIsPercentSign(char *pSrc);
static bool __MsgIsMultipartRelated(int type);
static bool __MsgIsPresentablePart(int type);
static bool __MsgIsText(int type);
static bool __MsgResolveNestedMultipart(MsgType *pPartType, MsgBody *pPartBody);
static bool __MsgIsHexChar(char *pSrc);
static char __MsgConvertHexValue(char *pSrc);
static int __MsgConvertCharToInt(char ch);
static bool __MsgCopyNestedMsgType(MsgType *pMsgType1, MsgType *pMsgType2);
static bool __MsgCopyNestedMsgParam(MsgContentParam *pParam1, MsgContentParam *pParam2);
static bool __MsgIsMultipartMixed(int type);

static bool __MsgIsInvalidFileNameChar(char ch);

static int __MsgGetLatin2UTFCodeSize(unsigned char *szSrc, int nChar);
static int __MsgLatin5code2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar);
static int __MsgGetLatin52UTFCodeSize(unsigned char *szSrc, int nChar);
static int __MsgLatin2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar);
static int __MsgLatin7code2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar);
static int __MsgGetLatin72UTFCodeSize(unsigned char *szSrc, int nChar);
static int __MsgUnicode2UTF(unsigned char *des, int outBufSize, unsigned short *szSrc, int nChar);
static int __MsgGetUnicode2UTFCodeSize(unsigned short *szSrc, int nChar);
static bool __MmsAddrUtilCheckEmailAddress(char *pszAddr);
static int __MsgCutUTFString(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar);
static void __MsgMIMERemoveQuote(char *szSrc);
static bool __MmsMultipartSaveAsTempFile(MsgType *pPartType, MsgBody *pPartBody, char *pszMailboxPath, char *pszMsgFilename, int index, bool bSave);

static bool __MmsGetMediaPartData(MsgType *pPartType, MsgBody *pPartBody, FILE *pFile);
static char *__MmsGetBinaryUTF8Data(char *pData, int nRead, int msgEncodingValue, int msgTypeValue, int msgCharsetValue, int *npRead);

#ifndef	__SUPPORT_DRM__
static bool __MsgMakeFileName(int iMsgType, char *szFileName, int nUntitleIndex);
#else
static bool __MsgMakeFileName(int iMsgType, char *szFileName, MsgDrmType drmType, int nUntitleIndex);
#endif

static bool __MmsDebugPrintMulitpartEntry(MsgMultipart *pMultipart, int index);

static char gszMmsLoadBuf1[MSG_MMS_DECODE_BUFFER_MAX + 1] = {0, };
static char gszMmsLoadBuf2[MSG_MMS_DECODE_BUFFER_MAX + 1] = {0, };

static char *gpCurMmsDecodeBuff = NULL;
static int gCurMmsDecodeBuffPos = 0;	/* next decoding position in gpCurMmsDecodeBuff  */
static int gMmsDecodeMaxLen = 0;
static int gMmsDecodeCurOffset = 0;	/* current offset in file (last read) */
static int gMmsDecodeBufLen = 0;		/* number of last read characters */

static char *gpMmsDecodeBuf1 = NULL;
static char *gpMmsDecodeBuf2 = NULL;

MmsHeader	mmsHeader =
{
	false,									//bActive
	NULL,									//pszOwner
	-1,										//msgID

	(MmsMsgType)MMS_MSGTYPE_ERROR,			//MmsMsgType			iType;
	"",										//char[]				szTrID;
	//"",										//short int				version;
	0,										//short int				version;
	0,										//UINT32				date;

	NULL,									//MsgHeaderAddress*		pFrom;
	NULL,									//MsgHeaderAddress*		pTo;
	NULL,									//MsgHeaderAddress*		pCc;
	NULL,									//MsgHeaderAddress*		pBcc;
	"",										//char[]				szSubject;
	(MmsResponseStatus)MMS_RESPSTATUS_OK,	//MmsResponseStatus		iResponseStatus;
	(MmsRetrieveStatus)MMS_RETRSTATUS_OK,	//MmsRetrieveStatus		iRetrieveStatus;
	"",										//char[]				szResponseText;
	"",										//char[]				szRetrieveText;


	/* has default value in specification */

	(MmsMsgClass)MMS_MSGCLASS_PERSONAL,		//MmsMsgClass			msgClass;
	{MMS_TIMETYPE_RELATIVE, 0},				//MmsTimeStruct			expiryTime;
	{MMS_TIMETYPE_RELATIVE, 0},				//MmsTimeStruct			deliveryTime;
	(MmsPriority)MMS_PRIORITY_NORMAL,		//MmsPriority			priority;		// Refer [OMA-MMS-ENC-v1_2-20030915-C]
	(MmsSenderVisible)MMS_SENDER_SHOW,		//MmsSenderVisible		senderVisible;
	(MmsReport)MMS_REPORT_NO,				//MmsReport				deliveryReport;
	(MmsReport)MMS_REPORT_NO,				//MmsReport				readReply;
	(MmsReportAllowed)MMS_REPORTALLOWED_NO,//MmsReportAllowed		iReportAllowed;
	"",										//char[]				szContentLocation;


	/* there is no right default value */

	(msg_delivery_report_status_t)MSG_DELIVERY_REPORT_NONE,		//MmsMsgStatus			iMsgStatus;
	(msg_read_report_status_t)MSG_READ_REPORT_NONE,		//MmsReadStatus			readStatus;

	/* MMS v1.1 ReplyCharge */
	{
		(MmsReplyChargeType)MMS_REPLY_NONE,	//MmsReplyChargeType	chargeType;
		{MMS_TIMETYPE_RELATIVE, 0},			//MmsTimeStruct			deadLine;
		0,									//int					chargeSize;
		"" ,								//char					szChargeID;
	},

	"",										//char[]				szMsgID;
	0,										//UINT32				msgSize;
};

#ifdef __SUPPORT_DRM__

#define	MMS_DRM2_CONVERT_BUFFER_MAX	4*1024
const UINT32 MMS_UINTVAR_LENGTH_1 =  0x0000007f;		//7bit
const UINT32 MMS_UINTVAR_LENGTH_2 =  0x00003fff;		//14bit
const UINT32 MMS_UINTVAR_LENGTH_3 =  0x001fffff;		//21bit
#endif

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
	mmsHeader.priority = (MmsPriority)MMS_PRIORITY_NORMAL;	// Refer [OMA-MMS-ENC-v1_2-20030915-C]
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
#ifdef __SUPPORT_DRM__
	mmsHeader.drmType = MSG_DRM_TYPE_NONE;
#endif

	__MmsDecodeInitialize();
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

	MSG_DEBUG("pFile=%d, total len=%d\n", pFile, totalLength);

	__MmsCleanDecodeBuff();

	if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos,
								   &gMmsDecodeCurOffset, gpMmsDecodeBuf1, gpMmsDecodeBuf2,
								   gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("fail to load to buffer \n");
		goto __CATCH;
	}

	while (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength)) {
		fieldCode = oneByte & 0x7f;

		switch (MmsGetBinaryType(MmsCodeFieldCode, fieldCode)) {
		case MMS_CODE_RESPONSESTATUS:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("responseStatus GetOneByte fail\n");
				goto __CATCH;
			}

			fieldValue = MmsGetBinaryType(MmsCodeResponseStatus, (UINT16)(oneByte & 0x7F));

			if (fieldValue == 0xFFFF) {
				MSG_DEBUG("responseStatus error\n");
				goto __CATCH;
			}

			if (fieldValue >= 0x0044 && fieldValue <= 0x005F) {
				fieldValue = 0x0040;
			} else if (fieldValue >= 0x006A && fieldValue <= 0x007F) {
				fieldValue = 0x0060;
			}

			mmsHeader.responseStatus = (MmsResponseStatus)fieldValue;

			MSG_DEBUG("response status = %s\n", MmsDebugGetResponseStatus(mmsHeader.responseStatus));

			break;

		case MMS_CODE_RETRIEVESTATUS:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("retrieveStatus GetOneByte fail\n");
				goto __CATCH;
			}

			fieldValue = MmsGetBinaryType(MmsCodeRetrieveStatus, (UINT16)(oneByte & 0x7F));

			if (fieldValue == 0xFFFF) {
				MSG_DEBUG("retrieveStatus error\n");
				goto __CATCH;
			}

			if (fieldValue >= 0x0043 && fieldValue <= 0x005F) {
				fieldValue = 0x0040; // 192; Error-transient-failure
			} else if (fieldValue >= 0x0064 && fieldValue <= 0x007F) {
				fieldValue = 0x0060; //224; Error-permanent-failure
			}

			mmsHeader.retrieveStatus = (MmsRetrieveStatus)fieldValue;

			MSG_DEBUG("retrieve status = %s\n",
															MmsDebugGetRetrieveStatus(mmsHeader.retrieveStatus));

			break;

		case MMS_CODE_RESPONSETEXT:

			if (__MmsBinaryDecodeEncodedString(pFile, mmsHeader.szResponseText, MMS_LOCALE_RESP_TEXT_LEN + 1, totalLength) == false) {
				MSG_DEBUG("invalid MMS_CODE_RESPONSETEXT \n");
				goto __CATCH;
			}

			MSG_DEBUG("response text = %s\n", mmsHeader.szResponseText);
			break;

		case MMS_CODE_RETRIEVETEXT:

			if (__MmsBinaryDecodeEncodedString(pFile, mmsHeader.szRetrieveText, MMS_LOCALE_RESP_TEXT_LEN + 1, totalLength) == false) {
				MSG_DEBUG("invalid MMS_CODE_RETRIEVETEXT \n");
				goto __CATCH;
			}

			MSG_DEBUG("retrieve text = %s\n", mmsHeader.szResponseText);
			break;

		case MMS_CODE_MSGID:

			if (__MmsBinaryDecodeText(pFile, mmsHeader.szMsgID, MMS_MSG_ID_LEN + 1, totalLength) < 0) {
				MSG_DEBUG("MMS_CODE_MSGID is invalid\n");
				goto __CATCH;
			}

			MSG_DEBUG("msg id = %s\n", mmsHeader.szMsgID);

			if (MsgStrlen (mmsHeader.szMsgID) > 2)
				__MsgMIMERemoveQuote (mmsHeader.szMsgID);

			break;

		case MMS_CODE_SUBJECT:

			if (__MmsBinaryDecodeEncodedString(pFile, mmsHeader.szSubject, MSG_LOCALE_SUBJ_LEN + 1, totalLength) == false) {
				MSG_DEBUG("invalid MMS_CODE_SUBJECT \n");
				goto __CATCH;
			}

			pLimitData = (char *)malloc(MSG_LOCALE_SUBJ_LEN + 1);

			if (pLimitData == NULL) {
				MSG_DEBUG("pLimitData malloc fail \n");
				goto __CATCH;
			}

			nRead = __MsgCutUTFString((unsigned char*)pLimitData, MSG_LOCALE_SUBJ_LEN + 1, (unsigned char*)mmsHeader.szSubject, MSG_SUBJ_LEN);
			MSG_DEBUG("Subject edit.. \n");

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

			MSG_DEBUG("subject = %s\n", mmsHeader.szSubject);
			break;

		case MMS_CODE_FROM:

			/* Value-length (Address-present-token Encoded-string-value | Insert-address-token) */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("MMS_CODE_FROM is invalid\n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MMS_CODE_FROM GetOneByte fail\n");
				goto __CATCH;
			}

			// DRM_TEMPLATE - start

			valueLength--;

			if (oneByte == (MmsGetBinaryValue(MmsCodeAddressType, MMS_ADDRESS_PRESENT_TOKEN)|0x80)) {
				if (valueLength > 0) {
					mmsHeader.pFrom = __MmsDecodeEncodedAddress(pFile, totalLength);
					if (mmsHeader.pFrom == NULL) {
						MSG_DEBUG("MMS_CODE_FROM __MmsDecodeEncodedAddress fail\n");
						goto __CATCH;
					}
				} else {
					mmsHeader.pFrom = (MsgHeaderAddress *)malloc(sizeof(MsgHeaderAddress));
					if (mmsHeader.pFrom == NULL)
						goto __CATCH;

					mmsHeader.pFrom->szAddr = (char *)malloc(1);
					if (mmsHeader.pFrom->szAddr == NULL) {
						free(mmsHeader.pFrom);
						mmsHeader.pFrom = NULL;
						goto __CATCH;
					}

					mmsHeader.pFrom->szAddr[0] = '\0';
					mmsHeader.pFrom->pNext = NULL;
				}

				MSG_DEBUG("from = %s\n", mmsHeader.pFrom->szAddr);
				// DRM_TEMPLATE - end
			} else if (oneByte == (MmsGetBinaryValue(MmsCodeAddressType, MMS_INSERT_ADDRESS_TOKEN)|0x80)) {
				/* Present Token only */
				MSG_DEBUG("MMS_CODE_FROM insert token\n");
			} else {
				/* from data broken */
				MSG_DEBUG("from addr broken\n");
				gCurMmsDecodeBuffPos--;
				goto __CATCH;
			}
			break;

		case MMS_CODE_TO:

			pAddr = __MmsDecodeEncodedAddress(pFile, totalLength);
			if (pAddr == NULL) {
				MSG_DEBUG("MMS_CODE_TO __MmsDecodeEncodedAddress fail\n");
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

			MSG_DEBUG("to = %s\n", mmsHeader.pTo->szAddr);
			break;

		case MMS_CODE_BCC:

			pAddr = __MmsDecodeEncodedAddress(pFile, totalLength);
			if (pAddr == NULL) {
				MSG_DEBUG("MMS_CODE_BCC __MmsDecodeEncodedAddress fail\n");
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

			MSG_DEBUG("bcc = %s\n", mmsHeader.pBcc->szAddr);
			break;

		case MMS_CODE_CC:

			pAddr = __MmsDecodeEncodedAddress(pFile, totalLength);
			if (pAddr == NULL) {
				MSG_DEBUG("MMS_CODE_CC __MmsDecodeEncodedAddress fail\n");
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
			MSG_DEBUG("cc = %s\n", mmsHeader.pCc->szAddr);
			break;

		case MMS_CODE_CONTENTLOCATION:

			if (__MmsBinaryDecodeText(pFile, mmsHeader.szContentLocation, MMS_LOCATION_LEN + 1, totalLength) < 0) {
				MSG_DEBUG("MMS_CODE_CONTENTLOCATION is invalid\n");
				goto __CATCH;
			}
			MSG_DEBUG("content location = %s\n", mmsHeader.szContentLocation);
			break;

		case MMS_CODE_DATE:

			if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.date, totalLength) == false) {
				MSG_DEBUG("MMS_CODE_DATE is invalid\n");
				goto __CATCH;
			}
			MSG_DEBUG("date = %d\n", mmsHeader.date);
			break;

		case MMS_CODE_DELIVERYREPORT:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("deliveryReport GetOneByte fail\n");
				goto __CATCH;
			}

			fieldValue = MmsGetBinaryType(MmsCodeDeliveryReport, (UINT16)(oneByte & 0x7F));

			if (fieldValue == 0xFFFF) {
				MSG_DEBUG("deliveryReport error\n");
				goto __CATCH;
			}

			mmsHeader.deliveryReport = (MmsReport)fieldValue;

			MSG_DEBUG("delivery report=%s\n", MmsDebugGetMmsReport(mmsHeader.deliveryReport));
			break;

		case MMS_CODE_DELIVERYTIME:

			/* value_length (absolute-token Long-integer | Relative-token Long-integer) */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("invalid MMS_CODE_DELIVERYTIME \n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("delivery time GetOneByte fail\n");
				goto __CATCH;
			}

			//DRM_TEMPLATE - start
			valueLength--;

			if (oneByte == (MmsGetBinaryValue(MmsCodeTimeType, MMS_TIMETYPE_ABSOLUTE)|0x80)) {
				mmsHeader.deliveryTime.type = MMS_TIMETYPE_ABSOLUTE;

				if (valueLength > 0) {
					if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.deliveryTime.time, totalLength) == false)	{
						MSG_DEBUG("invalid MMS_CODE_DELIVERYTIME\n");
						goto __CATCH;
					}
				}
			// DRM_TEMPLATE - end
			} else {
				mmsHeader.deliveryTime.type = MMS_TIMETYPE_RELATIVE;

				if (__MmsBinaryDecodeInteger(pFile, (UINT32*)&mmsHeader.deliveryTime.time, &tmpIntLen, totalLength) == false) {
					MSG_DEBUG("__MmsBinaryDecodeInteger fail...\n");
					goto __CATCH;
				}
			}
			MSG_DEBUG("delivery type=%d, time=%d\n", mmsHeader.deliveryTime.type, mmsHeader.deliveryTime.time);
			break;

		case MMS_CODE_EXPIRYTIME:

			/* value_length(absolute-token Long-integer | Relative-token Long-integer) */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("invalid MMS_CODE_EXPIRYTIME \n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("expiry time GetOneByte fail\n");
				goto __CATCH;
			}

			// DRM_TEMPLATE - start
			valueLength--;

			if (oneByte == (MmsGetBinaryValue(MmsCodeTimeType, MMS_TIMETYPE_ABSOLUTE)|0x80)) {
				mmsHeader.expiryTime.type = MMS_TIMETYPE_ABSOLUTE;

				if (valueLength > 0) {
					if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.expiryTime.time, totalLength) == false) {
						MSG_DEBUG("MMS_CODE_EXPIRYTIME is invalid\n");
						goto __CATCH;
					}
				}
			// DRM_TEMPLATE - end
			} else {
				mmsHeader.expiryTime.type = MMS_TIMETYPE_RELATIVE;

				if (__MmsBinaryDecodeInteger(pFile, (UINT32*)&mmsHeader.expiryTime.time, &tmpIntLen, totalLength) == false) {
					MSG_DEBUG("__MmsBinaryDecodeInteger fail...\n");
					goto __CATCH;
				}
			}
			MSG_DEBUG("expiry = %d\n", mmsHeader.expiryTime.time);
			break;

		case MMS_CODE_MSGCLASS:

			/* Class-value = Class-identifier | Token Text */

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgClass GetOneByte fail\n");
				goto __CATCH;
			}

			if (oneByte > 0x7f) {
				/* Class-identifier */
				mmsHeader.msgClass = (MmsMsgClass)MmsGetBinaryType(MmsCodeMsgClass, (UINT16)(oneByte & 0x7F));
			} else {
				if (__MmsBinaryDecodeText(pFile, szGarbageBuff, MSG_STDSTR_LONG, totalLength) < 0) {
					MSG_DEBUG("1. __MmsBinaryDecodeText fail. (class)\n");
					goto __CATCH;
				}
			}
			MSG_DEBUG("msg class=%s\n", MmsDebugGetMsgClass(mmsHeader.msgClass));
			break;

		case MMS_CODE_MSGSIZE:

			if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.msgSize, totalLength) == false) {
				MSG_DEBUG("MMS_CODE_MSGSIZE is invalid\n");
				goto __CATCH;
			}
			MSG_DEBUG("msg size = %d\n", mmsHeader.msgSize);
			break;

		case MMS_CODE_MSGSTATUS:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail\n");
				goto __CATCH;
			}

			mmsHeader.msgStatus =  (msg_delivery_report_status_t)MmsGetBinaryType(MmsCodeMsgStatus, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("msg status=%s \n", MmsDebugGetMsgStatus(mmsHeader.msgStatus)) ;
			break;

		case MMS_CODE_MSGTYPE:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail\n");
				goto __CATCH;
			}

			mmsHeader.type = (MmsMsgType)MmsGetBinaryType(MmsCodeMsgType, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("msg type=%s\n", MmsDebugGetMsgType(mmsHeader.type));
			break;

		case MMS_CODE_PRIORITY:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail\n");
				goto __CATCH;
			}
			mmsHeader.priority = (MmsPriority)MmsGetBinaryType(MmsCodePriority, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("priority=%d\n", mmsHeader.priority);
			break;

		case MMS_CODE_READREPLY:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail\n");
				goto __CATCH;
			}
			mmsHeader.readReply = (MmsReport)MmsGetBinaryType(MmsCodeReadReply, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("read reply=%s \n", MmsDebugGetMmsReport(mmsHeader.readReply));
			break;

		case MMS_CODE_REPORTALLOWED:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail\n");
				goto __CATCH;
			}
			mmsHeader.reportAllowed =  (MmsReportAllowed)MmsGetBinaryType(MmsCodeReportAllowed, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("mmsHeader.reportAllowed=%d\n", MmsDebugGetMmsReportAllowed(mmsHeader.reportAllowed));
			break;

		case MMS_CODE_SENDERVISIBILLITY:

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail\n");
				goto __CATCH;
			}
			mmsHeader.hideAddress= (MmsSenderVisible)!(MmsGetBinaryType(MmsCodeSenderVisibility, (UINT16)(oneByte &0x7F)));
			MSG_DEBUG("sender visible=%d \n", mmsHeader.hideAddress);
			break;

		case MMS_CODE_TRID:

			if (__MmsBinaryDecodeText(pFile, mmsHeader.szTrID, MMS_TR_ID_LEN + 1, totalLength) < 0) {
				MSG_DEBUG("Transaction ID Too Long \n");
				goto __CATCH;
			}
			MSG_DEBUG("trID = %s\n", mmsHeader.szTrID);
			break;

		case MMS_CODE_VERSION:
			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail\n");
				goto __CATCH;
			}
			mmsHeader.version = oneByte;

			MSG_DEBUG("ver = 0x%x\n", mmsHeader.version);
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
				MSG_DEBUG("msgStatus GetOneByte fail\n");
				goto __CATCH;
			}

			mmsHeader.readStatus =  (msg_read_report_status_t)MmsGetBinaryType(MmsCodeReadStatus, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("read status=%s\n", MmsDebugGetMmsReadStatus(mmsHeader.readStatus));
			break;

		case MMS_CODE_REPLYCHARGING:

			/* Reply-charging-value = Requested | Requested text only | Accepted | Accepted text only */

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail\n");
				goto __CATCH;
			}

			mmsHeader.replyCharge.chargeType =  (MmsReplyChargeType)MmsGetBinaryType(MmsCodeReplyCharging, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("mmsHeader.reply charge=%d\n", mmsHeader.replyCharge.chargeType);
			break;

		case MMS_CODE_REPLYCHARGINGDEADLINE:

			/* Reply-charging-deadline-value = Value-length (Absolute-token Date-value | Relative-token Delta-seconds-value) */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("invalid MMS_CODE_REPLYCHARGINGDEADLINE \n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("msgStatus GetOneByte fail\n");
				goto __CATCH;
			}

			if (oneByte == (MmsGetBinaryValue(MmsCodeTimeType, MMS_TIMETYPE_ABSOLUTE) | 0x80)) {
				mmsHeader.replyCharge.deadLine.type = MMS_TIMETYPE_ABSOLUTE;
			} else {
				mmsHeader.replyCharge.deadLine.type = MMS_TIMETYPE_RELATIVE;
			}

			// DRM_TEMPLATE - start
			valueLength--;

			if (valueLength > 0) {
				if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.replyCharge.deadLine.time, totalLength) == false) {
					MSG_DEBUG("MMS_CODE_REPLYCHARGINGDEADLINE is invalid\n");
					goto __CATCH;
				}
			}
			// DRM_TEMPLATE - end
			break;

		case MMS_CODE_REPLYCHARGINGID:

			/* Reply-charging-ID-value = Text-string */

			if (__MmsBinaryDecodeText(pFile, mmsHeader.replyCharge.szChargeID, MMS_MSG_ID_LEN + 1, totalLength) < 0) {
				MSG_DEBUG("1. __MmsBinaryDecodeText fail. (szReplyChargingID)\n");
				goto __CATCH;
			}
			break;

		case MMS_CODE_REPLYCHARGINGSIZE:

			/* Reply-charging-size-value = Long-integer */

			if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.replyCharge.chargeSize, totalLength) == false) {
				MSG_DEBUG("MMS_CODE_REPLYCHARGINGSIZE is invalid\n");
				goto __CATCH;
			}
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
				MSG_DEBUG("1. invalid MMS_CODE_PREVIOUSLYSENTBY \n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeInteger(pFile, &tmpInteger, &tmpIntLen, totalLength) == false) {
				MSG_DEBUG("2. invalid MMS_CODE_PREVIOUSLYSENTBY \n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeEncodedString(pFile, szGarbageBuff, MSG_STDSTR_LONG, totalLength) == false) {
				MSG_DEBUG("invalid MMS_CODE_RETRIEVETEXT \n");
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
				MSG_DEBUG("1. invalid MMS_CODE_PREVIOUSLYSENTDATE \n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeInteger(pFile, &tmpInteger, &tmpIntLen, totalLength) == false) {
				MSG_DEBUG("2. invalid MS_CODE_PREVIOUSLYSENTDATE \n");
				goto __CATCH;
			}

			if (__MmsDecodeLongInteger(pFile, (UINT32*)&tmpInteger, totalLength) == false) {
				MSG_DEBUG("3. invalid MMS_CODE_PREVIOUSLYSENTDATE \n");
				goto __CATCH;
			}
			break;

		default:

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
			{
				int		remainLength = 0;

				oneByte = 0x00;

				offset = __MmsGetDecodeOffset();
				if (offset >= totalLength)
					goto __RETURN;

				remainLength = totalLength - offset;

				while ((oneByte < 0x80) && (remainLength > 0)) {
					if (__MmsBinaryDecodeCheckAndDecreaseLength(&remainLength, 1) == false) {
						MSG_DEBUG("__MmsBinaryDecodeCheckAndDecreaseLength fail\n");
						goto __CATCH;
					}
					if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
						MSG_DEBUG("responseStatus GetOneByte fail\n");
						goto __CATCH;
					}
				}

				gCurMmsDecodeBuffPos--;
			}

			break;
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

	MSG_DEBUG("success\n");
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

	MSG_DEBUG("failed\n");

	return false;
}

bool MmsBinaryDecodeMsgBody(FILE *pFile, char *szFilePath, int totalLength)
{
	MSG_BEGIN();

	int length = 0;
	int offset = 0;

	if (szFilePath != NULL)
		strncpy(mmsHeader.msgType.szOrgFilePath, szFilePath , strlen(szFilePath));

	mmsHeader.msgType.offset = __MmsGetDecodeOffset() - 1;		// + Content-Type code value

	// read data(2K) from msg file(/User/Msg/Inbox/5) to gpCurMmsDecodeBuff for decoding
	if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos,
									&gMmsDecodeCurOffset, gpMmsDecodeBuf1, gpMmsDecodeBuf2,
									gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("fail to load to buffer \n");
		goto __CATCH;
	}

	// msg's type [ex] related, mixed, single part (jpg, amr and etc)
	length = __MmsBinaryDecodeContentType(pFile, &mmsHeader.msgType, totalLength);
	if (length == -1) {
		MSG_DEBUG("MMS_CODE_CONTENTTYPE is fail\n");
		goto __CATCH;
	}

	mmsHeader.msgType.size	 = length + 1; // + Content-Type code value
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

		MSG_DEBUG("Decode multipart\n");

		offset = __MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		if (__MmsBinaryDecodeMultipart(pFile, szFilePath, &mmsHeader.msgType, &mmsHeader.msgBody, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodeMultipart is fail.\n");
			goto __CATCH;
		}
		break;

	default:

		/* Single part message ---------------------------------------------- */
		if (szFilePath != NULL)
			strcpy(mmsHeader.msgBody.szOrgFilePath, szFilePath);

		offset = __MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		if (__MmsBinaryDecodePartBody(pFile, totalLength - mmsHeader.msgBody.offset, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodePartBody is fail.(Single Part)\n");
			goto __CATCH;
		}

		mmsHeader.msgBody.size = totalLength - mmsHeader.msgBody.offset;
		mmsHeader.msgType.contentSize = totalLength - mmsHeader.msgBody.offset;

		break;
	}

#ifdef __SUPPORT_DRM__
	mmsHeader.drmType = MsgGetDRMType(&mmsHeader.msgType, &mmsHeader.msgBody);
#endif

__RETURN:
	MSG_END();
	return true;

__CATCH:
	return false;
}

static bool __MmsBinaryDecodeParameter(FILE *pFile, MsgType *pMsgType, int valueLength, int totalLength)
{
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

	MSG_BEGIN();

	while (valueLength > 0) {
		if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
			MSG_DEBUG("paramCode _MmsBinaryDecodeGetOneByte fail\n");
			goto __CATCH;
		}

		paramCode = oneByte;
		valueLength--;

		switch (paramCode) {
		case 0x81: // charset

			if (__MmsBinaryDecodeCharset(pFile, (UINT32*)&(pMsgType->param.charset), &charSetLen, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeContentType : __MmsBinaryDecodeCharset fail.\n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, charSetLen) == false)
				goto __RETURN;

			break;

		case 0x85: //name = Text-string
		case 0x97: //name = Text-value  = No-value | Token-text | Quoted-string
			memset(pMsgType->param.szName, 0, sizeof(pMsgType->param.szName));
			length = __MmsDecodeGetFilename(pFile,  pMsgType->param.szName,
											 MSG_FILENAME_LEN_MAX -5,		// MSG_LOCALE_FILENAME_LEN_MAX + 1, :  change @ 110(Ui code have to change for this instead of DM)
											 totalLength);
			if (length < 0) {
				MSG_DEBUG("MmsBinaryDecodeContentType : __MmsDecodeGetFilename fail. (name parameter)\n");
				goto __CATCH;
			}

			if (__MsgCheckFileNameHasInvalidChar(pMsgType->param.szName)) {
				__MsgReplaceInvalidFileNameChar(pMsgType->param.szName, '_');
			}

			if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, length) == false)
				goto __RETURN;

			break;

		case 0x86: //filename = Text-string
		case 0x98: //filename = Text-value  = No-value | Token-text | Quoted-string
			memset(pMsgType->param.szFileName, 0, sizeof(pMsgType->param.szFileName));
			length = __MmsDecodeGetFilename(pFile, pMsgType->param.szFileName, MSG_FILENAME_LEN_MAX -5, totalLength);
			if (length < 0) {
				MSG_DEBUG("MmsBinaryDecodeContentType : __MmsDecodeGetFilename fail. (filename parameter)\n");
				goto __CATCH;
			}

			if (__MsgCheckFileNameHasInvalidChar(pMsgType->param.szFileName)) {
				__MsgReplaceInvalidFileNameChar(pMsgType->param.szFileName, '_');
			}

			if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, length) == false)
				goto __RETURN;

			break;

		case 0x89: //type = Constrained-encoding = Extension-Media | Short-integer

			if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("type _MmsBinaryDecodeGetOneByte fail\n");
				goto __CATCH;
			}

			if (oneByte > 0x7f) {
				pMsgType->param.type = MmsGetBinaryType(MmsCodeContentType,
														  (UINT16)(oneByte & 0x7f));
				if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, 1) == false)
					goto __RETURN;
			} else {
				gCurMmsDecodeBuffPos--;

				textLength = 0;
				szTypeString = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
				pMsgType->param.type = MmsGetTextType(MmsCodeContentType, szTypeString);

				if (szTypeString) {
					free(szTypeString);
					szTypeString = NULL;
				}

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, textLength) == false)
					goto __RETURN;
			}

			break;

		case 0x8A: //start encoding version 1.2
		case 0x99: //start encoding version 1.4

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

		case 0x8B: //startInfo encoding version 1.2
		case 0x9A: //startInfo encoding version 1.4

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
				MSG_DEBUG("Unsupported parameter\n");

				// In case of the last byte of Parameter field, it should be returned without decreasing the gCurMmsDecodeBuffPos value.
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
							pMsgType->param.szApplicationID = (char*) malloc(textLength + 1);
							memset(pMsgType->param.szApplicationID,  0,  textLength + 1);
							strncpy(pMsgType->param.szApplicationID, szTypeValue, textLength);
							MSG_DEBUG("Application-ID:%s",pMsgType->param.szApplicationID);
						} else if (strcasecmp(szTypeString,"Reply-To-Application-ID") == 0) {
							pMsgType->param.szReplyToApplicationID= (char*) malloc(textLength + 1);
							memset(pMsgType->param.szReplyToApplicationID, 0, textLength + 1);
							strncpy(pMsgType->param.szReplyToApplicationID, szTypeValue, textLength);
							MSG_DEBUG("ReplyToApplication-ID:%s",pMsgType->param.szReplyToApplicationID);
#endif
						}
						free(szTypeValue);
						szTypeValue = NULL;

						MSG_DEBUG("Unsupported parameter(%s)\n", szTypeValue);
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

	return true;

__CATCH:

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

	MSG_DEBUG("decoding content type..\n");

	length = __MmsDecodeValueLength(pFile, (UINT32*)&valueLength, totalLength);
	if (length <= 0) {
		/*
		 * Constrained-media or Single part message
		 * Constrained-media = Constrained-encoding = Extension-Media | Short-integer
		 * Extension-media   = *TEXT End-of-string
		 * Short-integer     = OCTET(1xxx xxxx)
		 */

		if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
			MSG_DEBUG("Constrained-media _MmsBinaryDecodeGetOneByte fail\n");
			goto __CATCH;
		}

		if (oneByte > 0x7F) {
			/* Short-integer */
			pMsgType->type = MmsGetBinaryType(MmsCodeContentType, (UINT16)(oneByte & 0x7F));
			length = 1;
		} else {
			char *pszTemp = NULL;

			/* Extension-Media */
			gCurMmsDecodeBuffPos--;

			textLength = 0;
			szTypeString = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);

			if (szTypeString && (strchr(szTypeString, ';')) != NULL) {
				pszTemp = __MsgGetStringUntilDelimiter(szTypeString, ';');
				if (pszTemp) {
					free(szTypeString);
					szTypeString = pszTemp;
				}
			}

			pMsgType->type = MmsGetTextType(MmsCodeContentType, szTypeString);

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
			MSG_DEBUG("Well-known-media _MmsBinaryDecodeGetOneByte fail\n");
			goto __CATCH;
		}

		if (oneByte > 0x7F) {
			/* Well-known-media */
			pMsgType->type = MmsGetBinaryType(MmsCodeContentType, (UINT16)(oneByte & 0x7F));
			valueLength--;
		} else {
			/* Extension-Media */
			gCurMmsDecodeBuffPos--;

			textLength = 0;
			szTypeString = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
			pMsgType->type = MmsGetTextType(MmsCodeContentType, szTypeString);
			valueLength -= textLength;

			if (szTypeString) {
				free(szTypeString);
				szTypeString = NULL;
			}
		}

		MSG_DEBUG("content type=%s\n", MmsDebugGetMimeType((MimeType)pMsgType->type));


		if (__MmsBinaryDecodeParameter(pFile, pMsgType, valueLength, totalLength) == false) {
			MSG_DEBUG("Content-Type parameter fail\n");
			goto __CATCH;
		}
	}

	return length;

__CATCH:
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
			MSG_DEBUG("field code GetOneByte fail\n");
			goto __CATCH;
		}

		if (0x80 <= oneByte && oneByte <= 0xC7) {
			/* Well-known-header = Well-known-field-name Wap-value (0x00 ~ 0x47) */

			if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, 1) == false)
				goto __RETURN;

			fieldCode = oneByte & 0x7f;

			switch (fieldCode) {
			case 0x0E:	//Content-Location
			case 0x04:	//Content-Location

				pLatinBuff = (char *)malloc(MMS_CONTENT_ID_LEN + 1);
				if (pLatinBuff == NULL)
					goto __CATCH;

				length = __MmsBinaryDecodeText(pFile, pLatinBuff, MMS_CONTENT_ID_LEN + 1, totalLength);
				if (length == -1) {
					MSG_DEBUG("MmsBinaryDecodePartHeader : __MmsBinaryDecodeQuotedString fail.\n");
					goto __CATCH;
				}

				szSrc = MsgChangeHexString(pLatinBuff);

				if (szSrc) {
					strcpy(pLatinBuff, szSrc);
					free(szSrc);
					szSrc = NULL;
				}

				textLength = strlen(pLatinBuff);

				if (__MsgLatin2UTF ((unsigned char*)pMsgType->szContentLocation, MMS_CONTENT_ID_LEN + 1, (unsigned char*)pLatinBuff, textLength) < 0) {
					MSG_DEBUG("MsgLatin2UTF fail \n");
					goto __CATCH;
				}

				free(pLatinBuff);
				pLatinBuff = NULL;

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, length) == false)
					goto __RETURN;

				break;

			case 0x40:	// Content-ID

				pLatinBuff = (char *)malloc(MMS_CONTENT_ID_LEN + 1);
				if (pLatinBuff == NULL)
					goto __CATCH;

				length = __MmsBinaryDecodeQuotedString(pFile, pLatinBuff, MMS_CONTENT_ID_LEN + 1, totalLength);

				if (length == -1) {
					MSG_DEBUG("MmsBinaryDecodePartHeader : Content-ID __MmsBinaryDecodeQuotedString fail.\n");
					goto __CATCH;
				}

				szSrc = MsgChangeHexString(pLatinBuff);

				if (szSrc) {
					strcpy(pLatinBuff, szSrc);
					free(szSrc);
					szSrc = NULL;
				}

				textLength = strlen(pLatinBuff);
				if (__MsgLatin2UTF ((unsigned char*)pMsgType->szContentID, MMS_CONTENT_ID_LEN + 1, (unsigned char*)pLatinBuff, textLength) < 0) {
					MSG_DEBUG("MsgLatin2UTF fail \n");
					goto __CATCH;
				}
				free(pLatinBuff);
				pLatinBuff = NULL;

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, length) == false)
					goto __RETURN;

				break;

			case 0x2E:	// Content-Disposition
			case 0x45:	// Content-Disposition

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
					MSG_DEBUG("Disposition value GetOneByte fail\n");
					goto __CATCH;
				}

				if (length > 0)
					valueLength--;

				if (oneByte >= 0x80) {
					pMsgType->disposition = MmsGetBinaryType(MmsCodeMsgDisposition, (UINT16)(oneByte & 0x7F));

					if (pMsgType->disposition == INVALID_HOBJ) {
						MSG_DEBUG("MmsBinaryDecodePartHeader : Content-Disposition MmsGetBinaryType fail.\n");
						pMsgType->disposition = MSG_DISPOSITION_ATTACHMENT;		// default
					}

					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, 1) == false)
						goto __RETURN;

					if (__MmsBinaryDecodeParameter(pFile, pMsgType, valueLength, totalLength) == false) {
						MSG_DEBUG("Disposition parameter fail\n");
						goto __CATCH;
					}

					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, valueLength) == false)
						goto __RETURN;
				} else {

					gCurMmsDecodeBuffPos--;
					valueLength++;

					pLatinBuff = (char *)malloc(MSG_FILENAME_LEN_MAX);
					memset(pLatinBuff, 0, MSG_FILENAME_LEN_MAX);

					textLength = __MmsBinaryDecodeText(pFile, pLatinBuff, MSG_FILENAME_LEN_MAX-1, totalLength);


					if (textLength < 0) {
						MSG_DEBUG("MmsBinaryDecodePartHeader : Content-Disposition decodingfail. \n");
						goto __CATCH;
					}
					free(pLatinBuff);
					pLatinBuff = NULL;

					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, textLength) == false)
						goto __RETURN;

					valueLength -= textLength;

					if (__MmsBinaryDecodeParameter(pFile, pMsgType, valueLength, totalLength) == false)
					{
						MSG_DEBUG("Disposition parameter fail\n");
						goto __CATCH;
					}

					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, valueLength) == false)
						goto __RETURN;

				}

				break;

			case 0x0B:	//Content-Encoding

				if (__MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
					MSG_DEBUG("Disposition value GetOneByte fail\n");
					goto __CATCH;
				}

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, 1) == false)
					goto __RETURN;

				break;

			case 0x0C:	//Content-Language

				if (__MmsBinaryDecodeInteger(pFile, (UINT32*)&tmpInt, &tmpIntLen, totalLength) == true) {
					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, tmpIntLen) == false)
						goto __RETURN;
				} else {
					char* cTemp = NULL;

					cTemp = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);

					if (cTemp == NULL) {
						MSG_DEBUG("MmsBinaryDecodePartHeader : __MmsBinaryDecodeText2 fail...\n");
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

			case 0x0D:	//Content-Length

				if (__MmsBinaryDecodeInteger(pFile, (UINT32*)&tmpInt, &tmpIntLen, totalLength) == false) {
					MSG_DEBUG("MmsBinaryDecodePartHeader : __MmsBinaryDecodeInteger fail...\n");
					goto __CATCH;
				}

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, tmpIntLen) == false)
					goto __RETURN;

				break;

			case 0x30:	//X-Wap-Content-URI skip this value

				MSG_DEBUG("MmsBinaryDecodePartHeader : X-Wap-Content-URI header.\n");
				pLatinBuff = (char *)malloc(MMS_TEXT_LEN);
				if (pLatinBuff == NULL)
					goto __CATCH;

				length = __MmsBinaryDecodeText(pFile, pLatinBuff, MMS_TEXT_LEN,	totalLength);

				if (length == -1) {
					MSG_DEBUG("MmsBinaryDecodePartHeader : __MmsBinaryDecodeQuotedString fail.\n");
					goto __CATCH;
				}

				MSG_DEBUG("MmsBinaryDecodePartHeader : X-Wap-Content-URI header decoded. Value length %d\n", length);
				free(pLatinBuff);
				pLatinBuff = NULL;

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, length) == false)
					goto __RETURN;

				MSG_DEBUG("MmsBinaryDecodePartHeader : X-Wap-Content-URI header skipped.\n");

				break;

			case 0x01:	// Accept-charset
				//if (NvGetInt(NV_SI_ADM_GCF_STATE) == 1)
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

					MSG_DEBUG("MmsBinaryDecodePartHeader : Accept-charset. \n");

					length = __MmsDecodeValueLength(pFile, &valueLength, totalLength);
					if (length > 0) {
						if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, length) == false)
							goto __RETURN;

					}

					if (__MmsBinaryDecodeInteger(pFile, (UINT32*)&charset, &charSetLen, totalLength) == false) {
						// We only support the well-known-charset format
						MSG_DEBUG("MmsBinaryDecodePartHeader : __MmsBinaryDecodeInteger fail...\n");
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

				MSG_DEBUG("MmsBinaryDecodePartHeader : unknown Value = 0x%x\n", oneByte);

				length = __MmsDecodeValueLength(pFile, &valueLength, totalLength);
				if (length <= 0) {
					MSG_DEBUG("MmsBinaryDecodePartHeader : 1. invalid MMS_CODE_PREVIOUSLYSENTDATE \n");
					goto __CATCH;
				}

				if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, length) == false)
					goto __RETURN;

				szTemp = (char *)malloc(valueLength);
				if (szTemp == NULL)
					goto __CATCH;

				if (__MmsBinaryDecodeGetBytes(pFile, szTemp, valueLength, totalLength) == false) {
					MSG_DEBUG("MmsBinaryDecodePartHeader : default _MmsBinaryDecodeGetBytes() fail\n");
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

			MSG_DEBUG(" Application-header = Token-text Application-specific-value \n");

			gCurMmsDecodeBuffPos--;

			/* Token-text */

			textLength = 0;
			pCode = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
			if (pCode == NULL) {
				MSG_DEBUG("pCode is null\n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, textLength) == false)
				goto __RETURN;

			MSG_DEBUG(" Token-text (%s) \n", pCode);


			/* Application-specific-value */

			textLength = 0;
			pValue = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
			if (pValue == NULL) {
				MSG_DEBUG("pValue is null\n");
				goto __CATCH;
			}

			MSG_DEBUG(" Application-specific-value (%s) \n", pValue);


			pParam = strchr(pValue, MSG_CH_ADDR_DELIMETER);
			if (pParam) {
				ch = *pParam;
				*pParam = '\0';
			}

			switch (MmsGetTextType(MmsCodeMsgBodyHeaderCode, pCode)) {
			case MMS_BODYHDR_TRANSFERENCODING:		// Content-Transfer-Encoding
				pMsgType->encoding = MmsGetTextType(MmsCodeContentTransferEncoding, pValue);
				break;

			case MMS_BODYHDR_CONTENTID:				// Content-ID

				pLatinBuff = (char *)malloc(MMS_CONTENT_ID_LEN + 1);
				if (pLatinBuff == NULL)
				{
					goto __CATCH;
				}

				__MsgMIMERemoveQuote (pValue);
				strncpy(pLatinBuff, pValue, MMS_MSG_ID_LEN);

				length = strlen(pLatinBuff);
				if (__MsgLatin2UTF ((unsigned char*)pMsgType->szContentID, MMS_CONTENT_ID_LEN + 1, (unsigned char*)pLatinBuff, length) < 0)
				{
					MSG_DEBUG("MsgLatin2UTF fail \n");
					goto __CATCH;
				}

				free(pLatinBuff);
				pLatinBuff = NULL;
				break;

			case MMS_BODYHDR_CONTENTLOCATION:		// Content-Location

				pLatinBuff = (char *)malloc(MMS_CONTENT_ID_LEN + 1);
				if (pLatinBuff == NULL)
					goto __CATCH;

				strncpy(pLatinBuff, pValue, MMS_MSG_ID_LEN);

				length = strlen(pLatinBuff);
				if (__MsgLatin2UTF ((unsigned char*)pMsgType->szContentLocation, MMS_CONTENT_ID_LEN + 1, (unsigned char*)pLatinBuff, length) < 0) {
					MSG_DEBUG("MsgLatin2UTF fail \n");
					goto __CATCH;
				}

				free(pLatinBuff);
				pLatinBuff = NULL;
				break;

			case MMS_BODYHDR_DISPOSITION:			// Content-Disposition
				pMsgType->disposition = MmsGetTextType(MmsCodeMsgDisposition, pValue);
				break;

			case MMS_BODYHDR_X_OMA_DRM_SEPARATE_DELIVERY:	// DRM RO WAITING
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
	}	//while

__RETURN:

	if (pLatinBuff) {
		free(pLatinBuff);
		pLatinBuff = NULL;
	}

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

	MSG_DEBUG("Number of Entries = %d\n", *npEntries);

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
		MSG_DEBUG("fail to seek file pointer \n");
		goto __CATCH;
	}

	__MmsCleanDecodeBuff();

	gMmsDecodeCurOffset = offset;

	if (offset >= totalLength)
		goto __RETURN;

	if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
								   gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("fail to load to buffer \n");
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
		MSG_DEBUG("fail to seek file pointer \n");
		goto __CATCH;
	}

	__MmsCleanDecodeBuff();

	gMmsDecodeCurOffset = offset;

	if (offset == totalLength)
		goto __RETURN;

	if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
									gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("fail to load to buffer \n");
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
	MsgMultipart *pPreMultipart	= NULL;
	int offset = 0;
	int	index = 0;

	MsgPresentationFactor factor = MSG_PRESENTATION_NONE;
	MsgPresentaionInfo presentationInfo;

	MSG_DEBUG("total length=%d\n", totalLength);

	presentationInfo.factor = MSG_PRESENTATION_NONE;
	presentationInfo.pCurPresentation = NULL;
	presentationInfo.pPrevPart = NULL;

	if (__MmsBinaryDecodeEntries(pFile, &nEntries, totalLength) == false) {
		MSG_DEBUG("MmsBinaryDecodeEntries is fail.\n");
		goto __CATCH;
	}

	if (pMsgBody->body.pMultipart != NULL)
		pLastMultipart = pMsgBody->body.pMultipart;

	while (nEntries) {
		MSG_DEBUG("decoding %dth multipart\n", index);

		offset = __MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		if ((pMultipart = __MsgAllocMultipart()) == NULL) {
			MSG_DEBUG("MsgAllocMultipart Fail \n");
			goto __CATCH;
		}

		if (__MmsBinaryDecodeEachPart(pFile, szFilePath, &(pMultipart->type), pMultipart->pBody, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodeEachPart is fail.(nEntries = %d)\n", nEntries);
			goto __CATCH;
		}

		if (pMsgType->param.type == MIME_APPLICATION_SMIL) {
			factor = __MsgIsPresentationEx(&(pMultipart->type), pMsgType->param.szStart, (MimeType)pMsgType->param.type);
		} else {
			factor = MSG_PRESENTATION_NONE;
		}
		// priority 1 : content type match, 2: content location, 3: type
		if (presentationInfo.factor < factor) {
			// Presentation part
			presentationInfo.factor = factor;
			presentationInfo.pPrevPart = pPreMultipart;
			presentationInfo.pCurPresentation = pMultipart;
		}

		if (pLastMultipart == NULL) {
			/* first multipart */
			pMsgBody->body.pMultipart = pMultipart;
			pLastMultipart = pMultipart;
			pPreMultipart = NULL;
		} else {
			pLastMultipart->pNext = pMultipart;
			pLastMultipart = pMultipart;
			pPreMultipart = pMultipart;
		}

		pMsgType->contentSize += pMultipart->pBody->size;

		nEntries--;

		__MmsDebugPrintMulitpartEntry(pMultipart, index++);

	}

	pMsgBody->size = totalLength - pMsgBody->offset;

#ifdef	__SUPPORT_DRM__
	if (MmsDrm2GetConvertState() != MMS_DRM2_CONVERT_REQUIRED)
#endif
		__MsgConfirmPresentationPart(pMsgType, pMsgBody, &presentationInfo);

	if (__MsgResolveNestedMultipart(pMsgType, pMsgBody) == false) {
		MSG_DEBUG("MmsBinaryDecodeMultipart : MsgResolveNestedMultipart failed \n");
		goto __CATCH;
	}

__RETURN:
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

	MSG_DEBUG("MmsBinaryDecodeEachPart: total length=%d\n", totalLength);

	/* header length */

	if (__MmsBinaryDecodeUintvar(pFile, &headerLength, totalLength) <= 0) {
		MSG_DEBUG("MmsBinaryDecodeEachPart: Get header length fail \n");
		goto __CATCH;
	}

	offset = __MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;


	/* body length */

	if (__MmsBinaryDecodeUintvar(pFile, &bodyLength, totalLength) <= 0) {
		MSG_DEBUG("MmsBinaryDecodeEachPart: Get body length fail\n");
		goto __CATCH;
	}


	offset = __MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;


	/* Content Type */
	if (szFilePath != NULL)
		strncpy(pMsgType->szOrgFilePath, szFilePath, strlen(szFilePath));

	pMsgType->offset = __MmsGetDecodeOffset();
	pMsgType->size = headerLength;
	pMsgType->contentSize = bodyLength;

	if (pMsgType->offset > totalLength)
		goto __RETURN;

	length = __MmsBinaryDecodeContentType(pFile, pMsgType, totalLength);
	if (length <= 0) {
		MSG_DEBUG("MmsBinaryDecodeEachPart: Decode contentType Fail \n");
		goto __CATCH;
	}

	offset = __MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;


	/* Part Header */

	if (__MmsBinaryDecodePartHeader(pFile, pMsgType, headerLength - length, totalLength) == false) {
		MSG_DEBUG("MmsBinaryDecodeEachPart: Decode contentHeader Fail \n");
		goto __CATCH;
	}

	offset = __MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;

	/* Part Body */

	if (szFilePath != NULL)
		strncpy(pMsgBody->szOrgFilePath, szFilePath, strlen(szFilePath));

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

		MSG_DEBUG("MmsBinaryDecodeEachPart: Decode multipart\n");

		if (__MmsBinaryDecodeMultipart(pFile, szFilePath, pMsgType, pMsgBody, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodeEachPart: MmsBinaryDecodeMultipart is fail.\n");
			goto __CATCH;
		}

		offset = __MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		break;


#ifdef __SUPPORT_DRM__

	case MIME_APPLICATION_VND_OMA_DRM_MESSAGE: /* Contains forwardLock OR combined-delivery media part */
		MSG_DEBUG("MmsBinaryDecodeEachPart: MIME_APPLICATION_VND_OMA_DRM_MESSAGE Part \n");

		if (MmsDrm2GetConvertState() != MMS_DRM2_CONVERT_NOT_FIXED && MmsDrm2GetConvertState() != MMS_DRM2_CONVERT_REQUIRED) {

			if (__MmsBinaryDecodeDRMContent(pFile, szFilePath, pMsgType, pMsgBody, bodyLength, totalLength) == false)
				goto __CATCH;
		} else {
			MmsDrm2SetConvertState(MMS_DRM2_CONVERT_REQUIRED);

			bSuccess = __MmsBinaryDecodePartBody(pFile, bodyLength, totalLength);
			if (bSuccess == false) {
				MSG_DEBUG("MmsBinaryDecodeEachPart: Decode contentBody Fail \n");
				goto __CATCH;
			}
		}
		offset = __MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		break;

	case MIME_APPLICATION_VND_OMA_DRM_CONTENT: /* Contains seperate-delivery media part (DCF) */

		MSG_DEBUG("MmsBinaryDecodeEachPart: MIME_APPLICATION_VND_OMA_DRM_CONTENT Part \n");

		if (__MmsBinaryDecodeDRMContent(pFile, szFilePath, pMsgType, pMsgBody, bodyLength, totalLength) == false)
			goto __CATCH;

		offset = __MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		break;
#endif

	default:
		MSG_DEBUG("MmsBinaryDecodeEachPart: Other normal Part \n");

		bSuccess = __MmsBinaryDecodePartBody(pFile, bodyLength, totalLength);
		if (bSuccess == false) {
			MSG_DEBUG("MmsBinaryDecodeEachPart: Decode contentBody Fail \n");
			goto __CATCH;
		}

		offset = __MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		break;
	}

	return true;

__RETURN:

	return true;

__CATCH:

	return false;
}

#ifdef __SUPPORT_DRM__
static bool __MmsBinaryDecodeDRMContent(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, unsigned int bodyLength, int totalLength)
{
	int offset = 0;
	char szTempFilePath[MSG_FILEPATH_LEN_MAX] = MSG_DATA_PATH"drm.dcf";
	char *pRawData = NULL;
	bool isFileCreated = false;

	MSG_DEBUG("bodyLength: %d\n", bodyLength);

	offset = __MmsGetDecodeOffset();

	if (offset >= totalLength)
		goto __RETURN;

	if (szFilePath != NULL)
		strncpy(pMsgBody->szOrgFilePath, szFilePath, strlen(szFilePath));
	if (szFilePath != NULL)
		strncpy(pMsgType->szOrgFilePath, szFilePath, strlen(szFilePath));

	pRawData = (char *)malloc(bodyLength);
	if (pRawData == NULL) {
		MSG_DEBUG("pRawData alloc FAIL \n");
		goto __CATCH;
	}

	if (MsgFseek(pFile, offset, SEEK_SET) < 0) {
		MSG_DEBUG("MsgFseek() returns -1 \n");
		goto __CATCH;
	}
	if (MsgReadFile(pRawData, sizeof(char), bodyLength, pFile) != (size_t)bodyLength) {
		MSG_DEBUG("FmReadFile() returns false \n");
		goto __CATCH;
	}
	if (MsgOpenCreateAndOverwriteFile(szTempFilePath, pRawData, bodyLength) == false) {
		MSG_DEBUG("MsgOpenCreateAndOverwriteFile() returns false \n");
		goto __CATCH;
	}

	isFileCreated = true;
	MSG_DEBUG("MmsDrm2GetConvertState() [%d]", MmsDrm2GetConvertState());

	if (pMsgType->type == MIME_APPLICATION_VND_OMA_DRM_MESSAGE && (MmsDrm2GetConvertState() != MMS_DRM2_CONVERT_FINISH)) {
		MmsDrm2SetConvertState(MMS_DRM2_CONVERT_REQUIRED);
	} else {
		if (MsgDRM2GetDRMInfo(szTempFilePath, pMsgType) == false) {
			MSG_DEBUG("MsgDRM2GetDRMInfo() returns false \n");
			goto __CATCH;
		}
	}

	if(remove(szTempFilePath) != 0)
		MSG_DEBUG("remove fail");
	isFileCreated = false;

	if (__MmsBinaryDecodeMovePointer(pFile, offset + bodyLength, totalLength) == false)
		goto __CATCH;

__RETURN:

	if (pRawData) {
		free(pRawData);
		pRawData = NULL;
	}

	return true;

__CATCH:
	if (isFileCreated)
		if(remove(szTempFilePath) != 0)
			MSG_DEBUG("remove fail");

	if (pRawData) {
		free(pRawData);
		pRawData = NULL;
	}

	return false;
}

static int __MmsDrm2BinaryEncodeUintvarLen(UINT32 integer)
{
	UINT32 length	= 0;

	/* Find encoded unitvar length */
	if (integer  <= MMS_UINTVAR_LENGTH_1) {
		length = 1;
	} else {
		if (integer <= MMS_UINTVAR_LENGTH_2) {
			length = 2;
		} else {
			if (integer <= MMS_UINTVAR_LENGTH_3) {
				length = 3;
			} else {
				length = 4;
			}
		}
	}

	return length;
}

static bool __MmsDrm2BinaryEncodeUintvar(UINT32 integer, int length, char *pszOutput)
{
	const char ZERO = 0x00;
	int i = 2;
	char szReverse[MSG_STDSTR_LONG] = {0, };

	union {
		UINT32 integer;
		char bytes[4];
	} source;
	source.integer = integer;
	memset(szReverse, 0, MSG_STDSTR_LONG);

	/* Seperate integer to 4 1 byte integer */
	szReverse[3] = source.bytes[3] & 0x0f;
	szReverse[0] = source.bytes[0];
	szReverse[0] = szReverse[0] & 0x7f;

	while (length >= i) {// initially, i = 2
		/* Move integer bit to proper position */
		source.integer = source.integer << 1;
		source.integer = source.integer >> 8;
		source.bytes[3] = ZERO;

		/* Retrive 1 encode uintvar */
		szReverse[i-1] = source.bytes[0];
		szReverse[i-1] = szReverse[i-1] | 0x80;
		i++;
	}

	for (i=0; i < length; i++)
		pszOutput[i] = szReverse[length - i - 1];

	return true;
}

static int __MmsDrm2GetEntriesValueLength(FILE *pFile, int orgOffset)
{
	char szEntries[5] = {0, };
	UINT8 oneByte	= 0;
	int j = 0;			//j is the length of nEntries value

	if (MsgReadFile(szEntries, sizeof(char), 4, pFile) != (size_t)4) {
		MSG_DEBUG("__MmsDrm2GetEntriesValueLength: FmReadFile() returns false \n");
		return false;
	}

	while (true) {
		oneByte = szEntries[j++];

		if (oneByte <= 0x7f)
			break;
	}

	//move file pointer to point nEntries
	if (MsgFseek(pFile, orgOffset, SEEK_SET) < 0) {
		MSG_DEBUG("__MmsDrm2GetEntriesValueLength: fail to seek file pointer\n");
		return false;
	}

	return j;
}

static bool __MmsDrm2WriteDataToConvertedFile(FILE *pSrcFile, FILE *pDestinationFile, char *pszMmsLoadTempBuf, int length, int bufLen)
{
	int loadLen = 0, totalLoadLen = 0, nRead = 0;

	for (int i=0; i<(length/bufLen)+1; i++) {
		loadLen = (length-totalLoadLen < bufLen) ? length-totalLoadLen : bufLen;

		memset(pszMmsLoadTempBuf, 0, MMS_DRM2_CONVERT_BUFFER_MAX + 1);

		if (MsgReadFile(pszMmsLoadTempBuf, sizeof(char), loadLen, pSrcFile) != (size_t)loadLen) {
			MSG_DEBUG("__MmsDrm2WriteDataToConvertedFile: FmReadFile() returns false \n");
			return false;
		}

		if (MsgWriteFile(pszMmsLoadTempBuf, sizeof(char), loadLen, pDestinationFile) != (size_t)loadLen) {
			MSG_DEBUG("__MmsDrm2WriteDataToConvertedFile: File Writing is failed.\n");
			return false;
		}

		totalLoadLen += nRead;
	}

	return true;
}

/*************************************************************************
 * description : make new message file converting CD & FL part of original message file to SD type
 * argument : void
 * return value
    - bool :  result of converting
**************************************************************************/
bool MmsDrm2ConvertMsgBody(char *szOriginFilePath)
{
	FILE *pFile = NULL;
	FILE *hConvertedFile = NULL;
	FILE *hTempFile = NULL;
	FILE *hFile = NULL;
	MsgMultipart *pMultipart = NULL;
	char szTempFilePath[MSG_FILEPATH_LEN_MAX] = MSG_DATA_PATH"Drm_Convert";
	char szTempFile[MSG_FILEPATH_LEN_MAX] = MSG_DATA_PATH"temp.dm";
	char *pszMmsLoadTempBuf = NULL;
	char *pszOrgData = NULL;
	int length = 0;
	int bufLen = MMS_DRM2_CONVERT_BUFFER_MAX;
	int curOffset = 0;

	MSG_DEBUG("start convert~~~~~~\n");

	pFile = MsgOpenFile(szOriginFilePath, "rb");
	if (pFile == NULL) {
		MSG_DEBUG("Open decode temporary file fail\n");
		goto __CATCH;
	}

	hConvertedFile = MsgOpenFile(MMS_DECODE_DRM_CONVERTED_TEMP_FILE, "wb+");
	if (hConvertedFile == NULL) {
		MSG_DEBUG("Open decode temporary file fail\n");
		goto __CATCH;
	}

	pszMmsLoadTempBuf = (char*)malloc(MMS_DRM2_CONVERT_BUFFER_MAX + 1);
	if (pszMmsLoadTempBuf == NULL) {
		MSG_DEBUG("malloc for pszMmsLoadTempBuf failed\n");
		goto __CATCH;
	}
	memset(pszMmsLoadTempBuf, 0, MMS_DRM2_CONVERT_BUFFER_MAX + 1);

	// MMS Header  copy
	length = mmsHeader.msgBody.offset;
	if (__MmsDrm2WriteDataToConvertedFile(pFile, hConvertedFile, pszMmsLoadTempBuf, length, bufLen) == false) {
		MSG_DEBUG("Write header data fail\n");
		goto __CATCH;
	}

	curOffset += length;	//change offset

	// MMS Body copy
	if (MsgIsMultipart(mmsHeader.msgType.type) == true)
	{
		// nEntries copy
		length = __MmsDrm2GetEntriesValueLength(pFile, curOffset);	// getting nEntries value's length

		if (__MmsDrm2WriteDataToConvertedFile(pFile, hConvertedFile, pszMmsLoadTempBuf, length, bufLen) == false) {
			MSG_DEBUG("Write nEntries fail\n");
			goto __CATCH;
		}

		curOffset += length;	//change offset

		// each Multipart entry copy
		pMultipart = mmsHeader.msgBody.body.pMultipart;

		while (pMultipart) {
			if (pMultipart->type.type == MIME_APPLICATION_VND_OMA_DRM_MESSAGE) {
				int orgDataLen = pMultipart->pBody->size;
				int nSize = 0;

				MSG_DEBUG("Write MIME_APPLICATION_VND_OMA_DRM_MESSAGE multipart data(orgDataLen = %d).\n", orgDataLen);

				pszOrgData = (char *)malloc(orgDataLen + 1);
				if (pszOrgData == NULL) {
					MSG_DEBUG("pszOrgData is NULL \n");
					goto __CATCH;
				}
				memset(pszOrgData, 0, orgDataLen + 1);

				// move file pointer to data
				if (MsgFseek(pFile, pMultipart->pBody->offset, SEEK_SET) < 0) {
					MSG_DEBUG("fail to seek file pointer 1\n");
					goto __CATCH;
				}

				if (MsgReadFile(pszOrgData, sizeof(char), orgDataLen, pFile) != (size_t)orgDataLen) {
					MSG_DEBUG("FmReadFile() returns false for orgData\n");
					goto __CATCH;
				}

				if((hFile = MsgOpenFile(szTempFile, "wb+")) == NULL) {
					MSG_DEBUG("file open failed [%s]", szTempFile);
					goto __CATCH;
				}

				if (MsgWriteFile(pszOrgData, sizeof(char), orgDataLen, hFile) != (size_t)orgDataLen) {
					MSG_DEBUG("File write error");
					goto __CATCH;
				}

				if (pszOrgData) {
					free(pszOrgData);
					pszOrgData = NULL;
				}

				MsgFflush(hFile);
				MsgCloseFile(hFile);

				hFile = NULL;

				// --> invoking drm agent api, converting data part start
				MSG_DEBUG("start data part convert by callling drm agent api\n");

				int ret = 0;
				ret = MsgDrmConvertDmtoDcfType(szTempFile, szTempFilePath);
				MSG_DEBUG("MsgDrmConvertDmtoDcfType returned %s", ret ? "true": "false");

				if (MsgGetFileSize(szTempFilePath, &nSize) == false) {
					MSG_DEBUG("MsgGetFileSize error");
					goto __CATCH;
				}
				MSG_DEBUG("end data part convert(converted data len = %d)\n", nSize);

				// move file pointer to the head of multipart
				if (MsgFseek(pFile, curOffset, SEEK_SET) < 0) {
					MSG_DEBUG("fail to seek file pointer 2\n");
					goto __CATCH;
				}

				// read headerLen, dataLen
				length = pMultipart->type.offset - curOffset;
				memset(pszMmsLoadTempBuf, 0, MMS_DRM2_CONVERT_BUFFER_MAX + 1);
				if (MsgReadFile(pszMmsLoadTempBuf, sizeof(char), length, pFile) != (size_t)length) {
					MSG_DEBUG("FmReadFile() returns false for headerLen, dataLen\n");
					goto __CATCH;
				}

				curOffset += length;

				// change dataLen based on converted data
				{
					UINT8	oneByte	= 0;
					int		j = 0;
					int		encodeLen = 0;
					char	szOutput[MSG_STDSTR_LONG] = {0, };

					while (true) {
						oneByte = pszMmsLoadTempBuf[j++];

						if (oneByte <= 0x7f)
							break;
					}

					encodeLen = __MmsDrm2BinaryEncodeUintvarLen((UINT32)nSize);
					__MmsDrm2BinaryEncodeUintvar((UINT32)nSize, encodeLen, szOutput);

					strncpy(&(pszMmsLoadTempBuf[j]), szOutput, encodeLen);
					pszMmsLoadTempBuf[j+encodeLen] = '\0';

					if (MsgWriteFile(pszMmsLoadTempBuf, sizeof(char), length, hConvertedFile) != (size_t)length) {
						MSG_DEBUG("Drm2WriteConvertData: FmWriteFile() returns false for dateLen\n");
						goto __CATCH;
					}
				}


				length = pMultipart->pBody->offset - pMultipart->type.offset;

				if (__MmsDrm2WriteDataToConvertedFile(pFile, hConvertedFile, pszMmsLoadTempBuf, length, bufLen) == false) {
					MSG_DEBUG("Drm2WriteConvertData: Write content type, headers fail\n");
					goto __CATCH;
				}

				curOffset += length;

				// write converted data
				hTempFile = MsgOpenFile(szTempFilePath, "rb");
				if (hTempFile == NULL) {
					MSG_DEBUG("Open decode temporary file fail\n");
					goto __CATCH;
				}

				length = nSize;

				if (__MmsDrm2WriteDataToConvertedFile(hTempFile, hConvertedFile, pszMmsLoadTempBuf, length, bufLen) == false) {
					MSG_DEBUG("Write converted data fail\n");
					goto __CATCH;
				}

				if (hTempFile != NULL) {
					MsgCloseFile(hTempFile);
					hTempFile = NULL;
				}

				curOffset += pMultipart->pBody->size;

				// move file pointer to the head of multipart
				if (MsgFseek(pFile, curOffset, SEEK_SET) < 0) {
					MSG_DEBUG("fail to seek file pointer \n");
					goto __CATCH;
				}
			} else {	// it doesn't need to convert if it is not CD or FL
				MSG_DEBUG("Write normal multipart data\n");

				length = pMultipart->pBody->offset + pMultipart->pBody->size - curOffset;

				if (__MmsDrm2WriteDataToConvertedFile(pFile, hConvertedFile, pszMmsLoadTempBuf, length, bufLen) == false) {
					MSG_DEBUG("Write multipart data fail \n");
					goto __CATCH;
				}

				curOffset += length;
			}

			pMultipart = pMultipart->pNext;
		}
	}

	MSG_DEBUG("end convert~~~~~~\n");

	if (pFile != NULL) {
		MsgCloseFile(pFile);
		pFile = NULL;
	}

	if (hConvertedFile != NULL) {
		MsgCloseFile(hConvertedFile);
		hConvertedFile = NULL;
	}

	if (pszMmsLoadTempBuf) {
		free(pszMmsLoadTempBuf);
		pszMmsLoadTempBuf = NULL;
	}

	if(remove(szTempFile) != 0)
		MSG_DEBUG("remove fail");
	if(remove(szTempFilePath) != 0)
		MSG_DEBUG("remove fail");

	return true;

__CATCH:

	if (pFile != NULL) {
		MsgCloseFile(pFile);
		pFile = NULL;
	}

	if (hConvertedFile != NULL) {
		MsgCloseFile(hConvertedFile);
		hConvertedFile = NULL;
	}

	if (hTempFile != NULL) {
		MsgCloseFile(hTempFile);
		hTempFile = NULL;
	}

	if (pszMmsLoadTempBuf) {
		free(pszMmsLoadTempBuf);
		pszMmsLoadTempBuf = NULL;
	}

	if (pszOrgData) {
		free(pszOrgData);
		pszOrgData = NULL;
	}

	if (hFile != NULL)
	{
		MsgCloseFile(hFile);
		hFile = NULL;
	}

	if (remove(szTempFile) != 0)
		MSG_DEBUG("remove fail");

	if (remove(szTempFilePath) != 0)
		MSG_DEBUG("remove fail");

	if (remove(MMS_DECODE_DRM_CONVERTED_TEMP_FILE) != 0)
		MSG_DEBUG("remove fail");	//remove convertin result if it goes to __CATCH

	return false;
}

/*************************************************************************
 * description : Function for decoding a converted file
 * argument : void
 * return value
    - bool :  result of converting
**************************************************************************/

bool MmsDrm2ReadMsgConvertedBody(MSG_MESSAGE_INFO_S *pMsg, bool bSavePartsAsTempFiles, bool bRetrieved, char *retrievedPath)
{
	MmsMsg *pMmsMsg;
	MmsPluginStorage::instance()->getMmsMessage(&pMmsMsg);
	MmsUnregisterDecodeBuffer();
#ifdef __SUPPORT_DRM__
	MmsReleaseMsgDRMInfo(&pMmsMsg->msgType.drmInfo);
#endif
	MmsReleaseMsgBody(&pMmsMsg->msgBody, pMmsMsg->msgType.type);

	if (MmsReadMsgBody(pMsg->msgId, bSavePartsAsTempFiles, bRetrieved, retrievedPath) == false) {
		MSG_DEBUG("MmsDrm2ReadMsgConvertedBody: _MmsReadMsgBody with converted file is failed\n");
		return false;
	}

	return true;
}

#endif

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
		MSG_DEBUG("_MmsBinaryDecodeGetOneByte: invalid file or buffer\n");
		goto __CATCH;
	}

	if (length < 1) {
		if (__MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
									   gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
			MSG_DEBUG("_MmsBinaryDecodeGetOneByte: fail to load to buffer \n");
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
			MSG_DEBUG("_MmsBinaryDecodeGetBytes: fail to load to buffer \n");
			goto __CATCH;
		}
	}

	for (i = 0; i < bufLen - 1; i++)
		szBuff[i] = gpCurMmsDecodeBuff[gCurMmsDecodeBuffPos++];


	gCurMmsDecodeBuffPos++;	//NULL

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
		MSG_DEBUG("_MmsBinaryDecodeGetLongBytes: fail to load to buffer \n");
		goto __CATCH;
	}

	while ((bufLen - iPos) >= gMmsDecodeMaxLen) {
		if (__MmsBinaryDecodeGetBytes(pFile, szBuff + iPos, gMmsDecodeMaxLen, totalLength) == false) {
			MSG_DEBUG("_MmsBinaryDecodeGetLongBytes: 1. _MmsBinaryDecodeGetBytes fail \n");
			goto __CATCH;
		}

		iPos += gMmsDecodeMaxLen;
	}

	if ((bufLen - iPos) > 0) {
		if (__MmsBinaryDecodeGetBytes(pFile, szBuff + iPos, (bufLen - iPos), totalLength) == false) {
			MSG_DEBUG("_MmsBinaryDecodeGetLongBytes: 2. _MmsBinaryDecodeGetBytes fail \n");
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
			MSG_DEBUG("__MmsBinaryDecodeUintvar: fail to load to buffer \n");
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
			MSG_DEBUG("__MmsBinaryDecodeUintvar : legnth is too long\n");
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
			MSG_DEBUG("__MmsHeaderDecodeIntegerByLength: _MmsBinaryDecodeGetOneByte fail\n");
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

	pData = (char *)malloc(length + 1);
	if (pData == NULL) {
		MSG_DEBUG("__MmsHeaderDecodeIntegerByLength: pData alloc fail\n");
		goto __CATCH;
	}
	memset(pData, 0, length + 1);

	if (__MmsBinaryDecodeGetBytes(pFile, pData, length + 1, totalLength) == false) {
		MSG_DEBUG("__MmsHeaderDecodeIntegerByLength: _MmsBinaryDecodeGetOneByte fail\n");
		goto __CATCH;
	}

	gCurMmsDecodeBuffPos--;	// - NULL

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
		MSG_DEBUG("__MmsBinaryDecodeInteger: GetOneByte fail\n");
		return false;
	}

	if (oneByte < 0x1F)				/* long integer : WAP-230-WSP-20010118-p, Proposed Version 18 January 2001 (pp.86) */
	{
		pData = (char *)malloc(oneByte + 1);
		if (pData == NULL) {
			MSG_DEBUG("__MmsBinaryDecodeInteger: pData memalloc fail\n");
			goto __CATCH;
		}
		memset(pData, 0, oneByte + 1);

		// Even NULL is copied in the _MmsBinaryDecodeGetBytes
		if (__MmsBinaryDecodeGetBytes(pFile, pData, oneByte + 1, totalLength) == false) {
			MSG_DEBUG("__MmsBinaryDecodeInteger: GetBytes fail\n");
			goto __CATCH;
		}

		gCurMmsDecodeBuffPos--;	// - NULL

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
			MSG_DEBUG("__MmsDecodeValueLength: __MmsBinaryDecodeUintvar fail..\n");
			goto __CATCH;
		}
		length ++;					// + length-quote
		*pValueLength = uintvar;
	} else {
		MSG_DEBUG("__MmsDecodeValueLength: not a value length type data\n");
		gCurMmsDecodeBuffPos--;
		return 0;
	}

	return length;

__CATCH:
	MSG_DEBUG("__MmsDecodeValueLength: getting data fail\n");
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
			MSG_DEBUG("__MmsDecodeValueLength2: __MmsBinaryDecodeUintvar fail..\n");
			goto __CATCH;
		}
		length ++;					// + length-quote
		*pValueLength = uintvar;
	} else {
		MSG_DEBUG("__MmsDecodeValueLength2: there is not length-quote, consider it as short length.\n");
		*pValueLength = oneByte;
		length = 1;
	}

	return length;

__CATCH:
	MSG_DEBUG("__MmsDecodeValueLength2: getting data fail\n");
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
		MSG_DEBUG("__MmsBinaryDecodeQuotedString: 1. fail to load to buffer \n");
		goto __CATCH;
	}

	length = strlen(gpCurMmsDecodeBuff) + 1;	// + NULL

	if (length == 0)
		goto __RETURN;

	while (length > gMmsDecodeBufLen) {
		if (gMmsDecodeBufLen <= 0) {
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: gMmsDecodeBufLen <= 0 \n");
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: %x %x %x %x %x\n",
										gpCurMmsDecodeBuff[0], gpCurMmsDecodeBuff[1], gpCurMmsDecodeBuff[2],
										gpCurMmsDecodeBuff[3], gpCurMmsDecodeBuff[4]);
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: %x %x %x %x %x\n",
										gpCurMmsDecodeBuff[5], gpCurMmsDecodeBuff[6], gpCurMmsDecodeBuff[7],
										gpCurMmsDecodeBuff[8], gpCurMmsDecodeBuff[9]);
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: %x %x %x %x %x\n",
										gpCurMmsDecodeBuff[10], gpCurMmsDecodeBuff[11], gpCurMmsDecodeBuff[12],
										gpCurMmsDecodeBuff[13], gpCurMmsDecodeBuff[14]);
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: %x %x %x %x %x\n",
										gpCurMmsDecodeBuff[15], gpCurMmsDecodeBuff[16], gpCurMmsDecodeBuff[17],
										gpCurMmsDecodeBuff[18], gpCurMmsDecodeBuff[19]);
			goto __CATCH;
		}

		pData = (char *)malloc(gMmsDecodeBufLen + 1);
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
			MSG_DEBUG("__MmsBinaryDecodeText: 2. fail to load to buffer \n");
			goto __CATCH;
		}
		length = strlen(gpCurMmsDecodeBuff) + 1;	// + NULL
	}	/* while */

	if (length > 0) {
		pData = (char *)malloc(length);
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
				strncpy(szBuff + iPos, (char*)pData, readBytes - 1);	// + NULL
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
		MSG_DEBUG("__MmsBinaryDecodeText: 1. fail to load to buffer \n");
		goto __CATCH;
	}

	length = strlen(gpCurMmsDecodeBuff) + 1;	// + NULL

	if (length == 0)
		goto __RETURN;

	while (length > gMmsDecodeBufLen) {
		if (gMmsDecodeBufLen <= 0) {
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: gMmsDecodeBufLen <= 0 \n");
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: %x %x %x %x %x\n", gpCurMmsDecodeBuff[0], gpCurMmsDecodeBuff[1], gpCurMmsDecodeBuff[2],
										gpCurMmsDecodeBuff[3], gpCurMmsDecodeBuff[4]);
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: %x %x %x %x %x\n", gpCurMmsDecodeBuff[5], gpCurMmsDecodeBuff[6], gpCurMmsDecodeBuff[7],
										gpCurMmsDecodeBuff[8], gpCurMmsDecodeBuff[9]);
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: %x %x %x %x %x\n", gpCurMmsDecodeBuff[10], gpCurMmsDecodeBuff[11], gpCurMmsDecodeBuff[12],
										gpCurMmsDecodeBuff[13], gpCurMmsDecodeBuff[14]);
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: %x %x %x %x %x\n", gpCurMmsDecodeBuff[15], gpCurMmsDecodeBuff[16], gpCurMmsDecodeBuff[17],
										gpCurMmsDecodeBuff[18], gpCurMmsDecodeBuff[19]);
			goto __CATCH;
		}

		pData = (char *)malloc(gMmsDecodeBufLen + 1);
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
			MSG_DEBUG("__MmsBinaryDecodeText: 2. fail to load to buffer \n");
			goto __CATCH;
		}
		length = strlen(gpCurMmsDecodeBuff) + 1;	// + NULL
	}	/* while */

	if (length > 0) {
		pData = (char *)malloc(length);
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
				strncpy(szBuff + iPos, (char*)pData, readBytes - 1);	// + NULL
				iPos += readBytes;
			}
		}

		if (pData) {
			free(pData);
			pData = NULL;
		}

		returnLength += length;		// + NULL
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
		MSG_DEBUG("__MmsBinaryDecodeTextLen: 1. fail to load to buffer \n");
		goto __CATCH;
	}

	length = strlen(gpCurMmsDecodeBuff) + 1;

	if (length == 0)
		goto __CATCH;

	while (length > gMmsDecodeBufLen) {
		if (gMmsDecodeBufLen <= 0) {
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: gMmsDecodeBufLen <= 0 \n");
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: %x %x %x %x %x\n",
										gpCurMmsDecodeBuff[0], gpCurMmsDecodeBuff[1], gpCurMmsDecodeBuff[2],
										gpCurMmsDecodeBuff[3], gpCurMmsDecodeBuff[4]);
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: %x %x %x %x %x\n",
										gpCurMmsDecodeBuff[5], gpCurMmsDecodeBuff[6], gpCurMmsDecodeBuff[7],
										gpCurMmsDecodeBuff[8], gpCurMmsDecodeBuff[9]);
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: %x %x %x %x %x\n",
										gpCurMmsDecodeBuff[10], gpCurMmsDecodeBuff[11], gpCurMmsDecodeBuff[12],
										gpCurMmsDecodeBuff[13], gpCurMmsDecodeBuff[14]);
			MSG_DEBUG("__MmsBinaryDecodeQuotedString: %x %x %x %x %x\n",
										gpCurMmsDecodeBuff[15], gpCurMmsDecodeBuff[16], gpCurMmsDecodeBuff[17],
										gpCurMmsDecodeBuff[18], gpCurMmsDecodeBuff[19]);
			goto __CATCH;
		}

		pData = (char *)malloc(gMmsDecodeBufLen + 1);
		if (pData == NULL)
			goto __CATCH;

		memset(pData, 0, gMmsDecodeBufLen + 1);

		if (__MmsBinaryDecodeGetBytes(pFile, pData, gMmsDecodeBufLen, totalLength) == false)
			goto __CATCH;

		if (szBuff == NULL)	{
			szBuff = (char *)malloc(gMmsDecodeBufLen + 1);
		} else {
			szTempPtr = (char *)realloc(szBuff, curLen + gMmsDecodeBufLen + 1);

			//NULL pointer check for realloc
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
							   totalLength) == false)
		{
			MSG_DEBUG("__MmsBinaryDecodeText: 2. fail to load to buffer \n");
			goto __CATCH;
		}
		length = strlen(gpCurMmsDecodeBuff) + 1;
	}	/* while */

	if (length > 0)	{
		pData = (char *)malloc(length);
		if (pData == NULL) {
			goto __CATCH;
		}

		if (__MmsBinaryDecodeGetBytes(pFile, pData, length, totalLength) == false) {
			goto __CATCH;
		}

		if (szBuff == NULL) {
			szBuff = (char *)malloc(length);
		} else {
			szTempPtr = (char *)realloc(szBuff, curLen + length);

			//NULL pointer check for realloc
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

		*pLength += length;		// + NULL
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
		MSG_DEBUG("__MmsBinaryDecodeCharset : __MmsBinaryDecodeInteger fail...\n");
		goto __CATCH;
	}

	if (integer == 0) {
		/* AnyCharSet : return MSG_CHARSET_UTF8 */
		*nCharSet = MSG_CHARSET_UTF8;
		return true;
	}

	*nCharSet = MmsGetBinaryType(MmsCodeCharSet, (UINT16)integer);

	if (*nCharSet == MIME_UNKNOWN) {
		MSG_DEBUG("__MmsBinaryDecodeCharset : MmsGetBinaryType fail..\n");
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

	MSG_DEBUG("__MmsBinaryDecodeEncodedString: decode string..\n");

	if (pFile == NULL || szBuff == NULL || bufLen <= 0) {
		MSG_DEBUG("__MmsBinaryDecodeEncodedString: invalid file or buffer\n");
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
			MSG_DEBUG("__MmsBinaryDecodeEncodedString : 1. __MmsBinaryDecodeText fail.\n");
			goto __CATCH;
		}
		break;

	default:

		/* Value-length Charset Text_string */

		if (__MmsBinaryDecodeCharset(pFile, &charSet, &charSetLen, totalLength) == false) {
			MSG_DEBUG("__MmsBinaryDecodeEncodedString : __MmsBinaryDecodeCharset error\n");
			goto __CATCH;			/* (valueLength + valueLengthLen) */
		}

		nTemp = __MmsBinaryDecodeText(pFile, szBuff, bufLen, totalLength);
		if (nTemp < 0) {
			/* There can be some error in data - no NULL -> try again with value length */

			pData = (char *)malloc(valueLength - charSetLen);
			if (pData == NULL) {
				MSG_DEBUG("__MmsBinaryDecodeEncodedString : pData alloc fail.\n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeGetLongBytes(pFile, pData, valueLength - charSetLen, totalLength) == false) {
				MSG_DEBUG("__MmsBinaryDecodeEncodedString : _MmsBinaryDecodeGetLongBytes fail.\n");
				goto __CATCH;
			}

			strncpy(szBuff, pData, bufLen - 1);
		}

		{//temp brace

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
				snprintf(szBuff, destLen, "%s", pDest);
			}
		}
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

	MSG_DEBUG("__MmsDecodeEncodedAddress: decoding address..\n");

	if (pFile == NULL) {
		MSG_DEBUG("__MmsDecodeEncodedAddress: invalid file or buffer\n");
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
			MSG_DEBUG("__MmsDecodeEncodedAddress : 1. __MmsBinaryDecodeText2 fail.\n");
			goto __CATCH;
		}
		break;

	default:

		/* Value-length Charset Text_string */

		if (__MmsBinaryDecodeCharset(pFile, &charSet, &charSetLen, totalLength) == false) {
			MSG_DEBUG("__MmsDecodeEncodedAddress : __MmsBinaryDecodeCharset error\n");
			goto __CATCH;
		}

		textLength = 0;
		pAddrStr   = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
		if (pAddrStr == NULL) {
			/* There can be some error in data - no NULL -> try again with value length */

			pAddrStr = (char *)malloc(valueLength - charSetLen);
			if (pAddrStr == NULL) {
				MSG_DEBUG("__MmsDecodeEncodedAddress : pData alloc fail.\n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeGetLongBytes(pFile, pAddrStr, valueLength - charSetLen, totalLength) == false) {
				MSG_DEBUG("__MmsDecodeEncodedAddress : _MmsBinaryDecodeGetLongBytes fail.\n");
				goto __CATCH;
			}
		}

		/* fixme: charset transformation */

		break;
	}

	pAddr = (MsgHeaderAddress *)malloc(sizeof(MsgHeaderAddress));
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

	//remove ""
	if (pLatinBuff) {
		szSrc = MsgRemoveQuoteFromFilename(pLatinBuff);
		if (szSrc) {
			strcpy(pLatinBuff, szSrc);
			free(szSrc);
			szSrc = NULL;
		}

		szSrc2 = MsgChangeHexString(pLatinBuff);
		if (szSrc2) {
			strcpy(pLatinBuff, szSrc2);
			free(szSrc2);
			szSrc2 = NULL;
		}

		if (__MsgIsUTF8String((unsigned char*)pLatinBuff, strlen(pLatinBuff)) == false) {
			length = strlen(pLatinBuff);

			int		utf8BufSize = 0;
			utf8BufSize = __MsgGetLatin2UTFCodeSize((unsigned char*)pLatinBuff, length);
			if (utf8BufSize < 3)
				utf8BufSize = 3;//min value

			pUTF8Buff = (char *)malloc(utf8BufSize + 1);
			if (pUTF8Buff == NULL) {
				MSG_DEBUG("__MmsDecodeGetFilename: pUTF8Buff alloc fail \n");
				goto __CATCH;
			}

			if (__MsgLatin2UTF ((unsigned char*)pUTF8Buff, utf8BufSize + 1, (unsigned char*)pLatinBuff, length) < 0) {
				MSG_DEBUG("__MmsDecodeGetFilename: MsgLatin2UTF fail \n");
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
			int nameLength	= 0;
			nameLength = (length < bufLen) ? (length - strlen(pExt)) : (bufLen - strlen(pExt));
			strncpy(szBuff, pUTF8Buff, nameLength);
			strcat (szBuff, pExt);
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

//  to get message body this function should be modified from message raw file.
bool MmsReadMsgBody(msg_message_id_t msgID, bool bSavePartsAsTempFiles, bool bRetrieved, char *retrievedPath)
{
	FILE *pFile	= NULL;
	MmsMsg *pMsg = NULL;
	MsgMultipart *pMultipart = NULL;
	int nSize = 0;
	char szFullPath[MSG_FILEPATH_LEN_MAX] = {0, };
	char szTempMediaDir[MSG_FILEPATH_LEN_MAX] = {0, };
	int attachmax = MSG_ATTACH_MAX;

	MSG_DEBUG("_MmsReadMsgBody: start read msg(msgID=%d)\n", msgID);

	MmsPluginStorage::instance()->getMmsMessage(&pMsg);
	memset(pMsg, 0, sizeof(MmsMsg));

	MmsInitHeader();

	if (bRetrieved && (retrievedPath != NULL)) {
		strncpy(szFullPath, retrievedPath, (strlen(retrievedPath) > MSG_FILEPATH_LEN_MAX ? MSG_FILEPATH_LEN_MAX:strlen(retrievedPath)));
	} else {
		MmsPluginStorage::instance()->getMmsRawFilePath(msgID, szFullPath);
	}

	pMsg->msgID = msgID;

	/*	read from MMS raw file	*/
	strncpy(pMsg->szFileName, szFullPath + strlen(MSG_DATA_PATH), strlen(szFullPath + strlen(MSG_DATA_PATH)));

	MSG_DEBUG("szFullPath = (%s)", szFullPath);

	if (MsgGetFileSize(szFullPath, &nSize) == false) {
		MSG_DEBUG("MsgGetFileSize: failed");
		goto __CATCH;
	}

	pFile = MsgOpenFile(szFullPath, "rb");

	if (pFile == NULL) {
		MSG_DEBUG("_MmsReadMsgBody: invalid mailbox\n");
		goto __CATCH;
	}

	MmsRegisterDecodeBuffer();

	if (MmsBinaryDecodeMsgHeader(pFile, nSize) == false) {
		MSG_DEBUG("_MmsReadMsgBody: MmsBinaryDecodeMsgHeader fail...\n");
		goto __CATCH;
	}

#ifdef	__SUPPORT_DRM__
	if (MmsDrm2GetConvertState() != MMS_DRM2_CONVERT_FINISH)
		MmsDrm2SetConvertState(MMS_DRM2_CONVERT_NONE);	//initialize convertState
#endif

	if (MmsBinaryDecodeMsgBody(pFile, szFullPath, nSize) == false) {
		MSG_DEBUG("_MmsReadMsgBody: MmsBinaryDecodeMsgBody fail\n");
		goto __CATCH;
	}

#ifdef	__SUPPORT_DRM__
	if (MmsDrm2GetConvertState() == MMS_DRM2_CONVERT_REQUIRED) {
		MSG_DEBUG("_MmsReadMsgBody: MmsDrm2GetConvertState returns MMS_DRM2_CONVERT_REQUIRED.\n");
		goto RETURN;
	}
#endif

	/* Set mmsHeader.msgType & msgBody to pMsg ----------- */

	pMsg->mmsAttrib.contentType = (MimeType)mmsHeader.msgType.type;

	memcpy(&(pMsg->msgType), &(mmsHeader.msgType), sizeof(MsgType));
	memcpy(&(pMsg->msgBody), &(mmsHeader.msgBody), sizeof(MsgBody));

	if (pMsg->msgBody.pPresentationBody) {
		if(MsgFseek(pFile, pMsg->msgBody.pPresentationBody->offset, SEEK_SET) < 0)
			goto __CATCH;

		pMsg->msgBody.pPresentationBody->body.pText = (char *)malloc(pMsg->msgBody.pPresentationBody->size + 1);
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

			if (pMultipart->type.type == MIME_TEXT_PLAIN)
				attachmax++;

			if ((mmsHeader.msgType.type == MIME_APPLICATION_VND_WAP_MULTIPART_MIXED)||(mmsHeader.msgType.type == MIME_MULTIPART_MIXED)) {
				if ((pMsg->nPartCount >= attachmax)&&(pMultipart->pNext != NULL)) {
					MmsReleaseMsgBody(pMultipart->pNext->pBody, pMultipart->pNext->type.type);

					free(pMultipart->pNext->pBody);
					pMultipart->pNext->pBody= NULL;

					free(pMultipart->pNext);

					pMultipart->pNext = NULL;
					break;
				}
			}
			pMultipart = pMultipart->pNext;
		}
	} else {
		if (pMsg->msgBody.size > 0)
			pMsg->nPartCount++;
	}

	/* 	make temporary	*/
	snprintf(szTempMediaDir, MSG_FILEPATH_LEN_MAX, MSG_DATA_PATH"%s.dir", pMsg->szFileName);

	if (MsgIsMultipart(pMsg->msgType.type) == true) {
		int partIndex = 0;
		pMultipart = pMsg->msgBody.body.pMultipart;

		if (bSavePartsAsTempFiles) {
			if (mkdir(szTempMediaDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
				if (errno == EEXIST) {
					MSG_DEBUG("The %s already exists", szTempMediaDir);
				} else {
					MSG_DEBUG("Fail to Create Dir [%s]", szTempMediaDir);
					goto __CATCH;
				}
			}
		}

		while (pMultipart) {

			if (__MmsMultipartSaveAsTempFile(&pMultipart->type, pMultipart->pBody,
											(char*)MSG_DATA_PATH, pMsg->szFileName, partIndex, bSavePartsAsTempFiles) == false)
				goto __CATCH;

			pMultipart = pMultipart->pNext;
			partIndex ++;
		}

	} else { //single part
		if (pMsg->nPartCount > 0) {

			if (bSavePartsAsTempFiles) {
				if (mkdir(szTempMediaDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
					if (errno == EEXIST) {
						MSG_DEBUG("The %s already exists", szTempMediaDir);
					} else {
						MSG_DEBUG("Fail to Create Dir [%s]", szTempMediaDir);
						goto __CATCH;
					}
				}
			}

			if (__MmsMultipartSaveAsTempFile( &pMsg->msgType, &pMsg->msgBody,
											(char*)MSG_DATA_PATH, pMsg->szFileName, 0, bSavePartsAsTempFiles) == false)
				goto __CATCH;
		}
	}
	MSG_DEBUG("****   _MmsReadMsgBody:  E  N  D   (Success)  ***\n");
	return true;

#ifdef	__SUPPORT_DRM__

RETURN:

	if (pFile != NULL) {
		MsgCloseFile(pFile);
		pFile = NULL;
	}

	return false;

#endif

__CATCH:

	MmsInitHeader();
	MmsUnregisterDecodeBuffer();

	if (pFile != NULL) {
		MsgCloseFile(pFile);
		pFile = NULL;
	}

#ifdef __SUPPORT_DRM__
	MmsReleaseMsgDRMInfo(&pMsg->msgType.drmInfo);
#endif

	MmsReleaseMsgBody(&pMsg->msgBody, pMsg->msgType.type);
	MSG_DEBUG("_MmsReadMsgBody:    E  N  D    (fail)    ******************** \n");

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
		MSG_DEBUG("_MsgGetStringUntilDelimiter: pszString == NULL \n");
		return NULL;
	}

	if ((pszStrDelimiter = strchr(pszString, delimiter)) == NULL) {
		MSG_DEBUG("_MsgGetStringUntilDelimiter: There is no %c in %s. \n", delimiter, pszString);
		return NULL;
	}

	bufLength = pszStrDelimiter - pszString;

	if ((pszBuffer = (char*)malloc (bufLength + 1)) == NULL) {
		MSG_DEBUG("malloc is failed");
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

	pNew = (char *)malloc(cLen + 1);
	if (pNew == NULL)
		return NULL;

	memset(pNew, 0, cLen + 1);

	for (cIndex = 0; cIndex< cLen ; cIndex++) {
		if (pOrg[cIndex] == '%') {
			if (pOrg[cIndex+1] != 0 && pOrg[cIndex+2] != 0) 	{
				snprintf(szBuf, sizeof(szBuf), "%c%c", pOrg[cIndex+1], pOrg[cIndex+2]); // read two chars after '%'

				if (__MsgIsHexChar(szBuf) == true) { // check the two character is between  0 ~ F
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

				pDec = MsgDecodeText(pValue);		// Api is to long, consider Add to another file (MsgMIMECodec.c)
			} else {
				pDec = MsgDecodeText(pName);
			}

			switch (_MsgGetCode(MSG_PARAM, pSrc)) {
			case MSG_PARAM_BOUNDARY:

				/* RFC 822: boundary := 0*69<bchars> bcharsnospace */

				memset (pType->param.szBoundary, 0, MSG_BOUNDARY_LEN + 1);
				strncpy(pType->param.szBoundary, pDec, MSG_BOUNDARY_LEN);
				MSG_DEBUG("_MsgParseParameter: szBoundary = %s \n", pType->param.szBoundary);
				break;

			case MSG_PARAM_CHARSET:
				pType->param.charset = _MsgGetCode(MSG_CHARSET, pDec);

				if (pType->param.charset == INVALID_HOBJ)
					pType->param.charset = MSG_CHARSET_UNKNOWN;

				MSG_DEBUG("_MsgParseParameter: type = %d    [charset] = %d \n", pType->type, pType->param.charset);
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
						strcat (pType->param.szName, pExt);
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

					// Remvoe '/', ex) Content-Type: image/gif; name="images/vf7.gif"
					__MsgRemoveFilePath(pType->param.szName);
				} else {
					MSG_DEBUG("_MsgParseParameter: MsgConvertLatin2UTF8FileName(%s) return NULL\n", pDec);
				}

				MSG_DEBUG("_MsgParseParameter: szName = %s \n", pType->param.szName);
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
						strcat (pType->param.szFileName, pExt);
					} else {
						strncpy(pType->param.szFileName, pUTF8Buff, (MSG_FILENAME_LEN_MAX-1));
					}
					free(pUTF8Buff);
					pUTF8Buff = NULL;

					if (__MsgChangeSpace(pType->param.szFileName, &szSrc) == true)
						strcpy(pType->param.szFileName, szSrc);

					if (szSrc) {
						free(szSrc);
						szSrc = NULL;
					}

					// Remvoe '/', ex) Content-Type: image/gif; name="images/vf7.gif"
					__MsgRemoveFilePath(pType->param.szFileName);
				} else {
					MSG_DEBUG("_MsgParseParameter: MsgConvertLatin2UTF8FileName(%s) return NULL\n", pDec);
				}

				MSG_DEBUG("_MsgParseParameter: szFileName = %s \n", pType->param.szFileName);

				break;

			case MSG_PARAM_TYPE:

				/* type/subtype of root. Only if content-type is multipart/related */

				pType->param.type = _MsgGetCode(MSG_TYPE, pDec);
				MSG_DEBUG("_MsgParseParameter: type = %d \n", pType->param.type);

				break;

			case MSG_PARAM_START:

				/* Content-id. Only if content-type is multipart/related */

				memset (pType->param.szStart, 0, MSG_MSG_ID_LEN + 1);
				strncpy(pType->param.szStart, pDec, MSG_MSG_ID_LEN);

				MSG_DEBUG("_MsgParseParameter: szStart = %s \n", pType->param.szStart);

				break;

			case MSG_PARAM_START_INFO :

				/* Only if content-type is multipart/related */

				memset (pType->param.szStartInfo, 0, MSG_MSG_ID_LEN + 1);
				strncpy(pType->param.szStartInfo, pDec, MSG_MSG_ID_LEN);

				MSG_DEBUG("_MsgParseParameter: szStartInfo = %s \n", pType->param.szStartInfo);

				break;

			case MSG_PARAM_REPORT_TYPE :

				//  only used as parameter of Content-Type: multipart/report; report-type=delivery-status;

				if (pDec != NULL && strcasecmp(pDec, "delivery-status") == 0) {
					pType->param.reportType = MSG_PARAM_REPORT_TYPE_DELIVERY_STATUS;
				} else {
					pType->param.reportType = MSG_PARAM_REPORT_TYPE_UNKNOWN;
				}

				MSG_DEBUG("_MsgParseParameter: reportType = %s \n", pDec);
				break;

			default:

				MSG_DEBUG("_MsgParseParameter: Unknown paremeter (%s)\n", pDec);
				break;
			}

			if (pDec) {
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

	// ignore empty space
	for (ret = ++s1; *ret == ' '; ret++)
		;

	// handle '(', ')', '\',  '\0'
	do {
		switch (*s1) {
		case '(':
			if (!__MsgSkipComment (s1,(long)NULL))
				return NULL;
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
			return NULL;
		case ' ':
			break;
		default:
			t = s1;
			break;
		}
	}while (s1++);

	return NULL;
}

static char *__MsgConvertLatin2UTF8FileName(char *pSrc)
{
	char *pUTF8Buff  = NULL;
	char *pData = NULL;


	//convert utf8 string
	if (__MsgIsUTF8String((unsigned char*)pSrc, strlen(pSrc)) == false) {
		int length  = 0;
		int utf8BufSize = 0;

		length = strlen(pSrc);
		utf8BufSize = __MsgGetLatin2UTFCodeSize((unsigned char*)pSrc, length);
		if (utf8BufSize < 3)
			utf8BufSize = 3; //min value

		pUTF8Buff = (char *)malloc(utf8BufSize + 1);

		if (pUTF8Buff == NULL) {
			MSG_DEBUG("MsgConvertLatin2UTF8FileName: pUTF8Buff alloc fail \n");
			goto __CATCH;
		}

		if (__MsgLatin2UTF ((unsigned char*)pUTF8Buff, utf8BufSize + 1, (unsigned char*)pSrc, length) < 0) {
			MSG_DEBUG("MsgConvertLatin2UTF8FileName: MsgLatin2UTF fail \n");
			goto __CATCH;
		}
	} else {
		int length = strlen(pSrc);
		pUTF8Buff = (char *)calloc(1, length+1);

		if (pUTF8Buff == NULL) {
			MSG_DEBUG("MsgConvertLatin2UTF8FileName: pUTF8Buff alloc fail \n");
			goto __CATCH;
		}

		memcpy(pUTF8Buff, pSrc, length);
	}

	//convert hex string
	if (__MsgIsPercentSign(pUTF8Buff) == true) {
		pData = MsgChangeHexString(pUTF8Buff);
		if (pData) {
			strcpy(pUTF8Buff, pData);
			free(pData);
			pData = NULL;
		}
	}

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

	pNew = (char *)malloc(cLen + 1);
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
	// Remvoe '/', ex) Content-Type: image/gif; name="images/vf7.gif"
	char *pPath = NULL;
	char *pTemp = NULL;
	char szFileName[MSG_FILENAME_LEN_MAX] = {0};

	if (pSrc == NULL)
		return;

	pTemp = pSrc;
	while ((pTemp = strchr(pTemp, '/')) != NULL) {
		// Find the last  '/'
		pPath = pTemp;
		pTemp++;
	}

	if (pPath) {
		MSG_DEBUG("_MsgRemoveFilePath: filename(%s)\n", pSrc);

		// case : images/vf7.gif -> vf7.gif
		if (pPath != NULL && *(pPath+1) != '\0') {
			strncpy(szFileName, pPath+1, strlen(pPath+1));
			strncpy(pSrc, szFileName , strlen(szFileName));
		}
	}
	// Remove additional file information
	// ex) Content-type: application/octet-stream; name="060728gibson_210.jpg?size=s"
	// if "?size=" exist, insert NULL char.
	{
		pTemp = strcasestr(pSrc, "?size=");
		if (pTemp != NULL)
			*pTemp = '\0';
	}
}

static bool __MsgIsUTF8String(unsigned char *szSrc, int nChar)
{
	MSG_DEBUG("MsgIsUTF8String: --------------- \n");

	if (szSrc == NULL) {
		MSG_DEBUG("MsgIsUTF8String: szSrc is NULL !!!! --------------- \n");
		return true;
	}

	while (nChar > 0 && (*szSrc != '\0')) {
		if (*szSrc < 0x80) {
			szSrc++;
			nChar--;
		} else if ((0xC0 <= *szSrc) && (*szSrc < 0xE0)) {
			if (*(szSrc + 1) >= 0x80) {
				szSrc += 2;
				nChar -= 2;
			} else {
				MSG_DEBUG("MsgIsUTF8String: 1. NOT utf8 range!\n");
				goto __CATCH;
			}
		} else if (*szSrc >= 0xE0) {
			if (*(szSrc + 1) >= 0x80) {
				if (*(szSrc + 2) >= 0x80) {
					szSrc += 3;
					nChar -= 3;
				} else {
					MSG_DEBUG("MsgIsUTF8String: 2. NOT utf8 range!\n");
					goto __CATCH;
				}
			} else {
				MSG_DEBUG("MsgIsUTF8String: 3. NOT utf8 range!\n");
				goto __CATCH;
			}
		} else {
			MSG_DEBUG("MsgIsUTF8String: 4. NOT utf8 range!\n");
			goto __CATCH;
		}
	}

	return true;

__CATCH:
	return false;
}

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

static MsgMultipart *__MsgAllocMultipart(void)
{
	MsgMultipart *pMultipart = NULL;

	MSG_DEBUG("MsgAllocMultipart: --------- \n");

	pMultipart = (MsgMultipart*)malloc(sizeof(MsgMultipart));
	if (pMultipart == NULL) {
		MSG_DEBUG("MsgAllocMultipart: pMultipart malloc Fail \n");
		goto __CATCH;
	}

	pMultipart->pBody = (MsgBody*)malloc(sizeof(MsgBody));
	if (pMultipart->pBody == NULL) {
		MSG_DEBUG("MsgAllocMultipart: pMultipart->pBody malloc Fail \n");
		goto __CATCH;
	}

	MmsInitMsgType(&pMultipart->type);
	MmsInitMsgBody(pMultipart->pBody);

	pMultipart->pNext = NULL;

	return pMultipart;

__CATCH:

	if (pMultipart) {
		if (pMultipart->pBody) {
			free(pMultipart->pBody);
			pMultipart->pBody = NULL;
		}

		free(pMultipart);
		pMultipart = NULL;
	}

	return NULL;
}

static MsgPresentationFactor __MsgIsPresentationEx(MsgType *multipartType, char* szStart, MimeType typeParam)
{
	char szTmpStart[MSG_MSG_ID_LEN + 3] = { 0, };
	char szTmpContentID[MSG_MSG_ID_LEN + 3] = { 0, };
	char szTmpContentLO[MSG_MSG_ID_LEN + 3] = { 0, };
	int strLen = 0;

	// remove '<' and '>' in Start Param : contentID ex] <0_1.jpg> or <1233445>
	if (szStart && szStart[0]) {
		int startLen = 0;
		startLen = strlen(szStart);
		if (szStart[0] == '<' && szStart[startLen - 1] == '>') {
			strncpy(szTmpStart, &szStart[1], startLen - 2);
		} else {
			strncpy(szTmpStart, szStart, startLen);
		}
	}

	// remove '<' and '>' in ContentID : contentID ex] <0_1.jpg> or <1233445>
	if (multipartType->szContentID[0]) 	{
		strLen = strlen(multipartType->szContentID);
		if (multipartType->szContentID[0] == '<' && multipartType->szContentID[strLen - 1] == '>') {
			strncpy(szTmpContentID, &(multipartType->szContentID[1]), strLen - 2);
		} else {
			strncpy(szTmpContentID, multipartType->szContentID, strLen);
		}
	}

	// remove '<' and '>' in ContentLocation : contentID ex] <0_1.jpg> or <1233445>
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

	// exception handling
	if (szTmpStart[0] != '\0') {
		// presentation part : 1.compare with contentID 2.compare with content Location 3. compare with type
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
		// assign the multipart to presentation part
		// remove the multipart(pCurPresentation) which is presentation part from the linked list.
		// if there is no presentation part -> assign first multipart to presentation part by force.
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

			// remove pCurPresentation from multipart linked list
			if ((pPresentationInfo->factor == MSG_PRESENTATION_NONE)||(pPresentationInfo->pPrevPart == NULL)) {
				// first part
				pMsgBody->body.pMultipart = pPresentationInfo->pCurPresentation->pNext;
				pMsgType->contentSize -= pPresentationInfo->pCurPresentation->pBody->size;
				pMsgBody->size -= pPresentationInfo->pCurPresentation->pBody->size;
				if (pPresentationInfo->pCurPresentation) {
#ifdef __SUPPORT_DRM__
					MmsReleaseMsgDRMInfo(&pPresentationInfo->pCurPresentation->type.drmInfo);
#endif
					free(pPresentationInfo->pCurPresentation);
					pPresentationInfo->pCurPresentation = NULL;
				}
			} else {
				// not a first part
				pPresentationInfo->pPrevPart->pNext = pPresentationInfo->pCurPresentation->pNext;
				pMsgType->contentSize -= pPresentationInfo->pCurPresentation->pBody->size;
				pMsgBody->size -= pPresentationInfo->pCurPresentation->pBody->size;
				if (pPresentationInfo->pCurPresentation) {
					free(pPresentationInfo->pCurPresentation);
					pPresentationInfo->pCurPresentation = NULL;
				}
			}
		} else if (pPresentationInfo->pCurPresentation != NULL && __MsgIsText(pPresentationInfo->pCurPresentation->type.type)) {
			/* NON-Presentable Part is some PLAIN part such as, text/plain, multipart/alternative.
			 * In this case, leave the Presentation part as a multipart and remove other multiparts.
			 */

			// Backup the multipart link information
			pNextPart = pMsgBody->body.pMultipart;

			// Copy presentation part as a main part
			memcpy(pMsgType, &pPresentationInfo->pCurPresentation->type, sizeof(MsgType));
			memcpy(pMsgBody, pPresentationInfo->pCurPresentation->pBody, sizeof(MsgBody));

			// Remove multipart linked list
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
#ifdef __SUPPORT_DRM__
			MmsReleaseMsgDRMInfo(&pMsgBody->presentationType.drmInfo);
#endif
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

#ifdef __SUPPORT_DRM__

bool MsgCopyDrmInfo(MsgType *pPartType)
{
	char *pExt = NULL;
	char *pTmpBuf = NULL;

	//convert application/vnd.oma.drm.content to media type
	pPartType->type = pPartType->drmInfo.contentType;

	// fix wrong file name presentation on save media screen.
	if (pPartType->szContentID[0] == '\0' && pPartType->drmInfo.szContentURI)
		strncpy(pPartType->szContentID, pPartType->drmInfo.szContentURI, MSG_MSG_ID_LEN);

	/* set title name (content name) */
	if (pPartType->param.szName[0] == '\0') {
		/*	szName is vitual name, real filename is *.dcf or *.dm	*/
		if (pPartType->drmInfo.szContentName && pPartType->drmInfo.szContentName[0] != '\0') {
			/* In case of szContentName retrieved from DRM agent is exist. */
			pTmpBuf = pPartType->drmInfo.szContentName;
		} else if (pPartType->szContentLocation[0] != '\0') 	{
			/* In case of szContentLocation parsed from MMS header */
			pTmpBuf = strrchr(pPartType->szContentLocation, '/');
			if (pTmpBuf == NULL)
				pTmpBuf = pPartType->szContentLocation;
		} else {
			/* use another name */
			/* possible NULL pointer assignment*/
			pTmpBuf = strdup("untitled");
		}

		if ((pExt = strrchr(pTmpBuf, '.')) != NULL) {
			int extLen = 0;
			int fileNameLen = 0;
			int tmpLen = 0;

			extLen = strlen(pExt);
			tmpLen = strlen(pTmpBuf);
			fileNameLen = (tmpLen - extLen < MSG_LOCALE_FILENAME_LEN_MAX - extLen)?(tmpLen - extLen):(MSG_LOCALE_FILENAME_LEN_MAX - extLen);
			strncpy(pPartType->param.szName, pTmpBuf, fileNameLen);
			strcpy (pPartType->param.szName + fileNameLen, pExt);
		} else {
			strncpy(pPartType->param.szName, pTmpBuf, MSG_LOCALE_FILENAME_LEN_MAX);
			__MsgMakeFileName(pPartType->type, pPartType->param.szName, MSG_DRM_TYPE_NONE, 0);
		}
	}

	return true;
}

#endif

static bool __MsgIsText(int type)
{
	if (type == MIME_TEXT_PLAIN || type == MIME_TEXT_HTML || type == MIME_TEXT_VND_WAP_WML ||
		type == MIME_TEXT_X_VNOTE || type == MIME_APPLICATION_SMIL || type == MIME_TEXT_X_IMELODY) {
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

		MSG_DEBUG("MsgResolveNestedMultipart : MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE\n");

		pSelectedPart = pPartBody->body.pMultipart;

		// NULL Pointer check!!
		if (pSelectedPart == NULL) {
			MSG_DEBUG("MsgResolveNestedMultipart : multipart(ALTERNATIVE) does not exist\n");
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
#ifdef __SUPPORT_DRM__
			MmsReleaseMsgDRMInfo(&pRemoveList->type.drmInfo);
#endif
			MmsReleaseMsgBody(pRemoveList->pBody, pRemoveList->type.type);

			free(pRemoveList->pBody);
			free(pRemoveList);
		}

		if (__MsgCopyNestedMsgType(pPartType, &(pSelectedPart->type)) == false) {
			MSG_DEBUG("MsgResolveNestedMultipart : MsgPriorityCopyMsgType failed \n");
			goto __CATCH;
		}

		if (pSelectedPart->pBody != NULL)
			memcpy(pPartBody, pSelectedPart->pBody, sizeof(MsgBody));

		if (pSelectedPart != NULL) {
#ifdef __SUPPORT_DRM__
			MmsReleaseMsgDRMInfo(&pSelectedPart->type.drmInfo);
#endif

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

		MSG_DEBUG("MsgResolveNestedMultipart : MIME_APPLICATION_VND_WAP_MULTIPART_RELATED\n");

		pSelectedPart = pPartBody->body.pMultipart;

		while (pSelectedPart) {
			if (__MsgIsMultipartMixed(pSelectedPart->type.type)) {

				if (pSelectedPart->pBody == NULL) {
					MSG_DEBUG("MsgResolveNestedMultipart :pSelectedPart->pBody(1) is NULL\n");
					break;
				}

				pFirstPart = pSelectedPart->pBody->body.pMultipart;

				if (pFirstPart == NULL) {
					MSG_DEBUG("MsgResolveNestedMultipart : multipart(RELATED) does not exist\n");
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
#ifdef __SUPPORT_DRM__
					MmsReleaseMsgDRMInfo(&pSelectedPart->type.drmInfo);
#endif
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

		MSG_DEBUG("MsgResolveNestedMultipart : MIME_APPLICATION_VND_WAP_MULTIPART_MIXED\n");

		pPrevPart = NULL;
		pSelectedPart = pPartBody->body.pMultipart;

		while (pSelectedPart) {
			if (MsgIsMultipart(pSelectedPart->type.type)) {
				if (pSelectedPart->pBody == NULL) {
					MSG_DEBUG("MsgResolveNestedMultipart :pSelectedPart->pBody(2) is NULL\n");
					break;
				}

				pFirstPart = pSelectedPart->pBody->body.pMultipart;

				// NULL Pointer check!!
				if (pFirstPart == NULL) {
					MSG_DEBUG("MsgResolveNestedMultipart : multipart does not exist\n");
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

#ifdef __SUPPORT_DRM__
				MmsReleaseMsgDRMInfo(&pSelectedPart->type.drmInfo);
#endif
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

		MSG_DEBUG("MsgResolveNestedMultipart : MIME_MULTIPART_REPORT \n");

		pTmpMultipart = pPartBody->body.pMultipart;
		pPrevPart = NULL;

		if (pTmpMultipart == NULL) {
			MSG_DEBUG("MsgResolveNestedMultipart : pTmpMultipart == NULL \n");
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
			MSG_DEBUG("MsgResolveNestedMultipart : MIME_MULTIPART_REPORT [no selected part]\n");

			pRemoveList = pPartBody->body.pMultipart->pNext;
			if (pPartBody->body.pMultipart != NULL) {
				pSelectedPart = pPartBody->body.pMultipart;
				pSelectedPart->pNext = NULL;
			}
		} else {
			if (pPrevPart == NULL) {
				// first part is selected
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
#ifdef __SUPPORT_DRM__
			MmsReleaseMsgDRMInfo(&pTmpMultipart->type.drmInfo);
#endif
			MmsReleaseMsgBody(pTmpMultipart->pBody, pTmpMultipart->type.type);
			pNextRemovePart = pTmpMultipart->pNext;

			free(pTmpMultipart->pBody);
			free(pTmpMultipart);
			pTmpMultipart = pNextRemovePart;
		}

		if (__MsgCopyNestedMsgType(pPartType, &(pSelectedPart->type)) == false) {
			MSG_DEBUG("MsgResolveNestedMultipart : MsgPriorityCopyMsgType failed \n");
			goto __CATCH;
		}

		if (pSelectedPart != NULL) {

			if (pSelectedPart->pBody != NULL)
				memcpy(pPartBody, pSelectedPart->pBody, sizeof(MsgBody));

#ifdef __SUPPORT_DRM__
			MmsReleaseMsgDRMInfo(&pSelectedPart->type.drmInfo);
#endif
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

	szTemp = (char *)malloc(length);
	if (szTemp == NULL) {
		MSG_DEBUG("MsgResolveContentURI: memory full\n");
		goto __CATCH;
	}

	memset(szTemp, 0, length);
	strcpy(szTemp, szSrc);

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
	int cLen = 0;	// length of pBuff
	char *pBuff = NULL;

	if (pSrc == NULL) {
		MSG_DEBUG("MsgRemoveQuoteFromFilename: pSrc is Null\n");
		return NULL;
	}

	cLen = strlen(pSrc);

	pBuff = (char *)malloc(cLen + 1);

	if (pBuff == NULL) {
		MSG_DEBUG("MsgRemoveQuoteFromFilename: pBuff mem alloc fail!\n");
		return NULL;
	}
	memset(pBuff, 0 , sizeof(char)*(cLen + 1));

	// remove front quote
	if (pSrc[0] == MSG_CH_QUOT) {
		cLen--;
		strncpy(pBuff, &pSrc[1], cLen);
		pBuff[cLen] = '\0';
	}

	if (pSrc[0] == MSG_CH_LF) {
		cLen--;
		strncpy(pBuff, &pSrc[1], cLen);
	} else {
		strcpy(pBuff, pSrc);
	}

	// remove last qoute
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
	if(!pMsgType1 || !pMsgType2)
		return false;

	if (pMsgType1->section == INVALID_HOBJ)
		pMsgType1->section = pMsgType2->section;

#ifdef __SUPPORT_DRM__
	int		length = 0;

	if (pMsgType1->drmInfo.drmType == MSG_DRM_TYPE_NONE)
		pMsgType1->drmInfo.drmType = pMsgType2->drmInfo.drmType;


	if (pMsgType1->szContentID[0] == '\0') {
		strcpy(pMsgType1->szContentID, pMsgType2->szContentID);

		if (pMsgType2->szContentID[0]) {
			length = MsgStrlen(pMsgType2->szContentID);
			if (pMsgType2->szContentID[0] == '<' && pMsgType2->szContentID[length - 1] == '>') {
				pMsgType1->drmInfo.szContentURI = MsgStrNCopy(pMsgType2->szContentID + 1, length - 2);
			} else {
				pMsgType1->drmInfo.szContentURI = MsgStrCopy(pMsgType2->szContentID);
			}
		}
	} else {
		length = MsgStrlen(pMsgType1->szContentID);
		if (pMsgType1->szContentID[0] == '<' && pMsgType1->szContentID[length - 1] == '>') {
			pMsgType1->drmInfo.szContentURI = MsgStrNCopy(pMsgType1->szContentID + 1, length - 2);
		} else {
			pMsgType1->drmInfo.szContentURI = MsgStrCopy(pMsgType1->szContentID);
		}
	}
#endif

	if (pMsgType1->szContentLocation[0] == '\0')
		strcpy(pMsgType1->szContentLocation, pMsgType2->szContentLocation);

	/* Copy informations - we shoud open the pMsgType2's orgFile
	 * concerning its offset and size.
	 */
	if (pMsgType2->szOrgFilePath[0] != '\0')
		strcpy(pMsgType1->szOrgFilePath, pMsgType2->szOrgFilePath);

	if (pMsgType2->disposition != INVALID_HOBJ)
		pMsgType1->disposition = pMsgType2->disposition;

	if ((pMsgType1->type != MIME_APPLICATION_VND_OMA_DRM_MESSAGE && pMsgType1->type != MIME_APPLICATION_VND_OMA_DRM_CONTENT) &&
		 pMsgType2->encoding != INVALID_HOBJ)
		pMsgType1->encoding = pMsgType2->encoding;

	pMsgType1->contentSize = pMsgType2->contentSize;
	pMsgType1->offset = pMsgType2->offset;
	pMsgType1->size = pMsgType2->size;
	pMsgType1->type = pMsgType2->type;

	__MsgCopyNestedMsgParam(&(pMsgType1->param), &(pMsgType2->param));

	if (pMsgType1->param.szName[0]) {
#ifdef __SUPPORT_DRM__
		pMsgType1->drmInfo.szContentName = MsgStrCopy(pMsgType2->param.szName);
#endif
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
	if (pParam2->szBoundary[0] !='\0')
		strcpy(pParam1->szBoundary, pParam2->szBoundary);

	if (pParam1->szFileName[0] =='\0')
		strcpy(pParam1->szFileName, pParam2->szFileName);

	if (pParam1->szName[0] =='\0')
		strcpy(pParam1->szName, pParam2->szName);

	if (pParam1->szStart[0] =='\0')
		strcpy(pParam1->szStart, pParam2->szStart);

	if (pParam1->szStartInfo[0] =='\0')
		strcpy(pParam1->szStartInfo, pParam2->szStartInfo);

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

bool MmsGetMsgAttrib(MmsMsgID msgID, MmsAttrib* pAttrib)
{
	MmsMsg *pMsg = NULL;

	memset(pAttrib, 0, sizeof(MmsAttrib));
	MmsPluginStorage::instance()->getMmsMessage(&pMsg);
	memcpy(pAttrib, &(pMsg->mmsAttrib), sizeof(MmsAttrib));

	MSG_DEBUG("MmsGetMsgAttrib: msgID = %lu ---------------------\n", msgID);

	if ('\0' != pMsg->szTrID[0])
		MSG_DEBUG("szTrID = %s \n", pMsg->szTrID);

	MSG_END();
	return true;
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

bool MmsDataUpdateLastStatus(MmsMsg *pMsg)
{
	MmsMsgMultiStatus*	pStatus = NULL;

	pStatus = pMsg->mmsAttrib.pMultiStatus;

	while (pStatus != NULL) {
		pStatus->bDeliveyrReportIsLast = false;
		pStatus->bReadReplyIsLast = false;
		pStatus = pStatus->pNext;
	}

	return true;
}


bool MmsAddrUtilCompareAddr(char *pszAddr1, char *pszAddr2)
{
	int len1;
	int len2;
	char *p;

	MmsAddrUtilRemovePlmnString(pszAddr1);
	MmsAddrUtilRemovePlmnString(pszAddr2);

	MSG_DEBUG("##### pszAddr1 = %s #####", pszAddr1);
	MSG_DEBUG("##### pszAddr2 = %s #####", pszAddr2);
	if (!strcmp(pszAddr1, pszAddr2))
		return true;

	len1 = strlen(pszAddr1);
	len2 = strlen(pszAddr2);

	if (len1 > len2) {
		p = strstr(pszAddr1, pszAddr2);
	} else {
		p = strstr(pszAddr2, pszAddr1);
	}

	if (p)
		return true;

	return false;
}

static int __MsgGetLatin2UTFCodeSize(unsigned char *szSrc, int nChar)
{
	int nCount = 0;

	MSG_DEBUG("MsgGetLatin2UTFCodeSize: --------------- \n");

	if ((szSrc == NULL) || (nChar <= 0)) {
		MSG_DEBUG("MsgGetLatin2UTFCodeSize: szSrc is NULL !!!! --------------- \n");
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

static int __MsgLatin5code2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar)
{
	unsigned char *org;
	unsigned char t1;
	unsigned char t2;
	unsigned short temp = 0;

	org = des;
	outBufSize--;	//Null Character

	while ((nChar > 0) && (*szSrc != '\0')) {

		if (*szSrc >= 0x01 && *szSrc <= 0x7F) { //basic common
			temp = (unsigned short)(*szSrc);

			outBufSize --;
			if (outBufSize < 0)
				goto __RETURN;

			*des = (unsigned char) ((*szSrc) & 0x007F);

			des++;
			szSrc++;
			nChar--;
		} else if ((*szSrc == 0x00) || (*szSrc >= 0x80 && *szSrc <= 0x9F) ||
					(*szSrc >= 0xA0 && *szSrc <= 0xCF) || (*szSrc >= 0xD1 && *szSrc <= 0xDC) ||
					(*szSrc >= 0xDF && *szSrc <= 0xEF) || (*szSrc >= 0xF1 && *szSrc <= 0xFC) ||
					(*szSrc == 0xFF)) {//uni 0x00A0 ~ 0x00CF

			temp = (unsigned short)(*szSrc);

			outBufSize -= 2;
			if (outBufSize < 0)
				goto __RETURN;

			t2 = (unsigned char) (temp & 0x003F);				//	right most 6 bit
			t1 = (unsigned char) ((temp & 0x07C0) >> 6);		//	right most 5 bit

			*des = 0xC0 | (t1 & 0x1F);
			*(des+1) = 0x80 | (t2 & 0x3F);

			des += 2;
			szSrc += 1;
			nChar -= 1;
		} else if (*szSrc == 0xD0) {//empty section OR vendor specific codes.

			temp = 0x011E;

			outBufSize -= 2;
			if (outBufSize < 0)
				goto __RETURN;

			t2 = (unsigned char) (temp & 0x003F);				//	right most 6 bit
			t1 = (unsigned char) ((temp & 0x07C0) >> 6);		//	right most 5 bit

			*des = 0xC0 | (t1 & 0x1F);
			*(des+1) = 0x80 | (t2 & 0x3F);

			des += 2;
			szSrc += 1;
			nChar -= 1;
		} else if (*szSrc == 0xDD) {
			temp = 0x0130;

			outBufSize -= 2;
			if (outBufSize < 0)
				goto __RETURN;

			t2 = (unsigned char) (temp & 0x003F);				//	right most 6 bit
			t1 = (unsigned char) ((temp & 0x07C0) >> 6);		//	right most 5 bit

			*des = 0xC0 | (t1 & 0x1F);
			*(des+1) = 0x80 | (t2 & 0x3F);

			des += 2;
			szSrc += 1;
			nChar -= 1;
		} else if (*szSrc == 0xDE) {
			temp = 0x015E;

			outBufSize -= 2;
			if (outBufSize < 0)
				goto __RETURN;

			t2 = (unsigned char) (temp & 0x003F);				//	right most 6 bit
			t1 = (unsigned char) ((temp & 0x07C0) >> 6);		//	right most 5 bit

			*des = 0xC0 | (t1 & 0x1F);
			*(des+1) = 0x80 | (t2 & 0x3F);

			des += 2;
			szSrc += 1;
			nChar -= 1;
		} else if (*szSrc == 0xF0) {
			temp = 0x011F;
				outBufSize -= 2;
			if (outBufSize < 0)
				goto __RETURN;

			t2 = (unsigned char) (temp & 0x003F);				//	right most 6 bit
			t1 = (unsigned char) ((temp & 0x07C0) >> 6);		//	right most 5 bit

			*des = 0xC0 | (t1 & 0x1F);
			*(des+1) = 0x80 | (t2 & 0x3F);

			des += 2;
			szSrc += 1;
			nChar -= 1;
		} else if (*szSrc == 0xFD) {
			temp = 0x0131;

			outBufSize -= 2;

			if (outBufSize < 0)
				goto __RETURN;

			t2 = (unsigned char) (temp & 0x003F);				//	right most 6 bit
			t1 = (unsigned char) ((temp & 0x07C0) >> 6);		//	right most 5 bit

			*des = 0xC0 | (t1 & 0x1F);
			*(des+1) = 0x80 | (t2 & 0x3F);

			des += 2;
			szSrc += 1;
			nChar -= 1;
		} else if (*szSrc == 0xFE) {
			temp = 0x015F;

			outBufSize -= 2;
			if (outBufSize < 0)
				goto __RETURN;

			t2 = (unsigned char) (temp & 0x003F);				//	right most 6 bit
			t1 = (unsigned char) ((temp & 0x07C0) >> 6);		//	right most 5 bit

			*des = 0xC0 | (t1 & 0x1F);
			*(des+1) = 0x80 | (t2 & 0x3F);

			des += 2;
			szSrc += 1;
			nChar -= 1;
		} else {
			return -1;
		}
	}
__RETURN:
	*des = 0;
	return(des-org);
}

static int __MsgGetLatin52UTFCodeSize(unsigned char *szSrc, int nChar)
{
	int nCount = 0;

	MSG_DEBUG("MsgGetLatin52UTFCodeSize: --------------- \n");

	if ((szSrc == NULL) || (nChar <= 0))
		return 0;

	while ((nChar > 0) && (*szSrc != '\0')) {
		if (*szSrc >= 0x01 && *szSrc <= 0x7F) {
			nCount += 1;
			szSrc++;
			nChar--;
		} else if (*szSrc == 0x00 || (*szSrc >= 0x80 && *szSrc <= 0x9F) ||
					(*szSrc >= 0xA0 && *szSrc <= 0xCF) || (*szSrc >= 0xD1 && *szSrc <= 0xDC) |
					(*szSrc >= 0xDF && *szSrc <= 0xEF) || (*szSrc >= 0xF1 && *szSrc <= 0xFC) ||
					*szSrc == 0xD0 || *szSrc == 0xDD || *szSrc == 0xDE || *szSrc == 0xF0 ||
					*szSrc == 0xFD || *szSrc == 0xFE	|| *szSrc == 0xFF) { //uni 0x00A0 ~ 0x00CF
			nCount += 2;
			szSrc++;
			nChar--;
		} else {
			return -1;
		}
	}
	return nCount;
}

static int __MsgLatin2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar)
{
	unsigned char*	org;
	unsigned char	t1, t2;

	MSG_DEBUG("MsgLatin2UTF: --------------- \n");

	org = des;
	outBufSize--;			// NULL character

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

			t2 = (unsigned char) (*szSrc & 0x003F);				//	right most 6 bit
			t1 = (unsigned char) ((*szSrc & 0xC0) >> 6);		//	right most 2 bit

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


static int __MsgLatin7code2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar)
{
	unsigned char *org;
	unsigned char t1;
	unsigned char t2;
	unsigned char t3;
	unsigned short temp = 0;

	MSG_DEBUG("MsgUnicode2UTF: --------------- \n");

	org = des;
	outBufSize--;	//Null Character

	while ((nChar > 0) && (*szSrc != '\0')) {
		if (*szSrc >= 0x01 && *szSrc <= 0x7F) {
			temp = (unsigned short)(*szSrc);

			outBufSize --;
			if (outBufSize < 0)
				goto __RETURN;

			*des = (unsigned char) (temp & 0x007F);

			des++;
			szSrc++;
			nChar--;

		} else if ((*szSrc == 0x00) || (*szSrc >= 0x80 && *szSrc <= 0x9F) ||
					(*szSrc >= 0xA3 && *szSrc <= 0xAD) || (*szSrc == 0xBB)) { // consider 0xA4, 0xA5

			temp = (unsigned short)(*szSrc);

			outBufSize -= 2;
			if (outBufSize < 0)
				goto __RETURN;

			t2 = (unsigned char) (temp & 0x003F);				//	right most 6 bit
			t1 = (unsigned char) ((temp & 0xC0) >> 6);		//	right most 2 bit

			*des = 0xC0 | (t1 & 0x1F);
			*(des + 1) = 0x80 | (t2 & 0x3F);

			des += 2;
			szSrc += 1;
			nChar -= 1;
		} else if (*szSrc == 0xA0) {
			temp = 0x0020;
			//*des = temp to utf-8
			outBufSize--;
			if (outBufSize < 0)
				goto __RETURN;

			*des = (unsigned char) (temp & 0x007F);

			des++;
			szSrc++;
			nChar--;

		} else if (*szSrc == 0xA1) {
			temp = 0x2018;

			outBufSize -= 3;
			if (outBufSize < 0)
				goto __RETURN;

			t3 = (unsigned char) (temp & 0x003F);					//	right most 6 bit
			t2 = (unsigned char) ((temp & 0x0FC0) >> 6);			//	right most 6 bit
			t1 = (unsigned char) ((temp & 0xF000) >> 12);			//	right most 4 bit

			*des = 0xE0 | (t1 & 0x0F);
			*(des+1) = 0x80 | (t2 & 0x3F);
			*(des+2) = 0x80 | (t3 & 0x3F);

			des += 3;
			szSrc += 1;
			nChar -= 1;

		} else if (*szSrc == 0xA2) {
			temp = 0x2019;

			outBufSize -= 3;
			if (outBufSize < 0)
				goto __RETURN;

			t3 = (unsigned char) (temp & 0x003F);					//	right most 6 bit
			t2 = (unsigned char) ((temp & 0x0FC0) >> 6);			//	right most 6 bit
			t1 = (unsigned char) ((temp & 0xF000) >> 12);			//	right most 4 bit

			*des = 0xE0 | (t1 & 0x0F);
			*(des+1) = 0x80 | (t2 & 0x3F);
			*(des+2) = 0x80 | (t3 & 0x3F);

			des += 3;
			szSrc += 1;
			nChar -= 1;

		} else if (*szSrc == 0xAF) {
			temp = 0x2015;

			outBufSize -= 3;
			if (outBufSize < 0)
				goto __RETURN;

			t3 = (unsigned char) (temp & 0x003F);					//	right most 6 bit
			t2 = (unsigned char) ((temp & 0x0FC0) >> 6);			//	right most 6 bit
			t1 = (unsigned char) ((temp & 0xF000) >> 12);			//	right most 4 bit

			*des = 0xE0 | (t1 & 0x0F);
			*(des+1) = 0x80 | (t2 & 0x3F);
			*(des+2) = 0x80 | (t3 & 0x3F);

			des += 3;
			szSrc += 1;
			nChar -= 1;

		} else if (0xB0 <= *szSrc && *szSrc <= 0xB4) { //0x00B0 ~ 0x00B4

			temp = (unsigned short)(*szSrc);

			outBufSize -= 2;
			if (outBufSize < 0)
				goto __RETURN;

			t2 = (unsigned char) (temp & 0x003F);				//	right most 6 bit
			t1 = (unsigned char) ((temp & 0x07C0) >> 6);		//	right most 5 bit

			*des = 0xC0 | (t1 & 0x1F);
			*(des+1) = 0x80 | (t2 & 0x3F);

			des += 2;
			szSrc += 1;
			nChar -= 1;

		} else if ((0xB5 <= *szSrc &&  *szSrc <= 0xBA) ||
				(0xBC <= *szSrc && *szSrc <= 0xD1) ||
				(0xD3 <= *szSrc && *szSrc <= 0xFE)) {
			temp= (unsigned short)(*szSrc + 0x02D0);

			outBufSize -= 2;
			if (outBufSize < 0)
				goto __RETURN;

			t2 = (unsigned char) (temp & 0x003F);				//	right most 6 bit
			t1 = (unsigned char) ((temp & 0x07C0) >> 6);		//	right most 5 bit

			*des = 0xC0 | (t1 & 0x1F);
			*(des+1) = 0x80 | (t2 & 0x3F);

			des += 2;
			szSrc += 1;
			nChar -= 1;

		} else {
			return -1;
		}

	}

__RETURN:
	*des = 0;
	return(des - org);
}

static int __MsgGetLatin72UTFCodeSize(unsigned char *szSrc, int nChar)
{
	int nCount = 0;

	MSG_DEBUG("MsgGetLatin72UTFCodeSize: --------------- \n");

	if ((szSrc == NULL) || (nChar <= 0))
		return 0;

	while ((nChar > 0) && (*szSrc != '\0')) {

		if ((*szSrc >= 0x01 && *szSrc <= 0x7F) || (*szSrc == 0xA0)) {
			nCount += 1;
			szSrc++;
			nChar--;
		} else if (*szSrc == 0x00 || (0x80 <= *szSrc && *szSrc <= 0x9F) || (0xA3 <= *szSrc && *szSrc <= 0xAD) ||
					(0xB0 <= *szSrc && *szSrc <= 0xB4) || (0xB5 <= *szSrc && *szSrc <= 0xFE)) {
			nCount += 2;
			szSrc++;
			nChar--;
		} else if (*szSrc == 0xA1 ||*szSrc == 0xA2 || *szSrc == 0xAF) {
			nCount += 3;
			szSrc += 1;
			nChar -= 1;

		} else {
			return -1;
		}
	}
	return nCount;
}

static int __MsgUnicode2UTF(unsigned char *des, int outBufSize, unsigned short *szSrc, int nChar)
{
	unsigned char *org;
	unsigned char t1;
	unsigned char t2;
	unsigned char t3;

	MSG_DEBUG("MsgUnicode2UTF: --------------- \n");

	org = des;
	outBufSize--;			// NULL character

	while ((nChar > 0) && (*szSrc != '\0')) {
		if (0x0001 <= *szSrc && *szSrc <= 0x007F) {
			/* check outbuffer's room for this UTF8 character */

			outBufSize --;
			if (outBufSize < 0)
				goto __RETURN;

			*des = (unsigned char) (*szSrc & 0x007F);

			des++;
			szSrc++;
			nChar--;
		} else if  ((*szSrc == 0x0000) || (0x0080 <= *szSrc && *szSrc <= 0x07FF)) {
			/* check outbuffer's room for this UTF8 character */

			outBufSize -= 2;
			if (outBufSize < 0)
				goto __RETURN;

			t2 = (unsigned char) (*szSrc & 0x003F);				//	right most 6 bit
			t1 = (unsigned char) ((*szSrc & 0x07C0) >> 6);		//	right most 5 bit

			*des = 0xC0 | (t1 & 0x1F);
			*(des+1) = 0x80 | (t2 & 0x3F);

			des += 2;
			szSrc += 1;
			nChar -= 1;
		} else {
			/* check outbuffer's room for this UTF8 character */

			outBufSize -= 3;
			if (outBufSize < 0)
				goto __RETURN;

			t3 = (unsigned char) (*szSrc & 0x003F);					//	right most 6 bit
			t2 = (unsigned char) ((*szSrc & 0x0FC0) >> 6);			//	right most 6 bit
			t1 = (unsigned char) ((*szSrc & 0xF000) >> 12);			//	right most 4 bit

			*des = 0xE0 | (t1 & 0x0F);
			*(des+1) = 0x80 | (t2 & 0x3F);
			*(des+2) = 0x80 | (t3 & 0x3F);

			des += 3;
			szSrc += 1;
			nChar -= 1;
		}
	}

__RETURN:

	*des = 0;
	return (des - org);
}

static int __MsgGetUnicode2UTFCodeSize(unsigned short *szSrc, int nChar)
{
	int nCount = 0;

	MSG_DEBUG("MsgGetUnicode2UTFCodeSize: --------------- \n");

	if ((szSrc == NULL) || (nChar <= 0)) {
		MSG_DEBUG("MsgGetUnicode2UTFCodeSize: szSrc is NULL !!!! --------------- \n");
		return 0;
	}

	while ((nChar > 0) && (*szSrc != '\0')) {
		if (0x0001 <= *szSrc && *szSrc <= 0x007F) {
			nCount += 1;
			szSrc++;
			nChar--;
		} else if  ((*szSrc == 0x0000) || (0x0080 <= *szSrc && *szSrc <= 0x07FF)) {
			nCount += 2;
			szSrc++;
			nChar--;
		} else {
			nCount += 3;
			szSrc++;
			nChar--;
		}
	}

	return nCount;
}

static bool __MmsAddrUtilCheckEmailAddress(char *pszAddr)
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
		MSG_DEBUG("MmsAddrUtilRemovePlmnString: pszAddr is null or zero\n");
		return false;
	}

	strLen = strlen(pszAddr);

	pszAddrCopy = (char*)calloc(1,strLen + 1);
	if (!pszAddrCopy) {
		MSG_DEBUG("MmsAddrUtilRemovePlmnString: pszAddrCopy is NULL, mem alloc failed\n");
		return false;
	}

	strcpy(pszAddrCopy, pszAddr);


	pszAddr[0] = 0;
	pszStrStart = pszAddrCopy;

	while (true) {
		char*	pszStrEnd = NULL;
		int		addressLen = 0;

		if (__MmsAddrUtilCheckEmailAddress(pszAddrCopy))
			pszStrEnd = strstr(pszStrStart, "/TYPE=PLMN");
		else
			pszStrEnd = strstr(pszStrStart, "/");

		if (!pszStrEnd) {
			// "/TYPE=PLMN" not found

			int remainedLen = strlen(pszStrStart);

			if (remainedLen <= 0)
				break;

			strcat(pszAddr, pszStrStart);

			break;
		}

		// Get one address length
		addressLen = pszStrEnd - pszStrStart;

		strncat(pszAddr, pszStrStart, addressLen);

		// Find next address
		pszStrStart = pszStrEnd;

		pszStrTemp = strstr(pszStrStart, MSG_MMS_STR_ADDR_DELIMETER);

		if (pszStrTemp) {
			addressLen = pszStrTemp - pszStrEnd;
			pszStrStart += addressLen;
		} else {
			pszStrStart += strlen(pszStrEnd);
		}

		if (pszStrStart[0] == 0)	// end of string
			break;


		strcat(pszAddr, MSG_MMS_STR_ADDR_DELIMETER);	// add ';'
		pszStrStart++;	// remove ';'
	}

	if (pszAddr[0] == 0)
		strcpy(pszAddr, pszAddrCopy);

	free(pszAddrCopy);

	return true;
}

static int __MsgCutUTFString(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar)
{
	unsigned char *org;

	MSG_DEBUG("MsgCutUTFString: --------------- \n");

	org = des;
	outBufSize--;			// NULL character

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
			MSG_DEBUG("MsgCutUTFString: utf8 incorrect range!\n");
		}

		nChar--;
	}

__RETURN:

	*des = 0;
	return (des - org);
}

static void __MsgMIMERemoveQuote(char *szSrc)
{
	int		length = 0;

	length = MsgStrlen(szSrc);
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

	MSG_DEBUG("MsgLoadDataToDecodeBuffer: \n");

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


bool MsgGetTypeByFileName(int *type, char *szFileName)
{
	char *pExt   = NULL;
	AvCodecType AvType = AV_CODEC_NONE;

	pExt = strrchr(szFileName, '.');
	if (pExt == NULL || pExt[0] == '\0')
		goto __CATCH;

	pExt++;

	if (strcasecmp(pExt, "mp4") == 0 ||strcasecmp(pExt, "mpeg4") == 0 ||strcasecmp(pExt, "3gp") == 0 ||strcasecmp(pExt, "3gpp") == 0) {

		if (szFileName[0] != '/')
			goto __CATCH;

		AvType = AvGetFileCodecType(szFileName);
		MSG_DEBUG("MsgGetTypeByFileName:AvType(0x%x)\n", AvType);

		switch (AvType)	{
		case AV_DEC_AUDIO_MPEG4:
			*type = MIME_AUDIO_MP4;
			break;

		case AV_DEC_VIDEO_MPEG4:
			*type = MIME_VIDEO_MP4;
			break;

		default:
			*type = MIME_VIDEO_3GPP;
			break;
		}
		return true;
	}

	if (strcasecmp(pExt, "amr") == 0) {
		*type = MIME_AUDIO_AMR;
		return true;
	} else if ((strcasecmp(pExt, "mid") == 0) || (strcasecmp(pExt, "midi") == 0)) {
		*type = MIME_AUDIO_MIDI;
		return true;
	} else if (strcasecmp(pExt, "imy") == 0) {
		*type = MIME_TEXT_X_IMELODY;
		return true;
	}

	*type = MimeGetMimeFromExtInt((const char*)pExt);
	MSG_DEBUG("MsgGetTypeByFileName: szFileName = %s     type = %d \n", szFileName, type);
	return true;


__CATCH:

	*type = MIME_UNKNOWN;
	MSG_DEBUG("MsgGetTypeByFileName: szFileName = %s     type = %d \n", szFileName, type);

	return false;

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
	char szFileName[MSG_FILENAME_LEN_MAX+1] = {0, };	// file name of temp file
	char szFullPath[MSG_FILEPATH_LEN_MAX] = {0, }; // full absolute path of temp file.

	MSG_DEBUG("****   _MmsSaveMediaData:  [Multi part]  START  ***\n");

	if (!pPartType) {
		MSG_DEBUG("pPartType is NULL\n");
		return true; // why true value is retured ??? ; false;
	}

	if (pPartType->param.szName[0] == '\0' && pPartType->param.szFileName[0] == '\0')
		snprintf(pPartType->param.szName, sizeof(pPartType->param.szName), "%s", pPartType->param.szFileName);

	if (pPartType->param.szName[0] != '\0') {
		snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%s", pPartType->param.szName);
	} else {
		snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%lu", (unsigned long)index);
	}


#ifndef __SUPPORT_DRM__
	__MsgMakeFileName(pPartType->type, szFileName, 0);	//FL & CD -> extension(.dm) SD -> extension(.dcf)
#else
	__MsgMakeFileName(pPartType->type, szFileName, pPartType->drmInfo.drmType, 0);	//FL & CD -> extension(.dm) SD -> extension(.dcf)
	if (MsgDRMIsForwardLockType(pPartType->drmInfo.drmType))
		MsgChangeDrm2FileName(szFileName);
#endif


	snprintf(szFullPath, MSG_FILEPATH_LEN_MAX, "%s%s.dir/%s", pszMailboxPath, pszMsgFilename, szFileName);	// get absolute path of each temp file of each part
	snprintf(pPartType->param.szFileName, MSG_FILENAME_LEN_MAX+1, "%s.dir/%s", pszMsgFilename, szFileName);		// store relative path of each temp file of each part including sub folder.

	if (pPartType->type == MIME_APPLICATION_OCTET_STREAM)
		MsgGetTypeByFileName(&pPartType->type, szFullPath);

	// save file
	if (bSave) {
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
		if (pPartType->drmInfo.drmType != MSG_DRM_TYPE_NONE) {
			MsgDrmRegisterFile(MSG_MODE_FILE, szFullPath, strlen(szFullPath));

			/* change szDrm2FullPath as current content path*/
			if (pPartType->drmInfo.szDrm2FullPath) {
				free(pPartType->drmInfo.szDrm2FullPath);
				pPartType->drmInfo.szDrm2FullPath = MsgStrCopy(szFullPath);
			}
		}

	}
	MSG_DEBUG("****   MmsGetMediaPartData:  [Multi part]  E  N  D  (Successfully)  ***\n");

	return true;

__CATCH:

	if (pFile != NULL) {
		MsgCloseFile(pFile);
		pFile = NULL;
	}

	return false;
}

static bool __MmsGetMediaPartData(MsgType *pPartType, MsgBody *pPartBody, FILE* pFile)
{
	int nRead = 0;
	int nRead2 = 0;
	char *pData = NULL;
	char *pNewData = NULL;
	char *pTempData = NULL;
	int msgEncodingValue = 0;
	int msgTypeValue = 0;
	int msgCharsetValue	= 0;
	int cidLen = 0;
	char *szCid = NULL;
	int offset = 0;
	int size = 0;

	msgEncodingValue = pPartType->encoding;
	msgTypeValue = pPartType->type;
	msgCharsetValue = pPartType->param.charset;

	cidLen = MsgStrlen(szCid);

	offset = pPartBody->offset;
	size = pPartBody->size;

	if (pPartBody->szOrgFilePath[0]) {
		pTempData = MsgOpenAndReadMmsFile(pPartBody->szOrgFilePath, offset, size, &nRead);

		if (pTempData == NULL) {
			MSG_DEBUG("MmsGetMediaPartData : pTempData read fail\n");
			goto __CATCH;
		}

		pData = pTempData;
	} else if (pPartBody->body.pText) {
		pData = pPartBody->body.pText;
		nRead = pPartBody->size;
	}

	if (pData == NULL) {
		MSG_DEBUG("MmsGetMediaPartData : there is no data \n");
		goto __RETURN;
	}

	pNewData = __MmsGetBinaryUTF8Data(pData, nRead, msgEncodingValue, msgTypeValue, msgCharsetValue, &nRead2);
	pPartType->encoding = MSG_ENCODING_BINARY;

	if (__MsgIsText(msgTypeValue))
		pPartType->param.charset = MSG_CHARSET_UTF8;

	if (MsgWriteFile(pNewData, sizeof(char), nRead2,  pFile) != (size_t)nRead2) {
		MSG_DEBUG("MmsGetMediaPartData: file writing fail \n");

		goto __CATCH;
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

static char *__MmsGetBinaryUTF8Data(char *pData, int nRead, int msgEncodingValue, int msgTypeValue, int msgCharsetValue, int *npRead)
{
	int nChar = 0;
	int nByte = 0;
	int nTemp = 0;
	char *pTemp = NULL;
	unsigned short *mszTempStr = NULL;
	char *pConvertedStr	= NULL;
	char *pConvertedData = NULL;
	char *pNewData = NULL;
	char *pReturnData = NULL;


	switch (msgEncodingValue) {
	case MSG_ENCODING_BASE64:

		pConvertedData = (char*)MsgDecodeBase64((UCHAR*)pData, (ULONG)nRead, (ULONG*)&nByte);
		MSG_DEBUG("MmsGetBinaryUTF8Data : MSG_ENCODING_BASE64     bodyLength = %d \n", nByte);

		pTemp = pConvertedData;
		nTemp = nByte;

		break;

	case MSG_ENCODING_QUOTE_PRINTABLE:

		pConvertedData = (char*)MsgDecodeQuotePrintable((UCHAR*)pData, (ULONG)nRead, (ULONG*)&nByte);
		MSG_DEBUG("MmsGetBinaryUTF8Data: MSG_ENCODING_QUOTE_PRINTABLE     bodyLength = %d \n", nByte);

		pTemp = pConvertedData;
		nTemp = nByte;

		break;

	default:

		MSG_DEBUG("MmsGetBinaryUTF8Data: 8bit    OR    Binary   bodyLength = %d \n", nRead);

		pTemp = pData;
		nTemp = nRead;

		break;
	}

	if (__MsgIsText(msgTypeValue)) {
		/* charset converting */

		switch (msgCharsetValue) {
		case MSG_CHARSET_UTF16:
		case MSG_CHARSET_USC2:

			MSG_DEBUG("MmsGetBinaryUTF8Data: MSG_CHARSET_USC2 \n");

			if (((UINT8)pTemp[0]) == 0xFF && ((UINT8)pTemp[1]) == 0xFE) {
				nChar = (nTemp / 2 - 1);

				mszTempStr = (unsigned short*) malloc(nChar * sizeof(unsigned short));
				if (mszTempStr == NULL) {
					MSG_DEBUG("MmsGetBinaryUTF8Data : 1. Memory Full !!! \n");
					goto __CATCH;
				}

				memcpy(mszTempStr, ((unsigned short*)pTemp + 1), nChar * sizeof(unsigned short));

				nByte = __MsgGetUnicode2UTFCodeSize(((unsigned short*)pTemp + 1), nChar);
				if (nByte < 3)
					nByte = 3; //min value

				pConvertedStr = (char *)malloc(nByte + 1);
				if (pConvertedStr != NULL)
					__MsgUnicode2UTF ((unsigned char*)pConvertedStr, nByte + 1, mszTempStr, nChar);
			} else {
				nChar = (nTemp / 2);

				mszTempStr = (unsigned short*) malloc(nChar * sizeof(unsigned short));
				if (mszTempStr == NULL) {
					MSG_DEBUG("MmsGetBinaryUTF8Data: 2. Memory Full !!! \n");
					goto __CATCH;
				}

				memcpy(mszTempStr, ((unsigned short*)pTemp), nChar * sizeof(unsigned short));

				nByte = __MsgGetUnicode2UTFCodeSize(((unsigned short*)pTemp), nChar);

				pConvertedStr = (char *)malloc(nByte + 1);
				if (pConvertedStr)
					__MsgUnicode2UTF ((unsigned char*)pConvertedStr, nByte + 1, mszTempStr, nChar);
			}

			if (pConvertedStr != NULL)
				pNewData = pConvertedStr;

			*npRead = nByte + 1;

			break;

		case MSG_CHARSET_US_ASCII:

			MSG_DEBUG("MmsGetBinaryUTF8Data: MSG_CHARSET_US_ASCII \n");

			/* fall through */

		case MSG_CHARSET_UTF8:

			MSG_DEBUG("MmsGetBinaryUTF8Data: MSG_CHARSET_UTF8 or Others \n");

			// skip BOM (Byte Order Mark) bytes .. (Please refer to the http://www.unicode.org/faq/utf_bom.html#BOM)
			if (nTemp >= 3) {
				if (((UINT8)pTemp[0]) == 0xEF && ((UINT8)pTemp[1]) == 0xBB && ((UINT8)pTemp[2]) == 0xBF) {
					pTemp += 3;
					nTemp -= 3;
				}
			}
			pNewData = pTemp;
			*npRead = nTemp;

			break;

		case MSG_CHARSET_ISO_8859_7:

			/* Greek */

			MSG_DEBUG("MmsGetBinaryUTF8Data: MSG_CHARSET_ISO_8859_7 \n");

			nByte = __MsgGetLatin72UTFCodeSize((unsigned char*)pTemp, nTemp);
			pConvertedStr = (char *)malloc(nByte + 1);
			if (pConvertedStr)
				__MsgLatin7code2UTF((unsigned char*)pConvertedStr, nByte + 1, (unsigned char*)pTemp, nTemp);

			pNewData = pConvertedStr;
			*npRead = nByte + 1;

			break;

		case MSG_CHARSET_ISO_8859_9:

			/* Turkish */
			MSG_DEBUG("MmsGetBinaryUTF8Data: MSG_CHARSET_ISO_8859_9 \n");

			nByte = __MsgGetLatin52UTFCodeSize((unsigned char*)pTemp, nTemp);
			pConvertedStr = (char *)malloc(nByte + 1);
			if (pConvertedStr)
					__MsgLatin5code2UTF((unsigned char*)pConvertedStr, nByte + 1, (unsigned char*)pTemp, nTemp);

			pNewData = pConvertedStr;
			*npRead = nByte + 1;

			break;

		default:

			MSG_DEBUG("MmsGetBinaryUTF8Data: Other charsets \n");

			nByte = __MsgGetLatin2UTFCodeSize((unsigned char*)pTemp, nTemp);
			pConvertedStr = (char *)malloc(nByte + 1);
			if (pConvertedStr)
				__MsgLatin2UTF((unsigned char*)pConvertedStr, nByte + 1, (unsigned char*)pTemp, nTemp);

			pNewData = pConvertedStr;
			*npRead = nByte + 1;

			break;
		}
	} else {
		pNewData = pTemp;
		*npRead = nTemp;
	}

	pReturnData = (char *)malloc(*npRead);
	if (pReturnData == NULL) {
		MSG_DEBUG("MmsGetBinaryUTF8Data : pReturnData alloc fail. \n");

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

	if (mszTempStr) {
		free(mszTempStr);
		mszTempStr = NULL;
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

	if (mszTempStr) {
		free(mszTempStr);
		mszTempStr = NULL;
	}

	return NULL;
}

#ifndef __SUPPORT_DRM__
static bool __MsgMakeFileName(int iMsgType, char *szFileName, int nUntitleIndex)
{
	char szText[MSG_FILENAME_LEN_MAX+1]={0,};
	char szTemp[MSG_FILENAME_LEN_MAX+1]={0,};
	char szTempFileName[MSG_FILENAME_LEN_MAX+1]={0,};
	char *pExt = NULL;


	MSG_DEBUG("MsgMakeFileName: iMsgType = %d     szFileName = %s \n", iMsgType, szFileName);

	if (szFileName == NULL)
		return false;

	if (szFileName && (szFileName[0] != '\0')) {
		MsgGetFileNameWithoutExtension (szTempFileName, szFileName);

		pExt = strrchr(szTempFileName, '.');
		if (pExt == NULL) {
			memset  (szText, 0, MSG_FILENAME_LEN_MAX+1);
			strncpy(szText, szTempFileName, MSG_FILEPATH_LEN_MAX - 1);
			strcat(szText, ".");			// add '.'
		} else {
			memset  (szText, 0, MSG_FILENAME_LEN_MAX+1);
			strncpy(szText, szTempFileName, pExt+1 - szFileName);	// add '.'
		}
	} else {
		if (nUntitleIndex >= 1) {
			snprintf(szText, MSG_FILENAME_LEN_MAX+1, "%s_%d.", "Untitled", nUntitleIndex);
		} else {
			snprintf(szText, MSG_FILENAME_LEN_MAX+1, "%s.", "Untitled");
		}
	}

	if (iMsgType == MIME_APPLICATION_OCTET_STREAM) {
		MSG_DEBUG("MsgMakeFileName: unsupported MsgType\n");
		goto __CATCH;
	} else {
		int		nLen = 0;
		strncpy(szTemp, szText, MSG_FILENAME_LEN_MAX - 5);
		if (iMsgType == MIME_UNKNOWN || (pExt = MimeGetExtFromMimeInt((MimeType)iMsgType)) == NULL) {
			MSG_DEBUG("MsgMakeFileName: Failed to get extension of that mime data file. \n");
			goto __CATCH;
		}
		nLen = MSG_FILENAME_LEN_MAX - strlen(szTemp);
		strncat(szTemp, pExt, nLen);
	}


	strcpy(szFileName, szTemp);

	MSG_DEBUG("MsgMakeFileName: made szFileName = %s \n", szFileName);

	return true;

__CATCH:
	{
		char *p = NULL;
		p = strrchr(szText, '.');
		if (p != NULL)
			*p = 0;
		snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%s", szText);

		return false;
	}
}
#else
static bool __MsgMakeFileName(int iMsgType, char *szFileName, MsgDrmType drmType, int nUntitleIndex)
{
	char szText[MSG_FILENAME_LEN_MAX+1]={0,};
	char szTemp[MSG_FILENAME_LEN_MAX+1]={0,};
	char szTempFileName[MSG_FILENAME_LEN_MAX+1]={0,};
	char *pExt = NULL;

	MSG_DEBUG("MsgMakeFileName: iMsgType = 0x%x, drmType = %d, szFileName = %s \n", iMsgType, drmType, szFileName);

	if (szFileName == NULL)
		return false;

	if (szFileName && (szFileName[0] != '\0')) {
		MsgGetFileNameWithoutExtension (szTempFileName, szFileName);

		if (drmType != MSG_DRM_TYPE_NONE) {
			pExt = strrchr(szTempFileName, '.');
			if (pExt == NULL) {
				memset(szText, 0, MSG_FILENAME_LEN_MAX+1);
				strncpy(szText, szTempFileName, MSG_FILENAME_LEN_MAX - 1);
				strcat(szText, ".");			// add '.'
			} else {
				memset(szText, 0, MSG_FILENAME_LEN_MAX+1);
				strncpy(szText, szTempFileName, pExt+1 - szFileName);
			}
		} else {
			pExt = strrchr(szTempFileName, '.');
			if (pExt == NULL) {
				memset(szText, 0, MSG_FILENAME_LEN_MAX+1);
				strncpy(szText, szTempFileName, MSG_FILENAME_LEN_MAX - 1);
				strcat(szText, ".");
			} else  {
				return true;
			}
		}
	} else {
		if (nUntitleIndex >= 1) {
			snprintf(szText, MSG_FILENAME_LEN_MAX+1, "%s_%d.", "untitled", nUntitleIndex);
		} else {
			snprintf(szText, MSG_FILENAME_LEN_MAX+1, "%s.", "untitled");
		}
	}

	if (drmType == MSG_DRM_TYPE_SD) {
		strncpy(szTemp, szText, MSG_FILENAME_LEN_MAX - 5);
		strcat(szTemp, "dcf");
	} else if (MsgDRMIsForwardLockType(drmType)) {
		strncpy(szTemp, szText, MSG_FILENAME_LEN_MAX - 4);
		strcat(szTemp, "dm");
	} else {
		if (iMsgType == MIME_APPLICATION_OCTET_STREAM) {
			MSG_DEBUG("MsgMakeFileName: unsupported MsgType\n");
			goto __CATCH;
		} else {
			int		nLen = 0;
			strncpy(szTemp, szText, MSG_FILENAME_LEN_MAX - 5);
			//temporary commented to save file as original name.
			if (pExt == NULL) {

				if (iMsgType == MIME_UNKNOWN || (pExt = MimeGetExtFromMimeInt((MimeType)iMsgType)) == NULL) {
					MSG_DEBUG("MsgMakeFileName: Failed to get extension of that mime data file. \n");
					goto __CATCH;
				}
			}

			nLen = MSG_FILENAME_LEN_MAX - strlen(szTemp);
			strncat(szTemp, pExt, nLen);
		}
	}

	strcpy(szFileName, szTemp);

	MSG_DEBUG("MsgMakeFileName: made szFileName = %s \n", szFileName);

	return true;

__CATCH:
	{
		char *p = NULL;
		p = strrchr(szText, '.');
		if (p != NULL)
			*p = 0;
		snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%s", szText);

		return false;
	}
}
#endif

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

int MmsGetMediaPartCount(msg_message_id_t msgId)
{
	MmsMsg *pMsg;

	MmsPluginStorage::instance()->getMmsMessage(&pMsg);

	if (msgId != pMsg->msgID) {
		MSG_DEBUG("Invalid Message Id");
		return -1;
	}

	return pMsg->nPartCount;
}

bool MmsGetMediaPartHeader(int index, MsgType *pHeader)
{
	MmsMsg *pMsg = NULL;
	MsgMultipart *pPart = NULL;

	if (pHeader == NULL) {
		MSG_DEBUG("MmsGetMediaPartHeader: Invalid pHeader input. It's null \n");
		return false;
	}

	MmsPluginStorage::instance()->getMmsMessage(&pMsg);

	MmsInitMsgType(pHeader);


	/* Requires header of non-presentation */
	if (MsgIsMultipart(pMsg->msgType.type)) {
		MSG_DEBUG("MmsGetMediaPartHeader: Multipart header [index = %d] \n", index);

		pPart = pMsg->msgBody.body.pMultipart;

		while (pPart && index--)
			pPart = pPart->pNext;

		if (pPart == NULL) {
			MSG_DEBUG("MmsGetMediaPartHeader: There is no such msg part.\n");
			return false;
		}

		memcpy(pHeader, &pPart->type, sizeof(MsgType));
	} else {
		MSG_DEBUG("MmsGetMediaPartHeader: Requires singlepart header \n");
		memcpy(pHeader, &pMsg->msgType, sizeof(MsgType));
	}

	return true;
}

static bool __MmsDebugPrintMulitpartEntry(MsgMultipart *pMultipart, int index)
{
	MSG_DEBUG("------------------------------\n");
	MSG_DEBUG("%dth multipart info\n", index);
	MSG_DEBUG("header size=%d\n", pMultipart->type.size);
	MSG_DEBUG("body size=%d\n", pMultipart->type.contentSize);
	MSG_DEBUG("content type=%s\n", MmsDebugGetMimeType((MimeType)pMultipart->type.type));
	MSG_DEBUG("content ID=%s\n", pMultipart->type.szContentID);
	MSG_DEBUG("content location=%s\n", pMultipart->type.szContentLocation);

	if (pMultipart->type.type == MIME_TEXT_PLAIN) {
		MSG_DEBUG("text info\n");
		MSG_DEBUG("charset=%d\n", pMultipart->type.param.charset);
		MSG_DEBUG("text file name=%s\n", pMultipart->type.param.szName);
	}
#ifdef __SUPPORT_DRM__
	if (pMultipart->type.drmInfo.drmType != MSG_DRM_TYPE_NONE) {
		MSG_DEBUG("drm info\n");
		MSG_DEBUG("drm type=%d      (0: NONE 1: Fowward Lock, 2:Combined Delivery, 3: Separated Delivery)\n", pMultipart->type.drmInfo.drmType);
		MSG_DEBUG("drm content type=%s\n", MmsDebugGetMimeType((MimeType)pMultipart->type.drmInfo.contentType));
		MSG_DEBUG("drm content URI=%s\n", pMultipart->type.drmInfo.szContentURI);
		MSG_DEBUG("drm2FullPath=%s\n", pMultipart->type.drmInfo.szDrm2FullPath);
	}
#endif
	MSG_DEBUG("------------------------------\n");
	return true;
}
