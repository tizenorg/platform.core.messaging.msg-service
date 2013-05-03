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

#ifndef MMS_PLUGIN_CODEC_COMMON_H
#define MMS_PLUGIN_CODEC_COMMON_H

#include "MmsPluginCodecTypes.h"

const char *MmsGetTextByCode(MmsCode i, UINT16 code);
const char *MmsGetTextValue(MmsCode i, int j);
const char *MmsGetTextValuebyField(int field, int value);
int MmsGetBinaryType(MmsCode i, UINT16 value);
int MmsGetTextType(MmsCode i, char *pValue);
UINT16 MmsGetBinaryValue(MmsCode i, int j);

void *MsgDecodeBase64(unsigned char *pSrc, unsigned long srcLen, unsigned long *len);
bool MsgEncode2Base64(void *pSrc, unsigned long srcLen, unsigned long *len, unsigned char *ret);
unsigned char *MsgDecodeQuotePrintable(unsigned char *pSrc, unsigned long srcLen, unsigned long *len);

char *MsgDecodeText(char *pOri);

const char *MmsDebugGetMimeType(MimeType mimeType);
const char *MmsDebugGetMmsReport(MmsReport report);
const char *MmsDebugGetMmsReportAllowed(MmsReportAllowed reportAllowed);
const char *MmsDebugGetMmsReadStatus(msg_read_report_status_t readStatus);
const char *MmsDebugGetMsgType(MmsMsgType msgType);
const char *MmsDebugGetResponseStatus(MmsResponseStatus responseStatus);
const char *MmsDebugGetRetrieveStatus(MmsRetrieveStatus retrieveStatus);
const char *MmsDebugGetMsgStatus(msg_delivery_report_status_t msgStatus);
const char *MmsDebugGetMsgClass(MmsMsgClass msgClass);
const char *MmsDebugGetDataType(MmsDataType dataType);

bool MmsInitMsgType(MsgType *pMsgType);
bool MmsInitMsgBody(MsgBody *pMsgBody);
bool MmsInitMsgContentParam(MsgContentParam *pMsgContentParam);
bool MmsInitMsgAttrib(MmsAttrib *pAttrib);

#ifdef __SUPPORT_DRM__
bool MmsInitMsgDRMInfo(MsgDRMInfo *pMsgDrmInfo);
void MmsReleaseMsgDRMInfo(MsgDRMInfo *pDrmInfo);
#endif//__SUPPORT_DRM__

bool MmsReleaseMsgBody(MsgBody *pBody, int type);
bool MmsReleaseMmsAttrib(MmsAttrib *pAttrib);
void MmsReleaseMmsMsg(MmsMsg *pMmsMsg);

#endif //MMS_PLUGIN_CODEC_COMMON_H
