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

#ifndef MSG_MMS_MESSAGE_H
#define MSG_MMS_MESSAGE_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgTypes.h"
#include "MsgMmsTypes.h"

msg_error_t _MsgMmsAddPage(MMS_MESSAGE_DATA_S *pMsgData, MMS_PAGE_S *pPage);
msg_error_t _MsgMmsAddMedia(MMS_PAGE_S *pPage, MMS_MEDIA_S *pMedia);
msg_error_t _MsgMmsAddAttachment(MMS_MESSAGE_DATA_S *pMsgData, MMS_ATTACH_S *pMedia);
msg_error_t _MsgMmsAddRegion(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_REGION * pRegion);
msg_error_t _MsgMmsAddTransition(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_TRANSITION *pTransition);
msg_error_t _MsgMmsAddMeta(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_META *pMeta);

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

void MsgMmsReleaseMmsLists(MMS_MESSAGE_DATA_S *pMsgData);

char *_MsgMmsSerializeMessageData(const MMS_MESSAGE_DATA_S *pMsgData, unsigned int *pSize);
bool _MsgMmsDeserializeMessageData(MMS_MESSAGE_DATA_S *pMsgData, const char *pData);

bool _MsgMmsSetRootLayout(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_ROOTLAYOUT *pRootlayout);

void _MsgMmsPrint(MMS_MESSAGE_DATA_S *pMsgData);
void _MsgMmsAttachPrint(MMS_ATTACH_S *attach);
void _MsgMmsMediaPrint(MMS_MEDIA_S *media);
void _MsgMmsPagePrint(MMS_PAGE_S *page);

MMS_HEADER_DATA_S *MsgMmsCreateHeader();
void MsgMmsInitHeader(MMS_HEADER_DATA_S *pMmsHeaderData);
void MsgMmsReleaseHeader(MMS_HEADER_DATA_S **ppMmHeadersData);

MMS_MULTIPART_DATA_S *MsgMmsCreateMultipart();
void MsgMmsInitMultipart(MMS_MULTIPART_DATA_S *pMmsMultipart);
void MsgMmsReleaseMultipart(MMS_MULTIPART_DATA_S **ppMmsMultipart);

MMS_DATA_S *MsgMmsCreate();

void MsgMmsRelease(MMS_DATA_S **ppMmsData);

int MsgMmsReleaseMultipartList(MMSList **ppMultipartList);

int MsgMmsGetSmilMultipart(MMSList *pMultipartList, MMS_MULTIPART_DATA_S **smil_multipart);//TODO : delete

int MsgMmsConvertMmsDataToMmsMessageData(MMS_DATA_S *pSrc, MMS_MESSAGE_DATA_S *pDst);

int MsgMmsConvertMmsMessageDataToMmsData(MMS_MESSAGE_DATA_S *pSrc, MMS_DATA_S *pDst);

MMS_ADDRESS_DATA_S *MsgMmsCreateAddress(int addressType, const char *addressVal);
void MsgMmsReleaseAddress(MMS_ADDRESS_DATA_S **ppMmsAddressData);
int MsgMmsReleaseAddressList(MMSList **ppAddressList);

int MsgMmsSetMultipartData(MMS_MULTIPART_DATA_S *pMultipart);
int MsgMmsSetMultipartFilePath(const char *dirPath, MMS_MULTIPART_DATA_S *pMultipart);

int MsgMmsSetMultipartListData(MMS_DATA_S *pMmsData);
int MsgMmsSetMultipartListFilePath(const char *dirPath, MMS_DATA_S *pMmsData);

int printMultipart(MMS_MULTIPART_DATA_S *multipart);
int printMultipartList(MMSList *pMultipartList);//for debug

void _MsgMmsMultipartPrint(MMS_MULTIPART_DATA_S *multipart);

bool  _MsgMmsRemoveEmptyObject(MMS_MESSAGE_DATA_S *pMmsMsg);

#endif // MSG_MMS_MESSAGE_H
