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
#include "drm-service.h"


typedef struct {
	DRM_RIGHTS_CONSUME_HANDLE hRightsConsume;
	MSG_DRM_TYPE eDRMType; //to have a drm type
	char *szOpenedDRMFileName;
} MSG_OPENEDDRM_INFO_T;


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

	if (!drm_svc_is_drm_file(pBuffer)) {	// Check whether DRM file or not
		MSG_DEBUG("file is not drm file");
		return false;
	}

	DRM_RESULT eDRMResult = drm_svc_register_file(pBuffer); // Register a DCF file
	if (DRM_RESULT_SUCCESS != eDRMResult) {
		MSG_DEBUG("drm_svc_register_file is failed : %d", eDRMResult);
		return false;
	}

	return true;
}

bool MsgDrmUnregisterFile(char *szFilename)
{
	if (szFilename == NULL) {
		MSG_DEBUG("[Error] szFilename is NULL");
		return false;
	}

	MSG_DEBUG("szFilename = %s", szFilename);

	DRM_RESULT eDRMResult = drm_svc_unregister_file(szFilename, DRM_TRUE); // Unregister a DCF file
	if (DRM_RESULT_SUCCESS != eDRMResult) {
		MSG_DEBUG("drm_svc_unregister_file : %d", eDRMResult);
		return false;
	}

	return true;
}

bool MsgDrmIsDrmFile(const char *szFilePath)
{
	if (drm_svc_is_drm_file(szFilePath) == DRM_FALSE) {
		MSG_DEBUG("file is not drm file");
		return false;
	}

	return true;
}

/*Added to convert the .dm files in to .dcf files since our platform supports only .dcf :: Start*/
bool MsgDrmConvertDmtoDcfType(char *inputFile, char *outputFile)
{
	if ((NULL == inputFile) || (NULL == outputFile)) {
		MSG_DEBUG("Invalid Input parameters");
		return false;
	}

	if (strstr(inputFile, ".dm")) {
		MSG_DEBUG("Current File extension is .dm %s", inputFile);
		DRM_RESULT ret;
		DRM_CONVERT_HANDLE hConvert = NULL;
		unsigned long written;

		FILE *fp = MsgOpenFile(inputFile, "rb");//Check fp

		if (fp == NULL) {
			MSG_DEBUG("[File Open Fail(Errno=%d)][ErrStr=%s]", errno, strerror(errno));
			strncpy(outputFile, inputFile, MSG_MAX_DRM_FILE_PATH);
			return false;
		}

		if (MsgFseek(fp, 0L, SEEK_END) < 0) {
			MsgCloseFile(fp);
			MSG_DEBUG("MsgFseek() returns negative value!!!");
			return false;
		}
		long retVal = MsgFtell(fp);

		if (retVal < 0) {
			MsgCloseFile(fp);
			MSG_DEBUG("ftell() returns negative value: [%ld]!!!", retVal);
			strncpy(outputFile, inputFile, MSG_MAX_DRM_FILE_PATH);
			return false;
		}

		unsigned long bufLen = retVal;
		MSG_DEBUG("fopen buffer len = %d", bufLen);
		if (MsgFseek(fp, 0, SEEK_SET) < 0) {
			MsgCloseFile(fp);
			MSG_DEBUG("MsgFseek() returns negative value!!!");
			return false;
		}

		char *buffer = (char *)malloc(bufLen);
		int readed_size = 0;
		int pathLen = strlen(inputFile);

		if (buffer == NULL) {
			MsgCloseFile(fp);
			MSG_DEBUG("malloc is failed ");
			strncpy(outputFile, inputFile, MSG_MAX_DRM_FILE_PATH);
			return false;
		}

		strncpy(outputFile, inputFile, pathLen - 2);
		strncat(outputFile, "dcf", 3);

		readed_size = MsgReadFile(buffer, 1, bufLen, fp);//Check for error
		MSG_DEBUG("fread read size = %d", readed_size);
		if (readed_size == 0) {
			MsgCloseFile(fp);
			free(buffer);
			MSG_DEBUG("MsgReadFile returns 0");
			return false;
		}

		ret = drm_svc_open_convert(outputFile, DRM_TRUE, &hConvert);//Check return value
		if (ret != DRM_RESULT_SUCCESS) {
			free(buffer);
			MsgCloseFile(fp);
			MSG_DEBUG("drm_svc_open_convert() return = failed (%d)", ret);
			strncpy(outputFile, inputFile, MSG_MAX_DRM_FILE_PATH);
			return false;
		}

		/*We can call drm_svc_write_convert in loop if file size is large*/
		ret = drm_svc_write_convert(hConvert, (unsigned char *)buffer, bufLen, &written);//check for error
		if (ret != DRM_RESULT_SUCCESS) {
			free(buffer);
			MsgCloseFile(fp);
			MSG_DEBUG("drm_svc_write_convert() return = failed (%d)", ret);
			strncpy(outputFile, inputFile, MSG_MAX_DRM_FILE_PATH);
			return false;
		}

		ret = drm_svc_close_convert(hConvert);//check for error
		if (ret != DRM_RESULT_SUCCESS) {
			free(buffer);
			MsgCloseFile(fp);
			MSG_DEBUG("drm_svc_close_convert() return = failed (%d)", ret);
			strncpy(outputFile, inputFile, MSG_MAX_DRM_FILE_PATH);
			return false;
		}

		MsgCloseFile(fp);
		free(buffer);
	} else {
		MSG_DEBUG("Current File extension is not .dm");

		MSG_DEBUG("inputFile = (%s)", inputFile);
		strncpy(outputFile, inputFile, MSG_MAX_DRM_FILE_PATH);

		return false;
	}

	return true;
}

bool MsgDrmIsConvertedFL(char *szFilePath)
{
	int ret = 0;

	if (szFilePath == NULL) {
		MSG_DEBUG("szFilePath is NULL.");
		return false;
	}

	MSG_DEBUG("szFilePath = [%s]", szFilePath);

	ret = drm_svc_is_converted_fl(szFilePath);

	if (ret != DRM_RESULT_SUCCESS) {
		MSG_DEBUG("Drm2IsConvertedEmbeddedFile returns false ret = %d", ret);
		return false;
	}

	return true;
}

int MsgDrmGetStreamSize(MSG_DRMHANDLE pHandle)
{ // Get DRM buffer size
	MSG_DEBUG("start");
	MSG_OPENEDDRM_INFO_T *pOpenDRMInfo = (MSG_OPENEDDRM_INFO_T *)pHandle;
	if (pOpenDRMInfo == NULL) {
		MSG_DEBUG("end : Fail");
		return 0;
	}

	drm_file_attribute_t tDRMAttribute = {0,};
	DRM_RESULT eDRMResult = drm_svc_get_fileattribute(pOpenDRMInfo->szOpenedDRMFileName, &tDRMAttribute); // Get attribute of DRM File
	MSG_DEBUG("drm_svc_get_fileattribute : %d", eDRMResult);
	if (DRM_RESULT_SUCCESS == eDRMResult) {
		return tDRMAttribute.size;
	}
	MSG_DEBUG("end : Fail");
	return 0;
}

bool  MsgDrmGetStream(MSG_DRMHANDLE pHandle, int nStreamSize, unsigned char *pStream)
{ // Get DRM buffer
	MSG_DEBUG("start  %d", nStreamSize);
	MSG_OPENEDDRM_INFO_T *pOpenDRMInfo = (MSG_OPENEDDRM_INFO_T *)pHandle;
	if (pOpenDRMInfo == NULL) {
		MSG_DEBUG("end : Fail");
		return false;
	}

	DRM_RESULT eDRMResult = DRM_RESULT_UNKNOWN_ERROR;
	DRM_FILE_HANDLE hFileHandle = NULL;
	eDRMResult = drm_svc_open_file(pOpenDRMInfo->szOpenedDRMFileName, DRM_PERMISSION_ANY, &hFileHandle); // Opens a DRM file
	MSG_DEBUG("drm_svc_open_file(%s) : %d", pOpenDRMInfo->szOpenedDRMFileName, eDRMResult);

	unsigned int nRealReadSize = 0;
	if (hFileHandle) {
		eDRMResult = drm_svc_read_file(hFileHandle, pStream, nStreamSize, &nRealReadSize); // Read the Decrypted Data from File Handle
        drm_svc_close_file(hFileHandle); // Close a DRM File which was opened before
		if (DRM_RESULT_SUCCESS == eDRMResult) {
			MSG_DEBUG("end : Success");
			return true;
		}
	}
	MSG_DEBUG("\n end : Fail \n=========================================\n");
	return false;
}

bool MsgDrmOpen(MSG_DRM_OPENMODE eMode, const char *pBuffer, int nSize, MSG_DRMHANDLE *pHandle)
{
	MSG_DEBUG("start (%d, %s, %d)", eMode, pBuffer, nSize);
	if (eMode == MSG_MODE_STREAM) {
		MSG_DEBUG("end : Fail(eMode == MSG_MODE_STREAM)");
		return false;
	}

	char szFullFilePath[MSG_MAX_DRM_FILE_PATH] = {0,};
	char szFinalFullFilePath[MSG_MAX_DRM_FILE_PATH] = {0,};
	memset(szFullFilePath, 0x00, sizeof(char) * MSG_MAX_DRM_FILE_PATH);
	if (pBuffer)
		strncpy(szFullFilePath, pBuffer, strlen(pBuffer));

	MsgDrmConvertDmtoDcfType(szFullFilePath, szFinalFullFilePath);

	if (!drm_svc_is_drm_file(szFinalFullFilePath)) {	// Check whether DRM file or not
		MSG_DEBUG("return eDRM_NOT_DRM_FILE");
		*pHandle = NULL;
		return false;
	}

	MSG_OPENEDDRM_INFO_T *pOpenDRMInfo = (MSG_OPENEDDRM_INFO_T *)malloc(sizeof(MSG_OPENEDDRM_INFO_T));
	if (pOpenDRMInfo) {
		memset(pOpenDRMInfo, 0x0, sizeof(MSG_OPENEDDRM_INFO_T));
		pOpenDRMInfo->szOpenedDRMFileName = (char *)malloc(sizeof(char) * (strlen(szFinalFullFilePath) + 1));
		if (pOpenDRMInfo->szOpenedDRMFileName) {
			memset(pOpenDRMInfo->szOpenedDRMFileName, 0x0, sizeof(char) * (strlen(szFinalFullFilePath) + 1));
			strncpy(pOpenDRMInfo->szOpenedDRMFileName, szFinalFullFilePath, strlen(szFinalFullFilePath));
			pOpenDRMInfo->eDRMType = MSG_DRM_NONE;
			*pHandle = (MSG_DRMHANDLE)pOpenDRMInfo;
			MSG_DEBUG("end : Success");

			return true;
		}
		free(pOpenDRMInfo);
	}
	MSG_DEBUG("end : Fail");
	return false;
}

bool  MsgDrmClose(MSG_DRMHANDLE pHandle)
{ // Close DRM
	MSG_DEBUG("start");
	MSG_OPENEDDRM_INFO_T *pOpenDRMInfo = (MSG_OPENEDDRM_INFO_T *)pHandle;
	if (pOpenDRMInfo == NULL) {
		MSG_DEBUG("end : Fail");
		return false;
	}
	//free allocated memory from MsgDrmOpen
	if (pOpenDRMInfo->szOpenedDRMFileName) {
		free(pOpenDRMInfo->szOpenedDRMFileName);
		pOpenDRMInfo->szOpenedDRMFileName = NULL;
	}

	free(pOpenDRMInfo);

	MSG_DEBUG("end : Success");
	return true;
}

bool MsgDrmGetMimeType(MSG_DRMHANDLE pHandle,  char *szMimeType, int nMimeTypeLen)
{
	MSG_DEBUG("start");
	MSG_OPENEDDRM_INFO_T *pOpenDRMInfo = (MSG_OPENEDDRM_INFO_T *)pHandle;
	if (pOpenDRMInfo == NULL) {
		MSG_DEBUG("end : Fail");
		return false;
	}

	drm_content_info_t tdcfContentinfo;
	memset(&tdcfContentinfo, 0x00, sizeof(drm_content_info_t));
	DRM_RESULT eDRMResult = drm_svc_get_content_info(pOpenDRMInfo->szOpenedDRMFileName, &tdcfContentinfo); // Get attribute of DRM File
	MSG_DEBUG("drm_svc_get_content_info : %d", eDRMResult);
	if (DRM_RESULT_SUCCESS == eDRMResult) {
		MSG_DEBUG("end Success (%s)", tdcfContentinfo.contentType);
		snprintf(szMimeType, nMimeTypeLen, "%s", tdcfContentinfo.contentType);
		return true;
	}
	MSG_DEBUG("end : Fail");
	return false;
}

bool MsgDrmGetDrmType(const char *szFileName, MSG_DRM_TYPE *eDRMType)
{
	if (szFileName == NULL || eDRMType == NULL) {
		MSG_DEBUG("Param is NULL");
		return false;
	}

	if (drm_svc_get_drm_type(szFileName) == DRM_FILE_TYPE_OMA) {
		drm_dcf_info_t drmInfo;
		DRM_RESULT eDRMResult = drm_svc_get_dcf_file_info(szFileName, &drmInfo); // Get information of DRM contents
		if (DRM_RESULT_SUCCESS != eDRMResult) {
			MSG_DEBUG("drm_svc_get_dcf_file_info is Fail eDRMResult = %d", eDRMResult);
			return false;
		}

		// Convert DRM_METHOD into MSG_DRM_TYPE
		switch (drmInfo.method) {
		case DRM_METHOD_FL:
			*eDRMType = MSG_DRM_FORWARD_LOCK;
			break;
		case DRM_METHOD_CD:
			*eDRMType = MSG_DRM_COMBINED_DELIVERY;
			break;
		case DRM_METHOD_SSD:
			*eDRMType = MSG_DRM_SEPARATE_DELIVERY;
			break;
		case DRM_METHOD_SD:
			*eDRMType = MSG_DRM_SEPARATE_DELIVERY;
			break;
		default:
			*eDRMType = MSG_DRM_NONE;
			break;
		}
		MSG_DEBUG("eDRMType : %d", *eDRMType);
	} else {
		MSG_DEBUG("This is not a DRM_FILE_TYPE_OMA type");
		return false;
	}

	return true;
}

bool MsgDrmConsumeRights(MSG_DRMHANDLE pHandle, MSG_DRM_RIGHT_TYPE eRightType, long nMiliSecs)
{
	MSG_DEBUG("start %d, %ld", eRightType, nMiliSecs);
	MSG_DEBUG("end : Success");
	return true;
}

bool MsgDrmIsAvailable(MSG_DRMHANDLE pHandle)
{
	MSG_DEBUG("start");
	MSG_OPENEDDRM_INFO_T *pOpenDRMInfo = (MSG_OPENEDDRM_INFO_T *)pHandle;
	if (pOpenDRMInfo == NULL) {
		MSG_DEBUG("end : Fail");
		return false;
	}

	if (pOpenDRMInfo->eDRMType == MSG_DRM_FORWARD_LOCK) {	//fl 은 ro 없이도 사용가능
		MSG_DEBUG("end : Success");
		return true;
	}

	DRM_PERMISSION_TYPE ePerType = DRM_PERMISSION_ANY;
	drm_best_rights_t tBestRight; //ro 있는지 여부만 알면 되므로 drm_svc_get_best_ro 로 체크하도록 수정
	memset(&tBestRight, 0x00, sizeof(drm_best_rights_t));
	bool eReturn = false;
	DRM_RESULT eDRMResult = drm_svc_get_best_ro(pOpenDRMInfo->szOpenedDRMFileName, ePerType, &tBestRight); // Check the valided Rights or not by DCF file
	MSG_DEBUG("drm_svc_get_best_ro : %d", eDRMResult);
	if (DRM_RESULT_SUCCESS == eDRMResult && tBestRight.rightStatus == DRM_RIGHT_VALID) {
		eReturn = true;
		MSG_DEBUG("end : Success");
	} else {
		MSG_DEBUG("end : Fail");
	}
	return eReturn;
}

bool MsgDrmOnStart(MSG_DRMHANDLE pHandle, MSG_DRM_RIGHT_TYPE eRightType)
{ // Start consume
	MSG_DEBUG("start %d", eRightType);
	MSG_OPENEDDRM_INFO_T *pOpenDRMInfo = (MSG_OPENEDDRM_INFO_T *)pHandle;
	if (pOpenDRMInfo == NULL) {
		MSG_DEBUG("end : Fail");
		return false;
	}

	if (pOpenDRMInfo->eDRMType == MSG_DRM_FORWARD_LOCK) {
		MSG_DEBUG("end : Success");
		return true;
	}

	DRM_PERMISSION_TYPE eDRMUsage = DRM_PERMISSION_ANY;
	DRM_RIGHTS_CONSUME_HANDLE hRightsConsume = 0;
	switch (eRightType) {
	case MMS_DRM_RIGHT_PLAY:
		eDRMUsage = DRM_PERMISSION_PLAY;
		break;
	case MSG_DRM_RIGHT_DISPLAY:
		eDRMUsage = DRM_PERMISSION_DISPLAY;
		break;
	case MSG_DRM_RIGHT_EXECUTE:
		eDRMUsage = DRM_PERMISSION_EXECUTE;
		break;
	case MSG_DRM_RIGHT_PRINT:
		eDRMUsage = DRM_PERMISSION_PRINT;
		break;
	case MSG_DRM_RIGHT_EXPORT:
		eDRMUsage = DRM_PERMISSION_EXPORT_MOVE; // DRM_PERMISSION_EXPORT_COPY 도 사용해야함
		break;
	}
	DRM_RESULT eDRMResult = drm_svc_open_consumption(pOpenDRMInfo->szOpenedDRMFileName, eDRMUsage, &hRightsConsume); // Open for consumption
	MSG_DEBUG("drm_svc_open_consumption : %d", eDRMResult);
	if (DRM_RESULT_SUCCESS != eDRMResult) {
		MSG_DEBUG("end : Fail");
		return false;
	}
	pOpenDRMInfo->hRightsConsume = hRightsConsume;
	eDRMResult = drm_svc_start_consumption(pOpenDRMInfo->hRightsConsume); // Start the consumption
	MSG_DEBUG("drm_svc_start_consumption : %d", eDRMResult);
	if (DRM_RESULT_SUCCESS == eDRMResult) {
		MSG_DEBUG("end : Success");
		return true;
	}
	MSG_DEBUG("end : Fail");
	return false;
}

bool MsgDrmOnPause(MSG_DRMHANDLE pHandle, MSG_DRM_RIGHT_TYPE eRightType)
{
	MSG_DEBUG("start %d", eRightType);
	MSG_OPENEDDRM_INFO_T *pOpenDRMInfo = (MSG_OPENEDDRM_INFO_T *)pHandle;
	if (pOpenDRMInfo == NULL) {
		MSG_DEBUG("end : Fail");
		return false;
	}

	if (pOpenDRMInfo->eDRMType == MSG_DRM_FORWARD_LOCK) {
		MSG_DEBUG("end : Success");
		return true;
	}

	DRM_RESULT eDRMResult = drm_svc_pause_consumption(pOpenDRMInfo->hRightsConsume); // Pause the consumption
	MSG_DEBUG("drm_svc_pause_consumption : %d", eDRMResult);
	if (DRM_RESULT_SUCCESS == eDRMResult) {
		MSG_DEBUG("end : Success");
		return true;
	}
	MSG_DEBUG("end : Fail");
	return false;
}

bool MsgDrmOnResume(MSG_DRMHANDLE pHandle, MSG_DRM_RIGHT_TYPE eRightType)
{
	MSG_DEBUG("start %d", eRightType);
	MSG_DEBUG("end : Success");
	return true;
}

bool MsgDrmOnStop(MSG_DRMHANDLE pHandle, MSG_DRM_RIGHT_TYPE eRightType)
{
	MSG_DEBUG("start %d", eRightType);
	MSG_OPENEDDRM_INFO_T *pOpenDRMInfo = (MSG_OPENEDDRM_INFO_T *)pHandle;
	if (pOpenDRMInfo == NULL) {
		MSG_DEBUG("end : Fail");
		return false;
	}

	if (pOpenDRMInfo->eDRMType == MSG_DRM_FORWARD_LOCK) {
		MSG_DEBUG("end : Success");
		return true;
	}

	DRM_RESULT eDRMResult = drm_svc_stop_consumption(pOpenDRMInfo->hRightsConsume); // Stop the consumption
	MSG_DEBUG("drm_svc_stop_consumption : %d", eDRMResult);
	if (DRM_RESULT_SUCCESS != eDRMResult) {
		MSG_DEBUG("end : Fail");
		return false;
	}

	eDRMResult = drm_svc_close_consumption(&(pOpenDRMInfo->hRightsConsume)); // Close the consumption
	MSG_DEBUG("drm_svc_close_consumption : %d", eDRMResult);
	if (DRM_RESULT_SUCCESS == eDRMResult) {
		pOpenDRMInfo->hRightsConsume = 0;
		MSG_DEBUG("end : Success");
		return true;
	}
	MSG_DEBUG("end : Fail");
	return false;
}

bool MsgDrmGetMimeTypeEx(const char *szFileName, char *szMimeType, int nMimeTypeLen)
{
	if (!szFileName || !szMimeType || !nMimeTypeLen) {
		MSG_DEBUG("param is NULL");
		return false;
	}

	char strTemp[MSG_MAX_DRM_FILE_PATH + 1] = {0,};

	strncpy(strTemp, szFileName, strlen(szFileName));

	drm_content_info_t tdcfContentinfo;
	memset(&tdcfContentinfo, 0x00, sizeof(drm_content_info_t));
	DRM_RESULT eDRMResult = drm_svc_get_content_info(strTemp, &tdcfContentinfo); // Get attribute of DRM File
	if (DRM_RESULT_SUCCESS == eDRMResult) {
		MSG_DEBUG("contentType = %s", tdcfContentinfo.contentType);
		snprintf(szMimeType, nMimeTypeLen, "%s", tdcfContentinfo.contentType);

		return true;
	} else {
		MSG_DEBUG("drm_svc_get_content_info is failed %d", eDRMResult);
	}

	return false;
}

bool MsgDrmGetContentID(const char *szFileName, char *szContentID, int nContentIDLen)
{
	if (!szFileName || !szContentID || !nContentIDLen) {
		MSG_DEBUG("param is NULL");
		return false;
	}

	char strTemp[MSG_MAX_DRM_FILE_PATH + 1] = {0,};

	strncpy(strTemp, szFileName, sizeof(strTemp));

	drm_content_info_t  tdcfContentinfo;
	memset(&tdcfContentinfo, 0x00, sizeof(drm_content_info_t));
	DRM_RESULT eDRMResult = drm_svc_get_content_info(strTemp, &tdcfContentinfo);
	if (DRM_RESULT_SUCCESS == eDRMResult) {
		MSG_DEBUG("contentID = %s", tdcfContentinfo.contentID);
        snprintf(szContentID, nContentIDLen, "%s", tdcfContentinfo.contentID);

		return true;
	} else {
		MSG_DEBUG("drm_svc_get_content_info is failed %d", eDRMResult);
	}

	return false;
}

