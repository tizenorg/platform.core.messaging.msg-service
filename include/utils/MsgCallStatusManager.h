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

#ifndef MSG_CALL_STATUS_MANAGER_H
#define MSG_CALL_STATUS_MANAGER_H

/*==================================================================================================
                                    INCLUDE FILES
==================================================================================================*/
#include <gio/gio.h>

/*==================================================================================================
                                    DEFINES
==================================================================================================*/
#define CALL_MGR_BUS_NAME "org.tizen.callmgr"
#define CALL_MGR_PATH_NAME "/org/tizen/callmgr"
#define CALL_MGR_INTERFACE_NAME "org.tizen.callmgr"
#define CALL_MGR_MEMBER_NAME "CallStatus"

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
void MsgInitCallStatusManager();
void MsgDeInitCallStatusManager();
int MsgGetCallStatus();

#endif // MSG_CALL_STATUS_MANAGER_H

