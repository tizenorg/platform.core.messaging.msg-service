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

#ifndef MMS_PLUGIN_DEBUG_H
#define	MMS_PLUGIN_DEBUG_H

const char *MmsDebugGetMimeType(MimeType mimeType);
const char *MmsDebugGetMmsReport(MmsReport report);
const char *MmsDebugGetMmsReportAllowed(MmsReportAllowed reportAllowed);
const char *MmsDebugGetMmsReadStatus(MSG_READ_REPORT_STATUS_T readStatus);
const char *MmsDebugGetMsgType(MmsMsgType msgType);
const char *MmsDebugGetResponseStatus(MmsResponseStatus responseStatus);
const char *MmsDebugGetRetrieveStatus(MmsRetrieveStatus retrieveStatus);
const char *MmsDebugGetMsgStatus(MSG_DELIVERY_REPORT_STATUS_T msgStatus);
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

