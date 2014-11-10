/*
 * msg-service
 *
 * Copyright (c) 2000 - 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef __MSG_DRM_WRAPPER_H_
#define __MSG_DRM_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

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
bool MsgDrmConvertDmtoDcfType(char *inputFile, char *outputFile);

#ifdef __cplusplus
}
#endif

#endif

