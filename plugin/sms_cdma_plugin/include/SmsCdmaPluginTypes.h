/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved
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

#include "MsgDebug.h"
#include "MsgTypes.h"
#include "MsgInternalTypes.h"

#ifndef SMS_CDMA_PLUGIN_TYPES_H
#define SMS_CDMA_PLUGIN_TYPES_H

#define MAX_SMS_SEND_RETRY	4
#define SMS_MAX_MESSAGE_ID 65536
#define SMS_SEQ_NUM_MAX 64
#define SMS_MAX_USER_DATA_LEN 160
#define SMS_MAX_SUBMIT_MESSAGE_ID 256

#define SMS_TRANS_ADDRESS_MAX_LEN 256

#define SMS_MAX_NUMBER_OF_ACK 8

#define SMS_PUSH_XML_HREF_TAG "href"
#define SMS_PUSH_XML_SI_ID_TAG "si-id"
#define SMS_PUSH_XML_CREATED_TAG "created"
#define SMS_PUSH_XML_EXPIRES_TAG "si-expires"
#define SMS_PUSH_XML_ACTION_TAG "action"

#define SMS_PUSH_XML_INVAL_OBJ "invalidate-object"
#define SMS_PUSH_XML_INVAL_SVC "invalidate-service"
#define SMS_PUSH_XML_CO_URI "uri"


typedef unsigned char sms_wap_app_code_t;	/* _sms_wap_app_code_e */

typedef struct _SMS_WSP_CONTENTS_TYPE_S {
	char*         contentsTypeName;
	unsigned char contentsTypeCode;
} SMS_WSP_CONTENTS_TYPE_S;


typedef struct _SMS_WSP_CHARSET_S {
	char*  charsetName;
	unsigned short charsetCode;
} SMS_WSP_CHARSET_S;


typedef struct _SMS_WAP_UNREGISTER_CONTENTS_TYPE_S {
	char*         contentsTypeName;
	unsigned short contentsTypeCode;
} SMS_WAP_UNREGISTER_CONTENTS_TYPE_S;


typedef struct _SMS_WSP_LANGUAGE_S {
	char*         languageName;
	unsigned char languageCode;
} SMS_WSP_LANGUAGE_S;


typedef struct _SMS_WSP_HEADER_PARAMETER_S {
	char*         parameterToken;
	unsigned int parameterCode;
} SMS_WSP_HEADER_PARAMETER_S;


typedef struct _SMS_WSP_METHOD_TYPE_S {
	char*         methodName;
	unsigned char methodCode;
} SMS_WSP_METHOD_TYPE_S;


typedef struct _SMS_WSP_SECURITY_TYPE_S {
	char*         SecurityTypeName;
	unsigned char SecurityTypeCode;
}SMS_WSP_SECURITY_TYPE_S;


typedef struct {
	msg_request_id_t			reqId;		/* < Indicates the request ID, which is unique. When applications submit a request to the framework, this value will be set by the framework. */
	MSG_MESSAGE_INFO_S			msgInfo;	/* < Indicates the message structure to be sent by applications. */
	MSG_SENDINGOPT_INFO_S		sendOptInfo;
} sms_request_info_s;


typedef struct _sms_sent_info_s {
	sms_request_info_s		reqInfo;		/* < Indicates the corresponding request structure. */
	bool						bLast;
} sms_sent_info_s;


enum _sms_network_status_e {
	SMS_NETWORK_SEND_SUCCESS = 0x00,
	SMS_NETWORK_SENDING,
	SMS_NETWORK_SEND_FAIL,
	SMS_NETWORK_SEND_FAIL_TIMEOUT,
	SMS_NETWORK_SEND_FAIL_MANDATORY_INFO_MISSING,
	SMS_NETWORK_SEND_FAIL_TEMPORARY,
	SMS_NETWORK_SEND_FAIL_BY_MO_CONTROL_WITH_MOD,
	SMS_NETWORK_SEND_FAIL_BY_MO_CONTROL_NOT_ALLOWED,
	SMS_NETWORK_SEND_FAIL_FDN_RESTRICED,
	SMS_NETWORK_SEND_PENDING,
	SMS_NETWORK_SEND_FAIL_UNKNOWN_SUBSCRIBER,
	SMS_NETWORK_SEND_FAIL_MS_DISABLED,
	SMS_NETWORK_SEND_FAIL_NETWORK_NOT_READY,
};


enum _sms_wap_app_code_e {
	SMS_WAP_APPLICATION_DEFAULT = 0x00,

	SMS_WAP_APPLICATION_PUSH_SI,
	SMS_WAP_APPLICATION_PUSH_SIC,

	SMS_WAP_APPLICATION_PUSH_SL,
	SMS_WAP_APPLICATION_PUSH_SLC,

	SMS_WAP_APPLICATION_PUSH_CO,
	SMS_WAP_APPLICATION_PUSH_COC,

	SMS_WAP_APPLICATION_MMS_UA,

	SMS_WAP_APPLICATION_PUSH_SIA,

	SMS_WAP_APPLICATION_SYNCML_DM_BOOTSTRAP,
	SMS_WAP_APPLICATION_SYNCML_DM_BOOTSTRAP_XML,
	SMS_WAP_APPLICATION_SYNCML_DM_NOTIFICATION,
	SMS_WAP_APPLICATION_SYNCML_DS_NOTIFICATION,
	SMS_WAP_APPLICATION_SYNCML_DS_NOTIFICATION_WBXML,

	SMS_WAP_APPLICATION_LOC_UA_WBXML,
	SMS_WAP_APPLICATION_LOC_UA_XML,

	SMS_WAP_APPLICATION_DRM_UA_XML,
	SMS_WAP_APPLICATION_DRM_UA_MESSAGE,
	SMS_WAP_APPLICATION_DRM_UA_CONETENT,
	SMS_WAP_APPLICATION_DRM_UA_RIGHTS_XML,
	SMS_WAP_APPLICATION_DRM_UA_RIGHTS_WBXML,
	SMS_WAP_APPLICATION_DRM_V2_RO_XML,
	SMS_WAP_APPLICATION_DRM_V2_ROAP_PDU_XML,
	SMS_WAP_APPLICATION_DRM_V2_ROAP_TRIGGER_XML,
	SMS_WAP_APPLICATION_DRM_V2_ROAP_TRIGGER_WBXML,

	SMS_WAP_APPLICATION_PUSH_PROVISIONING,
	SMS_WAP_APPLICATION_PUSH_PROVISIONING_XML,
	SMS_WAP_APPLICATION_PUSH_PROVISIONING_WBXML,

	SMS_WAP_APPLICATION_PUSH_BROWSER_SETTINGS,
	SMS_WAP_APPLICATION_PUSH_BROWSER_BOOKMARKS,
	SMS_WAP_APPLICATION_PUSH_SYNCSET_WBXML,
	SMS_WAP_APPLICATION_PUSH_SYNCSET_XML,

	SMS_WAP_APPLICATION_PUSH_EMAIL_XML,
	SMS_WAP_APPLICATION_PUSH_EMAIL_WBXML,

	SMS_WAP_APPLICATION_PUSH_IMPS_CIR,

	SMS_WAP_APPLICATION_PUSH_WAP_WMLC,

	SMS_WAP_APPLICATION_WML_UA,
	SMS_WAP_APPLICATION_WTA_UA,

	SMS_WAP_APPLICATION_PUSH_SYNCML,
	SMS_WAP_APPLICATION_LOC_UA,
	SMS_WAP_APPLICATION_SYNCML_DM,
	SMS_WAP_APPLICATION_PUSH_EMAIL,

	SMS_OMA_APPLICATION_ULP_UA,
	SMS_OMA_APPLICATION_DLOTA_UA,

	SMS_WAP_APPLICATION_LBS,
};


typedef unsigned char 	sms_network_status_t;	/* _sms_network_status_e */

/********************************************************************************/
/*								TELESERVICE LAYER 								*/
/********************************************************************************/


typedef bool 				sms_digit_mode_t;			/* _sms_digit_mode_e */

typedef bool 				sms_number_mode_t;			/* _sms_number_mode_e */

typedef unsigned char		sms_number_type_t;			/* _sms_number_type_e */

typedef unsigned char		sms_number_plan_t;			/* _sms_number_plan_e */


typedef enum _sms_message_type_e {
	SMS_TYPE_RESERVED 		= 0x00,		/* reserved */
	SMS_TYPE_DELIVER,					/* mobile-terminated only */
	SMS_TYPE_SUBMIT,					/* mobile-originated only */
	SMS_TYPE_CANCEL,					/* mobile-originated only */
	SMS_TYPE_DELIVERY_ACK,				/* mobile-terminated only*/
	SMS_TYPE_USER_ACK,					/* either direction */
	SMS_TYPE_READ_ACK,					/* either direction */
	SMS_TYPE_DELIVER_REPORT,			/* mobile-originated only */
	SMS_TYPE_SUBMIT_REPORT	= 0x08,		/* mobile-terminated only */
	SMS_TYPE_MAX_VALUE
}sms_message_type_t;


typedef enum _sms_alert_option_e {
	SMS_ALERT_NO_ALERT				= 0,
	SMS_ALERT_DEFAULT_ALERT,
	SMS_ALERT_VIBRATE_ONCE,
	SMS_ALERT_VIBRATE_REPEAT,
	SMS_ALERT_VISUAL_ONCE,
	SMS_ALERT_VISUAL_REPEAT,
	SMS_ALERT_LOW_PRIORITY_ONCE,
	SMS_ALERT_LOW_PRIORITY_REPEAT,
	SMS_ALERT_MEDIUM_PRIORITY_ONCE,
	SMS_ALERT_MEDIUM_PRIORITY_REPEAT,
	SMS_ALERT_HIGH_PRIORITY_ONCE,
	SMS_ALERT_HIGH_PRIORITY_REPEAT,
	SMS_ALERT_RESERVED
}sms_alert_option_t;


typedef enum _sms_language_type_e {
	SMS_LAN_UNKNOWN,
	SMS_LAN_ENGLISH,
	SMS_LAN_FRENCH,
	SMS_LAN_SPANISH,
	SMS_LAN_JAPANESE,
	SMS_LAN_KOREAN,
	SMS_LAN_CHINESE,
	SMS_LAN_HEBREW,
}sms_language_type_t;


typedef enum _sms_priority_indicator_e {
	SMS_PRIORITY_NORMAL				= 0x00,
	SMS_PRIORITY_INTERACTIVE,
	SMS_PRIORITY_URGENT,
	SMS_PRIORITY_EMERGENCY
}sms_priority_indicator_t;


typedef enum _sms_privacy_indicator_e {
	SMS_PRIVACY_NOT_RESTRICTED		= 0x00,
	SMS_PRIVACY_RESTRICTED,
	SMS_PRIVACY_CONFIDENTIAL,
	SMS_PRIVACY_SECRET
}sms_privacy_indicator_t;


typedef enum _sms_alert_priority_e {
	SMS_ALERT_MOBILE_DEFAULT		= 0x00,
	SMS_ALERT_LOW_PRIORITY,
	SMS_ALERT_MEDIUM_PRIORITY,
	SMS_ALERT_HIGH_PRIORITY
}sms_alert_priority_t;


typedef enum _sms_display_mode_e {
	SMS_DISPLAY_IMMEDIATE			= 0x00,
	SMS_DISPLAY_DEFAULT_SETTING,
	SMS_DISPLAY_USER_INVOKE,
	SMS_DISPLAY_RESERVED
}sms_display_mode_t;


typedef enum _sms_encoding_type_e {
	SMS_ENCODE_OCTET	= 0x0,
	SMS_ENCODE_EPM	= 0x1,	/*IS-91 Extended Protocol Message*/
	SMS_ENCODE_7BIT_ASCII	= 0x2,
	SMS_ENCODE_IA5	= 0x3,
	SMS_ENCODE_UNICODE	= 0x4,
	SMS_ENCODE_SHIFT_JIS	= 0x5,
	SMS_ENCODE_KOREAN	= 0x6,
	SMS_ENCODE_LATIN_HEBREW	= 0x7,
	SMS_ENCODE_LATIN	= 0x8,
	SMS_ENCODE_GSM7BIT	= 0x9,
	SMS_ENCODE_GSMDCS = 0xa,
	SMS_ENCODE_EUCKR = 0x10,
	SMS_ENCODE_RESERVED
}sms_encoding_type_t;


typedef enum _sms_relative_time_e {
	SMS_REL_TIME_5_MINS			= 0,
	SMS_REL_TIME_12_HOURS		= 143,
	SMS_REL_TIME_1_DAY			= 167,
	SMS_REL_TIME_2_DAYS			= 168,
	SMS_REL_TIME_3_DAYS			= 169,
	SMS_REL_TIME_1_WEEK			= 173,
	SMS_REL_TIME_INDEFINITE		= 245,
	SMS_REL_TIME_IMMEDIATE		= 246,
	SMS_REL_TIME_ACTIVE			= 247,
	SMS_REL_TIME_REGISTRATION	= 248,
	SMS_REL_TIME_RESERVED
}sms_relative_time_t;


typedef enum _sms_status_code_e {
	/* ERROR_CLASS = '00' (no error) */
	SMS_STATUS_ACCEPTED				= 0x00,
	SMS_STATUS_DEPOSITED			= 0x01,
	SMS_STATUS_DELIVERED			= 0x02,
	SMS_STATUS_CANCELLED			= 0x03,

	/* ERROR_CLASS = '10' (temporary condition) */
	SMS_STATUS_TEMP_NETWORK_CONGESTION	= 0x84,
	SMS_STATUS_TEMP_NETWORK_ERROR		= 0x85,
	SMS_STATUS_TEMP_UNKNOWN_ERROR		= 0x9F,

	/* ERROR_CLASS = '11' (permanent condition) */
	SMS_STATUS_PERMANENT_NETWORK_CONGESTION	= 0xC4,
	SMS_STATUS_PERMANENT_NETWORK_ERROR		= 0xC5,
	SMS_STATUS_PERMANENT_CANCEL_FAILED		= 0xC6,
	SMS_STATUS_PERMANENT_BLOCKED_DESTINATION	= 0xC7,
	SMS_STATUS_PERMANENT_TEXT_TOO_LONG		= 0xC8,
	SMS_STATUS_PERMANENT_DUPLICATE_MESSAGE	= 0xC9,
	SMS_STATUS_PERMANENT_INVALID_DESTINATION	= 0xCA,
	SMS_STATUS_PERMANENT_MESSAGE_EXPIRED	= 0xCD,
	SMS_STATUS_PERMANENT_UNKNOWN_ERROR		= 0xDF,
}sms_status_code_t;


typedef enum _sms_cmae_category_e {
	SMS_CMAE_CTG_GEO				= 0x00,
	SMS_CMAE_CTG_MET				= 0x01,
	SMS_CMAE_CTG_SAFETY			= 0x02,
	SMS_CMAE_CTG_SECURITY		= 0x03,
	SMS_CMAE_CTG_RESCUE			= 0x04,
	SMS_CMAE_CTG_FIRE				= 0x05,
	SMS_CMAE_CTG_HEALTH			= 0x06,
	SMS_CMAE_CTG_ENV				= 0x07,
	SMS_CMAE_CTG_TRANSPORT		= 0x08,
	SMS_CMAE_CTG_INFRA			= 0x09,
	SMS_CMAE_CTG_CBRNE			= 0x0a,
	SMS_CMAE_CTG_OTHER			= 0x0b,
	SMS_CMAE_CTG_RESERVED,
}sms_cmae_category_t;


typedef enum _sms_cmae_response_type_e {
	SMS_CMAE_RESP_TYPE_SHELTER				= 0x00,
	SMS_CMAE_RESP_TYPE_EVACUATE				= 0x01,
	SMS_CMAE_RESP_TYPE_PREPARE				= 0x02,
	SMS_CMAE_RESP_TYPE_EXECUTE				= 0x03,
	SMS_CMAE_RESP_TYPE_MONITOR				= 0x04,
	SMS_CMAE_RESP_TYPE_AVOID					= 0x05,
	SMS_CMAE_RESP_TYPE_ASSESS				= 0x06,
	SMS_CMAE_RESP_TYPE_NONE					= 0x07,
	SMS_CMAE_RESP_TYPE_RESERVED,
}sms_cmae_response_type_t;


typedef enum _sms_cmae_severity_e {
	SMS_CMAE_SEVERITY_EXTREME			= 0x0,
	SMS_CMAE_SEVERITY_SEVERE				= 0x1,
	SMS_CMAE_SEVERITY_RESERVED,
}sms_cmae_severity_t;


typedef enum _sms_cmae_urgency_e {
	SMS_CMAE_URGENCY_IMMEDIATE			= 0x0,
	SMS_CMAE_URGENCY_EXPECTED			= 0x1,
	SMS_CMAE_URGENCY_RESERVED,
}sms_cmae_urgency_t;


typedef enum _sms_cmae_certainty_e {
	SMS_CMAE_CERTAINTY_OBSERVED			= 0x0,
	SMS_CMAE_CERTAINTY_LIKELY			= 0x1,
	SMS_CMAE_CERTAINTY_RESERVED,
}sms_cmae_certainty_t;


typedef enum _sms_cmae_alert_handle_e {
	SMS_CMAE_ALERT_PRESIDENTIAL				= 0x00,
	SMS_CMAE_ALERT_EXTREME					= 0x01,
	SMS_CMAE_ALERT_SEVERE					= 0x02,
	SMS_CMAE_ALERT_AMBER						= 0x03,
	SMS_CMAE_ALERT_RESERVED,
}sms_cmae_alert_handle_t;


enum _sms_bearer_sub_param_e {
	SMS_BEARER_MESSAGE_IDENTIFIER				= 0x00,
	SMS_BEARER_USER_DATA						= 0x01,
	SMS_BEARER_USER_RESPONSE_CODE				= 0x02,
	SMS_BEARER_MSG_CENTER_TIME_STAMP			= 0x03,
	SMS_BEARER_VALIDITY_PERIOD_ABSOLUTE			= 0x04,
	SMS_BEARER_VALIDITY_PERIOD_RELATIVE			= 0x05,
	SMS_BEARER_DEFERRED_DELIVERY_TIME_ABSOLUTE	= 0x06,
	SMS_BEARER_DEFERRED_DELIVERY_TIME_RELATIVE	= 0x07,
	SMS_BEARER_PRIORITY_INDICATOR				= 0x08,
	SMS_BEARER_PRIVACY_INDICATOR				= 0x09,
	SMS_BEARER_REPLY_OPTION						= 0x0A,
	SMS_BEARER_NUMBER_OF_MESSAGES				= 0x0B,
	SMS_BEARER_ALERT_ON_MSG_DELIVERY			= 0x0C,
	SMS_BEARER_LANGUAGE_INDICATOR				= 0x0D,
	SMS_BEARER_CALLBACK_NUMBER					= 0x0E,
	SMS_BEARER_MSG_DISPLAY_MODE					= 0x0F,
	SMS_BEARER_MULTI_ENCODING_USER_DATA			= 0x10,
	SMS_BEARER_MSG_DEPOSIT_INDEX				= 0x11,
	SMS_BEARER_SVC_CATEGORY_PROGRAM_DATA		= 0x12,
	SMS_BEARER_SVC_CATEGORY_PROGRAM_RESULT		= 0x13,
	SMS_BEARER_MESSAGE_STATUS					= 0x14,
	SMS_BEARER_TP_FAILURE_CAUSE					= 0x15,
	SMS_BEARER_ENHANCED_VMN						= 0x16,
	SMS_BEARER_ENHANCED_VMN_ACK					= 0x17,
	SMS_BEARER_MAX_VALUE
};


enum _sms_svc_ctg_result_e {
	SMS_SVC_RESULT_SUCCESS					= 0x00,
	SMS_SVC_RESULT_MEMORY_LIMIT_EXCEEDED,
	SMS_SVC_RESULT_LIMIT_EXCEEDED,
	SMS_SVC_RESULT_ALREADY_PROGRAMMED,
	SMS_SVC_RESULT_NOT_PREVIOUSLY_PROGRAMMED,
	SMS_SVC_RESULT_INVALID_MAX_MESSAGES,
	SMS_SVC_RESULT_INVALID_ALERT_OPTION,
	SMS_SVC_RESULT_INVALID_SVC_CTG_NAME,
	SMS_SVC_RESULT_INSPECIFIED_PROGRAMMING_FAILURE,
	SMS_SVC_RESULT_RESERVED
};


enum _SMS_TIME_FORMAT_E {
	SMS_TIME_EMPTY = 0,
	SMS_TIME_RELATIVE	,
	SMS_TIME_ABSOLUTE
};


enum _sms_tp_failure_cause_e {
	/*	0x00 ~ 0x7f reserved			*/
	SMS_TP_CAUSE_RESERVED,

	/*	0x80 ~ 0x8f TP-PID errors	*/
	SMS_TP_CAUSE_TELEMATIC_INTERWORKING_NOT_SUPPORTED	= 0x80,
	SMS_TP_CAUSE_SHORT_MSG_TYPE_0_NOT_SUPPORTED			= 0x81,
	SMS_TP_CAUSE_CANNOT_REPLACE_SHORT_MSG					= 0x82,
	SMS_TP_CAUSE_UNSPECIFIED_TP_PID_ERROR					= 0x8f,

	/* 0x90 ~ 0x9f TP-DCS errors	*/
	SMS_TP_CAUSE_DCS_NOT_SPPORTED						= 0x90,
	SMS_TP_CAUSE_MSG_CLASS_NOT_SUPPORTED				= 0x91,
	SMS_TP_CAUSE_UNSPECIFIED_TP_DCS_ERROR				= 0x9f,

	/* 0xa0 ~ 0xaf TP-Command Errors	*/
	SMS_TP_CAUSE_CMD_CANNOT_BE_ACTIONED				= 0xa0,
	SMS_TP_CAUSE_CMD_UNSUPPORTED						= 0xa1,
	SMS_TP_CAUSE_UNSPECIFIED_TP_CMD_ERROR				= 0xaf,

	SMS_TP_CAUSE_TPDU_NOT_SUPPORTED						= 0xb0,

	SMS_TP_CAUSE_SC_BUSY									= 0xc0,
	SMS_TP_CAUSE_NO_SC_SUBCRIPTION						= 0xc1,
	SMS_TP_CAUSE_SC_SYSTEM_FAILURE						= 0xc2,
	SMS_TP_CAUSE_INVALID_SME_ADDRESS					= 0xc3,
	SMS_TP_CAUSE_DESTINATION_SME_BARRED				= 0xc4,
	SMS_TP_CAUSE_SM_REJECTED_DUPLICATE_SM				= 0xc5,
	SMS_TP_CAUSE_TP_VPF_NOT_SUPPORTED					= 0xc6,
	SMS_TP_CAUSE_TP_VP_NOT_SUPPORTED					= 0xc7,

	SMS_TP_CAUSE_SIM_SMS_STORAGE_FULL					= 0xd0,
	SMS_TP_CAUSE_NO_SMS_STORAGE_CAPABILITY_IN_SIM		= 0xd1,
	SMS_TP_CAUSE_ERROR_IN_MS								= 0xd2,
	SMS_TP_CAUSE_MEMORY_CAPACITY_EXCEEDED				= 0xd3,
	SMS_TP_CAUSE_SIM_APPLICATION_TOOLKIT_BUSY			= 0xd4,
	SMS_TP_CAUSE_SIM_DATA_DOWNLOAD_ERROR				= 0xd5,

	/* 0xe0 ~ 0xfe Values specific to an application	*/
	SMS_TP_CAUSE_SPECIFIC_TO_APPLICATION_MIN			= 0xe0,
	SMS_TP_CAUSE_SPECIFIC_TO_APPLICATION_MAX			= 0xfe,

	SMS_TP_CAUSE_UNSPECIFIED_ERROR_CAUSE				= 0xff
};


typedef struct _sms_trans_msg_id_s {
	unsigned short 		msg_id;
	bool				header_ind;
}sms_trans_msg_id_s;


typedef struct _sms_telesvc_addr_s {
	sms_digit_mode_t	digit_mode;
	sms_number_type_t	number_type;
	sms_number_plan_t	number_plan;
	unsigned int		addr_len;
	char				szData[SMS_TRANS_ADDRESS_MAX_LEN + 1];
}sms_telesvc_addr_s;


typedef struct _sms_reply_option_s {
	bool	user_ack_req;
	bool	deliver_ack_req;
	bool	read_ack_req;
	bool	report_req;
}sms_reply_option_s;


typedef struct _sms_time_relative_s {
	sms_relative_time_t		rel_time;
}sms_time_rel_s;

typedef struct _sms_time_stamp_s {
	unsigned char		year;		/* range 00-99 (96~99 : 19xx, 00~95 : 20xx) */
	unsigned char		month;		/* range 1-12 */
	unsigned char		day;
	unsigned char		hours;		/* range 0-23 */
	unsigned char		minutes;	/* range 0-59 */
	unsigned char		seconds;	/* range 0-59 */
}sms_time_abs_s;


typedef struct _sms_val_period_s {
	unsigned char 		format;
	union {
		sms_time_rel_s	rel_time;
		sms_time_abs_s	abs_time;
	}time;
}sms_val_period_s;


typedef struct _sms_encoding_specific_s {
	sms_encoding_type_t		encode_type;
	unsigned int			data_len;
	char 					user_data[SMS_MAX_USER_DATA_LEN +1];
}sms_encoding_specific_s;


typedef struct _sms_ctg_specific_s {
	unsigned char		operation_code;
	unsigned short		category;
	sms_language_type_t	language;
	unsigned char		max_msg;
	sms_alert_option_t	alert_option;
	unsigned int		data_len;
	char				user_data[SMS_MAX_USER_DATA_LEN +1];
}sms_ctg_specific_s;


typedef struct _sms_svc_ctg_program_data_s {
	sms_encoding_type_t		encode_type;
	unsigned int			num_data;
	sms_ctg_specific_s		*specific_data;
}sms_svc_ctg_program_data_s;


typedef struct _sms_telesvc_userdata_s {
	sms_encoding_type_t		encode_type;
	unsigned char			msg_type;
	unsigned int			data_len;
	unsigned char			user_data[SMS_MAX_USER_DATA_LEN +1];
}sms_telesvc_userdata_s;


typedef struct _sms_telesvc_cmasdata_s {
	unsigned int				data_len;
	sms_encoding_type_t		encode_type;
	unsigned char				alert_text[SMS_MAX_USER_DATA_LEN +1];
	sms_cmae_category_t		category;
	sms_cmae_response_type_t		response_type;
	sms_cmae_severity_t			severity;
	sms_cmae_urgency_t			urgency;
	sms_cmae_certainty_t			certainty;
	unsigned short		id;
	sms_cmae_alert_handle_t			alert_handle;		/* 00:Presidential 01:Extreme 02:Severe 03:AMBER 04:Test */
	sms_time_abs_s			expires;
	sms_language_type_t		language;
	bool 					is_wrong_recode_type;
}sms_telesvc_cmasdata_s;


typedef struct _sms_enhanced_vmn_s {
	sms_priority_indicator_t		priority;
	bool							password_req;
	bool							setup_req;
	bool							pw_change_req;
	unsigned char					min_pw_len;
	unsigned char					max_pw_len;
	unsigned char					vm_num_unheard_msg;
	bool							vm_mailbox_alm_full;
	bool							vm_mailbox_full;
	bool							reply_allowed;
	bool							fax_included;
	unsigned short				vm_len;
	unsigned char					vm_ret_day;
	unsigned short				vm_msg_id;
	unsigned short				vm_mailbox_id;

	sms_digit_mode_t				an_digit_mode;
	sms_number_type_t				an_number_type;
	sms_number_plan_t				an_number_plan;
	unsigned char					an_num_field;
	unsigned char					an_char[SMS_MAX_USER_DATA_LEN+1];

	sms_digit_mode_t				cli_digit_mode;
	sms_number_type_t				cli_number_type;
	sms_number_plan_t				cli_number_plan;
	unsigned char					cli_num_field;
	unsigned char					cli_char[SMS_MAX_USER_DATA_LEN+1];
}sms_enhanced_vmn_s;


typedef struct _sms_enhanced_vmn_ack_s {
	unsigned short				vm_mailbox_id;
	unsigned char					vm_num_unheard_msg;
	unsigned char					num_delete_ack;
	unsigned char					num_play_ack;

	unsigned short				da_vm_msg_id[SMS_MAX_NUMBER_OF_ACK+1];
	unsigned short				pa_vm_msg_id[SMS_MAX_NUMBER_OF_ACK+1];
}sms_enhanced_vmn_ack_s;


typedef struct _sms_telesvc_deliver_s {
	sms_trans_msg_id_s			msg_id;
	sms_telesvc_userdata_s		user_data;
	sms_telesvc_cmasdata_s		cmas_data;
	sms_time_abs_s				time_stamp;
	sms_val_period_s			val_period;
	sms_val_period_s			defer_val_period;
	sms_priority_indicator_t		priority;
	sms_privacy_indicator_t		privacy;
	sms_reply_option_s			reply_opt;
	unsigned int				num_msg;
	sms_alert_priority_t		alert_priority;
	sms_language_type_t			language;
	sms_telesvc_addr_s			callback_number;
	sms_display_mode_t			display_mode;
	sms_encoding_specific_s		multi_encode_data;
	unsigned short				deposit_id;
	/* sms_svc_ctg_program_data_s	svc_ctg; */
	sms_enhanced_vmn_s			enhanced_vmn;
	sms_enhanced_vmn_ack_s		enhanced_vmn_ack;
}sms_telesvc_deliver_s;


typedef struct _sms_telesvc_submit_s {
	sms_trans_msg_id_s			msg_id;
	sms_telesvc_userdata_s		user_data;
	sms_val_period_s			val_period;
	sms_val_period_s			defer_val_period;
	sms_priority_indicator_t	priority;
	sms_privacy_indicator_t		privacy;
	sms_reply_option_s			reply_opt;
	sms_alert_priority_t		alert_priority;
	sms_language_type_t			language;
	sms_telesvc_addr_s			callback_number;
	sms_encoding_specific_s		multi_encode_data;
	unsigned char				deposit_id;
	/* sms_svc_ctg_program_data_s	svc_ctg; */
}sms_telesvc_submit_s;


typedef struct _sms_telesvc_cancel_s {
	sms_trans_msg_id_s		msg_id;
}sms_telesvc_cancel_s;


typedef struct _sms_telesvc_user_ack_s {
	sms_trans_msg_id_s			msg_id;
	sms_telesvc_userdata_s		user_data;
	unsigned char				resp_code;
	sms_time_abs_s				time_stamp;
	sms_encoding_specific_s		multi_encode_data;
	unsigned char				deposit_id;
}sms_telesvc_user_ack_s;


typedef struct _sms_telesvc_deliver_ack_s {
	sms_trans_msg_id_s			msg_id;
	sms_telesvc_userdata_s		user_data;
	sms_time_abs_s				time_stamp;
	sms_encoding_specific_s		multi_encode_data;
	sms_status_code_t			msg_status;
}sms_telesvc_deliver_ack_s;


typedef struct _sms_telesvc_read_ack_s {
	sms_trans_msg_id_s			msg_id;
	sms_telesvc_userdata_s		user_data;
	sms_time_abs_s				time_stamp;
	sms_encoding_specific_s		multi_encode_data;
	unsigned char				deposit_id;
}sms_telesvc_read_ack_s;

typedef struct _sms_telesvc_deliver_report_s {
	sms_trans_msg_id_s		msg_id;
	unsigned char			tp_fail_cause;
	sms_telesvc_userdata_s	user_data;
	sms_language_type_t		language;
	unsigned char			multi_encode_data;
}sms_telesvc_report_s;


typedef struct _sms_telesvc_msg_s {
	sms_message_type_t		type;

	union {
		sms_telesvc_deliver_s			deliver;
		sms_telesvc_submit_s			submit;
		sms_telesvc_cancel_s			cancel;
		sms_telesvc_user_ack_s			user_ack;
		sms_telesvc_deliver_ack_s		delivery_ack;
		sms_telesvc_read_ack_s			read_ack;
		sms_telesvc_report_s 			report;
	}data;
}sms_telesvc_msg_s;



/********************************************************************************/
/*								TRANSPORT LAYER 								*/
/********************************************************************************/

typedef unsigned short 		sms_trans_param_id_t;		/* _sms_trans_param_id_e */

typedef unsigned short 		sms_trans_telesvc_id_t;		/* _sms_trans_telesvc_id_e */

typedef unsigned short 		sms_trans_svc_ctg_t;			/* _sms_trans_svc_ctg_e */

typedef unsigned char		sms_trans_reply_seq_t;


typedef enum _sms_trans_msg_type_e {
	SMS_TRANS_P2P_MSG			= 0x00,
	SMS_TRANS_BROADCAST_MSG		= 0x01,
	SMS_TRANS_ACK_MSG			= 0x02,
	SMS_TRANS_TYPE_RESERVED
}sms_trans_msg_type_t;


enum _sms_trans_param_id_e {
	SMS_TRANS_PARAM_TELESVC_IDENTIFIER		= 0x00,
	SMS_TRANS_PARAM_SERVICE_CATEGORY		= 0x01,
	SMS_TRANS_PARAM_ORG_ADDRESS				= 0x02,
	SMS_TRANS_PARAM_ORG_SUB_ADDRESS			= 0x02,
	SMS_TRANS_PARAM_DEST_ADDRESS			= 0x04,
	SMS_TRANS_PARAM_DEST_SUB_ADDRESS		= 0x05,
	SMS_TRANS_PARAM_BEARER_REPLY_OPTION		= 0x06,
	SMS_TRANS_PARAM_CAUSE_CODES				= 0x07,
	SMS_TRANS_PARAM_BEARER_DATA				= 0x08,
	SMS_TRANS_PARAM_RESERVED
};


enum _sms_trans_telesvc_id_e {
	SMS_TRANS_TELESVC_CMT_91		= 0x1000,	/* IS-91 Extended Protocol Enhanced Services */
	SMS_TRANS_TELESVC_CPT_95		= 0x1001,	/* Wireless Paging Teleservice */
	SMS_TRANS_TELESVC_CMT_95		= 0x1002,	/* Wireless Messaging Teleservice */
	SMS_TRANS_TELESVC_VMN_95		= 0x1003,	/* Voice Mail Notification */
	SMS_TRANS_TELESVC_WAP			= 0x1004,	/* Wireless Application Protocol */
	SMS_TRANS_TELESVC_WEMT			= 0x1005,	/* Wireless Enhanced Messaging Teleservice */
	SMS_TRANS_TELESVC_SCPT			= 0x1006,	/* Service Category Programming Teleservice */
	SMS_TRANS_TELESVC_CATPT			= 0x1007,	/* Card Application Toolkit Protocol Teleservice */
	SMS_TRANS_TELESVC_RESERVED		= 0xffff
};


enum _sms_trans_svc_ctg_e {
	SMS_TRANS_SVC_CTG_UNKNOWN				= 0x0000,
	SMS_TRANS_SVC_CTG_EMERGENCY				= 0x0001,
	SMS_TRANS_SVC_CTG_ADMINISTRATIVE		= 0x0002,
	SMS_TRANS_SVC_CTG_MAINTENANCE			= 0x0003,
	SMS_TRANS_SVC_CTG_GNEWS_LOCAL			= 0x0004,
	SMS_TRANS_SVC_CTG_GNEWS_REGIONAL		= 0x0005,
	SMS_TRANS_SVC_CTG_GNEWS_NATIONAL		= 0x0006,
	SMS_TRANS_SVC_CTG_GNEWS_INTERNATIONAL	= 0x0007,
	SMS_TRANS_SVC_CTG_BFNEWS_LOCAL			= 0x0008,
	SMS_TRANS_SVC_CTG_BFNEWS_REGIONAL		= 0x0009,
	SMS_TRANS_SVC_CTG_BFNEWS_NATIONAL		= 0x000a,
	SMS_TRANS_SVC_CTG_BFNEWS_INTERNATIONAL	= 0x000b,
	SMS_TRANS_SVC_CTG_SNEWS_LOCAL			= 0x000c,
	SMS_TRANS_SVC_CTG_SNEWS_REGIONAL		= 0x000d,
	SMS_TRANS_SVC_CTG_SNEWS_NATIONAL		= 0x000e,
	SMS_TRANS_SVC_CTG_SNEWS_INTERNATIONAL	= 0x000f,
	SMS_TRANS_SVC_CTG_ENEWS_LOCAL			= 0x0010,
	SMS_TRANS_SVC_CTG_ENEWS_REGIONAL		= 0x0011,
	SMS_TRANS_SVC_CTG_ENEWS_NATIONAL		= 0x0012,
	SMS_TRANS_SVC_CTG_ENEWS_INTERNATIONAL	= 0x0013,
	SMS_TRANS_SVC_CTG_LOCAL_WEATHER			= 0x0014,
	SMS_TRANS_SVC_CTG_TRAFFIC_REPORTS		= 0x0015,
	SMS_TRANS_SVC_CTG_FLIGHT_SCHEDULES		= 0x0016,
	SMS_TRANS_SVC_CTG_RESTAURANTS			= 0x0017,
	SMS_TRANS_SVC_CTG_LODGINGS				= 0x0018,
	SMS_TRANS_SVC_CTG_RETAIL_DIRECTORY		= 0x0019,
	SMS_TRANS_SVC_CTG_ADVERTISEMENTS		= 0x001a,
	SMS_TRANS_SVC_CTG_STOCK_QUOTES			= 0x001b,
	SMS_TRANS_SVC_CTG_EMPLOYMENT			= 0x001c,
	SMS_TRANS_SVC_CTG_MEDICAL				= 0x001d,
	SMS_TRANS_SVC_CTG_TECHNOLOGY_NEWS		= 0x001e,
	SMS_TRANS_SVC_CTG_MULTI_CTG				= 0x001f,
	SMS_TRANS_SVC_CTG_CATPT					= 0x0020,
	SMS_TRANS_SVC_CTG_KDDI_CORP_MIN1		= 0x0021,
	SMS_TRANS_SVC_CTG_KDDI_CORP_MAX1		= 0x003f,
	SMS_TRANS_SVC_CTG_CMAS_PRESIDENTIAL		= 0x1000,
	SMS_TRANS_SVC_CTG_CMAS_EXTREME			= 0x1001,
	SMS_TRANS_SVC_CTG_CMAS_SEVERE			= 0x1002,
	SMS_TRANS_SVC_CTG_CMAS_AMBER			= 0x1003,
	SMS_TRANS_SVC_CTG_CMAS_TEST				= 0x1004,
	SMS_TRANS_SVC_CTG_KDDI_CORP_MIN2		= 0x8001,
	SMS_TRANS_SVC_CTG_KDDI_CORP_MAX2		= 0x803f,
	SMS_TRANS_SVC_CTG_KDDI_CORP_MIN3		= 0xc001,
	SMS_TRANS_SVC_CTG_KDDI_CORP_MAX3		= 0xc03f,
	SMS_TRANS_SVC_CTG_RESERVED,
	SMS_TRANS_SVC_CTG_UNDEFINED				= 0x8001,
};


typedef enum _sms_trans_err_class_e {
	SMS_TRANS_ERR_CLASS_NONE				= 0x00,
	SMS_TRANS_ERR_CLASS_TEMPORARY			= 0x02,
	SMS_TRANS_ERR_CLASS_PERMANENT			= 0x03
}sms_trans_err_class_t;


typedef enum _sms_trans_cause_code_e {
	SMS_CAUSE_CODE_INVAILD_TELESERVICE_ID		= 0x04,
	SMS_CAUSE_CODE_SERVICE_TERMINATION_DENIED	= 0x62,
	SMS_TODO_FILL_THIS_ENUMS
}sms_trans_cause_code_t;


typedef enum _sms_trans_sub_addr_type_e {
	SMS_TRANS_SUB_ADDR_NSAP					= 0x00,
	SMS_TRANS_SUB_ADDR_USER					= 0x01,
	SMS_TRANS_SUB_ADDR_RESERVED
}sms_trans_sub_addr_type_t;



enum _sms_trans_dnet_addr_type_e {
	SMS_TRANS_DNET_UNKNOWN					= 0x00,
	SMS_TRANS_DNET_INTERNET_PROTOCOL		= 0x01,
	SMS_TRANS_DNET_INTERNET_MAIL_ADDR		= 0x02,
	SMS_TRANS_DNET_RESERVED
};


enum _sms_digit_mode_e {
	SMS_DIGIT_4BIT_DTMF	= 0,
	SMS_DIGIT_8BIT		= 1
};


enum _sms_number_mode_e {
	SMS_NUMBER_MODE_NONE_DATANETWORK	= 0,
	SMS_NUMBER_MODE_DATANETWORK			= 1,	/*using data network address format*/
};


enum _sms_dnet_number_type_e {
	SMS_ADDRESS_TYPE_UNKNOWN			= 0x00,
	SMS_ADDRESS_TYPE_INTERNET_PROTOCOL	= 0x01,
	SMS_ADDRESS_TYPE_EMAIL_ADDRESS		= 0x02,
};


enum _sms_number_type_e {
	SMS_NUMBER_TYPE_UNKNOWN	= 0x00,
	SMS_NUMBER_TYPE_INTERNATIONAL	= 0x01,
	SMS_NUMBER_TYPE_NATIONAL	= 0x02,
	SMS_NUMBER_TYPE_NETWORK_SPECIFIC	= 0x03,
	SMS_NUMBER_TYPE_SUBSCRIBER	= 0x04,
	SMS_NUMBER_TYPE_RESERVED_5	= 0x05,
	SMS_NUMBER_TYPE_ABBREVIATED	= 0x06,
	SMS_NUMBER_TYPE_RESERVED_7	= 0x07,
/*
 * TODO : check this type is valid or not
 *	SMS_NUMBER_TYPE_IP	= 0x11,
 *	SMS_NUMBER_TYPE_EMAILADDR	= 0x12,
 */
};


enum _sms_number_plan_e {
	SMS_NPI_UNKNOWN		= 0,
	SMS_NPI_ISDN		= 1,
	SMS_NPI_DATA		= 3,
	SMS_NPI_TELEX		= 4,
	SMS_NPI_PRIVATE		= 9,
	SMS_NPI_RESERVED	= 15,
};


typedef struct _sms_trans_addr_s {
	sms_digit_mode_t	digit_mode;
	sms_number_mode_t	number_mode;
	sms_number_type_t	number_type;
	sms_number_plan_t	number_plan;
	unsigned int		addr_len;
	char				szData[SMS_TRANS_ADDRESS_MAX_LEN + 1];
}sms_trans_addr_s;


typedef struct _sms_trans_sub_addr_s {
	sms_trans_sub_addr_type_t	type;
	bool						odd;
	unsigned int				addr_len;
	char						szData[SMS_TRANS_ADDRESS_MAX_LEN + 1];
}sms_trans_sub_addr_s;


typedef struct _sms_trans_cause_code_s {
	sms_trans_reply_seq_t			reply_seq;
	sms_trans_err_class_t			error_class;
	sms_trans_cause_code_t			cause_code;
}sms_trans_cause_code_s;


typedef struct _sms_trans_p2p_msg_s {
	sms_trans_telesvc_id_t			telesvc_id;
	sms_trans_svc_ctg_t				svc_ctg;
	sms_trans_addr_s				address;
	sms_trans_sub_addr_s			sub_address;
	sms_trans_reply_seq_t			reply_seq;
	sms_telesvc_msg_s				telesvc_msg;
}sms_trans_p2p_msg_s;


typedef struct _sms_trans_broadcast_msg_s {
	sms_trans_svc_ctg_t				svc_ctg;
	sms_telesvc_msg_s				telesvc_msg;
}sms_trans_broadcast_msg_s;


typedef struct _sms_trans_ack_msg_s {
	sms_trans_addr_s				address;
	sms_trans_sub_addr_s			sub_address;
	sms_trans_cause_code_s			cause_code;
}sms_trans_ack_msg_s;


typedef struct _sms_trans_msg_s {
	sms_trans_msg_type_t		type;
	union {
		sms_trans_p2p_msg_s			p2p_msg;
		sms_trans_broadcast_msg_s	cb_msg;
		sms_trans_ack_msg_s			ack_msg;
	}data;
}sms_trans_msg_s;


#endif /* SMS_CDMA_PLUGIN_TYPES_H */
