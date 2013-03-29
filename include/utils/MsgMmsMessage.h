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

#ifndef MSG_MMS_MESSAGE_H
#define MSG_MMS_MESSAGE_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgTypes.h"
#include "MsgMmsTypes.h"

int msg_verify_number(const char *raw, char *trimmed);
int msg_verify_email(const char *raw);
msg_error_t _MsgMmsAddPage(MMS_MESSAGE_DATA_S *pMsgData, MMS_PAGE_S *pPage);
msg_error_t _MsgMmsAddMedia(MMS_PAGE_S *pPage, MMS_MEDIA_S *pMedia);
msg_error_t _MsgMmsAddSmilDoc(char *pSmil, MMS_MESSAGE_DATA_S *pMsgData);
msg_error_t _MsgMmsAddAttachment(MMS_MESSAGE_DATA_S *pMsgData, MMS_ATTACH_S *pMedia);
msg_error_t _MsgMmsAddRegion(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_REGION * pRegion);
msg_error_t _MsgMmsAddTransition(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_TRANSITION *pTransition);
msg_error_t _MsgMmsAddMeta(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_META *pMeta);

bool _MsgMmsFindMatchedMedia(MMS_MESSAGE_DATA_S *pMsgData, char *pszFilePath);

MMS_PAGE_S *_MsgMmsGetPage(MMS_MESSAGE_DATA_S *pMsgData, int pageIdx);
MMS_SMIL_REGION *_MsgMmsGetSmilRegion(MMS_MESSAGE_DATA_S *pMsgData, int regionIdx);
MMS_MEDIA_S *_MsgMmsGetMedia(MMS_PAGE_S *pPage, int mediaIdx);
MMS_ATTACH_S *_MsgMmsGetAttachment(MMS_MESSAGE_DATA_S *pMsgData, int attachIdx);
MMS_SMIL_TRANSITION *_MsgMmsGetTransition(MMS_MESSAGE_DATA_S *pMsgData, int transitionIdx);
MMS_SMIL_META *_MsgMmsGetMeta(MMS_MESSAGE_DATA_S *pMsgData, int metaIdx);
int	_MsgMmsGetPageCount(MMS_MESSAGE_DATA_S *pMsgData);
int _MsgMmsGetAttachCount(MMS_MESSAGE_DATA_S *pMsgData);
int _MsgMmsGetTransitionCount(MMS_MESSAGE_DATA_S *pMsgData);
int _MsgMmsGetMetaCount(MMS_MESSAGE_DATA_S *pMsgData);

msg_error_t _MsgMmsReleasePageList(MMS_MESSAGE_DATA_S *pMsgData);
msg_error_t _MsgMmsReleaseRegionList(MMS_MESSAGE_DATA_S *pMsgData);
msg_error_t	_MsgMmsReleaseAttachList(MMS_MESSAGE_DATA_S *pMsgData);
msg_error_t _MsgMmsReleaseMetaList(MMS_MESSAGE_DATA_S *pMsgData);
msg_error_t _MsgMmsReleaseTransitionList(MMS_MESSAGE_DATA_S *pMsgData);

char *_MsgMmsSerializeMessageData(const MMS_MESSAGE_DATA_S *pMsgData, size_t *pSize);
bool _MsgMmsDeserializeMessageData(MMS_MESSAGE_DATA_S *pBody, char *pFileData);
bool _MsgMmsSetRootLayout(MMS_MESSAGE_DATA_S *pMmsMsg, MMS_SMIL_ROOTLAYOUT *pRootlayout);


#endif // MSG_MMS_MESSAGE_H
