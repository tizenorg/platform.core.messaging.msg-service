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

#ifndef __MSG_MGR_SOUND_H__
#define __MSG_MGR_SOUND_H__

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

/*==================================================================================================
                                    DEFINES
==================================================================================================*/


/*==================================================================================================
                                         TYPES
==================================================================================================*/
typedef unsigned char MSG_MGR_SOUND_TYPE_T;

/*==================================================================================================
                                         ENUMS
==================================================================================================*/
enum _MSG_MGR_SOUND_TYPE_E
{
	MSG_MGR_SOUND_PLAY_DEFAULT = 0,
	MSG_MGR_SOUND_PLAY_USER,
	MSG_MGR_SOUND_PLAY_EMERGENCY,
	MSG_MGR_SOUND_PLAY_VOICEMAIL,
};


/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/



/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
void initMsgMgrSoundPlayer();

void MsgMgrSoundPlayStart(const MSG_MGR_ADDRESS_INFO_S *pAddrInfo, MSG_MGR_SOUND_TYPE_T soundType);
void MsgMgrSoundPlayStop();

void MsgMgrGetRingtonePath(char *userRingtonePath, char **msg_tone_file_path_p);
void MsgMgrGetPlayStatus(bool bVoiceMail, bool *bPlaySound, bool *bPlayVibration, bool *bOnCall);

#endif /*__MSG_MGR_SOUND_H__ */

