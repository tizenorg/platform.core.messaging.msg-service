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

#include <wait.h>

#include "MsgCallStatusManager.h"
#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgGconfWrapper.h"
#include "MsgSettingTypes.h"
#include "MsgDrmWrapper.h"
#include "MsgSensorWrapper.h"
#include "MsgUtilFile.h"
#include "MsgStorageTypes.h"
#include "MsgContact.h"
#include "MsgAlarm.h"
#include "MsgNotificationWrapper.h"
#include "MsgSoundPlayer.h"

#ifndef MSG_WEARABLE_PROFILE

#include <player.h>
#include <sound_manager.h>
#include <feedback.h>
#include <feedback-internal.h>

/*==================================================================================================
                                    DEFINES
==================================================================================================*/
#define MAX_SOUND_FILE_LEN 1024

#define DEFAULT_ALERT_FILE		TZ_SYS_RO_APP_PATH "/" MSG_SETTING_APP_ID "shared/res/settings/Alerts/General notification_sdk.wav"

#define HAPTIC_TEST_ITERATION 1
#define MSG_VIBRATION_INTERVAL 3000
#define MSG_SOUND_TIMEOUT 5500


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
player_h g_PlayerHandle = NULL;
sound_stream_info_h g_stream_info = NULL;

pthread_mutex_t muMmPlay = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t muStream = PTHREAD_MUTEX_INITIALIZER;

#endif /* MSG_WEARABLE_PROFILE */

/*==================================================================================================
                                     IMPLEMENTATION OF Functions
==================================================================================================*/
void MsgSoundRepeatAlarmCB(int alarmId)
{
	MSG_BEGIN();

#ifndef MSG_WEARABLE_PROFILE

	MsgSoundPlayer::instance()->MsgSoundInitRepeatAlarm();

#endif /* MSG_WEARABLE_PROFILE */

	MSG_END();
	return;
}


void MsgSensorCBStop()
{
	MSG_BEGIN();

#ifndef MSG_WEARABLE_PROFILE

#if 0
	MsgSoundPlayer::instance()->MsgSoundPlayStop();
#else
	MsgDeleteNotification(MSG_NOTI_TYPE_ALL, -1);

	MsgRefreshNotification(MSG_NOTI_TYPE_ALL, false, MSG_ACTIVE_NOTI_TYPE_NONE);

#ifndef MSG_NOTI_INTEGRATION
	MsgRefreshNotification(MSG_NOTI_TYPE_SIM, false, MSG_ACTIVE_NOTI_TYPE_NONE);
#endif

#endif

#endif /* MSG_WEARABLE_PROFILE */

	MSG_END();
}

/*
static gboolean MsgSoundMelodyTimeout(gpointer data)
{
	MSG_BEGIN();

	MsgSoundPlayer::instance()->MsgSoundPlayStop();

	MSG_END();

	return FALSE;
}
*/

#ifndef MSG_WEARABLE_PROFILE
static void MsgSoundPlayeErrorCallback(int error_code, void *user_data)
{
	MSG_DEBUG("MsgSoundPlayeErrorCallback called [%d]", error_code);
	MsgSoundPlayer::instance()->MsgStreamStop();
	MsgSoundPlayer::instance()->MsgSoundPlayStart(NULL, MSG_SOUND_PLAY_DEFAULT);
}

static void MsgSoundPlayeCompletedCallback(void *user_data)
{
	MSG_DEBUG("MsgSoundPlayeCompletedCallback called");
	MsgSoundPlayer::instance()->MsgSoundPlayStop();
}

static void MsgSoundPlayeInterruptedCallback(player_interrupted_code_e code, void *user_data)
{
	MSG_DEBUG("MsgSoundPlayeInterruptedCallback called [%d]", code);
	MsgSoundPlayer::instance()->MsgSoundPlayStop();
}

static void MsgStreamFocusCallback(sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason_for_change, const char *additional_info, void *user_data)
{
	MSG_DEBUG("MsgStreamFocusCallback called, reason_for_change [%d], additional_info [%s]", reason_for_change, additional_info);

	sound_stream_focus_state_e playback_focus_state = SOUND_STREAM_FOCUS_STATE_ACQUIRED;

	sound_manager_get_focus_state(stream_info, &playback_focus_state, NULL);
	if (playback_focus_state == SOUND_STREAM_FOCUS_STATE_RELEASED) {
		MSG_DEBUG("sound stream focus released");
		MsgSoundPlayer::instance()->MsgSoundPlayStop();
	}
}
#endif

/*==================================================================================================
                                     IMPLEMENTATION OF MsgSoundPlayer - Member Functions
==================================================================================================*/
MsgSoundPlayer* MsgSoundPlayer::pInstance = NULL;


MsgSoundPlayer::MsgSoundPlayer()
{
#ifndef MSG_WEARABLE_PROFILE
	bPlaying = false;
	bVibrating = false;
	bFeedbackInit = false;
	g_alarmId = 0;

	defaultRingtonePath = NULL;

	if (MsgSettingGetString(VCONFKEY_SETAPPL_NOTI_RINGTONE_DEFAULT_PATH_STR, &defaultRingtonePath) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetString() is failed");
	}

	if (defaultRingtonePath == NULL || MsgGetFileSize(defaultRingtonePath) < 1) {
		defaultRingtonePath = (char *)DEFAULT_ALERT_FILE;
	}

	MSG_DEBUG("defaultRingtonePath [%s]", defaultRingtonePath);

	if (MsgSensorConnect() == MSG_SUCCESS) {
		if (MsgRegSensorCB(&MsgSensorCBStop) != MSG_SUCCESS) {
			MSG_DEBUG("Fail to MsgRegSensorCB.");
			MsgSensorDisconnect();
		}
	} else {
		MSG_DEBUG("Fail to MsgSensorConnect.");
	}
#endif /* MSG_WEARABLE_PROFILE */
}


MsgSoundPlayer::~MsgSoundPlayer()
{
}


MsgSoundPlayer* MsgSoundPlayer::instance()
{
	if (!pInstance) {
		MSG_DEBUG("pInstance is NULL. Now creating instance.");
		pInstance = new MsgSoundPlayer();
	}

	return pInstance;
}


void MsgSoundPlayer::MsgGetRingtonePath(char *userRingtonePath, char **msg_tone_file_path_p)
{
#ifndef MSG_WEARABLE_PROFILE
	int tmpVal = 0;
	if (MsgSettingGetInt(MSG_SETTING_RINGTONE_TYPE, &tmpVal) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetInt() is failed");
	}
	MSG_RINGTONE_TYPE_T ringtoneType = (MSG_RINGTONE_TYPE_T)tmpVal;

	MSG_DEBUG("Ringtone type = [%d]", ringtoneType);

	if (ringtoneType == MSG_RINGTONE_TYPE_SILENT) {
		*msg_tone_file_path_p = NULL;
		return;
	}

	char *tmpFilePath = NULL;
	*msg_tone_file_path_p = new char[MSG_FILEPATH_LEN_MAX+1];

	char *msg_tone_file_path = *msg_tone_file_path_p;

	bool bUserRingtone = userRingtonePath && userRingtonePath[0] != '\0';
	if (bUserRingtone) {
		tmpFilePath = userRingtonePath;
	} else {
		if (ringtoneType == MSG_RINGTONE_TYPE_DEFAULT) {
			if (MsgSettingGetString(VCONFKEY_SETAPPL_NOTI_MSG_RINGTONE_PATH_STR, &tmpFilePath) != MSG_SUCCESS) {
				MSG_INFO("MsgSettingGetString() is failed");
			}
		} else {
			if (MsgSettingGetString(MSG_SETTING_RINGTONE_PATH, &tmpFilePath) != MSG_SUCCESS) {
				MSG_INFO("MsgSettingGetString() is failed");
			}
		}
	}

	memset(msg_tone_file_path, 0x00, sizeof(char)*(MSG_FILEPATH_LEN_MAX+1));

	if ((tmpFilePath == NULL || MsgGetFileSize(tmpFilePath) < 1) || /* wrong ringtone file path */
			(MsgDrmIsDrmFile(tmpFilePath) && !MsgDrmCheckRingtone(tmpFilePath))) { /* check DRM */
		if (tmpFilePath && (strcmp(tmpFilePath, "silent") == 0)) {
			MSG_DEBUG("Set ringtone to NONE(Silent)");
			msg_tone_file_path = NULL;
		} else {
			MSG_DEBUG("Set ringtone to defaultRingtonePath.");
			strncpy(msg_tone_file_path, defaultRingtonePath, MSG_FILEPATH_LEN_MAX);
		}
	} else {
		MSG_DEBUG("Set ringtone to tmpFilePath.");
		snprintf(msg_tone_file_path, MSG_FILEPATH_LEN_MAX, "%s", tmpFilePath);
	}

	if (tmpFilePath && !bUserRingtone) {
		free(tmpFilePath);
		tmpFilePath = NULL;
	}
#endif /* MSG_WEARABLE_PROFILE */
}

#ifndef MSG_WEARABLE_PROFILE
void MsgSoundPlayer::MsgGetPlayStatus(bool bOnCall, bool bSound, bool bVibration, bool bMsgSound, bool bMsgVibration, bool *bPlaySound, bool *bPlayVibration)
{
	MSG_BEGIN();

	/* samsung basic concept : feedback should follow profile setting.
	 * And if notification setting exist in msg app, then apply it prior to profile when profile either sound or vibration.
	 * But, Play sound when profile is vibration during call, if device does not support haptic */
	if (bSound || bVibration) {
		bool bHantic_device = false;
#ifdef FEATURE_HAPTIC_ENABLE
		bHantic_device = true;
#endif
		if (bHantic_device || (bOnCall == false)) {
			if (bSound) {
				if (bMsgSound) {
					MSG_DEBUG("Play sound.");
					*bPlaySound = true;
				}

				if (bMsgVibration) {
					MSG_DEBUG("Play vibration.");
					*bPlayVibration = true;
				}
			} else {
				if (bMsgVibration) {
					MSG_DEBUG("Play vibration.");
					*bPlayVibration = true;
				}
			}
		} else { /* during call */
			if (bMsgSound || bMsgVibration) {
				MSG_DEBUG("Play sound.");
				*bPlaySound = true;
			}
		}
	}
}
#endif /* MSG_WEARABLE_PROFILE */

void MsgSoundPlayer::MsgGetPlayStatus(bool bVoiceMail, bool *bPlaySound, bool *bPlayVibration, bool *bOnCall)
{
	MSG_BEGIN();

#ifndef MSG_WEARABLE_PROFILE

	if (!bPlaySound || !bPlayVibration || !bOnCall) {
		MSG_DEBUG("IN parameter is NULL.");
		return;
	}

	*bPlaySound = false;
	*bPlayVibration = false;
	*bOnCall = false;

	/* Global setting */
	bool bSoundOn = false; /* sound setting on notification panel */
	bool bVibrationOn = false; /* vibration setting on notification panel */

	if (MsgSettingGetBool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, &bSoundOn) != MSG_SUCCESS)
		MSG_INFO("MsgSettingGetBool() is failed");

	if (MsgSettingGetBool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, &bVibrationOn) != MSG_SUCCESS)
		MSG_INFO("MsgSettingGetBool() is failed");

	/* Alert setting */
#if 0	/* not used value */
	bool bNotiVibrationOn = false; /* alert vibration */
	if (MsgSettingGetBool(VCONFKEY_SETAPPL_VIBRATE_WHEN_NOTIFICATION_BOOL, &bNotiVibrationOn) != MSG_SUCCESS)
		MSG_INFO("MsgSettingGetBool() is failed");
#endif

	bool bMsgSettingNoti = true; /* Alert for message notification */
	bool bMsgSettingVibration = false; /* vibration for message notification */

	MSG_RINGTONE_TYPE_T ringtoneType = MSG_RINGTONE_TYPE_DEFAULT; /*sound type for message notification */
	bool bMsgSettingSound = true;

	if (MsgSettingGetBool(MSG_SETTING_VIBRATION, &bMsgSettingVibration) != MSG_SUCCESS)
		MSG_INFO("MsgSettingGetBool() is failed");

	if (MsgSettingGetBool(MSG_SETTING_NOTIFICATION, &bMsgSettingNoti) != MSG_SUCCESS)
		MSG_INFO("MsgSettingGetBool() is failed");

	int tmpVal = 0;
	if (MsgSettingGetInt(MSG_SETTING_RINGTONE_TYPE, &tmpVal) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetInt() is failed");
	}
	ringtoneType = (MSG_RINGTONE_TYPE_T)tmpVal;
	if (ringtoneType == MSG_RINGTONE_TYPE_SILENT)
		bMsgSettingSound = false;

	MSG_SEC_DEBUG("Sound status : Sound mode[%d], Manner mode[%d]", bSoundOn, bVibrationOn);
	MSG_SEC_DEBUG("Msg Setting : Noti Alert[%d], vibration[%d], sound[%d], ringtoneType[%d]", bMsgSettingNoti, bMsgSettingVibration, bMsgSettingSound, ringtoneType);

	int callStatus = 0;
/*	int alertOnCall = 0; */

	callStatus = MsgGetCallStatus();
	MSG_DEBUG("Call Status [%d]", callStatus);

	/* Check call status */
	if (callStatus > 0 && callStatus < 3) {
		/* 1. On Call */
		*bOnCall = true; /* set call status; */
	} else {
		/* 2. Call is not active */

		MSG_DEBUG("Call is not active.");
		int voiceRecording = 0;
		if (MsgSettingGetInt(VCONFKEY_RECORDER_STATE, &voiceRecording) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}

		if (bVoiceMail) {	/* 2-1. Voice message */
			if (bMsgSettingNoti) {
				MsgGetPlayStatus(false, bSoundOn, bVibrationOn, bMsgSettingSound, bMsgSettingVibration, bPlaySound, bPlayVibration);

			} else {
				MSG_DEBUG("It doesn't play sound/vibration - voice message.");
			}
		} else {	/* 2-1. Normal message */
			if (bMsgSettingNoti) {
				if (voiceRecording != VCONFKEY_RECORDER_STATE_RECORDING) {
					MsgGetPlayStatus(false, bSoundOn, bVibrationOn, bMsgSettingSound, bMsgSettingVibration, bPlaySound, bPlayVibration);
				} else {
					MSG_DEBUG("It doesn't play sound/vibration.");
				}
			} else {
				MSG_DEBUG("It doesn't play sound/vibration.");
			}
		}
	}

#endif /* MSG_WEARABLE_PROFILE */

	MSG_END();
}


void MsgSoundPlayer::MsgSoundPlayStart(const MSG_ADDRESS_INFO_S *pAddrInfo, MSG_SOUND_TYPE_T soundType)
{
	MSG_BEGIN();
#ifndef MSG_WEARABLE_PROFILE

	MSG_DEBUG("soundType [%d]", soundType);

	/* check camera state */
	int cameraState = 0;	/* camera recording state */
	if (MsgSettingGetInt(VCONFKEY_CAMERA_STATE, &cameraState) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetInt() is failed");
	}
	MSG_SEC_DEBUG("Camera state [%d]", cameraState);

	if (cameraState == VCONFKEY_CAMERA_STATE_RECORDING) {
		MSG_END();
		return;
	}

/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	/* get contact information */
	MSG_CONTACT_INFO_S contactInfo;
	memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

	if (pAddrInfo) {
		/* Get Contact Info */
		if (MsgGetContactInfo(pAddrInfo, &contactInfo) != MSG_SUCCESS) {
			MSG_DEBUG("MsgGetContactInfo() fail.");
		}
	}
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
	/* get ringtone file path */
	char *msg_tone_file_path = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&msg_tone_file_path, unique_ptr_deleter);

	if (soundType == MSG_SOUND_PLAY_EMERGENCY) {
		msg_tone_file_path = new char[MAX_SOUND_FILE_LEN+1];
		memset(msg_tone_file_path, 0x00, sizeof(char)*(MAX_SOUND_FILE_LEN+1));
	} else if (soundType == MSG_SOUND_PLAY_DEFAULT) {
		msg_tone_file_path = new char[MAX_SOUND_FILE_LEN+1];
		memset(msg_tone_file_path, 0x00, sizeof(char)*(MAX_SOUND_FILE_LEN+1));
		strncpy(msg_tone_file_path, DEFAULT_ALERT_FILE, MAX_SOUND_FILE_LEN);
	}
/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	else {
		MsgGetRingtonePath(contactInfo.alerttonePath, &msg_tone_file_path);
	}
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
	MSG_SEC_DEBUG("soundType [%d], Sound File [%s]", soundType, msg_tone_file_path);

	/* get sound play status */
	bool bPlaySound = false;
	bool bPlayVibration = false;
	bool bVoiceMsg = (soundType == MSG_SOUND_PLAY_VOICEMAIL)?true:false;
	bool bOnCall = false;

	MsgGetPlayStatus(bVoiceMsg, &bPlaySound, &bPlayVibration, &bOnCall);

	MSG_SEC_DEBUG("sound=[%d], vibration=[%d], voice_msg?[%d], on_call?[%d]",
			bPlaySound, bPlayVibration, bVoiceMsg, bOnCall);

	/* play sound */
	if (bPlaySound) {
		int err = MsgStreamStart(soundType);

		if (err != SOUND_MANAGER_ERROR_NONE)
			MSG_DEBUG("MsgStreamStart() Failed : %d", err);
		else
			MsgSoundPlayMelody(msg_tone_file_path);
	}

/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	/* play vibration */
	if (bPlayVibration) {
		MsgSoundPlayVibration(contactInfo.vibrationPath);
	}
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */

	/* For repeatition. */
/*	MsgSoundSetRepeatAlarm(); */

#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();
}


void MsgSoundPlayer::MsgSoundPlayStop()
{
	MSG_BEGIN();
#ifndef MSG_WEARABLE_PROFILE

	int err = 0;
	pthread_mutex_lock(&muMmPlay);

	if (bPlaying == true && g_PlayerHandle != NULL) {
		MSG_DEBUG("stopping the player.");
		/* Stop playing media contents */
		err = player_stop(g_PlayerHandle);

		if (err != PLAYER_ERROR_NONE)
			MSG_DEBUG("stopping the player handle failed");
	}

	if (g_PlayerHandle != NULL) {
		MSG_DEBUG("destroy the player.");

		player_unset_error_cb(g_PlayerHandle);
		player_unset_completed_cb(g_PlayerHandle);
		player_unset_interrupted_cb(g_PlayerHandle);
		player_unprepare(g_PlayerHandle);
		player_destroy(g_PlayerHandle);
	}

	g_PlayerHandle = NULL;
	bPlaying = false;

	pthread_mutex_unlock(&muMmPlay);

	MsgStreamStop();

#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();
}


int MsgSoundPlayer::MsgStreamStart(MSG_SOUND_TYPE_T soundType)
{
	MSG_BEGIN();
	int err = 0;

#ifndef MSG_WEARABLE_PROFILE
	pthread_mutex_lock(&muStream);

	if (g_stream_info != NULL) {
		err = sound_manager_destroy_stream_information(g_stream_info);
		if (err != SOUND_MANAGER_ERROR_NONE)
			MSG_DEBUG("sound_manager_destroy_stream_information() Failed : %d", err);

		g_stream_info = NULL;
	}

	if (soundType == MSG_SOUND_PLAY_EMERGENCY)
		err = sound_manager_create_stream_information(SOUND_STREAM_TYPE_EMERGENCY, MsgStreamFocusCallback, NULL, &g_stream_info);
	else
		err = sound_manager_create_stream_information(SOUND_STREAM_TYPE_NOTIFICATION, MsgStreamFocusCallback, NULL, &g_stream_info);

	if (err != SOUND_MANAGER_ERROR_NONE) {
		MSG_DEBUG("sound_manager_create_stream_information() Failed : %d", err);
		pthread_mutex_unlock(&muStream);
		return err;
	}

	err = sound_manager_acquire_focus(g_stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
	pthread_mutex_unlock(&muStream);

#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();
	return err;
}


void MsgSoundPlayer::MsgStreamStop()
{
	MSG_BEGIN();
#ifndef MSG_WEARABLE_PROFILE
	pthread_mutex_lock(&muStream);

	if (g_stream_info != NULL) {
		int err = sound_manager_release_focus(g_stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
		if (err != SOUND_MANAGER_ERROR_NONE)
			MSG_DEBUG("sound_manager_release_focus() Failed : %d", err);

		err = sound_manager_destroy_stream_information(g_stream_info);
		if (err != SOUND_MANAGER_ERROR_NONE)
			MSG_DEBUG("sound_manager_destroy_stream_information() Failed : %d", err);

		g_stream_info = NULL;
	}
	pthread_mutex_unlock(&muStream);

#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();
}


void MsgSoundPlayer::MsgSoundPlayMelody(char *pMsgToneFilePath)
{
#ifndef MSG_WEARABLE_PROFILE
	int err = PLAYER_ERROR_NONE;

	if (!pMsgToneFilePath) {
		MSG_DEBUG("Ringtone path is NULL");
		return;
	}

	pthread_mutex_lock(&muMmPlay);

	if (g_stream_info == NULL) {
		MSG_DEBUG("g_stream_info is NULL");
		pthread_mutex_unlock(&muMmPlay);
		return;
	}

	if (g_PlayerHandle) {
		player_unset_error_cb(g_PlayerHandle);
		player_unset_completed_cb(g_PlayerHandle);
		player_unset_interrupted_cb(g_PlayerHandle);
		player_unprepare(g_PlayerHandle);
		player_destroy(g_PlayerHandle);
	}

	err = player_create(&g_PlayerHandle);

	pthread_mutex_unlock(&muMmPlay);

	if (err != PLAYER_ERROR_NONE) {
		MSG_DEBUG("creating the player handle failed");
		return;
	}

	/* Setting the call back function msg_callback */
	player_set_error_cb(g_PlayerHandle, MsgSoundPlayeErrorCallback, NULL);
	player_set_completed_cb(g_PlayerHandle, MsgSoundPlayeCompletedCallback, NULL);
	player_set_interrupted_cb(g_PlayerHandle, MsgSoundPlayeInterruptedCallback, NULL);

	player_set_audio_policy_info(g_PlayerHandle, g_stream_info);

	do {
		err = player_set_uri(g_PlayerHandle, (const char *)pMsgToneFilePath);
		if (err != PLAYER_ERROR_NONE)
			MSG_DEBUG("player_set_uri() error : [%d]", err);

		err = player_prepare(g_PlayerHandle);
		if (err != PLAYER_ERROR_NONE) {
			MSG_DEBUG("player_prepare() error : [%d]", err);
			if (pMsgToneFilePath != defaultRingtonePath) {
				pMsgToneFilePath = defaultRingtonePath;
			} else {
				return;
			}
		}
	} while (err != PLAYER_ERROR_NONE);


	pthread_mutex_lock(&muMmPlay);
	MSG_DEBUG("player_start with [%s]", pMsgToneFilePath);
	err = player_start(g_PlayerHandle);

	if (err != PLAYER_ERROR_NONE) {
	 	MSG_DEBUG("player_start, FAIL [%x]", err);
	} else {
		/* Add Timer to stop playing after 5 sec. */
		/*
		int g_contact_timer = -1;
		g_contact_timer = g_timeout_add(MSG_SOUND_TIMEOUT, (GSourceFunc)MsgSoundMelodyTimeout, NULL);
		*/

		bPlaying = true;
	}
	pthread_mutex_unlock(&muMmPlay);

#endif
}


void MsgSoundPlayer::MsgSoundPlayVibration(char *vibrationPath)
{
	MSG_BEGIN();

#ifndef MSG_WEARABLE_PROFILE

	int ret = 0;

	if (!bFeedbackInit) {
		int ret = feedback_initialize();

		if (ret != FEEDBACK_ERROR_NONE) {
			MSG_DEBUG("Fail to feedback_initialize : [%d]", ret);
			bFeedbackInit = false;
			return;
		} else {
			MSG_DEBUG("Success to feedback_initialize.");
			bFeedbackInit = true;
		}
	}

	if (vibrationPath && strlen(vibrationPath)) {
		MSG_DEBUG("vibrationPath: [%s]", vibrationPath);
		ret = feedback_set_resource_path(FEEDBACK_TYPE_VIBRATION, FEEDBACK_PATTERN_MESSAGE, vibrationPath);
		if (ret != FEEDBACK_ERROR_NONE)
			MSG_DEBUG("Fail to feedback_set_resource_path");
		ret = feedback_play_type(FEEDBACK_TYPE_VIBRATION, FEEDBACK_PATTERN_MESSAGE);

		if (ret != FEEDBACK_ERROR_NONE)
			MSG_DEBUG("Fail to feedback_play_type");
	} else {
		ret = feedback_set_resource_path(FEEDBACK_TYPE_VIBRATION, FEEDBACK_PATTERN_MESSAGE, NULL);
		if (ret != FEEDBACK_ERROR_NONE)
			MSG_DEBUG("Fail to feedback_set_resource_path");

		ret = feedback_play_type(FEEDBACK_TYPE_VIBRATION, FEEDBACK_PATTERN_MESSAGE);
		if (ret != FEEDBACK_ERROR_NONE)
			MSG_DEBUG("Fail to feedback_play_type");
	}

#endif /* MSG_WEARABLE_PROFILE */

	MSG_END();
}


void MsgSoundPlayer::MsgSoundSetRepeatAlarm()
{
#ifndef MSG_WEARABLE_PROFILE

	int nRepeatValue = 0;
	long	nRepeatTime = 0;

	if (MsgSettingGetInt(MSG_ALERT_REP_TYPE, &nRepeatValue) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetInt() is failed");
	}

	switch (nRepeatValue) {
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

	if (nRepeatTime > 0) {
		if (g_alarmId > 0) {
			if (MsgAlarmRemove(g_alarmId) != MSG_SUCCESS)
				MSG_FATAL("MsgAlarmRemove fail.");

			g_alarmId = 0;
		}
		MsgSoundCreateRepeatAlarm(nRepeatTime);
	}

#endif /* MSG_WEARABLE_PROFILE */
}


void MsgSoundPlayer::MsgSoundCreateRepeatAlarm(int RepeatTime)
{
	MSG_BEGIN();

#ifndef MSG_WEARABLE_PROFILE

	int alarmId = 0;
	time_t tmp_time;
	struct tm repeat_tm;

	time(&tmp_time);

	tmp_time += (RepeatTime*60);
	tzset();
	localtime_r(&tmp_time, &repeat_tm);

	if (MsgAlarmRegistration(&repeat_tm, MsgSoundRepeatAlarmCB, &alarmId) != MSG_SUCCESS) {
		MSG_DEBUG("MsgAlarmRegistration fail.");
		return;
	}

	g_alarmId = alarmId;
	MSG_DEBUG("Set g_alarmId to [%d]", alarmId);

	MSG_END();

#endif /* MSG_WEARABLE_PROFILE */
}


int MsgSoundPlayer::MsgSoundGetUnreadMsgCnt()
{
	int unreadCnt = 0;

#ifndef MSG_WEARABLE_PROFILE
	int unreadSms = 0;
	int unreadMms = 0;

	/* Get SMS Count */
	if (MsgSettingGetInt(VCONFKEY_MESSAGE_RECV_SMS_STATE, &unreadSms) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetInt() is failed");
	}

	/* Get MMS Count */
	if (MsgSettingGetInt(VCONFKEY_MESSAGE_RECV_MMS_STATE, &unreadMms) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetInt() is failed");
	}

	unreadCnt = unreadSms + unreadMms;

	MSG_DEBUG("unread count : [%d]", unreadCnt);

#endif /* MSG_WEARABLE_PROFILE */

	return unreadCnt;
}

void MsgSoundPlayer::MsgSoundInitRepeatAlarm()
{
	MSG_BEGIN();

#ifndef MSG_WEARABLE_PROFILE

	int nRepeatValue = 0;
	long	nRepeatTime = 0;

	g_alarmId = 0;

	if (MsgSoundGetUnreadMsgCnt() <= 0) {
		MSG_DEBUG("no unread msg");
		return;
	}

	if (MsgSettingGetInt(MSG_ALERT_REP_TYPE, &nRepeatValue) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetInt() is failed");
	}

	switch (nRepeatValue) {
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
		MsgSoundPlayStart(NULL, MSG_SOUND_PLAY_USER);

#endif /* MSG_WEARABLE_PROFILE */

	MSG_END();
}
