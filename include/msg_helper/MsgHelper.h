/*
*
* Copyright (c) 2000-2012 Samsung Electronics Co., Ltd. All Rights Reserved.
*
* This file is part of msg-service.
*
* Contact: Jaeyun Jeong <jyjeong@samsung.com>
*          Sangkoo Kim <sangkoo.kim@samsung.com>
*          Seunghwan Lee <sh.cat.lee@samsung.com>
*          SoonMin Jung <sm0415.jung@samsung.com>
*          Jae-Young Lee <jy4710.lee@samsung.com>
*          KeeBum Kim <keebum.kim@samsung.com>
*
* PROPRIETARY/CONFIDENTIAL
*
* This software is the confidential and proprietary information of
* SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered
* into with SAMSUNG ELECTRONICS.
*
* SAMSUNG make no representations or warranties about the suitability
* of the software, either express or implied, including but not limited
* to the implied warranties of merchantability, fitness for a particular
* purpose, or non-infringement. SAMSUNG shall not be liable for any
* damages suffered by licensee as a result of using, modifying or
* distributing this software or its derivatives.
*
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


/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/

// SoundPlayer
MSG_ERROR_T MsgSoundPlayUninit();
void MsgSoundPlayStart();
void MsgSoundPlayStop();
int MsgSoundPlayMelody(char *pMsgToneFilePath, bool bIncreasing);
void MsgSoundPlayVibration();


#endif // MSG_HELPER_H
