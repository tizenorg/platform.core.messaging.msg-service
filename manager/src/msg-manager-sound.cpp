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

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <player.h>
#include <sound_manager.h>
#include <feedback.h>
#include <feedback-internal.h>
#include <vconf.h>

#include <msg-manager-contact.h>
#include <msg-manager-debug.h>
#include <msg-manager-notification.h>
#include <msg-manager-sound.h>
#include <msg-manager-util.h>

/*==================================================================================================
                                    DEFINES
==================================================================================================*/

#define DEFAULT_ALERT_FILE		TZ_SYS_RO_APP_PATH "/" MSG_SETTING_APP_ID "shared/res/settings/Alerts/General notification_sdk.wav"

#define MSG_MGR_SOUND_TIMEOUT 5500


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
player_h g_PlayerHandle = NULL;
sound_stream_info_h g_stream_info = NULL;
char *defaultRingtonePath = NULL;
bool bFeedbackInit = false;
bool bPlaying = false;

pthread_mutex_t muMmPlay = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t muStream = PTHREAD_MUTEX_INITIALIZER;

/*==================================================================================================
										FUNCTION DEFINE
===================================================================================================*/
int MsgMgrGetFileSize(const char *pFileName);

int MsgMgrStreamStart(MSG_MGR_SOUND_TYPE_T soundType);
void MsgMgrStreamStop();
void MsgMgrGetPlayStatus(bool bOnCall, bool bSound, bool bVibration, bool bMsgSound, bool bMsgVibration, bool *bPlaySound, bool *bPlayVibration);

void MsgMgrSoundPlayMelody(char *pMsgToneFilePath);
void MsgMgrSoundPlayVibration();

/*==================================================================================================
									FUNCTION IMPLEMENTATION
==================================================================================================*/
int MsgMgrGetFileSize(const char *pFileName)
{
	struct stat file_stat;

	if (lstat(pFileName, &file_stat)) {
		MSG_MGR_FATAL("file[%s] error lstat: %s", pFileName, g_strerror(errno));
		return -1;
	}

	return (int)file_stat.st_size;
}


static void MsgMgrSoundPlayeErrorCallback(int error_code, void *user_data)
{
	MSG_MGR_DEBUG("MsgMgrSoundPlayeErrorCallback called [%d]", error_code);
	MsgMgrStreamStop();
	MsgMgrSoundPlayStart(NULL, MSG_MGR_SOUND_PLAY_DEFAULT);
}

static void MsgMgrSoundPlayeCompletedCallback(void *user_data)
{
	MSG_MGR_DEBUG("MsgMgrSoundPlayeCompletedCallback called");
	MsgMgrSoundPlayStop();
}

static void MsgMgrSoundPlayeInterruptedCallback(player_interrupted_code_e code, void *user_data)
{
	MSG_MGR_DEBUG("MsgMgrSoundPlayeInterruptedCallback called [%d]", code);
	MsgMgrSoundPlayStop();
}

static void MsgStreamFocusCallback(sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason_for_change, const char *additional_info, void *user_data)
{
	MSG_MGR_DEBUG("MsgStreamFocusCallback called, reason_for_change [%d], additional_info [%s]", reason_for_change, additional_info);

	sound_stream_focus_state_e playback_focus_state = SOUND_STREAM_FOCUS_STATE_ACQUIRED;

	sound_manager_get_focus_state(stream_info, &playback_focus_state, NULL);
	if (playback_focus_state == SOUND_STREAM_FOCUS_STATE_RELEASED) {
		MSG_MGR_DEBUG("sound stream focus released");
		MsgMgrSoundPlayStop();
	}
}


void initMsgMgrSoundPlayer()
{
	bPlaying = false;
	bFeedbackInit = false;

	defaultRingtonePath = vconf_get_str(VCONFKEY_SETAPPL_NOTI_RINGTONE_DEFAULT_PATH_STR);

	if (defaultRingtonePath == NULL || MsgMgrGetFileSize(defaultRingtonePath) < 1) {
			defaultRingtonePath = (char *)DEFAULT_ALERT_FILE;
	}

	MSG_MGR_DEBUG("defaultRingtonePath [%s]", defaultRingtonePath);
}


void MsgMgrGetRingtonePath(char *userRingtonePath, char **msg_tone_file_path_p)
{
	int tmpVal = 0;
	if (vconf_get_int(MSG_SETTING_RINGTONE_TYPE, &tmpVal) != 0) {
		MSG_MGR_INFO("vcong_get_int() is failed");
	}
	int ringtoneType = tmpVal;

	MSG_MGR_DEBUG("Ringtone type = [%d]", ringtoneType);

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
			tmpFilePath = vconf_get_str(VCONFKEY_SETAPPL_NOTI_MSG_RINGTONE_PATH_STR);
		} else {
			tmpFilePath = vconf_get_str(MSG_SETTING_RINGTONE_PATH);
		}
	}

	memset(msg_tone_file_path, 0x00, sizeof(char)*(MSG_FILEPATH_LEN_MAX+1));
#if MSG_MGR_DRM_SUPPORT
	if ((tmpFilePath == NULL || MsgMgrGetFileSize(tmpFilePath) < 1) || /* wrong ringtone file path */
			(MsgDrmIsDrmFile(tmpFilePath) && !MsgDrmCheckRingtone(tmpFilePath))) { /* check DRM */
#endif
	if ((tmpFilePath == NULL || MsgMgrGetFileSize(tmpFilePath) < 1)) { /* wrong ringtone file path */
		if (tmpFilePath && (strcmp(tmpFilePath, "silent") == 0)) {
			MSG_MGR_DEBUG("Set ringtone to NONE(Silent)");
			msg_tone_file_path = NULL;
		} else {
			MSG_MGR_DEBUG("Set ringtone to defaultRingtonePath.");
			if (defaultRingtonePath && defaultRingtonePath[0] != '\0') {
				MSG_MGR_DEBUG("defaultRingtonePath [%s]", defaultRingtonePath);
				snprintf(msg_tone_file_path, MSG_FILEPATH_LEN_MAX, "%s", defaultRingtonePath);
			} else {
				MSG_MGR_DEBUG("defaultRingtonePath is null");
				msg_tone_file_path = NULL;
			}
		}
	} else {
		MSG_MGR_DEBUG("Set ringtone to tmpFilePath.");
		snprintf(msg_tone_file_path, MSG_FILEPATH_LEN_MAX, "%s", tmpFilePath);
	}

	if (tmpFilePath && !bUserRingtone) {
		free(tmpFilePath);
		tmpFilePath = NULL;
	}
}

void MsgMgrGetPlayStatus(bool bVoiceMail, bool *bPlaySound, bool *bPlayVibration, bool *bOnCall)
{
	MSG_MGR_BEGIN();

	if (!bPlaySound || !bPlayVibration) {
		MSG_MGR_DEBUG("IN parameter is NULL.");
		return;
	}

	*bPlaySound = false;
	*bPlayVibration = false;

	/* Global setting */
	int bSoundOn = 0; /* sound setting on notification panel */
	int bVibrationOn = 0; /* vibration setting on notification panel */

	if (vconf_get_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, &bSoundOn) != 0)
		MSG_MGR_INFO("vconf_get_bool() is failed");

	if (vconf_get_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, &bVibrationOn) != 0)
		MSG_MGR_INFO("vconf_get_bool() is failed");

	/* Alert setting */
#if 0	/* not used value */
	int bNotiVibrationOn = 0; /* alert vibration */
	if (vconf_get_bool(VCONFKEY_SETAPPL_VIBRATE_WHEN_NOTIFICATION_BOOL, &bNotiVibrationOn) != 0)
		MSG_MGR_INFO("vconf_get_bool() is failed");
#endif

	int bMsgSettingNoti = 1; /* Alert for message notification */
	int bMsgSettingVibration = 0; /* vibration for message notification */

	int ringtoneType = MSG_RINGTONE_TYPE_DEFAULT; /*sound type for message notification */
	bool bMsgSettingSound = true;

	if (vconf_get_bool(MSG_SETTING_VIBRATION, &bMsgSettingVibration) != 0)
		MSG_MGR_INFO("vconf_get_bool() is failed");

	if (vconf_get_bool(MSG_SETTING_NOTIFICATION, &bMsgSettingNoti) != 0)
		MSG_MGR_INFO("vconf_get_bool() is failed");

	int tmpVal = 0;
	if (vconf_get_int(MSG_SETTING_RINGTONE_TYPE, &tmpVal) != 0) {
		MSG_MGR_INFO("vconf_get_int() is failed");
	}
	ringtoneType = tmpVal;
	if (ringtoneType == MSG_RINGTONE_TYPE_SILENT)
		bMsgSettingSound = false;

	MSG_MGR_SEC_DEBUG("Sound status : Sound mode[%d], Manner mode[%d]", bSoundOn, bVibrationOn);
	MSG_MGR_SEC_DEBUG("Msg Setting : Noti Alert[%d], vibration[%d], sound[%d], ringtoneType[%d]", bMsgSettingNoti, bMsgSettingVibration, bMsgSettingSound, ringtoneType);

	cm_call_status_e callStatus = CM_CALL_STATUS_IDLE;
/*	int alertOnCall = 0; */

	callStatus = MsgMgrGetCallStatus();
	MSG_MGR_DEBUG("Call Status [%d]", callStatus);

	/* Check call status */
	if (callStatus > CM_CALL_STATUS_IDLE && callStatus < CM_CALL_STATUS_MAX) {
		/* 1. On Call */
		*bOnCall = true; /* set call status; */
	} else {
		/* 2. Call is not active */
		MSG_MGR_DEBUG("Call is not active.");
		int voiceRecording = 0;
		if (vconf_get_int(VCONFKEY_RECORDER_STATE, &voiceRecording) != 0) {
			MSG_MGR_INFO("vconf_get_int() is failed");
		}

		if (bVoiceMail) {	/* 2-1. Voice message */
			if (bMsgSettingNoti) {
				MsgMgrGetPlayStatus(false, bSoundOn, bVibrationOn, bMsgSettingSound, bMsgSettingVibration, bPlaySound, bPlayVibration);

			} else {
				MSG_MGR_DEBUG("It doesn't play sound/vibration - voice message.");
			}
		} else {	/* 2-1. Normal message */
			if (bMsgSettingNoti) {
				if (voiceRecording != VCONFKEY_RECORDER_STATE_RECORDING) {
					MsgMgrGetPlayStatus(false, bSoundOn, bVibrationOn, bMsgSettingSound, bMsgSettingVibration, bPlaySound, bPlayVibration);
				} else {
					MSG_MGR_DEBUG("It doesn't play sound/vibration.");
				}
			} else {
				MSG_MGR_DEBUG("It doesn't play sound/vibration.");
			}
		}
	}

	MSG_MGR_END();
}


void MsgMgrGetPlayStatus(bool bOnCall, bool bSound, bool bVibration, bool bMsgSound, bool bMsgVibration, bool *bPlaySound, bool *bPlayVibration)
{
	MSG_MGR_BEGIN();

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
					MSG_MGR_DEBUG("Play sound.");
					*bPlaySound = true;
				}

				if (bMsgVibration) {
					MSG_MGR_DEBUG("Play vibration.");
					*bPlayVibration = true;
				}
			} else {
				if (bMsgVibration) {
					MSG_MGR_DEBUG("Play vibration.");
					*bPlayVibration = true;
				}
			}
		} else { /* during call */
			if (bMsgSound || bMsgVibration) {
				MSG_MGR_DEBUG("Play sound.");
				*bPlaySound = true;
			}
		}
	}

	MSG_MGR_END();
}


void MsgMgrSoundPlayStart(const MSG_MGR_ADDRESS_INFO_S *pAddrInfo, MSG_MGR_SOUND_TYPE_T soundType)
{
	MSG_MGR_BEGIN();

	MSG_MGR_DEBUG("soundType [%d]", soundType);

	/* check camera state */
	int cameraState = 0;	/* camera recording state */
	if (vconf_get_int(VCONFKEY_CAMERA_STATE, &cameraState) != 0) {
		MSG_MGR_INFO("vconf_get_int() is failed");
	}
	MSG_MGR_SEC_DEBUG("Camera state [%d]", cameraState);

	if (cameraState == VCONFKEY_CAMERA_STATE_RECORDING) {
		MSG_MGR_END();
		return;
	}

	MSG_MGR_CONTACT_INFO_S contactInfo = {0, };
	if (pAddrInfo) {
		if (MsgMgrGetContactInfo(pAddrInfo, &contactInfo) != 0) {
			MSG_MGR_DEBUG("MsgGetContactInfo() fail.");
		}
	}

	/* get ringtone file path */
	char *msg_tone_file_path = NULL;

	if (soundType == MSG_MGR_SOUND_PLAY_EMERGENCY) {
		msg_tone_file_path = new char[MSG_FILEPATH_LEN_MAX+1];
		memset(msg_tone_file_path, 0x00, sizeof(char)*(MSG_FILEPATH_LEN_MAX+1));
	} else if (soundType == MSG_MGR_SOUND_PLAY_DEFAULT) {
		msg_tone_file_path = new char[MSG_FILEPATH_LEN_MAX+1];
		memset(msg_tone_file_path, 0x00, sizeof(char)*(MSG_FILEPATH_LEN_MAX+1));
		snprintf(msg_tone_file_path, MSG_FILEPATH_LEN_MAX, "%s", DEFAULT_ALERT_FILE);
	} else {
		MsgMgrGetRingtonePath(contactInfo.alerttonePath, &msg_tone_file_path);
	}
	MSG_MGR_SEC_DEBUG("soundType [%d], Sound File [%s]", soundType, msg_tone_file_path);

	/* get sound play status */
	bool bPlaySound = false;
	bool bPlayVibration = false;
	bool bVoiceMsg = (soundType == MSG_MGR_SOUND_PLAY_VOICEMAIL)?true:false;
	bool bOnCall = false;

	MsgMgrGetPlayStatus(bVoiceMsg, &bPlaySound, &bPlayVibration, &bOnCall);

	MSG_MGR_SEC_DEBUG("sound=[%d], vibration=[%d], voice_msg?[%d], on_call?[%d]",
			bPlaySound, bPlayVibration, bVoiceMsg, bOnCall);

	/* play sound */
	if (bPlaySound) {
		int err = MsgMgrStreamStart(soundType);

		if (err != SOUND_MANAGER_ERROR_NONE)
			MSG_MGR_DEBUG("MsgMgrStreamStart() Failed : %d", err);
		else
			MsgMgrSoundPlayMelody(msg_tone_file_path);
	}

	if (bPlayVibration)
		MsgMgrSoundPlayVibration();

	if (msg_tone_file_path)
		delete [] msg_tone_file_path;

	MSG_MGR_END();
}


void MsgMgrSoundPlayStop()
{
	MSG_MGR_BEGIN();

	int err = 0;
	pthread_mutex_lock(&muMmPlay);

	if (bPlaying == true && g_PlayerHandle != NULL) {
		MSG_MGR_DEBUG("stopping the player.");
		/* Stop playing media contents */
		err = player_stop(g_PlayerHandle);

		if (err != PLAYER_ERROR_NONE)
			MSG_MGR_DEBUG("stopping the player handle failed");
	}

	if (g_PlayerHandle != NULL) {
		MSG_MGR_DEBUG("destroy the player.");

		player_unset_error_cb(g_PlayerHandle);
		player_unset_completed_cb(g_PlayerHandle);
		player_unset_interrupted_cb(g_PlayerHandle);
		player_unprepare(g_PlayerHandle);
		player_destroy(g_PlayerHandle);
	}

	g_PlayerHandle = NULL;
	bPlaying = false;

	pthread_mutex_unlock(&muMmPlay);

	MsgMgrStreamStop();

	MSG_MGR_END();
}


int MsgMgrStreamStart(MSG_MGR_SOUND_TYPE_T soundType)
{
	MSG_MGR_BEGIN();
	int err = 0;

	pthread_mutex_lock(&muStream);

	if (g_stream_info != NULL) {
		err = sound_manager_destroy_stream_information(g_stream_info);
		if (err != SOUND_MANAGER_ERROR_NONE)
			MSG_MGR_DEBUG("sound_manager_destroy_stream_information() Failed : %d", err);

		g_stream_info = NULL;
	}

	if (soundType == MSG_MGR_SOUND_PLAY_EMERGENCY)
		err = sound_manager_create_stream_information(SOUND_STREAM_TYPE_EMERGENCY, MsgStreamFocusCallback, NULL, &g_stream_info);
	else
		err = sound_manager_create_stream_information(SOUND_STREAM_TYPE_NOTIFICATION, MsgStreamFocusCallback, NULL, &g_stream_info);

	if (err != SOUND_MANAGER_ERROR_NONE) {
		MSG_MGR_DEBUG("sound_manager_create_stream_information() Failed : %d", err);
		pthread_mutex_unlock(&muStream);
		return err;
	}

	err = sound_manager_acquire_focus(g_stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
	pthread_mutex_unlock(&muStream);

	MSG_MGR_END();
	return err;
}


void MsgMgrStreamStop()
{
	MSG_MGR_BEGIN();
	pthread_mutex_lock(&muStream);

	if (g_stream_info != NULL) {
		int err = sound_manager_release_focus(g_stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
		if (err != SOUND_MANAGER_ERROR_NONE)
			MSG_MGR_DEBUG("sound_manager_release_focus() Failed : %d", err);

		err = sound_manager_destroy_stream_information(g_stream_info);
		if (err != SOUND_MANAGER_ERROR_NONE)
			MSG_MGR_DEBUG("sound_manager_destroy_stream_information() Failed : %d", err);

		g_stream_info = NULL;
	}
	pthread_mutex_unlock(&muStream);

	MSG_MGR_END();
}


void MsgMgrSoundPlayMelody(char *pMsgToneFilePath)
{
	int err = PLAYER_ERROR_NONE;

	if (!pMsgToneFilePath) {
		MSG_MGR_DEBUG("Ringtone path is NULL");
		return;
	}

	pthread_mutex_lock(&muMmPlay);

	if (g_stream_info == NULL) {
		MSG_MGR_DEBUG("g_stream_info is NULL");
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
		MSG_MGR_DEBUG("creating the player handle failed");
		return;
	}

	/* Setting the call back function msg_callback */
	player_set_error_cb(g_PlayerHandle, MsgMgrSoundPlayeErrorCallback, NULL);
	player_set_completed_cb(g_PlayerHandle, MsgMgrSoundPlayeCompletedCallback, NULL);
	player_set_interrupted_cb(g_PlayerHandle, MsgMgrSoundPlayeInterruptedCallback, NULL);

	player_set_audio_policy_info(g_PlayerHandle, g_stream_info);

	do {
		err = player_set_uri(g_PlayerHandle, (const char *)pMsgToneFilePath);
		if (err != PLAYER_ERROR_NONE)
			MSG_MGR_DEBUG("player_set_uri() error : [%d]", err);

		err = player_prepare(g_PlayerHandle);
		if (err != PLAYER_ERROR_NONE) {
			MSG_MGR_DEBUG("player_prepare() error : [%d]", err);
			if (pMsgToneFilePath != defaultRingtonePath) {
				pMsgToneFilePath = defaultRingtonePath;
			} else {
				return;
			}
		}
	} while (err != PLAYER_ERROR_NONE);


	pthread_mutex_lock(&muMmPlay);
	MSG_MGR_DEBUG("player_start with [%s]", pMsgToneFilePath);
	err = player_start(g_PlayerHandle);

	if (err != PLAYER_ERROR_NONE) {
		MSG_MGR_DEBUG("player_start, FAIL [%x]", err);
	} else {
		/* Add Timer to stop playing after 5 sec. */
		/*
		int g_contact_timer = -1;
		g_contact_timer = g_timeout_add(MSG_MGR_SOUND_TIMEOUT, (GSourceFunc)MsgSoundMelodyTimeout, NULL);
		*/

		bPlaying = true;
	}
	pthread_mutex_unlock(&muMmPlay);
}


void MsgMgrSoundPlayVibration()
{
	MSG_MGR_BEGIN();

	int ret = 0;

	if (!bFeedbackInit) {
		int ret = feedback_initialize();

		if (ret != FEEDBACK_ERROR_NONE) {
			MSG_MGR_DEBUG("Fail to feedback_initialize : [%d]", ret);
			bFeedbackInit = false;
			return;
		} else {
			MSG_MGR_DEBUG("Success to feedback_initialize.");
			bFeedbackInit = true;
		}
	}

#if 1
	ret = feedback_play_type(FEEDBACK_TYPE_VIBRATION, FEEDBACK_PATTERN_MESSAGE);
	if (ret != FEEDBACK_ERROR_NONE)
		MSG_MGR_DEBUG("Fail to feedback_play_type");
#else
	if (vibrationPath && strlen(vibrationPath)) {
		MSG_MGR_DEBUG("vibrationPath: [%s]", vibrationPath);
		ret = feedback_set_resource_path(FEEDBACK_TYPE_VIBRATION, FEEDBACK_PATTERN_MESSAGE, vibrationPath);
		if (ret != FEEDBACK_ERROR_NONE)
			MSG_MGR_DEBUG("Fail to feedback_set_resource_path");
		ret = feedback_play_type(FEEDBACK_TYPE_VIBRATION, FEEDBACK_PATTERN_MESSAGE);

		if (ret != FEEDBACK_ERROR_NONE)
			MSG_MGR_DEBUG("Fail to feedback_play_type");
	} else {
		ret = feedback_set_resource_path(FEEDBACK_TYPE_VIBRATION, FEEDBACK_PATTERN_MESSAGE, NULL);
		if (ret != FEEDBACK_ERROR_NONE)
			MSG_MGR_DEBUG("Fail to feedback_set_resource_path");

		ret = feedback_play_type(FEEDBACK_TYPE_VIBRATION, FEEDBACK_PATTERN_MESSAGE);
		if (ret != FEEDBACK_ERROR_NONE)
			MSG_MGR_DEBUG("Fail to feedback_play_type");
	}
#endif
	MSG_MGR_END();
}
