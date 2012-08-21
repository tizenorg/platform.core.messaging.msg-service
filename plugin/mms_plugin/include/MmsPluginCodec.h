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

#ifndef MMS_PLUGIN_CODEC_H
#define MMS_PLUGIN_CODEC_H


/*==================================================================================================
							HEADER
==================================================================================================*/

#include <glib.h>
#include <vconf.h>
#include "MsgMmsTypes.h"
#include "MmsPluginMessage.h"
#include "MsgTransportTypes.h"

/*==================================================================================================
							DEFINE
==================================================================================================*/

#define	MMS_MAJOR_VERSION			1

#if defined(MMS_V1_1)
#define	MMS_MINOR_VERSION			1
#define	MMS_VERSION					0x91

#elif defined(MMS_V1_2)
#define	MMS_MINOR_VERSION			2
#define	MMS_VERSION					0x92

#else
#define	MMS_MINOR_VERSION			0
#define	MMS_VERSION					0x90
#endif

#define	MMS_SIGN_TEXT_LEN			100
#define	MMS_MAX_FIELD_VALUE_COUNT	74
#define	MMS_MAX_FIELD_TYPE_COUNT	21
#define	MSG_MMS_ENCODE_BUFFER_MAX	(2 * 1024)
#define	MSG_MMS_DECODE_BUFFER_MAX	(2 * 1024)
#define	MSB								0x80
#define	QUOTE							0x7F
#define	MARK							0x22	// "
#define	NO_VALUE						0x00
#define	LENGTH_QUOTE					0x1F
#define	MSG_STDSTR_LONG				0xFF
#define	INVALID_VALUE					-1
#define	MSG_INVALID_VALUE				-1

#define	MMS_MAX_FIELD_VALUE_COUNT	74
#define	MMS_MAX_FIELD_TYPE_COUNT	21

#define MSG_LOCAL_TEMP_BUF_SIZE 1024
#define MSG_INBOX_DIRECTORY			"/User/Msg/Inbox"
/*==================================================================================================
							ENUMS
==================================================================================================*/

typedef enum {
	MMS_CODE_BCC,
	MMS_CODE_CC,
	MMS_CODE_CONTENTLOCATION,
	MMS_CODE_CONTENTTYPE,
	MMS_CODE_DATE,
	MMS_CODE_DELIVERYREPORT,
	MMS_CODE_DELIVERYTIME,
	MMS_CODE_EXPIRYTIME,
	MMS_CODE_FROM,
	MMS_CODE_MSGCLASS,
	MMS_CODE_MSGID,
	MMS_CODE_MSGTYPE,
	MMS_CODE_VERSION,
	MMS_CODE_MSGSIZE,
	MMS_CODE_PRIORITY,
	MMS_CODE_READREPLY,
	MMS_CODE_REPORTALLOWED,
	MMS_CODE_RESPONSESTATUS,
	MMS_CODE_RETRIEVESTATUS,		/* Add by MMSENC v1.1 */
	MMS_CODE_RESPONSETEXT,
	MMS_CODE_RETRIEVETEXT,			/* Add by MMSENC v1.1 */
	MMS_CODE_SENDERVISIBILLITY,
	MMS_CODE_MSGSTATUS,
	MMS_CODE_SUBJECT,
	MMS_CODE_TO,
	MMS_CODE_TRID,

	/* Add by MMSENC v1.1 */
	MMS_CODE_READSTATUS,
	MMS_CODE_REPLYCHARGING,
	MMS_CODE_REPLYCHARGINGDEADLINE,
	MMS_CODE_REPLYCHARGINGID,
	MMS_CODE_REPLYCHARGINGSIZE,
	MMS_CODE_PREVIOUSLYSENTBY,
	MMS_CODE_PREVIOUSLYSENTDATE,

	MMS_CODE_TRANSFERENCODING,
	MMS_CODE_DISPOSITION,
	MMS_CODE_CONTENT_ID
} MmsFieldCode;

enum {
	MMS_BODYHDR_TRANSFERENCODING,
	MMS_BODYHDR_DISPOSITION,
	MMS_BODYHDR_CONTENTID,
	MMS_BODYHDR_CONTENTLOCATION,
	MMS_BODYHDR_X_OMA_DRM_SEPARATE_DELIVERY,	// DRM RO WAITING
};

typedef enum {
	//code
	MmsCodeFieldCode,
	MmsCodeParameterCode,
	MmsCodeMsgBodyHeaderCode,

	//Data
	MmsCodeMsgType,
	MmsCodeDeliveryReport,
	MmsCodeTimeType,
	MmsCodeMsgClass,
	MmsCodePriority,
	MmsCodeResponseStatus,
	MmsCodeRetrieveStatus,
	MmsCodeReadReply,
	MmsCodeReportAllowed,
	MmsCodeSenderVisibility,
	MmsCodeMsgStatus,
	MmsCodeReadStatus,
	MmsCodeAddressType,
	MmsCodeCharSet,
	MmsCodeReplyCharging,

	MmsCodeContentType,

	MmsCodeMsgDisposition,
	MmsCodeContentTransferEncoding
} MmsCode;

typedef enum {
	MSG_PRESENTATION_NONE = -1,
	MSG_PRESENTATION_FIRSTPART,		//Content-type == type parameter
	MSG_PRESENTATION_TYPE_BASE,		//Content-type == type parameter
	MSG_PRESENTATION_LOCATION,		//Content-Location == start parameter
	MSG_PRESENTATION_ID,			//Content-ID == start parameter
} MsgPresentationFactor;

/*==================================================================================================
							Structures
==================================================================================================*/

typedef struct {
	const char *szText;
	UINT16 binary;
} MmsField;


typedef struct {
	char *pData;
	UINT32 length;
} EncodedPData;

typedef struct {
	MsgPresentationFactor factor;
	MsgMultipart *pPrevPart;
	MsgMultipart *pCurPresentation;
} MsgPresentaionInfo;

// for Decoding & Encoding
typedef struct {
	bool bActive;
	char *pszOwner;
	int msgID;		// if noti.ind, msgID is -1.

	MmsMsgType type;
	char szTrID[MMS_TR_ID_LEN+1];
	UINT8 version;
	UINT32 date;

	MsgHeaderAddress *pFrom;		//"/TYPE=PLMN", /"TYPE=IPv4", "/TYPE=IPv6"
	MsgHeaderAddress *pTo;
	MsgHeaderAddress *pCc;
	MsgHeaderAddress *pBcc;
	char szSubject[MSG_LOCALE_SUBJ_LEN + 1];
	MmsResponseStatus responseStatus;
	MmsRetrieveStatus retrieveStatus;
	char szResponseText[MMS_LOCALE_RESP_TEXT_LEN + 1];
	char szRetrieveText[MMS_LOCALE_RESP_TEXT_LEN + 1];

	MmsMsgClass msgClass;
	MmsTimeStruct expiryTime;
	MmsTimeStruct deliveryTime;
	MmsPriority priority;
	MmsSenderVisible hideAddress;
	MmsReport deliveryReport;
	MmsReport readReply;
	MmsReportAllowed reportAllowed;
	char szContentLocation[MMS_LOCATION_LEN + 1];

	msg_delivery_report_status_t msgStatus;
	msg_read_report_status_t readStatus;

	MmsReplyCharge replyCharge;

	// only used at Decoding module
	char szMsgID[MMS_MSG_ID_LEN + 1];
	UINT32 msgSize;

#ifdef __SUPPORT_DRM__
	MsgDrmType drmType;
#endif

	// dependent to Client implementation
	MsgType msgType;
	MsgBody msgBody;
} MmsHeader;

extern const MmsField gMmsField[MMS_MAX_FIELD_TYPE_COUNT][MMS_MAX_FIELD_VALUE_COUNT];
extern char gszMmsLoadBuf1[MSG_MMS_DECODE_BUFFER_MAX + 1];
extern char gszMmsLoadBuf2[MSG_MMS_DECODE_BUFFER_MAX + 1];

extern MmsHeader mmsHeader;

/* Common */
char *_MmsGetTextValue(MmsCode i, int j);
char *_MmsGetTextValuebyField(int field, int value);
int _MmsGetTextType(MmsCode i, char *pValue);
UINT16 _MmsGetBinaryValue(MmsCode i, int j);
int _MmsGetBinaryType(MmsCode i, UINT16 value);
UINT8 _MmsGetVersion(MmsMsg *pMsg);
bool _MmsSetVersion(int majorVer, int minorVer);


/* Decoding */
void _MmsInitHeader();
void _MmsCleanDecodeBuff(void);
bool _MmsDecodeInitialize(void);
void _MmsRegisterDecodeBuffer(char *pInBuff1, char *pInBuffer2, int maxLen);
void _MmsUnregisterDecodeBuffer(void);
int _MmsGetDecodeOffset(void);

bool MmsBinaryDecodeMsgBody(FILE *pFile, char *szFilePath, int totalLength);
bool MmsBinaryDecodeContentType(FILE *pFile, char *szFilePath, int totalLength); // for JAVA MMS AppId

bool MmsBinaryDecodeEachPart(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, int totalLength);
int MmsBinaryDecodeContentType(FILE *pFile, MsgType *pMsgType, int totalLength);
bool MmsBinaryDecodePartHeader(FILE *pFile, MsgType *pMsgType, int headerLen, int totalLength);
bool MmsBinaryDecodePartBody(FILE *pFile, UINT32 bodyLength, int totalLength);
bool MmsBinaryDecodeEntries(FILE *pFile, UINT32 *npEntries, int totalLength);
bool MmsBinaryDecodeMsgHeader(FILE *pFile, int totalLength);

bool MmsBinaryDecodeMultipart(FILE *pFile, char *szFilePath, MsgType *pMsgType, MsgBody *pMsgBody, int totalLength);


/* Encoding */
void _MmsRegisterEncodeBuffer(char *pInBuff, int maxLen);
void _MmsUnregisterEncodeBuffer(void);
int _MmsGetEncodeOffset(void);

bool _MmsEncodeMsg(void);
bool _MmsEncodeSendReq(FILE *pFile, MmsMsg *pMsg);
bool _MmsEncodeTemplate(FILE *pFile, MmsMsg *pMsg);	// just encode MMS Body without any header.
bool _MmsEncodeAckInd(FILE *pFile, char *szTrID, bool bReportAllowed);
bool _MmsEncodeNotiRespInd(FILE *pFile, char *szTrID, msg_delivery_report_status_t iStatus, bool bReportAllowed);
bool _MmsEncodeForwardReq(FILE *pFile, char *szContentLocation, char *szForwardTo, char *szForwardCc, char *szForwardBcc);
bool _MmsEncodeReadReport10(FILE *pFile, MmsMsg *pMsg, msg_read_report_status_t mmsReadStatus);
bool _MmsEncodeReadReport11(FILE *pFile, MmsMsg *pMsg, msg_read_report_status_t mmsReadStatus);


bool MmsBinaryEncodeMsgBody(FILE *pFile, MsgType *pType, MsgBody *pBody, int nPartCount, bool bTemplate);//NEW_TEMPLATE

int MmsBinaryEncodeContentTypeLen(MsgType *pType);
bool MmsBinaryEncodeContentType(FILE *pFile, MsgType *pType, int typeLength);
int MmsBinaryEncodeContentHeaderLen(MsgContentType contentType, MsgType *pType, bool bMultipart);
bool MmsBinaryEncodeContentHeader(FILE *pFile, MsgContentType contentType, MsgType *pType, bool bMultipart);
bool MmsBinaryEncodeContentBody(FILE *pFile, MsgBody *pBody);
bool MmsBinaryEncodeMsgPart(FILE *pFile, int contentType, MsgType *pType, MsgBody *pBody);
bool MmsBinaryEncodeSendReqHdrwithinBufRegi(FILE *pFile, MmsMsg *pMsg);
bool MsgWriteDataFromEncodeBuffer(FILE *pFile, char *pInBuffer, int *pPtr, int maxLen, int *pOffset);
bool MsgLoadDataToDecodeBuffer(FILE *pFile, char **ppBuf, int *pPtr, int *pOffset, char *pInBuf1, char *pInBuf2, int maxLen, int *npRead, int endOfFile);
bool MsgFreeHeaderAddress(MsgHeaderAddress *pAddr);
bool MsgCheckFileNameHasInvalidChar(char *szName);
bool _MsgReplaceInvalidFileNameChar(char *szInText, char replaceChar);
char *_MsgGetStringUntilDelimiter(char *pszString, char delimiter);
char *MsgChangeHexString(char *pOrg);
bool _MsgParseParameter(MsgType *pType, char *pSrc);
char *_MsgSkipWS(char *s);
char *__MsgSkipComment(char *s, long trim);
int _MsgGetCode(MsgHeaderField tableId, char *pStr);
char *MsgConvertLatin2UTF8FileName(char *pSrc);
bool _MsgChangeSpace(char *pOrg, char **ppNew);
void _MsgRemoveFilePath(char *pSrc);
bool MsgIsUTF8String(unsigned char *szSrc, int nChar);
bool MsgIsPercentSign(char *pSrc);
MsgMultipart *MsgAllocMultipart(void);
bool _MsgInitMsgType(MsgType *pMsgType);
bool _MsgInitMsgBody(MsgBody *pMsgBody);
MsgPresentationFactor MsgIsPresentationEx(MsgType *multipartType, char *szStart, MsgContentType typeParam);
void MsgConfirmPresentationPart(MsgType *pMsgType, MsgBody *pMsgBody, MsgPresentaionInfo *pPresentationInfo);
bool MsgIsMultipartRelated(int type);
bool MsgIsPresentablePart(int type);
bool MsgIsText(int type);
bool _MsgFreeBody(MsgBody *pBody, int type);
bool MsgFreeAttrib(MmsAttrib *pAttrib);
bool MsgResolveNestedMultipart(MsgType *pPartType, MsgBody *pPartBody);
char *MsgResolveContentURI(char *szSrc);
bool _MsgParsePartHeader(MsgType *pType, const char *pRawData, int nRawData);
char *MsgRemoveQuoteFromFilename(char *pSrc);
MmsMsg *MmsGetMsgById(MmsMsgID msgID, bool msgLock);
bool MsgIsMultipart(int type);
bool MsgIsHexChar(char *pSrc);
char _MsgConvertHexValue(char *pSrc);
int __MsgConvertCharToInt(char ch);
bool __MsgInitMsgContentParam(MsgContentParam *pMsgContentParam);
bool MsgCopyNestedMsgType(MsgType *pMsgType1, MsgType *pMsgType2);
bool MsgCopyNestedMsgParam(MsgContentParam *pParam1, MsgContentParam *pParam2);
bool MsgIsMultipartMixed(int type);
bool MsgStrcpyWithoutCRLF(char *pOrg, char **ppNew);
bool __MsgIsInvalidFileNameChar(char ch);
bool MmsAddrUtilCompareAddr(char *pszAddr1, char *pszAddr2);
bool _MmsDataUpdateLastStatus(MmsMsg *pMsg);

bool _MsgEncodeBase64(void *pSrc, unsigned long srcLen, unsigned long *len, unsigned char *ret);
void *_MsgDecodeBase64(unsigned char *pSrc, unsigned long srcLen, unsigned long *len);
bool _MsgEncodeQuotePrintable(unsigned char *pSrc, unsigned long srcLen, unsigned long *len, unsigned char *ret);
unsigned char *_MsgDecodeQuotePrintable(unsigned char *pSrc, unsigned long srcLen, unsigned long *len);
bool _MsgEncode2Base64(void *pSrc, unsigned long srcLen, unsigned long *len, unsigned char *ret);
char *_MsgDecodeText(char *pOri);
char *MsgEncodeText(char *pOri);
int MsgGetLatin2UTFCodeSize(unsigned char *szSrc, int nChar);
int MsgLatin5code2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar);
int MsgGetLatin52UTFCodeSize(unsigned char *szSrc, int nChar);
int MsgLatin2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar);
int MsgLatin7code2UTF(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar);
int MsgGetLatin72UTFCodeSize(unsigned char *szSrc, int nChar);
int MsgUnicode2UTF(unsigned char *des, int outBufSize, unsigned short *szSrc, int nChar);
int MsgGetUnicode2UTFCodeSize(unsigned short *szSrc, int nChar);
bool MmsAddrUtilCheckEmailAddress(char *pszAddr);
bool MmsAddrUtilRemovePlmnString(char *pszAddr);
int MsgCutUTFString(unsigned char *des, int outBufSize, unsigned char *szSrc, int nChar);
void MsgMIMERemoveQuote(char *szSrc);
bool _MmsMultipartSaveAsTempFile(MsgType *pPartType, MsgBody *pPartBody, char *pszMailboxPath, char *pszMsgFilename, int index, bool bSave);
bool MsgGetTypeByFileName(int *type, char *szFileName);
bool MmsGetMediaPartData(MsgType *pPartType, MsgBody *pPartBody, FILE *pFile);
char *MmsGetBinaryUTF8Data(char *pData, int nRead, int msgEncodingValue, int msgTypeValue, int msgCharsetValue, int *npRead);
#ifndef	__SUPPORT_DRM__
bool MsgMakeFileName(int iMsgType, char *szFileName, int nUntitleIndex);
#else
bool MsgMakeFileName(int iMsgType, char *szFileName, MsgDrmType drmType, int nUntitleIndex);
#endif
bool MsgGetFileNameWithoutExtension(char *szOutputName, char *szName);
int MmsGetMediaPartCount(msg_message_id_t msgId);
bool MmsGetMediaPartHeader(int index, MsgType *pHeader);
bool MmsGetMsgAttrib(MmsMsgID msgID, MmsAttrib *pAttrib);
bool _MmsReadMsgBody(msg_message_id_t msgID, bool bSavePartsAsTempFiles, bool bRetrieved, char *retrievedPath);

#ifdef __SUPPORT_DRM__
void _MsgFreeDRMInfo(MsgDRMInfo *pDrmInfo);
bool MsgCopyDrmInfo(MsgType *pPartType);
bool MmsDrm2ConvertMsgBody(char *szOriginFilePath);
bool MmsDrm2ReadMsgConvertedBody(MSG_MESSAGE_INFO_S *pMsg, bool bSavePartsAsTempFiles, bool bRetrieved, char *retrievedPath);
bool MmsDrm2DecodeConvertedMsg(int msgID, char *pszFullPath);
#endif

bool MmsDebugPrintMulitpartEntry(MsgMultipart *pMultipart, int index);

#endif
