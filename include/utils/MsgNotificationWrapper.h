/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef MSG_QUICKPANEL_WRAPPER_H
#define MSG_QUICKPANEL_WRAPPER_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgInternalTypes.h"
#include "MsgSqliteWrapper.h"


/*==================================================================================================
                                         DEFINES
==================================================================================================*/
#define MSG_APP_PACKAGE_NAME	"message"
#define MSG_SYS_PACKAGE_NAME	"sys_string"
#define NORMAL_MSG_ICON_PATH	"/usr/apps/org.tizen.message/res/icons/default/small/org.tizen.message.noti.png"
#define VOICE_MSG_ICON_PATH		"/usr/apps/org.tizen.message/res/icons/default/small/org.tizen.message.voice.png"
#define CB_MSG_ICON_PATH			"/usr/apps/org.tizen.message/res/icons/default/small/org.tizen.message.noti.png"
#define NOTI_MSG_ICON_PATH		"/usr/apps/org.tizen.message/res/icons/default/small/org.tizen.message.noti.png"

#define MSG_APP_LOCALEDIR			"/usr/apps/org.tizen.message/res/locale"
#define SENDING_MULTIMEDIA_MESSAGE_FAILED	"Sending multimedia message failed"
#define MULTIMEDIA_MESSAGE_SENT					"Multimedia message sent"
#define RETRIEVING_MESSAGE_FAILED					"Retrieving message failed"
#define MESSAGE_RETRIEVED								"Message retrieved"
#define SMS_MESSAGE_SENT					"Message sent"
#define SMS_MESSAGE_SENDING_FAIL			"Sending message failed"
#define MESSAGE						"Message"
#define NEW_MESSAGE				"New message"
#define NEW_MESSAGES			"New messages"
#define MSG_SYS_LOCALEDIR			"/usr/share/locale"

#define NOTIFICATION_PRIV_ID DEFAULT_SETTING_PATH"/notification_priv_id"

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
msg_error_t MsgInsertNoti(MSG_MESSAGE_INFO_S* pMsg);

msg_error_t MsgInsertMmsReportToNoti(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S* pMsg);

msg_error_t MsgRefreshNoti(bool bWithTicker);
msg_error_t MsgCleanAndResetNoti();

msg_error_t MsgInsertTicker(const char* pTickerMsg, const char* pLocaleTickerMsg);
msg_error_t MsgInsertBadge(unsigned int unreadMsgCnt);

#endif // MSG_QUICKPANEL_WRAPPER_H
