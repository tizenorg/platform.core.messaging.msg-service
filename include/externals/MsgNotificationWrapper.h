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
#include "MsgSqliteWrapper.h"

/*==================================================================================================
                                         DEFINES
==================================================================================================*/
#define MSG_APP_PACKAGE_NAME	"message"

#define MSG_APP_LOCALEDIR			"/usr/apps/" MSG_DEFAULT_APP_ID "/res/locale"

#define MSG_NOTIFICATION_ICON_DIR			MSG_QUICKPANEL_APP_ID "/shared/res/noti_icons/Message"

// notification icon
#define MSG_NORMAL_ICON_PATH	"/usr/apps/" MSG_NOTIFICATION_ICON_DIR "/noti_message.png"
#define MSG_SIM_ICON_PATH	"/usr/apps/" MSG_NOTIFICATION_ICON_DIR "/noti_message_sim-card.png"
#define MSG_VOICE_ICON_PATH		"/usr/apps/" MSG_NOTIFICATION_ICON_DIR "/noti_voice_mail.png"
#define MSG_CB_ICON_PATH			"/usr/apps/" MSG_NOTIFICATION_ICON_DIR "/noti_message_cb-msg.png"
#define MSG_READ_ICON_PATH		"/usr/apps/" MSG_NOTIFICATION_ICON_DIR "/noti_message_read_report.png"
#define MSG_DELIVERY_ICON_PATH	"/usr/apps/" MSG_NOTIFICATION_ICON_DIR "/noti_message_delivery_report.png"
#define MSG_REPLY_ICON_PATH			"/usr/apps/" MSG_NOTIFICATION_ICON_DIR "/noti_message_reply.png"
#define MSG_SMS_SENDING_FAILED_ICON_PATH		"/usr/apps/" MSG_NOTIFICATION_ICON_DIR "/noti_message_failed.png"
#define MSG_MMS_SENDING_FAILED_ICON_PATH		"/usr/apps/" MSG_NOTIFICATION_ICON_DIR "/noti_message_mms_failed.png"
#define MSG_MMS_RETRIVE_FAILED_ICON_PATH		"/usr/apps/" MSG_NOTIFICATION_ICON_DIR "/noti_message_mms_problem.png"
#define MSG_EMERGENCY_ICON_PATH		"/usr/apps/" MSG_DEFAULT_APP_ID "/res/icons/default/small/noti_emergency_mode.png"

// status bar icon
#define MSG_NORMAL_STATUS_ICON				"/usr/apps/" MSG_DEFAULT_APP_ID "/res/icons/default/small/status_bar_message.png"
#define MSG_FAILED_STATUS_ICON				"/usr/apps/" MSG_DEFAULT_APP_ID "/res/icons/default/small/status_bar_cancel_message.png"
#define MSG_DELIVER_REPORT_STATUS_ICON	"/usr/apps/" MSG_DEFAULT_APP_ID "/res/icons/default/small/status_bar_delivery_report_message.png"
#define MSG_READ_REPORT_STATUS_ICON		"/usr/apps/" MSG_DEFAULT_APP_ID "/res/icons/default/small/status_bar_read_report_message.png"
#define MSG_VOICE_MSG_STATUS_ICON			"/usr/apps/" MSG_DEFAULT_APP_ID "/res/icons/default/small/status_bar_voicemail.png"
#define MSG_SIM_FULL_STATUS_ICON		"/usr/apps/" MSG_DEFAULT_APP_ID "/res/icons/default/small/status_bar_sim_card_full.png"

#ifndef TIZEN_I586_ENABLED
// system string
#define NEW_MESSAGE			"IDS_MSGF_POP_NEW_MESSAGE"
#define NEW_MESSAGES			"IDS_MSGF_POP_NEW_MESSAGES"
#define MSG_UNKNOWN_SENDER	"IDS_MSGF_BODY_UNKNOWN"
#define MSG_NO_SUBJECT			"IDS_MSGF_BODY_NO_SUBJECT"


// message app string for ticker noti
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
#define SMS_SIM_CARD_FULL				"IDS_MSG_HEADER_SIM_CARD_FULL"
#define SMS_MESSAGE_SIZE_OVER_SIM_SLOT_SIZE	"IDS_MSGF_BODY_MESSAGE_IS_TOO_LARGE_TO_STORE_AS_A_SINGLE_MESSAGE_TO_SIM_CARD"

// message app string for quickpanel noti
#define MSG_MESSAGE		"IDS_MSGF_BODY_MESSAGE"
#define VOICE_MAIL					"IDS_MSGF_BODY_VOICEMAIL"
#define NEW_VOICE_MAIL			"IDS_MSGF_BODY_NEW_VOICEMAIL"
#define APP_NEW_MESSAGE "IDS_MSGF_POP_NEW_MESSAGE"
#define APP_NEW_MESSAGES "IDS_MSGF_POP_NEW_MESSAGES"
#define CB_MESSAGE "IDS_MSGF_BODY_CB_MESSAGES"
#define PUSH_MESSAGE "IDS_MSGF_BODY_PUSH_MESSAGES"
#define DELIVERY_MESSAGE "IDS_MSGF_BODY_DELIVERY_REPORT"
#define READ_REPORT_MESSAGE "IDS_MSGF_POP_READ_REPORT"
#define EXPIRED_MESSAGE "IDS_MSGF_BODY_MESSAGE_HAS_EXPIRED"
#define REJECTED_MESSAGE "IDS_MSGF_BODY_MMSDELIVERYMSGREJECTED"
#define DEFERRED_MESSAGE "IDS_MSGF_POP_MESSAGE_DEFERRED"
#define UNRECOGNISED_MESSAGE "IDS_MSGF_POP_MESSAGE_UNRECOGNISED"
#define INDETEMINATE_MESSAGE "IDS_MSGF_POP_INDETERMINATE"
#define UNREACHABLE_MESSAGE "IDS_MSGF_POP_UNREACHABLE"
#define DELIVERED_MESSAGE "IDS_MSGF_BODY_MESSAGE_DELIVERED"
#define VOICE_1_MESSAGE	"IDS_MSGF_BODY_1_MESSAGE"
#define VOICE_N_MESSAGE	"IDS_MSGF_BODY_PD_MESSAGES"
#define SIM_CARD_MESSAGE		"IDS_MSGF_BODY_SIM_CARD_MESSAGES"
#define READ_REPORT_READ		"IDS_COM_BODY_READ"
#define READ_REPORT_DELETE	"IDS_COM_POP_DELETED"
#define FAILED_MSG_EXIST	"IDS_MSG_BODY_FAILED_MESSAGES_EXIST"
#define ENABLE_EMERGENCY_MODE	"IDS_MSG_BODY_ENABLE_EMERGENCY_MODE_JPN_DCM"
#define EMERGENCY_MODE_DESCRIPTION	"IDS_MSG_SBODY_TAP_HERE_TO_ENABLE_EMERGENCY_MODE_TO_SAVE_BATTERY_POWER_JPN_DCM"
#define FAILED_TO_SEND_MESSAGE	"IDS_MSG_TPOP_FAILED_TO_SEND_MESSAGE_ABB"
#else
// system string
#define NEW_MESSAGE			"New message"
#define NEW_MESSAGES			"New messages"
#define MSG_UNKNOWN_SENDER	"New message from Unknown."
#define MSG_NO_SUBJECT			"No subject"


// message app string for ticker noti
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
#define SMS_SIM_CARD_FULL				"SIM card full"
#define SMS_MESSAGE_SIZE_OVER_SIM_SLOT_SIZE	"Message is too large to store as a single message to SIM card." //

// message app string for quickpanel noti
#define MSG_MESSAGE		"Message"
#define VOICE_MAIL					"Voicemail"
#define NEW_VOICE_MAIL			"New voicemail"
#define APP_NEW_MESSAGE "New message"
#define APP_NEW_MESSAGES "New messages"
#define CB_MESSAGE "CB messages"
#define PUSH_MESSAGE "Push messages"
#define DELIVERY_MESSAGE "Delivery report"
#define READ_REPORT_MESSAGE "Read report"
#define EXPIRED_MESSAGE "Message expired."
#define REJECTED_MESSAGE "Message rejected."
#define DEFERRED_MESSAGE "Message deferred"
#define UNRECOGNISED_MESSAGE "Message unrecognised"
#define INDETEMINATE_MESSAGE "Indeterminate."
#define UNREACHABLE_MESSAGE "Unreachable"
#define DELIVERED_MESSAGE "Message delivered"
#define VOICE_1_MESSAGE	"1 message"
#define VOICE_N_MESSAGE	"%d messages"
#define SIM_CARD_MESSAGE		"SIM card messages"
#define READ_REPORT_READ		"Read"
#define READ_REPORT_DELETE	"Deleted."
#define FAILED_MSG_EXIST	"Failed messages exist."
#define ENABLE_EMERGENCY_MODE	"Enable Emergency mode"
#define EMERGENCY_MODE_DESCRIPTION	"Tap here to enable Emergency mode to save battery power."
#define FAILED_TO_SEND_MESSAGE	"Failed to send message." //
#endif

#define NOTIFICATION_PRIV_ID 	DEFAULT_SETTING_PATH"/notification_priv_id"
#define VOICE_NOTI_ID_1 		DEFAULT_SETTING_PATH"/voice_noti_id1"
#define VOICE_NOTI_ID_2			DEFAULT_SETTING_PATH"/voice_noti_id2"
#define CB_NOTI_PRIV_ID 		DEFAULT_SETTING_PATH"/cb_noti_priv_id"
#define SIM_MSG_NOTI_PRIV_ID 	DEFAULT_SETTING_PATH"/sim_msg_noti_priv_id"
#define MSG_SENTFAIL_NOTI_ID 		DEFAULT_SETTING_PATH"/sentfail_noti_id"
#define SIM_FULL_NOTI_PRIV_ID	DEFAULT_SETTING_PATH"/sim_full_noti_id"

#define MSG_NOTI_TEXT_LEN	(512)
#define MSG_NOTI_TEXT_LEN_S	(256)

enum _msg_notification_type_e
{
	MSG_NOTI_TYPE_ALL = 0x00,

	// Refresh single/multiple notification
	MSG_NOTI_TYPE_NORMAL,
	MSG_NOTI_TYPE_CB,
	MSG_NOTI_TYPE_SIM,
	MSG_NOTI_TYPE_FAILED,

	// Add only single notification
	MSG_NOTI_TYPE_VOICE_1,
	MSG_NOTI_TYPE_VOICE_2,
	MSG_NOTI_TYPE_MWI,
	MSG_NOTI_TYPE_CLASS0,
	MSG_NOTI_TYPE_SMS_DELIVERY_REPORT,
	MSG_NOTI_TYPE_MMS_READ_REPORT,
	MSG_NOTI_TYPE_MMS_DELIVERY_REPORT,

	MSG_NOTI_TYPE_SIM_FULL,
};

typedef unsigned char msg_notification_type_t; //_msg_notification_type_e

msg_error_t MsgInsertNotification(MSG_MESSAGE_INFO_S *msg_info);

msg_error_t MsgAddNotification(msg_notification_type_t noti_type, MSG_MESSAGE_INFO_S *msg_info);
msg_error_t MsgRefreshNotification(msg_notification_type_t noti_type, bool bFeedback, bool bTicker);
msg_error_t MsgAddReportNotification(msg_notification_type_t noti_type, MSG_MESSAGE_INFO_S *msg_info);
msg_error_t MsgDeleteReportNotification(const char *addr);

msg_error_t MsgInsertTicker(const char* pTickerMsg, const char* pLocaleTickerMsg, bool bPlayFeedback, int msgId);
msg_error_t MsgInsertInstantMessage(msg_notification_type_t noti_type);
msg_error_t MsgInitNoti();
msg_error_t MsgInsertBadge(unsigned int unreadMsgCnt);
msg_error_t MsgDeleteNoti(msg_notification_type_t noti_type, int simIndex);

void MsgRefreshAllNotification(bool bWithSimNoti, bool bFeedback, bool bTickerNoti);
void MsgDeleteNotification(msg_notification_type_t noti_type, int simIndex);
void MsgInitReportNotiList();

char *getTranslateText(const char *pkg_name, const char *locale_dir, const char *text);

#endif
