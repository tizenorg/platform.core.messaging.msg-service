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

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgGconfWrapper.h"
#include "MsgHelper.h"

#include <sensor.h>

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
int sensorHandler = -1;

msg_sensor_cb cbFunction = NULL;

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

void MsgSensorCB(unsigned int event_type, sensor_event_data_t *event_data , void *data)
{
	int *my_event_data;

	my_event_data = (int *)(event_data->event_data);

	if (event_type == MOTION_ENGINE_EVENT_TOP_TO_BOTTOM)
		if(*my_event_data == MOTION_ENGIEN_TOP_TO_BOTTOM_DETECTION) {
			MSG_DEBUG("top to bottom event detected.");
			if(MsgSettingGetInt(VCONFKEY_SETAPPL_MOTION_ACTIVATION))
				if(MsgSettingGetInt(VCONFKEY_SETAPPL_USE_TURN_OVER))
					cbFunction();
		}
}


msg_error_t MsgSensorConnect()
{
	sensorHandler = sf_connect(MOTION_SENSOR);
	if (sensorHandler < 0) {
		MSG_DEBUG("sensor attach fail.");
		return MSG_ERR_UNKNOWN;
	}

	return MSG_SUCCESS;
}


void MsgSensorDisconnect()
{
	if(cbFunction != NULL)
		cbFunction = NULL;

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
}


msg_error_t MsgRegSensorCB(msg_sensor_cb cb)
{
	int resultCondition = -1;

	if (sensorHandler < 0) {
		MSG_DEBUG("Not connected to sensor FW.");
		return MSG_ERR_UNKNOWN;
	}

	if(cb != NULL) {
		// regist cb.
		cbFunction = cb;
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

	return MSG_SUCCESS;
}
