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

#ifndef MSG_HELPER_H
#define MSG_HELPER_H


#include "MsgTypes.h"

/*==================================================================================================
					DEFINES
==================================================================================================*/

#define MSG_SOUND_START	"SOUND_START"
#define MSG_SOUND_STOP		"SOUND_STOP"

#define MAX_SOUND_FILE_LEN 1024

#define DEFAULT_FILE 		"/usr/share/media/Sherbet.wav"
#define ALERT_ON_CALL_TONE	"/opt/etc/msg-service/alert_on_call.mp3"

#define HAPTIC_TEST_ITERATION 1
#define MSG_VIBRATION_INTERVAL 3000

typedef void (*msg_sensor_cb)();

/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/

// SoundPlayer
msg_error_t MsgSoundPlayUninit();
void MsgSoundPlayStart();
void MsgSoundPlayStop();
int MsgSoundPlayMelody(char *pMsgToneFilePath, bool bIncreasing);
void MsgSoundPlayVibration();

//Sensor FW wrapper.
msg_error_t MsgSensorConnect();
void MsgSensorDisconnect();
msg_error_t MsgRegSensorCB(msg_sensor_cb cb);

#endif // MSG_HELPER_H