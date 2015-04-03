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

	MsgDrmType drmType;

	// dependent to Client implementation
	MsgType msgType;
	MsgBody msgBody;
} MmsHeader;

extern __thread MmsHeader mmsHeader;

/* Decoding */
void MmsInitHeader();
void MmsReleaseHeader(MmsHeader *mms);
void MmsRegisterDecodeBuffer();
void MmsUnregisterDecodeBuffer(void);
bool MmsBinaryDecodeMsgHeader(FILE *pFile, int totalLength);
bool MmsBinaryDecodeMsgBody(FILE *pFile, char *szFilePath, int totalLength);
bool MmsReadMsgBody(msg_message_id_t msgID, bool bSavePartsAsTempFiles, bool bRetrieved, char *retrievedPath);


char *MsgChangeHexString(char *pOrg);
char *MsgResolveContentURI(char *szSrc);
char *MsgRemoveQuoteFromFilename(char *pSrc);
bool MsgIsMultipart(int type);
bool MmsAddrUtilRemovePlmnString(char *pszAddr);
bool MmsAddrUtilCheckEmailAddress(char *pszAddr);
bool MsgGetFileNameWithoutExtension(char *szOutputName, char *szName);
bool MsgGetFileName(char *szFilePath, char *szFileName, int size);
bool MmsGetMediaPartHeader(int index, MsgType *pHeader);

class MmsPluginDecoder {
public:
	static MmsPluginDecoder *instance();

	void decodeMmsPdu(MmsMsg *pMmsMsg, msg_message_id_t msgID, const char *pduFilePath);
private:
	static MmsPluginDecoder *pInstance;

	MmsPluginDecoder();
	~MmsPluginDecoder();

};
#endif //MMS_PLUGIN_DECODE_H
