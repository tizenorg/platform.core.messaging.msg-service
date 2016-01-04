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

#ifndef MSG_SOUND_PLAYER_H
#define MSG_SOUND_PLAYER_H


/*==================================================================================================
                                         TYPES
==================================================================================================*/
typedef unsigned char MSG_SOUND_TYPE_T;

/*==================================================================================================
                                         ENUMS
==================================================================================================*/
enum _MSG_SOUND_TYPE_E
{
	MSG_SOUND_PLAY_DEFAULT = 0,
	MSG_SOUND_PLAY_USER,
	MSG_SOUND_PLAY_EMERGENCY,
	MSG_SOUND_PLAY_VOICEMAIL,
};

/*==================================================================================================
                                CLASS DEFINITIONS
==================================================================================================*/
class MsgSoundPlayer {
public:
	static MsgSoundPlayer* instance();

	void MsgSoundPlayStart(const MSG_ADDRESS_INFO_S *pAddrInfo, MSG_SOUND_TYPE_T soundType);
	void MsgSoundPlayStop();

	void MsgSoundInitRepeatAlarm();
	void MsgSoundPlayDtmf();

	void MsgGetRingtonePath(char *userRingtonePath, char **msg_tone_file_path_p);
	void MsgGetPlayStatus(bool bVoiceMail, bool *bPlaySound, bool *bPlayVibration, bool *bOnCall);

private:
	MsgSoundPlayer();
	~MsgSoundPlayer();

	void MsgSoundPlayMelody(char *pMsgToneFilePath);
	void MsgSoundPlayVibration(char *vibrationPath);

	void MsgSoundSetRepeatAlarm();
	void MsgSoundCreateRepeatAlarm(int RepeatTime);
	int MsgSoundGetUnreadMsgCnt();
	void MsgGetPlayStatus(bool bOnCall, bool bSound, bool bVibration, bool bMsgSound, bool bMsgVibration, bool *bPlaySound, bool *bPlayVibration);

	static MsgSoundPlayer* pInstance;

	bool bPlaying;
	bool bVibrating;
	bool bFeedbackInit;

	int g_alarmId;

	char *defaultRingtonePath;
};

#endif /* MSG_SOUND_PLAYER_H */

