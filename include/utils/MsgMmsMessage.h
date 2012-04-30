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

#ifndef MSG_MMS_MESSAGE_H
#define MSG_MMS_MESSAGE_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgTypes.h"
#include "MsgMmsTypes.h"

int msg_verify_number(const char *raw, char *trimmed);
int msg_verify_email(const char *raw);
MSG_ERROR_T _MsgMmsAddPage(MMS_MESSAGE_DATA_S *pMsgData, MMS_PAGE_S *pPage);
MSG_ERROR_T _MsgMmsAddMedia(MMS_PAGE_S *pPage, MMS_MEDIA_S *pMedia);
MSG_ERROR_T _MsgMmsAddSmilDoc(char *pSmil, MMS_MESSAGE_DATA_S *pMsgData);
MSG_ERROR_T _MsgMmsAddAttachment(MMS_MESSAGE_DATA_S *pMsgData, MMS_ATTACH_S *pMedia);
MSG_ERROR_T _MsgMmsAddRegion(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_REGION * pRegion);
MSG_ERROR_T _MsgMmsAddTransition(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_TRANSITION *pTransition);
MSG_ERROR_T _MsgMmsAddMeta(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_META *pMeta);

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

MSG_ERROR_T _MsgMmsReleasePageList(MMS_MESSAGE_DATA_S *pMsgData);
MSG_ERROR_T _MsgMmsReleaseRegionList(MMS_MESSAGE_DATA_S *pMsgData);
MSG_ERROR_T	_MsgMmsReleaseAttachList(MMS_MESSAGE_DATA_S *pMsgData);
MSG_ERROR_T _MsgMmsReleaseMetaList(MMS_MESSAGE_DATA_S *pMsgData);
MSG_ERROR_T _MsgMmsReleaseTransitionList(MMS_MESSAGE_DATA_S *pMsgData);

char *_MsgMmsSerializeMessageData(const MMS_MESSAGE_DATA_S *pMsgData, unsigned int *pSize);
bool _MsgMmsDeserializeMessageData(MMS_MESSAGE_DATA_S *pBody, char *pFileData);
bool _MsgMmsSetRootLayout(MMS_MESSAGE_DATA_S *pMmsMsg, MMS_SMIL_ROOTLAYOUT *pRootlayout);


#endif // MSG_MMS_MESSAGE_H
