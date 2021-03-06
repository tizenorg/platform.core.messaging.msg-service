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
#define NORMAL_MSG_ICON_PATH	"/opt/apps/org.tizen.message/res/icons/default/small/org.tizen.message.noti.png"
#define VOICE_MSG_ICON_PATH		"/opt/apps/org.tizen.message/res/icons/default/small/org.tizen.message.voice.png"
#define CB_MSG_ICON_PATH			"/opt/apps/org.tizen.message/res/icons/default/small/org.tizen.message.noti.png"
#define NOTI_MSG_ICON_PATH		"/opt/apps/org.tizen.message/res/icons/default/small/org.tizen.message.noti.png"

#define MSG_APP_PACKAGE_NAME	"message"
#define MSG_APP_LOCALEDIR			"/opt/apps/org.tizen.message/res/locale"

#define SENDING_MULTIMEDIA_MESSAGE_FAILED	"Sending multimedia message failed"
#define MULTIMEDIA_MESSAGE_SENT					"Multimedia message sent"
#define RETRIEVING_MESSAGE_FAILED					"Retrieving message failed"
#define MESSAGE_RETRIEVED								"Message retrieved"


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

msg_error_t MsgInsertNoti(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S* pMsg);

msg_error_t MsgInsertSmsReportToNoti(MsgDbHandler *pDbHandle, msg_message_id_t MsgId, msg_delivery_report_status_t Status);

msg_error_t MsgInsertMmsReportToNoti(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S* pMsg);

msg_error_t MsgDeleteNotiByMsgId(msg_message_id_t msgId);

msg_error_t MsgDeleteNotiByThreadId(msg_thread_id_t ThreadId);

msg_error_t MsgInsertTicker(const char* pTickerMsg, const char* pLocaleTickerMsg);

#endif // MSG_QUICKPANEL_WRAPPER_H
