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

#ifndef MMS_PLUGIN_ENCODE_H
#define MMS_PLUGIN_ENCODE_H

#include <stdio.h>
#include "MmsPluginCodecTypes.h"
#include "MmsPluginCodecCommon.h"
#include "MmsPluginMessage.h"

/* Encoding */
void MmsRegisterEncodeBuffer(char *pInBuff, int maxLen);
void MmsUnregisterEncodeBuffer(void);

bool MmsEncodeSendReq(FILE *pFile, MmsMsg *pMsg);
bool MmsEncodeTemplate(FILE *pFile, MmsMsg *pMsg);
bool MmsEncodeAckInd(FILE *pFile, char *szTrID, bool bReportAllowed);
bool MmsEncodeNotiRespInd(FILE *pFile, char *szTrID, msg_delivery_report_status_t iStatus, bool bReportAllowed);

bool MmsEncodeReadReport10(FILE *pFile, MmsMsg *pMsg, msg_read_report_status_t mmsReadStatus);
bool MmsEncodeReadReport11(FILE *pFile, MmsMsg *pMsg, msg_read_report_status_t mmsReadStatus);

#endif //MMS_PLUGIN_ENCODE_H
