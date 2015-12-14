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


#include <map>

#include "MsgDebug.h"
#include "MsgAlarm.h"

#include <alarm.h>


/*==================================================================================================
                                     DEFINES
==================================================================================================*/
typedef std::map<int, msg_alarm_cb> callBackMap;

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
bool alarmInit = false;
callBackMap alarmCBMap;

/*==================================================================================================
                            INTERNAL FUNCTION IMPLEMENTATION
==================================================================================================*/
int MsgAlarmCB(int alarmId, void *pUserParam)
{
	MSG_DEBUG("MsgAlarmCB is called. alarmId [%d]", alarmId);

	callBackMap::iterator it = alarmCBMap.find(alarmId);

	if (it == alarmCBMap.end()) {
		MSG_DEBUG("alarmId is not found.");
	} else {
		msg_alarm_cb alarmCBfunction = it->second;
		if (alarmCBfunction)
			alarmCBfunction(alarmId);

		alarmCBMap.erase(it);
	}

	return 0;
}


msg_error_t MsgAlarmInit()
{
	MSG_BEGIN();

	alarmCBMap.clear();

	int retval = alarmmgr_init("msg-service-tools");
	if (retval != ALARMMGR_RESULT_SUCCESS) {
		MSG_DEBUG("alarmmgr_init error [%d]", retval);
		return MSG_ERR_UNKNOWN;
	}

	retval = alarmmgr_set_cb(MsgAlarmCB, NULL);
	if (retval != ALARMMGR_RESULT_SUCCESS)
		MSG_DEBUG("alarmmgr_set_cb() [%d]", retval);

	alarmInit = true;

	MSG_END();
	return MSG_SUCCESS;
}

/*==================================================================================================
                            FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgAlarmRegistration(struct tm *timeInfo, msg_alarm_cb userCB, int *alarmId)
{
	MSG_BEGIN();

	if (!alarmInit) {
		MSG_DEBUG("alarm manager is not initialized. Retry once.");
		MsgAlarmInit();
		if (!alarmInit) {
			MSG_DEBUG("alarm manager is still not initialized. So return error.");
			return MSG_ERR_UNKNOWN;
		}
	}

	if (timeInfo == NULL || alarmId == NULL) {
		MSG_DEBUG("(timeInfo == NULL || alarmId == NULL)");
		return MSG_ERR_INVALID_PARAMETER;
	}

	*alarmId = 0;
	alarm_info_t *alarm_info = NULL;
	alarm_id_t alarm_id = -1;
	alarm_date_t target_time;

	int retval = ALARMMGR_RESULT_SUCCESS;

	alarm_info = alarmmgr_create_alarm();
	if (alarm_info == NULL) {
		MSG_DEBUG("alarmmgr_create_alarm error.");
		return MSG_ERR_UNKNOWN;
	}

	target_time.year = timeInfo->tm_year + 1900;
	target_time.month = timeInfo->tm_mon + 1;
	target_time.day = timeInfo->tm_mday;
	target_time.hour = timeInfo->tm_hour;
	target_time.min = timeInfo->tm_min;
	target_time.sec = timeInfo->tm_sec;

	MSG_DEBUG("Alarm Time : [%d-%d-%d %d:%d:%d] ",
			target_time.year, target_time.month, target_time.day,
			target_time.hour, target_time.min, target_time.sec);

	retval = alarmmgr_set_time(alarm_info, target_time);
	if (retval != ALARMMGR_RESULT_SUCCESS)
		MSG_DEBUG("alarmmgr_set_time ret[%d]", retval);

	retval = alarmmgr_set_repeat_mode(alarm_info, ALARM_REPEAT_MODE_ONCE, 0);
	if (retval != ALARMMGR_RESULT_SUCCESS)
		MSG_DEBUG("alarmmgr_set_repeat_mode ret[%d]", retval);

	retval = alarmmgr_set_type(alarm_info, ALARM_TYPE_DEFAULT);
	if (retval != ALARMMGR_RESULT_SUCCESS)
		MSG_DEBUG("alarmmgr_set_type ret[%d]", retval);

	retval = alarmmgr_add_alarm_with_localtime(alarm_info, NULL, &alarm_id);
	MSG_DEBUG("alarmmgr_add_alarm_with_localtime ret[%d], alarm_id[%d]", retval, alarm_id);

	retval = alarmmgr_free_alarm(alarm_info);
	if (retval != ALARMMGR_RESULT_SUCCESS)
		MSG_DEBUG("alarmmgr_free_alarm ret[%d]", retval);

	*alarmId = (int)alarm_id;

	alarmCBMap[*alarmId] = userCB;

	MSG_END();
	return MSG_SUCCESS;
}


msg_error_t MsgAlarmRemove(int alarmId)
{
	MSG_BEGIN();

	if (!alarmInit) {
		MSG_DEBUG("alarm manager is not initialized. Retry once.");
		MsgAlarmInit();
		if (!alarmInit) {
			MSG_DEBUG("alarm manager is still not initialized. So return error.");
			return MSG_ERR_UNKNOWN;
		}
	}

	if (alarmmgr_remove_alarm(alarmId) != ALARMMGR_RESULT_SUCCESS) {
		MSG_DEBUG("alarmmgr_remove_alarm faild.");
		return MSG_ERR_UNKNOWN;
	}

	MSG_END();
	return MSG_SUCCESS;
}
