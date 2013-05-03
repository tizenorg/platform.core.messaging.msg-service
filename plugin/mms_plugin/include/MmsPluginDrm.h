/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef MMS_PLUGIN_DRM_H
#define MMS_PLUGIN_DRM_H

#include "MsgMmsTypes.h"
#include "MmsPluginCodecTypes.h"

#ifdef __SUPPORT_DRM__

#define	MMS_DECODE_DRM_CONVERTED_TEMP_FILE MSG_DATA_PATH"Mms_Decode_Drm_Converted"
#define	MMS_MIMETYPELENGTH 50

typedef enum {
	MMS_DRM2_CONVERT_NONE,
	MMS_DRM2_CONVERT_NOT_FIXED,
	MMS_DRM2_CONVERT_REQUIRED,
	MMS_DRM2_CONVERT_FINISH
} MmsDrm2ConvertState;

MsgDrmType MsgGetDRMType(MsgType *pMsgType, MsgBody *pMsgBody);
void MmsDrm2SetConvertState(MmsDrm2ConvertState newConvertState);
MmsDrm2ConvertState MmsDrm2GetConvertState(void);
bool MsgDRM2GetDRMInfo(char *szFilePath, MsgType *pMsgType);
bool MsgDRMIsForwardLockType(MsgDrmType drmType);
bool MsgChangeDrm2FileName(char *szFileName);
bool MsgIsDCFFile(char *szFilePath);

#endif //__SUPPORT_DRM__

#endif //MMS_PLUGIN_DRM_H
