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

#include <string.h>

#if MSG_DRM_SUPPORT
#include <drm_client_types.h>
#include <drm_client.h>
#endif

#include "MsgDrmWrapper.h"
#include "MmsPluginDrm.h"
#include "MmsPluginCodec.h"
#include "MmsPluginMIME.h"
#include "MmsPluginDebug.h"

bool MmsPluginDrmGetInfo(const char *szFilePath, MsgType *pMsgType)
{
#if MSG_DRM_SUPPORT
	if (szFilePath == NULL || pMsgType == NULL) {
		MSG_DEBUG("Param is NULL szFilePath = %d, pMsgType = %d", szFilePath, pMsgType);
		return false;
	}

	char szMimeType[DRM_MAX_LEN_MIME + 1] = {0,};
	char szContentID[DRM_MAX_LEN_CID + 1] = {0,};
	MSG_DRM_TYPE drmType = MSG_DRM_NONE;
	int ret = 0;

	MsgDrmGetDrmType(szFilePath, &drmType);
	MsgDrmGetMimeTypeEx(szFilePath, szMimeType, sizeof(szMimeType));
	MsgDrmGetContentID(szFilePath, szContentID, sizeof(szContentID));

	MSG_DEBUG("drmType: [%d], mimetype: [%s], contentID: [%s]", drmType, szMimeType, szContentID);

	switch (drmType) {
	case MSG_DRM_FORWARD_LOCK:
		pMsgType->drmInfo.drmType = MSG_DRM_TYPE_FL;
		pMsgType->drmInfo.contentType = MimeGetMimeIntFromMimeString(szMimeType);
		break;

	case MSG_DRM_COMBINED_DELIVERY:
		pMsgType->drmInfo.drmType = MSG_DRM_TYPE_CD;
		pMsgType->drmInfo.szContentURI = MsgResolveContentURI(szContentID);
		break;

	case MSG_DRM_SEPARATE_DELIVERY:
		pMsgType->drmInfo.drmType = MSG_DRM_TYPE_SD;
		pMsgType->drmInfo.contentType = MimeGetMimeIntFromMimeString(szMimeType);

		drm_content_info_s dcfHdrInfo;
		bzero(&dcfHdrInfo, sizeof(drm_content_info_s));
		ret = drm_get_content_info(szFilePath, &dcfHdrInfo);
		if (ret != DRM_RETURN_SUCCESS)
			MSG_DEBUG("drm_get_content_info is failed, ret=[%d]", ret);

		drm_file_info_s fileInfo;
		bzero(&fileInfo, sizeof(drm_file_info_s));
		ret = drm_get_file_info(szFilePath, &fileInfo);
		if (ret != DRM_RETURN_SUCCESS)
			MSG_DEBUG("drm_get_file_info is failed, ret=[%d]", ret);

		if (fileInfo.oma_info.version == DRM_OMA_DRMV1_RIGHTS) {
			pMsgType->drmInfo.szContentName = MsgRemoveQuoteFromFilename(dcfHdrInfo.title);
			pMsgType->drmInfo.szContentDescription = g_strdup(dcfHdrInfo.description);
		}
		break;

	default:
		pMsgType->drmInfo.drmType = MSG_DRM_TYPE_NONE;
		break;
	}

	pMsgType->drmInfo.szDrm2FullPath = g_strdup(szFilePath);
	MSG_DEBUG("pMsgType->drmInfo.szDrm2FullPath: %s", pMsgType->drmInfo.szDrm2FullPath);

#endif
	return true;
}
