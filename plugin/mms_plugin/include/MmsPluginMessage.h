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

#ifndef MMS_PLUGIN_MESSAGE_H
#define MMS_PLUGIN_MESSAGE_H


/*==================================================================================================
							INCLUDE FILES
==================================================================================================*/
#include "MsgTypes.h"
#include "MsgInternalTypes.h"
#include "MsgMmsTypes.h"
#include "MsgSettingTypes.h"
#include "MmsPluginMIME.h"
#include "MmsPluginCodec.h"
/*==================================================================================================
							DEFINES
==================================================================================================*/
#define		MMS_TEXT_LEN						2000
#define		MMS_READ_REPORT_STRING_READ		"Read:"
#define		MMS_READ_REPORT_STRING_DELETED	"Deleted:"

#define		MSG_ATTACH_MAX		20
#define		MSG_STR_ADDR_DELIMETER			";"
#define		MSG_CH_ADDR_DELIMETER			';'

bool MmsInitMsgType(MsgType *pMsgType);
bool MmsInitMsgBody(MsgBody *pMsgBody);
bool MmsInitMsgContentParam(MsgContentParam *pMsgContentParam);
bool MmsInitMsgAttrib(MmsAttrib *pAttrib);
bool MmsSetMsgAddressList(MmsAttrib *pAttrib, const MSG_MESSAGE_INFO_S *pMsgInfo);
char *MmsComposeAddress(const MSG_MESSAGE_INFO_S *pMsgInfo, int recipientType);
bool MmsGetMsgBodyfromMsgInfo(const MSG_MESSAGE_INFO_S *pMsgInfo, MMS_MESSAGE_DATA_S *pMsgBody, char *pFileData);
bool MmsGetSmilRawData(MMS_MESSAGE_DATA_S *pMsgBody, char *pRawData, int *nSize);
bool MmsInsertPresentation(MmsMsg *pMsg, MimeType mimeType, char *pData, int size);
bool MmsInsertPartFromFile(MmsMsg *pMsg, char *szTitleName, char *szOrgFilePath, char *szContentID);
bool MmsIsMultipart(int type);
bool MmsGetTypeByFileName(int *type, char *szFileName);
MsgMultipart *MmsMakeMultipart(MimeType mimeType, char *szTitleName, char *szOrgFilePath, void *pData, int offset, int size, char *szContentID);
bool MmsIsText(int type);
bool MmsIsVitemContent(int type, char *pszName);
bool MmsComposeMessage(MmsMsg *pMmsMsg, MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMsgData, char *pFileData);
void MmsComposeNotiMessage(MmsMsg *pMmsMsg, msg_message_id_t msgID);
bool MmsGetMmsMessageBody(MmsMsg *pMmsMsg, char *retrievedFilePath);
bool MmsComposeForwardHeader(MmsMsg *pMmsMsg, const MSG_MESSAGE_INFO_S *pMsgInfo);
bool MmsComposeForwardMessage(MmsMsg *pMmsMsg, char *retrievedFilePath);
#ifdef MMS_DELIEVERY_IND_ENABLED
MmsMsgMultiStatus *MmsComposeDeliveryIndMessage(MmsMsg *pMmsMsg, msg_message_id_t msgId);
#endif
void MmsComposeReadReportMessage(MmsMsg *pMmsMsg, const MSG_MESSAGE_INFO_S *pMsgInfo, msg_message_id_t selectedMsgId);
MmsMsgMultiStatus *MmsGetMultiStatus(msg_message_id_t msgId);
int MmsSearchMsgId(char *toNumber, char *szMsgID);
MsgMultipart *MmsAllocMultipart(void);
msg_error_t MmsAddAttachment(MMS_MESSAGE_DATA_S *pMsgData, MMS_MEDIA_S *pMedia);
bool MmsCheckAdditionalMedia(MMS_MESSAGE_DATA_S *pMsgData, MsgType *partHeader);
bool MmsRemovePims(MMS_MESSAGE_DATA_S *pMsgData);
#ifdef __SUPPORT_DRM__
bool __MsgInitMsgDRMInfo(MsgDRMInfo *pMsgDrmInfo);
#endif

msg_error_t MmsMakePreviewInfo(int msgId, MMS_MESSAGE_DATA_S *pMmsMsg);
#endif
