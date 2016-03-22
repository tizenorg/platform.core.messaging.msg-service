/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef MSG_GCONF_WRAPPER_H
#define MSG_GCONF_WRAPPER_H

/* #define USE_GCONF */
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


typedef void (*_vconf_change_cb)(keynode_t *key, void* data);

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
msg_error_t 	MsgSettingSetString(const char *pKey, const char *pSetValue);
msg_error_t 	MsgSettingSetInt(const char *pKey, int nSetValue);
msg_error_t 	MsgSettingSetBool(const char *pKey, bool bSetValue);

msg_error_t 	MsgSettingGetString(const char *pKey, char **pVal);
msg_error_t		MsgSettingGetInt(const char *pKey, int *pVal);
msg_error_t 	MsgSettingGetBool(const char *pKey, bool *pVal);

msg_error_t 	MsgSettingHandleNewMsg(int SmsCnt, int MmsCnt);
msg_error_t 	MsgSettingSetIndicator(int SmsCnt, int MmsCnt);

int	MsgSettingGetAutoReject();
bool	MsgSettingGetUnknownAutoReject();

msg_error_t MsgSettingRegVconfCBCommon(const char *pKey, _vconf_change_cb pCb);
msg_error_t MsgSettingRemoveVconfCBCommon(const char *pKey, _vconf_change_cb pCb);

#endif /* MSG_GCONF_WRAPPER_H */
