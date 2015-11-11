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
#include <gesture_recognition.h>

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/

gesture_h gestureHandler = NULL;

msg_sensor_cb SensorCBFunction = NULL;

#endif /* MSG_WEARABLE_PROFILE */

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

#ifndef MSG_WEARABLE_PROFILE
void MsgGestureCB(gesture_type_e gesture, const gesture_data_h data, double timestamp, gesture_error_e error, void *user_data)
{
	if (error != GESTURE_ERROR_NONE) {
		MSG_DEBUG("Gesture error = [%d]", error);
		return;
	}

	gesture_event_e event;
	int ret = gesture_get_event(data, &event);
	if (ret == GESTURE_ERROR_NONE && event == GESTURE_EVENT_DETECTED && \
			gesture == GESTURE_TURN_FACE_DOWN) {
		MSG_DEBUG("GESTURE_TURN_FACE_DOWN gesture detected.");
		if (MsgSettingGetInt(VCONFKEY_SETAPPL_MOTION_ACTIVATION)) {
			if (MsgSettingGetInt(VCONFKEY_SETAPPL_USE_TURN_OVER)) {
				if (SensorCBFunction)
					SensorCBFunction();
			}
		}
	}
}

#endif /* MSG_WEARABLE_PROFILE */

msg_error_t MsgSensorConnect()
{
#ifndef MSG_WEARABLE_PROFILE
	bool supported = false;
	int ret = gesture_is_supported(GESTURE_TURN_FACE_DOWN, &supported);
	if (ret != GESTURE_ERROR_NONE) {
		MSG_DEBUG("gesture_is_supported is failed [%d]", ret);
		return MSG_ERR_UNKNOWN;
	}
	if (!supported) {
		MSG_DEBUG("GESTURE_TURN_FACE_DOWN not supported in the current device.");
		return MSG_ERR_UNKNOWN;
	}

	ret = gesture_create(&gestureHandler);
	if (ret != GESTURE_ERROR_NONE) {
		MSG_DEBUG("gesture_create is failed [%d]", ret);
		return MSG_ERR_UNKNOWN;
	}
#endif /* MSG_WEARABLE_PROFILE */

	return MSG_SUCCESS;
}


void MsgSensorDisconnect()
{
#ifndef MSG_WEARABLE_PROFILE
	if (SensorCBFunction != NULL)
		SensorCBFunction = NULL;

	if (gestureHandler == NULL)
		return;

	try {
		gesture_stop_recognition(gestureHandler);
	} catch(int exception) {
		MSG_FATAL("gesture_stop_recognition error [%d]", exception);
	}

	gesture_release(gestureHandler);
	gestureHandler = NULL;
#endif /* MSG_WEARABLE_PROFILE */
}


msg_error_t MsgRegSensorCB(msg_sensor_cb cb)
{
#ifndef MSG_WEARABLE_PROFILE
	if (gestureHandler == NULL) {
		MSG_DEBUG("Not connected to gesture FW.");
		return MSG_ERR_UNKNOWN;
	}

	if (cb != NULL) {
		/* regist cb. */
		SensorCBFunction = cb;
	} else {
		MSG_DEBUG("cb is NULL.");
		return MSG_ERR_UNKNOWN;
	}

	int ret = gesture_start_recognition(gestureHandler, GESTURE_TURN_FACE_DOWN, \
				GESTURE_OPTION_DEFAULT, MsgGestureCB, NULL);
	if (ret != GESTURE_ERROR_NONE) {
		MSG_DEBUG("gesture_start_recognition failed");
		return MSG_ERR_UNKNOWN;
	}
#endif /* MSG_WEARABLE_PROFILE */

	return MSG_SUCCESS;
}
