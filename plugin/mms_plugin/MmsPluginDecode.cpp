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

#include<stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

#include "MsgDebug.h"
#include "MsgException.h"
#include "MmsPluginMessage.h"
#include "MmsPluginStorage.h"
#include "MsgUtilFile.h"
#include "MmsPluginCodec.h"
#include "MmsPluginDebug.h"
#include "MsgSettingTypes.h"
#include "MsgSettingHandler.h"
#include "MmsPluginInternal.h"
#include "MmsPluginAvCodec.h"
#include "MmsPluginStorage.h"
#include "MmsPluginSmil.h"
#ifdef __SUPPORT_DRM__
#include "MmsPluginDrm.h"
#include "MsgDrmWrapper.h"
#include "drm-service.h"
#endif


static bool _MmsBinaryDecodeGetBytes(FILE *pFile, char *szBuff, int bufLen, int totalLength);		/* bufLen < gMmsDecodeMaxLen */
static bool _MmsBinaryDecodeGetLongBytes(FILE *pFile, char *szBuff, int bufLen, int totalLength);	/* no bufLen limit */
static bool _MmsBinaryDecodeGetOneByte(FILE *pFile, UINT8 *pOneByte, int totalLength);

static int __MmsBinaryDecodeUintvar(FILE *pFile, UINT32 *pUintVar, int totalLength);
static int __MmsDecodeValueLength(FILE *pFile, UINT32 *pValueLength, int totalLength);
static int __MmsDecodeValueLength2(FILE *pFile, UINT32 *pValueLength, int totalLength);
static int __MmsBinaryDecodeQuotedString(FILE *pFile, char *szBuff, int bufLen, int totalLength);
static bool __MmsBinaryDecodeEncodedString(FILE *pFile, char *szBuff, int bufLen, int totalLength);
static bool __MmsBinaryDecodeInteger(FILE *pFile, UINT32 *pInteger, int *pIntLen, int totalLength);
static bool __MmsBinaryDecodeCharset(FILE *pFile, UINT32 *nCharSet, int *pCharSetLen, int totalLength);
static char *__MmsBinaryDecodeText2(FILE *pFile, int totalLength, int *pLength);
static int __MmsBinaryDecodeText(FILE *pFile, char *szBuff, int bufLen, int totalLength);
static UINT32 __MmsHeaderDecodeIntegerByLength(FILE *pFile, UINT32 length, int totalLength);
static bool __MmsDecodeLongInteger(FILE *pFile, UINT32 *pLongInteger, int totalLength);
static int __MmsDecodeGetFilename(FILE *pFile, char *szBuff, int bufLen, int totalLength);
static MsgHeaderAddress *__MmsDecodeEncodedAddress(FILE *pFile, int totalLength);

#ifdef __SUPPORT_DRM__
bool __MmsParseDCFInfo(FILE *pFile, MsgDRMInfo *pDrmInfo, int totalLength);
bool __MmsParseDCFHdr(FILE *pFile, MsgDRMInfo *pDrmInfo, UINT32 headerLen, int totalLength);
bool MmsBinaryDecodeDRMMessage(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, unsigned int fullBodyLength, int totalLength);
bool MmsBinaryDecodeDRMContent(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, unsigned int bodyLength, int totalLength);
#endif

static bool MmsBinaryDecodeMovePointer(FILE *pFile, int offset, int totalLength);
bool MmsBinaryIsTextEncodedPart(FILE *pFile, int totalLength);
static bool	__MmsBinaryDecodeCheckAndDecreaseLength(int *pLength, int valueLength);

bool __MmsTextDecodeMsgHeader(FILE *pFile);
bool MmsTextDecodeMsgBody(FILE *pFile);

char gszMmsLoadBuf1[MSG_MMS_DECODE_BUFFER_MAX + 1] = {0, };
char gszMmsLoadBuf2[MSG_MMS_DECODE_BUFFER_MAX + 1] = {0, };

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

	(MSG_DELIVERY_REPORT_STATUS_T)MSG_DELIVERY_REPORT_NONE,		//MmsMsgStatus			iMsgStatus;
	(MSG_READ_REPORT_STATUS_T)MSG_READ_REPORT_NONE,		//MmsReadStatus			readStatus;

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


bool _MmsDecodeInitialize(void)
{
	MmsInitMsgType(&mmsHeader.msgType);
	MmsInitMsgBody(&mmsHeader.msgBody);

	return true;
}

bool __MmsSetMmsHeaderOwner(int msgID, char* pszNewOwner)
{
	if (pszNewOwner) {
		// set new owner
		MSG_DEBUG("__MmsSetMmsHeaderOwner: set (%s, msgID=%d)\n", pszNewOwner, msgID);

		if ((mmsHeader.pszOwner = (char*)malloc(strlen(pszNewOwner) + 1)) == NULL)
			return false;

		memset (mmsHeader.pszOwner, 0, strlen(pszNewOwner) + 1) ;

		strcpy(mmsHeader.pszOwner, pszNewOwner);
		mmsHeader.bActive = true;
		mmsHeader.msgID = msgID;
	} else {
		// delete current owner
		if (mmsHeader.pszOwner)	{
			MSG_DEBUG("__MmsSetMmsHeaderOwner: free (%s %d)\n", mmsHeader.pszOwner, msgID);
			free(mmsHeader.pszOwner);
			mmsHeader.pszOwner = NULL;
		}
		mmsHeader.bActive = false;
		mmsHeader.msgID = -1;
	}

	return true;
}


void _MmsInitHeader()
{
	mmsHeader.type = MMS_MSGTYPE_ERROR;

	memset(mmsHeader.szTrID, 0, MMS_TR_ID_LEN + 1);
	mmsHeader.version = MMS_VERSION;
	mmsHeader.date = 0;

	MsgFreeHeaderAddress(mmsHeader.pFrom);
	MsgFreeHeaderAddress(mmsHeader.pTo);
	MsgFreeHeaderAddress(mmsHeader.pCc);
	MsgFreeHeaderAddress(mmsHeader.pBcc);

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


	mmsHeader.msgStatus = (MSG_DELIVERY_REPORT_STATUS_T)MSG_DELIVERY_REPORT_NONE;
	mmsHeader.readStatus = (MSG_READ_REPORT_STATUS_T)MSG_READ_REPORT_NONE;

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

	_MmsDecodeInitialize();
}



void _MmsCleanDecodeBuff(void)
{
	memset(gpMmsDecodeBuf1, 0, gMmsDecodeMaxLen + 1);
	memset(gpMmsDecodeBuf2, 0, gMmsDecodeMaxLen + 1);
	gpCurMmsDecodeBuff = NULL;
	gCurMmsDecodeBuffPos = 0;
	gMmsDecodeBufLen = 0;
}


void _MmsRegisterDecodeBuffer(char *pInBuff1, char *pInBuff2, int maxLen)
{
	gpMmsDecodeBuf1 = pInBuff1;
	gpMmsDecodeBuf2 = pInBuff2;
	gpCurMmsDecodeBuff = NULL;
	gCurMmsDecodeBuffPos = 0;
	gMmsDecodeMaxLen = maxLen;
	gMmsDecodeCurOffset = 0;
	gMmsDecodeBufLen = 0;
}

void _MmsUnregisterDecodeBuffer(void)
{
	gpMmsDecodeBuf1 = NULL;
	gpMmsDecodeBuf2 = NULL;
	gpCurMmsDecodeBuff = NULL;
	gCurMmsDecodeBuffPos = 0;
	gMmsDecodeMaxLen = 0;
	gMmsDecodeCurOffset = 0;
	gMmsDecodeBufLen = 0;
}


int _MmsGetDecodeOffset(void)
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

	MSG_DEBUG("MmsBinaryDecodeMsgHeader: pFile=%d, total len=%d\n", pFile, totalLength);

	_MmsCleanDecodeBuff();

	if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos,
								   &gMmsDecodeCurOffset, gpMmsDecodeBuf1, gpMmsDecodeBuf2,
								   gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("MmsBinaryDecodeMsgHeader: fail to load to buffer \n");
		goto __CATCH;
	}

	while (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength)) {
		fieldCode = oneByte & 0x7f;

		switch (_MmsGetBinaryType(MmsCodeFieldCode, fieldCode)) {
		case MMS_CODE_RESPONSESTATUS:

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: responseStatus GetOneByte fail\n");
				goto __CATCH;
			}

			fieldValue = _MmsGetBinaryType(MmsCodeResponseStatus, (UINT16)(oneByte & 0x7F));

			if (fieldValue == 0xFFFF) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: responseStatus error\n");
				goto __CATCH;
			}

			if (fieldValue >= 0x0044 && fieldValue <= 0x005F) {
				fieldValue = 0x0040;
			} else if (fieldValue >= 0x006A && fieldValue <= 0x007F) {
				fieldValue = 0x0060;
			}

			mmsHeader.responseStatus = (MmsResponseStatus)fieldValue;

			MSG_DEBUG("MmsBinaryDecodeMsgHeader: response status = %s\n", MmsDebugGetResponseStatus(mmsHeader.responseStatus));

			break;

		case MMS_CODE_RETRIEVESTATUS:

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: retrieveStatus GetOneByte fail\n");
				goto __CATCH;
			}

			fieldValue = _MmsGetBinaryType(MmsCodeRetrieveStatus, (UINT16)(oneByte & 0x7F));

			if (fieldValue == 0xFFFF) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: retrieveStatus error\n");
				goto __CATCH;
			}

			if (fieldValue >= 0x0043 && fieldValue <= 0x005F) {
				fieldValue = 0x0040; // 192; Error-transient-failure
			} else if (fieldValue >= 0x0064 && fieldValue <= 0x007F) {
				fieldValue = 0x0060; //224; Error-permanent-failure
			}

			mmsHeader.retrieveStatus = (MmsRetrieveStatus)fieldValue;

			MSG_DEBUG("MmsBinaryDecodeMsgHeader: retrieve status = %s\n",
															MmsDebugGetRetrieveStatus(mmsHeader.retrieveStatus));

			break;

		case MMS_CODE_RESPONSETEXT:

			if (__MmsBinaryDecodeEncodedString(pFile, mmsHeader.szResponseText, MMS_LOCALE_RESP_TEXT_LEN + 1, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : invalid MMS_CODE_RESPONSETEXT \n");
				goto __CATCH;
			}

			MSG_DEBUG("MmsBinaryDecodeMsgHeader: response text = %s\n", mmsHeader.szResponseText);
			break;

		case MMS_CODE_RETRIEVETEXT:

			if (__MmsBinaryDecodeEncodedString(pFile, mmsHeader.szRetrieveText, MMS_LOCALE_RESP_TEXT_LEN + 1, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : invalid MMS_CODE_RETRIEVETEXT \n");
				goto __CATCH;
			}

			MSG_DEBUG("MmsBinaryDecodeMsgHeader: retrieve text = %s\n", mmsHeader.szResponseText);
			break;

		case MMS_CODE_MSGID:

			if (__MmsBinaryDecodeText(pFile, mmsHeader.szMsgID, MMS_MSG_ID_LEN + 1, totalLength) < 0) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : MMS_CODE_MSGID is invalid\n");
				goto __CATCH;
			}

			MSG_DEBUG("MmsBinaryDecodeMsgHeader: msg id = %s\n", mmsHeader.szMsgID);

			if (MsgStrlen (mmsHeader.szMsgID) > 2)
				MsgMIMERemoveQuote (mmsHeader.szMsgID);

			break;

		case MMS_CODE_SUBJECT:

			if (__MmsBinaryDecodeEncodedString(pFile, mmsHeader.szSubject, MSG_LOCALE_SUBJ_LEN + 1, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : invalid MMS_CODE_SUBJECT \n");
				goto __CATCH;
			}

			pLimitData = (char *)malloc(MSG_LOCALE_SUBJ_LEN + 1);

			if (pLimitData == NULL) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : pLimitData malloc fail \n");
				goto __CATCH;
			}

			nRead = MsgCutUTFString((unsigned char*)pLimitData, MSG_LOCALE_SUBJ_LEN + 1, (unsigned char*)mmsHeader.szSubject, MSG_SUBJ_LEN);
			MSG_DEBUG("MmsBinaryDecodeMsgHeader : Subject edit.. \n");

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

			MSG_DEBUG("MmsBinaryDecodeMsgHeader: subject = %s\n", mmsHeader.szSubject);
			break;

		case MMS_CODE_FROM:

			/* Value-length (Address-present-token Encoded-string-value | Insert-address-token) */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : MMS_CODE_FROM is invalid\n");
				goto __CATCH;
			}

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: MMS_CODE_FROM GetOneByte fail\n");
				goto __CATCH;
			}

			// DRM_TEMPLATE - start

			valueLength--;

			if (oneByte == (_MmsGetBinaryValue(MmsCodeAddressType, MMS_ADDRESS_PRESENT_TOKEN)|0x80)) {
				if (valueLength > 0) {
					mmsHeader.pFrom = __MmsDecodeEncodedAddress(pFile, totalLength);
					if (mmsHeader.pFrom == NULL) {
						MSG_DEBUG("MmsBinaryDecodeMsgHeader : MMS_CODE_FROM __MmsDecodeEncodedAddress fail\n");
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

				MSG_DEBUG("MmsBinaryDecodeMsgHeader: from = %s\n", mmsHeader.pFrom->szAddr);
				// DRM_TEMPLATE - end
			} else if (oneByte == (_MmsGetBinaryValue(MmsCodeAddressType, MMS_INSERT_ADDRESS_TOKEN)|0x80)) {
				/* Present Token only */
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : MMS_CODE_FROM insert token\n");
			} else {
				/* from data broken */
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: from addr broken\n");
				gCurMmsDecodeBuffPos--;
				goto __CATCH;
			}
			break;

		case MMS_CODE_TO:

			pAddr = __MmsDecodeEncodedAddress(pFile, totalLength);
			if (pAddr == NULL) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : MMS_CODE_TO __MmsDecodeEncodedAddress fail\n");
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

			MSG_DEBUG("MmsBinaryDecodeMsgHeader: to = %s\n", mmsHeader.pTo->szAddr);
			break;

		case MMS_CODE_BCC:

			pAddr = __MmsDecodeEncodedAddress(pFile, totalLength);
			if (pAddr == NULL) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : MMS_CODE_BCC __MmsDecodeEncodedAddress fail\n");
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

			MSG_DEBUG("MmsBinaryDecodeMsgHeader: bcc = %s\n", mmsHeader.pBcc->szAddr);
			break;

		case MMS_CODE_CC:

			pAddr = __MmsDecodeEncodedAddress(pFile, totalLength);
			if (pAddr == NULL) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : MMS_CODE_CC __MmsDecodeEncodedAddress fail\n");
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
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: cc = %s\n", mmsHeader.pCc->szAddr);
			break;

		case MMS_CODE_CONTENTLOCATION:

			if (__MmsBinaryDecodeText(pFile, mmsHeader.szContentLocation, MMS_LOCATION_LEN + 1, totalLength) < 0) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : MMS_CODE_CONTENTLOCATION is invalid\n");
				goto __CATCH;
			}
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: content location = %s\n", mmsHeader.szContentLocation);
			break;

		case MMS_CODE_DATE:

			if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.date, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : MMS_CODE_DATE is invalid\n");
				goto __CATCH;
			}
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: date = %d\n", mmsHeader.date);
			break;

		case MMS_CODE_DELIVERYREPORT:

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: deliveryReport GetOneByte fail\n");
				goto __CATCH;
			}

			fieldValue = _MmsGetBinaryType(MmsCodeDeliveryReport, (UINT16)(oneByte & 0x7F));

			if (fieldValue == 0xFFFF) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: deliveryReport error\n");
				goto __CATCH;
			}

			mmsHeader.deliveryReport = (MmsReport)fieldValue;

			MSG_DEBUG("MmsBinaryDecodeMsgHeader: delivery report=%s\n", MmsDebugGetMmsReport(mmsHeader.deliveryReport));
			break;

		case MMS_CODE_DELIVERYTIME:

			/* value_length (absolute-token Long-integer | Relative-token Long-integer) */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : invalid MMS_CODE_DELIVERYTIME \n");
				goto __CATCH;
			}

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: delivery time GetOneByte fail\n");
				goto __CATCH;
			}

			//DRM_TEMPLATE - start
			valueLength--;

			if (oneByte == (_MmsGetBinaryValue(MmsCodeTimeType, MMS_TIMETYPE_ABSOLUTE)|0x80)) {
				mmsHeader.deliveryTime.type = MMS_TIMETYPE_ABSOLUTE;

				if (valueLength > 0) {
					if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.deliveryTime.time, totalLength) == false)	{
						MSG_DEBUG("MmsBinaryDecodeMsgHeader : invalid MMS_CODE_DELIVERYTIME\n");
						goto __CATCH;
					}
				}
			// DRM_TEMPLATE - end
			} else {
				mmsHeader.deliveryTime.type = MMS_TIMETYPE_RELATIVE;

				if (__MmsBinaryDecodeInteger(pFile, (UINT32*)&mmsHeader.deliveryTime.time, &tmpIntLen, totalLength) == false) {
					MSG_DEBUG("MmsBinaryDecodeMsgHeader : __MmsBinaryDecodeInteger fail...\n");
					goto __CATCH;
				}
			}
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: delivery type=%d, time=%d\n", mmsHeader.deliveryTime.type, mmsHeader.deliveryTime.time);
			break;

		case MMS_CODE_EXPIRYTIME:

			/* value_length(absolute-token Long-integer | Relative-token Long-integer) */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : invalid MMS_CODE_EXPIRYTIME \n");
				goto __CATCH;
			}

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: expiry time GetOneByte fail\n");
				goto __CATCH;
			}

			// DRM_TEMPLATE - start
			valueLength--;

			if (oneByte == (_MmsGetBinaryValue(MmsCodeTimeType, MMS_TIMETYPE_ABSOLUTE)|0x80)) {
				mmsHeader.expiryTime.type = MMS_TIMETYPE_ABSOLUTE;

				if (valueLength > 0) {
					if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.expiryTime.time, totalLength) == false) {
						MSG_DEBUG("MmsBinaryDecodeMsgHeader : MMS_CODE_EXPIRYTIME is invalid\n");
						goto __CATCH;
					}
				}
			// DRM_TEMPLATE - end
			} else {
				mmsHeader.expiryTime.type = MMS_TIMETYPE_RELATIVE;

				if (__MmsBinaryDecodeInteger(pFile, (UINT32*)&mmsHeader.expiryTime.time, &tmpIntLen, totalLength) == false) {
					MSG_DEBUG("MmsBinaryDecodeMsgHeader : __MmsBinaryDecodeInteger fail...\n");
					goto __CATCH;
				}
			}
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: expiry = %d\n", mmsHeader.expiryTime.time);
			break;

		case MMS_CODE_MSGCLASS:

			/* Class-value = Class-identifier | Token Text */

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: msgClass GetOneByte fail\n");
				goto __CATCH;
			}

			if (oneByte > 0x7f) {
				/* Class-identifier */
				mmsHeader.msgClass = (MmsMsgClass)_MmsGetBinaryType(MmsCodeMsgClass, (UINT16)(oneByte & 0x7F));
			} else {
				if (__MmsBinaryDecodeText(pFile, szGarbageBuff, MSG_STDSTR_LONG, totalLength) < 0) {
					MSG_DEBUG("MmsBinaryDecodeMsgHeader: 1. __MmsBinaryDecodeText fail. (class)\n");
					goto __CATCH;
				}
			}
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: msg class=%s\n", MmsDebugGetMsgClass(mmsHeader.msgClass));
			break;

		case MMS_CODE_MSGSIZE:

			if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.msgSize, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : MMS_CODE_MSGSIZE is invalid\n");
				goto __CATCH;
			}
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: msg size = %d\n", mmsHeader.msgSize);
			break;

		case MMS_CODE_MSGSTATUS:

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: msgStatus GetOneByte fail\n");
				goto __CATCH;
			}

			mmsHeader.msgStatus =  (MSG_DELIVERY_REPORT_STATUS_T)_MmsGetBinaryType(MmsCodeMsgStatus, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: msg status=%s \n", MmsDebugGetMsgStatus(mmsHeader.msgStatus)) ;
			break;

		case MMS_CODE_MSGTYPE:

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: msgStatus GetOneByte fail\n");
				goto __CATCH;
			}

			mmsHeader.type = (MmsMsgType)_MmsGetBinaryType(MmsCodeMsgType, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: msg type=%s\n", MmsDebugGetMsgType(mmsHeader.type));
			break;

		case MMS_CODE_PRIORITY:

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: msgStatus GetOneByte fail\n");
				goto __CATCH;
			}
			mmsHeader.priority = (MmsPriority)_MmsGetBinaryType(MmsCodePriority, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: priority=%d\n", mmsHeader.priority);
			break;

		case MMS_CODE_READREPLY:

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: msgStatus GetOneByte fail\n");
				goto __CATCH;
			}
			mmsHeader.readReply = (MmsReport)_MmsGetBinaryType(MmsCodeReadReply, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: read reply=%s \n", MmsDebugGetMmsReport(mmsHeader.readReply));
			break;

		case MMS_CODE_REPORTALLOWED:

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: msgStatus GetOneByte fail\n");
				goto __CATCH;
			}
			mmsHeader.reportAllowed =  (MmsReportAllowed)_MmsGetBinaryType(MmsCodeReportAllowed, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: mmsHeader.reportAllowed=%d\n", MmsDebugGetMmsReportAllowed(mmsHeader.reportAllowed));
			break;

		case MMS_CODE_SENDERVISIBILLITY:

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: msgStatus GetOneByte fail\n");
				goto __CATCH;
			}
			mmsHeader.hideAddress= (MmsSenderVisible)!(_MmsGetBinaryType(MmsCodeSenderVisibility, (UINT16)(oneByte &0x7F)));
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: sender visible=%d \n", mmsHeader.hideAddress);
			break;

		case MMS_CODE_TRID:

			if (__MmsBinaryDecodeText(pFile, mmsHeader.szTrID, MMS_TR_ID_LEN + 1, totalLength) < 0) {
				MSG_DEBUG("Transaction ID Too Long \n");
				goto __CATCH;
			}
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: trID = %s\n", mmsHeader.szTrID);
			break;

		case MMS_CODE_VERSION:
			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: msgStatus GetOneByte fail\n");
				goto __CATCH;
			}
			mmsHeader.version = oneByte;

			MSG_DEBUG("MmsBinaryDecodeMsgHeader: ver = 0x%x\n", mmsHeader.version);
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

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: msgStatus GetOneByte fail\n");
				goto __CATCH;
			}

			mmsHeader.readStatus =  (MSG_READ_REPORT_STATUS_T)_MmsGetBinaryType(MmsCodeReadStatus, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: read status=%s\n", MmsDebugGetMmsReadStatus(mmsHeader.readStatus));
			break;

		case MMS_CODE_REPLYCHARGING:

			/* Reply-charging-value = Requested | Requested text only | Accepted | Accepted text only */

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: msgStatus GetOneByte fail\n");
				goto __CATCH;
			}

			mmsHeader.replyCharge.chargeType =  (MmsReplyChargeType)_MmsGetBinaryType(MmsCodeReplyCharging, (UINT16)(oneByte & 0x7F));
			MSG_DEBUG("MmsBinaryDecodeMsgHeader: mmsHeader.reply charge=%d\n", mmsHeader.replyCharge.chargeType);
			break;

		case MMS_CODE_REPLYCHARGINGDEADLINE:

			/* Reply-charging-deadline-value = Value-length (Absolute-token Date-value | Relative-token Delta-seconds-value) */

			if (__MmsDecodeValueLength(pFile, &valueLength, totalLength) <= 0) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : invalid MMS_CODE_REPLYCHARGINGDEADLINE \n");
				goto __CATCH;
			}

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: msgStatus GetOneByte fail\n");
				goto __CATCH;
			}

			if (oneByte == (_MmsGetBinaryValue(MmsCodeTimeType, MMS_TIMETYPE_ABSOLUTE) | 0x80)) {
				mmsHeader.replyCharge.deadLine.type = MMS_TIMETYPE_ABSOLUTE;
			} else {
				mmsHeader.replyCharge.deadLine.type = MMS_TIMETYPE_RELATIVE;
			}

			// DRM_TEMPLATE - start
			valueLength--;

			if (valueLength > 0) {
				if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.replyCharge.deadLine.time, totalLength) == false) {
					MSG_DEBUG("MmsBinaryDecodeMsgHeader : MMS_CODE_REPLYCHARGINGDEADLINE is invalid\n");
					goto __CATCH;
				}
			}
			// DRM_TEMPLATE - end
			break;

		case MMS_CODE_REPLYCHARGINGID:

			/* Reply-charging-ID-value = Text-string */

			if (__MmsBinaryDecodeText(pFile, mmsHeader.replyCharge.szChargeID, MMS_MSG_ID_LEN + 1, totalLength) < 0) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader: 1. __MmsBinaryDecodeText fail. (szReplyChargingID)\n");
				goto __CATCH;
			}
			break;

		case MMS_CODE_REPLYCHARGINGSIZE:

			/* Reply-charging-size-value = Long-integer */

			if (__MmsDecodeLongInteger(pFile, (UINT32*)&mmsHeader.replyCharge.chargeSize, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : MMS_CODE_REPLYCHARGINGSIZE is invalid\n");
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
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : 1. invalid MMS_CODE_PREVIOUSLYSENTBY \n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeInteger(pFile, &tmpInteger, &tmpIntLen, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : 2. invalid MMS_CODE_PREVIOUSLYSENTBY \n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeEncodedString(pFile, szGarbageBuff, MSG_STDSTR_LONG, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : invalid MMS_CODE_RETRIEVETEXT \n");
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
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : 1. invalid MMS_CODE_PREVIOUSLYSENTDATE \n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeInteger(pFile, &tmpInteger, &tmpIntLen, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : 2. invalid MS_CODE_PREVIOUSLYSENTDATE \n");
				goto __CATCH;
			}

			if (__MmsDecodeLongInteger(pFile, (UINT32*)&tmpInteger, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeMsgHeader : 3. invalid MMS_CODE_PREVIOUSLYSENTDATE \n");
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

				offset = _MmsGetDecodeOffset();
				if (offset >= totalLength)
					goto __RETURN;

				remainLength = totalLength - offset;

				while ((oneByte < 0x80) && (remainLength > 0)) {
					if (__MmsBinaryDecodeCheckAndDecreaseLength(&remainLength, 1) == false) {
						MSG_DEBUG("MmsBinaryDecodeMsgHeader: __MmsBinaryDecodeCheckAndDecreaseLength fail\n");
						goto __CATCH;
					}
					if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
						MSG_DEBUG("MmsBinaryDecodeMsgHeader: responseStatus GetOneByte fail\n");
						goto __CATCH;
					}
				}

				gCurMmsDecodeBuffPos--;
			}

			break;
		}	/* switch */

		offset = _MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

	}	/* while */


__RETURN:

	MSG_DEBUG("MmsBinaryDecodeMsgHeader: success\n");
	return true;


__CATCH:
	MSG_DEBUG("MmsBinaryDecodeMsgHeader: failed\n");

	return false;
}

#ifdef FEATURE_JAVA_MMS
// for JAVA MMS AppId - check later whether it can be handled by MmsBinaryDecodeMsgHeader
bool MmsBinaryDecodeContentType(FILE *pFile, char *szFilePath, int totalLength)
{
	int length	= 0;

	MSG_DEBUG("MmsBinaryDecodeContentType:\n");

	if (szFilePath != NULL)
		strncpy(mmsHeader.msgType.szOrgFilePath, szFilePath, strlen(szFilePath));
	mmsHeader.msgType.offset = _MmsGetDecodeOffset() - 1;		// + Content-Type code value

	// read data(2K) from msg file(/User/Msg/Inbox/5) to gpCurMmsDecodeBuff for decoding
	if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos,
									&gMmsDecodeCurOffset, gpMmsDecodeBuf1, gpMmsDecodeBuf2,
									gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("MmsBinaryDecodeContentType: fail to load to buffer \n");
		goto __CATCH;
	}

	// msg's type [ex] related, mixed, single part (jpg, amr and etc)
	length = MmsBinaryDecodeContentType(pFile, &mmsHeader.msgType, totalLength);
	if (length == -1) {
		MSG_DEBUG("MmsBinaryDecodeContentType: MMS_CODE_CONTENTTYPE is fail\n");
		goto __CATCH;
	}
	MSG_DEBUG("Content-Type: Application-ID:%s Reply-To-Application-ID:%s",mmsHeader.msgType.param.szApplicationID, mmsHeader.msgType.param.szReplyToApplicationID);

	return true;

__CATCH:

	/* fixme: Delete multipart using MmsDeleteMsg() */

	return false;
}
#endif

bool MmsBinaryDecodeMsgBody(FILE *pFile, char *szFilePath, int totalLength)
{
	int length = 0;
	int offset = 0;

	MsgMultipart *pMultipart = NULL;

	MSG_DEBUG("MmsBinaryDecodeMsgBody:\n");

	if (szFilePath != NULL)
		strncpy(mmsHeader.msgType.szOrgFilePath, szFilePath , strlen(szFilePath));

	mmsHeader.msgType.offset = _MmsGetDecodeOffset() - 1;		// + Content-Type code value

	// read data(2K) from msg file(/User/Msg/Inbox/5) to gpCurMmsDecodeBuff for decoding
	if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos,
									&gMmsDecodeCurOffset, gpMmsDecodeBuf1, gpMmsDecodeBuf2,
									gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("MmsBinaryDecodeMsgBody: fail to load to buffer \n");
		goto __CATCH;
	}

	// msg's type [ex] related, mixed, single part (jpg, amr and etc)
	length = MmsBinaryDecodeContentType(pFile, &mmsHeader.msgType, totalLength);
	if (length == -1) {
		MSG_DEBUG("MmsBinaryDecodeMsgBody: MMS_CODE_CONTENTTYPE is fail\n");
		goto __CATCH;
	}

	mmsHeader.msgType.size	 = length + 1;						// + Content-Type code value
	mmsHeader.msgBody.offset = _MmsGetDecodeOffset();

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

		MSG_DEBUG("__MmsBinaryMsgBodyDecode: Decode multipart\n");

		offset = _MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		if (MmsBinaryDecodeMultipart(pFile, szFilePath, &mmsHeader.msgType, &mmsHeader.msgBody, totalLength) == false) {
			MSG_DEBUG("__MmsBinaryMsgBodyDecode: MmsBinaryDecodeMultipart is fail.\n");
			goto __CATCH;
		}
		break;

	default:

		/* Single part message ---------------------------------------------- */

		strcpy(mmsHeader.msgBody.szOrgFilePath, szFilePath);

		offset = _MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		if (MmsBinaryDecodePartBody(pFile, totalLength - mmsHeader.msgBody.offset, totalLength) == false) {
			MSG_DEBUG("__MmsBinaryMsgBodyDecode: MmsBinaryDecodePartBody is fail.(Single Part)\n");
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

	return true;

__CATCH:

	/* fixme: Delete multipart using MmsDeleteMsg() */

	if (pMultipart) {
		if (pMultipart->pBody) {
			if (pMultipart->pBody->body.pText) {
				free(pMultipart->pBody->body.pText);
				pMultipart->pBody->body.pText = NULL;
			}

			free(pMultipart->pBody);
			pMultipart->pBody = NULL;
		}

		free(pMultipart);
		pMultipart = NULL;
	}

	return false;
}

#if MMS_ENABLE_EXTEND_CFM
// DRM (Extended)CFM
//	Get Extended CFM value.
static bool __MmsConvertString2Bool(char *pszValue)
{
	if (!strcasecmp(pszValue, "no"))
		return true;

	return false;
}
#endif

bool MmsBinaryDecodeParameter(FILE *pFile, MsgType *pMsgType, int valueLength, int totalLength)
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

	MSG_DEBUG("MmsBinaryDecodeParameter: \n");

	while (valueLength > 0) {
		if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodeContentType: paramCode _MmsBinaryDecodeGetOneByte fail\n");
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

			if (MsgCheckFileNameHasInvalidChar(pMsgType->param.szName)) {
				_MsgReplaceInvalidFileNameChar(pMsgType->param.szName, '_');
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

			if (MsgCheckFileNameHasInvalidChar(pMsgType->param.szFileName)) {
				_MsgReplaceInvalidFileNameChar(pMsgType->param.szFileName, '_');
			}

			if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, length) == false)
				goto __RETURN;

			break;

		case 0x89: //type = Constrained-encoding = Extension-Media | Short-integer

			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
				MSG_DEBUG("MmsBinaryDecodeContentType: type _MmsBinaryDecodeGetOneByte fail\n");
				goto __CATCH;
			}

			if (oneByte > 0x7f) {
				pMsgType->param.type = _MmsGetBinaryType(MmsCodeContentType,
														  (UINT16)(oneByte & 0x7f));
				if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, 1) == false)
					goto __RETURN;
			} else {
				gCurMmsDecodeBuffPos--;

				textLength = 0;
				szTypeString = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
				pMsgType->param.type = _MmsGetTextType(MmsCodeContentType, szTypeString);

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
				MSG_DEBUG("MmsBinaryDecodeContentType: Unsupported parameter\n");

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
					MSG_DEBUG("MmsBinaryDecodeContentType: Unsupported parameter(%d)\n", integer);
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

						MSG_DEBUG("MmsBinaryDecodeContentType: Unsupported parameter(%s)\n", szTypeValue);
						if (__MmsBinaryDecodeCheckAndDecreaseLength(&valueLength, textLength) == false)
							goto __RETURN;
					}
				}

				if (szTypeString) {
					MSG_DEBUG("MmsBinaryDecodeContentType: Unsupported parameter(%s)\n", szTypeString);
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

	if (szTypeString) {
		free(szTypeString);
		szTypeString = NULL;
	}
	return false;
}



/**
 * Decode Encoded Content type
 *
 * @param 	pEncodedData	[in] ContentType encoded data
 * @param	pMsgType		[out] Decoded MsgType
 * @return 	Decoded address list
 */
int MmsBinaryDecodeContentType(FILE *pFile, MsgType *pMsgType, int totalLength)
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

	MSG_DEBUG("MmsBinaryDecodeContentType: decoding content type..\n");


	length = __MmsDecodeValueLength(pFile, (UINT32*)&valueLength, totalLength);
	if (length <= 0) {
		/*
		 * Constrained-media or Single part message
		 * Constrained-media = Constrained-encoding = Extension-Media | Short-integer
		 * Extension-media   = *TEXT End-of-string
		 * Short-integer     = OCTET(1xxx xxxx)
		 */

		if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodeContentType: Constrained-media _MmsBinaryDecodeGetOneByte fail\n");
			goto __CATCH;
		}

		if (oneByte > 0x7F) {
			/* Short-integer */
			pMsgType->type = _MmsGetBinaryType(MmsCodeContentType, (UINT16)(oneByte & 0x7F));
			length = 1;
		} else {
			char *pszTemp = NULL;

			/* Extension-Media */
			gCurMmsDecodeBuffPos--;

			textLength = 0;
			szTypeString = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);

			if (szTypeString && (strchr(szTypeString, ';')) != NULL) {
				pszTemp = _MsgGetStringUntilDelimiter(szTypeString, ';');
				if (pszTemp) {
					free(szTypeString);
					szTypeString = pszTemp;
				}
			}

			pMsgType->type = _MmsGetTextType(MmsCodeContentType, szTypeString);

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

		if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodeContentType: Well-known-media _MmsBinaryDecodeGetOneByte fail\n");
			goto __CATCH;
		}

		if (oneByte > 0x7F) {
			/* Well-known-media */
			pMsgType->type = _MmsGetBinaryType(MmsCodeContentType, (UINT16)(oneByte & 0x7F));
			valueLength--;
		} else {
			/* Extension-Media */
			gCurMmsDecodeBuffPos--;

			textLength = 0;
			szTypeString = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
			pMsgType->type = _MmsGetTextType(MmsCodeContentType, szTypeString);
			valueLength -= textLength;

			if (szTypeString) {
				free(szTypeString);
				szTypeString = NULL;
			}
		}

		MSG_DEBUG("MmsBinaryDecodeContentType: content type=%s\n", MmsDebugGetMimeType((MimeType)pMsgType->type));


		if (MmsBinaryDecodeParameter(pFile, pMsgType, valueLength, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodeContentType: Content-Type parameter fail\n");
			goto __CATCH;
		}
	}

	return length;

__CATCH:
	return -1;
}


bool MmsBinaryDecodePartHeader(FILE *pFile, MsgType *pMsgType, int headerLen, int totalLength)
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
		if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodePartHeader: field code GetOneByte fail\n");
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

				if (MsgLatin2UTF ((unsigned char*)pMsgType->szContentLocation, MMS_CONTENT_ID_LEN + 1, (unsigned char*)pLatinBuff, textLength) < 0) {
					MSG_DEBUG("MmsBinaryDecodePartHeader: MsgLatin2UTF fail \n");
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
				if (MsgLatin2UTF ((unsigned char*)pMsgType->szContentID, MMS_CONTENT_ID_LEN + 1, (unsigned char*)pLatinBuff, textLength) < 0) {
					MSG_DEBUG("MmsBinaryDecodePartHeader: MsgLatin2UTF fail \n");
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

				if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
					MSG_DEBUG("MmsBinaryDecodePartHeader: Disposition value GetOneByte fail\n");
					goto __CATCH;
				}

				if (length > 0)
					valueLength--;

				if (oneByte >= 0x80) {
					pMsgType->disposition = _MmsGetBinaryType(MmsCodeMsgDisposition, (UINT16)(oneByte & 0x7F));

					if (pMsgType->disposition == INVALID_HOBJ) {
						MSG_DEBUG("MmsBinaryDecodePartHeader : Content-Disposition _MmsGetBinaryType fail.\n");
						pMsgType->disposition = MSG_DISPOSITION_ATTACHMENT;		// default
					}

					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, 1) == false)
						goto __RETURN;

					if (MmsBinaryDecodeParameter(pFile, pMsgType, valueLength, totalLength) == false) {
						MSG_DEBUG("MmsBinaryDecodePartHeader: Disposition parameter fail\n");
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

					if (MmsBinaryDecodeParameter(pFile, pMsgType, valueLength, totalLength) == false)
					{
						MSG_DEBUG("MmsBinaryDecodePartHeader: Disposition parameter fail\n");
						goto __CATCH;
					}

					if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, valueLength) == false)
						goto __RETURN;

				}

				break;

			case 0x0B:	//Content-Encoding

				if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
					MSG_DEBUG("MmsBinaryDecodePartHeader: Disposition value GetOneByte fail\n");
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
						; Equivalent to the special RFC2616 charset value ¡°*¡±
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
						_MmsGetBinaryType(MmsCodeCharSet, (UINT16)charset);

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

				if (_MmsBinaryDecodeGetBytes(pFile, szTemp, valueLength, totalLength) < 0) {
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

			MSG_DEBUG(" MmsBinaryDecodePartHeader: Application-header = Token-text Application-specific-value \n");

			gCurMmsDecodeBuffPos--;

			/* Token-text */

			textLength = 0;
			pCode = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
			if (pCode == NULL) {
				MSG_DEBUG("MmsBinaryDecodePartHeader: pCode is null\n");
				goto __CATCH;
			}

			if (__MmsBinaryDecodeCheckAndDecreaseLength(&headerLen, textLength) == false)
				goto __RETURN;

			MSG_DEBUG(" MmsBinaryDecodePartHeader: Token-text (%s) \n", pCode);


			/* Application-specific-value */

			textLength = 0;
			pValue = __MmsBinaryDecodeText2(pFile, totalLength, &textLength);
			if (pValue == NULL) {
				MSG_DEBUG("MmsBinaryDecodePartHeader: pValue is null\n");
				goto __CATCH;
			}

			MSG_DEBUG(" MmsBinaryDecodePartHeader: Application-specific-value (%s) \n", pValue);


			pParam = strchr(pValue, MSG_CH_ADDR_DELIMETER);
			if (pParam) {
				ch = *pParam;
				*pParam = '\0';
			}

			switch (_MmsGetTextType(MmsCodeMsgBodyHeaderCode, pCode)) {
			case MMS_BODYHDR_TRANSFERENCODING:		// Content-Transfer-Encoding
				pMsgType->encoding = _MmsGetTextType(MmsCodeContentTransferEncoding, pValue);
				break;

			case MMS_BODYHDR_CONTENTID:				// Content-ID

				pLatinBuff = (char *)malloc(MMS_CONTENT_ID_LEN + 1);
				if (pLatinBuff == NULL)
				{
					goto __CATCH;
				}

				MsgMIMERemoveQuote (pValue);
				strncpy(pLatinBuff, pValue, MMS_MSG_ID_LEN);

				length = strlen(pLatinBuff);
				if (MsgLatin2UTF ((unsigned char*)pMsgType->szContentID, MMS_CONTENT_ID_LEN + 1, (unsigned char*)pLatinBuff, length) < 0)
				{
					MSG_DEBUG("MmsBinaryDecodePartHeader: MsgLatin2UTF fail \n");
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
				if (MsgLatin2UTF ((unsigned char*)pMsgType->szContentLocation, MMS_CONTENT_ID_LEN + 1, (unsigned char*)pLatinBuff, length) < 0) {
					MSG_DEBUG("MmsBinaryDecodePartHeader: MsgLatin2UTF fail \n");
					goto __CATCH;
				}

				free(pLatinBuff);
				pLatinBuff = NULL;
				break;

			case MMS_BODYHDR_DISPOSITION:			// Content-Disposition
				pMsgType->disposition = _MmsGetTextType(MmsCodeMsgDisposition, pValue);
				break;

			case MMS_BODYHDR_X_OMA_DRM_SEPARATE_DELIVERY:	// DRM RO WAITING
					break;

			default:
				MSG_DEBUG("Unknown Field : %s, Value: %s\n", pCode, pValue);
				break;
			}

			if (pParam) {
				_MsgParseParameter(pMsgType, pParam + 1);
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


bool MmsBinaryDecodeEntries(FILE *pFile, UINT32 *npEntries, int totalLength)
{
	int length = 0;

	length = __MmsBinaryDecodeUintvar(pFile, npEntries, totalLength);
	if (length <= 0) {
		goto __CATCH;
	}

	MSG_DEBUG("MmsBinaryDecodeEntries: Number of Entries = %d\n", *npEntries);

	return true;

__CATCH:
	return false;
}



bool MmsBinaryDecodePartBody(FILE *pFile, UINT32 bodyLength, int totalLength)
{
	int offset = 0;

	/*
	 * Currently, offset and size is
	 * the only information used with msgBody.
	 * If you need, add here more information
	 */

	MSG_DEBUG("MmsBinaryDecodePartBody: \n");

	offset = _MmsGetDecodeOffset();
	offset += bodyLength;

	if (MsgFseek(pFile, offset, SEEK_SET) < 0) {
		MSG_DEBUG("MmsBinaryDecodePartBody: fail to seek file pointer \n");
		goto __CATCH;
	}

	_MmsCleanDecodeBuff();

	gMmsDecodeCurOffset = offset;

	if (offset >= totalLength)
		goto __RETURN;

	if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
								   gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("_MmsBinaryDecodeGetOneByte: fail to load to buffer \n");
		goto __CATCH;
	}

	return true;

__RETURN:
	return true;

__CATCH:
	return false;
}


static bool MmsBinaryDecodeMovePointer(FILE *pFile, int offset, int totalLength)
{
	if (offset > totalLength)
		goto __RETURN;

	if (MsgFseek(pFile, offset, SEEK_SET) < 0) {
		MSG_DEBUG("MmsBinaryDecodeMovePointer: fail to seek file pointer \n");
		goto __CATCH;
	}

	_MmsCleanDecodeBuff();

	gMmsDecodeCurOffset = offset;

	if (offset == totalLength)
		goto __RETURN;

	if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
									gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("MmsBinaryDecodeMovePointer: fail to load to buffer \n");
		goto __CATCH;
	}

__RETURN:
	return true;

__CATCH:
	return false;
}


bool MmsBinaryIsTextEncodedPart(FILE *pFile, int totalLength)
{
	UINT8 oneByte	= 0;
	int byteCount = 0;

	byteCount++;				//check 0x0D

	if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false)
		goto __CATCH;

	if (oneByte != 0x0D) {
		//it can be started "--" without 0x0D 0x0A
		if (oneByte != 0x2D) {
			goto __CATCH;
		} else {
			byteCount++;		// check 0x2D ('-')
			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false)
				goto __CATCH;

			if (oneByte != 0x2D) {
				goto __CATCH;
			} else {
				goto __RETURN;
			}
		}
	} else {
		byteCount++;			//check 0x0A
		if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false)
			goto __CATCH;

		if (oneByte != 0x0A) {
			goto __CATCH;
		} else {
			byteCount++;		// check 0x2D ('-')
			if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false)
				goto __CATCH;

			if (oneByte != 0x2D) {
				goto __CATCH;
			} else {
				byteCount++;	// check 0x2D ('-')
				if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false)
					goto __CATCH;

				if (oneByte != 0x2D) {
					goto __CATCH;
				} else {
					goto __RETURN;
				}
			}
		}
	}

__RETURN:
	gCurMmsDecodeBuffPos -= byteCount;
	return true;

__CATCH:
	gCurMmsDecodeBuffPos -= byteCount;
	return false;
}


bool MmsBinaryDecodeMultipart(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, int totalLength)
{
	UINT32 nEntries = 0;
	MsgMultipart *pMultipart = NULL;
	MsgMultipart *pLastMultipart = NULL;
	MsgMultipart *pPreMultipart	= NULL;
	int offset = 0;
	int	index = 0;

	MsgPresentationFactor factor = MSG_PRESENTATION_NONE;
	MsgPresentaionInfo presentationInfo;

	MSG_DEBUG("MmsBinaryDecodeMultipart: total length=%d\n", totalLength);

	presentationInfo.factor = MSG_PRESENTATION_NONE;
	presentationInfo.pCurPresentation = NULL;
	presentationInfo.pPrevPart = NULL;

	if (MmsBinaryDecodeEntries(pFile, &nEntries, totalLength) == false) {
		MSG_DEBUG("MmsBinaryDecodeMultipart: MmsBinaryDecodeEntries is fail.\n");
		goto __CATCH;
	}

	while (nEntries) {
		MSG_DEBUG("MmsBinaryDecodeMultipart: decoding %dth multipart\n", index);

		offset = _MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		if ((pMultipart = MsgAllocMultipart()) == NULL) {
			MSG_DEBUG("MmsBinaryDecodeMultipart: MsgAllocMultipart Fail \n");
			goto __CATCH;
		}

		if (MmsBinaryDecodeEachPart(pFile, szFilePath, &(pMultipart->type), pMultipart->pBody, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodeMultipart: MmsBinaryDecodeEachPart is fail.(nEntries = %d)\n", nEntries);
			goto __CATCH;
		}

		if (pMsgType->param.type == MIME_APPLICATION_SMIL) {
			factor = MsgIsPresentationEx(&(pMultipart->type), pMsgType->param.szStart, (MimeType)pMsgType->param.type);
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

		if (pMsgBody->body.pMultipart == NULL) {
			/* first multipart */
			pMsgBody->body.pMultipart = pMultipart;
			pLastMultipart			  = pMultipart;
			pPreMultipart			  = NULL;
		} else if (pLastMultipart != NULL) {
			pPreMultipart = pLastMultipart;

			pLastMultipart->pNext	= pMultipart;
			pLastMultipart			= pMultipart;
		}

		pMsgType->contentSize += pMultipart->pBody->size;

		nEntries--;
		pPreMultipart = pMultipart;
		MmsDebugPrintMulitpartEntry(pMultipart, index++);

	}

	pMsgBody->size = totalLength - pMsgBody->offset;

#ifdef	__SUPPORT_DRM__
	if (MmsDrm2GetConvertState() != MMS_DRM2_CONVERT_REQUIRED)
#endif
		MsgConfirmPresentationPart(pMsgType, pMsgBody, &presentationInfo);

	if (MsgResolveNestedMultipart(pMsgType, pMsgBody) == false) {
		MSG_DEBUG("MmsBinaryDecodeMultipart : MsgResolveNestedMultipart failed \n");
		goto __CATCH;
	}

__RETURN:
	return true;

__CATCH:
	return false;
}


bool MmsBinaryDecodeEachPart(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, int totalLength)
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

	offset = _MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;


	/* body length */

	if (__MmsBinaryDecodeUintvar(pFile, &bodyLength, totalLength) <= 0) {
		MSG_DEBUG("MmsBinaryDecodeEachPart: Get body length fail\n");
		goto __CATCH;
	}


	offset = _MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;


	/* Content Type */
	if (szFilePath != NULL)
		strncpy(pMsgType->szOrgFilePath, szFilePath, strlen(szFilePath));

	pMsgType->offset = _MmsGetDecodeOffset();
	pMsgType->size = headerLength;
	pMsgType->contentSize = bodyLength;

	if (pMsgType->offset > totalLength)
		goto __RETURN;

	length = MmsBinaryDecodeContentType(pFile, pMsgType, totalLength);
	if (length <= 0) {
		MSG_DEBUG("MmsBinaryDecodeEachPart: Decode contentType Fail \n");
		goto __CATCH;
	}

	offset = _MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;


	/* Part Header */

	if (MmsBinaryDecodePartHeader(pFile, pMsgType, headerLength - length, totalLength) == false) {
		MSG_DEBUG("MmsBinaryDecodeEachPart: Decode contentHeader Fail \n");
		goto __CATCH;
	}

	offset = _MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;

	/* Part Body */

	if (szFilePath != NULL)
		strncpy(pMsgBody->szOrgFilePath, szFilePath, strlen(szFilePath));

	pMsgBody->offset = _MmsGetDecodeOffset();
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

		if (MmsBinaryDecodeMultipart(pFile, szFilePath, pMsgType, pMsgBody, totalLength) == false) {
			MSG_DEBUG("MmsBinaryDecodeEachPart: MmsBinaryDecodeMultipart is fail.\n");
			goto __CATCH;
		}

		offset = _MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		break;


#ifdef __SUPPORT_DRM__

	case MIME_APPLICATION_VND_OMA_DRM_MESSAGE: /* Contains forwardLock OR combined-delivery media part */
		MSG_DEBUG("MmsBinaryDecodeEachPart: MIME_APPLICATION_VND_OMA_DRM_MESSAGE Part \n");

		if (MmsDrm2GetConvertState() != MMS_DRM2_CONVERT_NOT_FIXED && MmsDrm2GetConvertState() != MMS_DRM2_CONVERT_REQUIRED) {

			if (MmsBinaryDecodeDRMContent(pFile, szFilePath, pMsgType, pMsgBody, bodyLength, totalLength) == false)
				goto __CATCH;
		} else {
			MmsDrm2SetConvertState(MMS_DRM2_CONVERT_REQUIRED);

			bSuccess = MmsBinaryDecodePartBody(pFile, bodyLength, totalLength);
			if (bSuccess == false) {
				MSG_DEBUG("MmsBinaryDecodeEachPart: Decode contentBody Fail \n");
				goto __CATCH;
			}
		}
		offset = _MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		break;

	case MIME_APPLICATION_VND_OMA_DRM_CONTENT: /* Contains seperate-delivery media part (DCF) */

		MSG_DEBUG("MmsBinaryDecodeEachPart: MIME_APPLICATION_VND_OMA_DRM_CONTENT Part \n");

		if (MmsBinaryDecodeDRMContent(pFile, szFilePath, pMsgType, pMsgBody, bodyLength, totalLength) == false)
			goto __CATCH;

		offset = _MmsGetDecodeOffset();
		if (offset >= totalLength)
			goto __RETURN;

		break;
#endif

	default:
		MSG_DEBUG("MmsBinaryDecodeEachPart: Other normal Part \n");

		bSuccess = MmsBinaryDecodePartBody(pFile, bodyLength, totalLength);
		if (bSuccess == false) {
			MSG_DEBUG("MmsBinaryDecodeEachPart: Decode contentBody Fail \n");
			goto __CATCH;
		}

		offset = _MmsGetDecodeOffset();
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
bool __MmsParseDCFInfo(FILE *pFile, MsgDRMInfo *pDrmInfo, int totalLength)
{
	UINT8 version = 0;
	UINT8 contentTypeLen = 0;
	UINT8 contentURILen = 0;
	char *szContentType	= NULL;
	char *szContentURI = NULL;

	if (_MmsBinaryDecodeGetOneByte(pFile, &version, totalLength) == false) {
		MSG_DEBUG("__MmsParseDCFInfo: [version] GetOneByte fail\n");
		goto __CATCH;
	}

	if (_MmsBinaryDecodeGetOneByte(pFile, &contentTypeLen, totalLength) == false) {
		MSG_DEBUG("__MmsParseDCFInfo: [contentTypeLen] GetOneByte fail\n");
		goto __CATCH;
	}

	if (_MmsBinaryDecodeGetOneByte(pFile, &contentURILen, totalLength) == false) {
		MSG_DEBUG("__MmsParseDCFInfo: [contentURILen] GetOneByte fail\n");
		goto __CATCH;
	}

	//Get media content-type (mime-type)
	szContentType = (char *)malloc(contentTypeLen + 1);
	if (szContentType == NULL)
		goto __CATCH;

	memset(szContentType, 0, contentTypeLen + 1);

	if (_MmsBinaryDecodeGetBytes(pFile, szContentType, contentTypeLen + 1, totalLength) < 0) {
		MSG_DEBUG("__MmsParseDCFInfo : contentType is invalid\n");
		goto __CATCH;
	}
	gCurMmsDecodeBuffPos--;
	pDrmInfo->contentType = (MsgContentType)_MsgGetCode(MSG_TYPE, szContentType);


	//Get content-ID - 1.remover "cid:",   2.resolve "%hexa",   3.and copy the string
	szContentURI = (char *)malloc(contentURILen + 1);
	if (szContentURI == NULL)
		goto __CATCH;

	memset(szContentURI, 0, contentURILen + 1);

	if (_MmsBinaryDecodeGetBytes(pFile, szContentURI, contentURILen + 1, totalLength) < 0) {
		MSG_DEBUG("__MmsParseDCFInfo : contentType is invalid\n");
		goto __CATCH;
	}
	gCurMmsDecodeBuffPos--;
	pDrmInfo->szContentURI = MsgResolveContentURI(szContentURI);


	if (szContentType) {
		free(szContentType);
		szContentType = NULL;
	}

	if (szContentURI) {
		free(szContentURI);
		szContentURI = NULL;
	}
	return true;


__CATCH:

	if (szContentType) {
		free(szContentType);
		szContentType = NULL;
	}

	if (szContentURI) {
		free(szContentURI);
		szContentURI = NULL;
	}
	return false;

}


bool __MmsParseDCFHdr(FILE *pFile, MsgDRMInfo *pDrmInfo, UINT32 headerLen, int totalLength)
{
	char *szDCFHdr = NULL;
	MsgType partType;
	ULONG nRead = 0;
	int offset = 0;

	/* add to parse DCF header such as,
	 *  Right-Issuer, Content-Name, and Content-Description.
	 */

	szDCFHdr = (char *)malloc(headerLen + 1);
	if (szDCFHdr == NULL) {
		MSG_DEBUG("__MmsParseDCFHdr: szDCFHdr alloc fail\n");
		goto __CATCH;
	}
	memset(szDCFHdr, 0, headerLen + 1);

	offset = _MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;

	if (MsgFseek(pFile, offset, SEEK_SET) < 0)
		goto __CATCH;

	if ((nRead = MsgReadFile(szDCFHdr, sizeof(char), headerLen, pFile)) == 0){
		goto __CATCH;
	}
	szDCFHdr[nRead] = '\0';

	_MsgInitMsgType(&partType);
	_MsgParsePartHeader(&partType, szDCFHdr, headerLen);

	pDrmInfo->szContentName = partType.drmInfo.szContentName;
	pDrmInfo->szContentVendor = partType.drmInfo.szContentVendor;
	pDrmInfo->szContentDescription = partType.drmInfo.szContentDescription;
	pDrmInfo->szRightIssuer = partType.drmInfo.szRightIssuer;

	if (MmsBinaryDecodeMovePointer(pFile, offset + headerLen, totalLength) == false)
		goto __CATCH;

__RETURN:

	if (szDCFHdr) {
		free(szDCFHdr);
		szDCFHdr = NULL;
	}

	return true;

__CATCH:

	if (szDCFHdr) {
		free(szDCFHdr);
		szDCFHdr = NULL;
	}

	return false;
}


bool MmsBinaryDecodeDRMContent(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, unsigned int bodyLength, int totalLength)
{
	int offset = 0;
	char szTempFilePath[MSG_FILEPATH_LEN_MAX] = MSG_DATA_PATH"drm.dcf";
	char *pRawData = NULL;
	bool isFileCreated = false;

	MSG_DEBUG("bodyLength: %d\n", bodyLength);

	offset = _MmsGetDecodeOffset();

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

	if (pMsgType->type == MIME_APPLICATION_VND_OMA_DRM_MESSAGE && (MmsDrm2GetConvertState()!=MMS_DRM2_CONVERT_FINISH)) {
		MmsDrm2SetConvertState(MMS_DRM2_CONVERT_REQUIRED);
	} else {
		if (MsgDRM2GetDRMInfo(szTempFilePath, pMsgType) == false) {
			MSG_DEBUG("MsgDRM2GetDRMInfo() returns false \n");
			goto __CATCH;
		}
	}

	remove(szTempFilePath);
	isFileCreated = false;

	if (MmsBinaryDecodeMovePointer(pFile, offset + bodyLength, totalLength) == false)
		goto __CATCH;

__RETURN:

	if (pRawData) {
		free(pRawData);
		pRawData = NULL;
	}

	return true;

__CATCH:
	if (isFileCreated)
		remove(szTempFilePath);

	if (pRawData) {
		free(pRawData);
		pRawData = NULL;
	}

	return false;
}


bool MmsBinaryDecodeDRMMessage(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, unsigned int fullBodyLength, int totalLength)
{
	int offset = 0;
	char szTempFilePath[MSG_FILEPATH_LEN_MAX] = "/User/Msg/Mms/Temp/drm.dm";
	char *pRawData = NULL;
	bool isFileCreated = false;

	offset = _MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;

	if (szFilePath != NULL)
		strncpy(pMsgBody->szOrgFilePath, szFilePath, strlen(szFilePath));
	if (szFilePath != NULL)
		strncpy(pMsgType->szOrgFilePath, szFilePath, strlen(szFilePath));

	pRawData = (char *)malloc(fullBodyLength);
	if (pRawData == NULL) {
		MSG_DEBUG("pRawData alloc FAIL \n");
		goto __CATCH;
	}

	if (MsgFseek(pFile, offset,  SEEK_SET) < 0) {
		MSG_DEBUG("MsgFseek() returns -1 \n");
		goto __CATCH;
	}

	if (MsgReadFile(pRawData, sizeof(char), fullBodyLength, pFile)!= (size_t)fullBodyLength) {
		MSG_DEBUG("FmReadFile() returns false \n");
		goto __CATCH;
	}

	if (MsgOpenCreateAndOverwriteFile(szTempFilePath, pRawData, fullBodyLength) == false) {
		MSG_DEBUG("MsgOpenCreateAndOverwriteFile() returns false \n");
		goto __CATCH;
	}
	isFileCreated = true;

	if (strstr(szTempFilePath, ".dm")) {
		char szConvertedFilePath[MSG_FILEPATH_LEN_MAX] = {0,};

		if (MsgDrmConvertDmtoDcfType(szTempFilePath, szConvertedFilePath)) {
			remove(szTempFilePath);
			memset(szTempFilePath, 0, MSG_FILEPATH_LEN_MAX);
			strncpy(szTempFilePath, szConvertedFilePath, MSG_FILEPATH_LEN_MAX-1);
		}
	}

	if (MsgDRM2GetDRMInfo(szTempFilePath, pMsgType) == false) {
		MSG_DEBUG("MsgDRM2GetDRMInfo() returns false \n");
		goto __CATCH;
	}

	remove(szTempFilePath);
	isFileCreated = false;

	if (MmsBinaryDecodeMovePointer(pFile, offset + fullBodyLength, totalLength) == false)
		goto __CATCH;

__RETURN:

	if (pRawData) {
		free(pRawData);
		pRawData = NULL;
	}
	return true;

__CATCH:
	if (isFileCreated)
		remove(szTempFilePath);

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
				MSG_DEBUG("MmsDrm2ConvertMsgBody: end data part convert(converted data len = %d)\n", nSize);
				// <-- invoking drm agent api, converting data part end

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

	if (hFile != NULL) {
		MsgCloseFile(hFile);
		hFile = NULL;
	}

	remove(szTempFile);
	remove(szTempFilePath);

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

	remove(szTempFile);
	remove(szTempFilePath);
	remove(MMS_DECODE_DRM_CONVERTED_TEMP_FILE);	//remove convertin result if it goes to __CATCH

	return false;
}


/*************************************************************************
 * description : Function for decoding a converted file
 * argument : void
 * return value
    - bool :  result of converting
**************************************************************************/
bool MmsDrm2DecodeConvertedMsg(int msgID, char *pszFullPath)
{
	FILE *hConvertedFile = NULL;
	int	nSize = 0;

	MSG_DEBUG("MmsDrm2DecodeConvertedMsg: start re-decoding~~~~~~\n");

	// free
	_MsgFreeDRMInfo(&mmsHeader.msgType.drmInfo);
	_MsgFreeBody(&mmsHeader.msgBody, mmsHeader.msgType.type);

	_MmsInitHeader();
	_MmsUnregisterDecodeBuffer();

	// start decoding
	_MmsRegisterDecodeBuffer(gszMmsLoadBuf1, gszMmsLoadBuf2, MSG_MMS_DECODE_BUFFER_MAX);

	// open converted file
	if ((hConvertedFile = MsgOpenFile(MMS_DECODE_DRM_CONVERTED_TEMP_FILE, "rb")) == NULL) {
		MSG_DEBUG("MmsDrm2ReDecodeMsg: opening temporary file failed\n");
		goto __CATCH;
	}

	if (MsgGetFileSize(MMS_DECODE_DRM_CONVERTED_TEMP_FILE, &nSize) == false) {
		MSG_DEBUG("MsgGetFileSize: failed\n");
		goto __CATCH;
	}

	if (!MmsBinaryDecodeMsgHeader(hConvertedFile, nSize)) {
		MSG_DEBUG("MmsDrm2ReDecodeMsg: decoding header(binary mode) failed\n");
		goto __CATCH;
	}

	if (!MmsBinaryDecodeMsgBody(hConvertedFile, pszFullPath, nSize)) {
		MSG_DEBUG("MmsDrm2ReDecodeMsg: decoding body failed\n");
		goto __CATCH;
	}

	if (hConvertedFile != NULL) {
		MsgCloseFile(hConvertedFile);
		hConvertedFile = NULL;
	}

	return true;

__CATCH:

	if (hConvertedFile != NULL) {
		MsgCloseFile(hConvertedFile);
		remove(MMS_DECODE_DRM_CONVERTED_TEMP_FILE);
		hConvertedFile = NULL;
	}

	return false;
}


bool MmsDrm2ReadMsgConvertedBody(MSG_MESSAGE_INFO_S *pMsg, bool bSavePartsAsTempFiles, bool bRetrieved, char *retrievedPath)
{
	MmsMsg *pMmsMsg;
	MmsPluginStorage::instance()->getMmsMessage(&pMmsMsg);
	_MmsUnregisterDecodeBuffer();
#ifdef __SUPPORT_DRM__
	_MsgFreeDRMInfo(&pMmsMsg->msgType.drmInfo);
#endif
	_MsgFreeBody(&pMmsMsg->msgBody, pMmsMsg->msgType.type);

	if (_MmsReadMsgBody(pMsg->msgId, bSavePartsAsTempFiles, bRetrieved, retrievedPath) == false) {
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

bool _MmsBinaryDecodeGetOneByte(FILE *pFile, UINT8 *pOneByte, int totalLength)
{
	int length = gMmsDecodeMaxLen - gCurMmsDecodeBuffPos;

	if (pFile == NULL || pOneByte == NULL)
	{
		MSG_DEBUG("_MmsBinaryDecodeGetOneByte: invalid file or buffer\n");
		goto __CATCH;
	}

	if (length < 1) {
		if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
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
bool _MmsBinaryDecodeGetBytes(FILE *pFile, char *szBuff, int bufLen, int totalLength)
{
	int length = gMmsDecodeMaxLen - gCurMmsDecodeBuffPos;
	int i = 0;


	if (pFile == NULL || szBuff == NULL || bufLen == 0 || bufLen > gMmsDecodeMaxLen)
		goto __CATCH;

	memset(szBuff, 0, bufLen);

	if (length < bufLen) {
		if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
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



bool _MmsBinaryDecodeGetLongBytes(FILE *pFile, char *szBuff, int bufLen, int totalLength)
{
	int iPos = 0;

	if (pFile == NULL || szBuff == NULL || bufLen == 0)
		goto __CATCH;

	memset(szBuff, 0, bufLen);

	if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
								   gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
		MSG_DEBUG("_MmsBinaryDecodeGetLongBytes: fail to load to buffer \n");
		goto __CATCH;
	}

	while ((bufLen - iPos) >= gMmsDecodeMaxLen) {
		if (_MmsBinaryDecodeGetBytes(pFile, szBuff + iPos, gMmsDecodeMaxLen, totalLength) == false) {
			MSG_DEBUG("_MmsBinaryDecodeGetLongBytes: 1. _MmsBinaryDecodeGetBytes fail \n");
			goto __CATCH;
		}

		iPos += gMmsDecodeMaxLen;
	}

	if ((bufLen - iPos) > 0) {
		if (_MmsBinaryDecodeGetBytes(pFile, szBuff + iPos, (bufLen - iPos), totalLength) == false) {
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
		if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
									   gpMmsDecodeBuf1, gpMmsDecodeBuf2, gMmsDecodeMaxLen, &gMmsDecodeBufLen, totalLength) == false) {
			MSG_DEBUG("__MmsBinaryDecodeUintvar: fail to load to buffer \n");
			goto __CATCH;
		}
	}

	while (true) {
		if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false)
			goto __CATCH;

		if (oneByte > 0x7f)	{
			iBuff[count++] = oneByte;
		} else {
			iBuff[count++] = oneByte;
			break;
		}

		if (count > 5) {
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
		if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
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

	if (_MmsBinaryDecodeGetBytes(pFile, pData, length + 1, totalLength) == false) {
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

	if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
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
		if (_MmsBinaryDecodeGetBytes(pFile, pData, oneByte + 1, totalLength) == false) {
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

	if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
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

	if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false) {
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

	if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
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

		if (_MmsBinaryDecodeGetBytes(pFile, pData, gMmsDecodeBufLen, totalLength) == false)
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

		if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
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

		if (_MmsBinaryDecodeGetBytes(pFile, pData, length, totalLength) == false)
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

	if (pData) {
		free(pData);
		pData = NULL;
	}

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

	offset = _MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;

	memset(szBuff, 0, bufLen);

	if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
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

		if (_MmsBinaryDecodeGetBytes(pFile, pData, gMmsDecodeBufLen, totalLength) == false)
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

		if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
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

		if (_MmsBinaryDecodeGetBytes(pFile, pData, length, totalLength) == false)
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

	if (pData) {
		free(pData);
		pData = NULL;
	}

	szBuff[0] = '\0';
	length = 0;

	MmsBinaryDecodeMovePointer(pFile, offset, totalLength);

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
	offset = _MmsGetDecodeOffset();
	if (offset >= totalLength)
		goto __RETURN;

	if (MsgLoadDataToDecodeBuffer(pFile, &gpCurMmsDecodeBuff, &gCurMmsDecodeBuffPos, &gMmsDecodeCurOffset,
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

		if (_MmsBinaryDecodeGetBytes(pFile, pData, gMmsDecodeBufLen, totalLength) == false)
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

		if (MsgLoadDataToDecodeBuffer(pFile,
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

		if (_MmsBinaryDecodeGetBytes(pFile, pData, length, totalLength) == false) {
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
	if (pData) {
		free(pData);
		pData = NULL;
	}

	*pLength = 1;

	MmsBinaryDecodeMovePointer(pFile, offset, totalLength);

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
	 *						; Equivalent to the special RFC2616 charset value ¡°*¡±
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

	*nCharSet = _MmsGetBinaryType(MmsCodeCharSet, (UINT16)integer);

	if (*nCharSet == MIME_UNKNOWN) {
		MSG_DEBUG("__MmsBinaryDecodeCharset : _MmsGetBinaryType fail..\n");
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
	int nChar = 0;
	int nRead2 = 0;
	int nByte = 0;
	int nTemp = 0;
	char *pData = NULL;
	char *pTempData = NULL;
	unsigned short *mszTempStr = NULL;
	char *pConvertedStr = NULL;
	char *pNewData = NULL;


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

			if (_MmsBinaryDecodeGetLongBytes(pFile, pData, valueLength - charSetLen, totalLength) == false) {
				MSG_DEBUG("__MmsBinaryDecodeEncodedString : _MmsBinaryDecodeGetLongBytes fail.\n");
				goto __CATCH;
			}

			strncpy(szBuff, pData, bufLen - 1);
		}


		/* fixme: charset transformation */
		switch (charSet) {
		case MSG_CHARSET_UTF16:
		case MSG_CHARSET_USC2:

			MSG_DEBUG("__MmsBinaryDecodeEncodedString: MSG_CHARSET_USC2 \n");

			nTemp = strlen(szBuff);
			pTempData = (char *)malloc(nTemp + 1);
			if (pTempData == NULL) {
				MSG_DEBUG("__MmsBinaryDecodeEncodedString: Memory Full \n");
				goto __CATCH;
			}

			memset(pTempData, 0, nTemp + 1);
			memcpy(pTempData, szBuff, nTemp + 1);

			if (((UINT8)pTempData[0]) == 0xFF && ((UINT8)pTempData[1]) == 0xFE) {
				if ((nChar = (nTemp / 2 - 1)) <= 0)	{
					MSG_DEBUG("__MmsBinaryDecodeEncodedString(%d) : nChar is invalid value (%d), charset(%d)\n", __LINE__, nChar, charSet);
					goto __CATCH ;
				}

				mszTempStr = (unsigned short*) malloc(nChar * sizeof(unsigned short));
				if (mszTempStr == NULL) {
					MSG_DEBUG("MmsGetMediaPartData : 1. Memory Full !!! \n");
					goto __CATCH;
				}

				memcpy(mszTempStr, ((unsigned short*)pTempData + 1), nChar * sizeof(unsigned short));

				nByte = MsgGetUnicode2UTFCodeSize(((unsigned short*)pTempData + 1), nChar);

				pConvertedStr = (char *)malloc(nByte + 1);
				if (pConvertedStr)
					MsgUnicode2UTF ((unsigned char*)pConvertedStr, nByte + 1, mszTempStr, nChar);
			} else {
				if ((nChar = (nTemp / 2)) <= 0) {
					MSG_DEBUG("__MmsBinaryDecodeEncodedString(%d) : nChar is invalid value (%d), charset(%d)\n", __LINE__, nChar, charSet);
					goto __CATCH ;
				}

				mszTempStr = (unsigned short*) malloc(nChar * sizeof(unsigned short));
				if (mszTempStr == NULL) {
					MSG_DEBUG("__MmsBinaryDecodeEncodedString: 2. Memory Full !!! \n");
					goto __CATCH;
				}

				memcpy(mszTempStr, ((unsigned short*)pTempData), nChar * sizeof(unsigned short));

				nByte = MsgGetUnicode2UTFCodeSize(((unsigned short*)pTempData), nChar);

				pConvertedStr = (char *)malloc(nByte + 1);
				if (pConvertedStr != NULL)
					MsgUnicode2UTF ((unsigned char*)pConvertedStr, nByte + 1, mszTempStr, nChar);
			}

			if (pConvertedStr != NULL) {
				pNewData = pConvertedStr;
				nRead2 = nByte;

				strncpy(szBuff, pNewData, bufLen - 1);
			}

			break;

		case MSG_CHARSET_US_ASCII:

			MSG_DEBUG("__MmsBinaryDecodeEncodedString: MSG_CHARSET_US_ASCII \n");

		case MSG_CHARSET_UTF8:

			MSG_DEBUG("__MmsBinaryDecodeEncodedString: MSG_CHARSET_UTF8 or Others \n");

			pNewData = pTempData;
			nRead2 = nTemp;

			break;

		case MSG_CHARSET_ISO_8859_7: /* Greek */

			MSG_DEBUG("__MmsBinaryDecodeEncodedString: MSG_CHARSET_ISO_8859_7 \n");

			nTemp = strlen(szBuff);
			pTempData = (char *)malloc(nTemp + 1);
			if (pTempData == NULL)
			{
				MSG_DEBUG("__MmsBinaryDecodeEncodedString: Memory Full \n");
				goto __CATCH;
			}

			memset(pTempData, 0 , nTemp + 1);
			memcpy(pTempData, szBuff, nTemp + 1);

			nByte = MsgGetLatin72UTFCodeSize((unsigned char*)pTempData, nTemp);
			pConvertedStr = (char *)malloc(nByte + 1);

			if (pConvertedStr != NULL) {
				MsgLatin7code2UTF((unsigned char*)pConvertedStr, nByte + 1, (unsigned char*)pTempData, nTemp);

				pNewData = pConvertedStr;
				nRead2 = nByte;

				strncpy(szBuff, pNewData, bufLen - 1);
			}

			break;

		case MSG_CHARSET_ISO_8859_9: /* Turkish */

			MSG_DEBUG("__MmsBinaryDecodeEncodedString: MSG_CHARSET_ISO_8859_9 \n");

			nTemp = strlen(szBuff);
			pTempData = (char *)malloc(nTemp + 1);
			if (pTempData == NULL) {
				MSG_DEBUG("__MmsBinaryDecodeEncodedString: Memory Full \n");
				goto __CATCH;
			}

			memset(pTempData, 0 , nTemp + 1);
			memcpy(pTempData, szBuff, nTemp + 1);

			nByte = MsgGetLatin52UTFCodeSize((unsigned char*)pTempData, nTemp);
			pConvertedStr = (char *)malloc(nByte + 1);

			if (pConvertedStr != NULL) {
				MsgLatin5code2UTF((unsigned char*)pConvertedStr, nByte + 1, (unsigned char*)pTempData, nTemp);

				pNewData = pConvertedStr;
				nRead2 = nByte;

				strncpy(szBuff, pNewData, bufLen - 1);
			}

			break;

		default:

			MSG_DEBUG("__MmsBinaryDecodeEncodedString: Other charsets \n");

			nTemp = strlen(szBuff);
			pTempData = (char *)malloc(nTemp + 1);
			if (pTempData == NULL) {
				MSG_DEBUG("__MmsBinaryDecodeEncodedString: Memory Full \n");
				goto __CATCH;
			}

			memset(pTempData, 0, nTemp + 1);
			memcpy(pTempData, szBuff, nTemp + 1);

			nByte = MsgGetLatin2UTFCodeSize((unsigned char*)pTempData, nTemp);
			pConvertedStr = (char *)malloc(nByte + 1);

			if (pConvertedStr != NULL) {
				MsgLatin2UTF((unsigned char*)pConvertedStr, nByte + 1, (unsigned char*)pTempData, nTemp);

				pNewData = pConvertedStr;
				nRead2 = nByte;

				strncpy(szBuff, pNewData, bufLen - 1);
			}

			break;

		} //switch (charset)

	} //switch (__MmsDecodeValueLength....)

	if (pData) {
		free(pData);
		pData = NULL;
	}

	if (pTempData) {
		free(pTempData);
		pTempData = NULL;
	}

	if (mszTempStr) {
		free(mszTempStr);
		mszTempStr = NULL;
	}

	if (pConvertedStr) {
		free(pConvertedStr);
		pConvertedStr = NULL;
	}


	return true;

__CATCH:

	if (pData) {
		free(pData);
		pData = NULL;
	}

	if (pTempData) {
		free(pTempData);
		pTempData = NULL;
	}

	if (mszTempStr) {
		free(mszTempStr);
		mszTempStr = NULL;
	}

	if (pConvertedStr) {
		free(pConvertedStr);
		pConvertedStr = NULL;
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

			if (_MmsBinaryDecodeGetLongBytes(pFile, pAddrStr, valueLength - charSetLen, totalLength) == false) {
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

	if (_MmsBinaryDecodeGetOneByte(pFile, &oneByte, totalLength) == false)
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

		if (MsgIsUTF8String((unsigned char*)pLatinBuff, strlen(pLatinBuff)) == false) {
			length = strlen(pLatinBuff);

			int		utf8BufSize = 0;
			utf8BufSize = MsgGetLatin2UTFCodeSize((unsigned char*)pLatinBuff, length);
			pUTF8Buff = (char *)malloc(utf8BufSize + 1);
			if (pUTF8Buff == NULL) {
				MSG_DEBUG("__MmsDecodeGetFilename: pUTF8Buff alloc fail \n");
				goto __CATCH;
			}

			if (MsgLatin2UTF ((unsigned char*)pUTF8Buff, utf8BufSize + 1, (unsigned char*)pLatinBuff, length) < 0) {
				MSG_DEBUG("__MmsDecodeGetFilename: MsgLatin2UTF fail \n");
				goto __CATCH;
			}
			free(pLatinBuff);
			pLatinBuff = NULL;
		} else {
			pTmpBuff = _MsgDecodeText(pLatinBuff);
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

	   T  E  X  T         D  E  C  O  D  I  N  G

   ==========================================================*/


bool MmsTextDecodeMsgBody(FILE *pFile)
{
	MSG_DEBUG("MmsTextDecodeMsgBody: \n");
	return false;
}

bool __MmsTextDecodeMsgHeader(FILE *pFile)
{
	return true;
}


/* ==========================================================

	   M  M  S        D  E  C  O  D  I  N  G

   ==========================================================*/

//  to get message body this function should be modified from message raw file.
bool _MmsReadMsgBody(MSG_MESSAGE_ID_T msgID, bool bSavePartsAsTempFiles, bool bRetrieved, char *retrievedPath)
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

	_MmsInitHeader();

	if (bRetrieved && (retrievedPath != NULL)) {
		strncpy(szFullPath, retrievedPath, (strlen(retrievedPath) > MSG_FILEPATH_LEN_MAX ? MSG_FILEPATH_LEN_MAX:strlen(retrievedPath)));
	} else {
		MmsPluginStorage::instance()->getMmsRawFilePath(msgID, szFullPath);
	}

	pMsg->msgID = msgID;

	/*	read from MMS raw file	*/
	if (szFullPath != NULL)
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

	_MmsRegisterDecodeBuffer(gszMmsLoadBuf1, gszMmsLoadBuf2, MSG_MMS_DECODE_BUFFER_MAX);

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

	pMsg->mmsAttrib.contentType = (MsgContentType)mmsHeader.msgType.type;

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
					_MsgFreeBody(pMultipart->pNext->pBody, pMultipart->pNext->type.type);

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

	///////////////////////////////////////////////
	// call before processing urgent event.
	//_MmsInitHeader();
	//_MmsUnregisterDecodeBuffer();
	///////////////////////////////////////////////

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

			if (_MmsMultipartSaveAsTempFile(&pMultipart->type, pMultipart->pBody,
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

			if (_MmsMultipartSaveAsTempFile( &pMsg->msgType, &pMsg->msgBody,
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

	_MmsInitHeader();
	_MmsUnregisterDecodeBuffer();

	if (pFile != NULL) {
		MsgCloseFile(pFile);
		pFile = NULL;
	}

#ifdef __SUPPORT_DRM__
	_MsgFreeDRMInfo(&pMsg->msgType.drmInfo);
#endif

	_MsgFreeBody(&pMsg->msgBody, pMsg->msgType.type);
	MSG_DEBUG("_MmsReadMsgBody:    E  N  D    (fail)    ******************** \n");

	return false;
}


bool MsgFreeHeaderAddress(MsgHeaderAddress *pAddr)
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

bool MsgCheckFileNameHasInvalidChar(char *szName)
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

bool _MsgReplaceInvalidFileNameChar(char *szInText, char replaceChar)
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

char *_MsgGetStringUntilDelimiter(char *pszString, char delimiter)
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

				if (MsgIsHexChar(szBuf) == true) { // check the two character is between  0 ~ F
					OneChar = _MsgConvertHexValue(szBuf);

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

bool _MsgParseParameter(MsgType *pType, char *pSrc)
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
		pSrc = _MsgSkipWS(pSrc);
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

				pDec = _MsgDecodeText(pValue);		// Api is to long, consider Add to another file (MsgMIMECodec.c)
			} else {
				pDec = _MsgDecodeText(pName);
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

				pUTF8Buff = MsgConvertLatin2UTF8FileName(pDec);

				if (pUTF8Buff) {
					if ((pExt = strrchr(pUTF8Buff, '.')) != NULL) {
						if ((MSG_FILENAME_LEN_MAX-1) < strlen(pUTF8Buff)) {
							nameLen = (MSG_FILENAME_LEN_MAX-1) - strlen(pExt);
						} else {
							nameLen = strlen(pUTF8Buff) - strlen(pExt);
						}

						strncpy(pType->param.szName, pUTF8Buff, nameLen);
						strcat (pType->param.szName, pExt);
					} else {
						strncpy(pType->param.szName, pUTF8Buff, (MSG_FILENAME_LEN_MAX-1));
					}
					free(pUTF8Buff);
					pUTF8Buff = NULL;

					if (_MsgChangeSpace(pType->param.szName, &szSrc) == true) {
						if (szSrc)
							strncpy(pType->param.szName, szSrc , strlen(szSrc));
					}

					if (szSrc) {
						free(szSrc);
						szSrc = NULL;
					}

					// Remvoe '/', ex) Content-Type: image/gif; name="images/vf7.gif"
					_MsgRemoveFilePath(pType->param.szName);
				} else {
					MSG_DEBUG("_MsgParseParameter: MsgConvertLatin2UTF8FileName(%s) return NULL\n", pDec);
				}

				MSG_DEBUG("_MsgParseParameter: szName = %s \n", pType->param.szName);
				break;

			case MSG_PARAM_FILENAME:

				memset (pType->param.szFileName, 0, MSG_FILENAME_LEN_MAX+1);

				pUTF8Buff = MsgConvertLatin2UTF8FileName(pDec);

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

					if (_MsgChangeSpace(pType->param.szFileName, &szSrc) == true)
						strcpy(pType->param.szFileName, szSrc);

					if (szSrc) {
						free(szSrc);
						szSrc = NULL;
					}

					// Remvoe '/', ex) Content-Type: image/gif; name="images/vf7.gif"
					_MsgRemoveFilePath(pType->param.szFileName);
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

char *_MsgSkipWS(char *s)
{
	while (true) {
		if ((*s == MSG_CH_CR) || (*s == MSG_CH_LF) || (*s == MSG_CH_SP)	|| (*s == MSG_CH_TAB)) {
			++s;
		} else if ((*s != '(') || (__MsgSkipComment(s,(long)NULL)==NULL)) {
			return s;
		}
	}
}

char *__MsgSkipComment (char *s,long trim)
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

char *MsgConvertLatin2UTF8FileName(char *pSrc)
{
	char *pUTF8Buff  = NULL;
	char *pData = NULL;


	//convert utf8 string
	if (MsgIsUTF8String((unsigned char*)pSrc, strlen(pSrc)) == false) {
		int length  = 0;
		int utf8BufSize = 0;

		length = strlen(pSrc);
		utf8BufSize = MsgGetLatin2UTFCodeSize((unsigned char*)pSrc, length);
		pUTF8Buff = (char *)malloc(utf8BufSize + 1);

		if (pUTF8Buff == NULL) {
			MSG_DEBUG("MsgConvertLatin2UTF8FileName: pUTF8Buff alloc fail \n");
			goto __CATCH;
		}

		if (MsgLatin2UTF ((unsigned char*)pUTF8Buff, utf8BufSize + 1, (unsigned char*)pSrc, length) < 0) {
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
	if (MsgIsPercentSign(pUTF8Buff) == true) {
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
	if (pData) {
		free(pData);
		pData = NULL;
	}
	return NULL;
}

bool _MsgChangeSpace(char *pOrg, char **ppNew)
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

void _MsgRemoveFilePath(char *pSrc)
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

bool MsgIsUTF8String(unsigned char *szSrc, int nChar)
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

bool MsgIsPercentSign(char *pSrc)
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

MsgMultipart *MsgAllocMultipart(void)
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

	_MsgInitMsgType(&pMultipart->type);
	_MsgInitMsgBody(pMultipart->pBody);

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

bool _MsgInitMsgType(MsgType *pMsgType)
{
	pMsgType->offset = 0;
	pMsgType->size = 0;
	pMsgType->contentSize = 0;
	pMsgType->disposition = 0;
	pMsgType->encoding = 0;
	pMsgType->type = MIME_UNKNOWN;
#ifdef FEATURE_JAVA_MMS_MIME
	pMsgType->szMimeString[0] ='\0';
#endif
	pMsgType->section = 0;

	pMsgType->szOrgFilePath[0] = '\0';
	pMsgType->szContentID[0] = '\0';
	pMsgType->szContentLocation[0] = '\0';

	pMsgType->szContentRepPos[0] = '\0';
	pMsgType->szContentRepSize[0] = '\0';
	pMsgType->szContentRepIndex[0] = '\0';

	__MsgInitMsgContentParam(&pMsgType->param);
#ifdef __SUPPORT_DRM__
	__MsgInitMsgDRMInfo(&pMsgType->drmInfo);
#endif

	return true;
}


bool __MsgInitMsgContentParam(MsgContentParam *pMsgContentParam)
{
	pMsgContentParam->charset = MSG_CHARSET_UNKNOWN;
	pMsgContentParam->type = MIME_UNKNOWN;
	pMsgContentParam->szBoundary[0] = '\0';
	pMsgContentParam->szFileName[0] = '\0';
	pMsgContentParam->szName[0] = '\0';
#ifdef FEATURE_JAVA_MMS
	pMsgContentParam->szApplicationID = NULL;
	pMsgContentParam->szReplyToApplicationID = NULL;
#endif
	pMsgContentParam->szStart[0] = '\0';
	pMsgContentParam->szStartInfo[0] = '\0';
	pMsgContentParam->pPresentation = NULL;

	pMsgContentParam->reportType = MSG_PARAM_REPORT_TYPE_UNKNOWN; //  add only used as parameter of Content-Type: multipart/report; report-type

	return true;
}


bool _MsgInitMsgBody(MsgBody *pMsgBody)
{
	pMsgBody->offset = 0;
	pMsgBody->size = 0;
	pMsgBody->body.pText = NULL;
	pMsgBody->szOrgFilePath[0] = '\0';

	_MsgInitMsgType(&pMsgBody->presentationType);
	pMsgBody->pPresentationBody = NULL;

	memset(pMsgBody->szOrgFilePath, 0, MSG_FILEPATH_LEN_MAX);

	return true;
}


MsgPresentationFactor MsgIsPresentationEx(MsgType *multipartType, char* szStart, MsgContentType typeParam)
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

void MsgConfirmPresentationPart(MsgType *pMsgType, MsgBody *pMsgBody, MsgPresentaionInfo *pPresentationInfo)
{
	MSG_BEGIN();
	MsgMultipart *pNextPart = NULL;
	MsgMultipart *pRemovePart = NULL;

	if (MsgIsMultipartRelated(pMsgType->type)) {
		// assign the multipart to presentation part
		// remove the multipart(pCurPresentation) which is presentation part from the linked list.
		// if there is no presentation part -> assign first multipart to presentation part by force.
		if (pPresentationInfo->pCurPresentation == NULL) {
			pPresentationInfo->pCurPresentation	= pMsgBody->body.pMultipart;
			pPresentationInfo->pPrevPart		= NULL;
			pPresentationInfo->factor			= MSG_PRESENTATION_NONE;
		}

		if (pPresentationInfo->pCurPresentation != NULL && MsgIsPresentablePart(pPresentationInfo->pCurPresentation->type.type)) {
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
					_MsgFreeDRMInfo(&pPresentationInfo->pCurPresentation->type.drmInfo);
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
		} else if (pPresentationInfo->pCurPresentation != NULL && MsgIsText(pPresentationInfo->pCurPresentation->type.type)) {
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
					_MsgFreeBody(pRemovePart->pBody, pRemovePart->type.type);
					free(pRemovePart->pBody);
					pRemovePart->pBody = NULL;
				}

				free(pRemovePart);
				pRemovePart = NULL;
			}
		} else {
#ifdef __SUPPORT_DRM__
			_MsgFreeDRMInfo(&pMsgBody->presentationType.drmInfo);
#endif
			_MsgInitMsgType(&pMsgBody->presentationType);
			pMsgBody->pPresentationBody = NULL;
		}
	}
	MSG_END();
}

bool MsgIsMultipartRelated(int type)
{
	if (type == MIME_MULTIPART_RELATED || type == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {
		return true;
	} else {
		return false;
	}
}

bool MsgIsPresentablePart(int type)
{
	if (type == MIME_TEXT_HTML || type == MIME_TEXT_VND_WAP_WML || type == MIME_APPLICATION_SMIL) {
		return true;
	} else {
		return false;
	}
}

#ifdef __SUPPORT_DRM__
void _MsgFreeDRMInfo(MsgDRMInfo *pDrmInfo)
{
	MSG_DEBUG("_MsgFreeDRMInfo: S T A R T  !!! \n");

	if (pDrmInfo == NULL) {
		MSG_DEBUG("pDrmInfo is NULL");
		return;
	}

	if (pDrmInfo->szContentDescription) {
		free(pDrmInfo->szContentDescription);
		pDrmInfo->szContentDescription = NULL;
	}

	if (pDrmInfo->szContentVendor) {
		free(pDrmInfo->szContentVendor);
		pDrmInfo->szContentVendor = NULL;
	}

	if (pDrmInfo->szContentName) {
		free(pDrmInfo->szContentName);
		pDrmInfo->szContentName = NULL;
	}

	if (pDrmInfo->szContentURI) {
		free(pDrmInfo->szContentURI);
		pDrmInfo->szContentURI = NULL;
	}

	if (pDrmInfo->szRightIssuer) {
		free(pDrmInfo->szRightIssuer);
		pDrmInfo->szRightIssuer = NULL;
	}

	if (pDrmInfo->szDrm2FullPath) {
		free(pDrmInfo->szDrm2FullPath);
		pDrmInfo->szDrm2FullPath = NULL;
	}

#if MMS_ENABLE_EXTEND_CFM
	// DRM (Extended)CFM
	if (pDrmInfo->pszContentType) {
		free(pDrmInfo->pszContentType);
		pDrmInfo->pszContentType = NULL;
	}
#endif

	pDrmInfo->contentType = MIME_UNKNOWN;
	pDrmInfo->drmType = MSG_DRM_TYPE_NONE;
}

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
			MsgMakeFileName(pPartType->type, pPartType->param.szName, MSG_DRM_TYPE_NONE, 0);
		}
	}

	return true;
}

#endif

bool MsgIsText(int type)
{
	if (type == MIME_TEXT_PLAIN || type == MIME_TEXT_HTML || type == MIME_TEXT_VND_WAP_WML ||
		type == MIME_TEXT_X_VNOTE || type == MIME_APPLICATION_SMIL || type == MIME_TEXT_X_IMELODY) {
		return true;
	} else {
		return false;
	}
}

bool MsgFreeAttrib(MmsAttrib *pAttrib)
{
	MSG_BEGIN();

	if (pAttrib == NULL) {
		MSG_DEBUG("pAttrib is NULL");
		return false;
	}

	if (pAttrib->szTo) {
		free(pAttrib->szTo);
		pAttrib->szTo = NULL;
	}

	if (pAttrib->szCc) {
		free(pAttrib->szCc);
		pAttrib->szCc = NULL;
	}

	if (pAttrib->szBcc) {
		free(pAttrib->szBcc);
		pAttrib->szBcc = NULL;
	}

	//check if pMultiStatus should be freed or not, because pMultiStatus is not allocated
	if (pAttrib->pMultiStatus) {
		MmsMsgMultiStatus *pMultiStatus = pAttrib->pMultiStatus;
		MmsMsgMultiStatus *pCurStatus = NULL;

		while (pMultiStatus != NULL ) {
			pCurStatus = pMultiStatus;
			pMultiStatus = pMultiStatus->pNext;

			if (pCurStatus) {
				free(pCurStatus);
				pCurStatus = NULL;
			}
		}

		pAttrib->pMultiStatus = NULL;
	}


	MSG_END();

	return true;
}

bool _MsgFreeBody(MsgBody *pBody, int type)
{
	MSG_DEBUG("_MsgFreeBody: S T A R T  !!! \n") ;

	if (pBody == NULL) {
		MSG_DEBUG("_MsgFreeBody: pBody == NULL \n" );
		MSG_DEBUG("_MsgFreeBody: E  N  D   (End)!!! \n") ;

		return false;
	}

	switch (type) {
	case MIME_MULTIPART_REPORT:
	case MIME_APPLICATION_VND_OMA_DRM_MESSAGE:
	case MIME_APPLICATION_VND_WAP_MULTIPART_MIXED:
	case MIME_APPLICATION_VND_WAP_MULTIPART_RELATED:
	case MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC:
	case MIME_MULTIPART_MIXED:
	case MIME_MULTIPART_RELATED:
	case MIME_MULTIPART_ALTERNATIVE:
	case MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE:
		{
			MsgMultipart *pMulti = pBody->body.pMultipart;
			MsgMultipart *pCurrPart = NULL;
			MsgBody *pPresentation = pBody->pPresentationBody;
			while (pMulti != NULL) {
				pCurrPart = pMulti;

				pMulti = pMulti->pNext;

				if (pCurrPart) {
#ifdef __SUPPORT_DRM__
					_MsgFreeDRMInfo(&pCurrPart->type.drmInfo);
#endif

					if (pCurrPart->pBody) {
						if (pCurrPart->pBody->body.pBinary) {
							free(pCurrPart->pBody->body.pBinary);
							pCurrPart->pBody->body.pBinary = NULL;
						}
						free(pCurrPart->pBody);
						pCurrPart->pBody = NULL;
					}
					free(pCurrPart);
					pCurrPart = NULL;
				}
			}

			pBody->body.pMultipart = NULL;

			if (pPresentation) {
				if (pPresentation->body.pText) {
					free(pPresentation->body.pText);
					pPresentation->body.pText = NULL;
				}
				free(pPresentation);
				pBody->pPresentationBody = NULL;
			}

			_MsgInitMsgType(&pBody->presentationType);

			break;
		}

	default:
		/* Any single part */

		if (pBody->body.pBinary) {
			free(pBody->body.pBinary);
			pBody->body.pBinary = NULL;
		}

		break;
	}

	MSG_DEBUG("_MsgFreeBody: E  N  D  (Successfully) !!! \n") ;

	return true;

}

bool MsgResolveNestedMultipart(MsgType *pPartType, MsgBody *pPartBody)
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
			_MsgFreeDRMInfo(&pRemoveList->type.drmInfo);
#endif
			_MsgFreeBody(pRemoveList->pBody, pRemoveList->type.type);

			free(pRemoveList->pBody);
			free(pRemoveList);
		}

		if (MsgCopyNestedMsgType(pPartType, &(pSelectedPart->type)) == false) {
			MSG_DEBUG("MsgResolveNestedMultipart : MsgPriorityCopyMsgType failed \n");
			goto __CATCH;
		}

		if (pSelectedPart->pBody != NULL)
			memcpy(pPartBody, pSelectedPart->pBody, sizeof(MsgBody));

		if (pSelectedPart != NULL) {
#ifdef __SUPPORT_DRM__
			_MsgFreeDRMInfo(&pSelectedPart->type.drmInfo);
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
			if (MsgIsMultipartMixed(pSelectedPart->type.type)) {

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
					_MsgFreeDRMInfo(&pSelectedPart->type.drmInfo);
#endif
					free(pSelectedPart->pBody);
					free(pSelectedPart);
				}
				pSelectedPart = pTmpMultipart;
			} else if (MsgIsMultipartRelated(pSelectedPart->type.type) && pPrevPart != NULL) {
				pPrevPart->pNext = pTmpMultipart = pSelectedPart->pNext;
				_MsgFreeBody(pSelectedPart->pBody, pSelectedPart->type.type);

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
				_MsgFreeDRMInfo(&pSelectedPart->type.drmInfo);
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
			MSG_DEBUG("MsgResolveNestedMultipart : MIME_MULTIPART_REPORT [no selected part]\n");

			pRemoveList = pPartBody->body.pMultipart->pNext;
			pSelectedPart = pPartBody->body.pMultipart;
			pSelectedPart->pNext = NULL;
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
			_MsgFreeDRMInfo(&pTmpMultipart->type.drmInfo);
#endif
			_MsgFreeBody(pTmpMultipart->pBody, pTmpMultipart->type.type);
			pNextRemovePart = pTmpMultipart->pNext;

			free(pTmpMultipart->pBody);
			free(pTmpMultipart);
			pTmpMultipart = pNextRemovePart;
		}

		if (MsgCopyNestedMsgType(pPartType, &(pSelectedPart->type)) == false) {
			MSG_DEBUG("MsgResolveNestedMultipart : MsgPriorityCopyMsgType failed \n");
			goto __CATCH;
		}

		if (pSelectedPart->pBody != NULL)
			memcpy(pPartBody, pSelectedPart->pBody, sizeof(MsgBody));

		if (pSelectedPart != NULL) {
#ifdef __SUPPORT_DRM__
			_MsgFreeDRMInfo(&pSelectedPart->type.drmInfo);
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
	if (szTemp) {
		free(szTemp);
		szTemp = NULL;
	}

	return NULL;
}

bool _MsgParsePartHeader(MsgType *pType, const char *pRawData, int nRawData)
{
	char ch = '\0';
	int cRaw = nRawData;
	char *pFieldLine = NULL;
	char *pFieldValue = NULL;
	char *szFieldValue = NULL;
	char *szSrc = NULL;
	int nLen = 0;
	char fieldLine[MSG_LINE_LEN] = {0, }; //temporary line buffer
	int length = 0;

	char szTempBuf[MSG_LOCAL_TEMP_BUF_SIZE];

	while ((cRaw > 0) && *pRawData != MSG_CH_LF) {
		memset(fieldLine, 0, MSG_LINE_LEN);
		pFieldLine = fieldLine;

		ch = MSG_CH_SP; //remember previous character.

		while (ch) {
			switch (ch = *pRawData++) {
			case MSG_CH_CR: //'\r'
				if (*pRawData == MSG_CH_LF)
					break;

			case MSG_CH_LF: //'\n'
				if (*pRawData != MSG_CH_SP && *pRawData != MSG_CH_TAB) {
					ch = MSG_CH_NULL;
					*pFieldLine = ch;
					pFieldLine++;
				}
				break;

			case MSG_CH_TAB: //'\t'
				*pFieldLine = MSG_CH_SP;
				pFieldLine++;
				break;

			default:
				*pFieldLine = ch;
				pFieldLine++;
				break;
			}

			if (--cRaw <= 0) {  //If is the last character of header
				*pFieldLine = MSG_CH_NULL;
				pFieldLine++;
			}
		}


		if ((pFieldValue =  strchr(fieldLine, MSG_CH_COLON)) != NULL) {
			char*	pTemp = pFieldValue;
			char*	pValue = NULL;

			*pFieldValue++ = MSG_CH_NULL; //remove ':'

			while (*pFieldValue == MSG_CH_SP) //remove space ' '
				pFieldValue++;

			while ((fieldLine < pTemp--) && (*pTemp == MSG_CH_SP))
				*pTemp = MSG_CH_NULL;

			nLen = strlen(pFieldValue);

			if (nLen >= MSG_LOCAL_TEMP_BUF_SIZE) {
				szFieldValue = (char *)malloc(nLen + 1);
				memset(szFieldValue, 0 , nLen + 1);
				memcpy(szFieldValue, pFieldValue, nLen + 1);
			} else {
				memset(szTempBuf, 0, MSG_LOCAL_TEMP_BUF_SIZE);
				strcpy(szTempBuf, pFieldValue);
				szFieldValue = szTempBuf;
			}

			if (MsgStrcpyWithoutCRLF(szFieldValue, &szSrc) == true) {
				if (szSrc != NULL)
					strncpy(szFieldValue, szSrc, strlen(szSrc));
			}
			if (szSrc) {
				free(szSrc);
				szSrc = NULL;
			}

			switch (_MsgGetCode(MSG_FIELD, fieldLine)) {
			case MSG_FIELD_CONTENT_ID:
				if ((pTemp = strchr(szFieldValue, MSG_CH_SEMICOLON)) != NULL)
					*pTemp++ = MSG_CH_NULL;

				pValue = _MsgDecodeText(szFieldValue);
				memset (pType->szContentID, 0, MSG_MSG_ID_LEN + 1);

				if (pValue) {
					// support CD & SD
					length = MsgStrlen(pValue);
					if (pValue[0] == '<' && pValue[length-1] == '>') {
						strncpy(pType->szContentID, pValue + 1, length - 2);
					} else {
						strncpy(pType->szContentID, pValue, MSG_MSG_ID_LEN);
					}

					MSG_DEBUG("_MsgParsePartHeader: Content-ID = %s     %s\n", pFieldValue, pType->szContentID);

					if (pTemp != NULL)
						_MsgParseParameter(pType, pTemp);

					free(pValue);
					pValue = NULL;
				}

				break;

			case MSG_FIELD_CONTENT_LOCATION:
				if ((pTemp = strchr(szFieldValue, MSG_CH_SEMICOLON)) != NULL)
					*pTemp++ = MSG_CH_NULL;

				pValue = _MsgDecodeText(szFieldValue);
				memset (pType->szContentLocation, 0, MSG_MSG_ID_LEN + 1);

				if (pValue) {
					strncpy(pType->szContentLocation, pValue, MSG_MSG_ID_LEN);

					MSG_DEBUG("_MsgParsePartHeader: Content-Location = %s     %s\n", pFieldValue, pType->szContentLocation);

					if (pTemp != NULL)
						_MsgParseParameter(pType, pTemp);

					free(pValue);
					pValue = NULL;
				}

				break;

			case MSG_FIELD_CONTENT_TYPE:
				if ((pTemp = strchr(szFieldValue, MSG_CH_SEMICOLON)) == NULL) {
					if ((pTemp = strchr(szFieldValue, MSG_CH_SP)) != NULL)
						*pTemp++ = MSG_CH_NULL;
				} else {
					*pTemp++ = MSG_CH_NULL;
				}

				pType->type = _MsgGetCode(MSG_TYPE, szFieldValue);

				MSG_DEBUG("_MsgParsePartHeader: Content-Type = %s     %d\n", pFieldValue, pType->type);

				if (pType->type == INVALID_HOBJ)
					pType->type = MIME_UNKNOWN;

				if (pTemp != NULL)
					_MsgParseParameter(pType, pTemp);

				break;

			case MSG_FIELD_CONTENT_TRANSFER_ENCODING:
				if ((pTemp = strchr(szFieldValue, MSG_CH_SEMICOLON)) == NULL) {
					if ((pTemp = strchr(szFieldValue, MSG_CH_SP)) != NULL)
						*pTemp++ = MSG_CH_NULL;
				} else {
					*pTemp++ = MSG_CH_NULL;
				}

				pType->encoding = _MsgGetCode(MSG_ENCODING, szFieldValue);

				MSG_DEBUG("_MsgParsePartHeader: Content-Encoding = %s     %d\n", pFieldValue, pType->encoding);

				if (pTemp != NULL)
					_MsgParseParameter(pType, pTemp);

				break;

			case MSG_FIELD_CONTENT_DISPOSITION:
				if ((pTemp = strchr(szFieldValue, MSG_CH_SEMICOLON)) == NULL) {
					if ((pTemp = strchr(szFieldValue, MSG_CH_SP)) != NULL)
						*pTemp++ = MSG_CH_NULL;
				} else {
					*pTemp++ = MSG_CH_NULL;
				}

				pType->disposition = _MsgGetCode(MSG_DISPOSITION, szFieldValue);

				MSG_DEBUG("_MsgParsePartHeader: Content-Disposition = %s     %d\n", pFieldValue, pType->disposition);

				if (pTemp != NULL)
					_MsgParseParameter(pType, pTemp);

				break;

#ifdef __SUPPORT_DRM__

			case MSG_FIELD_CONTENT_NAME:
				/* add to parse DCF header such as,
				 * Right-Issuer, Content-Name, and Content-Description.
				 */
				MsgMIMERemoveQuote(szFieldValue);
				pType->drmInfo.szContentName = MsgStrCopy(szFieldValue);
				break;

			case MSG_FIELD_CONTENT_DESCRIPTION:
				MsgMIMERemoveQuote(szFieldValue);
				pType->drmInfo.szContentDescription = MsgStrCopy(szFieldValue);
				break;

			case MSG_FIELD_CONTENT_VENDOR:
				MsgMIMERemoveQuote(szFieldValue);
				pType->drmInfo.szContentVendor = MsgStrCopy(szFieldValue);
				break;

			case MSG_FIELD_RIGHT_ISSUER:
				MsgMIMERemoveQuote(szFieldValue);
				pType->drmInfo.szRightIssuer = MsgStrCopy(szFieldValue);
				break;


			case MSG_FIELD_DRM_CONVERTED:
				MsgMIMERemoveQuote(szFieldValue);
				pType->nDrmConvertedStatus = atoi(szFieldValue);
				break;

#endif
			case MSG_FILED_CONTENT_REPLACE_POS:
				MsgMIMERemoveQuote(szFieldValue);
				strncpy(pType->szContentRepPos, szFieldValue, sizeof(pType->szContentRepPos));
				break;

			case MSG_FILED_CONTENT_REPLACE_SIZE:
				MsgMIMERemoveQuote(szFieldValue);
				strncpy(pType->szContentRepSize, szFieldValue, sizeof(pType->szContentRepSize));
				break;

			case MSG_FILED_CONTENT_REPLACE_INDEX:
				MsgMIMERemoveQuote(szFieldValue);
				strncpy(pType->szContentRepIndex, szFieldValue, sizeof(pType->szContentRepIndex));
				break;

			default:
				MSG_DEBUG("_MsgParsePartHeader: Unhandled field \n");
				break;
			}
		}

		if (szFieldValue != NULL && szFieldValue != szTempBuf) {
			free(szFieldValue);
			szFieldValue = NULL;
		}

	}

	return true;
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


bool MsgIsHexChar(char *pSrc)
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

char _MsgConvertHexValue(char *pSrc)
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

int __MsgConvertCharToInt(char ch)
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

bool MsgCopyNestedMsgType(MsgType *pMsgType1, MsgType *pMsgType2)
{
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

	MsgCopyNestedMsgParam(&(pMsgType1->param), &(pMsgType2->param));

	if (pMsgType1->param.szName[0]) {
#ifdef __SUPPORT_DRM__
		pMsgType1->drmInfo.szContentName = MsgStrCopy(pMsgType2->param.szName);
#endif
	}

	return true;
}


bool MsgCopyNestedMsgParam(MsgContentParam *pParam1, MsgContentParam *pParam2)
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

bool MsgIsMultipartMixed(int type)
{
	if (type == MIME_APPLICATION_VND_WAP_MULTIPART_MIXED || type == MIME_MULTIPART_MIXED) {
		return true;
	} else {
		return false;
	}
}

bool MsgStrcpyWithoutCRLF(char *pOrg, char **ppNew)
{
	int nLen = 0;
	int	i = 0;
	int index = 0;
	char*	pDest = NULL;


	nLen = strlen(pOrg);

	pDest = (char *)malloc(nLen + 1);

	if (pDest == NULL) {
		MSG_DEBUG("MsgStrcpyWithoutCRLF: malloc is failed\n");
		return false;
	}

	memset(pDest, 0 , (nLen + 1));

	for (i = 0; i < nLen ; i++) {
		if (i < nLen - 2) {
			if ((pOrg[i] == MSG_CH_CR && pOrg[i+1] == MSG_CH_LF && pOrg[i+2] == MSG_CH_TAB) ||
				(pOrg[i] == MSG_CH_CR && pOrg[i+1] == MSG_CH_LF && pOrg[i+2] == MSG_CH_SP)) {
				i+=2;
				continue;
			}
		}
		pDest[index++] = pOrg[i];
	}
	*ppNew = pDest;

	return true;
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

bool __MsgIsInvalidFileNameChar(char ch)
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

bool _MmsDataUpdateLastStatus(MmsMsg *pMsg)
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

int MsgGetLatin2UTFCodeSize(unsigned char *szSrc, int nChar)
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

int MsgLatin5code2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar)
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

int MsgGetLatin52UTFCodeSize(unsigned char *szSrc, int nChar)
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

int MsgLatin2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar)
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


int MsgLatin7code2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar)
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

int MsgGetLatin72UTFCodeSize(unsigned char *szSrc, int nChar)
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

int MsgUnicode2UTF(unsigned char *des, int outBufSize, unsigned short *szSrc, int nChar)
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

int MsgGetUnicode2UTFCodeSize(unsigned short *szSrc, int nChar)
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
		MSG_DEBUG("MmsAddrUtilRemovePlmnString: pszAddr is null or zero\n");
		return false;
	}

	strLen = strlen(pszAddr);

	pszAddrCopy = (char*)malloc(strLen + 1);
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

		if (MmsAddrUtilCheckEmailAddress(pszAddrCopy))
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

int MsgCutUTFString(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar)
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

void MsgMIMERemoveQuote(char *szSrc)
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

bool MsgLoadDataToDecodeBuffer(FILE *pFile, char **ppBuf, int *pPtr, int *pOffset, char *pInBuf1, char *pInBuf2, int maxLen, int *pBufLen, int endOfFile)
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
bool _MmsMultipartSaveAsTempFile(MsgType *pPartType, MsgBody *pPartBody, char *pszMailboxPath, char *pszMsgFilename, int index, bool bSave)
{
	FILE *pFile = NULL;
	char *pExt = NULL;
	char szFileName[MSG_FILENAME_LEN_MAX+1] = {0, };	// file name of temp file
	char szFullPath[MSG_FILEPATH_LEN_MAX] = {0, }; // full absolute path of temp file.

	MSG_DEBUG("****   _MmsSaveMediaData:  [Multi part]  START  ***\n");

	if (!pPartType) {
		MSG_DEBUG("pPartType is NULL\n");
		return true; // why true value is retured ??? ; false;
	}

	if (pPartType->param.szName[0] == '\0' && pPartType->param.szFileName[0] == '\0')
		strcpy(pPartType->param.szName, pPartType->param.szFileName);

	if (pPartType->param.szName) {
		strcpy(szFileName, pPartType->param.szName);
	} else {
		snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%lu", (unsigned long)index);
	}


#ifndef __SUPPORT_DRM__
	MsgMakeFileName(pPartType->type, szFileName, 0);	//FL & CD -> extension(.dm) SD -> extension(.dcf)
#else
	MsgMakeFileName(pPartType->type, szFileName, pPartType->drmInfo.drmType, 0);	//FL & CD -> extension(.dm) SD -> extension(.dcf)
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

		if (MmsGetMediaPartData(pPartType, pPartBody, pFile) == false) {
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



bool MmsGetMediaPartData(MsgType *pPartType, MsgBody *pPartBody, FILE* pFile)
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

	pNewData = MmsGetBinaryUTF8Data(pData, nRead, msgEncodingValue, msgTypeValue, msgCharsetValue, &nRead2);
	pPartType->encoding = MSG_ENCODING_BINARY;

	if (MsgIsText(msgTypeValue))
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

char *MmsGetBinaryUTF8Data(char *pData, int nRead, int msgEncodingValue, int msgTypeValue, int msgCharsetValue, int *npRead)
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

		pConvertedData = (char*)_MsgDecodeBase64((UCHAR*)pData, (ULONG)nRead, (ULONG*)&nByte);
		MSG_DEBUG("MmsGetBinaryUTF8Data : MSG_ENCODING_BASE64     bodyLength = %d \n", nByte);

		pTemp = pConvertedData;
		nTemp = nByte;

		break;

	case MSG_ENCODING_QUOTE_PRINTABLE:

		pConvertedData = (char*)_MsgDecodeQuotePrintable((UCHAR*)pData, (ULONG)nRead, (ULONG*)&nByte);
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

	if (MsgIsText(msgTypeValue)) {
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

				nByte = MsgGetUnicode2UTFCodeSize(((unsigned short*)pTemp + 1), nChar);

				pConvertedStr = (char *)malloc(nByte + 1);
				if (pConvertedStr != NULL)
					MsgUnicode2UTF ((unsigned char*)pConvertedStr, nByte + 1, mszTempStr, nChar);
			} else {
				nChar = (nTemp / 2);

				mszTempStr = (unsigned short*) malloc(nChar * sizeof(unsigned short));
				if (mszTempStr == NULL) {
					MSG_DEBUG("MmsGetBinaryUTF8Data: 2. Memory Full !!! \n");
					goto __CATCH;
				}

				memcpy(mszTempStr, ((unsigned short*)pTemp), nChar * sizeof(unsigned short));

				nByte = MsgGetUnicode2UTFCodeSize(((unsigned short*)pTemp), nChar);

				pConvertedStr = (char *)malloc(nByte + 1);
				if (pConvertedStr)
					MsgUnicode2UTF ((unsigned char*)pConvertedStr, nByte + 1, mszTempStr, nChar);
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

			nByte = MsgGetLatin72UTFCodeSize((unsigned char*)pTemp, nTemp);
			pConvertedStr = (char *)malloc(nByte + 1);
			if (pConvertedStr)
				MsgLatin7code2UTF((unsigned char*)pConvertedStr, nByte + 1, (unsigned char*)pTemp, nTemp);

			pNewData = pConvertedStr;
			*npRead = nByte + 1;

			break;

		case MSG_CHARSET_ISO_8859_9:

			/* Turkish */
			MSG_DEBUG("MmsGetBinaryUTF8Data: MSG_CHARSET_ISO_8859_9 \n");

			nByte = MsgGetLatin52UTFCodeSize((unsigned char*)pTemp, nTemp);
			pConvertedStr = (char *)malloc(nByte + 1);
			if (pConvertedStr)
					MsgLatin5code2UTF((unsigned char*)pConvertedStr, nByte + 1, (unsigned char*)pTemp, nTemp);

			pNewData = pConvertedStr;
			*npRead = nByte + 1;

			break;

		default:

			MSG_DEBUG("MmsGetBinaryUTF8Data: Other charsets \n");

			nByte = MsgGetLatin2UTFCodeSize((unsigned char*)pTemp, nTemp);
			pConvertedStr = (char *)malloc(nByte + 1);
			if (pConvertedStr)
				MsgLatin2UTF((unsigned char*)pConvertedStr, nByte + 1, (unsigned char*)pTemp, nTemp);

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
bool MsgMakeFileName(int iMsgType, char *szFileName, int nUntitleIndex)
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
bool MsgMakeFileName(int iMsgType, char *szFileName, MsgDrmType drmType, int nUntitleIndex)
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
			if (strrchr(szTempFileName, '.'))
				return true;

			memset(szText, 0, MSG_FILENAME_LEN_MAX+1);
			strncpy(szText, szTempFileName, MSG_FILENAME_LEN_MAX - 1);
		//temporary commented to save file as original name.
			pExt = strrchr(szFileName, '.');
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

int MmsGetMediaPartCount(MSG_MESSAGE_ID_T msgId)
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

	_MsgInitMsgType(pHeader);


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

bool MmsDebugPrintMulitpartEntry(MsgMultipart *pMultipart, int index)
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

