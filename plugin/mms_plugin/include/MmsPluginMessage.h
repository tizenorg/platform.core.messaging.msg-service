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

#ifndef MMS_PLUGIN_MESSAGE_H
#define MMS_PLUGIN_MESSAGE_H

#include "MmsPluginTypes.h"
#include "MmsPluginCodec.h"

MsgMultipart *MmsAllocMultipart(void);
MsgMultipart *MmsMakeMultipart(MimeType mimeType, char *szTitleName, char *szOrgFilePath, char *szContentID, char *szContentLocation);

bool MmsIsMultipart(int type);
bool MmsIsText(int type);
bool MmsIsVitemContent(int type, char *pszName);

bool MmsComposeMessage(MmsMsg *pMmsMsg, MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMsgData, char *pFileData);
void MmsComposeNotiMessage(MmsMsg *pMmsMsg, msg_message_id_t msgID);
void MmsComposeReadReportMessage(MmsMsg *pMmsMsg, const MSG_MESSAGE_INFO_S *pMsgInfo, msg_message_id_t selectedMsgId);
bool MmsComposeSendReq(MmsMsg *pMmsMsg, MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMsgData);


bool MmsEncodeMmsMessage(MmsMsg *pMmsMsg, const char *raw_filepath);

bool MmsMakeMmsData(MmsMsg *pMsg, MMS_MESSAGE_DATA_S *pMmsMsg);

bool MmsCheckAdditionalMedia(MMS_MESSAGE_DATA_S *pMsgData, MsgType *partHeader);

bool MmsRemovePims(MMS_MESSAGE_DATA_S *pMsgData);

msg_error_t MmsMakePreviewInfo(int msgId, MMS_MESSAGE_DATA_S *pMmsMsg);

void MmsPrintFileInfoForVLD(MMS_MESSAGE_DATA_S *pMmsMsg);
int MmsUpdatePreviewData(MSG_MESSAGE_INFO_S *pMsgInfo);
#endif //MMS_PLUGIN_MESSAGE_H
