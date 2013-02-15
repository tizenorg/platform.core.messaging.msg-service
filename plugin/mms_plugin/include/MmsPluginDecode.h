/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef MMS_PLUGIN_DECODE_H
#define MMS_PLUGIN_DECODE_H

#include "MmsPluginTypes.h"
#include "MmsPluginCodecTypes.h"

typedef struct _MsgHeaderAddress	MsgHeaderAddress;

typedef int MmsMsgID;

typedef enum {
	MSG_PRESENTATION_NONE = -1,
	MSG_PRESENTATION_FIRSTPART,		//Content-type == type parameter
	MSG_PRESENTATION_TYPE_BASE,		//Content-type == type parameter
	MSG_PRESENTATION_LOCATION,		//Content-Location == start parameter
	MSG_PRESENTATION_ID,			//Content-ID == start parameter
} MsgPresentationFactor;

typedef struct {
	char *pData;
	UINT32 length;
} EncodedPData;

typedef struct {
	MsgPresentationFactor factor;
	MsgMultipart *pPrevPart;
	MsgMultipart *pCurPresentation;
} MsgPresentaionInfo;

struct _MsgHeaderAddress {
	char *szAddr;
	MsgHeaderAddress *pNext;
};

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

extern MmsHeader mmsHeader;

/* Decoding */
void MmsInitHeader();
void MmsRegisterDecodeBuffer();
void MmsUnregisterDecodeBuffer(void);
bool MmsBinaryDecodeMsgHeader(FILE *pFile, int totalLength);
bool MmsBinaryDecodeMsgBody(FILE *pFile, char *szFilePath, int totalLength);
bool MmsReadMsgBody(msg_message_id_t msgID, bool bSavePartsAsTempFiles, bool bRetrieved, char *retrievedPath);


char *MsgChangeHexString(char *pOrg);
char *MsgResolveContentURI(char *szSrc);
char *MsgRemoveQuoteFromFilename(char *pSrc);
bool MsgIsMultipart(int type);
bool MmsAddrUtilCompareAddr(char *pszAddr1, char *pszAddr2);
bool MmsDataUpdateLastStatus(MmsMsg *pMsg);
bool MmsAddrUtilRemovePlmnString(char *pszAddr);
bool MsgGetTypeByFileName(int *type, char *szFileName);
bool MsgGetFileNameWithoutExtension(char *szOutputName, char *szName);
int MmsGetMediaPartCount(msg_message_id_t msgId);
bool MmsGetMediaPartHeader(int index, MsgType *pHeader);
bool MmsGetMsgAttrib(MmsMsgID msgID, MmsAttrib *pAttrib);

#ifdef __SUPPORT_DRM__
bool MsgCopyDrmInfo(MsgType *pPartType);
bool MmsDrm2ConvertMsgBody(char *szOriginFilePath);
bool MmsDrm2ReadMsgConvertedBody(MSG_MESSAGE_INFO_S *pMsg, bool bSavePartsAsTempFiles, bool bRetrieved, char *retrievedPath);
#endif

#endif //MMS_PLUGIN_DECODE_H
