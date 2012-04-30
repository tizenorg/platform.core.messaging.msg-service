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

#include <string.h>
#include "MmsPluginDrm.h"
#include "MmsPluginCodec.h"
#include "MsgMmsTypes.h"
#include "MsgDrmWrapper.h"
#include "MsgDebug.h"
#include "drm-service.h"

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

	char szMimeType[DRM_MAX_TYPE_LEN + 1];
	char szContentID[DRM_MAX_CID_LEN + 1];
	MSG_DRM_TYPE drmType = MSG_DRM_NONE;

	MsgDrmGetDrmType(szFilePath, &drmType);
	MsgDrmGetMimeTypeEx(szFilePath, szMimeType, DRM_MAX_TYPE_LEN + 1);
	MsgDrmGetContentID(szFilePath, szContentID, DRM_MAX_CID_LEN + 1);
	MSG_DEBUG("drmType: %d", drmType);

	switch (drmType) {
	case MSG_DRM_FORWARD_LOCK:
		pMsgType->drmInfo.drmType = MSG_DRM_TYPE_FL;
		pMsgType->drmInfo.contentType = (MsgContentType)_MsgGetCode(MSG_TYPE, szMimeType);
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

		pMsgType->drmInfo.contentType = (MsgContentType)_MsgGetCode(MSG_TYPE, szMimeType);

		drm_dcf_header_t dcfHdrInfo;
		drm_svc_get_dcf_header_info(szFilePath, &dcfHdrInfo);

		if (dcfHdrInfo.version == DRM_OMA_DRMV1_RIGHTS) {
			pMsgType->drmInfo.szContentName = MsgRemoveQuoteFromFilename(dcfHdrInfo.headerUnion.headerV1.contentName);
			pMsgType->drmInfo.szContentDescription = MsgStrCopy(dcfHdrInfo.headerUnion.headerV1.contentDescription);
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
