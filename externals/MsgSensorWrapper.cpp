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


#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgGconfWrapper.h"
#include "MsgSensorWrapper.h"

#ifndef MSG_WEARABLE_PROFILE
#include <sensor_internal.h>

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/

int sensorHandler = -1;

msg_sensor_cb SensorCBFunction = NULL;

#endif // MSG_WEARABLE_PROFILE

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

#ifndef MSG_WEARABLE_PROFILE

void MsgSensorCB(unsigned int event_type, sensor_event_data_t *event_data , void *data)
{
	int *my_event_data;

	my_event_data = (int *)(event_data->event_data);

	if (event_type == MOTION_ENGINE_EVENT_TOP_TO_BOTTOM)
		if(*my_event_data == MOTION_ENGIEN_TOP_TO_BOTTOM_DETECTION) {
			MSG_DEBUG("top to bottom event detected.");
			if(MsgSettingGetInt(VCONFKEY_SETAPPL_MOTION_ACTIVATION))
				if(MsgSettingGetInt(VCONFKEY_SETAPPL_USE_TURN_OVER)) {
					if(SensorCBFunction) SensorCBFunction();
				}
		}
}

#endif // MSG_WEARABLE_PROFILE

msg_error_t MsgSensorConnect()
{

#ifndef MSG_WEARABLE_PROFILE

	sensorHandler = sf_connect(MOTION_SENSOR);
	if (sensorHandler < 0) {
		MSG_DEBUG("sensor attach fail.");
		return MSG_ERR_UNKNOWN;
	}

#endif // MSG_WEARABLE_PROFILE

	return MSG_SUCCESS;
}


void MsgSensorDisconnect()
{

#ifndef MSG_WEARABLE_PROFILE

	if(SensorCBFunction != NULL)
		SensorCBFunction = NULL;

	if (sensorHandler < 0)
		return;

	try
	{
		sf_stop(sensorHandler);
	}
	catch(int exception)
	{
		MSG_FATAL("sf_stop error[%d]", exception);
	}
	sf_disconnect(sensorHandler);

#endif // MSG_WEARABLE_PROFILE

}


msg_error_t MsgRegSensorCB(msg_sensor_cb cb)
{

#ifndef MSG_WEARABLE_PROFILE

	int resultCondition = -1;

	if (sensorHandler < 0) {
		MSG_DEBUG("Not connected to sensor FW.");
		return MSG_ERR_UNKNOWN;
	}

	if(cb != NULL) {
		// regist cb.
		SensorCBFunction = cb;
	} else {
		MSG_DEBUG("cb is NULL.");
		return MSG_ERR_UNKNOWN;
	}

	resultCondition = sf_register_event(sensorHandler, MOTION_ENGINE_EVENT_TOP_TO_BOTTOM , NULL , MsgSensorCB,NULL);
	if (resultCondition < 0) {
		MSG_DEBUG("SLP_sensor_register_cb fail to gather data.");
		return MSG_ERR_UNKNOWN;
	}

	MSG_DEBUG("Start SF.");
	resultCondition = sf_start(sensorHandler, 0);
	if (resultCondition < 0) {
		MSG_DEBUG("SLP_sensor_start fail.");
		return MSG_ERR_UNKNOWN;
	}

#endif // MSG_WEARABLE_PROFILE

	return MSG_SUCCESS;
}