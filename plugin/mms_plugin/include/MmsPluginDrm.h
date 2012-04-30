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

#include "MsgMmsTypes.h"
#include "MmsPluginMessage.h"

#ifdef __SUPPORT_DRM__

#define	MMS_DECODE_DRM_CONVERTED_TEMP_FILE MSG_DATA_PATH"Mms_Decode_Drm_Converted"
#define	MMS_MIMETYPELENGTH 50

typedef	enum {
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
#endif
