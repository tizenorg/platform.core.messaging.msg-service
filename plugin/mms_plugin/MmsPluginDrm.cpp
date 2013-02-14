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

#include <string.h>
#include <drm_client_types.h>
#include <drm_client.h>
#include "MsgMmsTypes.h"
#include "MsgDrmWrapper.h"
#include "MmsPluginDrm.h"
#include "MmsPluginCodec.h"
#include "MmsPluginMIME.h"
#include "MmsPluginDebug.h"
#include "MmsPluginUtil.h"

#ifdef __SUPPORT_DRM__

MmsDrm2ConvertState	mmsDrm2ConvertState;

MsgDrmType MsgGetDRMType(MsgType *pMsgType, MsgBody *pMsgBody)
{
	MsgDrmType drmType = MSG_DRM_TYPE_NONE;
	MsgMultipart *pMultipart = NULL;

	if (MsgIsMultipart(pMsgType->type)) {
		pMultipart = pMsgBody->body.pMultipart;
		while (pMultipart) {
			if (drmType < pMultipart->type.drmInfo.drmType)
				drmType = pMultipart->type.drmInfo.drmType;

			pMultipart = pMultipart->pNext;
		}
	} else {
		drmType = pMsgType->drmInfo.drmType;
	}

	return drmType;
}

void MmsDrm2SetConvertState(MmsDrm2ConvertState newConvertState)
{
	mmsDrm2ConvertState = newConvertState;
	MSG_DEBUG("MmsDrm2SetConvertState: mmsDrm2ConvertState = %d\n", mmsDrm2ConvertState);
}

MmsDrm2ConvertState MmsDrm2GetConvertState(void)
{
	return mmsDrm2ConvertState;
}

bool MsgDRM2GetDRMInfo(char *szFilePath, MsgType *pMsgType)
{
	if (szFilePath == NULL || pMsgType == NULL) {
		MSG_DEBUG("Param is NULL szFilePath = %d, pMsgType = %d", szFilePath, pMsgType);
		return false;
	}

	char szMimeType[DRM_MAX_LEN_MIME + 1];
	char szContentID[DRM_MAX_LEN_CID + 1];
	MSG_DRM_TYPE drmType = MSG_DRM_NONE;

	MsgDrmGetDrmType(szFilePath, &drmType);
	MsgDrmGetMimeTypeEx(szFilePath, szMimeType, sizeof(szMimeType));
	MsgDrmGetContentID(szFilePath, szContentID, sizeof(szContentID));
	MSG_DEBUG("drmType: %d", drmType);

	switch (drmType) {
	case MSG_DRM_FORWARD_LOCK:
		pMsgType->drmInfo.drmType = MSG_DRM_TYPE_FL;
		pMsgType->drmInfo.contentType = (MimeType)_MsgGetCode(MSG_TYPE, szMimeType);
		if (MsgCopyDrmInfo(pMsgType) == false) {
			MSG_DEBUG("MsgDRM2GetDRMInfo : MsgCopyDrmInfo failed");
			return false;
		}
		break;

	case MSG_DRM_COMBINED_DELIVERY:
		pMsgType->drmInfo.drmType = MSG_DRM_TYPE_CD;
		pMsgType->drmInfo.szContentURI = MsgResolveContentURI(szContentID);
		break;

	case MSG_DRM_SEPARATE_DELIVERY:
		pMsgType->drmInfo.drmType = MSG_DRM_TYPE_SD;

		pMsgType->drmInfo.contentType = (MimeType)_MsgGetCode(MSG_TYPE, szMimeType);

		drm_content_info_s dcfHdrInfo;
		bzero(&dcfHdrInfo, sizeof(drm_content_info_s));
		drm_get_content_info(szFilePath, &dcfHdrInfo);

		drm_file_info_s fileInfo;
		bzero(&fileInfo, sizeof(drm_file_info_s));
		drm_get_file_info(szFilePath,&fileInfo);

		if (fileInfo.oma_info.version == DRM_OMA_DRMV1_RIGHTS) {
			pMsgType->drmInfo.szContentName = MsgRemoveQuoteFromFilename(dcfHdrInfo.title);
			pMsgType->drmInfo.szContentDescription = MsgStrCopy(dcfHdrInfo.description);
		}
		break;

	default:
		pMsgType->drmInfo.drmType = MSG_DRM_TYPE_NONE;
		break;
	}

	pMsgType->drmInfo.szDrm2FullPath = MsgStrCopy(szFilePath);
	MSG_DEBUG("pMsgType->drmInfo.szDrm2FullPath: %s", pMsgType->drmInfo.szDrm2FullPath);

	return true;
}

bool MsgDRMIsForwardLockType(MsgDrmType	drmType)
{
	switch (drmType) {
	case MSG_DRM_TYPE_FL:
	case MSG_DRM_TYPE_CD:
		return true;

	case MSG_DRM_TYPE_NONE:		//SD & plain can be forwarded
	case MSG_DRM_TYPE_SD:
	default:
		return false;
	}
}

bool MsgChangeDrm2FileName(char *szFileName)
{
	char szTempFileName[MSG_FILENAME_LEN_MAX] = {0,};

	if (szFileName == NULL || szFileName[0] == '\0')
		return false;

	MsgGetFileNameWithoutExtension(szTempFileName, szFileName);

	if (strrchr(szTempFileName, '.'))
		return true;

	strcat(szTempFileName, ".dcf");
	strcpy(szFileName, szTempFileName);

	MSG_DEBUG("MsgChangeDrm2FileName: made szFileName = %s \n", szFileName);

	return true;
}

bool MsgIsDCFFile(char *szFilePath)
{
	int length = 0;

	MSG_DEBUG("MsgIsDCFFile: szFilePath = %s \n", szFilePath);

	length = MsgStrlen(szFilePath);
	if (szFilePath[length - 4] == '.' &&
		(szFilePath[length - 3] == 'd' || szFilePath[length - 3] == 'D') &&
		(szFilePath[length - 2] == 'c' || szFilePath[length - 2] == 'C') &&
		(szFilePath[length - 1] == 'f' || szFilePath[length - 1] == 'F'))
		return true;

	return false;
}

#endif
