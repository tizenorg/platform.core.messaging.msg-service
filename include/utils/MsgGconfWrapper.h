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
msg_error_t 	MsgSettingSetString(const char *pKey, const char *pSetValue);
msg_error_t 	MsgSettingSetInt(const char *pKey, int nSetValue);
msg_error_t 	MsgSettingSetBool(const char *pKey, bool bSetValue);

char* 			MsgSettingGetString(const char *pKey);
int 				MsgSettingGetInt(const char *pKey);
int 				MsgSettingGetBool(const char *pKey, bool *pVal);

msg_error_t 	MsgSettingHandleNewMsg(int SmsCnt, int MmsCnt);
msg_error_t 	MsgSettingSetIndicator(int SmsCnt, int MmsCnt);

bool	MsgSettingGetAutoReject();
bool	MsgSettingGetUnknownAutoReject();

void	MsgSettingRegVconfCB();
void	MsgSettingRemoveVconfCB();

#endif // MSG_GCONF_WRAPPER_H
