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

#ifndef MSG_SENSOR_H
#define MSG_SENSOR_H


#include "MsgTypes.h"

/*==================================================================================================
					DEFINES
==================================================================================================*/

typedef void (*msg_sensor_cb)();

/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/

msg_error_t MsgSensorConnect();
void MsgSensorDisconnect();
msg_error_t MsgRegSensorCB(msg_sensor_cb cb);

#endif // MSG_SENSOR_H
