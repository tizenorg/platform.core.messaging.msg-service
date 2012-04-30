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

#ifndef __MSG_DRM_WRAPPER_H_
#define __MSG_DRM_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void* MSG_DRMHANDLE;

typedef enum {
	MMS_DRM_RIGHT_PLAY,								//video
	MSG_DRM_RIGHT_DISPLAY,							//image
	MSG_DRM_RIGHT_EXECUTE,							//game
	MSG_DRM_RIGHT_PRINT,
	MSG_DRM_RIGHT_EXPORT,
} MSG_DRM_RIGHT_TYPE;

typedef enum {
	MSG_MODE_FILE,
	MSG_MODE_STREAM
} MSG_DRM_OPENMODE;

typedef	enum {
	MSG_DRM_NONE = 0,
	MSG_DRM_FORWARD_LOCK,
	MSG_DRM_COMBINED_DELIVERY,
	MSG_DRM_SEPARATE_DELIVERY
} MSG_DRM_TYPE;


bool MsgDrmRegisterFile(MSG_DRM_OPENMODE eMode, char *pBuffer, int nSize);
bool MsgDrmUnregisterFile(char *szFilename);

bool MsgDrmIsDrmFile(const char *szFilePath);

bool MsgDrmGetMimeTypeEx(const char *szFileName, char *szMimeType, int nMimeTypeLen);
bool MsgDrmGetContentID(const char *szFileName, char *szContentID, int nContentIDLen);
bool MsgDrmGetDrmType(const char *szFileName, MSG_DRM_TYPE *eDRMType);


bool MsgDrmOpen(MSG_DRM_OPENMODE eMode, const char *pBuffer, int nSize, MSG_DRMHANDLE *pHandle);
bool MsgDrmClose(MSG_DRMHANDLE);
bool MsgDrmGetMimeType(MSG_DRMHANDLE pHandle, char *szMimeType, int nMimeTypeLen);
int MsgDrmGetStreamSize(MSG_DRMHANDLE pHandle);
bool MsgDrmGetStream(MSG_DRMHANDLE pHandle, int nStreamSize, unsigned char *pStream);

bool MsgDrmIsAvailable(MSG_DRMHANDLE pHandle);
bool MsgDrmConsumeRights(MSG_DRMHANDLE pHandle, MSG_DRM_RIGHT_TYPE eRightType, long nMiliSecs);
bool MsgDrmOnStart(MSG_DRMHANDLE pHandle, MSG_DRM_RIGHT_TYPE eRightType);
bool MsgDrmOnPause(MSG_DRMHANDLE pHandle, MSG_DRM_RIGHT_TYPE eRightType);
bool MsgDrmOnResume(MSG_DRMHANDLE pHandle, MSG_DRM_RIGHT_TYPE eRightType);
bool MsgDrmOnStop(MSG_DRMHANDLE pHandle, MSG_DRM_RIGHT_TYPE eRightType);


bool MsgDrmConvertDmtoDcfType(char *inputFile, char *outputFile);
bool MsgDrmIsConvertedFL(char *szFilePath);
#ifdef __cplusplus
}
#endif

#endif

