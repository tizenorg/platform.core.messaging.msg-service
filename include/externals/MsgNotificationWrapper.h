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

#ifndef MSG_NOTIFICATION_WRAPPER_H
#define MSG_NOTIFICATION_WRAPPER_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgInternalTypes.h"

/*==================================================================================================
                                         DEFINES
==================================================================================================*/
#define MSG_APP_PACKAGE_NAME	"message"

#define MSG_APP_LOCALEDIR			TZ_SYS_RO_APP_PATH "/" MSG_DEFAULT_APP_ID "/res/locale"


#ifndef TIZEN_I586_ENABLED
/* message app string for ticker noti */
#define SENDING_MULTIMEDIA_MESSAGE_FAILED	"IDS_MSGF_POP_SENDING_MULTIMEDIA_MESSAGE_FAILED"
#define MULTIMEDIA_MESSAGE_SENT					"IDS_MSGF_POP_MULTIMEDIA_MESSAGE_SENT"
#define RETRIEVING_MESSAGE_FAILED					"IDS_MSGF_POP_RETRIEVING_MESSAGE_FAILED"
#define MESSAGE_RETRIEVED								"IDS_MSGF_BODY_MESSAGE_RETRIEVED"
#define SMS_MESSAGE_SENT								"IDS_MSGC_POP_MESSAGESENT"
#define SMS_MESSAGE_SENDING_FAIL					"IDS_MSGF_POP_SENDING_MESSAGE_FAILED"
#define SMS_MESSAGE_SENDING_PENDING					"IDS_MSG_POP_UNABLE_TO_SEND_MESSAGE_IT_WILL_BE_SENT_WHEN_SERVICE_AVAILABLE"
#define SMS_MESSAGE_SIM_MESSAGE_FULL				"IDS_MSGF_BODY_SIM_MEMORY_FULL_DELETE_SOME_ITEMS"
#define SMS_MESSAGE_MEMORY_FULL			"IDS_MSGF_POP_NOT_ENOUGH_MEMORY_DELETE_SOME_ITEMS"
#define SMS_FDN_RESTRICTED				"IDS_MSGF_POP_FDN_ENABLED_CANT_SEND_MSG"
#define SMS_MESSAGE_SIZE_OVER_SIM_SLOT_SIZE	"IDS_MSGF_BODY_MESSAGE_IS_TOO_LARGE_TO_STORE_AS_A_SINGLE_MESSAGE_TO_SIM_CARD"

#else
/* message app string for ticker noti */
#define SENDING_MULTIMEDIA_MESSAGE_FAILED	"Sending multimedia message failed"
#define MULTIMEDIA_MESSAGE_SENT					"Multimedia message sent."
#define RETRIEVING_MESSAGE_FAILED					"Retrieving message failed"
#define MESSAGE_RETRIEVED								"Message retrieved"
#define SMS_MESSAGE_SENT								"Message sent."
#define SMS_MESSAGE_SENDING_FAIL					"Sending message failed."
#define SMS_MESSAGE_SENDING_PENDING					"Unable to send message. It will be sent when service available."
#define SMS_MESSAGE_SIM_MESSAGE_FULL				"SIM memory full. Delete some items."
#define SMS_MESSAGE_MEMORY_FULL			"Not enough memory. Delete some items."
#define SMS_FDN_RESTRICTED				"Unable to send the message while Fixed dialling mode is enabled."
#define SMS_MESSAGE_SIZE_OVER_SIM_SLOT_SIZE	"Message is too large to store as a single message to SIM card."

#endif

enum _msg_notification_type_e {
	MSG_NOTI_TYPE_ALL = 0x00,

	/* Refresh single/multiple notification */
	MSG_NOTI_TYPE_NORMAL,
	MSG_NOTI_TYPE_CB,
	MSG_NOTI_TYPE_SIM,
	MSG_NOTI_TYPE_FAILED,

	/* Add only single notification */
	MSG_NOTI_TYPE_VOICE_1,
	MSG_NOTI_TYPE_VOICE_2,
	MSG_NOTI_TYPE_MWI,
	MSG_NOTI_TYPE_CLASS0,
	MSG_NOTI_TYPE_SMS_DELIVERY_REPORT,
	MSG_NOTI_TYPE_MMS_READ_REPORT,
	MSG_NOTI_TYPE_MMS_DELIVERY_REPORT,

	MSG_NOTI_TYPE_SIM_FULL,
};

enum _msg_active_notification_type_e {
	MSG_ACTIVE_NOTI_TYPE_NONE,
	MSG_ACTIVE_NOTI_TYPE_ACTIVE,
	MSG_ACTIVE_NOTI_TYPE_INSTANT,
};

enum _MSG_SOUND_TYPE_E
{
	MSG_SOUND_PLAY_DEFAULT = 0,
	MSG_SOUND_PLAY_USER,
	MSG_SOUND_PLAY_EMERGENCY,
	MSG_SOUND_PLAY_VOICEMAIL,
};

typedef unsigned char msg_notification_type_t; /* _msg_notification_type_e */
typedef unsigned char msg_active_notification_type_t; /* _msg_active_notification_type_e */
typedef unsigned char MSG_SOUND_TYPE_T;

msg_error_t MsgInsertNotification(MSG_MESSAGE_INFO_S *msg_info);

msg_error_t MsgAddNotification(msg_notification_type_t noti_type, MSG_MESSAGE_INFO_S *msg_info);
msg_error_t MsgRefreshNotification(msg_notification_type_t noti_type, bool bFeedback, msg_active_notification_type_t active_type);
msg_error_t MsgAddReportNotification(msg_notification_type_t noti_type, MSG_MESSAGE_INFO_S *msg_info);
msg_error_t MsgInsertOnlyActiveNotification(msg_notification_type_t noti_type, MSG_MESSAGE_INFO_S *pMsgInfo);
msg_error_t MsgDeleteReportNotification(const char *addr);

msg_error_t MsgInsertTicker(const char* pTickerMsg, const char* pLocaleTickerMsg, bool bPlayFeedback, int msgId);
msg_error_t MsgDeleteNoti(msg_notification_type_t noti_type, int simIndex);

void MsgSoundPlayStart(const MSG_ADDRESS_INFO_S *pAddrInfo, MSG_SOUND_TYPE_T soundType);

void MsgRefreshAllNotification(bool bWithSimNoti, bool bFeedback, msg_active_notification_type_t active_type);

#endif
