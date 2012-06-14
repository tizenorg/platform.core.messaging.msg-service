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

#include <pthread.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgSettingTypes.h"
#include "MsgGconfWrapper.h"
#include "MsgHelper.h"

#include <devman_haptic.h>
#include <svi.h>

#include <mm_error.h>
#include <mm_player.h>
#include <mm_session_private.h>


extern void worker_done();

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
static MMHandleType hPlayerHandle = 0;
static bool bPlaying = false;
static bool bVibrating = false;
static int dev_handle;

pthread_mutex_t muMmPlay = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvMmPlay = PTHREAD_COND_INITIALIZER;

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
static gboolean MsgSoundMelodyTimeout(gpointer data)
{
	MSG_BEGIN();

	MsgSoundPlayStop();
	if(!bPlaying && !bVibrating)
		worker_done();

	MSG_END();

	return FALSE;
}


static gboolean MsgSoundVibTimeout(gpointer data)
{
	MSG_BEGIN();

	int ret = 0;

	if (bVibrating == true) {
		ret = device_haptic_stop_play(dev_handle);

		if (ret != 0) {
			MSG_DEBUG("Fail to stop haptic : [%d]", ret);
		}

		ret = device_haptic_close(dev_handle);

		if (ret != 0) {
			MSG_DEBUG("Fail to close haptic : [%d]", ret);
		}

		bVibrating = false;
	}

	if(!bPlaying && !bVibrating)
		worker_done();

	MSG_END();

	return FALSE;
}


static int MsgSoundPlayCallback(int message, void *param, void *user_param)
{
	switch (message)
	{
		case MM_MESSAGE_ERROR:
			MSG_DEBUG("ERROR is happened.");
			MsgSoundPlayUninit();
			break;
		case MM_MESSAGE_BEGIN_OF_STREAM:
			MSG_DEBUG("Play is started.");
			break;
		case MM_MESSAGE_END_OF_STREAM:
			MSG_DEBUG("end of stream");
			MsgSoundPlayStop();
			if(!bPlaying && !bVibrating)
				worker_done();
			break;
		default:
			MSG_DEBUG("message = %d", message);
			break;
	}

	return 1;
}


void* MsgPlayThread(void *data)
{
	MSG_BEGIN();

	bool bSoundOn = false;
	bool bVibrationOn = false;
	int callStatus = 0;	/* 0 - off, 1 - sound, 2 - vibration */
	int alertOnCall = 0;

	char *msg_tone_file_path = NULL;
	AutoPtr<char> buf(&msg_tone_file_path);

	char *tmpFileFath = NULL;

	tmpFileFath = MsgSettingGetString(VCONFKEY_SETAPPL_NOTI_MSG_RINGTONE_PATH_STR);

	if (tmpFileFath == NULL) {
		msg_tone_file_path = new char[MAX_SOUND_FILE_LEN];
		strncpy(msg_tone_file_path, DEFAULT_FILE, MAX_SOUND_FILE_LEN-1);
	} else {
		msg_tone_file_path = new char[MAX_SOUND_FILE_LEN];
		strncpy(msg_tone_file_path, tmpFileFath, MAX_SOUND_FILE_LEN-1);
		free(tmpFileFath);
		tmpFileFath = NULL;
	}

	MSG_DEBUG("Sound File [%s]", msg_tone_file_path);

	MsgSettingGetBool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, &bSoundOn);
	MsgSettingGetBool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, &bVibrationOn);

	int err = MM_ERROR_NONE;

	err = mm_session_init(MM_SESSION_TYPE_NOTIFY);

	if(err != MM_ERROR_NONE)
		MSG_DEBUG("MM Session Init Failed");
	else
		MSG_DEBUG("MM Session Init Success : %d", err);

	hPlayerHandle = 0;

	err = mm_player_create(&hPlayerHandle);

	if (err != MM_ERROR_NONE) {
  		MSG_DEBUG("creating the player handle failed");
		return NULL;
	}

	/* Setting the call back function msg_callback */
	mm_player_set_message_callback(hPlayerHandle, MsgSoundPlayCallback, (void *)hPlayerHandle);

	callStatus = MsgSettingGetInt(VCONFKEY_CALL_STATE);
	MSG_DEBUG("Call Status = %d", callStatus);

	if (callStatus > VCONFKEY_CALL_OFF && callStatus < VCONFKEY_CALL_STATE_MAX) {
		alertOnCall = MsgSettingGetInt(VCONFKEY_CISSAPPL_ALERT_ON_CALL_INT);
		MSG_DEBUG("Alert On Call = %d", alertOnCall);

		if (alertOnCall == 0) {
			MSG_DEBUG("Call is active & Alert on Call - Off");
		} else if (alertOnCall == 1) {
			MSG_DEBUG("Call is active & Alert on Call - Sound");

			if (bSoundOn)
				MsgSoundPlayMelody(msg_tone_file_path, false);
		} else if (alertOnCall == 2) {
			MSG_DEBUG("Call is active & Alert on Call - Vibration");

			if (bVibrationOn)
				MsgSoundPlayVibration();
		}
	} else{
		MSG_DEBUG("Call is not active");

		if (bVibrationOn) {
			MSG_DEBUG("Play vibration.");
			MsgSoundPlayVibration();
		}

		if (bSoundOn) {
			MSG_DEBUG("Play sound.");
			MsgSoundPlayMelody(msg_tone_file_path, false);
		}
	}

	err = mm_session_finish();

	if (err != MM_ERROR_NONE)
		MSG_DEBUG("MM Session Finish Failed");
	else
		MSG_DEBUG("MM Session Finish Success : %d", err);

	if(!bPlaying && !bVibrating)
		worker_done();

	MSG_END();

	return NULL;
}


MSG_ERROR_T MsgSoundPlayUninit()
{
	MSG_BEGIN();

	int err = MM_ERROR_NONE;

	/* Uninitializing the player module */
	err = mm_player_unrealize(hPlayerHandle);

	/* Destroying the player handle */
	err = mm_player_destroy(hPlayerHandle);

	pthread_mutex_lock(&muMmPlay);

	bPlaying = false;

	pthread_mutex_unlock(&muMmPlay);

	pthread_cond_signal(&cvMmPlay);

	hPlayerHandle = 0;

	MSG_END();

	return MSG_SUCCESS;
}


void MsgSoundPlayStart()
{
	MSG_BEGIN();

	pthread_mutex_lock(&muMmPlay);

	if (bPlaying == true) {
		MSG_DEBUG("Ringtone is Playing...");
		pthread_mutex_unlock(&muMmPlay);
		return;
	}

	pthread_mutex_unlock(&muMmPlay);

	pthread_t tid;

	if (pthread_create(&tid, NULL, &MsgPlayThread, (void*)NULL) == 0) {
		MSG_DEBUG("Ring alert thread created = %d", tid);
	} else {
		MSG_DEBUG("Creating Thread was failed");
		return;
	}

	MSG_END();
}


void MsgSoundPlayStop()
{
	MSG_BEGIN();

	pthread_mutex_lock(&muMmPlay);

	if (bPlaying == false) {
		MSG_DEBUG("Ringtone is Not Playing...");
		pthread_mutex_unlock(&muMmPlay);
		return;
	}

	pthread_mutex_unlock(&muMmPlay);

	/* Stop playing media contents */
	MSG_DEBUG("Before mm_player_stop, %p", hPlayerHandle);

	int err = mm_player_stop(hPlayerHandle);

	MSG_DEBUG("After mm_player_stop");

	if (err != MM_ERROR_NONE) {
		MSG_DEBUG("stopping the player handle failed");
	}

	MsgSoundPlayUninit();

	MSG_END();
}


int MsgSoundPlayMelody(char *pMsgToneFilePath, bool bIncreasing)
{
	int err = MM_ERROR_NONE;

	/* Setting fade in,out */
	err = mm_player_set_attribute(hPlayerHandle, NULL, "sound_priority", 2, NULL);

	if (err != MM_ERROR_NONE) {
		MSG_DEBUG("error setting the profile attr");
		return err;
	}

	/* Setting the Volume */
	err = mm_player_set_attribute(hPlayerHandle, NULL, "sound_volume_type", MM_SOUND_VOLUME_TYPE_NOTIFICATION,
													"profile_uri", pMsgToneFilePath, strlen(pMsgToneFilePath), NULL);

	if (err != MM_ERROR_NONE) {
		MSG_DEBUG("error setting the profile attr");
		return err;
	}

	err = mm_player_realize(hPlayerHandle);

	if (err != MM_ERROR_NONE) {
		MSG_DEBUG("mm_player_realize() error : [%d]", err);
		return err;
	}

	/* Add Timer to stop playing after 5 sec. */
	int g_contact_timer = -1;
	g_contact_timer = g_timeout_add(5500, (GSourceFunc)MsgSoundMelodyTimeout, NULL);

	err = mm_player_start(hPlayerHandle);

	if (err != MM_ERROR_NONE) {
	 	MSG_DEBUG("mm_player_start, FAIL [%x]", err);
		bPlaying = false;

		return err;
	}

	bPlaying = true;

	pthread_mutex_lock(&muMmPlay);

	while (bPlaying)
	{
		MSG_DEBUG("Ring Alert Playing");
		pthread_cond_wait(&cvMmPlay, &muMmPlay);
	}

	pthread_mutex_unlock(&muMmPlay);

	MSG_DEBUG("Ring Alert Idle");

	return err;
}


void MsgSoundPlayVibration()
{
	MSG_BEGIN();

	int ret = 0;
	int vibLevel = 0;
	char ivtFilePath[MAX_SOUND_FILE_LEN] = {0,};

	vibLevel = MsgSettingGetInt(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT);

	if (vibLevel > 0) {
		bVibrating = true;

		dev_handle = device_haptic_open(DEV_IDX_0, 0);

		g_timeout_add(MSG_VIBRATION_INTERVAL , MsgSoundVibTimeout, NULL);

		/* set timer to stop vibration, then play melody */
		svi_get_path(SVI_TYPE_VIB, SVI_VIB_NOTIFICATION_MESSAGE, ivtFilePath, sizeof(ivtFilePath));
		ret = device_haptic_play_file(dev_handle, ivtFilePath, HAPTIC_TEST_ITERATION, vibLevel);

		if (ret != 0) {
			MSG_DEBUG("Fail to play haptic : [%d]", ret);
		}
	}

	MSG_END();
}


int MsgSoundGetUnreadMsgCnt()
{
	int unreadCnt = 0;

	/*  Get SMS Count */
	unreadCnt = MsgSettingGetInt(VCONFKEY_MESSAGE_RECV_SMS_STATE);

	/*  Get MMS Count */
	unreadCnt += MsgSettingGetInt(VCONFKEY_MESSAGE_RECV_MMS_STATE);

	MSG_DEBUG("unread count : [%d]", unreadCnt);

	return unreadCnt;
}
