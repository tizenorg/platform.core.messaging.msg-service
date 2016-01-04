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

#ifdef _USE_MM_FW_
#include <mm_error.h>
#include <mm_player.h>
#include <mm_session_private.h>
#include <mm_sound.h>
#endif

#include <feedback.h>
#include <feedback-internal.h>

/*==================================================================================================
                                    DEFINES
==================================================================================================*/
#define MAX_SOUND_FILE_LEN 1024

#define DEFAULT_ALERT_FILE		"/opt/usr/share/settings/Alerts/Whistle.ogg"

#define HAPTIC_TEST_ITERATION 1
#define MSG_VIBRATION_INTERVAL 3000
#define MSG_SOUND_TIMEOUT 5500


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
#ifdef _USE_MM_FW_
MMHandleType hPlayerHandle = 0;
#endif

pthread_mutex_t muMmPlay = PTHREAD_MUTEX_INITIALIZER;

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

#if 0
static int MsgSoundPlayCallback(int message, void *param, void *user_param)
{
#ifdef _USE_MM_FW_
#ifndef MSG_WEARABLE_PROFILE
	switch (message) {
	case MM_MESSAGE_BEGIN_OF_STREAM:
		MSG_DEBUG("Play is started.");
		break;
	case MM_MESSAGE_END_OF_STREAM:
	case MM_MESSAGE_STATE_INTERRUPTED:
		MSG_DEBUG("EOS or Interrupted.");
		MsgSoundPlayer::instance()->MsgSoundPlayStop();
		break;
	case MM_MESSAGE_FILE_NOT_SUPPORTED:
	case MM_MESSAGE_FILE_NOT_FOUND:
	case MM_MESSAGE_DRM_NOT_AUTHORIZED:
	case MM_MESSAGE_ERROR:
		MSG_DEBUG("message [%d] & play with default", message);
		MsgSoundPlayer::instance()->MsgSoundPlayStart(NULL, MSG_SOUND_PLAY_DEFAULT);
		break;
	default:
		MSG_DEBUG("message [%d]", message);
		break;
	}
#endif /* MSG_WEARABLE_PROFILE */
#endif
	return 1;
}
#endif

/*==================================================================================================
                                     IMPLEMENTATION OF MsgSoundPlayer - Member Functions
==================================================================================================*/
MsgSoundPlayer* MsgSoundPlayer::pInstance = NULL;


MsgSoundPlayer::MsgSoundPlayer()
{
#ifdef _USE_MM_FW_
#ifndef MSG_WEARABLE_PROFILE
	bPlaying = false;
	bVibrating = false;
	bFeedbackInit = false;
	g_alarmId = 0;

	defaultRingtonePath = NULL;

	defaultRingtonePath = MsgSettingGetString(VCONFKEY_SETAPPL_NOTI_RINGTONE_DEFAULT_PATH_STR);

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
#endif
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
	MSG_RINGTONE_TYPE_T ringtoneType = (MSG_RINGTONE_TYPE_T)MsgSettingGetInt(MSG_SETTING_RINGTONE_TYPE);

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
			tmpFilePath = MsgSettingGetString(VCONFKEY_SETAPPL_NOTI_MSG_RINGTONE_PATH_STR);
		} else {
			tmpFilePath = MsgSettingGetString(MSG_SETTING_RINGTONE_PATH);
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


bool MsgIsSoundPlayOnCall(void)
{
	bool bPlayOnCall = false;
#ifdef _USE_MM_FW_
#ifndef MSG_WEARABLE_PROFILE

	int err = MM_ERROR_NONE;

	mm_sound_device_in soundIn = MM_SOUND_DEVICE_IN_NONE;
	mm_sound_device_out soundOut = MM_SOUND_DEVICE_OUT_NONE;

	err = mm_sound_get_active_device(&soundIn, &soundOut);

	if (err == MM_ERROR_NONE) {
		if (soundOut & (MM_SOUND_DEVICE_OUT_RECEIVER|MM_SOUND_DEVICE_OUT_SPEAKER|MM_SOUND_DEVICE_OUT_BT_SCO))
			bPlayOnCall = true;
		else
			bPlayOnCall = false;

		MSG_DEBUG("mm_sound_device_out=[0x%04x],bPlayOnCall=[%d]", soundOut, bPlayOnCall);
	} else {
		MSG_DEBUG("mm_sound_get_active_device() err=[%d]", err);
	}

#endif /* MSG_WEARABLE_PROFILE */
#endif
	return bPlayOnCall;
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

	MsgSettingGetBool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, &bSoundOn);
	MsgSettingGetBool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, &bVibrationOn);

	/* Alert setting */
#if 0	/* not used value */
	bool bNotiVibrationOn = false; /* alert vibration */
	MsgSettingGetBool(VCONFKEY_SETAPPL_VIBRATE_WHEN_NOTIFICATION_BOOL, &bNotiVibrationOn);
#endif

	bool bMsgSettingNoti = true; /* Alert for message notification */
	bool bMsgSettingVibration = false; /* vibration for message notification */

	MSG_RINGTONE_TYPE_T ringtoneType = MSG_RINGTONE_TYPE_DEFAULT; /*sound type for message notification */
	bool bMsgSettingSound = true;

	MsgSettingGetBool(MSG_SETTING_VIBRATION, &bMsgSettingVibration);
	MsgSettingGetBool(MSG_SETTING_NOTIFICATION, &bMsgSettingNoti);
	ringtoneType = (MSG_RINGTONE_TYPE_T)MsgSettingGetInt(MSG_SETTING_RINGTONE_TYPE);
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
#if 0
		alertOnCall = MsgSettingGetInt(VCONFKEY_CISSAPPL_ALERT_ON_CALL_INT);
		MSG_DEBUG("Alert On Call [%d]", alertOnCall);

		if (alertOnCall == 0) {
			MSG_DEBUG("Call is active & Alert on Call - Off");
		} else if (alertOnCall == 1) {
			/* set default value to true, while on Call alert sound to be played and vibration to be off. */
			bool bPlayOnCall = true;

			if (!bSoundOn) {
				/* check whther sound should be on when sound setting on notification panel is off */
				bPlayOnCall = MsgIsSoundPlayOnCall();
			}

			if (bVoiceMail) {	/* 1-1. Voice message */
				if (bPlayOnCall && bMsgSettingNoti) {
					MSG_DEBUG("On call, Play sound - voice message");
					*bPlaySound = true;
				} else {
					MSG_DEBUG("On call, It doesn't play sound - voice message");
				}

				MSG_DEBUG("On call, It doesn't play vibration - voice message");
			} else {	/* 1-2. Normal message */
				if (bMsgSettingNoti) {
					if (bPlayOnCall) {
						MSG_DEBUG("On call, Play sound");
						*bPlaySound = true;
					} else {
						MSG_DEBUG("On call, It doesn't play sound");
					}

					MSG_DEBUG("On call, It doesn't play vibration");
				}
			}
		}
#endif
	} else {
		/* 2. Call is not active */

		MSG_DEBUG("Call is not active.");
		int voiceRecording = MsgSettingGetInt(VCONFKEY_RECORDER_STATE);

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
#ifdef _USE_MM_FW_
#ifndef MSG_WEARABLE_PROFILE

	MSG_DEBUG("soundType [%d]", soundType);

	/* check camera state */
	int cameraState = 0;	/* camera recording state */
	cameraState = MsgSettingGetInt(VCONFKEY_CAMERA_STATE);
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
		int err = MM_ERROR_NONE;

		if (soundType == MSG_SOUND_PLAY_EMERGENCY)
			err = mm_session_init(MM_SESSION_TYPE_EMERGENCY);
		else
			err = mm_session_init(MM_SESSION_TYPE_NOTIFY);

		if (err != MM_ERROR_NONE)
			MSG_DEBUG("MM Session Init Failed");
		else
			MSG_DEBUG("MM Session Init Success : %d", err);


		MsgSoundPlayMelody(msg_tone_file_path);

		err = mm_session_finish();

		if (err != MM_ERROR_NONE)
			MSG_DEBUG("MM Session Finish Failed.");
		else
			MSG_DEBUG("MM Session Finish Success : %d", err);
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
#endif
	MSG_END();
}


void MsgSoundPlayer::MsgSoundPlayStop()
{
	MSG_BEGIN();
#ifdef _USE_MM_FW_
#ifndef MSG_WEARABLE_PROFILE

	pthread_mutex_lock(&muMmPlay);

	if (bPlaying == true && hPlayerHandle != 0) {
		MSG_DEBUG("stopping the player.");
		/* Stop playing media contents */
		int err = mm_player_stop(hPlayerHandle);

		if (err != MM_ERROR_NONE)
			MSG_DEBUG("stopping the player handle failed");
	}

	if (hPlayerHandle != 0) {
		MSG_DEBUG("destroy the player.");

		/* Uninitializing the player module */
		mm_player_unrealize(hPlayerHandle);

		/* Destroying the player handle */
		mm_player_destroy(hPlayerHandle);
	}

	hPlayerHandle = 0;
	bPlaying = false;

	pthread_mutex_unlock(&muMmPlay);

#endif /* MSG_WEARABLE_PROFILE */
#endif
	MSG_END();
}


void MsgSoundPlayer::MsgSoundPlayMelody(char *pMsgToneFilePath)
{
#ifndef MSG_WEARABLE_PROFILE
#ifdef _USE_MM_FW_
	int err = MM_ERROR_NONE;

	if (!pMsgToneFilePath) {
		MSG_DEBUG("Ringtone path is NULL");
		return;
	}

	pthread_mutex_lock(&muMmPlay);

	if (hPlayerHandle != 0) {
		mm_player_unrealize(hPlayerHandle);
		mm_player_destroy(hPlayerHandle);
		hPlayerHandle = 0;
	}

	err = mm_player_create(&hPlayerHandle);

	pthread_mutex_unlock(&muMmPlay);

	if (err != MM_ERROR_NONE) {
		MSG_DEBUG("creating the player handle failed");
		return;
	}

	/* Setting the call back function msg_callback */
	mm_player_set_message_callback(hPlayerHandle, MsgSoundPlayCallback, NULL);

	do {
		/* Setting fade in/out, Volume */
		err = mm_player_set_attribute(hPlayerHandle, NULL,
				"sound_volume_type", MM_SOUND_VOLUME_TYPE_NOTIFICATION,
				"profile_uri", pMsgToneFilePath, strlen(pMsgToneFilePath),
				"sound_priority", 2,
				NULL);

		if (err != MM_ERROR_NONE)
			MSG_DEBUG("error setting the profile attr [%d]", err);

		err = mm_player_realize(hPlayerHandle);

		if (err != MM_ERROR_NONE) {
			MSG_DEBUG("mm_player_realize() error : [%d]", err);
			if (pMsgToneFilePath != defaultRingtonePath) {
				pMsgToneFilePath = defaultRingtonePath;
			} else {
				return;
			}
		}
	} while (err != MM_ERROR_NONE);


	pthread_mutex_lock(&muMmPlay);
	MSG_DEBUG("mm_player_start with [%s]", pMsgToneFilePath);
	err = mm_player_start(hPlayerHandle);

	if (err != MM_ERROR_NONE) {
	 	MSG_DEBUG("mm_player_start, FAIL [%x]", err);
	} else {
		/* Add Timer to stop playing after 5 sec. */
		/*
		int g_contact_timer = -1;
		g_contact_timer = g_timeout_add(MSG_SOUND_TIMEOUT, (GSourceFunc)MsgSoundMelodyTimeout, NULL);
		*/

		bPlaying = true;
	}
	pthread_mutex_unlock(&muMmPlay);

#endif /* MSG_WEARABLE_PROFILE */
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

void MsgSoundPlayer::MsgSoundPlayDtmf()
{
	MSG_BEGIN();
#ifdef _USE_MM_FW_
#ifndef MSG_WEARABLE_PROFILE

	int ret = 0;
	int hToneHandle = 0;

	ret = mm_sound_play_tone(MM_SOUND_TONE_PROP_BEEP2, VOLUME_TYPE_SYSTEM, 1.0, 300, &hToneHandle);

	if (ret < 0) {
		MSG_DEBUG("play tone failed.");
	} else {
		MSG_DEBUG("play tone success.");
	}

#endif /* MSG_WEARABLE_PROFILE */
#endif
	MSG_END();
}


void MsgSoundPlayer::MsgSoundSetRepeatAlarm()
{
#ifndef MSG_WEARABLE_PROFILE

	int nRepeatValue = 0;
	long	nRepeatTime = 0;

	nRepeatValue = MsgSettingGetInt(MSG_ALERT_REP_TYPE);

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

	/* Get SMS Count */
	unreadCnt = MsgSettingGetInt(VCONFKEY_MESSAGE_RECV_SMS_STATE);

	/* Get MMS Count */
	unreadCnt += MsgSettingGetInt(VCONFKEY_MESSAGE_RECV_MMS_STATE);

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

	nRepeatValue = MsgSettingGetInt(MSG_ALERT_REP_TYPE);

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
