/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved
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

#ifndef __MSG_MGR_ALARM_H__
#define __MSG_MGR_ALARM_H__

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <time.h>

#include <msg.h>
#include <alarm.h>

/*==================================================================================================
					DEFINES
==================================================================================================*/

typedef void (*msg_mgr_alarm_cb)(int alarmId);

/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/
int MsgMgrAlarmRegistration(struct tm *timeInfo, msg_mgr_alarm_cb userCB, int *alarmId);
int MsgMgrAlarmRemove(int alarmId);


#endif /*__MSG_MGR_ALARM_H__ */

