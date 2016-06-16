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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "MsgCppTypes.h"
#include "MsgStorageTypes.h"
#include "MsgSettingTypes.h"
#include "MsgUtilFile.h"
#include "MsgGconfWrapper.h"
#include "MsgMmsMessage.h"
#include "MmsPluginTypes.h"
#include "MmsPluginDebug.h"
#include "MmsPluginMessage.h"
#include "MsgUtilMime.h"
#include "MmsPluginStorage.h"
#include "MmsPluginUtil.h"
#include "MsgSmil.h"
#include "MmsPluginAppBase.h"
#include "MsgUtilFunction.h"

#define PRINT_KEY_VAL_STR(key, val)\
if (val) {\
MSG_DEBUG("%-20s: %s", key, val);\
}\

#define PRINT_KEY_VAL_INT(key, val)\
if (val) {\
MSG_DEBUG("%-20s: %d", key, val);\
}\

static MsgMultipart *MmsGetNthMultipart(MmsMsg *pMsg, int index);
static MsgMultipart *MmsMakeMultipart(MimeType mimeType, char *szTitleName, char *szOrgFilePath, char *szContentID, char *szContentLocation);
static bool MmsSetMsgAddressList(MmsAttrib *pAttrib, const MSG_MESSAGE_INFO_S *pMsgInfo);
static char *MmsComposeAddress(const MSG_MESSAGE_INFO_S *pMsgInfo, int recipientType);
static bool MmsInsertPresentation(MmsMsg *pMsg, MimeType mimeType, const char *content_id, char *pData, int size);
static bool MmsInsertPartFromMultipart(MmsMsg *pMsg, MMS_MULTIPART_DATA_S *pNewMultipart);
static bool MmsInsertPartToMmsData(MMS_MESSAGE_DATA_S *pMsgData, MMS_MULTIPART_DATA_S *pMultipart);
static bool MmsInsertMixedPartToMmsData(MMS_MESSAGE_DATA_S *pMsgData, MMS_MULTIPART_DATA_S *pMultipart);
static void printMmsAttribute(MmsAttrib *pAttrib);

bool convertMediaToMultipart(MMS_MEDIA_S *pSrcMedia, MMS_MULTIPART_DATA_S *pDestMultipart);
bool convertAttachToMultipart(MMS_ATTACH_S *pSrcMedia, MMS_MULTIPART_DATA_S *pDestMultipart);
MMSList * MmsConvertAddressToNewStyle(const char *szAddr);
char *MmsConvertAddressToOldStyle(MMSList *pAddressList);

void printMmsAttribute(MmsAttrib *pAttrib)
{
	MSG_ERR_RET_M(pAttrib == NULL, "attribute is NULL");

	MSG_DEBUG("# LeaveCopy [%d]", pAttrib->bLeaveCopy);
	MSG_DEBUG("# DeliveryTime type [%d], time [%d]", pAttrib->deliveryTime.type, pAttrib->deliveryTime.time);
	MSG_DEBUG("# ExpiryTime type [%d], time [%d]", pAttrib->expiryTime.type, pAttrib->expiryTime.time);
	MSG_DEBUG("# Priority [%d]", pAttrib->priority);
	MSG_DEBUG("# AskDeliveryReport [%d]", pAttrib->bAskDeliveryReport);
	MSG_DEBUG("# AskReadReply [%d]", pAttrib->bAskReadReply);
}

bool MmsSetMsgAddressList(MmsAttrib *pAttrib, const MSG_MESSAGE_INFO_S * pMsgInfo)
{
	MSG_BEGIN();

	pAttrib->szTo = MmsComposeAddress(pMsgInfo, MSG_RECIPIENTS_TYPE_TO);
	if (pAttrib->szTo) {
		MSG_SEC_DEBUG("To address: %s", pAttrib->szTo);
	}

	pAttrib->szCc = MmsComposeAddress(pMsgInfo, MSG_RECIPIENTS_TYPE_CC);
	if (pAttrib->szCc) {
		MSG_SEC_DEBUG("Cc address: %s", pAttrib->szCc);
	}

	pAttrib->szBcc = MmsComposeAddress(pMsgInfo, MSG_RECIPIENTS_TYPE_BCC);
	if (pAttrib->szBcc) {
		MSG_SEC_DEBUG("Bcc address: %s", pAttrib->szBcc);
	}

	MSG_END();
	return true;
}

char *MmsComposeAddress(const MSG_MESSAGE_INFO_S *pMsgInfo, int recipientType)
{
	MSG_BEGIN();
	int	addrLen = 0;
	int	nAddressCnt = 0;
	int nRecpCnt = 0;
	char pString[MSG_LOCALE_NAME_LEN + MSG_ADDR_LEN + 3] = {0, };
	char *szCompose;
	const char *typePlmn = "/TYPE=PLMN";

	nAddressCnt = pMsgInfo->nAddressCnt;

	/* Calculate allocated buffer size */
	for (int i = 0; i < nAddressCnt; ++i) {
		MSG_SEC_DEBUG("recipientType: %d, address value: %s", pMsgInfo->addressList[i].recipientType, pMsgInfo->addressList[i].addressVal);

		if (pMsgInfo->addressList[i].recipientType == MSG_RECIPIENTS_TYPE_UNKNOWN)
			pMsgInfo->addressList[i].recipientType = MSG_RECIPIENTS_TYPE_TO;

		if (pMsgInfo->addressList[i].recipientType == recipientType) {
			if (pMsgInfo->addressList[i].addressType == MSG_ADDRESS_TYPE_PLMN) {
				addrLen += strlen(typePlmn);
				addrLen += strlen(pMsgInfo->addressList[i].addressVal);
			} else if (pMsgInfo->addressList[i].addressType == MSG_ADDRESS_TYPE_EMAIL) {
				addrLen += strlen(pMsgInfo->addressList[i].addressVal);
			}
			/*  Need to consider IPV4, IPV6, and Alias formatted address */

			nRecpCnt++;
		}
	}

	if (nRecpCnt > 1)
		addrLen = addrLen + nRecpCnt - 1;
	szCompose = (char *)calloc(addrLen + 1, 1);

	/* Address String copy */
	for (int i = 0; i < nAddressCnt; ++i) {
		if (pMsgInfo->addressList[i].recipientType == recipientType) {
			if (szCompose && strlen(szCompose) > 0)
				g_strlcat(szCompose, MSG_STR_ADDR_DELIMETER, addrLen + 1);

			memset(pString, 0x00, (MSG_LOCALE_NAME_LEN + MSG_ADDR_LEN + 3) * sizeof(char));
			if (pMsgInfo->addressList[i].addressType == MSG_ADDRESS_TYPE_PLMN) {
				snprintf(pString, MSG_LOCALE_NAME_LEN + MSG_ADDR_LEN + 3, "%s%s", pMsgInfo->addressList[i].addressVal, typePlmn);
				MSG_DEBUG("%s", pString);
			} else if (pMsgInfo->addressList[i].addressType == MSG_ADDRESS_TYPE_EMAIL) {
				snprintf(pString, MSG_LOCALE_NAME_LEN + MSG_ADDR_LEN + 3, "%s", pMsgInfo->addressList[i].addressVal);
			}
			/* Need to consider IPV4, IPV6, and Alias formatted address */

			g_strlcat(szCompose, pString, addrLen + 1);
		}
	}

	MSG_END();
	return szCompose;
}

bool MmsInsertPresentation(MmsMsg *pMsg, MimeType mimeType, const char *content_id, char *pData, int size)
{
	MSG_BEGIN();

	if (pMsg == NULL) {
		MSG_DEBUG("pMsg is NULL");
		return false;
	}

	if (pMsg->msgBody.pPresentationBody != NULL)
		goto __CATCH;

	memset(&pMsg->msgBody.presentationType, 0, sizeof(MsgType));
	pMsg->msgBody.pPresentationBody = (MsgBody *)malloc(sizeof(MsgBody));
	if (pMsg->msgBody.pPresentationBody == NULL)
		goto __CATCH;

	MmsInitMsgBody(pMsg->msgBody.pPresentationBody);

	pMsg->msgBody.pPresentationBody->body.pText = (char *)malloc(size + 1);
	if (pMsg->msgBody.pPresentationBody->body.pText == NULL)
		goto __CATCH;

	pMsg->msgBody.pPresentationBody->size = size;
	pMsg->msgBody.presentationType.type = mimeType;
	pMsg->msgBody.presentationType.param.charset = MSG_CHARSET_UTF8;

	if (content_id && strlen(content_id) > 0) {
		snprintf(pMsg->msgBody.presentationType.szContentID, MSG_MSG_ID_LEN + 1, "%s", content_id);
	} else {
		snprintf(pMsg->msgBody.presentationType.szContentID, MSG_MSG_ID_LEN + 1, "<_S_>"); /* default */
	}

	snprintf(pMsg->msgType.param.szStart, MSG_MSG_ID_LEN + 1, "%s", pMsg->msgBody.presentationType.szContentID);

	pMsg->msgType.param.type = mimeType;

	memset(pMsg->msgBody.pPresentationBody->body.pText, 0, size + 1);
	strncpy(pMsg->msgBody.pPresentationBody->body.pText, pData, size);

	MSG_END();
	return true;

__CATCH:

	if (pMsg->msgBody.pPresentationBody != NULL) {
		if (pMsg->msgBody.pPresentationBody->body.pText != NULL) {
			free(pMsg->msgBody.pPresentationBody->body.pText);
			pMsg->msgBody.pPresentationBody->body.pText = NULL;
		}

		free(pMsg->msgBody.pPresentationBody);
		pMsg->msgBody.pPresentationBody = NULL;
	}

	return false;
}

MsgMultipart *MmsMakeMultipart(MimeType mimeType, char *szTitleName, char *szOrgFilePath, char *szContentID, char *szContentLocation)
{
	MsgMultipart *pMultipart = NULL;

	if ((pMultipart = MmsAllocMultipart()) == NULL)
		return NULL;

	pMultipart->type.type = mimeType;

	if (szTitleName && szTitleName[0]) {
		memset(pMultipart->type.param.szName, 0, MSG_LOCALE_FILENAME_LEN_MAX + 1);

		gchar *tmpTitleName = msg_replace_non_ascii_char(szTitleName, '_');
		if (tmpTitleName) {
			MSG_SEC_DEBUG("tmpTitleName = [%s]", tmpTitleName);
			snprintf(pMultipart->type.param.szName, MSG_LOCALE_FILENAME_LEN_MAX + 1, "%s", tmpTitleName);
			g_free(tmpTitleName);
			tmpTitleName = NULL;
		} else {
			MSG_WARN("tmpTitleName is NULL.");
			snprintf(pMultipart->type.param.szName, MSG_LOCALE_FILENAME_LEN_MAX + 1, "%s", szTitleName);
		}
	}

	if (szContentID && szContentID[0]) {
		memset(pMultipart->type.szContentID, 0, MSG_MSG_ID_LEN + 1);
		snprintf(pMultipart->type.szContentID, MSG_MSG_ID_LEN + 1, "<%s>", szContentID);
	}

	if (szContentLocation && szContentLocation[0]) {
		memset(pMultipart->type.szContentLocation, 0, MSG_MSG_ID_LEN + 1);

		gchar *tmpContentLocation = msg_replace_non_ascii_char(szContentLocation, '_');
		if (tmpContentLocation) {
			MSG_SEC_DEBUG("tmpContentLocation = [%s]", tmpContentLocation);
			snprintf(pMultipart->type.szContentLocation, MSG_MSG_ID_LEN + 1, "%s", tmpContentLocation);
			g_free(tmpContentLocation);
			tmpContentLocation = NULL;
		}
	}

	if (MmsIsTextType(mimeType) == true) {
		if (!MmsIsVitemContent(mimeType, pMultipart->type.param.szName)) {
			pMultipart->type.param.charset = MSG_CHARSET_UTF8;
		}
		pMultipart->type.encoding = MSG_ENCODING_8BIT;
	} else {
		pMultipart->type.encoding = MSG_ENCODING_BINARY;
	}

	if (szOrgFilePath) {
		strncpy(pMultipart->pBody->szOrgFilePath, szOrgFilePath, MSG_FILEPATH_LEN_MAX - 1);
		strncpy(pMultipart->type.szOrgFilePath, szOrgFilePath, MSG_FILEPATH_LEN_MAX - 1);
		pMultipart->pBody->offset = 0;
		pMultipart->pBody->size = MsgGetFileSize(szOrgFilePath);
	}
	return pMultipart;
}

void MmsComposeNotiMessage(MmsMsg *pMmsMsg, msg_message_id_t msgID)
{
	MSG_BEGIN();

	struct tm timeInfo;
	time_t RawTime = 0;
	time_t nTimeInSecs = 0;

	MmsInitMsgAttrib(&pMmsMsg->mmsAttrib);
	MmsInitMsgType(&pMmsMsg->msgType);
	MmsInitMsgBody(&pMmsMsg->msgBody);

	pMmsMsg->msgID = msgID;

	pMmsMsg->mmsAttrib.version = mmsHeader.version;

	/* setting date */
	time(&RawTime);
	localtime_r(&RawTime, &timeInfo);
	nTimeInSecs = mktime(&timeInfo);

	pMmsMsg->mmsAttrib.date = nTimeInSecs;

	pMmsMsg->mmsAttrib.bReportAllowed = (mmsHeader.reportAllowed != MMS_REPORTALLOWED_YES);
	pMmsMsg->mmsAttrib.bAskDeliveryReport = (mmsHeader.deliveryReport != MMS_REPORT_YES);

	MSG_DEBUG("######## Version = %d ########", pMmsMsg->mmsAttrib.version);

	strncpy(pMmsMsg->szTrID, mmsHeader.szTrID, MMS_TR_ID_LEN);
	strncpy(pMmsMsg->szMsgID, mmsHeader.szMsgID, MMS_MSG_ID_LEN);
	pMmsMsg->szForwardMsgID[0] = '\0';

	if (mmsHeader.pFrom) {
		MmsAddrUtilRemovePlmnString(mmsHeader.pFrom->szAddr);
		strncpy(pMmsMsg->mmsAttrib.szFrom, mmsHeader.pFrom->szAddr, MSG_LOCALE_ADDR_LEN + 9);
	}

	strncpy(pMmsMsg->mmsAttrib.szSubject, mmsHeader.szSubject, MSG_LOCALE_SUBJ_LEN);
	strncpy(pMmsMsg->szContentLocation, mmsHeader.szContentLocation, MMS_LOCATION_LEN);

	pMmsMsg->mmsAttrib.msgClass = mmsHeader.msgClass;
	pMmsMsg->mmsAttrib.msgSize = mmsHeader.msgSize;
	pMmsMsg->mmsAttrib.expiryTime.type = mmsHeader.expiryTime.type;
	pMmsMsg->mmsAttrib.expiryTime.time = mmsHeader.expiryTime.time;
	pMmsMsg->mmsAttrib.dataType = MMS_DATATYPE_NOTIFY;
	pMmsMsg->mmsAttrib.bRead = false;
	pMmsMsg->mailbox = MSG_INBOX_ID;

	pMmsMsg->mmsAttrib.replyCharge.chargeType = mmsHeader.replyCharge.chargeType;
	pMmsMsg->mmsAttrib.replyCharge.deadLine.type = mmsHeader.replyCharge.deadLine.type;
	pMmsMsg->mmsAttrib.replyCharge.deadLine.time = mmsHeader.replyCharge.deadLine.time;
	pMmsMsg->mmsAttrib.replyCharge.chargeSize = mmsHeader.replyCharge.chargeSize;

	strncpy(pMmsMsg->mmsAttrib.replyCharge.szChargeID, mmsHeader.replyCharge.szChargeID, MMS_MSG_ID_LEN);

	MSG_END();
}

void MmsComposeReadReportMessage(MmsMsg *pMmsMsg, const MSG_MESSAGE_INFO_S *pMsgInfo, msg_message_id_t selectedMsgId)
{
	struct tm timeInfo;
	time_t RawTime = 0;
	time_t nTimeInSecs = 0;

	MmsInitMsgAttrib(&pMmsMsg->mmsAttrib);
	MmsInitMsgType(&pMmsMsg->msgType);
	MmsInitMsgBody(&pMmsMsg->msgBody);

	/* setting mmsMsg structure */
	pMmsMsg->mailbox = pMsgInfo->folderId;
	pMmsMsg->msgID = pMsgInfo->msgId;

	memset(pMmsMsg->szTrID, 0, MMS_TR_ID_LEN + 1);
	memset(pMmsMsg->szContentLocation, 0, MMS_LOCATION_LEN + 1);
	memset(pMmsMsg->szForwardMsgID, 0, MMS_MSG_ID_LEN + 1);

	pMmsMsg->mmsAttrib.dataType = MMS_DATATYPE_DRAFT;

	/* setting date */
	time(&RawTime);
	localtime_r(&RawTime, &timeInfo);
	nTimeInSecs = mktime(&timeInfo);
	pMmsMsg->mmsAttrib.date = nTimeInSecs;

	/* setting szMsgId */
	MmsPluginStorage::instance()->getMmsMessageId(selectedMsgId, pMmsMsg);

	/* setting subject */
	snprintf(pMmsMsg->mmsAttrib.szSubject, sizeof(pMmsMsg->mmsAttrib.szSubject), "%s", pMsgInfo->subject);

	/* setting adddress */
	MmsSetMsgAddressList(&pMmsMsg->mmsAttrib, pMsgInfo);

	if (pMmsMsg->mmsAttrib.szTo)
		snprintf(pMmsMsg->mmsAttrib.szFrom, sizeof(pMmsMsg->mmsAttrib.szFrom), "%s", pMmsMsg->mmsAttrib.szTo);
}

msg_error_t MmsMakeMultipartThumbnailInfo(MMS_MULTIPART_DATA_S *pMultipart, char *thumbnail_path)
{
	if (pMultipart == NULL || thumbnail_path == NULL)
		return MSG_ERR_NULL_POINTER;

/*
	if (MimeGetMainTypeString(MimeGetMimeStringFromMimeInt(pMultipart->type)) != MIME_MAINTYPE_VIDEO)
		return MSG_ERR_INVALID_PARAMETER;
*/

	char szFileName[MSG_FILENAME_LEN_MAX+1] = {0, };
	char szFileNameWoExt[MSG_FILENAME_LEN_MAX+1] = {0, };
	char thumbPath[MSG_FILEPATH_LEN_MAX+1] = {0, };
	char *pszExt = NULL;
	char *pszOrgFileName = NULL;

	memset(szFileName, 0x00, MSG_FILENAME_LEN_MAX+1);
	memset(thumbPath, 0x00, MSG_FILEPATH_LEN_MAX);

	MSG_SEC_DEBUG("drm type = %d, %s", pMultipart->drmType, pMultipart->szFilePath);

	if (pMultipart->drmType == MSG_DRM_TYPE_NONE) {
		pszOrgFileName = strrchr(pMultipart->szFilePath, '/');

		if (pszOrgFileName) {
			pszExt = strrchr(pszOrgFileName, '.');
		} else {
			MSG_DEBUG("Fail in getting filename without extension string");
			return MSG_ERR_PLUGIN_STORAGE;
		}

		if (pszExt) {
			strncpy(szFileNameWoExt, pszOrgFileName + 1, strlen(pszOrgFileName + 1) - strlen(pszExt));
		} else {
			if (strlen(pszOrgFileName + 1) <= sizeof(szFileNameWoExt))
				strncpy(szFileNameWoExt, pszOrgFileName + 1, strlen(pszOrgFileName + 1));
			else
				strncpy(szFileNameWoExt, pszOrgFileName + 1, sizeof(szFileNameWoExt));
		}
		snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "thumb_msg_%s", szFileNameWoExt);

		if (pszExt && !strcasecmp(pszExt, ".png")) {
			snprintf(thumbPath, MSG_FILEPATH_LEN_MAX, "%s%s.png", MSG_THUMBNAIL_PATH, szFileName);
		} else {
			snprintf(thumbPath, MSG_FILEPATH_LEN_MAX, "%s%s.jpg", MSG_THUMBNAIL_PATH, szFileName);
		}

		if (MakeThumbnail(pMultipart->szFilePath, thumbPath) == true) {
			memcpy(thumbnail_path, &thumbPath, strlen(thumbPath));
			MSG_SEC_DEBUG("Generated thumbnail: %s ", thumbnail_path);
			return MSG_SUCCESS;
		} else {
			MSG_SEC_DEBUG("Fail of generating thumbnail: %s to %s", pMultipart->szFilePath, thumbPath);
		}
	}

	return MSG_ERR_PLUGIN_STORAGE;
}

void MmsPrintFileInfoForVLD(MMS_MESSAGE_DATA_S *pMmsMsg)
{
	MMS_PAGE_S *pPage = NULL;
	MMS_MEDIA_S *pMedia = NULL;

	if (pMmsMsg == NULL)
		return;

	if (pMmsMsg->pageCnt > 0) {
		for (int i = 0; i < pMmsMsg->pageCnt; i++) {
			pPage = _MsgMmsGetPage(pMmsMsg, i);

			if (pPage == NULL)
				continue;

			for (int j = 0; j < pPage->mediaCnt; j++) {
				pMedia = _MsgMmsGetMedia(pPage, j);
				if (pMedia == NULL)
					continue;

				MSG_MMS_VLD_FILE("[%s], %d", pMedia->szFilePath, MsgGetFileSize(pMedia->szFilePath));
			}
		}
	}

	int attachCnt = _MsgMmsGetAttachCount(pMmsMsg);
	if (attachCnt > 0) {
		for (int i = 0; i < pMmsMsg->attachCnt; i++) {
			MMS_ATTACH_S *pAttach = _MsgMmsGetAttachment(pMmsMsg, i);
			MSG_SEC_DEBUG("[%s], %d", pAttach->szFilePath, MsgGetFileSize(pAttach->szFilePath));
		}
	}
}

bool IsMatchedMedia(MMS_MEDIA_S *media, MMS_MULTIPART_DATA_S *pMultipart)
{
	if (strlen(pMultipart->szContentID) > 0) {
		char szTempContentID[MSG_MSG_ID_LEN + 1] = {0, };
		MmsRemoveLessGreaterChar(pMultipart->szContentID, szTempContentID, sizeof(szTempContentID));

		if (strcmp(media->szContentID,  szTempContentID) == 0) {
			return true;
		}

		if (strcmp(media->szContentLocation,  szTempContentID) == 0) {
			return true;
		}
	}

	if (strlen(pMultipart->szContentLocation) > 0) {
		if (strcmp(media->szContentID,  pMultipart->szContentLocation) == 0) {
			return true;
		}

		if (strcmp(media->szContentLocation,  pMultipart->szContentLocation) == 0) {
			return true;
		}
	}

	MSG_SEC_DEBUG("There is not matched media cid [%s], cl, [%s], multipart cid [%s], cl, [%s]", media->szContentID, media->szContentLocation, pMultipart->szContentID, pMultipart->szContentLocation);
	return false;
}

MsgMultipart *MmsGetNthMultipart(MmsMsg *pMsg, int index)
{
	MsgMultipart *pPart = NULL;

	if (MsgIsMultipart(pMsg->msgType.type) == true) {
		pPart = pMsg->msgBody.body.pMultipart;

		while (pPart && index--) {
			pPart = pPart->pNext;
		}

		if (pPart == NULL) {
			MSG_DEBUG("There is no such Multipart [index = %d].", index);
			return NULL;
		} else {
			return pPart;
		}
	} else {
		MSG_DEBUG("This Msg is not Multipart");
	}

	return NULL;
}

bool MmsFindAndInsertPart(MMS_MESSAGE_DATA_S *pMsgData, MMS_MULTIPART_DATA_S *pMultipart)
{
	bool insert_media = false;

	if (pMsgData->pagelist) {
		for (int pageIdx = 0; pageIdx < pMsgData->pageCnt; pageIdx++) {
			MMS_PAGE_S *page = (MMS_PAGE_S *)g_list_nth_data(pMsgData->pagelist, pageIdx);

			if (page && page->medialist) {
				for (int mediaIdx = 0; mediaIdx < page->mediaCnt; mediaIdx++) {
					MMS_MEDIA_S *media = (MMS_MEDIA_S *)g_list_nth_data(page->medialist, mediaIdx);

					if (media) { /* add media */
						if (IsMatchedMedia(media, pMultipart) == true) { /* matched media */
#if 0
							if (media->mediatype == MMS_SMIL_MEDIA_IMG_OR_VIDEO) { /* ref type is not insert part */
								MSG_DEBUG("## Matched but media type is ref ##");
								return false;
							}
#endif

							insert_media = true;
							media->drmType = pMultipart->drmType;
							snprintf(media->szFilePath, sizeof(media->szFilePath), "%s", pMultipart->szFilePath);
							snprintf(media->szFileName, sizeof(media->szFileName), "%s", pMultipart->szFileName);
							snprintf(media->szContentID, sizeof(media->szContentID), "%s", pMultipart->szContentID);
							snprintf(media->szContentLocation, sizeof(media->szContentLocation), "%s", pMultipart->szContentLocation);
							snprintf(media->szContentType, sizeof(media->szContentType), "%s", pMultipart->szContentType);

							MSG_SEC_DEBUG("InsertPart to pageIndx [%d] mediaIdx[%d] media[%p] : path = [%s], name = [%s], cid = [%s], cl = [%s], ct = [%s]"\
									, pageIdx, mediaIdx, media, media->szFilePath, media->szFileName, media->szContentID, media->szContentLocation, media->szContentType);
						}
					}
				} /* end for media list */
			}
		} /* end for page list */
	}

	return insert_media;
}

bool MmsInsertPartToMmsData(MMS_MESSAGE_DATA_S *pMsgData, MMS_MULTIPART_DATA_S *pMultipart)
{
	MSG_BEGIN();

	bool isInsert = false;

	if (pMsgData == NULL || pMultipart == NULL) {
		return false;
	}

	isInsert = MmsFindAndInsertPart(pMsgData, pMultipart);

	if (isInsert == false) {
		MMS_ATTACH_S *attachment = NULL;
		attachment = (MMS_ATTACH_S *)calloc(sizeof(MMS_ATTACH_S), 1);
		if (attachment) {
			attachment->drmType = pMultipart->drmType;

			if (strlen(pMultipart->szContentType) > 0) {
				snprintf(attachment->szContentType, sizeof(attachment->szContentType), "%s", pMultipart->szContentType);
				attachment->mediatype =  pMultipart->type;
			}

			snprintf(attachment->szFilePath, sizeof(attachment->szFilePath), "%s", pMultipart->szFilePath);
			snprintf(attachment->szFileName, sizeof(attachment->szFileName), "%s", pMultipart->szFileName);
			attachment->fileSize = MsgGetFileSize(attachment->szFilePath);

			MSG_SEC_DEBUG("Insert Attach to attachment[%p] : path = [%s], name = [%s], ct = [%s], size = [%d]"\
							, attachment, attachment->szFilePath, attachment->szFileName, attachment->szContentType, attachment->fileSize);

			if (_MsgMmsAddAttachment(pMsgData, attachment) != MSG_SUCCESS) {
				g_free(attachment);
				return false;
			}
		} else {
			return false;
		}
	}

	MSG_END();
	return true;
}

bool MmsInsertMixedPartToMmsData(MMS_MESSAGE_DATA_S *pMsgData, MMS_MULTIPART_DATA_S *pMultipart)
{
	MSG_BEGIN();

	if (pMsgData == NULL || pMultipart == NULL) {
		return false;
	}

	MimeMainType mainType = MimeGetMainTypeInt(pMultipart->type);
	MmsSmilMediaType mediatype = MMS_SMIL_MEDIA_INVALID;

	switch(mainType) {
	case MIME_MAINTYPE_AUDIO:
		mediatype = MMS_SMIL_MEDIA_AUDIO;
		break;
	case MIME_MAINTYPE_IMAGE:
		mediatype = MMS_SMIL_MEDIA_IMG;
		break;
	case MIME_MAINTYPE_TEXT:
		mediatype = MMS_SMIL_MEDIA_TEXT;
		break;
	case MIME_MAINTYPE_VIDEO:
		mediatype = MMS_SMIL_MEDIA_VIDEO;
		break;
	default:
		mediatype = MMS_SMIL_MEDIA_INVALID;
		break;
	}

	if (mediatype != MMS_SMIL_MEDIA_INVALID) {
		MMS_PAGE_S *pPage = (MMS_PAGE_S *)calloc(1, sizeof(MMS_PAGE_S));
		MMS_MEDIA_S *media = (MMS_MEDIA_S *)calloc(1, sizeof(MMS_MEDIA_S));
		if (pPage && media) {
			media->mediatype = mediatype;
			media->drmType = pMultipart->drmType;
			snprintf(media->szFilePath, sizeof(media->szFilePath), "%s", pMultipart->szFilePath);
			snprintf(media->szFileName, sizeof(media->szFileName), "%s", pMultipart->szFileName);
			snprintf(media->szContentID, sizeof(media->szContentID), "%s", pMultipart->szContentID);
			snprintf(media->szContentLocation, sizeof(media->szContentLocation), "%s", pMultipart->szContentLocation);
			snprintf(media->szContentType, sizeof(media->szContentType), "%s", pMultipart->szContentType);

			MSG_SEC_DEBUG("InsertPart to media[%p] type[%d] : path = [%s], name = [%s], cid = [%s], cl = [%s], ct = [%s]"\
					, media, mediatype,  media->szFilePath, media->szFileName, media->szContentID, media->szContentLocation, media->szContentType);

			if (_MsgMmsAddMedia(pPage, media) != MSG_SUCCESS) {
				g_free(pPage);
				g_free(media);
				return false;
			}

			if (_MsgMmsAddPage(pMsgData, pPage) != MSG_SUCCESS) {
				g_free(pPage);
				g_free(media);
				return false;
			}
		} else {
			g_free(pPage);
			g_free(media);
			return false;
		}
	} else {
		MMS_ATTACH_S *attachment = NULL;
		attachment = (MMS_ATTACH_S *)calloc(sizeof(MMS_ATTACH_S), 1);
		if (attachment) {
			attachment->mediatype =  pMultipart->type;
			attachment->drmType = pMultipart->drmType;
			snprintf(attachment->szContentType, sizeof(attachment->szContentType), "%s", pMultipart->szContentType);
			snprintf(attachment->szFilePath, sizeof(attachment->szFilePath), "%s", pMultipart->szFilePath);
			snprintf(attachment->szFileName, sizeof(attachment->szFileName), "%s", pMultipart->szFileName);
			attachment->fileSize = MsgGetFileSize(attachment->szFilePath);
			MSG_SEC_DEBUG("Insert Attach to attachment[%p] : path = [%s], name = [%s], ct = [%s], size = [%d]"\
							, attachment, attachment->szFilePath, attachment->szFileName, attachment->szContentType, attachment->fileSize);

			if (_MsgMmsAddAttachment(pMsgData, attachment) != MSG_SUCCESS) {
				g_free(attachment);
				return false;
			}
		} else {
			return false;
		}
	}

	MSG_END();
	return true;
}

bool MmsInsertPartFromMultipart(MmsMsg *pMsg, MMS_MULTIPART_DATA_S *pNewMultipart)
{
	MsgMultipart *pMultipart = NULL;
	MsgMultipart *pLastPart = NULL;

	if (MmsIsMultipart(pMsg->msgType.type) == true) {
		/* Insert as a multipart */
		pMultipart = MmsMakeMultipart(pNewMultipart->type, pNewMultipart->szFileName, pNewMultipart->szFilePath, pNewMultipart->szContentID, pNewMultipart->szContentLocation);

		if (pMultipart == NULL)
			goto __CATCH;

		if (pMsg->mmsAttrib.contentType == MIME_APPLICATION_VND_WAP_MULTIPART_MIXED ||
			pMsg->mmsAttrib.contentType == MIME_MULTIPART_MIXED)
			pMultipart->type.disposition = MSG_DISPOSITION_ATTACHMENT;

		if (pMsg->msgBody.body.pMultipart == NULL) {
			pMsg->msgBody.body.pMultipart = pMultipart;
		} else {
			pLastPart = pMsg->msgBody.body.pMultipart;
			while (pLastPart->pNext) {
				pLastPart = pLastPart->pNext;
			}

			pLastPart->pNext = pMultipart;
		}

		pMsg->msgBody.size += pMultipart->pBody->size;
		pMsg->msgType.contentSize += pMultipart->pBody->size;
	} else {
		/* Single part - Insert as a message body */
		if (pMsg->mmsAttrib.contentType != pNewMultipart->type || pMsg->msgType.type != pNewMultipart->type)
			goto __CATCH;

		strncpy(pMsg->msgType.param.szName, pNewMultipart->szFileName, MSG_LOCALE_FILENAME_LEN_MAX);

		if (MmsIsTextType(pMsg->msgType.type) == true) {
			pMsg->msgType.param.charset = MSG_CHARSET_UTF8;
		}

		strncpy(pMsg->msgBody.szOrgFilePath, pNewMultipart->szFilePath, MSG_FILEPATH_LEN_MAX - 1);
		pMsg->msgBody.offset = 0;
		pMsg->msgBody.size = MsgGetFileSize(pNewMultipart->szFilePath);
		pMsg->msgType.contentSize = MsgGetFileSize(pNewMultipart->szFilePath);
	}

	pMsg->nPartCount++;

	return true;

__CATCH:
	return false;
}

bool MmsConvertMsgData(MmsMsg *pMsg, MMS_MESSAGE_DATA_S *pMmsMsg)
{
	MSG_BEGIN();

/*	bzero(pMmsMsg, sizeof(MMS_MESSAGE_DATA_S)); */
	pMmsMsg->regionCnt = 0;
	pMmsMsg->pageCnt = 0;
	pMmsMsg->attachCnt = 0;
	pMmsMsg->transitionCnt = 0;
	pMmsMsg->metaCnt = 0;
	memset(pMmsMsg->szSmilFilePath, 0, MSG_FILEPATH_LEN_MAX);

	if (pMsg->mmsAttrib.contentType == MIME_MULTIPART_RELATED || pMsg->mmsAttrib.contentType == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {
		char *pSmilDoc = NULL;
		if (pMsg->msgBody.pPresentationBody) {
			if (pMsg->msgBody.pPresentationBody->body.pText) {
				pSmilDoc = pMsg->msgBody.pPresentationBody->body.pText;
				if (pSmilDoc) {
					MsgSmilParseSmilDoc(pMmsMsg, pSmilDoc);
				}

				pMmsMsg->smil.type = MIME_APPLICATION_SMIL;
				snprintf(pMmsMsg->smil.szContentType, MSG_MSG_ID_LEN, "%s", MimeGetMimeStringFromMimeInt(pMsg->msgBody.presentationType.type));
				snprintf(pMmsMsg->smil.szContentID, MSG_MSG_ID_LEN, "%s", pMsg->msgBody.presentationType.szContentID);
				snprintf(pMmsMsg->smil.szContentLocation, MSG_MSG_ID_LEN, "%s", pMsg->msgBody.presentationType.szContentLocation);
				snprintf(pMmsMsg->smil.szFileName, MSG_FILENAME_LEN_MAX, "%s", pMsg->msgBody.presentationType.param.szName);
				snprintf(pMmsMsg->smil.szFilePath, MSG_FILEPATH_LEN_MAX, "%s%s", MSG_DATA_PATH, pMsg->msgBody.presentationType.param.szFileName);
			}
		} else {
			MSG_DEBUG("Not Exist pPresentationBody");
		}
	}

	/*If mms content type is TEXTPLAN add to First Page*/
	if (pMsg->mmsAttrib.contentType == MIME_TEXT_PLAIN) {
		MsgType partHeader;
		int partCnt = pMsg->nPartCount;

		if (partCnt <= 0) {
			MSG_DEBUG("partCnt=%d\n", partCnt);
		} else {
			if (MmsGetMediaPartHeader(0, &partHeader) == false) {
				MSG_DEBUG("Failed to get MediaPart MmsGetMediaPartHeader");
				goto FREE_CATCH;
			}

			if (partHeader.contentSize > 0) {
				char szBuf[MSG_FILEPATH_LEN_MAX + 1] = {0, };
				MMS_PAGE_S *page = NULL;
				MMS_MEDIA_S *media = NULL;

				snprintf(szBuf, sizeof(szBuf), "%s", partHeader.param.szFileName);
				snprintf(partHeader.param.szFileName, sizeof(partHeader.param.szFileName), "%s%s", MSG_DATA_PATH, szBuf);

				page = (MMS_PAGE_S *)calloc(1, sizeof(MMS_PAGE_S));
				if (page == NULL) {
					MSG_FATAL("page allocation error");
					goto FREE_CATCH;
				}

				media = (MMS_MEDIA_S *)calloc(1, sizeof(MMS_MEDIA_S));
				if (media == NULL) {
					MSG_FATAL("media allocation error");
					free(page);
					goto FREE_CATCH;
				}

				media->mediatype = MMS_SMIL_MEDIA_TEXT;
				snprintf(media->szFilePath, sizeof(media->szFilePath), "%s", partHeader.param.szFileName);
				snprintf(media->szFileName, sizeof(media->szFileName), "%s", partHeader.param.szName);
				snprintf(media->szContentID, sizeof(media->szContentID), "%s", partHeader.szContentID);
				snprintf(media->szContentLocation, sizeof(media->szContentLocation), "%s", partHeader.szContentLocation);

				_MsgMmsAddMedia(page, media);

				_MsgMmsAddPage(pMmsMsg, page);
			}
		}
	} else {
		int partCnt = pMsg->nPartCount;

		for (int i = 0; i < partCnt; ++i) {
			MsgMultipart *multipart = MmsGetNthMultipart(pMsg, i);
			MMS_MULTIPART_DATA_S pMultipart;

			if (multipart == NULL) {
				MSG_DEBUG("multipart is NULL [%d]", i);
				goto FREE_CATCH;
			}

			bzero(&pMultipart, sizeof(MMS_MULTIPART_DATA_S));

			pMultipart.type = (MimeType)multipart->type.type;
			MSG_DEBUG("Mime Type : %s :%d", MimeGetMimeStringFromMimeInt(multipart->type.type), multipart->type.type);
			snprintf(pMultipart.szContentID, sizeof(pMultipart.szContentID), "%s", multipart->type.szContentID);
			snprintf(pMultipart.szContentLocation, sizeof(pMultipart.szContentLocation), "%s", multipart->type.szContentLocation);
			snprintf(pMultipart.szFileName, sizeof(pMultipart.szFileName), "%s", multipart->type.param.szName);
			snprintf(pMultipart.szFilePath, sizeof(pMultipart.szFilePath), "%s", multipart->pBody->szOrgFilePath);
			snprintf(pMultipart.szContentType, sizeof(pMultipart.szContentType), "%s", MimeGetMimeStringFromMimeInt(multipart->type.type));

			if (multipart->type.drmInfo.drmType != MSG_DRM_TYPE_NONE) {
				pMultipart.drmType = multipart->type.drmInfo.drmType;
			}

			_MsgMmsMultipartPrint(&pMultipart);

			if (pMsg->mmsAttrib.contentType == MIME_MULTIPART_RELATED || pMsg->mmsAttrib.contentType == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {
				if (MmsInsertPartToMmsData(pMmsMsg, &pMultipart) == false) {
					MSG_DEBUG("Fail to MmsSetMultipartToMmsData");
					goto FREE_CATCH;
				}
			} else {
				if (MmsInsertMixedPartToMmsData(pMmsMsg, &pMultipart) == false) {
					MSG_DEBUG("Fail to MmsSetMultipartToMmsData");
					goto FREE_CATCH;
				}
			}
		}
	}

	_MsgMmsRemoveEmptyObject(pMmsMsg);

	MSG_END();
	return true;
FREE_CATCH:
	return false;
}

void MmsPrintMmsMsg(const MmsMsg *pMmsMsg)
{
	MmsMsgType msgType = pMmsMsg->mmsAttrib.msgType;
	if (msgType == MMS_MSGTYPE_SEND_REQ) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_SEND_REQ");
	} else if (msgType == MMS_MSGTYPE_SEND_CONF) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_SEND_CONF");
	} else if (msgType == MMS_MSGTYPE_NOTIFICATION_IND) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_NOTIFICATION_IND");
	} else if (msgType == MMS_MSGTYPE_NOTIFYRESP_IND) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_NOTIFYRESP_IND");
	} else if (msgType == MMS_MSGTYPE_RETRIEVE_CONF) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_RETRIEVE_CONF");
	} else if (msgType == MMS_MSGTYPE_ACKNOWLEDGE_IND) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_ACKNOWLEDGE_IND");
	} else if (msgType == MMS_MSGTYPE_DELIVERY_IND) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_DELIVERY_IND");
	} else if (msgType == MMS_MSGTYPE_READREC_IND) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_READREC_IND");
	} else if (msgType == MMS_MSGTYPE_READORG_IND) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_READORG_IND");
	} else if (msgType == MMS_MSGTYPE_FORWARD_REQ) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_FORWARD_REQ");
	} else if (msgType == MMS_MSGTYPE_FORWARD_CONF) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_FORWARD_CONF");
	} else if (msgType == MMS_MSGTYPE_READ_REPLY) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_READ_REPLY");
	} else {
		PRINT_KEY_VAL_STR("mms type", "Unknown");
	}

	PRINT_KEY_VAL_STR("mms ver", pMmsMsg->mmsAttrib.version);
}

int MmsUpdateMultipartList(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();
	MMSList *multipart_list = NULL;

	if (MmsPluginStorage::instance()->getMultipartList(pMsgInfo->msgId, &multipart_list) != MSG_SUCCESS)
		return -1;

	for (int i = 0; i < (int)g_list_length(multipart_list); i++) {
		MMS_MULTIPART_DATA_S *pMultipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(multipart_list, i);
		MmsPluginStorage::instance()->updateMultipart(pMsgInfo->msgId, true, pMultipart);
	}

	MSG_END();
	return 0;
}

int MmsUpdatePreviewData(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	char szFullPath[MSG_FILEPATH_LEN_MAX] = {0, };

	MmsMsg *mmsMsg = NULL;
	unique_ptr<MmsMsg*, void(*)(MmsMsg**)> buf(&mmsMsg, unique_ptr_deleter);
	mmsMsg = (MmsMsg *)new char[sizeof(MmsMsg)];
	memset(mmsMsg, 0x00, sizeof(MmsMsg));

	MmsPluginStorage::instance()->getMmsRawFilePath(pMsgInfo->msgId, szFullPath, sizeof(szFullPath));
	MmsPluginDecoder::instance()->decodeMmsPdu(mmsMsg, pMsgInfo->msgId, szFullPath);

	/* make Preview info for APP */
	MmsPluginAppBase appBase(mmsMsg);
	appBase.makePreviewInfo(pMsgInfo->msgId, true, szFullPath);
	appBase.getFirstPageTextFilePath(pMsgInfo->msgText, sizeof(pMsgInfo->msgText));

	MmsReleaseMmsMsg(mmsMsg);
	MSG_END();
	return 0;
}

/* MmsMsg -> MMS_DATA_S */
bool MmsConvertMmsData(MmsMsg *pMmsMsg, MMS_DATA_S *pMmsData)
{
	MSG_BEGIN();

	MMS_HEADER_DATA_S *pHeaderData = pMmsData->header;
	if (pHeaderData) {
		snprintf(pHeaderData->messageID,  sizeof(pHeaderData->messageID), "%s", pMmsMsg->szMsgID);
		snprintf(pHeaderData->trID,  sizeof(pHeaderData->trID), "%s", pMmsMsg->szTrID);
		snprintf(pHeaderData->contentLocation,  sizeof(pHeaderData->contentLocation), "%s", pMmsMsg->szContentLocation);
		snprintf(pHeaderData->szContentType,  sizeof(pHeaderData->szContentType), "%s", MimeGetMimeStringFromMimeInt(pMmsMsg->mmsAttrib.contentType));

		pHeaderData->messageType = pMmsMsg->mmsAttrib.msgType;
		pHeaderData->mmsVersion = pMmsMsg->mmsAttrib.version;
		pHeaderData->messageClass = pMmsMsg->mmsAttrib.msgClass;
		pHeaderData->contentClass = 0;
		pHeaderData->mmsPriority = pMmsMsg->mmsAttrib.priority;
		pHeaderData->expiry.type = pMmsMsg->mmsAttrib.expiryTime.type;
		pHeaderData->expiry.time = pMmsMsg->mmsAttrib.expiryTime.time;

		pHeaderData->bDeliveryReport = pMmsMsg->mmsAttrib.bAskDeliveryReport;
		pHeaderData->date = pMmsMsg->mmsAttrib.date;
		pHeaderData->mmsVersion = pMmsMsg->mmsAttrib.version;

		pHeaderData->bReadReport = pMmsMsg->mmsAttrib.bAskReadReply;
		pHeaderData->bHideAddress = pMmsMsg->mmsAttrib.bHideAddress;

		snprintf(pHeaderData->szSubject, sizeof(pHeaderData->szSubject), "%s", pMmsMsg->mmsAttrib.szSubject);

		pHeaderData->to = MmsConvertAddressToNewStyle(pMmsMsg->mmsAttrib.szTo);
		pHeaderData->cc = MmsConvertAddressToNewStyle(pMmsMsg->mmsAttrib.szCc);
		pHeaderData->bcc = MmsConvertAddressToNewStyle(pMmsMsg->mmsAttrib.szBcc);

		snprintf(pHeaderData->szFrom, sizeof(pHeaderData->szFrom), "%s", pMmsMsg->mmsAttrib.szFrom);
	}

	if (pMmsMsg->mmsAttrib.contentType == MIME_MULTIPART_RELATED || pMmsMsg->mmsAttrib.contentType == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {
		MMS_MULTIPART_DATA_S *pMultipart = MsgMmsCreateMultipart();
		if (pMultipart) {
			pMultipart->type = MIME_APPLICATION_SMIL;
			snprintf(pMultipart->szContentType, sizeof(pMultipart->szContentType), "%s", "application/smil");
			snprintf(pMultipart->szContentID, sizeof(pMultipart->szContentID), "%s", pMmsMsg->msgBody.presentationType.szContentID);
			snprintf(pMultipart->szContentLocation, sizeof(pMultipart->szContentLocation), "%s", pMmsMsg->msgBody.presentationType.szContentLocation);
			snprintf(pMultipart->szFileName, sizeof(pMultipart->szFileName), "%s", pMmsMsg->msgBody.presentationType.param.szName);
			snprintf(pMultipart->szFilePath, sizeof(pMultipart->szFilePath), "%s%s", MSG_DATA_PATH, pMmsMsg->msgBody.presentationType.param.szFileName);

			pMmsData->smil = pMultipart;
		}
	}

	int partCnt = pMmsMsg->nPartCount;

	for (int i = 0; i < partCnt; ++i) {
		MsgMultipart *multipart = MmsGetNthMultipart(pMmsMsg, i);

		if (multipart) {
			MMS_MULTIPART_DATA_S *pMultipart = MsgMmsCreateMultipart();

			if (pMultipart) {
				pMultipart->type = (MimeType)multipart->type.type;

				snprintf(pMultipart->szContentType, sizeof(pMultipart->szContentType), "%s", MimeGetMimeStringFromMimeInt(multipart->type.type));
				snprintf(pMultipart->szContentID, sizeof(pMultipart->szContentID), "%s", multipart->type.szContentID);
				snprintf(pMultipart->szContentLocation, sizeof(pMultipart->szContentLocation), "%s", multipart->type.szContentLocation);
				snprintf(pMultipart->szFileName, sizeof(pMultipart->szFileName), "%s", multipart->type.param.szName);
				snprintf(pMultipart->szFilePath, sizeof(pMultipart->szFilePath), "%s", multipart->pBody->szOrgFilePath);

#ifdef __SUPPORT_DRM__
				if (multipart->type.drmInfo.drmType != MSG_DRM_TYPE_NONE) {
					pMultipart->drmType = multipart->type.drmInfo.drmType;
				}
#endif
				pMmsData->multipartlist = g_list_append(pMmsData->multipartlist, pMultipart);
			}
		}
	}

	MSG_END();
	return true;
}

/* For Encode raw file */
bool MmsConvertMmsMsg(MmsMsg *pMmsMsg, MMS_DATA_S *pMmsData)
{
	MSG_BEGIN();

	/* Initialize mmsMsg structure */
	MmsInitMsgAttrib(&pMmsMsg->mmsAttrib);
	MmsInitMsgType(&pMmsMsg->msgType);
	MmsInitMsgBody(&pMmsMsg->msgBody);

	pMmsMsg->mmsAttrib.dataType = MMS_DATATYPE_DRAFT;

	MMS_HEADER_DATA_S *pHeaderData = pMmsData->header;

	if (pHeaderData) {
		if (strlen(pHeaderData->contentLocation) > 0) {
			snprintf(pMmsMsg->szContentLocation, sizeof(pMmsMsg->szContentLocation), "%s", pHeaderData->contentLocation);
		}

		pMmsMsg->mmsAttrib.contentType = (MimeType)pHeaderData->contentType;
		pMmsMsg->msgType.type = pHeaderData->contentType;
		pMmsMsg->mmsAttrib.date = pHeaderData->date;
		pMmsMsg->mmsAttrib.bAskDeliveryReport = pHeaderData->bDeliveryReport;
		pMmsMsg->mmsAttrib.deliveryTime.type = pHeaderData->delivery.type;
		pMmsMsg->mmsAttrib.deliveryTime.time = pHeaderData->delivery.time;
		pMmsMsg->mmsAttrib.expiryTime.type = pHeaderData->expiry.type;
		pMmsMsg->mmsAttrib.expiryTime.time = pHeaderData->expiry.time;
		pMmsMsg->mmsAttrib.msgClass = (MmsMsgClass)pHeaderData->messageClass;

		if (strlen(pHeaderData->messageID) > 0) {
			snprintf(pMmsMsg->szMsgID, sizeof(pMmsMsg->szMsgID), "%s", pHeaderData->messageID);
		}

		pMmsMsg->mmsAttrib.msgType = (MmsMsgType)pHeaderData->messageType;
		pMmsMsg->mmsAttrib.version  = pHeaderData->mmsVersion;
		pMmsMsg->mmsAttrib.priority = (MmsPriority)pHeaderData->mmsPriority;
		pMmsMsg->mmsAttrib.bAskReadReply = pHeaderData->bReadReport;
		pMmsMsg->mmsAttrib.bHideAddress = pHeaderData->bHideAddress;

		if (strlen(pHeaderData->trID) > 0) {
			snprintf(pMmsMsg->szTrID, sizeof(pMmsMsg->szTrID), "%s", pHeaderData->trID);
		}


		snprintf(pMmsMsg->mmsAttrib.szSubject, sizeof(pMmsMsg->mmsAttrib.szSubject), "%s", pHeaderData->szSubject);

		pMmsMsg->mmsAttrib.szTo = MmsConvertAddressToOldStyle(pHeaderData->to);
		pMmsMsg->mmsAttrib.szCc = MmsConvertAddressToOldStyle(pHeaderData->cc);
		pMmsMsg->mmsAttrib.szBcc = MmsConvertAddressToOldStyle(pHeaderData->bcc);

		snprintf(pMmsMsg->mmsAttrib.szFrom, sizeof(pMmsMsg->mmsAttrib.szFrom), "%s", pHeaderData->szFrom);
	} /* CID 41988: Moving all de-referencing of pHeaderData inside null-check block */

	printMmsAttribute(&pMmsMsg->mmsAttrib);

	if (pMmsData->multipartlist) {
		if (pMmsData->smil) {
			MMS_MULTIPART_DATA_S *smil_multipart = pMmsData->smil;

			if (smil_multipart) {
				pMmsMsg->mmsAttrib.contentType = MIME_APPLICATION_VND_WAP_MULTIPART_RELATED;
				pMmsMsg->msgType.type = MIME_APPLICATION_VND_WAP_MULTIPART_RELATED;

				gchar *contents = NULL;
				gsize length = 0;

				if (MsgAccessFile(smil_multipart->szFilePath, F_OK)) {
					g_file_get_contents((gchar*)smil_multipart->szFilePath, (gchar**)&contents, (gsize*)&length, NULL);

					MmsInsertPresentation(pMmsMsg, MIME_APPLICATION_SMIL, NULL, contents, length);

					g_free(contents);

				} else {
					contents = smil_multipart->pMultipartData;
					length = smil_multipart->nMultipartDataLen;

					MmsInsertPresentation(pMmsMsg, MIME_APPLICATION_SMIL, NULL, contents, length);
				}
			}
		} else {
			pMmsMsg->mmsAttrib.contentType = MIME_APPLICATION_VND_WAP_MULTIPART_MIXED;
			pMmsMsg->msgType.type = MIME_APPLICATION_VND_WAP_MULTIPART_MIXED;
		}

		int len = g_list_length(pMmsData->multipartlist);

		for (int i = 0; i < len; i++) {
			MMS_MULTIPART_DATA_S *multipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(pMmsData->multipartlist, i);

			if (multipart) {
				if (multipart->type == MIME_UNKNOWN)
					multipart->type = MimeGetMimeIntFromMimeString(multipart->szContentType);

				if (MmsInsertPartFromMultipart(pMmsMsg, multipart) == false) {
					return false;
				}
			}
		} /* end for */
	}

	MSG_END();
	return true;
}

char *MmsConvertAddressToOldStyle(MMSList *pAddressList)
{
	MSG_BEGIN();
	int addrLen = 0;
	int nAddressCnt = 0;
	char pString[MSG_LOCALE_NAME_LEN + MSG_ADDR_LEN + 3] = {0, };
	char *szCompose = NULL;

	nAddressCnt = g_list_length(pAddressList);

	/* Calculate allocated buffer size */
	for (int i = 0; i < nAddressCnt; ++i) {
		MMS_ADDRESS_DATA_S * pAddressData = (MMS_ADDRESS_DATA_S *)g_list_nth_data(pAddressList, i);
		if (pAddressData) {
			MSG_SEC_DEBUG("address type : %d, address value: %s", pAddressData->address_type, pAddressData->address_val);
			if (pAddressData->address_type == MSG_ADDRESS_TYPE_PLMN) {
				addrLen += strlen("/TYPE=PLMN");
				addrLen += strlen(pAddressData->address_val);
			} else {
				addrLen += strlen(pAddressData->address_val);
			}
		}
	}

	if (nAddressCnt > 1)
		addrLen = addrLen + nAddressCnt - 1;

	szCompose = (char *)calloc(addrLen + 1, 1);

	if (szCompose) {
		/* Address String copy */
		for (int i = 0; i < nAddressCnt; ++i) {
			MMS_ADDRESS_DATA_S * pAddressData = (MMS_ADDRESS_DATA_S *)g_list_nth_data(pAddressList, i);

			if (pAddressData) {
				if (strlen(szCompose) > 0)
					g_strlcat(szCompose, MSG_STR_ADDR_DELIMETER, addrLen + 1);

				memset(pString, 0x00, (MSG_LOCALE_NAME_LEN + MSG_ADDR_LEN + 3) * sizeof(char));
				if (pAddressData->address_type == MSG_ADDRESS_TYPE_PLMN) {
					snprintf(pString, MSG_LOCALE_NAME_LEN + MSG_ADDR_LEN + 3, "%s%s", pAddressData->address_val, "/TYPE=PLMN");
					MSG_DEBUG("%s", pString);
				} else {
					snprintf(pString, MSG_LOCALE_NAME_LEN + MSG_ADDR_LEN + 3, "%s", pAddressData->address_val);
				}

				g_strlcat(szCompose, pString, addrLen + 1);
			}
		}
	}

	MSG_END();
	return szCompose;
}

MMSList * MmsConvertAddressToNewStyle(const char *szAddr)
{
	MMSList *pAddressList = NULL;
	MMS_ADDRESS_DATA_S *pAddressData = NULL;

	char *pTempChar = NULL;
	char *pStartPtr = NULL;
	char *pEndPtr = NULL;

	if (szAddr == NULL)
		return NULL;

	pTempChar = strdup(szAddr);

	pStartPtr = pTempChar;

	while (pStartPtr && pStartPtr[0]) {
		int addLen = 0;
		char tempAddress[512] = {0, };
		char tempAddress2[512] = {0, };

		pEndPtr = strchr(pStartPtr, MSG_CH_SEMICOLON);

		if (pEndPtr) {
			addLen = pEndPtr - pStartPtr;
		} else {
			addLen = strlen(pStartPtr);
		}

		if (addLen) {
			strncpy(tempAddress, pStartPtr, addLen);
			int addr_type;
			int addLen2;
			char *pSlash = strchr(tempAddress, '/');

			if (pSlash) {
				addLen2 = pSlash - tempAddress;
				addr_type = MSG_ADDRESS_TYPE_PLMN;
			} else {
				addLen2 = strlen(tempAddress);
				addr_type = MSG_ADDRESS_TYPE_EMAIL;
			}

			if (addLen2) {
				strncpy(tempAddress2, tempAddress, addLen2);
				pAddressData = MsgMmsCreateAddress(addr_type, tempAddress2);
			}

			if (pAddressData)
				pAddressList = g_list_append(pAddressList, pAddressData);

			pStartPtr = pStartPtr + addLen + 1;
		} else {
			break;
		}
	}

	if (pTempChar)
		free(pTempChar);

	MSG_END();
	return pAddressList;
}

bool convertMediaToMultipart(MMS_MEDIA_S *pSrcMedia, MMS_MULTIPART_DATA_S *pDestMultipart)
{
	bzero(pDestMultipart, sizeof(MMS_MULTIPART_DATA_S));

	snprintf(pDestMultipart->szContentID, sizeof(pDestMultipart->szContentID), "%s", pSrcMedia->szContentID);
	snprintf(pDestMultipart->szContentLocation, sizeof(pDestMultipart->szContentLocation), "%s", pSrcMedia->szContentLocation);
	snprintf(pDestMultipart->szFileName, sizeof(pDestMultipart->szFileName), "%s", pSrcMedia->szFileName);
	snprintf(pDestMultipart->szFilePath, sizeof(pDestMultipart->szFilePath), "%s", pSrcMedia->szFilePath);


	if (strlen(pSrcMedia->szContentType) > 0) {
		snprintf(pDestMultipart->szContentType, sizeof(pDestMultipart->szContentType), "%s", pSrcMedia->szContentType);
		pDestMultipart->type = MimeGetMimeIntFromMimeString(pSrcMedia->szContentType);

	} else {
		MimeMainType mainType = MIME_MAINTYPE_UNKNOWN;
		MimeType pMimeType = MIME_UNKNOWN;
		const char *pszMimeType = NULL;

		if (pSrcMedia->mediatype == MMS_SMIL_MEDIA_IMG)
			mainType = MIME_MAINTYPE_IMAGE;
		else if (pSrcMedia->mediatype == MMS_SMIL_MEDIA_AUDIO)
			mainType = MIME_MAINTYPE_AUDIO;
		else if (pSrcMedia->mediatype ==  MMS_SMIL_MEDIA_VIDEO)
			mainType = MIME_MAINTYPE_VIDEO;
		else if (pSrcMedia->mediatype == MMS_SMIL_MEDIA_TEXT)
			mainType = MIME_MAINTYPE_TEXT;

		MsgGetMimeTypeFromFileName(mainType, pSrcMedia->szFilePath, &pMimeType, &pszMimeType);

		if (pszMimeType) {
			snprintf(pDestMultipart->szContentType, sizeof(pDestMultipart->szContentType), "%s", pszMimeType);
			pDestMultipart->type = pMimeType;
		} else {
			snprintf(pDestMultipart->szContentType, sizeof(pDestMultipart->szContentType), "%s", "application/octet-stream");
			pDestMultipart->type = MIME_APPLICATION_OCTET_STREAM;
		}
	}

	return true;
}

bool convertAttachToMultipart(MMS_ATTACH_S *pSrcMedia, MMS_MULTIPART_DATA_S *pDestMultipart)
{
	bzero(pDestMultipart, sizeof(MMS_MULTIPART_DATA_S));

	snprintf(pDestMultipart->szFileName, sizeof(pDestMultipart->szFileName), "%s", pSrcMedia->szFileName);
	snprintf(pDestMultipart->szFilePath, sizeof(pDestMultipart->szFilePath), "%s", pSrcMedia->szFilePath);

	if (strlen(pSrcMedia->szContentType) > 0) {
		snprintf(pDestMultipart->szContentType, sizeof(pDestMultipart->szContentType), "%s", pSrcMedia->szContentType);
		pDestMultipart->type = MimeGetMimeIntFromMimeString(pSrcMedia->szContentType);
	} else {
		MimeMainType mainType = MIME_MAINTYPE_UNKNOWN;
		MimeType pMimeType = MIME_UNKNOWN;
		const char *pszMimeType = NULL;

		MsgGetMimeTypeFromFileName(mainType, pSrcMedia->szFilePath, &pMimeType, &pszMimeType);

		if (pszMimeType) {
			snprintf(pDestMultipart->szContentType, sizeof(pDestMultipart->szContentType), "%s", pszMimeType);
			pDestMultipart->type = pMimeType;
		} else {
			snprintf(pDestMultipart->szContentType, sizeof(pDestMultipart->szContentType), "%s", "application/octet-stream");
			pDestMultipart->type = MIME_APPLICATION_OCTET_STREAM;
		}
	}

	return true;
}

bool convertMsgMultipartToMultipart(MsgMultipart *pSrcMultipart, MMS_MULTIPART_DATA_S *pDestMultipart)
{
	pDestMultipart->type = (MimeType)pSrcMultipart->type.type;

	MSG_DEBUG("Mime Type : %s :%d", MimeGetMimeStringFromMimeInt(pSrcMultipart->type.type), pSrcMultipart->type.type);
	snprintf(pDestMultipart->szContentID, sizeof(pDestMultipart->szContentID), "%s", pSrcMultipart->type.szContentID);
	snprintf(pDestMultipart->szContentLocation, sizeof(pDestMultipart->szContentLocation), "%s", pSrcMultipart->type.szContentLocation);
	snprintf(pDestMultipart->szFileName, sizeof(pDestMultipart->szFileName), "%s", pSrcMultipart->type.param.szName);
	snprintf(pDestMultipart->szFilePath, sizeof(pDestMultipart->szFilePath), "%s", pSrcMultipart->pBody->szOrgFilePath);

	snprintf(pDestMultipart->szContentType, sizeof(pDestMultipart->szContentType), "%s", MimeGetMimeStringFromMimeInt(pSrcMultipart->type.type));

	if (pSrcMultipart->type.drmInfo.drmType != MSG_DRM_TYPE_NONE) {
		pDestMultipart->drmType = pSrcMultipart->type.drmInfo.drmType;
	}

	return true;
}

gint __compare_str(gconstpointer a, gconstpointer b)
{
	return g_strcmp0((char *)a, (char *)b);
}

/* change file name to Ascii & space -> '_' */
/* If replaced filename is duplicated, then it append number */
bool MmsChangeFileNameToAscii(MMS_MESSAGE_DATA_S *pMsgData)
{
	int pageCnt;
	int mediaCnt;
	int attachCnt;

	GList *r_list = NULL; /* renamed file list */

	pageCnt = g_list_length(pMsgData->pagelist);

	for (int i = 0; i < pageCnt; i++) {
		MMS_PAGE_S *pPage = NULL;
		pPage = (MMS_PAGE_S *)g_list_nth_data(pMsgData->pagelist, i);
		if (pPage == NULL) {
			/* CID 355351: Freeing r_list in case of error too to prevent memory leak */
			if (r_list != NULL)
				g_list_free(r_list);
			return false;
		}

		mediaCnt = g_list_length(pPage->medialist);
		for (int j = 0; j < mediaCnt; j++) {
			MMS_MEDIA_S *pMedia = NULL;
			pMedia = (MMS_MEDIA_S *)g_list_nth_data(pPage->medialist, j);
			if (pMedia == NULL) {
				/* CID 355351: Freeing r_list in case of error too to prevent memory leak */
				if (r_list != NULL)
					g_list_free(r_list);
				return false;
			}

			if (strlen(pMedia->szFileName) > 0) {
				char *str = NULL;

				MmsReplaceSpaceChar(pMedia->szFileName);

				str = MmsReplaceNonAsciiUtf8(pMedia->szFileName, '_');

				if (str) {
					int count = 1;
					char *str2 = g_strdup(str);
					char *ext = strrchr(str, '.');

					if (ext == NULL || *(ext + 1) == '\0') {
						ext = NULL;
					} else {
						*ext = '\0';
						ext = ext + 1;
					}

					while (g_list_find_custom(r_list, str2, __compare_str) != NULL) {
						g_free(str2);

						if (ext)
							str2 = g_strdup_printf("%s_%d.%s", str, count++, ext);
						else
							str2 = g_strdup_printf("%s_%d", str, count++);
					}

					g_free(str);

					snprintf(pMedia->szFileName, sizeof(pMedia->szFileName), "%s", str2);

					MSG_SEC_DEBUG("replace filename [%s]", pMedia->szFileName);

					r_list = g_list_append(r_list, pMedia->szFileName);

					g_free(str2);
				}
			}
		} /* end for media */
	} /* end for page */

	attachCnt = g_list_length(pMsgData->attachlist);
	for (int i = 0; i < attachCnt; i++) {
		MMS_ATTACH_S *pAttach = NULL;
		pAttach = (MMS_ATTACH_S *)g_list_nth_data(pMsgData->attachlist, i);
		if (pAttach == NULL) {
			/* CID 355351: Freeing r_list in case of error too to prevent memory leak */
			if (r_list != NULL)
				g_list_free(r_list);
			return false;
		}

		if (strlen(pAttach->szFileName) > 0) {
			char *str = NULL;

			MmsReplaceSpaceChar(pAttach->szFileName);

			str = MmsReplaceNonAsciiUtf8(pAttach->szFileName, '_');
			if (str) {
				int count = 1;
				char *str2 = g_strdup(str);
				char *ext = strrchr(str, '.');

				if (ext == NULL || *(ext + 1) == '\0') {
					ext = NULL;
				} else {
					*ext = '\0';
					ext = ext + 1;
				}

				while (g_list_find_custom(r_list, str2, __compare_str) != NULL) {
					g_free(str2);

					if (ext)
						str2 = g_strdup_printf("%s_%d.%s", str, count++, ext);
					else
						str2 = g_strdup_printf("%s_%d", str, count++);
				}

				g_free(str);

				snprintf(pAttach->szFileName, sizeof(pAttach->szFileName), "%s", str2);

				MSG_SEC_DEBUG("replace filename [%s]", pAttach->szFileName);

				g_free(str2);
			}
		}
	} /* end for attach */

	g_list_free(r_list);

	return true;
}

