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

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>

#include <alarm.h>
#include <device/display.h>

#include <msg_storage.h>

#include <msg-manager-util.h>
#include <msg-manager-debug.h>
#include <msg-manager-notification.h>

/*==================================================================================================
                                     DEFINES
==================================================================================================*/
typedef std::map<int, msg_mgr_alarm_cb> callBackMap;

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
bool alarmInit = false;
callBackMap alarmCBMap;

extern cm_client_h cm_handle;

/*==================================================================================================
                            INTERNAL FUNCTION IMPLEMENTATION
==================================================================================================*/
int MsgMgrAlarmCB(int alarmId, void *pUserParam)
{
	MSG_MGR_DEBUG("MsgMgrAlarmCB is called. alarmId [%d]", alarmId);

	callBackMap::iterator it = alarmCBMap.find(alarmId);

	if (it == alarmCBMap.end()) {
		MSG_MGR_DEBUG("alarmId is not found.");
	} else {
		msg_mgr_alarm_cb alarmCBfunction = it->second;
		if (alarmCBfunction)
			alarmCBfunction(alarmId);

		alarmCBMap.erase(it);
	}

	return 0;
}


int MsgMgrAlarmInit()
{
	MSG_MGR_BEGIN();

	alarmCBMap.clear();

	int retval = alarmmgr_init("msg-service-tools");
	if (retval != ALARMMGR_RESULT_SUCCESS) {
		MSG_MGR_DEBUG("alarmmgr_init error [%d]", retval);
		return -1;
	}

	retval = alarmmgr_set_cb(MsgMgrAlarmCB, NULL);
	if (retval != ALARMMGR_RESULT_SUCCESS)
		MSG_MGR_DEBUG("alarmmgr_set_cb() [%d]", retval);

	alarmInit = true;

	MSG_MGR_END();
	return 0;
}

/*==================================================================================================
                            FUNCTION IMPLEMENTATION
==================================================================================================*/
int MsgMgrAlarmRegistration(struct tm *timeInfo, msg_mgr_alarm_cb userCB, int *alarmId)
{
	MSG_MGR_BEGIN();

	if (!alarmInit) {
		MSG_MGR_DEBUG("alarm manager is not initialized. Retry once.");
		MsgMgrAlarmInit();
		if (!alarmInit) {
			MSG_MGR_DEBUG("alarm manager is still not initialized. So return error.");
			return -1;
		}
	}

	if (timeInfo == NULL || alarmId == NULL) {
		MSG_MGR_DEBUG("(timeInfo == NULL || alarmId == NULL)");
		return -1;
	}

	*alarmId = 0;
	alarm_info_t *alarm_info = NULL;
	alarm_id_t alarm_id = -1;
	alarm_date_t target_time;

	int retval = ALARMMGR_RESULT_SUCCESS;

	alarm_info = alarmmgr_create_alarm();
	if (alarm_info == NULL) {
		MSG_MGR_DEBUG("alarmmgr_create_alarm error.");
		return -1;
	}

	target_time.year = timeInfo->tm_year + 1900;
	target_time.month = timeInfo->tm_mon + 1;
	target_time.day = timeInfo->tm_mday;
	target_time.hour = timeInfo->tm_hour;
	target_time.min = timeInfo->tm_min;
	target_time.sec = timeInfo->tm_sec;

	MSG_MGR_DEBUG("Alarm Time : [%d-%d-%d %d:%d:%d] ",
			target_time.year, target_time.month, target_time.day,
			target_time.hour, target_time.min, target_time.sec);

	retval = alarmmgr_set_time(alarm_info, target_time);
	if (retval != ALARMMGR_RESULT_SUCCESS)
		MSG_MGR_DEBUG("alarmmgr_set_time ret[%d]", retval);

	retval = alarmmgr_set_repeat_mode(alarm_info, ALARM_REPEAT_MODE_ONCE, 0);
	if (retval != ALARMMGR_RESULT_SUCCESS)
		MSG_MGR_DEBUG("alarmmgr_set_repeat_mode ret[%d]", retval);

	retval = alarmmgr_set_type(alarm_info, ALARM_TYPE_DEFAULT);
	if (retval != ALARMMGR_RESULT_SUCCESS)
		MSG_MGR_DEBUG("alarmmgr_set_type ret[%d]", retval);

	retval = alarmmgr_add_alarm_with_localtime(alarm_info, NULL, &alarm_id);
	MSG_MGR_DEBUG("alarmmgr_add_alarm_with_localtime ret[%d], alarm_id[%d]", retval, alarm_id);

	retval = alarmmgr_free_alarm(alarm_info);
	if (retval != ALARMMGR_RESULT_SUCCESS)
		MSG_MGR_DEBUG("alarmmgr_free_alarm ret[%d]", retval);

	*alarmId = (int)alarm_id;

	alarmCBMap[*alarmId] = userCB;

	MSG_MGR_END();
	return 0;
}


int MsgMgrAlarmRemove(int alarmId)
{
	MSG_MGR_BEGIN();

	if (!alarmInit) {
		MSG_MGR_DEBUG("alarm manager is not initialized. Retry once.");
		MsgMgrAlarmInit();
		if (!alarmInit) {
			MSG_MGR_DEBUG("alarm manager is still not initialized. So return error.");
			return -1;
		}
	}

	if (alarmmgr_remove_alarm(alarmId) != ALARMMGR_RESULT_SUCCESS) {
		MSG_MGR_DEBUG("alarmmgr_remove_alarm faild.");
		return -1;
	}

	MSG_MGR_END();
	return 0;
}


cm_call_status_e MsgMgrGetCallStatus()
{
	cm_call_status_e call_status = CM_CALL_STATUS_IDLE;
	cm_get_call_status(cm_handle, &call_status);

	return call_status;
}


void MsgMgrChangePmState()
{
	MSG_MGR_BEGIN();
	if (MsgMgrCheckNotificationSettingEnable() == false)
		return;

	cm_call_status_e callStatus = CM_CALL_STATUS_IDLE;

	callStatus = MsgMgrGetCallStatus();
	MSG_MGR_DEBUG("Call Status = %d", callStatus);

	if (callStatus > CM_CALL_STATUS_IDLE && callStatus < CM_CALL_STATUS_MAX) {
		MSG_MGR_DEBUG("Call is activated. Do not turn on the lcd.");
	} else {
		MSG_MGR_DEBUG("Call is not activated. Turn on the lcd.");
		device_display_change_state(DISPLAY_STATE_NORMAL);
	}

	MSG_MGR_END();
}
