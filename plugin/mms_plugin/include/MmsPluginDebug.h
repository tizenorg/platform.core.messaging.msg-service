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

#ifndef MMS_PLUGIN_DEBUG_H
#define	MMS_PLUGIN_DEBUG_H

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
#ifdef MMS_PLUGIN_DEBUG_ENABLE
bool MmsDebugPrintMsgAttributes(char *pszFunc, MmsAttrib *pAttrib, bool bAll);
char *MmsDebugGetMsgDrmType(MsgDrmType drmType);
char *MmsDebugGetDrmDeliveryMode(DrmDeliveryMode deliveryMode);
char *MmsDebugGetDrmRightState(DrmRightState rightState);
bool MmsDebugPrintDrmRight(DrmRight *pDrmRight);
char *MmsDebugPrintMsgDRMStatus(MsgDRMStatus status);
bool MmsDebugPrintMulitpartEntry(MsgMultipart *pMultipart, int index);
char *DebugPrintGetRmResultInd(MmsRmResultInd indType);
char *DebugPrintHttpStatusCode(int status);
char *DebugPrintRmMethodType(MmsRmMethodType method);
char *DebugPrintGetMmsRmNetState(MmsRmNetState state);
char *DebugPrintGetMmsRmEntityState(MmsRmExEntityState stateEx);
char *MmsDebugPrintMmsRmResult(MmsRmResult result);
void MmsDebugPrintReqEntityInfo(MmsRmRequest *pEntity);
char *MmsDebugPrintHttpErrorCode(int errCode);
char *MmsDebugPrintProtoErrorCode(int errCode);
char *DebugPrintWspResult(WspResult wspResult);
char *DebugPrintWspState(MmsRmWapState wspState);
char *MmsDebugPrintRmPduType(MmsRmPduType pduType);
char *MmsDebugPrintMailboxType(MsgMailboxType mailboxType);
#endif

typedef enum {
	MMS_DEBUG_EV_NONE,
	MMS_DEBUG_EV_MMS,
	MMS_DEBUG_EV_SMS,
	MMS_DEBUG_EV_COMMON,
	MMS_DEBUG_EV_EMAIL

} MmsDebugEvType;

bool MmsDebugPrintCurrentEventHandler(char *pszFunc, MmsDebugEvType evType);

#endif //MMS_PLUGIN_DEBUG_H

