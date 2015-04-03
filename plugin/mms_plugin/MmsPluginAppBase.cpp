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

#include "MsgMmsMessage.h"
#include "MmsPluginDebug.h"
#include "MmsPluginStorage.h"
#include "MmsPluginMessage.h"
#include "MmsPluginMIME.h"
#include "MmsPluginAppBase.h"
#include "MmsPluginUtil.h"
#include "MmsPluginTcs.h"
#include "MsgUtilFile.h"

msg_error_t MmsMakePreviewInfo(int msgId, MMS_MESSAGE_DATA_S *pMmsMsg, bool allow_malware, const char *raw_filepath);

MmsPluginAppBase::MmsPluginAppBase(){}

MmsPluginAppBase::MmsPluginAppBase(MMS_DATA_S *pMmsData)
{
	setMmsData(pMmsData);
}

MmsPluginAppBase::MmsPluginAppBase(MmsMsg *pMmsMsg)
{
	setMmsData(pMmsMsg);
}

MmsPluginAppBase::~MmsPluginAppBase()
{
	MsgMmsReleaseMmsLists(&mmsMsgData);
}

void MmsPluginAppBase::setMmsData(MmsMsg *pMmsMsg)
{
	MmsConvertMsgData(pMmsMsg, &mmsMsgData);
}

void MmsPluginAppBase::setMmsData(MMS_DATA_S *pMmsData)
{
	MsgMmsConvertMmsDataToMmsMessageData(pMmsData, &mmsMsgData);
}

void MmsPluginAppBase::makePreviewInfo(msg_message_id_t  msgId, bool allow_malware, const char *raw_filepath)
{
	MmsMakePreviewInfo(msgId, &mmsMsgData, allow_malware, raw_filepath);
}

void MmsPluginAppBase::getFirstPageTextFilePath(char *textBuf, int textBufSize)
{

	MMS_PAGE_S *pPage = NULL;
	MMS_MEDIA_S *pMedia = NULL;

	MMS_MESSAGE_DATA_S *pMmsMsgData = &mmsMsgData;

	if (pMmsMsgData == NULL)
		return;

	// Get the text data from the 1st slide.
	if (pMmsMsgData->pageCnt > 0) {

		pPage = _MsgMmsGetPage(pMmsMsgData, 0);

		if (pPage) {

			for (int j = 0; j < pPage->mediaCnt; ++j) {

				pMedia = _MsgMmsGetMedia(pPage, j);

				if (pMedia->mediatype == MMS_SMIL_MEDIA_TEXT) {

					MimeType mimeType = MIME_UNKNOWN;

					MmsGetMimeTypeFromFileName(MIME_MAINTYPE_UNKNOWN, pMedia->szFilePath, &mimeType, NULL);

					if (mimeType == MIME_TEXT_X_VCALENDAR || mimeType == MIME_TEXT_X_VCARD || mimeType == MIME_TEXT_X_VTODO || mimeType == MIME_TEXT_X_VNOTE) {
						MSG_DEBUG("Media Type is Text, but Vobject file [%s]", pMedia->szFilePath);
					} else {
						MSG_DEBUG("Text path : [%s]", pMedia->szFilePath);
						snprintf(textBuf, textBufSize, "%s", pMedia->szFilePath);//Set Text Filepath of First Pages
					}
					break;
				}
			}
		}
	}

	return;
}


//FIXME::need to move AppBase
msg_error_t MmsMakePreviewInfo(int msgId, MMS_MESSAGE_DATA_S *pMmsMsg, bool allow_malware, const char *raw_filepath)
{
	MMS_PAGE_S *pPage = NULL;
	MMS_MEDIA_S *pMedia = NULL;
	int bc_level = -1;

	int ref_attach_count = 0;
	const char * attachment_name = NULL;

	if (pMmsMsg == NULL)
		return MSG_ERR_NULL_POINTER;

	MmsPluginStorage::instance()->removePreviewInfo(msgId); //remove exist previnfo

	//scan malware in raw file
	if (raw_filepath && strlen(raw_filepath) > 0 && MsgAccessFile(raw_filepath, F_OK) == true) {
		int tcs_ret = 0; // MmsPluginTcsScanFile(raw_filepath, &bc_level);
		if (tcs_ret == 0) {
			if (bc_level > -1) {
				MSG_DEBUG("malware exist, level = %d", bc_level);
				MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_MALWARE, "malware", bc_level);
			}
		}
	}

	//check ref type and increase attach count
	if (pMmsMsg->pageCnt > 0) {
		for (int i = 0; i < pMmsMsg->pageCnt; i++) {
			pPage = _MsgMmsGetPage(pMmsMsg, i);
			if (pPage) {
				for (int j = 0; j < pPage->mediaCnt; j++) {
					pMedia = _MsgMmsGetMedia(pPage, j);
					if (pMedia) { //IF Vobject type add to Attach in Preview data

						MimeType mimeType = MIME_UNKNOWN;
						MmsGetMimeTypeFromFileName(MIME_MAINTYPE_UNKNOWN, pMedia->szFilePath, &mimeType, NULL);
						if (mimeType == MIME_TEXT_X_VCALENDAR || mimeType == MIME_TEXT_X_VCARD) {

							MSG_DEBUG("Unsupported File [%s] It will be add to attach list", pMedia->szFilePath);

							ref_attach_count++;

							if (attachment_name == NULL) {
								attachment_name = pMedia->szFileName;
							}
						}

					}
				}
			}
		}
	}


	if (pMmsMsg->pageCnt > 0) {

		MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_PAGE, (char *)"pagecount", pMmsMsg->pageCnt);

		pPage = _MsgMmsGetPage(pMmsMsg, 0);

		if (pPage) {
			for (int j = 0; j < pPage->mediaCnt; j++) {

				pMedia = _MsgMmsGetMedia(pPage, j);

				if (pMedia == NULL) {
					MSG_ERR("[%d]th pMedia is NULL", j);
					continue;
				}

				MSG_SEC_DEBUG("pMedia's Name: %s", pMedia->szFilePath);

				if (allow_malware == false && bc_level > -1) {
					if (pMedia->mediatype == MMS_SMIL_MEDIA_AUDIO) {
						MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_AUDIO, pMedia->szFileName);
					}
				} else {

					if (j == 0) { //First Page, First Media
						MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_1ST_MEDIA, pMedia->szFilePath);
					}

					if (pMedia->mediatype == MMS_SMIL_MEDIA_IMG || pMedia->mediatype == MMS_SMIL_MEDIA_VIDEO) {
						char szFileName[MSG_FILENAME_LEN_MAX+1] = {0, };
						char thumbPath[MSG_FILEPATH_LEN_MAX+1] = {0, };
						char *pszExt = NULL;

						memset(szFileName, 0x00, MSG_FILENAME_LEN_MAX+1);
						memset(thumbPath, 0x00, MSG_FILEPATH_LEN_MAX);

						MSG_DEBUG("drm type = %d, %s", pMedia->drmType, pMedia->szFilePath);

						if (pMedia->drmType == MSG_DRM_TYPE_NONE) {

							snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%d.mms",msgId);

							if ((pszExt = strrchr(pMedia->szFilePath, '.')) != NULL && !strcasecmp(pszExt, ".png")) {
								snprintf(thumbPath, MSG_FILEPATH_LEN_MAX, "%s%s.png", MSG_THUMBNAIL_PATH, szFileName);
							} else {
								snprintf(thumbPath, MSG_FILEPATH_LEN_MAX, "%s%s.jpg", MSG_THUMBNAIL_PATH, szFileName);
							}

							if (MmsMakeImageThumbnail(pMedia->szFilePath, thumbPath) == true) {
								if (pMedia->mediatype == MMS_SMIL_MEDIA_IMG) {
									MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_IMG, thumbPath);
								} else {
									MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_VIDEO, thumbPath);
								}
							} else {
								MSG_DEBUG("Fail of generating thumbnail: %s to %s", pMedia->szFilePath, thumbPath);
							}
						}
					} else if (pMedia->mediatype == MMS_SMIL_MEDIA_AUDIO) {
						MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_AUDIO, pMedia->szFileName);
					}
				}
			}
		} // end for
	} else {
		MSG_DEBUG("There is no page");
	}

	int attachCnt = _MsgMmsGetAttachCount(pMmsMsg);
	if (attachCnt > 0) {

		MMS_ATTACH_S *pAttach = _MsgMmsGetAttachment(pMmsMsg, 0);

		MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_ATTACH, pAttach->szFileName, attachCnt);
		MmsPluginStorage::instance()->updateMmsAttachCount(msgId, attachCnt); // for Get Message

		if (attachment_name == NULL) {
			attachment_name = pAttach->szFileName;
		}

	} else {
		MSG_DEBUG("There is no attachment");
	}

	if (attachCnt + ref_attach_count > 0 && attachment_name) {

		MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_ATTACH, attachment_name, attachCnt + ref_attach_count);
		MmsPluginStorage::instance()->updateMmsAttachCount(msgId, attachCnt + ref_attach_count);
	}

	return MSG_SUCCESS;
}
