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
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

#include "MsgTypes.h"
#include "MsgMmsTypes.h"
#include "MsgMmsMessage.h"
#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgStorageTypes.h"
#include "MsgInternalTypes.h"


MMS_SMIL_ROOTLAYOUT	rootlayout;

static void __release_glist_element(gpointer data, gpointer user_data);
static void __release_page_element(gpointer data, gpointer user_data);

static void __release_glist_element(gpointer data, gpointer user_data)
{
	if(data != NULL) {
		free(data);
	}
}

static void __release_page_element(gpointer data, gpointer user_data)
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

msg_error_t _MsgMmsReleasePageList(MMS_MESSAGE_DATA_S *pMsgData)
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

msg_error_t _MsgMmsReleaseRegionList(MMS_MESSAGE_DATA_S *pMsgData)
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

msg_error_t _MsgMmsReleaseAttachList(MMS_MESSAGE_DATA_S *pMsgData)
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

msg_error_t _MsgMmsReleaseTransitionList(MMS_MESSAGE_DATA_S *pMsgData)
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

msg_error_t _MsgMmsReleaseMetaList(MMS_MESSAGE_DATA_S *pMsgData)
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

	return MSG_SUCCESS;
}

msg_error_t _MsgMmsAddMedia(MMS_PAGE_S* pPage, MMS_MEDIA_S *pMedia)
{
	if(pPage == NULL || pMedia == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	pPage->medialist = g_list_append(pPage->medialist, pMedia);
	pPage->mediaCnt++;
		MSG_DEBUG("media's mediatype: %d", pMedia->mediatype);
		MSG_DEBUG("media's filename: %s", pMedia->szFileName);
		MSG_DEBUG("media's filepath: %s", pMedia->szFilePath);
		MSG_DEBUG("media's contentId: %s", pMedia->szContentID);
	MSG_DEBUG("page's media count: %d", pPage->mediaCnt);

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

bool _MsgMmsFindMatchedMedia(MMS_MESSAGE_DATA_S *pMsgData, char *pszFilePath)
{
	if (pMsgData == NULL || pszFilePath == NULL)
		return false;

	if (pMsgData->pagelist) {
		for (int pageIdx = 0; pageIdx < pMsgData->pageCnt; pageIdx++) {
			MMS_PAGE_S *page = (MMS_PAGE_S *)g_list_nth_data(pMsgData->pagelist, pageIdx);

			if (page && page->medialist) {
				for (int mediaIdx = 0; mediaIdx < page->mediaCnt; mediaIdx++) {
					MMS_MEDIA_S *media = (MMS_MEDIA_S *)g_list_nth_data(page->medialist, mediaIdx);
					if (media) {
						if (strcmp(pszFilePath, media->szFilePath) == 0)
							return true;
					}
				}
			}
		}
	}

	return false;
}

msg_error_t _MsgMmsAddSmilDoc(char* pSmil, MMS_MESSAGE_DATA_S* pMsgData)
{
	MSG_DEBUG("MsgMmsAddSmilDoc");

	if(pSmil == NULL || pMsgData == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	char	fullpath[MSG_FILEPATH_LEN_MAX] = {0,};
	char smilFileName[MSG_FILENAME_LEN_MAX+1] = {0,};
	time_t	RawTime = 0;

	//Create smilFileName
	time(&RawTime);
	snprintf(smilFileName, MSG_FILENAME_LEN_MAX+1, "%lu", RawTime);
	snprintf(fullpath, MSG_FILEPATH_LEN_MAX, "%s%s", MSG_SMIL_FILE_PATH, smilFileName);

	if (MsgWriteSmilFile(fullpath, pSmil, strlen(pSmil)) == false) {
		MSG_DEBUG("MsgWriteSmilFile error");
		return MSG_ERR_MMS_ERROR;
	}
	strncpy(pMsgData->szSmilFilePath, smilFileName, MSG_FILEPATH_LEN_MAX-1);

	return MSG_SUCCESS;
}


char* _MsgMmsSerializeMessageData(const MMS_MESSAGE_DATA_S* pMsgData, unsigned int *pSize)
{
	MSG_DEBUG("MsgMmsSerializeMessageData");

	if (pMsgData == NULL)
		return NULL;

	int bufsize = 0;
	int offset = 0;
	int pageCnt = 0;
	char *buf = NULL;

	pageCnt = pMsgData->pageCnt;

	int mediaCnt = 0;

	if (pMsgData->pagelist) {
		for (int pageIdx = 0; pageIdx < pageCnt; pageIdx++) {
			bufsize += sizeof(int);	// Media cnt

			MMS_PAGE_S *page = (MMS_PAGE_S *)g_list_nth_data(pMsgData->pagelist, pageIdx);
			mediaCnt = page->mediaCnt;

			if (page->medialist) {
				for (int i = 0; i < mediaCnt; i++) {
					MMS_MEDIA_S *media = (MMS_MEDIA_S *)g_list_nth_data(page->medialist, i);

					if (media->mediatype == MMS_SMIL_MEDIA_TEXT) {
						bufsize += (sizeof(MmsSmilMediaType) + MSG_FILENAME_LEN_MAX + 2 * MSG_FILEPATH_LEN_MAX + MSG_MSG_ID_LEN + 1
							+ MAX_SMIL_ALT_LEN + MAX_SMIL_REGION_ID
#ifdef __SUPPORT_DRM__
							+ sizeof(MsgDrmType) + MSG_FILEPATH_LEN_MAX
#endif
							+ MAX_SMIL_TRANSIN_ID + MAX_SMIL_TRANSOUT_ID +
							7 * sizeof(int) + 4* sizeof(bool) + sizeof(MmsTextDirection) /*+ sizeof(MmsSmilFontType)*/);
					} else {
						bufsize += (sizeof(MmsSmilMediaType) + MSG_FILENAME_LEN_MAX + 2 * MSG_FILEPATH_LEN_MAX + MSG_MSG_ID_LEN + 1
							+ MAX_SMIL_ALT_LEN + MAX_SMIL_REGION_ID
#ifdef __SUPPORT_DRM__
							+ sizeof(MsgDrmType) + MSG_FILEPATH_LEN_MAX
#endif
							+ MAX_SMIL_TRANSIN_ID + MAX_SMIL_TRANSOUT_ID + 5 * sizeof(int)
#ifdef MMS_SMIL_ANIMATE
							+ MAX_SMIL_ANIMATE_ATTRIBUTE_NAME + MAX_SMIL_ANIMATE_ATTRIBUTE_TYPE + MAX_SMIL_ANIMATE_TARGET_ELEMENT
							+ MAX_SMIL_ANIMATE_CALC_MODE + 5 * sizeof(int)
#endif
							);
					}
				}
			}

			bufsize += sizeof(int) * 6;
		}
	}

	bufsize += sizeof(int); // region count;
	if (pMsgData->regionlist) {
		int elementSize = g_list_length(pMsgData->regionlist);
		bufsize += elementSize * (MAX_SMIL_REGION_ID + 4 * sizeof(MMS_LENGTH) + sizeof(int) + sizeof(REGION_FIT_TYPE_T));
	}

	bufsize += sizeof(int); // attachment count;
	if (pMsgData->attachlist) {
		int elementSize = g_list_length(pMsgData->attachlist);
		bufsize += elementSize * (sizeof(MimeType) + MSG_FILENAME_LEN_MAX + MSG_FILEPATH_LEN_MAX + sizeof(int)
#ifdef __SUPPORT_DRM__
				+ sizeof(MsgDrmType) + MSG_FILEPATH_LEN_MAX
#endif
				);
	}

	bufsize += sizeof(int); // transition count;
	if (pMsgData->transitionlist) {
		int elementSize = g_list_length(pMsgData->transitionlist);
		bufsize += elementSize * (MAX_SMIL_TRANSITION_ID + sizeof(MmsSmilTransType) + sizeof(MmsSmilTransSubType) + sizeof(int));
	}

	bufsize += sizeof(int); // meta count;
	if (pMsgData->metalist) {
		int elementSize = g_list_length(pMsgData->metalist);
		bufsize += elementSize * (MAX_SMIL_META_ID + MAX_SMIL_META_NAME + MAX_SMIL_META_CONTENT);
	}

	bufsize += sizeof(MMS_SMIL_ROOTLAYOUT);

#ifdef FEATURE_JAVA_MMS
	bufsize += sizeof(MMS_APPID_INFO_S);
#endif

	int filePathLen = strlen(pMsgData->szSmilFilePath);

	bufsize += sizeof(int) + filePathLen + sizeof(int);

	MSG_DEBUG("MsgMmsSerializeMessageData: bufsize = %d", bufsize);

	buf = (char *)calloc(bufsize, 1);

	// copy file path length
	MSG_DEBUG("MsgMmsSerializeMessageData: smilFilePath Length = %d",  filePathLen);

	memcpy(buf, &filePathLen, sizeof(int));

	offset += sizeof(int);

	// copy file path
	MSG_DEBUG("MsgMmsSerializeMessageData: smilFilePath = %s",  pMsgData->szSmilFilePath);

	if (filePathLen > 0) {
		memcpy(buf + offset, pMsgData->szSmilFilePath, filePathLen);

		offset += filePathLen;
	}

	// copy page count
	MSG_DEBUG("MsgMmsSerializeMessageData: pageCnt = %d",  pMsgData->pageCnt);

	memcpy(buf + offset, &(pMsgData->pageCnt), sizeof(int));

	offset += sizeof(int);

	if (pMsgData->pagelist) {
		for (int pageIdx = 0; pageIdx < pageCnt; pageIdx++)
		{
			MMS_PAGE_S *page = (MMS_PAGE_S *)g_list_nth_data(pMsgData->pagelist, pageIdx);
			mediaCnt = page->mediaCnt;

			memcpy(buf + offset, &mediaCnt, sizeof(int));
			offset += sizeof(int);

			MSG_DEBUG("MsgMmsSerializeMessageData: mediaCnt = %d",  mediaCnt);
			if (page->medialist) {
				for (int i = 0; i < mediaCnt; ++ i) {
					MMS_MEDIA_S *media = (MMS_MEDIA_S *)g_list_nth_data(page->medialist, i);

					memcpy(buf + offset, &(media->mediatype), sizeof(MmsSmilMediaType));
					offset += sizeof(MmsSmilMediaType);
					MSG_DEBUG("%d media's mediatype = %d", i, media->mediatype);

					memcpy(buf + offset, media->szSrc, MSG_FILEPATH_LEN_MAX);
					offset +=  MSG_FILEPATH_LEN_MAX;

					memcpy(buf + offset, media->szFileName, MSG_FILENAME_LEN_MAX);
					offset += MSG_FILENAME_LEN_MAX;
					MSG_DEBUG("%d media's filename = %s", i, media->szFileName);

					memcpy(buf + offset, media->szFilePath, MSG_FILEPATH_LEN_MAX);
					offset += MSG_FILEPATH_LEN_MAX;
					MSG_DEBUG("%d media's filepath = %s", i, media->szFilePath);

					memcpy(buf + offset, media->szContentID, MSG_MSG_ID_LEN + 1);
					offset +=  MSG_MSG_ID_LEN + 1;
					MSG_DEBUG("%d media's contentID = %s", i, media->szContentID);

					memcpy(buf + offset, media->regionId, MAX_SMIL_REGION_ID);
					offset +=  MAX_SMIL_REGION_ID;
					MSG_DEBUG("%d media's regionId = %s", i, media->regionId);

					memcpy(buf + offset, media->szAlt, MAX_SMIL_ALT_LEN);
					offset +=  MAX_SMIL_ALT_LEN;

#ifdef __SUPPORT_DRM__
					memcpy(buf + offset, &(media->drmType), sizeof(MsgDrmType));
					offset +=  sizeof(MsgDrmType);
					memcpy(buf + offset, media->szDrm2FullPath, MSG_FILEPATH_LEN_MAX);
					offset += MSG_FILEPATH_LEN_MAX;
#endif

					if (media->mediatype == MMS_SMIL_MEDIA_TEXT) {
						MSG_DEBUG("##### Media = TEXT #####");
						memcpy(buf + offset, media->sMedia.sText.szTransInId, MAX_SMIL_TRANSIN_ID);
						offset +=  MAX_SMIL_TRANSIN_ID;

						memcpy(buf + offset, media->sMedia.sText.szTransOutId, MAX_SMIL_TRANSOUT_ID);
						offset +=  MAX_SMIL_TRANSOUT_ID;

						memcpy(buf + offset, &(media->sMedia.sText.nRepeat), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sText.nBegin), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sText.nEnd), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sText.nDurTime), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sText.nBgColor), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sText.bBold), sizeof(bool));
						offset += sizeof(bool);

						memcpy(buf + offset, &(media->sMedia.sText.bUnderLine), sizeof(bool));
						offset += sizeof(bool);

						memcpy(buf + offset, &(media->sMedia.sText.bItalic), sizeof(bool));
						offset += sizeof(bool);

						memcpy(buf + offset, &(media->sMedia.sText.bReverse), sizeof(bool));
						offset += sizeof(bool);

						memcpy(buf + offset, &(media->sMedia.sText.nDirection), sizeof(MmsTextDirection));
						offset += sizeof(MmsTextDirection);

						//memcpy(buf + offset, &(media->sMedia.sText.nFont), sizeof(MmsSmilFontType));
						//offset += sizeof(MmsSmilFontType);

						memcpy(buf + offset, &(media->sMedia.sText.nSize), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sText.nColor), sizeof(int));
						offset += sizeof(int);
					} else {
						MSG_DEBUG("##### Media = IMAGE, AUDIO, VIDEO #####");
						memcpy(buf + offset, media->sMedia.sAVI.szTransInId, MAX_SMIL_TRANSIN_ID);
						offset +=  MAX_SMIL_TRANSIN_ID;

						memcpy(buf + offset, media->sMedia.sAVI.szTransOutId, MAX_SMIL_TRANSOUT_ID);
						offset +=  MAX_SMIL_TRANSOUT_ID;

						memcpy(buf + offset, &(media->sMedia.sAVI.nRepeat), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sAVI.nBegin), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sAVI.nEnd), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sAVI.nDurTime), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sAVI.nBgColor), sizeof(int));
						offset += sizeof(int);

#ifdef MMS_SMIL_ANIMATE
						memcpy(buf + offset, media->sMedia.sAVI.nAttributeName, MAX_SMIL_ANIMATE_ATTRIBUTE_NAME);
						offset +=  MAX_SMIL_ANIMATE_ATTRIBUTE_NAME;

						memcpy(buf + offset, media->sMedia.sAVI.nAttributeType, MAX_SMIL_ANIMATE_ATTRIBUTE_TYPE);
						offset +=  MAX_SMIL_ANIMATE_ATTRIBUTE_TYPE;

						memcpy(buf + offset, media->sMedia.sAVI.nTargetElement, MAX_SMIL_ANIMATE_TARGET_ELEMENT);
						offset +=  MAX_SMIL_ANIMATE_TARGET_ELEMENT;

						memcpy(buf + offset, &(media->sMedia.sAVI.nFrom), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sAVI.nTo), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sAVI.nBy), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sAVI.nValues), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, &(media->sMedia.sAVI.nDur), sizeof(int));
						offset += sizeof(int);

						memcpy(buf + offset, media->sMedia.sAVI.nCalcMode, MAX_SMIL_ANIMATE_CALC_MODE);
						offset +=  MAX_SMIL_ANIMATE_CALC_MODE;
#endif
					}
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

	memcpy(buf + offset, &pMsgData->regionCnt, sizeof(int));
	offset += sizeof(int);
	MSG_DEBUG("pMsgData->regionCnt: %d", pMsgData->regionCnt);

	if (pMsgData->regionlist) {
		for (int i = 0; i < pMsgData->regionCnt; ++ i) {
			MMS_SMIL_REGION *region = (MMS_SMIL_REGION *)g_list_nth_data(pMsgData->regionlist, i);

			memcpy(buf + offset, region->szID, MAX_SMIL_REGION_ID);
			offset += MAX_SMIL_REGION_ID;
			MSG_DEBUG("%d region's ID = %s", i, region->szID);
			memcpy(buf + offset, &region->nLeft, sizeof(MMS_LENGTH));
			offset += sizeof(MMS_LENGTH);
			memcpy(buf + offset, &region->nTop, sizeof(MMS_LENGTH));
			offset += sizeof(MMS_LENGTH);
			memcpy(buf + offset, &region->width, sizeof(MMS_LENGTH));
			offset += sizeof(MMS_LENGTH);
			memcpy(buf + offset, &region->height, sizeof(MMS_LENGTH));
			offset += sizeof(MMS_LENGTH);
			memcpy(buf + offset, &region->bgColor, sizeof(int));
			offset += sizeof(int);
			memcpy(buf + offset, &region->fit, sizeof(REGION_FIT_TYPE_T));
			offset += sizeof(REGION_FIT_TYPE_T);
		}
	}

	memcpy(buf + offset, &pMsgData->attachCnt, sizeof(int));
	offset += sizeof(int);
	MSG_DEBUG("pMsgData->attachCnt: %d", pMsgData->attachCnt);

	if (pMsgData->attachlist) {
		for (int i = 0; i < pMsgData->attachCnt; ++ i) {
			MMS_ATTACH_S *attach = (MMS_ATTACH_S *)g_list_nth_data(pMsgData->attachlist, i);

			memcpy(buf + offset, &(attach->mediatype), sizeof(MimeType));
			offset += sizeof(MimeType);
			MSG_DEBUG("%d attachment's mediatype = %d", i, attach->mediatype);

			memcpy(buf + offset, attach->szFileName, MSG_FILENAME_LEN_MAX);
			offset += MSG_FILENAME_LEN_MAX;
			MSG_DEBUG("%d attachment's filename = %s", i, attach->szFileName);

			memcpy(buf + offset, attach->szFilePath, MSG_FILEPATH_LEN_MAX);
			offset += MSG_FILEPATH_LEN_MAX;
			MSG_DEBUG("%d attachment's filepath = %s", i, attach->szFilePath);

			memcpy(buf + offset, &(attach->fileSize), sizeof(int));
			offset +=  sizeof(int);
			MSG_DEBUG("%d attachment's file size = %d", i, attach->fileSize);

#ifdef __SUPPORT_DRM__
			memcpy(buf + offset, &(attach->drmType), sizeof(MsgDrmType));
			offset +=  sizeof(MsgDrmType);
			memcpy(buf + offset, attach->szDrm2FullPath, MSG_FILEPATH_LEN_MAX);
			offset += MSG_FILEPATH_LEN_MAX;
#endif
		}
	}

	memcpy(buf + offset, &pMsgData->transitionCnt, sizeof(int));
	offset += sizeof(int);
	MSG_DEBUG("pMsgData->transitionCnt: %d", pMsgData->transitionCnt);

	if (pMsgData->transitionlist) {
		for (int i = 0; i < pMsgData->transitionCnt; ++ i) {
			MMS_SMIL_TRANSITION *transition = (MMS_SMIL_TRANSITION *)g_list_nth_data(pMsgData->transitionlist, i);

			memcpy(buf + offset, transition->szID, MAX_SMIL_TRANSITION_ID);
			offset += MAX_SMIL_TRANSITION_ID;
			MSG_DEBUG("%d transition's ID = %s", i, transition->szID);

			memcpy(buf + offset, &transition->nType, sizeof(MmsSmilTransType));
			offset += sizeof(MmsSmilTransType);
			memcpy(buf + offset, &transition->nSubType, sizeof(MmsSmilTransSubType));
			offset += sizeof(MmsSmilTransSubType);
			memcpy(buf + offset, &transition->nDur, sizeof(int));
			offset += sizeof(int);
		}
	}

	memcpy(buf + offset, &pMsgData->metaCnt, sizeof(int));
	offset += sizeof(int);
	MSG_DEBUG("pMsgData->metaCnt: %d", pMsgData->metaCnt);

	if (pMsgData->metalist) {
		for (int i = 0; i < pMsgData->metaCnt; ++ i) {
			MMS_SMIL_META *meta = (MMS_SMIL_META *)g_list_nth_data(pMsgData->metalist, i);

			memcpy(buf + offset, meta->szID, MAX_SMIL_META_ID);
			offset += MAX_SMIL_META_ID;
			MSG_DEBUG("%d meta's ID = %s", i, meta->szID);

			memcpy(buf + offset, meta->szName, MAX_SMIL_META_NAME);
			offset += MAX_SMIL_META_NAME;
			MSG_DEBUG("%d meta's ID = %s", i, meta->szID);

			memcpy(buf + offset, meta->szContent, MAX_SMIL_META_CONTENT);
			offset += MAX_SMIL_META_CONTENT;
			MSG_DEBUG("%d meta's ID = %s", i, meta->szID);
		}
	}

	memcpy(buf + offset, &pMsgData->rootlayout, sizeof(MMS_SMIL_ROOTLAYOUT));
	offset += sizeof(MMS_SMIL_ROOTLAYOUT);

#ifdef FEATURE_JAVA_MMS
	memcpy(buf + offset, &pMsgData->msgAppId, sizeof(MMS_APPID_INFO_S));
	offset += sizeof(MMS_APPID_INFO_S);
#endif

	*pSize = offset;

	return buf;
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


bool		_MsgMmsDeserializeMessageData(MMS_MESSAGE_DATA_S* pBody, char* pData)
{
	MSG_DEBUG("MmsGetMsgBodyfromFile");

	if (pBody == NULL || pData == NULL) {
		MSG_DEBUG("param is NULL. pBody = %x, pData = %x", pBody, pData);
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

	memcpy(&filePathLen, pData, sizeof(int));

	offset += sizeof(int);

	MSG_DEBUG("Smil File Path Length : %d", filePathLen);

	if (filePathLen > MSG_FILEPATH_LEN_MAX) {
		MSG_DEBUG("Smil File Path Length is abnormal.");
		return false;
	}

	memset(pBody->szSmilFilePath, 0x00, MSG_FILEPATH_LEN_MAX);

	if (filePathLen > 0) {
		memcpy(pBody->szSmilFilePath, pData + offset, filePathLen);

		offset += filePathLen;
	}

	memcpy(&(pBody->pageCnt), pData + offset, sizeof(int));

	offset += sizeof(int);

	pageCnt = pBody->pageCnt;

	MSG_DEBUG("MMS PAGE COUNT: %d", pageCnt);

	for (int j = 0; j < pageCnt; ++j) {
		pPage = (MMS_PAGE_S *)calloc(sizeof(MMS_PAGE_S), 1);

		memcpy(&pPage->mediaCnt, pData + offset, sizeof(int));
		offset += sizeof(int);
		MSG_DEBUG("MMS MEDIA COUNT: %d", pPage->mediaCnt);

		for (int i = 0; i < pPage->mediaCnt; ++i) {
			pMedia = (MMS_MEDIA_S *)calloc(sizeof(MMS_MEDIA_S), 1);

			memcpy(&pMedia->mediatype, pData + offset, sizeof(int));
			offset += sizeof(int);

			memcpy(pMedia->szSrc, pData + offset, MSG_FILEPATH_LEN_MAX);
			offset += MSG_FILEPATH_LEN_MAX;

			memcpy(pMedia->szFileName, pData + offset, MSG_FILENAME_LEN_MAX);
			offset += MSG_FILENAME_LEN_MAX;

			memcpy(pMedia->szFilePath, pData + offset, MSG_FILEPATH_LEN_MAX);
			offset += MSG_FILEPATH_LEN_MAX;

			memcpy(pMedia->szContentID, pData + offset, MSG_MSG_ID_LEN+1);
			offset += MSG_MSG_ID_LEN + 1;

			memcpy(pMedia->regionId, pData + offset, MAX_SMIL_REGION_ID);
			offset += MAX_SMIL_REGION_ID;

			memcpy(pMedia->szAlt, pData + offset, MAX_SMIL_ALT_LEN);
			offset += MAX_SMIL_ALT_LEN;

#ifdef __SUPPORT_DRM__
			memcpy(&pMedia->drmType, pData + offset, sizeof(MsgDrmType));
			offset += sizeof(MsgDrmType);

			memcpy(pMedia->szDrm2FullPath, pData + offset, MSG_FILEPATH_LEN_MAX);
			offset += MSG_FILEPATH_LEN_MAX;
#endif

			if (pMedia->mediatype == MMS_SMIL_MEDIA_TEXT) {
				MSG_DEBUG("##### MEDIA TYPE = TEXT #####");
				memcpy(pMedia->sMedia.sText.szTransInId, pData + offset, MAX_SMIL_TRANSIN_ID);
				offset += MAX_SMIL_TRANSIN_ID;

				memcpy(pMedia->sMedia.sText.szTransOutId, pData + offset, MAX_SMIL_TRANSOUT_ID);
				offset += MAX_SMIL_TRANSOUT_ID;

				memcpy(&pMedia->sMedia.sText.nRepeat, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sText.nBegin, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sText.nEnd, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sText.nDurTime, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sText.nBgColor, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sText.bBold, pData + offset, sizeof(bool));
				offset += sizeof(bool);

				memcpy(&pMedia->sMedia.sText.bUnderLine, pData + offset, sizeof(bool));
				offset += sizeof(bool);

				memcpy(&pMedia->sMedia.sText.bItalic, pData + offset, sizeof(bool));
				offset += sizeof(bool);

				memcpy(&pMedia->sMedia.sText.bReverse, pData + offset, sizeof(bool));
				offset += sizeof(bool);

				memcpy(&pMedia->sMedia.sText.nDirection, pData + offset, sizeof(MmsTextDirection));
				offset += sizeof(MmsTextDirection);

				//memcpy(&pMedia->sMedia.sText.nFont, pData + offset, sizeof(MmsSmilFontType));
				//offset += sizeof(MmsSmilFontType);

				memcpy(&pMedia->sMedia.sText.nSize, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sText.nColor, pData + offset, sizeof(int));
				offset += sizeof(int);
			} else {
				MSG_DEBUG("##### MEDIA TYPE = IMAGE, AUDIO, VIDEO #####");
				memcpy(pMedia->sMedia.sAVI.szTransInId, pData + offset, MAX_SMIL_TRANSIN_ID);
				offset += MAX_SMIL_TRANSIN_ID;

				memcpy(pMedia->sMedia.sAVI.szTransOutId, pData + offset, MAX_SMIL_TRANSOUT_ID);
				offset += MAX_SMIL_TRANSOUT_ID;

				memcpy(&pMedia->sMedia.sAVI.nRepeat, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sAVI.nBegin, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sAVI.nEnd, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sAVI.nDurTime, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sAVI.nBgColor, pData + offset, sizeof(int));
				offset += sizeof(int);
#ifdef MMS_SMIL_ANIMATE
				memcpy(pMedia->sMedia.sAVI.nAttributeName, pData + offset, MAX_SMIL_ANIMATE_ATTRIBUTE_NAME);
				offset += MAX_SMIL_ANIMATE_ATTRIBUTE_NAME;

				memcpy(pMedia->sMedia.sAVI.nAttributeType, pData + offset, MAX_SMIL_ANIMATE_ATTRIBUTE_TYPE);
				offset += MAX_SMIL_ANIMATE_ATTRIBUTE_TYPE;

				memcpy(pMedia->sMedia.sAVI.nTargetElement, pData + offset, MAX_SMIL_ANIMATE_TARGET_ELEMENT);
				offset += MAX_SMIL_ANIMATE_TARGET_ELEMENT;

				memcpy(&pMedia->sMedia.sAVI.nFrom, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sAVI.nTo, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sAVI.nBy, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sAVI.nValues, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(&pMedia->sMedia.sAVI.nDur, pData + offset, sizeof(int));
				offset += sizeof(int);

				memcpy(pMedia->sMedia.sAVI.nCalcMode, pData + offset, MAX_SMIL_ANIMATE_CALC_MODE);
				offset += MAX_SMIL_ANIMATE_CALC_MODE;
#endif
			}
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

		pBody->pagelist = g_list_append(pBody->pagelist, pPage);
	}

	//Processing Region List
	memcpy(&pBody->regionCnt, pData + offset, sizeof(int));
	offset += sizeof(int);

	MSG_DEBUG(" pBody->regionCnt: %d", pBody->regionCnt);

	for (int i = 0; i < pBody->regionCnt; ++i) {
		pRegion = (MMS_SMIL_REGION *)calloc(sizeof(MMS_SMIL_REGION), 1);

		memcpy(pRegion->szID, pData + offset, MAX_SMIL_REGION_ID);
		offset += MAX_SMIL_REGION_ID;
		memcpy(&pRegion->nLeft, pData + offset, sizeof(MMS_LENGTH));
		offset += sizeof(MMS_LENGTH);
		memcpy(&pRegion->nTop, pData + offset, sizeof(MMS_LENGTH));
		offset += sizeof(MMS_LENGTH);
		memcpy(&pRegion->width, pData + offset, sizeof(MMS_LENGTH));
		offset += sizeof(MMS_LENGTH);
		memcpy(&pRegion->height, pData + offset, sizeof(MMS_LENGTH));
		offset += sizeof(MMS_LENGTH);
		memcpy(&pRegion->bgColor, pData + offset, sizeof(int));
		offset += sizeof(int);
		memcpy(&pRegion->fit, pData + offset, sizeof(REGION_FIT_TYPE_T));
		offset += sizeof(REGION_FIT_TYPE_T);

		pBody->regionlist = g_list_append(pBody->regionlist, pRegion);
	}

	//Processing Attachment List
	memcpy(&pBody->attachCnt, pData + offset, sizeof(int));
	offset += sizeof(int);

	MSG_DEBUG(" pBody->attachCnt: %d", pBody->attachCnt);

	for (int i = 0; i < pBody->attachCnt; ++i) {
		pAttach = (MMS_ATTACH_S *)calloc(sizeof(MMS_ATTACH_S), 1);

		memcpy(&pAttach->mediatype, pData + offset, sizeof(MimeType));
		offset += sizeof(MimeType);

		memcpy(pAttach->szFileName, pData + offset, MSG_FILENAME_LEN_MAX);
		offset += MSG_FILENAME_LEN_MAX;

		memcpy(pAttach->szFilePath, pData + offset, MSG_FILEPATH_LEN_MAX);
		offset += MSG_FILEPATH_LEN_MAX;

		memcpy(&pAttach->fileSize, pData + offset, sizeof(int));
		offset += sizeof(int);

#ifdef __SUPPORT_DRM__
		memcpy(&pAttach->drmType, pData + offset, sizeof(MsgDrmType));
		offset += sizeof(MsgDrmType);

		memcpy(pAttach->szDrm2FullPath, pData + offset, MSG_FILEPATH_LEN_MAX);
		offset += MSG_FILEPATH_LEN_MAX;
#endif

		pBody->attachlist = g_list_append(pBody->attachlist, pAttach);
	}

	//Processing Transition List
	memcpy(&pBody->transitionCnt, pData + offset, sizeof(int));
	offset += sizeof(int);

	MSG_DEBUG(" pBody->transitionCnt: %d", pBody->transitionCnt);

	for (int i = 0; i < pBody->transitionCnt; ++i) {
		pTransition = (MMS_SMIL_TRANSITION *)calloc(sizeof(MMS_SMIL_TRANSITION), 1);

		memcpy(pTransition->szID, pData + offset, MAX_SMIL_TRANSITION_ID);
		offset += MAX_SMIL_TRANSITION_ID;

		memcpy(&pTransition->nType, pData + offset, sizeof(MmsSmilTransType));
		offset += sizeof(MmsSmilTransType);
		memcpy(&pTransition->nSubType, pData + offset, sizeof(MmsSmilTransSubType));
		offset += sizeof(MmsSmilTransSubType);
		memcpy(&pTransition->nDur, pData + offset, sizeof(int));
		offset += sizeof(int);

		pBody->transitionlist = g_list_append(pBody->transitionlist, pTransition);
	}

	//Processing Meta List
	memcpy(&pBody->metaCnt, pData + offset, sizeof(int));
	offset += sizeof(int);

	MSG_DEBUG(" pBody->metaCnt: %d", pBody->metaCnt);

	for (int i = 0; i < pBody->metaCnt; ++i) {
		pMeta = (MMS_SMIL_META *)calloc(sizeof(MMS_SMIL_META), 1);

		memcpy(pMeta->szID, pData + offset, MAX_SMIL_META_ID);
		offset += MAX_SMIL_META_ID;

		memcpy(pMeta->szName, pData + offset, MAX_SMIL_META_NAME);
		offset += MAX_SMIL_META_NAME;

		memcpy(pMeta->szContent, pData + offset, MAX_SMIL_META_CONTENT);
		offset += MAX_SMIL_META_CONTENT;

		pBody->metalist = g_list_append(pBody->metalist, pMeta);
	}

	memcpy(&pBody->rootlayout, pData + offset, sizeof(MMS_SMIL_ROOTLAYOUT));
	offset += sizeof(MMS_SMIL_ROOTLAYOUT);

#ifdef FEATURE_JAVA_MMS
	memcpy(&pBody->msgAppId, pData + offset, sizeof(MMS_APPID_INFO_S));
	offset += sizeof(MMS_APPID_INFO_S);
	MSG_DEBUG("java_app_id valid:%d, appId:%s repleToAppId:%s", pBody->msgAppId.valid, pBody->msgAppId.appId, pBody->msgAppId.replyToAppId);
#endif
	//free(pData);

	return true;
}

bool	_MsgMmsSetRootLayout(MMS_MESSAGE_DATA_S* pMmsMsg, MMS_SMIL_ROOTLAYOUT* pRootlayout)
{
	memcpy(&pMmsMsg->rootlayout, pRootlayout, sizeof(MMS_SMIL_ROOTLAYOUT));
	return true;
}

