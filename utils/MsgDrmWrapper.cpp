/*
* Copyright 2012  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.tizenopensource.org/license
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "MsgDebug.h"
#include "MsgDrmWrapper.h"
#include "MsgUtilFile.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <drm_client_types.h>
#include <drm_client.h>

#define MSG_MAX_DRM_FILE_PATH MSG_FILEPATH_LEN_MAX

bool MsgDrmRegisterFile(MSG_DRM_OPENMODE eMode, char *pBuffer, int nSize)
{
	if (eMode == MSG_MODE_STREAM) {
		MSG_DEBUG("Fail(eMode == MSG_MODE_STREAM)");
		return false;
	}

	if (pBuffer == NULL) {
		MSG_DEBUG("[Error] pBuffer is NULL");
		return false;
	}

	MSG_DEBUG("buffer = %s, nSize = %d", pBuffer, nSize);

	drm_bool_type_e isDrm;
	int eDRMResult = drm_is_drm_file(pBuffer, &isDrm);

	if (eDRMResult != DRM_RETURN_SUCCESS || isDrm != DRM_TRUE) {
		MSG_DEBUG("file is not drm file");
		return false;
	}

	drm_request_type_e request_type = DRM_REQUEST_TYPE_REGISTER_FILE;

	eDRMResult = drm_process_request(request_type, pBuffer, NULL);
	if (DRM_RETURN_SUCCESS != eDRMResult) {
		MSG_DEBUG("drm_process_request is failed : %d", eDRMResult);
		return false;
	}
	MSG_END();
	return true;
}

bool MsgDrmUnregisterFile(char *szFilename)
{
	if (szFilename == NULL) {
		MSG_DEBUG("[Error] szFilename is NULL");
		return false;
	}

	MSG_DEBUG("szFilename = %s", szFilename);

	drm_request_type_e request_type = DRM_REQUEST_TYPE_UNREGISTER_FILE;

	int eDRMResult = drm_process_request(request_type, szFilename, NULL); // Unregister a DCF file
	if (DRM_RETURN_SUCCESS != eDRMResult) {
		MSG_DEBUG("drm_process_request : %d", eDRMResult);
		return false;
	}

	return true;
}

bool MsgDrmIsDrmFile(const char *szFilePath)
{
	drm_bool_type_e isDrm;
	int eDRMResult = drm_is_drm_file(szFilePath, &isDrm);

	if (eDRMResult != DRM_RETURN_SUCCESS || isDrm != DRM_TRUE) {
		MSG_DEBUG("file is not drm file");
		return false;
	}

	return true;
}

/*Added to convert the .dm files in to .dcf files since our platform supports only .dcf :: Start*/
bool MsgDrmConvertDmtoDcfType(char *inputFile, char *outputFile)
{
	return true;
}

bool MsgDrmGetDrmType(const char *szFileName, MSG_DRM_TYPE *eDRMType)
{
	if (szFileName == NULL || eDRMType == NULL) {
		MSG_DEBUG("Param is NULL");
		return false;
	}

	drm_file_type_e file_type;
	int result = drm_get_file_type(szFileName, &file_type);
	if (result != DRM_RETURN_SUCCESS) {
		MSG_DEBUG("drm_get_file_type is failed %d", result);
		return false;
	}

	if (file_type == DRM_TYPE_OMA_V1) {
		drm_file_info_s drmInfo;
		bzero(&drmInfo, sizeof(drm_file_info_s));
		int eDRMResult = drm_get_file_info(szFileName, &drmInfo);
		if (DRM_RETURN_SUCCESS != eDRMResult) {
			MSG_DEBUG("drm_get_file_info is Fail eDRMResult = %d", eDRMResult);
			return false;
		}

		// Convert DRM_METHOD into MSG_DRM_TYPE
		switch (drmInfo.oma_info.method) {
		case DRM_METHOD_TYPE_FORWARD_LOCK:
			*eDRMType = MSG_DRM_FORWARD_LOCK;
			break;
		case DRM_METHOD_TYPE_COMBINED_DELIVERY:
			*eDRMType = MSG_DRM_COMBINED_DELIVERY;
			break;
		case DRM_METHOD_TYPE_SEPARATE_DELIVERY:
			*eDRMType = MSG_DRM_SEPARATE_DELIVERY;
			break;
		default:
			*eDRMType = MSG_DRM_NONE;
			break;
		}
		MSG_DEBUG("eDRMType : %d", *eDRMType);
	} else {
		MSG_DEBUG("This is not a DRM_TYPE_OMA_V1 type");
		return false;
	}

	return true;
}

bool MsgDrmGetMimeTypeEx(const char *szFileName, char *szMimeType, int nMimeTypeLen)
{
	if (!szFileName || !szMimeType || !nMimeTypeLen) {
		MSG_DEBUG("param is NULL");
		return false;
	}

	char strTemp[MSG_MAX_DRM_FILE_PATH + 1] = {0,};

	strncpy(strTemp, szFileName, strlen(szFileName));

	drm_content_info_s tdcfContentinfo;
	memset(&tdcfContentinfo, 0x00, sizeof(drm_content_info_s));
	int eDRMResult = drm_get_content_info(strTemp, &tdcfContentinfo); // Get attribute of DRM File
	if (DRM_RETURN_SUCCESS == eDRMResult) {
		MSG_DEBUG("contentType = %s", tdcfContentinfo.mime_type);
		snprintf(szMimeType, nMimeTypeLen, "%s", tdcfContentinfo.mime_type);

	} else {
		MSG_DEBUG("drm_get_content_info is failed %d", eDRMResult);
		return false;
	}

	return true;
}

bool MsgDrmGetContentID(const char *szFileName, char *szContentID, int nContentIDLen)
{
	if (!szFileName || !szContentID || !nContentIDLen) {
		MSG_DEBUG("param is NULL");
		return false;
	}

	char strTemp[MSG_MAX_DRM_FILE_PATH + 1] = {0,};

	strncpy(strTemp, szFileName, sizeof(strTemp));

	drm_content_info_s  content_info;
	memset(&content_info, 0x00, sizeof(drm_content_info_s));

	int result = drm_get_content_info(strTemp, &content_info);
	if (DRM_RETURN_SUCCESS == result) {
		MSG_DEBUG("contentID = %s", content_info.content_id);
        snprintf(szContentID, nContentIDLen, "%s", content_info.content_id);
	} else {
		MSG_DEBUG("drm_get_content_info is failed %d", result);
		return false;
	}

	return true;
}

