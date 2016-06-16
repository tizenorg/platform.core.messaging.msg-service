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

#ifndef __MSG_MGR_NOTIFICATION_H__
#define __MSG_MGR_NOTIFICATION_H__

/*==================================================================================================
										INCLUDE FILES
==================================================================================================*/
#include <msg.h>

#include <app_control.h>

/*==================================================================================================
										DEFINES
==================================================================================================*/

#define MSG_NOTI_INTEGRATION

#define MSGFW_MESSAGE_TABLE_NAME				"MSG_MESSAGE_TABLE"
#define MSGFW_FOLDER_TABLE_NAME					"MSG_FOLDER_TABLE"
#define MSGFW_ADDRESS_TABLE_NAME				"MSG_ADDRESS_TABLE"
#define MSGFW_CONVERSATION_TABLE_NAME		"MSG_CONVERSATION_TABLE"
#define MMS_PLUGIN_MESSAGE_TABLE_NAME		"MSG_MMS_MESSAGE_TABLE"
#define MSGFW_SMS_REPORT_TABLE_NAME		"MSG_SMS_REPORT_TABLE"
#define MSGFW_REPORT_TABLE_NAME					"MSG_REPORT_TABLE"

#define MSG_MGR_APP_ID				"org.tizen.msg-manager"
#define MSG_DEFAULT_APP_ID			"org.tizen.message"
#define MSG_QUICKPANEL_APP_ID		"org.tizen.quickpanel"
#define MSG_CALL_APP_ID				"org.tizen.call-ui"
#define MSG_SETTING_APP_ID			"org.tizen.setting"

#define MSG_APP_PACKAGE_NAME	"message"

#define MSG_APP_LOCALEDIR			TZ_SYS_RO_APP_PATH "/" MSG_MGR_APP_ID "/res/locale"

#define MSG_NOTIFICATION_ICON_DIR			"/" MSG_QUICKPANEL_APP_ID "/shared/res/noti_icons"

/* notification icon */
#define MSG_NORMAL_ICON_PATH	TZ_SYS_RO_APP_PATH MSG_NOTIFICATION_ICON_DIR "/Message/noti_message.png"
#define MSG_SIM_ICON_PATH	TZ_SYS_RO_APP_PATH MSG_NOTIFICATION_ICON_DIR "/Message/noti_message_sim-card.png"
#define MSG_OTA_ICON_PATH	TZ_SYS_RO_APP_PATH MSG_NOTIFICATION_ICON_DIR "/Message/noti_message_OTA.png"
#define MSG_VOICE_ICON_PATH		TZ_SYS_RO_APP_PATH MSG_NOTIFICATION_ICON_DIR "/Message/noti_voice_mail.png"
#define MSG_CB_ICON_PATH			TZ_SYS_RO_APP_PATH MSG_NOTIFICATION_ICON_DIR "/Message/noti_message_cb-msg.png"
#define MSG_READ_ICON_PATH		TZ_SYS_RO_APP_PATH MSG_NOTIFICATION_ICON_DIR "/Message/noti_message_read_report.png"
#define MSG_DELIVERY_ICON_PATH	TZ_SYS_RO_APP_PATH MSG_NOTIFICATION_ICON_DIR "/Message/noti_message_delivery_report.png"
#define MSG_REPLY_ICON_PATH			TZ_SYS_RO_APP_PATH MSG_NOTIFICATION_ICON_DIR "/Message/noti_message_reply.png"
#define MSG_SMS_SENDING_FAILED_ICON_PATH		TZ_SYS_RO_APP_PATH MSG_NOTIFICATION_ICON_DIR "/Message/noti_message_failed.png"
#define MSG_MMS_SENDING_FAILED_ICON_PATH		TZ_SYS_RO_APP_PATH MSG_NOTIFICATION_ICON_DIR "/Message/noti_message_mms_failed.png"
#define MSG_MMS_RETRIVE_FAILED_ICON_PATH		TZ_SYS_RO_APP_PATH MSG_NOTIFICATION_ICON_DIR "/Message/noti_message_mms_problem.png"
#define MSG_EMERGENCY_ICON_PATH		TZ_SYS_RO_APP_PATH MSG_DEFAULT_APP_ID "/res/icons/default/small/noti_emergency_mode.png"
#define MSG_NO_CONTACT_PROFILE_ICON_PATH	TZ_SYS_RO_APP_PATH MSG_NOTIFICATION_ICON_DIR "/Contact/noti_contact_default.png"
#define MSG_ACTIVE_PUSH_ICON_PATH		"reserved:push_message"

/* status bar icon */
#define MSG_NORMAL_STATUS_ICON				"reserved://indicator/icons/notify_message"
#define MSG_FAILED_STATUS_ICON				"reserved://indicator/icons/notify_message_failed"
#define MSG_DELIVER_REPORT_STATUS_ICON	"reserved://indicator/icons/delivery_report_message"
#define MSG_READ_REPORT_STATUS_ICON		"reserved://indicator/icons/read_report_message"
#define MSG_VOICE_MSG_STATUS_ICON			"reserved://indicator/icons/notify_voicemail"
#define MSG_SIM_FULL_STATUS_ICON			"reserved://indicator/icons/sim_card_full"

#ifndef TIZEN_I586_ENABLED
/* system string */
#define NEW_MESSAGE			"IDS_MSGF_POP_NEW_MESSAGE"
#define NEW_MESSAGES			"IDS_MSGF_POP_NEW_MESSAGES"
#define MSG_UNKNOWN_SENDER	"IDS_MSGF_BODY_UNKNOWN"
#define MSG_NO_SUBJECT			"IDS_MSGF_BODY_NO_SUBJECT"


/* message app string for ticker noti */
#define SENDING_MULTIMEDIA_MESSAGE_FAILED	"IDS_MSGF_POP_SENDING_MULTIMEDIA_MESSAGE_FAILED"
#define MESSAGE_RETRIEVED								"IDS_MSGF_BODY_MESSAGE_RETRIEVED"
#define SMS_MESSAGE_SENDING_FAIL					"IDS_MSGF_POP_SENDING_MESSAGE_FAILED"
#define SMS_MESSAGE_SIM_MESSAGE_FULL				"IDS_MSGF_BODY_SIM_MEMORY_FULL_DELETE_SOME_ITEMS"
#define SMS_MESSAGE_MEMORY_FULL			"IDS_MSGF_POP_NOT_ENOUGH_MEMORY_DELETE_SOME_ITEMS"
#define SMS_SIM_CARD_FULL				"IDS_MSG_HEADER_SIM_CARD_FULL"

/* message app string for quickpanel noti */
#define MSG_MESSAGE		"IDS_MSGF_BODY_MESSAGE"
#define VOICE_MAIL					"IDS_MSGF_BODY_VOICEMAIL"
#define NEW_VOICE_MAIL			"IDS_MSGF_BODY_NEW_VOICEMAIL"
#define CB_MESSAGE "IDS_MSGF_BODY_CB_MESSAGES"
#define CP_MESSAGE "IDS_MSGF_BODY_CP_MESSAGES"
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
#define READ_REPORT_DELETE	"IDS_MSG_OPT_DELETE"
#define FAILED_MSG_EXIST	"IDS_MSG_BODY_FAILED_MESSAGES_EXIST"
#define ENABLE_EMERGENCY_MODE	"IDS_MSG_BODY_ENABLE_EMERGENCY_MODE_JPN_DCM"
#define EMERGENCY_MODE_DESCRIPTION	"IDS_MSG_SBODY_TAP_HERE_TO_ENABLE_EMERGENCY_MODE_TO_SAVE_BATTERY_POWER_JPN_DCM"
#define FAILED_TO_SEND_MESSAGE	"IDS_MSG_HEADER_FAILED_TO_SEND_MESSAGE_ABB"
#define MESSAGE_SIZE_UNIT_KB	"IDS_MSGF_BODY_MSGSIZE_KB"
#define MESSAGE_SIZE_STRING		"IDS_MSGF_BODY_MESSAGE_SIZE"
#define MSG_SUBJECT_COLON	"IDS_MSGF_OPT_SUBJECT_COLON"
#else
/* system string */
#define NEW_MESSAGE			"New message"
#define NEW_MESSAGES			"New messages"
#define MSG_UNKNOWN_SENDER	"New message from Unknown."
#define MSG_NO_SUBJECT			"No subject"


/* message app string for ticker noti */
#define SMS_MESSAGE_MEMORY_FULL			"Not enough memory. Delete some items."
#define SMS_SIM_CARD_FULL				"SIM card full"

/* message app string for quickpanel noti */
#define MSG_MESSAGE		"Message"
#define VOICE_MAIL					"Voicemail"
#define NEW_VOICE_MAIL			"New voicemail"
#define CB_MESSAGE "CB messages"
#define CP_MESSAGE "CP messages"
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
#define FAILED_TO_SEND_MESSAGE	"Failed to send message."
#define MESSAGE_SIZE_UNIT_KB	"KB"
#define MESSAGE_SIZE_STRING		"Message Size"
#define MSG_SUBJECT_COLON	"Subject:"
#endif


#define NOTIFICATION_PRIV_ID	"db/private/msg-service/notification_priv_id"
#define VOICE_NOTI_ID_1		"db/private/msg-service/voice_noti_id1"
#define VOICE_NOTI_ID_2			"db/private/msg-service/voice_noti_id2"
#define CB_NOTI_PRIV_ID		"db/private/msg-service/cb_noti_priv_id"
#define SIM_MSG_NOTI_PRIV_ID	"db/private/msg-service/sim_msg_noti_priv_id"
#define MSG_SENTFAIL_NOTI_ID		"db/private/msg-service/sentfail_noti_id"
#define SIM_FULL_NOTI_PRIV_ID	"db/private/msg-service/sim_full_noti_id"

#define MSG_SETTING_NOTIFICATION	"db/private/msg-service/general/notification"
#define MSG_SETTING_VIBRATION		"db/private/msg-service/general/vibration"
#define MSG_SETTING_PREVIEW			"db/private/msg-service/general/preview"
#define MSG_SETTING_RINGTONE_PATH			"db/private/msg-service/general/ringtone_path"
#define MSG_SETTING_RINGTONE_TYPE			"db/private/msg-service/general/ringtone_type"

#define VOICEMAIL_NUMBER				"db/private/msg-service/voice_mail/voice_mail_number"
#define VOICEMAIL_COUNT				"db/private/msg-service/voice_mail/voice_mail_count"
#define VOICEMAIL_ALPHA_ID				"db/private/msg-service/voice_mail/voice_mail_alphaid"
#define VOICEMAIL_DEFAULT_NUMBER		"db/private/msg-service/voice_mail/voice_mail_default_number"
#define VOICEMAIL_DEFAULT_ALPHA_ID		""

#define MSG_ALERT_REP_TYPE			"db/setting/sound/noti/msg_alert_rep_type"

#define MSG_TEL_URI_VOICEMAIL		"tel:VOICEMAIL"

#define MSG_NOTI_TEXT_LEN	(512)
#define MSG_NOTI_TEXT_LEN_S	(256)
#define MAX_VCONFKEY_NAME_LEN	128
#define MSG_ACTIVE_NOTI_BUTTON_NUM 3
#define MAX_QUERY_LEN		4096



typedef unsigned char msg_mgr_notification_type_t; /* _msg_notification_type_e */
typedef unsigned char msg_mgr_active_notification_type_t; /* _msg_active_notification_type_e */


/*==================================================================================================
										ENUMS
==================================================================================================*/

enum _msg_mgr_notification_type_e {
	MSG_MGR_NOTI_TYPE_ALL = 0x00,

	/* Refresh single/multiple notification */
	MSG_MGR_NOTI_TYPE_NORMAL,
	MSG_MGR_NOTI_TYPE_CB,
	MSG_MGR_NOTI_TYPE_SIM,
	MSG_MGR_NOTI_TYPE_FAILED,

	/* Add only single notification */
	MSG_MGR_NOTI_TYPE_VOICE_1,
	MSG_MGR_NOTI_TYPE_VOICE_2,
	MSG_MGR_NOTI_TYPE_MWI,
	MSG_MGR_NOTI_TYPE_CLASS0,
	MSG_MGR_NOTI_TYPE_SMS_DELIVERY_REPORT,
	MSG_MGR_NOTI_TYPE_MMS_READ_REPORT,
	MSG_MGR_NOTI_TYPE_MMS_DELIVERY_REPORT,

	MSG_MGR_NOTI_TYPE_SIM_FULL,
};


enum _msg_mgr_active_notification_type_e {
	MSG_MGR_ACTIVE_NOTI_TYPE_NONE,
	MSG_MGR_ACTIVE_NOTI_TYPE_ACTIVE,
	MSG_MGR_ACTIVE_NOTI_TYPE_INSTANT,
};


enum _msg_maintype_e {
	MSG_UNKNOWN_TYPE = 0,		/**< Unknown main type */
	MSG_SMS_TYPE,				/**< SMS */
	MSG_MMS_TYPE,				/**< MMS */
};


enum _msg_subtype_e {
	/* SMS Specific Message Type */
	MSG_NORMAL_SMS = 0,			/**< Text SMS message */
	MSG_CB_SMS,					/**< Cell Broadcasting  message */
	MSG_JAVACB_SMS,				/**< JAVA Cell Broadcasting  message */
	MSG_TYPE0_SMS,					/**< Short Message Type 0 */
	MSG_REPLACE_TYPE1_SMS,		/**< Replace Short Message Type 1 */
	MSG_REPLACE_TYPE2_SMS,		/**< Replace Short Message Type 2 */
	MSG_REPLACE_TYPE3_SMS,		/**< Replace Short Message Type 3 */
	MSG_REPLACE_TYPE4_SMS,		/**< Replace Short Message Type 4 */
	MSG_REPLACE_TYPE5_SMS,		/**< Replace Short Message Type 5 */
	MSG_REPLACE_TYPE6_SMS,		/**< Replace Short Message Type 6 */
	MSG_REPLACE_TYPE7_SMS,		/**< Replace Short Message Type 7 */
	MSG_WAP_SI_SMS,				/**< WAP Push Message SI */
	MSG_WAP_SL_SMS,				/**< WAP Push Message SL */
	MSG_WAP_CO_SMS,				/**< WAP Push Message CO */
	MSG_MWI_VOICE_SMS,			/**< MWI Message Voice */
	MSG_MWI_FAX_SMS,				/**< MWI Message Fax */
	MSG_MWI_EMAIL_SMS,			/**< MWI Message Email */
	MSG_MWI_OTHER_SMS,			/**< MWI Message Other */
	MSG_STATUS_REPORT_SMS,		/**< SMS-STATUS-REPORT */
	MSG_SYNCML_CP,				/**< SyncML Message CP */
	MSG_LBS_SMS,					/**< LBS Message */
	MSG_REJECT_SMS,				/**< Reject Message */
	MSG_CONCAT_SIM_SMS,			/**< Concatenated Message in SIM */
};

/*==================================================================================================
										STRUCTURES
==================================================================================================*/
typedef struct _msg_mgr_message_info_s {
	msg_message_id_t msgId;
	int sim_idx;
	time_t displayTime;
	msg_network_status_t networkStatus;
	char displayName[MAX_DISPLAY_NAME_LEN+1];
	char addressVal[MAX_ADDRESS_VAL_LEN+1];
	char msgText[MAX_MSG_TEXT_LEN+1];
} MSG_MGR_MESSAGE_INFO_S;

/*==================================================================================================
										FUNCTION PROTOTYPES
==================================================================================================*/

void MsgMgrInitNoti();

int MsgMgrInsertOnlyActiveNotification(msg_mgr_notification_type_t noti_type, MSG_MGR_MESSAGE_INFO_S *msg_info);
int MsgMgrRefreshNotification(msg_mgr_notification_type_t noti_type, bool bFeedback, msg_mgr_active_notification_type_t active_type);
int MsgMgrAddReportNotification(msg_mgr_notification_type_t noti_type, MSG_MGR_MESSAGE_INFO_S *msg_info);
int MsgMgrDeleteReportNotification(const char *addr);
int MsgMgrAddNotification(msg_mgr_notification_type_t noti_type, MSG_MGR_MESSAGE_INFO_S *msg_info);
int MsgMgrDeleteNoti(msg_mgr_notification_type_t noti_type, int simIndex);

void MsgMgrRefreshAllNotification(bool bWithSimNoti, bool bFeedback, msg_mgr_active_notification_type_t active_type);
int MsgMgrInsertInstantMessage(msg_mgr_notification_type_t noti_type);
bool MsgMgrCheckNotificationSettingEnable();
int MsgMgrInsertTicker(const char* pTickerMsg, const char* pLocaleTickerMsg, bool bPlayFeedback, int msgId);
int MsgMgrInsertBadge(unsigned int unreadMsgCnt);


#endif /*__MSG_MGR_NOTIFICATION_H__ */

