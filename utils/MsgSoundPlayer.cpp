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

#include <wait.h>

#include "MsgHelper.h"
#include "MsgDebug.h"
#include "MsgGconfWrapper.h"
#include "MsgSettingTypes.h"
#include "MsgSoundPlayer.h"

#include <alarm.h>

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
static bool g_bRepeat = false;
static alarm_id_t g_alarmId = 0;


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/


void MsgSoundPlayStart()
{
	MSG_BEGIN();

	// Run helper App
	pid_t childpid;
	childpid = fork();

	if (childpid == -1)
	{
		MSG_DEBUG("Failed to fork");
	}

	if (childpid == 0)
	{
		MSG_DEBUG("Child Process - Run helper app for Sound");

		execl("/usr/bin/msg-helper", MSG_SOUND_START, NULL);

		MSG_DEBUG("Faild to run helper app for Sound");

		exit(0);
	}
	else if (childpid != 0)
	{
		MSG_DEBUG("Parent Process - Mms Plugin Storage.");
	}

	if (g_bRepeat == false)
		g_bRepeat = MsgSoundSetRepeatAlarm();

	MSG_END();
}

void MsgSoundPlayStop()
{
	MSG_BEGIN();

	// Run helper App
	pid_t childpid;
	childpid = fork();

	if (childpid == -1)
	{
		MSG_DEBUG("Failed to fork");
	}

	if (childpid == 0)
	{
		MSG_DEBUG("Child Process - Run helper app for Sound");

		execl("/usr/bin/msg-helper", MSG_SOUND_STOP, NULL);

		MSG_DEBUG("Faild to run helper app for Sound");

		exit(0);
	}
	else if (childpid != 0)
	{
		MSG_DEBUG("Parent Process - Mms Plugin Storage.");
	}

	MSG_END();
}


bool MsgSoundSetRepeatAlarm()
{
	bool bRet = false;

	int nRepeatValue = 0;
	long	nRepeatTime = 0;

	nRepeatValue = MsgSettingGetInt(MSG_ALERT_TONE);

	switch (nRepeatValue)
	{
		case MSG_ALERT_TONE_ONCE:
			nRepeatTime = 0;
		break;

		case MSG_ALERT_TONE_2MINS:
			nRepeatTime = 2;
		break;

		case MSG_ALERT_TONE_5MINS:
			nRepeatTime = 5;
		break;

		case MSG_ALERT_TONE_10MINS:
			nRepeatTime = 10;
		break;

		default:
			MSG_DEBUG("Invalid Repetition time");
		break;
	}

	MSG_DEBUG("nRepeatTime = %d", nRepeatTime);

	if (nRepeatTime > 0)
	{
		bRet = MsgSoundCreateRepeatAlarm(nRepeatTime);
	}

	return bRet;
}


bool MsgSoundCreateRepeatAlarm(int RepeatTime)
{
	MSG_BEGIN();

	time_t current_time;
	struct tm current_tm;

	time(&current_time);

	localtime_r(&current_time, &current_tm);

	int retval =0;

	retval = alarmmgr_init("msg-service-tools");

	if (retval != 0)
	{
		MSG_FATAL("alarmmgr_init() error [%d]", retval);
		return false;
	}

	alarm_info_t* alarm_info;
	alarm_date_t target_time;

	alarm_info = alarmmgr_create_alarm();

	target_time.year = 0;
	target_time.month = 0;
	target_time.day = 0;

	if (current_tm.tm_min+RepeatTime < 60)
	{
		target_time.hour = current_tm.tm_hour;
		target_time.min = current_tm.tm_min+RepeatTime;
	}
	else
	{
		if (current_tm.tm_hour < 12)
		{
			target_time.hour = current_tm.tm_hour+1;
		}
		else
		{
			target_time.hour = (current_tm.tm_hour+1)%12;
		}

		target_time.min = (current_tm.tm_min+RepeatTime)%60;
	}

	target_time.sec = current_tm.tm_sec;

	alarmmgr_set_time(alarm_info, target_time);
	alarmmgr_set_repeat_mode(alarm_info, ALARM_REPEAT_MODE_ONCE, 0);
	alarmmgr_set_type(alarm_info, ALARM_TYPE_VOLATILE);
	alarmmgr_add_alarm_with_localtime(alarm_info, NULL, &g_alarmId);

	retval = alarmmgr_set_cb(MsgSoundRepeatAlarmCB, NULL);

	if (retval != 0)
	{
		MSG_DEBUG("alarmmgr_set_cb() error [%d]", retval);
		return false;
	}

	MSG_DEBUG("Repeat Alarm Time : [%d-%d-%d %d:%d:%d]",
		target_time.year,target_time.month,target_time.day,
		target_time.hour, target_time.min, target_time.sec);

	MSG_END();

	return true;
}


int MsgSoundRepeatAlarmCB(int TimerId, void *pUserParam)
{
	MSG_BEGIN();

	g_bRepeat = false;

	if (MsgSoundGetUnreadMsgCnt() <= 0)
	{
		MSG_DEBUG("no unread msg");

		return 0;
	}

	MsgSoundPlayStart();

	MSG_END();

	return 0;
}


int MsgSoundGetUnreadMsgCnt()
{
	int unreadCnt = 0;

	// Get SMS Count
	unreadCnt = MsgSettingGetInt(VCONFKEY_MESSAGE_RECV_SMS_STATE);

	// Get MMS Count
	unreadCnt += MsgSettingGetInt(VCONFKEY_MESSAGE_RECV_MMS_STATE);

	MSG_DEBUG("unread count : [%d]", unreadCnt);

	return unreadCnt;
}

void MsgSoundInitRepeatAlarm()
{
	MSG_BEGIN();

	int nRepeatValue = 0;
	long	nRepeatTime = 0;

	g_bRepeat = false;

	if (MsgSoundGetUnreadMsgCnt() <= 0) {
		MSG_DEBUG("no unread msg");
		return;
	}

	nRepeatValue = MsgSettingGetInt(MSG_ALERT_TONE);

	switch (nRepeatValue)
	{
		case MSG_ALERT_TONE_ONCE:
			nRepeatTime = 0;
		break;

		case MSG_ALERT_TONE_2MINS:
			nRepeatTime = 2;
		break;

		case MSG_ALERT_TONE_5MINS:
			nRepeatTime = 5;
		break;

		case MSG_ALERT_TONE_10MINS:
			nRepeatTime = 10;
		break;

		default:
			MSG_DEBUG("Invalid Repetition time");
		break;
	}

	MSG_DEBUG("nRepeatTime = %d", nRepeatTime);

	if (nRepeatTime > 0)
		MsgSoundPlayStart();

	MSG_END();
}
