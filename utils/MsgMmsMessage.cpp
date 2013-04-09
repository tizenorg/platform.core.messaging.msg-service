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

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "MsgTypes.h"
#include "MsgMmsTypes.h"
#include "MsgMmsMessage.h"
#include "MsgDebug.h"

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

char* _MsgMmsSerializeMessageData(const MMS_MESSAGE_DATA_S *pMsgData, size_t *pSize)
{
	MSG_BEGIN();

	if (pMsgData == NULL)
		return NULL;

	int bufsize = 0;
	int offset = 0;
	int pageCnt = 0;
	char *buf = NULL;

	bufsize += sizeof(int);	// Page cnt

	pageCnt = pMsgData->pageCnt;

	if (pMsgData->pagelist) {

		for (int pageIdx = 0; pageIdx < pageCnt; pageIdx++) {

			int mediaCnt = 0;

			MMS_PAGE_S *page = (MMS_PAGE_S *)g_list_nth_data(pMsgData->pagelist, pageIdx);

			mediaCnt = page->mediaCnt;

			bufsize += sizeof(int);	// Media cnt

			if (page->medialist) {
				bufsize += sizeof(MMS_MEDIA_S) * mediaCnt;
			}

			bufsize += sizeof(int) * 6;//Dur, Begin, End, Min, Max, Repeat
		}
	}

	bufsize += sizeof(int); // region count;

	if (pMsgData->regionlist) {
		int elementSize = g_list_length(pMsgData->regionlist);
		bufsize += elementSize * (sizeof(MMS_SMIL_REGION));
	}

	bufsize += sizeof(int); // attachment count;
	if (pMsgData->attachlist) {
		int elementSize = g_list_length(pMsgData->attachlist);
		bufsize += elementSize * sizeof(MMS_ATTACH_S);
	}

	bufsize += sizeof(int); // transition count;
	if (pMsgData->transitionlist) {
		int elementSize = g_list_length(pMsgData->transitionlist);
		bufsize += elementSize * sizeof(MMS_SMIL_TRANSITION);
	}

	bufsize += sizeof(int); // meta count;
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

	bufsize += sizeof(int);	// type
	bufsize += sizeof(MMS_HEADER_DATA_S);
	bufsize += sizeof(MMS_MULTIPART_DATA_S);

	MSG_DEBUG("Serialize bufsize = %d", bufsize);

	buf = (char *)calloc(bufsize, 1);

	int serial_index = 0;

	memcpy(buf, &pMsgData->backup_type, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] backup type = %d",serial_index++, offset, pMsgData->backup_type);
	offset += sizeof(int);

	//smilFilePath
	memcpy(buf + offset , &filePathLen, sizeof(int));

	// copy file path
	MSG_DEBUG("[#%2d][%5d] smilFilePath = %s, len = %d",serial_index++, offset, pMsgData->szSmilFilePath, filePathLen);
	offset += sizeof(int);

	if (filePathLen > 0) {
		memcpy(buf + offset, pMsgData->szSmilFilePath, filePathLen);
		offset += filePathLen;
	}

	// copy page count
	MSG_DEBUG("[#%2d][%5d] page count = %d",serial_index++, offset, pMsgData->pageCnt);
	memcpy(buf + offset, &(pMsgData->pageCnt), sizeof(int));
	offset += sizeof(int);

	if (pMsgData->pagelist) {

		for (int pageIdx = 0; pageIdx < pageCnt; pageIdx++) {
			MMS_PAGE_S *page = (MMS_PAGE_S *)g_list_nth_data(pMsgData->pagelist, pageIdx);

			MSG_DEBUG("[#%2d][%5d][%d page] media count = %d",serial_index++, offset, pageIdx, page->mediaCnt);
			memcpy(buf + offset, &page->mediaCnt, sizeof(int));
			offset += sizeof(int);

			if (page->medialist) {
				for (int i = 0; i < page->mediaCnt; ++ i) {
					MMS_MEDIA_S *media = (MMS_MEDIA_S *)g_list_nth_data(page->medialist, i);
					memcpy(buf + offset, media, sizeof(MMS_MEDIA_S));
					offset += sizeof(MMS_MEDIA_S);
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

	MSG_DEBUG("[#%2d][%5d] region count = %d",serial_index++, offset, pMsgData->regionCnt);

	memcpy(buf + offset, &pMsgData->regionCnt, sizeof(int));
	offset += sizeof(int);

	if (pMsgData->regionlist) {
		for (int i = 0; i < pMsgData->regionCnt; ++ i) {
			MMS_SMIL_REGION *region = (MMS_SMIL_REGION *)g_list_nth_data(pMsgData->regionlist, i);
			memcpy(buf + offset, region, sizeof(MMS_SMIL_REGION));
			offset += sizeof(MMS_SMIL_REGION);
		}
	}

	MSG_DEBUG("[#%2d][%5d] attach count = %d",serial_index++, offset, pMsgData->attachCnt);

	memcpy(buf + offset, &pMsgData->attachCnt, sizeof(int));
	offset += sizeof(int);

	if (pMsgData->attachlist) {
		for (int i = 0; i < pMsgData->attachCnt; ++ i) {
			MMS_ATTACH_S *attach = (MMS_ATTACH_S *)g_list_nth_data(pMsgData->attachlist, i);
			memcpy(buf + offset, attach, sizeof(MMS_ATTACH_S));
			offset += sizeof(MMS_ATTACH_S);
		}
	}

	MSG_DEBUG("[#%2d][%5d] transition count = %d",serial_index++, offset, pMsgData->transitionCnt);

	memcpy(buf + offset, &pMsgData->transitionCnt, sizeof(int));
	offset += sizeof(int);
	if (pMsgData->transitionlist) {
		for (int i = 0; i < pMsgData->transitionCnt; ++ i) {
			MMS_SMIL_TRANSITION *transition = (MMS_SMIL_TRANSITION *)g_list_nth_data(pMsgData->transitionlist, i);
			memcpy(buf + offset, transition, sizeof(MMS_SMIL_TRANSITION));
			offset += sizeof(MMS_SMIL_TRANSITION);
		}
	}

	MSG_DEBUG("[#%2d][%5d] meta count = %d",serial_index++, offset, pMsgData->metaCnt);
	memcpy(buf + offset, &pMsgData->metaCnt, sizeof(int));
	offset += sizeof(int);

	if (pMsgData->metalist) {
		for (int i = 0; i < pMsgData->metaCnt; ++ i) {
			MMS_SMIL_META *meta = (MMS_SMIL_META *)g_list_nth_data(pMsgData->metalist, i);

			memcpy(buf + offset, meta, sizeof(MMS_SMIL_META));
			offset += sizeof(MMS_SMIL_META);
		}
	}

	MSG_DEBUG("[#%2d][%5d] root layout",serial_index++, offset);
	memcpy(buf + offset, &pMsgData->rootlayout, sizeof(MMS_SMIL_ROOTLAYOUT));
	offset += sizeof(MMS_SMIL_ROOTLAYOUT);

#ifdef FEATURE_JAVA_MMS
	MSG_DEBUG("[#%2d][%5d] java mms",serial_index++, offset);
	memcpy(buf + offset, &pMsgData->msgAppId, sizeof(MMS_APPID_INFO_S));
	offset += sizeof(MMS_APPID_INFO_S);
#endif

	memcpy (buf + offset, &pMsgData->header, sizeof(MMS_HEADER_DATA_S));
	MSG_DEBUG("[#%2d][%5d] mms header",serial_index++, offset);
	offset += sizeof(MMS_HEADER_DATA_S);

	memcpy (buf + offset, &pMsgData->smil, sizeof(MMS_MULTIPART_DATA_S));
	MSG_DEBUG("[#%2d][%5d] mms smil",serial_index++, offset);
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
	MSG_DEBUG("[#%2d][%5d] backup type = %d",serial_index++, offset, type);
	offset += sizeof(int);

	pMsgData->backup_type = type;

	memcpy(&filePathLen, pData + offset, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] smil path len = %d",serial_index, offset, filePathLen);
	offset += sizeof(int);

	if (filePathLen > MSG_FILEPATH_LEN_MAX) {
		MSG_DEBUG("Smil File Path Length is abnormal.");
		return false;
	}

	memset(pMsgData->szSmilFilePath, 0x00, MSG_FILEPATH_LEN_MAX);

	if (filePathLen > 0) {
		memcpy(pMsgData->szSmilFilePath, pData + offset, filePathLen);
		MSG_DEBUG("[#%2d][%5d] smil path = %s",serial_index, offset, pMsgData->szSmilFilePath);
		offset += filePathLen;
	}

	serial_index++;

	memcpy(&(pMsgData->pageCnt), pData + offset, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] page count = %d",serial_index++, offset, pMsgData->pageCnt);
	offset += sizeof(int);

	pageCnt = pMsgData->pageCnt;

	for (int j = 0; j < pageCnt; ++j) {
		pPage = (MMS_PAGE_S *)calloc(sizeof(MMS_PAGE_S), 1);

		memcpy(&pPage->mediaCnt, pData + offset, sizeof(int));
		MSG_DEBUG("[#%2d][%5d][%d page] media count = %d", serial_index++, offset, j, pPage->mediaCnt);
		offset += sizeof(int);

		for (int i = 0; i < pPage->mediaCnt; ++i) {
			pMedia = (MMS_MEDIA_S *)calloc(sizeof(MMS_MEDIA_S), 1);

			memcpy(pMedia, pData + offset, sizeof(MMS_MEDIA_S));

			offset += sizeof(MMS_MEDIA_S);

			pPage->medialist = g_list_append(pPage->medialist, pMedia);
		}

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
	}

	//Processing Region List
	memcpy(&pMsgData->regionCnt, pData + offset, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] region count = %d",serial_index++, offset, pMsgData->regionCnt);
	offset += sizeof(int);

	//MSG_DEBUG(" pBody->regionCnt: %d", pBody->regionCnt);

	for (int i = 0; i < pMsgData->regionCnt; ++i) {
		pRegion = (MMS_SMIL_REGION *)calloc(sizeof(MMS_SMIL_REGION), 1);

		memcpy(pRegion, pData + offset, sizeof(MMS_SMIL_REGION));
		offset += sizeof(MMS_SMIL_REGION);

		pMsgData->regionlist = g_list_append(pMsgData->regionlist, pRegion);
	}

	//Processing Attachment List
	memcpy(&pMsgData->attachCnt, pData + offset, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] attach count = %d",serial_index++, offset, pMsgData->attachCnt);
	offset += sizeof(int);

	for (int i = 0; i < pMsgData->attachCnt; ++i) {
		pAttach = (MMS_ATTACH_S *)calloc(sizeof(MMS_ATTACH_S), 1);

		memcpy(pAttach, pData + offset, sizeof(MMS_ATTACH_S));
		offset += sizeof(MMS_ATTACH_S);

		pMsgData->attachlist = g_list_append(pMsgData->attachlist, pAttach);
	}

	//Processing Transition List
	memcpy(&pMsgData->transitionCnt, pData + offset, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] transition count = %d",serial_index++, offset, pMsgData->transitionCnt);
	offset += sizeof(int);

	for (int i = 0; i < pMsgData->transitionCnt; ++i) {
		pTransition = (MMS_SMIL_TRANSITION *)calloc(sizeof(MMS_SMIL_TRANSITION), 1);

		memcpy(pTransition, pData + offset, sizeof(MMS_SMIL_TRANSITION));

		offset += sizeof(MMS_SMIL_TRANSITION);

		pMsgData->transitionlist = g_list_append(pMsgData->transitionlist, pTransition);
	}

	//Processing Meta List
	memcpy(&pMsgData->metaCnt, pData + offset, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] meta count = %d",serial_index++, offset, pMsgData->metaCnt);
	offset += sizeof(int);

	for (int i = 0; i < pMsgData->metaCnt; ++i) {
		pMeta = (MMS_SMIL_META *)calloc(sizeof(MMS_SMIL_META), 1);

		memcpy(pMeta, pData + offset, sizeof(MMS_SMIL_META));

		offset += sizeof(MMS_SMIL_META);

		pMsgData->metalist = g_list_append(pMsgData->metalist, pMeta);
	}

	MSG_DEBUG("[#%2d][%5d] root layout",serial_index++, offset);
	memcpy(&pMsgData->rootlayout, pData + offset, sizeof(MMS_SMIL_ROOTLAYOUT));
	offset += sizeof(MMS_SMIL_ROOTLAYOUT);

#ifdef FEATURE_JAVA_MMS
	MSG_DEBUG("[#%2d][%5d] java mms",serial_index++, offset);
	memcpy(&pMsgData->msgAppId, pData + offset, sizeof(MMS_APPID_INFO_S));
	offset += sizeof(MMS_APPID_INFO_S);
//	MSG_DEBUG("java_app_id valid:%d, appId:%s repleToAppId:%s", pBody->msgAppId.valid, pBody->msgAppId.appId, pBody->msgAppId.replyToAppId);
#endif

	memcpy(&pMsgData->header, pData + offset, sizeof(MMS_HEADER_DATA_S));
	MSG_DEBUG("[#%2d][%5d] mms header",serial_index++, offset);
	offset += sizeof(MMS_HEADER_DATA_S);

	memcpy(&pMsgData->smil, pData + offset, sizeof(MMS_MULTIPART_DATA_S));
	MSG_DEBUG("[#%2d][%5d] mms smil",serial_index++, offset);
	offset += sizeof(MMS_MULTIPART_DATA_S);

	MSG_DEBUG("Final offset : %d", offset);
	MSG_END();
	return true;
}

