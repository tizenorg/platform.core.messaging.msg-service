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
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/smack.h>

#include "MsgTypes.h"
#include "MsgMmsTypes.h"
#include "MsgUtilFile.h"
#include "MsgMmsMessage.h"
#include "MsgUtilFile.h"
#include "MsgUtilFunction.h"
#include "MsgSmil.h"
#include "MsgDebug.h"
#include "MsgSerialize.h"

static void __release_glist_element(gpointer data, gpointer user_data);
static void __release_page_element(gpointer data, gpointer user_data);
static msg_error_t __releasePageList(MMS_MESSAGE_DATA_S *pMsgData);
static msg_error_t __releaseRegionList(MMS_MESSAGE_DATA_S *pMsgData);
static msg_error_t __releaseAttachList(MMS_MESSAGE_DATA_S *pMsgData);
static msg_error_t __releaseTransitionList(MMS_MESSAGE_DATA_S *pMsgData);
static msg_error_t __releaseMetaList(MMS_MESSAGE_DATA_S *pMsgData);

void __release_glist_element(gpointer data, gpointer user_data)
{
	if(data != NULL) {
		free(data);
	}
}

void __release_page_element(gpointer data, gpointer user_data)
{
	if(data != NULL) {
		MMS_PAGE_S *page = (MMS_PAGE_S *)data;

		if (page->medialist) {
			MMS_MEDIA_S *media = NULL;
			int mediaCnt = g_list_length(page->medialist);

			for (int i = 0; i < mediaCnt; i++) {
				media = (MMS_MEDIA_S *)g_list_nth_data(page->medialist, i);
				if (media)
					free(media);
			}

			g_list_free(page->medialist);
			page->medialist = NULL;
			page->mediaCnt = 0;
		}

		free(page);
	}
}

msg_error_t __releasePageList(MMS_MESSAGE_DATA_S *pMsgData)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (pMsgData->pagelist) {
		g_list_foreach(pMsgData->pagelist, __release_page_element, NULL);
		g_list_free(pMsgData->pagelist);
		pMsgData->pagelist = NULL;
	}

	pMsgData->pageCnt = 0;

	return MSG_SUCCESS;
}

msg_error_t __releaseRegionList(MMS_MESSAGE_DATA_S *pMsgData)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (pMsgData->regionlist) {
		g_list_foreach(pMsgData->regionlist, __release_glist_element, NULL);
		g_list_free(pMsgData->regionlist);
		pMsgData->regionlist = NULL;
	}

	pMsgData->regionCnt = 0;

	return MSG_SUCCESS;
}

msg_error_t __releaseAttachList(MMS_MESSAGE_DATA_S *pMsgData)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (pMsgData->attachlist) {
		g_list_foreach(pMsgData->attachlist, __release_glist_element, NULL);
		g_list_free(pMsgData->attachlist);
		pMsgData->attachlist = NULL;
	}

	pMsgData->attachCnt = 0;

	return MSG_SUCCESS;
}

msg_error_t __releaseTransitionList(MMS_MESSAGE_DATA_S *pMsgData)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (pMsgData->transitionlist) {
		g_list_foreach(pMsgData->transitionlist, __release_glist_element, NULL);
		g_list_free(pMsgData->transitionlist);
		pMsgData->transitionlist = NULL;
	}

	pMsgData->transitionCnt = 0;

	return MSG_SUCCESS;
}

msg_error_t __releaseMetaList(MMS_MESSAGE_DATA_S *pMsgData)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (pMsgData->metalist) {
		g_list_foreach(pMsgData->metalist, __release_glist_element, NULL);
		g_list_free(pMsgData->metalist);
		pMsgData->metalist = NULL;
	}

	pMsgData->metaCnt = 0;

	return MSG_SUCCESS;
}

void MsgMmsReleaseMmsLists(MMS_MESSAGE_DATA_S *pMsgData)
{
	__releasePageList(pMsgData);
	__releaseRegionList(pMsgData);
	__releaseAttachList(pMsgData);
	__releaseTransitionList(pMsgData);
	__releaseMetaList(pMsgData);
}

msg_error_t _MsgMmsAddRegion(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_REGION* pRegion)
{
	if(pMsgData == NULL || pRegion == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	pMsgData->regionlist = g_list_append(pMsgData->regionlist, pRegion);
	pMsgData->regionCnt++;

	return MSG_SUCCESS;
}

msg_error_t _MsgMmsAddPage(MMS_MESSAGE_DATA_S *pMsgData, MMS_PAGE_S *pPage)
{
	if(pMsgData == NULL || pPage == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	pMsgData->pagelist = g_list_append(pMsgData->pagelist, pPage);
	pMsgData->pageCnt++;
	MSG_DEBUG("MmsData's Page Count : %d", pMsgData->pageCnt);
	return MSG_SUCCESS;
}

msg_error_t _MsgMmsAddMedia(MMS_PAGE_S* pPage, MMS_MEDIA_S *pMedia)
{
	if(pPage == NULL || pMedia == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	pPage->medialist = g_list_append(pPage->medialist, pMedia);
	pPage->mediaCnt++;
	MSG_DEBUG("Page's media count: %d", pPage->mediaCnt);
	return MSG_SUCCESS;
}

msg_error_t _MsgMmsAddTransition(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_TRANSITION* pTransition)
{
	if(pMsgData == NULL || pTransition == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	pMsgData->transitionlist = g_list_append(pMsgData->transitionlist, pTransition);
	pMsgData->transitionCnt++;

	return MSG_SUCCESS;
}

msg_error_t _MsgMmsAddMeta(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_META* pMeta)
{
	if(pMsgData == NULL || pMeta == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	pMsgData->metalist = g_list_append(pMsgData->metalist, pMeta);
	pMsgData->metaCnt++;

	return MSG_SUCCESS;
}

msg_error_t _MsgMmsAddAttachment(MMS_MESSAGE_DATA_S *pMsgData, MMS_ATTACH_S *pAttach)
{
	if(pMsgData == NULL || pAttach == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	pMsgData->attachlist = g_list_append(pMsgData->attachlist, pAttach);
	pMsgData->attachCnt++;

	return MSG_SUCCESS;
}

int _MsgMmsGetPageCount(MMS_MESSAGE_DATA_S *pMsgData)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return 0;
	}

	int count = 0;

	if (pMsgData->pagelist)
		count = g_list_length(pMsgData->pagelist);

	MSG_DEBUG("Page Count: %d", count);
	return count;
}

MMS_PAGE_S *_MsgMmsGetPage(MMS_MESSAGE_DATA_S *pMsgData, int pageIdx)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return NULL;
	}

	MMS_PAGE_S *page = NULL;

	if (pMsgData->pagelist)
		page = (MMS_PAGE_S *)g_list_nth_data(pMsgData->pagelist, pageIdx);

	return page;
}

int _MsgMmsGetAttachCount(MMS_MESSAGE_DATA_S *pMsgData)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return 0;
	}

	int count = 0;

	if (pMsgData->attachlist)
		count = g_list_length(pMsgData->attachlist);

	MSG_DEBUG("Attachment Count: %d", count);
	return count;
}

MMS_ATTACH_S *_MsgMmsGetAttachment(MMS_MESSAGE_DATA_S *pMsgData, int attachIdx)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return NULL;
	}

	MMS_ATTACH_S *attach = NULL;
	if (pMsgData->attachlist)
		attach = (MMS_ATTACH_S *)g_list_nth_data(pMsgData->attachlist, attachIdx);

	return attach;
}

MMS_SMIL_REGION *_MsgMmsGetSmilRegion(MMS_MESSAGE_DATA_S *pMsgData, int regionIdx)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return NULL;
	}

	MMS_SMIL_REGION *region = NULL;

	if (pMsgData->regionlist)
		region = (MMS_SMIL_REGION *)g_list_nth_data(pMsgData->regionlist, regionIdx);

	return region;
}

MMS_MEDIA_S *_MsgMmsGetMedia(MMS_PAGE_S *pPage, int mediaIdx)
{
	if (!pPage) {
		MSG_FATAL("pPage is NULL");
		return NULL;
	}

	if (mediaIdx > pPage->mediaCnt || mediaIdx < 0) {
		MSG_FATAL("Invalid media index = %d", mediaIdx);
		return NULL;
	}

	MMS_MEDIA_S *media = NULL;
	if (pPage->medialist)
		media = (MMS_MEDIA_S *)g_list_nth_data(pPage->medialist, mediaIdx);

	return media;
}

int _MsgMmsGetTransitionCount(MMS_MESSAGE_DATA_S *pMsgData)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return 0;
	}

	int count = 0;

	if (pMsgData->transitionlist)
		count = g_list_length(pMsgData->transitionlist);

	MSG_DEBUG("Transition Count: %d", count);
	return count;
}

MMS_SMIL_TRANSITION *_MsgMmsGetTransition(MMS_MESSAGE_DATA_S *pMsgData, int transitionIdx)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return NULL;
	}

	MMS_SMIL_TRANSITION *transition = NULL;
	if (pMsgData->transitionlist)
		transition = (MMS_SMIL_TRANSITION *)g_list_nth_data(pMsgData->transitionlist, transitionIdx);

	return transition;
}

MMS_SMIL_META *_MsgMmsGetMeta(MMS_MESSAGE_DATA_S *pMsgData, int metaIdx)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return NULL;
	}

	MMS_SMIL_META *meta = NULL;

	if (pMsgData->metalist)
		meta = (MMS_SMIL_META *)g_list_nth_data(pMsgData->metalist, metaIdx);

	return meta;
}

int	_MsgMmsGetMetaCount(MMS_MESSAGE_DATA_S *pMsgData)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("pMsgData is NULL");
		return 0;
	}

	int count = 0;

	if (pMsgData->metalist)
		count = g_list_length(pMsgData->metalist);

	MSG_DEBUG("Meta Count: %d", count);
	return count;
}

bool _MsgMmsSetRootLayout(MMS_MESSAGE_DATA_S *pMsgData, MMS_SMIL_ROOTLAYOUT *pRootlayout)
{
	memcpy(&pMsgData->rootlayout, pRootlayout, sizeof(MMS_SMIL_ROOTLAYOUT));
	return true;
}

char* _MsgMmsSerializeMessageData(const MMS_MESSAGE_DATA_S *pMsgData, unsigned int *pSize)
{
	MSG_BEGIN();

	if (pMsgData == NULL)
		return NULL;

	int bufsize = 0;
	int offset = 0;
	int pageCnt = 0;
	char *buf = NULL;

	bufsize += sizeof(int);	/* Page cnt */

	pageCnt = pMsgData->pageCnt;

	if (pMsgData->pagelist) {
		for (int pageIdx = 0; pageIdx < pageCnt; pageIdx++) {
			int mediaCnt = 0;

			MMS_PAGE_S *page = (MMS_PAGE_S *)g_list_nth_data(pMsgData->pagelist, pageIdx);

			mediaCnt = page->mediaCnt;

			bufsize += sizeof(int);	/* Media cnt */

			if (page->medialist) {
				bufsize += sizeof(MMS_MEDIA_S) * mediaCnt;
			}

			bufsize += sizeof(int) * 6; /* Dur, Begin, End, Min, Max, Repeat */
		}
	}

	bufsize += sizeof(int); /* region count; */

	if (pMsgData->regionlist) {
		int elementSize = g_list_length(pMsgData->regionlist);
		bufsize += elementSize * (sizeof(MMS_SMIL_REGION));
	}

	bufsize += sizeof(int); /* attachment count; */
	if (pMsgData->attachlist) {
		int elementSize = g_list_length(pMsgData->attachlist);
		bufsize += elementSize * sizeof(MMS_ATTACH_S);
	}

	bufsize += sizeof(int); /* transition count; */
	if (pMsgData->transitionlist) {
		int elementSize = g_list_length(pMsgData->transitionlist);
		bufsize += elementSize * sizeof(MMS_SMIL_TRANSITION);
	}

	bufsize += sizeof(int); /* meta count; */
	if (pMsgData->metalist) {
		int elementSize = g_list_length(pMsgData->metalist);
		bufsize += elementSize * sizeof(MMS_SMIL_META);
	}

	bufsize += sizeof(MMS_SMIL_ROOTLAYOUT);

#ifdef FEATURE_JAVA_MMS
	bufsize += sizeof(MMS_APPID_INFO_S);
#endif

	int filePathLen = strlen(pMsgData->szSmilFilePath);

	bufsize += sizeof(int) + filePathLen;

	bufsize += sizeof(int);	/* type */
	bufsize += sizeof(MMS_HEADER_DATA_S);
	bufsize += sizeof(MMS_MULTIPART_DATA_S);

	MSG_DEBUG("Serialize bufsize = %d", bufsize);

	buf = (char *)calloc(bufsize, 1);

	if (buf == NULL)
		return buf;

	int serial_index = 0;

	memcpy(buf, &pMsgData->backup_type, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] backup type = %d", serial_index++, offset, pMsgData->backup_type);
	offset += sizeof(int);

	/* smilFilePath */
	memcpy(buf + offset , &filePathLen, sizeof(int));

	/* copy file path */
	MSG_SEC_DEBUG("[#%2d][%5d] smilFilePath = %s, len = %d", serial_index++, offset, pMsgData->szSmilFilePath, filePathLen);
	offset += sizeof(int);

	if (filePathLen > 0) {
		memcpy(buf + offset, pMsgData->szSmilFilePath, filePathLen);
		offset += filePathLen;
	}

	/* copy page count */
	MSG_DEBUG("[#%2d][%5d] page count = %d", serial_index++, offset, pMsgData->pageCnt);
	memcpy(buf + offset, &(pMsgData->pageCnt), sizeof(int));
	offset += sizeof(int);

	if (pMsgData->pagelist) {
		for (int pageIdx = 0; pageIdx < pageCnt; pageIdx++) {
			MMS_PAGE_S *page = (MMS_PAGE_S *)g_list_nth_data(pMsgData->pagelist, pageIdx);

			MSG_DEBUG("[#%2d][%5d][%d page] media count = %d", serial_index++, offset, pageIdx, page->mediaCnt);
			memcpy(buf + offset, &page->mediaCnt, sizeof(int));
			offset += sizeof(int);

			if (page->medialist) {
				for (int i = 0; i < page->mediaCnt; ++i) {
					MMS_MEDIA_S *media = (MMS_MEDIA_S *)g_list_nth_data(page->medialist, i);
					memcpy(buf + offset, media, sizeof(MMS_MEDIA_S));
					offset += sizeof(MMS_MEDIA_S);
					MSG_SEC_DEBUG("[#%2d][%5d][%d page][%d media] media type [%d],  drm type [%d], filepath [%s]", serial_index++, offset, pageIdx, i, media->mediatype, media->drmType, media->szFilePath);
				}
			}

			memcpy(buf + offset, &page->nDur, sizeof(int));
			offset += sizeof(int);
			memcpy(buf + offset, &page->nBegin, sizeof(int));
			offset += sizeof(int);
			memcpy(buf + offset, &page->nEnd, sizeof(int));
			offset += sizeof(int);
			memcpy(buf + offset, &page->nMin, sizeof(int));
			offset += sizeof(int);
			memcpy(buf + offset, &page->nMax, sizeof(int));
			offset += sizeof(int);
			memcpy(buf + offset, &page->nRepeat, sizeof(int));
			offset += sizeof(int);
		}
	}

	MSG_DEBUG("[#%2d][%5d] region count = %d", serial_index++, offset, pMsgData->regionCnt);

	memcpy(buf + offset, &pMsgData->regionCnt, sizeof(int));
	offset += sizeof(int);

	if (pMsgData->regionlist) {
		for (int i = 0; i < pMsgData->regionCnt; ++i) {
			MMS_SMIL_REGION *region = (MMS_SMIL_REGION *)g_list_nth_data(pMsgData->regionlist, i);
			memcpy(buf + offset, region, sizeof(MMS_SMIL_REGION));
			offset += sizeof(MMS_SMIL_REGION);
		}
	}

	MSG_DEBUG("[#%2d][%5d] attach count = %d", serial_index++, offset, pMsgData->attachCnt);

	memcpy(buf + offset, &pMsgData->attachCnt, sizeof(int));
	offset += sizeof(int);

	if (pMsgData->attachlist) {
		for (int i = 0; i < pMsgData->attachCnt; ++i) {
			MMS_ATTACH_S *attach = (MMS_ATTACH_S *)g_list_nth_data(pMsgData->attachlist, i);
			memcpy(buf + offset, attach, sizeof(MMS_ATTACH_S));
			offset += sizeof(MMS_ATTACH_S);
			MSG_SEC_DEBUG("[#%2d][%5d][%d attach] attach filepath = %s, drm type = [%d]", serial_index++, offset, i, attach->szFilePath, attach->drmType);
		}
	}

	MSG_DEBUG("[#%2d][%5d] transition count = %d", serial_index++, offset, pMsgData->transitionCnt);

	memcpy(buf + offset, &pMsgData->transitionCnt, sizeof(int));
	offset += sizeof(int);
	if (pMsgData->transitionlist) {
		for (int i = 0; i < pMsgData->transitionCnt; ++i) {
			MMS_SMIL_TRANSITION *transition = (MMS_SMIL_TRANSITION *)g_list_nth_data(pMsgData->transitionlist, i);
			memcpy(buf + offset, transition, sizeof(MMS_SMIL_TRANSITION));
			offset += sizeof(MMS_SMIL_TRANSITION);
		}
	}

	MSG_DEBUG("[#%2d][%5d] meta count = %d", serial_index++, offset, pMsgData->metaCnt);
	memcpy(buf + offset, &pMsgData->metaCnt, sizeof(int));
	offset += sizeof(int);

	if (pMsgData->metalist) {
		for (int i = 0; i < pMsgData->metaCnt; ++i) {
			MMS_SMIL_META *meta = (MMS_SMIL_META *)g_list_nth_data(pMsgData->metalist, i);

			memcpy(buf + offset, meta, sizeof(MMS_SMIL_META));
			offset += sizeof(MMS_SMIL_META);
		}
	}

	MSG_DEBUG("[#%2d][%5d] root layout", serial_index++, offset);
	memcpy(buf + offset, &pMsgData->rootlayout, sizeof(MMS_SMIL_ROOTLAYOUT));
	offset += sizeof(MMS_SMIL_ROOTLAYOUT);

#ifdef FEATURE_JAVA_MMS
	MSG_DEBUG("[#%2d][%5d] java mms", serial_index++, offset);
	memcpy(buf + offset, &pMsgData->msgAppId, sizeof(MMS_APPID_INFO_S));
	offset += sizeof(MMS_APPID_INFO_S);
#endif

	memcpy (buf + offset, &pMsgData->header, sizeof(MMS_HEADER_DATA_S));
	MSG_DEBUG("[#%2d][%5d] mms header", serial_index++, offset);
	offset += sizeof(MMS_HEADER_DATA_S);

	memcpy (buf + offset, &pMsgData->smil, sizeof(MMS_MULTIPART_DATA_S));
	MSG_DEBUG("[#%2d][%5d] mms smil", serial_index++, offset);
	offset += sizeof(MMS_MULTIPART_DATA_S);

	*pSize = offset;

	MSG_DEBUG("Expect Buffer Size: %d, Final offset : %d", bufsize, offset);
	MSG_END();
	return buf;
}

bool _MsgMmsDeserializeMessageData(MMS_MESSAGE_DATA_S *pMsgData, const char *pData)
{
	MSG_BEGIN();

	if (pMsgData == NULL || pData == NULL) {
		MSG_DEBUG("param is NULL. pBody = %x, pData = %x", pMsgData, pData);
		return false;
	}

	int offset = 0;
	int pageCnt = 0;
	int filePathLen = 0;

	MMS_PAGE_S *pPage = NULL;
	MMS_MEDIA_S *pMedia = NULL;
	MMS_SMIL_REGION *pRegion = NULL;
	MMS_ATTACH_S *pAttach = NULL;
	MMS_SMIL_TRANSITION *pTransition = NULL;
	MMS_SMIL_META *pMeta = NULL;

	int serial_index = 0;

	int type;

	memcpy(&type, pData, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] backup type = %d", serial_index++, offset, type);
	offset += sizeof(int);

	pMsgData->backup_type = type;

	memcpy(&filePathLen, pData + offset, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] smil path len = %d", serial_index, offset, filePathLen);
	offset += sizeof(int);

	if (filePathLen > MSG_FILEPATH_LEN_MAX) {
		MSG_DEBUG("Smil File Path Length is abnormal.");
		return false;
	}

	memset(pMsgData->szSmilFilePath, 0x00, MSG_FILEPATH_LEN_MAX);

	if (filePathLen > 0) {
		memcpy(pMsgData->szSmilFilePath, pData + offset, filePathLen);
		MSG_DEBUG("[#%2d][%5d] smil path = %s", serial_index, offset, pMsgData->szSmilFilePath);
		offset += filePathLen;
	}

	serial_index++;

	memcpy(&(pMsgData->pageCnt), pData + offset, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] page count = %d", serial_index++, offset, pMsgData->pageCnt);
	offset += sizeof(int);

	pageCnt = pMsgData->pageCnt;

	for (int j = 0; j < pageCnt; ++j) {
		pPage = (MMS_PAGE_S *)calloc(sizeof(MMS_PAGE_S), 1);
		if (pPage == NULL)
			return false;

		memcpy(&pPage->mediaCnt, pData + offset, sizeof(int));
		MSG_DEBUG("[#%2d][%5d][%d page] media count = %d", serial_index++, offset, j, pPage->mediaCnt);
		offset += sizeof(int);

		for (int i = 0; i < pPage->mediaCnt; ++i) {
			pMedia = (MMS_MEDIA_S *)calloc(sizeof(MMS_MEDIA_S), 1);
			if (pMedia == NULL) {
				free(pPage);
				return false;
			}

			memcpy(pMedia, pData + offset, sizeof(MMS_MEDIA_S));

			offset += sizeof(MMS_MEDIA_S);

			MSG_SEC_DEBUG("[#%2d][%5d][%d page][%d media] media type [%d], drm type [%d], filepath [%s], content type [%s]", serial_index++, offset, j, i, pMedia->mediatype, pMedia->drmType, pMedia->szFilePath, pMedia->szContentType);

			if (strlen(pMedia->szFilePath) >0) {
				pPage->medialist = g_list_append(pPage->medialist, pMedia);
			} else {
				free(pMedia);
			}
		}

		pPage->mediaCnt = g_list_length(pPage->medialist);

		memcpy(&pPage->nDur , pData + offset, sizeof(int));
		offset += sizeof(int);
		memcpy(&pPage->nBegin , pData + offset, sizeof(int));
		offset += sizeof(int);
		memcpy(&pPage->nEnd , pData + offset, sizeof(int));
		offset += sizeof(int);
		memcpy(&pPage->nMin , pData + offset, sizeof(int));
		offset += sizeof(int);
		memcpy(&pPage->nMax , pData + offset, sizeof(int));
		offset += sizeof(int);
		memcpy(&pPage->nRepeat , pData + offset, sizeof(int));
		offset += sizeof(int);

		pMsgData->pagelist = g_list_append(pMsgData->pagelist, pPage);
#if 0
		if (pPage->medialist) {
			pMsgData->pagelist = g_list_append(pMsgData->pagelist, pPage);
		} else {
			free(pPage);
			pPage = NULL;
		}
#endif
	}

	pMsgData->pageCnt = g_list_length(pMsgData->pagelist);

	/* Processing Region List */
	memcpy(&pMsgData->regionCnt, pData + offset, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] region count = %d", serial_index++, offset, pMsgData->regionCnt);
	offset += sizeof(int);

	/* MSG_DEBUG(" pBody->regionCnt: %d", pBody->regionCnt); */

	for (int i = 0; i < pMsgData->regionCnt; ++i) {
		pRegion = (MMS_SMIL_REGION *)calloc(sizeof(MMS_SMIL_REGION), 1);
		if (pRegion == NULL)
			return false;

		memcpy(pRegion, pData + offset, sizeof(MMS_SMIL_REGION));
		offset += sizeof(MMS_SMIL_REGION);

		pMsgData->regionlist = g_list_append(pMsgData->regionlist, pRegion);
	}

	/* Processing Attachment List */
	memcpy(&pMsgData->attachCnt, pData + offset, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] attach count = %d", serial_index++, offset, pMsgData->attachCnt);
	offset += sizeof(int);

	for (int i = 0; i < pMsgData->attachCnt; ++i) {
		pAttach = (MMS_ATTACH_S *)calloc(sizeof(MMS_ATTACH_S), 1);
		if (pAttach == NULL)
			return false;

		memcpy(pAttach, pData + offset, sizeof(MMS_ATTACH_S));
		offset += sizeof(MMS_ATTACH_S);

		MSG_SEC_DEBUG("[#%2d][%5d][%d attach] drm type [%d],  attach filepath = [%s], content type = [%s]", serial_index++, offset, i, pAttach->drmType, pAttach->szFilePath, pAttach->szContentType);
		if (strlen(pAttach->szFilePath) >0) {
			pMsgData->attachlist = g_list_append(pMsgData->attachlist, pAttach);
		} else {
			free(pAttach);
			pAttach = NULL;
		}
	}

	pMsgData->attachCnt = g_list_length(pMsgData->attachlist);

	/* Processing Transition List */
	memcpy(&pMsgData->transitionCnt, pData + offset, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] transition count = %d", serial_index++, offset, pMsgData->transitionCnt);
	offset += sizeof(int);

	for (int i = 0; i < pMsgData->transitionCnt; ++i) {
		pTransition = (MMS_SMIL_TRANSITION *)calloc(sizeof(MMS_SMIL_TRANSITION), 1);
		if (pTransition == NULL)
			return false;

		memcpy(pTransition, pData + offset, sizeof(MMS_SMIL_TRANSITION));
		offset += sizeof(MMS_SMIL_TRANSITION);

		pMsgData->transitionlist = g_list_append(pMsgData->transitionlist, pTransition);
	}

	/* Processing Meta List */
	memcpy(&pMsgData->metaCnt, pData + offset, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] meta count = %d", serial_index++, offset, pMsgData->metaCnt);
	offset += sizeof(int);

	for (int i = 0; i < pMsgData->metaCnt; ++i) {
		pMeta = (MMS_SMIL_META *)calloc(sizeof(MMS_SMIL_META), 1);
		if (pMeta == NULL)
			return false;

		memcpy(pMeta, pData + offset, sizeof(MMS_SMIL_META));
		offset += sizeof(MMS_SMIL_META);

		pMsgData->metalist = g_list_append(pMsgData->metalist, pMeta);
	}

	MSG_DEBUG("[#%2d][%5d] root layout", serial_index++, offset);
	memcpy(&pMsgData->rootlayout, pData + offset, sizeof(MMS_SMIL_ROOTLAYOUT));
	offset += sizeof(MMS_SMIL_ROOTLAYOUT);

#ifdef FEATURE_JAVA_MMS
	MSG_DEBUG("[#%2d][%5d] java mms", serial_index++, offset);
	memcpy(&pMsgData->msgAppId, pData + offset, sizeof(MMS_APPID_INFO_S));
	offset += sizeof(MMS_APPID_INFO_S);
/*	MSG_DEBUG("java_app_id valid:%d, appId:%s repleToAppId:%s", pBody->msgAppId.valid, pBody->msgAppId.appId, pBody->msgAppId.replyToAppId); */
#endif

	memcpy(&pMsgData->header, pData + offset, sizeof(MMS_HEADER_DATA_S));
	MSG_DEBUG("[#%2d][%5d] mms header", serial_index++, offset);
	offset += sizeof(MMS_HEADER_DATA_S);

	memcpy(&pMsgData->smil, pData + offset, sizeof(MMS_MULTIPART_DATA_S));
	MSG_DEBUG("[#%2d][%5d] mms smil", serial_index++, offset);
	offset += sizeof(MMS_MULTIPART_DATA_S);

	MSG_DEBUG("Final offset : %d", offset);
	MSG_END();
	return true;
}

void _MsgMmsAttachPrint(MMS_ATTACH_S *attach)
{
	if (attach == NULL) {
		MSG_DEBUG("Invalid Parameter");
		return;
	}

	MSG_DEBUG("%-25s : %d", "Attach Mimetype", attach->mediatype);
	MSG_SEC_DEBUG("%-25s : %s", "Attach content type", attach->szContentType);
	MSG_SEC_DEBUG("%-25s : %s", "Attach filename", attach->szFileName);
	MSG_SEC_DEBUG("%-25s : %s", "Attach filepath", attach->szFilePath);
	MSG_DEBUG("%-25s : %d", "Attach filesize", attach->fileSize);
	MSG_DEBUG("%-25s : %d", "Attach drm type", attach->drmType);
	MSG_SEC_DEBUG("%-25s : %s", "Attach drm filepath", attach->szDrm2FullPath);
}

void _MsgMmsMediaPrint(MMS_MEDIA_S *media)
{
	if (media == NULL) {
		MSG_DEBUG("Invalid Parameter");
		return;
	}

	if (media->mediatype == MMS_SMIL_MEDIA_INVALID) {
		MSG_DEBUG("%-25s : %s", "Media type", "MMS_SMIL_MEDIA_INVALID");
	} else if (media->mediatype == MMS_SMIL_MEDIA_IMG) {
		MSG_DEBUG("%-25s : %s", "Media type", "MMS_SMIL_MEDIA_IMG");
	} else if (media->mediatype == MMS_SMIL_MEDIA_AUDIO) {
		MSG_DEBUG("%-25s : %s", "Media type", "MMS_SMIL_MEDIA_AUDIO");
	} else if (media->mediatype == MMS_SMIL_MEDIA_VIDEO) {
		MSG_DEBUG("%-25s : %s", "Media type", "MMS_SMIL_MEDIA_VIDEO");
	} else if (media->mediatype == MMS_SMIL_MEDIA_TEXT) {
		MSG_DEBUG("%-25s : %s", "Media type", "MMS_SMIL_MEDIA_TEXT");
	} else if (media->mediatype == MMS_SMIL_MEDIA_ANIMATE) {
		MSG_DEBUG("%-25s : %s", "Media type", "MMS_SMIL_MEDIA_ANIMATE");
	} else if (media->mediatype == MMS_SMIL_MEDIA_IMG_OR_VIDEO) {
		MSG_DEBUG("%-25s : %s", "Media type", "MMS_SMIL_MEDIA_REF");
	} else {
		MSG_DEBUG("%-25s : Unknown [%d]", "Media type", media->mediatype);
	}

	MSG_DEBUG("%-25s : %s", "Media src", media->szSrc);
	MSG_SEC_DEBUG("%-25s : %s", "Media filename", media->szFileName);
	MSG_SEC_DEBUG("%-25s : %s", "Media filepath", media->szFilePath);
	MSG_SEC_DEBUG("%-25s : %s", "Media content type", media->szContentType);
	MSG_SEC_DEBUG("%-25s : %s", "Media content id", media->szContentID);
	MSG_SEC_DEBUG("%-25s : %s", "Media content location", media->szContentLocation);
	MSG_SEC_DEBUG("%-25s : %s", "Media region id", media->regionId);
	MSG_DEBUG("%-25s : %s", "Media alt", media->szAlt);
	MSG_DEBUG("%-25s : %d", "Media drm type", media->drmType);
	MSG_SEC_DEBUG("%-25s : %s", "Media drm filepath", media->szDrm2FullPath);
}

void _MsgMmsPagePrint(MMS_PAGE_S *page)
{
	if (page == NULL) {
		MSG_DEBUG("Invalid Parameter");
		return;
	}

	MSG_DEBUG("%-25s : %d", "Page duration", page->nDur);

	if (page->medialist) {
		int list_count = g_list_length(page->medialist);
		MSG_DEBUG("Media Count is [%d]", list_count);
		for (int i = 0; i < list_count; i++) {
			MMS_MEDIA_S *media = (MMS_MEDIA_S *)g_list_nth_data(page->medialist, i);
			if (media) {
				MSG_DEBUG("[%d]th Media", i);
				_MsgMmsMediaPrint(media);
			} else {
				MSG_DEBUG("Not Exist Media Data in [%d]th", i);
			}
		}
	}
}

void _MsgMmsPrint(MMS_MESSAGE_DATA_S *pMsgData)
{
	if (pMsgData == NULL) {
		MSG_DEBUG("Invalid Parameter");
		return;
	}

	if (pMsgData->pagelist) {
		int list_count = g_list_length(pMsgData->pagelist);
		MSG_DEBUG("Page Count is [%d]", list_count);
		for (int i = 0; i < list_count; i++) {
			MMS_PAGE_S *page = (MMS_PAGE_S *)g_list_nth_data(pMsgData->pagelist, i);
			if (page) {
				MSG_DEBUG("[%d]th Page", i);
				_MsgMmsPagePrint(page);
			} else {
				MSG_DEBUG("Not Exist Page Data in [%d]th", i);
			}
		}
	}

	if (pMsgData->attachlist) {
		int list_count = g_list_length(pMsgData->attachlist);
		MSG_DEBUG("Attach Count is [%d]", list_count);
		for (int i = 0; i < list_count; i++) {
			MMS_ATTACH_S *attach = (MMS_ATTACH_S *)g_list_nth_data(pMsgData->attachlist, i);
			if (attach) {
				MSG_DEBUG("[%d]th Attach", i);
				_MsgMmsAttachPrint(attach);
			} else {
				MSG_DEBUG("Not Exist Attach Data in [%d]th", i);
			}
		}
	}
}

MMS_ADDRESS_DATA_S *MsgMmsCreateAddress(int addressType, const char *addressVal)
{
	MMS_ADDRESS_DATA_S * pMmsAddressData = (MMS_ADDRESS_DATA_S * )calloc(1, sizeof(MMS_ADDRESS_DATA_S));
	if (pMmsAddressData != NULL) {
		pMmsAddressData->address_type = addressType;
		pMmsAddressData->address_val = strdup(addressVal);
	}
	return pMmsAddressData;
}

void MsgMmsReleaseAddress(MMS_ADDRESS_DATA_S **ppMmsAddressData)
{
	if (ppMmsAddressData && *ppMmsAddressData) {
		MMS_ADDRESS_DATA_S *pMmsAddressData = *ppMmsAddressData;

		if (pMmsAddressData->address_val) {
			free(pMmsAddressData->address_val);
			pMmsAddressData->address_val = NULL;
		}

		free(pMmsAddressData);
		*ppMmsAddressData = NULL;
	}
}

void MsgMmsInitMultipart(MMS_MULTIPART_DATA_S *pMmsMultipart)
{
	if (pMmsMultipart) {
		pMmsMultipart->type = MIME_UNKNOWN;
		pMmsMultipart->szContentType[0] = '\0';
		pMmsMultipart->szFileName[0] = '\0';
		pMmsMultipart->szContentID[0] = '\0';
		pMmsMultipart->szContentLocation[0] = '\0';
		pMmsMultipart->drmType = MSG_DRM_TYPE_NONE;
		pMmsMultipart->szFilePath[0] = '\0';
		pMmsMultipart->pMultipartData = NULL;
		pMmsMultipart->nMultipartDataLen = 0;
		pMmsMultipart->tcs_bc_level = -1;
		pMmsMultipart->malware_allow = 0;
		pMmsMultipart->szThumbFilePath[0] = '\0';
	}
}

MMS_MULTIPART_DATA_S *MsgMmsCreateMultipart()
{
	MMS_MULTIPART_DATA_S *pMmsMultipart = (MMS_MULTIPART_DATA_S *)calloc(1, sizeof(MMS_MULTIPART_DATA_S));
	if (pMmsMultipart) {
		pMmsMultipart->type = MIME_UNKNOWN;
		pMmsMultipart->szContentType[0] = '\0';
		pMmsMultipart->szFileName[0] = '\0';
		pMmsMultipart->szContentID[0] = '\0';
		pMmsMultipart->szContentLocation[0] = '\0';
		pMmsMultipart->drmType = MSG_DRM_TYPE_NONE;
		pMmsMultipart->szFilePath[0] = '\0';
		pMmsMultipart->pMultipartData = NULL;
		pMmsMultipart->nMultipartDataLen = 0;
		pMmsMultipart->tcs_bc_level = -1;
		pMmsMultipart->malware_allow = 0;
		pMmsMultipart->szThumbFilePath[0] = '\0';
	}

	return pMmsMultipart;
}

void MsgMmsReleaseMultipart(MMS_MULTIPART_DATA_S **ppMmsMultipart)
{
	if (ppMmsMultipart && *ppMmsMultipart) {
		MMS_MULTIPART_DATA_S *pDelMmsMultipart = *ppMmsMultipart;

		if (pDelMmsMultipart->pMultipartData != NULL) {
			g_free(pDelMmsMultipart->pMultipartData);
			pDelMmsMultipart->pMultipartData = NULL;
		}
		g_free(pDelMmsMultipart);
		*ppMmsMultipart = NULL;
	}
}

void MsgMmsInitHeader(MMS_HEADER_DATA_S *pMmsHeaderData)
{
	if (pMmsHeaderData) {
		pMmsHeaderData->bcc = NULL;
		pMmsHeaderData->cc = NULL;
		pMmsHeaderData->contentLocation[0] = '\0';
		pMmsHeaderData->szContentType[0] = '\0';
		pMmsHeaderData->date = 0;
		pMmsHeaderData->bDeliveryReport = false;
		pMmsHeaderData->delivery.type = MMS_TIMETYPE_NONE;
		pMmsHeaderData->delivery.time = 0;
		pMmsHeaderData->expiry.type = MMS_TIMETYPE_NONE;
		pMmsHeaderData->expiry.time = 0;
		pMmsHeaderData->szFrom[0] = '\0';
		pMmsHeaderData->messageClass = -1;
		pMmsHeaderData->messageID[0]= '\0';
		pMmsHeaderData->messageType = -1;
		pMmsHeaderData->mmsVersion = -1;
		pMmsHeaderData->messageSize = 0;
		pMmsHeaderData->mmsPriority = -1;
		pMmsHeaderData->bReadReport = 0;
		pMmsHeaderData->bHideAddress = false;
		pMmsHeaderData->mmsStatus = MSG_DELIVERY_REPORT_NONE;
		pMmsHeaderData->szSubject[0] = '\0';
		pMmsHeaderData->to = NULL;
		pMmsHeaderData->trID[0] = '\0';
		pMmsHeaderData->contentClass = -1;
	}
}

MMS_HEADER_DATA_S *MsgMmsCreateHeader()
{
	MMS_HEADER_DATA_S *pMmsHeaderData = (MMS_HEADER_DATA_S *)calloc(1, sizeof(MMS_HEADER_DATA_S));
	if (pMmsHeaderData) {
		pMmsHeaderData->bcc = NULL; /* Bcc */
		pMmsHeaderData->cc = NULL;  /* Cc */
		pMmsHeaderData->contentLocation[0] = '\0';
		pMmsHeaderData->szContentType[0] = '\0';
		pMmsHeaderData->date = 0;
		pMmsHeaderData->bDeliveryReport = false; /* X-Mms-Delivery-Report */
		pMmsHeaderData->delivery.type = MMS_TIMETYPE_NONE;
		pMmsHeaderData->delivery.time = 0;
		pMmsHeaderData->expiry.type = MMS_TIMETYPE_NONE;
		pMmsHeaderData->expiry.time = 0;
		pMmsHeaderData->szFrom[0] = '\0'; /* From */
		pMmsHeaderData->messageClass = -1; /* Personal | Advertisement | Informational | Auto */
		pMmsHeaderData->messageID[0]= '\0';
		pMmsHeaderData->messageType = -1; /* MmsMsgType : ex) sendreq */
		pMmsHeaderData->mmsVersion = -1; /* 1.0 1.3 */
		pMmsHeaderData->messageSize = 0; /* X-Mms-Message-Size */
		pMmsHeaderData->mmsPriority = -1; /* MSG_PRIORITY_TYPE_E : Low | Normal | High */
		pMmsHeaderData->bReadReport = 0; /* X-Mms-Read-Report */
		pMmsHeaderData->bHideAddress = false; /* X-Mms-Sender-Visibility */
		pMmsHeaderData->mmsStatus = MSG_DELIVERY_REPORT_NONE; /* X-Mms-Status */
		pMmsHeaderData->szSubject[0] = '\0'; /* Subject */
		pMmsHeaderData->to = NULL;
		pMmsHeaderData->trID[0] = '\0';
		pMmsHeaderData->contentClass = -1;/* text | image-basic| image-rich | video-basic | video-rich | megapixel | content-basic | content-rich */
#if 0
		X-Mms-Report-Allowed
		X-Mms-Response-Status
		X-Mms-Response-Text
		X-Mms-Retrieve-Status
		X-Mms-Retrieve-Text
		X-Mms-Read-Status
		X-Mms-Reply-Charging
		X-Mms-Reply-Charging-Deadline
		X-Mms-Reply-Charging-ID
		X-Mms-Reply-Charging-Size
		X-Mms-Previously-Sent-By
		X-Mms-Previously-Sent-Date
		X-Mms-Store
		X-Mms-MM-State
		X-Mms-MM-Flags
		X-Mms-Store-Status
		X-Mms-Store-Status-Text
		X-Mms-Stored
		X-Mms-Attributes
		X-Mms-Totals
		X-Mms-Mbox-Totals
		X-Mms-Quotas
		X-Mms-Mbox-Quotas
		X-Mms-Message-Count
		Content
		X-Mms-Start
		Additional-headers
		X-Mms-Distribution-Indicator
		X-Mms-Element-Descriptor
		X-Mms-Limit
		X-Mms-Recommended-Retrieval-Mode
		X-Mms-Recommended-Retrieval-Mode-Text
		X-Mms-Status-Text
		X-Mms-Applic-ID
		X-Mms-Reply-Applic-ID
		X-Mms-Aux-Applic-Info
		X-Mms-DRM-Content
		X-Mms-Adaptation-Allowed
		X-Mms-Replace-ID
		X-Mms-Cancel-ID
		X-Mms-Cancel-Status
#endif
	}

	return pMmsHeaderData;
}

void MsgMmsReleaseHeader(MMS_HEADER_DATA_S **ppMmHeadersData)
{
	if (ppMmHeadersData && *ppMmHeadersData) {
		MMS_HEADER_DATA_S *pMmsHeaderData = *ppMmHeadersData;
		MsgMmsReleaseAddressList(&pMmsHeaderData->to);
		MsgMmsReleaseAddressList(&pMmsHeaderData->cc);
		MsgMmsReleaseAddressList(&pMmsHeaderData->bcc);
		free(pMmsHeaderData);

		*ppMmHeadersData = NULL;
	}
}

MMS_DATA_S *MsgMmsCreate()
{
	MMS_DATA_S * mms_data = (MMS_DATA_S * )calloc(1, sizeof(MMS_DATA_S));
	if (mms_data) {
		mms_data->header = NULL;
		mms_data->multipartlist = NULL;
		mms_data->smil = NULL;
	}
	return mms_data;
}

void MsgMmsRelease(MMS_DATA_S **ppMmsData)
{
	if (ppMmsData && *ppMmsData) {
		MMS_DATA_S *pMmsData = *ppMmsData;

		if (pMmsData->header)
			MsgMmsReleaseHeader(&pMmsData->header);

		if (pMmsData->smil)
			MsgMmsReleaseMultipart(&pMmsData->smil);

		MsgMmsReleaseMultipartList(&pMmsData->multipartlist);

		free(pMmsData);

		*ppMmsData = NULL;
	}
}

static void __freeMultipartListItem(gpointer data)
{
	MMS_MULTIPART_DATA_S * pMultipart = (MMS_MULTIPART_DATA_S *)data;

	if (pMultipart) {
		MsgMmsReleaseMultipart(&pMultipart);
	}
}

int MsgMmsReleaseMultipartList(MMSList **ppMultipartList)
{
	if (ppMultipartList && *ppMultipartList) {
		g_list_free_full(*ppMultipartList, __freeMultipartListItem);
		*ppMultipartList = NULL;
	}

	return 0;
}

static void __freeAddressListItem(gpointer data)
{
	MMS_ADDRESS_DATA_S * pAddressData = (MMS_ADDRESS_DATA_S *)data;

	MsgMmsReleaseAddress(&pAddressData);
}

int MsgMmsReleaseAddressList(MMSList **ppAddressList)
{
	if (ppAddressList && *ppAddressList) {
		g_list_free_full(*ppAddressList, __freeAddressListItem);
		*ppAddressList = NULL;
	}

	return 0;
}

static void removeLessGreaterMark(const char *szSrcID, char *szDest, int destSize)
{
	char szBuf[MSG_MSG_ID_LEN + 1] = {0, };
	int cLen = strlen(szSrcID);

	if (cLen > 1 && szSrcID[0] == '<' && szSrcID[cLen - 1] == '>') {
		strncpy(szBuf, &szSrcID[1], cLen - 2);
		szBuf[cLen - 2] = '\0';
	} else if (cLen > 1 && szSrcID[0] == '"' && szSrcID[cLen-1] == '"') {
		strncpy(szBuf, &szSrcID[1], cLen - 2);
		szBuf[cLen - 2] = '\0';
	} else {
		strncpy(szBuf, szSrcID, cLen);
		szBuf[cLen] = '\0';
	}

	snprintf(szDest, destSize, "%s", szBuf);
}

static bool IsMatchedMedia(MMS_MEDIA_S *media, MMS_MULTIPART_DATA_S *pMultipart)
{
	if (strlen(pMultipart->szContentID) > 0) {
		char szTempContentID[MSG_MSG_ID_LEN + 1] = {0, };
		removeLessGreaterMark(pMultipart->szContentID, szTempContentID, sizeof(szTempContentID));

		if (strcmp(media->szContentID, szTempContentID) == 0) {
			if (strlen(media->szContentLocation) > 0 && strlen(pMultipart->szContentLocation) > 0) {
				if (strcmp(media->szContentLocation,  pMultipart->szContentLocation) == 0) {
					return true;
				} else {
					/* go through */
				}
			} else {
				return true;
			}
		}

		if (strcmp(media->szContentLocation,  szTempContentID) == 0) {
			return true;
		}

		if (media->szContentID[0] != '\0') {
			char *pszExt = strrchr(media->szContentID, '.');
			if (pszExt) {
				char tmpContentID[MSG_MSG_ID_LEN+1] = {0};
				strncpy(tmpContentID, media->szContentID, strlen(media->szContentID));
				int extLength = strlen(pszExt);
				int contentIDLength = strlen(media->szContentID);
				tmpContentID[contentIDLength-extLength] = '\0';

				if (g_strcmp0(tmpContentID, szTempContentID) == 0)
					return true;
			}
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

	return false;
}

static bool MmsFindAndInsertPart(MMS_MESSAGE_DATA_S *pMsgData, MMS_MULTIPART_DATA_S *pMultipart)
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

static bool MmsInsertPartToMmsData(MMS_MESSAGE_DATA_S *pMsgData, MMS_MULTIPART_DATA_S *pMultipart)
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
			attachment->mediatype = pMultipart->type;
			snprintf(attachment->szContentType, sizeof(attachment->szContentType), "%s", pMultipart->szContentType);
			snprintf(attachment->szFilePath, sizeof(attachment->szFilePath), "%s", pMultipart->szFilePath);
			snprintf(attachment->szFileName, sizeof(attachment->szFileName), "%s", pMultipart->szFileName);
			attachment->fileSize = MsgGetFileSize(attachment->szFilePath);
			MSG_SEC_DEBUG("Insert Attach to attachment[%p] : path = [%s], name = [%s], ct = [%s], size = [%d]"\
							, attachment, attachment->szFilePath, attachment->szFileName, attachment->szContentType, attachment->fileSize);
		}

		if (_MsgMmsAddAttachment(pMsgData, attachment) != MSG_SUCCESS) {
			g_free(attachment);
			return false;
		}
	}

	MSG_END();
	return true;
}

int MsgMmsConvertMmsDataToMmsMessageData(MMS_DATA_S *pSrc, MMS_MESSAGE_DATA_S *pDst)
{
	MSG_BEGIN();

	bzero(pDst, sizeof(MMS_MESSAGE_DATA_S));

	if (pSrc->smil) {
		if (MsgAccessFile(pSrc->smil->szFilePath, F_OK)) {
			gchar *contents = NULL;
			gsize length = 0;

			g_file_get_contents((gchar*)pSrc->smil->szFilePath, (gchar**)&contents, (gsize*)&length, NULL);

			if (contents) {
				MsgSmilParseSmilDoc(pDst, contents);
				g_free(contents);
			}
		} else {
			if (pSrc->smil->pMultipartData) {
				char *smil_data = (char *)calloc(1, sizeof(char)*(pSrc->smil->nMultipartDataLen + 1));
				if (smil_data) {
					memcpy(smil_data, pSrc->smil->pMultipartData, pSrc->smil->nMultipartDataLen);
					MsgSmilParseSmilDoc(pDst, smil_data);
					g_free(smil_data);
				}
			}
		}
	}

	int len = g_list_length(pSrc->multipartlist);
	for (int i = 0; i < len; i++) {
		MMS_MULTIPART_DATA_S *multipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(pSrc->multipartlist, i);
		if (multipart) {
			MmsInsertPartToMmsData(pDst, multipart);
		}
	}

	MSG_END();
	return 0;
}

int MsgMmsConvertMmsMessageDataToMmsData(MMS_MESSAGE_DATA_S *pSrc, MMS_DATA_S *pDst)
{
	MSG_BEGIN();

	char *pRawData = NULL;

	pDst->backup_type = pSrc->backup_type;

	int pageCnt = _MsgMmsGetPageCount(pSrc);
	if (pSrc->smil.szFilePath[0] != '\0') {
		MMS_MULTIPART_DATA_S *pMultipart = MsgMmsCreateMultipart();
		if (pMultipart)
			memcpy(pMultipart, &pSrc->smil, sizeof(MMS_MULTIPART_DATA_S));
		pDst->smil = pMultipart;
	} else if (pageCnt > 0) {	/* Multipart related */
		MsgSmilGenerateSmilDoc(pSrc, &pRawData);
		if (pRawData) {
			MMS_MULTIPART_DATA_S *pMultipart = MsgMmsCreateMultipart();

			MSG_DEBUG("%s", pRawData);
			if (pMultipart) {
				pMultipart->pMultipartData = pRawData;
				pMultipart->nMultipartDataLen = strlen(pRawData);
				pMultipart->type = MIME_APPLICATION_SMIL;
				snprintf(pMultipart->szContentType, sizeof(pMultipart->szContentType), "%s", "application/smil");
				pDst->smil = pMultipart;
			}
		} else {
			MSG_DEBUG("Fail to Generate SmilDoc");
		}
	}

	for (int i = 0; i < pageCnt; ++i) {
		MMS_PAGE_S *pPage = _MsgMmsGetPage(pSrc, i);
		if (pPage) {
			int mediaCnt = pPage->mediaCnt;
			MSG_DEBUG("PAGE %d's media Cnt: %d", i+1, mediaCnt);

			for (int j = 0; j < mediaCnt; ++j) {
				MMS_MEDIA_S *pMedia = _MsgMmsGetMedia(pPage, j);
				if ((pMedia) && (pMedia->szFilePath[0] != 0)) {
					MMS_MULTIPART_DATA_S *pMultipart = MsgMmsCreateMultipart();
					if (pMultipart) {
						snprintf(pMultipart->szContentID, sizeof(pMultipart->szContentID), "%s", pMedia->szContentID);
						snprintf(pMultipart->szContentLocation, sizeof(pMultipart->szContentLocation), "%s", pMedia->szContentLocation);
						snprintf(pMultipart->szFileName, sizeof(pMultipart->szFileName), "%s", pMedia->szFileName);
						snprintf(pMultipart->szFilePath, sizeof(pMultipart->szFilePath), "%s", pMedia->szFilePath);
						snprintf(pMultipart->szContentType, sizeof(pMultipart->szContentType), "%s", pMedia->szContentType);
						pDst->multipartlist = g_list_append(pDst->multipartlist, pMultipart);
					}
				}
			}
		}
	}

	/* Processing Attachment List */
	int attachCnt = _MsgMmsGetAttachCount(pSrc);

	for (int i = 0; i < attachCnt; ++i) {
		MMS_ATTACH_S *pMedia = _MsgMmsGetAttachment(pSrc, i);
		if (pMedia->szFilePath[0] != 0) {
			MMS_MULTIPART_DATA_S *pMultipart = MsgMmsCreateMultipart();
			if (pMultipart) {
				snprintf(pMultipart->szContentID, sizeof(pMultipart->szContentID), "%s", pMedia->szFileName);
				snprintf(pMultipart->szContentLocation, sizeof(pMultipart->szContentLocation), "%s", pMedia->szFileName);
				snprintf(pMultipart->szFileName, sizeof(pMultipart->szFileName), "%s", pMedia->szFileName);
				snprintf(pMultipart->szFilePath, sizeof(pMultipart->szFilePath), "%s", pMedia->szFilePath);
				snprintf(pMultipart->szContentType, sizeof(pMultipart->szContentType), "%s", pMedia->szContentType);

				pDst->multipartlist = g_list_append(pDst->multipartlist, pMultipart);
			}
		}
	}

	MSG_END();

	return 0;
}

int MsgMmsGetSmilMultipart(MMSList *pMultipartList, MMS_MULTIPART_DATA_S **smil_multipart)
{
	const char *smil_content_type = "application/smil";

	int len = g_list_length(pMultipartList);

	for (int i = 0; i < len; i++) {
		MMS_MULTIPART_DATA_S *multipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(pMultipartList, i);

		if (multipart) {
			if (strcasecmp(multipart->szContentType, smil_content_type) == 0) {
				*smil_multipart = multipart;
				break;
			}
		}
	}

	return 0;
}

/* get content from filepath and save to pMultipartData */
int MsgMmsSetMultipartData(MMS_MULTIPART_DATA_S *pMultipart)
{
	if (pMultipart->pMultipartData != NULL)
		return 0;

	if (g_file_get_contents((gchar*)pMultipart->szFilePath, (gchar**)&pMultipart->pMultipartData, (gsize*)&pMultipart->nMultipartDataLen, NULL) == false)
		return -1;

	/* Due to Get data for Backup message */
	/* memset(pMultipart->szFilePath, 0x00, sizeof(pMultipart->szFilePath)); */
	return 0;
}

int MsgMmsSetMultipartListData(MMS_DATA_S *pMmsData)
{
	MSG_BEGIN();

	MMSList *multipart_list = pMmsData->multipartlist;

	if (multipart_list) {
		if (pMmsData->smil) {
			MsgMmsSetMultipartData(pMmsData->smil);
		}

		for (int i = 0; i < (int)g_list_length(multipart_list); i++) {
			MMS_MULTIPART_DATA_S *pMultipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(multipart_list, i);

			if (pMultipart) {
				MsgMmsSetMultipartData(pMultipart);/* app file -> data */
			}
		}
	}

	MSG_END();
	return 0;
}

/* pMultipartData set to file path */
int MsgMmsSetMultipartFilePath(const char *dirPath, MMS_MULTIPART_DATA_S *pMultipart)
{
	if (g_file_test(dirPath, G_FILE_TEST_IS_DIR) != true) {
		MSG_SEC_DEBUG("g_file_test is false: [%s] is not dir or not exist", dirPath);
		return -1;
	}

	memset(pMultipart->szFilePath, 0x00, sizeof(pMultipart->szFilePath));

	snprintf(pMultipart->szFilePath, sizeof(pMultipart->szFilePath), "%s/%s", dirPath, pMultipart->szFileName);

	/* remove space character of original file name */
	msg_replace_space_char(pMultipart->szFilePath);

	if (!MsgCreateFile(pMultipart->szFilePath, pMultipart->pMultipartData, pMultipart->nMultipartDataLen)) {
		MSG_SEC_DEBUG("Fail to set content to file [%s]", pMultipart->szFilePath);
		return -1;
	}

	return 0;
}

int MsgMmsSetMultipartListFilePath(const char *dirPath, MMS_DATA_S *pMmsData)
{
	MSG_BEGIN();

	char working_dir[MSG_FILENAME_LEN_MAX+1] = {0, };

	snprintf(working_dir, sizeof(working_dir), "%s", dirPath);

	if (mkdir(working_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
		if (errno == EEXIST) {
			MSG_SEC_DEBUG("exist dir : [%s]", working_dir);
		} else {
			MSG_SEC_DEBUG("Fail to Create Dir [%s]", working_dir);
			return -1;
		}
	}

	MMSList *multipart_list = pMmsData->multipartlist;

	if (pMmsData->smil) {
		snprintf(pMmsData->smil->szFileName, sizeof(pMmsData->smil->szFileName), "%s", "smil.smil");
		MsgMmsSetMultipartFilePath(working_dir, pMmsData->smil);
	}

	if (multipart_list) {
		for (int i = 0; i < (int)g_list_length(multipart_list); i++) {
			MMS_MULTIPART_DATA_S *pMultipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(multipart_list, i);

			if (pMultipart) {
				MsgMmsSetMultipartFilePath(working_dir, pMultipart); /* data -> svc file */
			}
		}
	}

	MSG_END();
	return 0;
}

int printMultipart(MMS_MULTIPART_DATA_S *multipart)
{
	if (multipart) {
		MSG_DEBUG("multipart ptr [%p]", multipart);
		MSG_DEBUG("type : %d", multipart->type);
		MSG_DEBUG("type str : %s", multipart->szContentType);
		MSG_DEBUG("cid : %s", multipart->szContentID);
		MSG_DEBUG("cl : %s", multipart->szContentLocation);
		MSG_DEBUG("name : %s", multipart->szFileName);
		MSG_DEBUG("filepath : %s", multipart->szFilePath);
		MSG_DEBUG("tcs_bc_level : %d", multipart->tcs_bc_level);
		MSG_DEBUG("malware_allow : %d", multipart->malware_allow);
		MSG_DEBUG("thumbfilepath : %s", multipart->szThumbFilePath);
	}
	return 0;
}

int printMultipartList(MMSList *pMultipartList)
{
	int len = g_list_length(pMultipartList);

	for (int i = 0; i < len; i++) {
		MMS_MULTIPART_DATA_S *multipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(pMultipartList, i);

		if (multipart) {
			MSG_DEBUG("[%d] multipart ptr [%p]", i,  multipart);
			MSG_DEBUG("type : %d", multipart->type);
			MSG_DEBUG("type str : %s", multipart->szContentType);
			MSG_DEBUG("cid : %s", multipart->szContentID);
			MSG_DEBUG("cl : %s", multipart->szContentLocation);
			MSG_DEBUG("name : %s", multipart->szFileName);
			MSG_DEBUG("filepath : %s", multipart->szFilePath);
			MSG_DEBUG("tcs_bc_level : %d", multipart->tcs_bc_level);
			MSG_DEBUG("malware_allow : %d", multipart->malware_allow);
			MSG_DEBUG("thumbfilepath : %s", multipart->szThumbFilePath);
		}
	}

	return 0;
}

void _MsgMmsMultipartPrint(MMS_MULTIPART_DATA_S *multipart)
{
	if (multipart == NULL) {
		MSG_DEBUG("Invalid Parameter");
		return;
	}

	MSG_DEBUG("%-25s : %d", "Multipart type", multipart->type);
	MSG_SEC_DEBUG("%-25s : %s", "Multipart filename", multipart->szFileName);
	MSG_SEC_DEBUG("%-25s : %s", "Multipart filepath", multipart->szFilePath);
	MSG_SEC_DEBUG("%-25s : %s", "Multipart content type", multipart->szContentType);
	MSG_SEC_DEBUG("%-25s : %s", "Multipart content id", multipart->szContentID);
	MSG_SEC_DEBUG("%-25s : %s", "Multipart content location", multipart->szContentLocation);
	MSG_DEBUG("%-25s : %d", "Multipart drm type", multipart->drmType);
}

bool  _MsgMmsRemoveEmptyMedia(MMS_PAGE_S *pPage)
{
	MMS_MEDIA_S *pMedia = NULL;

	int mediaCnt  = g_list_length(pPage->medialist);

	for (int i = 0; i < mediaCnt; i++) {
		GList *nth = g_list_nth(pPage->medialist, i);

		if (nth == NULL)
			return false;

		pMedia = (MMS_MEDIA_S *)nth->data;

		if (pMedia == NULL)
			continue;

		if (strlen(pMedia->szFilePath) == 0) {
			MSG_DEBUG("Found Empty Media [%d]", i);

			g_free(pMedia);

			nth->data = NULL;
		}
	}

	pPage->medialist = g_list_remove_all(pPage->medialist, NULL);

	pPage->mediaCnt = g_list_length(pPage->medialist);

	return true;
}

/* remove media object with no filepath */
bool _MsgMmsRemoveEmptyObject(MMS_MESSAGE_DATA_S *pMmsMsg)
{
	MMS_PAGE_S *pPage;

	int pageCnt = g_list_length(pMmsMsg->pagelist);

	for (int i = 0; i < pageCnt; i++) {
		GList *nth = g_list_nth(pMmsMsg->pagelist, i);

		if (nth == NULL)
			return false;

		pPage = (MMS_PAGE_S *)nth->data;

		if (pPage == NULL)
			continue;

		_MsgMmsRemoveEmptyMedia(pPage);

#if 0
		if (g_list_length(pPage->medialist) == 0) {
			MSG_DEBUG("Found Empty Page [%d]", i);

			g_free(pPage);

			nth->data = NULL;
		}
#endif
	}

	pMmsMsg->pagelist = g_list_remove_all(pMmsMsg->pagelist, NULL);

	pMmsMsg->pageCnt = g_list_length(pMmsMsg->pagelist);

	return true;
}

int MsgMmsCheckFilepathSmack(int fd, const char* ipc_filename)
{
	int err = MSG_SUCCESS;

	char *app_smack_label = NULL;
	smack_new_label_from_socket(fd, &app_smack_label);
	if (app_smack_label == NULL) {
		return MSG_ERR_PERMISSION_DENIED;
	}

	MSG_SEC_DEBUG("app_smack_label [%s]", app_smack_label);

	char ipc_filepath[MSG_FILEPATH_LEN_MAX+1] = {0, };
	snprintf(ipc_filepath, MSG_FILEPATH_LEN_MAX, "%s%s", MSG_IPC_DATA_PATH, ipc_filename);

	gchar *serialized_data = NULL;
	gsize serialized_len = 0;
	MSG_SEC_DEBUG("ipc_path [%s]", ipc_filepath);

	if (!g_file_get_contents((gchar*)ipc_filepath, (gchar**)&serialized_data, (gsize*)&serialized_len, NULL)) {
		MSG_FREE(app_smack_label);
		return MSG_ERR_PERMISSION_DENIED;
	}

	MMS_DATA_S *mms_data = NULL;

	MsgDeserializeMmsData(serialized_data, serialized_len, &mms_data);

	if (mms_data) {
		if (mms_data->multipartlist) {
			int len = g_list_length(mms_data->multipartlist);
			for (int i = 0; i < len; i++) {
				MMS_MULTIPART_DATA_S *multipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(mms_data->multipartlist, i);
				if (multipart) {
					err = MsgCheckFilepathSmack(app_smack_label, multipart->szFilePath);
					if (err != MSG_SUCCESS)
						break;
				}
			}
		}
		if (err == MSG_SUCCESS && mms_data->smil) {
			err = MsgCheckFilepathSmack(app_smack_label, mms_data->smil->szFilePath);
		}
		MsgMmsRelease(&mms_data);
	} else {
		err = MSG_ERR_INVALID_PARAMETER;
	}

	MSG_FREE(serialized_data);
	MSG_FREE(app_smack_label);
	return err;
}
