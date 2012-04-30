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

#ifndef MSG_GCONF_WRAPPER_H
#define MSG_GCONF_WRAPPER_H

//#define USE_GCONF
/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgInternalTypes.h"

#ifdef USE_GCONF
#include <gconf/gconf-client.h>
#include <glib.h>
#else
#include <vconf.h>
#endif

/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/
#ifdef USE_GCONF
typedef struct _MSG_GOBJECT_CLIENT_S
{
	GObject* object;
}MSG_GOBJECT_CLIENT_S;
#endif


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
MSG_ERROR_T 	MsgSettingSetString(const char *pKey, const char *pSetValue);
MSG_ERROR_T 	MsgSettingSetInt(const char *pKey, int nSetValue);
MSG_ERROR_T 	MsgSettingSetBool(const char *pKey, bool bSetValue);

char* 			MsgSettingGetString(const char *pKey);
int 				MsgSettingGetInt(const char *pKey);
int 				MsgSettingGetBool(const char *pKey, bool *pVal);

MSG_ERROR_T 	MsgSettingHandleNewMsg(int SmsCnt, int MmsCnt);
MSG_ERROR_T 	MsgSettingSetIndicator(int SmsCnt, int MmsCnt);

#endif // MSG_GCONF_WRAPPER_H
