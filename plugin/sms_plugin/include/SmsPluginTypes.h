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

#ifndef SMS_PLUGIN_TYPES_H
#define SMS_PLUGIN_TYPES_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgInternalTypes.h"
#include "MsgStorageTypes.h"
#include "MsgSettingTypes.h"


/*==================================================================================================
                                    DEFINES
==================================================================================================*/
#define MAX_ADDRESS_LEN 			21 // including '+'
#define MAX_USER_DATA_LEN 			160
#define MAX_GSM_7BIT_DATA_LEN 		160
#define MAX_UCS2_DATA_LEN 			140
#define MAX_TPDU_DATA_LEN			165
#define MAX_SMSC_LEN 				20
#define MAX_ADD_PARAM_LEN			12
#define MAX_ABS_TIME_PARAM_LEN		7
#define MAX_REL_TIME_PARAM_LEN		1
#define MAX_UD_HEADER_NUM			7
#define MAX_SAT_TPDU_LEN			175
#define MAX_CBMSG_PAGE_SIZE		93
#define MAX_CBMSG_PAGE_NUM		15
#define MAX_SIM_SMS_NUM			90

#define SMS_PUSH_XML_HREF_TAG		"href"
#define SMS_PUSH_XML_SI_ID_TAG		"si-id"
#define SMS_PUSH_XML_CREATED_TAG	"created"
#define SMS_PUSH_XML_EXPIRES_TAG	"si-expires"
#define SMS_PUSH_XML_ACTION_TAG	"action"

#define SMS_PUSH_XML_INVAL_OBJ		"invalidate-object"
#define SMS_PUSH_XML_INVAL_SVC		"invalidate-service"
#define SMS_PUSH_XML_CO_URI		"uri"


/*==================================================================================================
                                         TYPES
==================================================================================================*/

/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_TPDU_TYPE_T;

/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_VPF_T;

/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_TON_T;

/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_NPI_T;

/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_PID_T;


/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_MSG_CLASS_T;


/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_CODING_SCHEME_T;


/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_CODING_GROUP_T;


/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_INDICATOR_TYPE_T;


/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_TIME_FORMAT_T;


/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_FAIL_CAUSE_T;


/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_STATUS_T;


/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_REF_NUMBER_T;


/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_REPORT_TYPE_T;


/**
\brief Represents TPDU Type.
*/
typedef unsigned char SMS_UDH_TYPE_T;


/**
\brief Represents WAP Push App Code.
*/
typedef unsigned char SMS_WAP_APP_CODE_T;


typedef unsigned char SMS_CB_NETWORK_TYPE_T;


typedef unsigned char SMS_CBMSG_TYPE_T;


typedef unsigned char SMS_CBMSG_LANG_TYPE_T;


typedef unsigned char SMS_CBMSG_CODING_GROUP_T;


typedef unsigned char SMS_SAT_CMD_TYPE_T;


typedef unsigned short SMS_SIM_EFILE_NAME_T;


typedef unsigned char SMS_LANGUAGE_ID_T;


/*==================================================================================================
                                    ENUMS
==================================================================================================*/
enum _SMS_TPDU_TYPE_E
{
	SMS_TPDU_SUBMIT,
	SMS_TPDU_DELIVER,
	SMS_TPDU_DELIVER_REP,
	SMS_TPDU_STATUS_REP,
};


enum _SMS_VPF_E
{
	SMS_VPF_NOT_PRESENT = 0,
	SMS_VPF_ENHANCED,
	SMS_VPF_RELATIVE,
	SMS_VPF_ABSOLUTE,
};


enum _SMS_TON_E
{
	SMS_TON_UNKNOWN = 0,		/* unknown */
	SMS_TON_INTERNATIONAL,	/* international */
	SMS_TON_NATIONAL,			/* national */
	SMS_TON_NETWORK,			/* network */
	SMS_TON_SUBSCRIBER,		/* subscriber */
	SMS_TON_ALPHANUMERIC,		/* alpha numberic */
	SMS_TON_ABBREVIATED,		/* abbreviated */
	SMS_TON_RESERVE,			/* reserve */
};


enum _SMS_NPI_E
{
	SMS_NPI_UNKNOWN = 0,		/* unknown */
	SMS_NPI_ISDN = 1,			/* idsn */
	SMS_NPI_DATA = 3,			/* data */
	SMS_NPI_TELEX = 4,			/* telex */
	SMS_NPI_SMSC = 5,			/* smsc */
	SMS_NPI_NATIONAL = 8,		/* national */
	SMS_NPI_PRIVATE = 9,			/* private */
	SMS_NPI_ERMES = 10,			/* ermes */
};


enum _SMS_PID_E
{
	SMS_PID_NORMAL = 0x00,

	SMS_PID_TELEX = 0x21,
	SMS_PID_GROUP3_TELEX = 0x22,
	SMS_PID_GROUP4_TELEX = 0x23,
	SMS_PID_VOICE = 0x24,
	SMS_PID_ERMES = 0x25,
	SMS_PID_NPS = 0x26,
	SMS_PID_VIDEO = 0x27,
	SMS_PID_TELETEX_UNSPEC = 0x28,
	SMS_PID_TELETEX_PSPDN = 0x29,
	SMS_PID_TELETEX_CSPDN = 0x2A,
	SMS_PID_TELETEX_PSTN = 0x2B,
	SMS_PID_TELETEX_ISDN = 0x2C,
	SMS_PID_UCI = 0x2D,
	SMS_PID_x400 = 0x31,
	SMS_PID_EMAIL = 0x32,

	SMS_PID_TYPE0 = 0x40,
	SMS_PID_REPLACE_TYPE1 = 0x41,
	SMS_PID_REPLACE_TYPE2 = 0x42,
	SMS_PID_REPLACE_TYPE3 = 0x43,
	SMS_PID_REPLACE_TYPE4 = 0x44,
	SMS_PID_REPLACE_TYPE5 = 0x45,
	SMS_PID_REPLACE_TYPE6 = 0x46,
	SMS_PID_REPLACE_TYPE7 = 0x47,

	SMS_PID_RETURN_CALL = 0x5F,
};


enum _SMS_MSG_CLASS_E
{
	SMS_MSG_CLASS_0 = 0,
	SMS_MSG_CLASS_1,
	SMS_MSG_CLASS_2,
	SMS_MSG_CLASS_3,
	SMS_MSG_CLASS_NONE,
};


enum _SMS_CODING_SCHEME_E
{
	SMS_CHARSET_7BIT = 0,
	SMS_CHARSET_8BIT,
	SMS_CHARSET_UCS2,
	SMS_CHARSET_AUTO,
	SMS_CHARSET_EUCKR,
};


enum _SMS_CODING_GROUP_E
{
	SMS_GROUP_GENERAL = 0,
	SMS_GROUP_CODING_CLASS,
	SMS_GROUP_DELETION,
	SMS_GROUP_DISCARD,
	SMS_GROUP_STORE,
	SMS_GROUP_UNKNOWN,
};


enum _SMS_INDICATOR_TYPE_E
{
	SMS_VOICE_INDICATOR = 0,
	SMS_FAX_INDICATOR,
	SMS_EMAIL_INDICATOR,
	SMS_OTHER_INDICATOR,
};


enum _SMS_TIME_FORMAT_E
{
	SMS_TIME_RELATIVE = 0,
	SMS_TIME_ABSOLUTE
};


enum _SMS_FAIL_CAUSE_E
{
	SMS_FC_MSG_TYPE0_NOT_SUPPORTED = 0x81,
	SMS_FC_SM_CANNOT_BE_REPLACED = 0x82,
	SMS_FC_UNSPEC_PID_ERROR = 0x8F,
	SMS_FC_MSG_CLASS_NOT_SUPPORTED = 0x91,
	SMS_FC_UNSPEC_DCS_ERROR = 0x9F,
	SMS_FC_TPDU_NOT_SUPPORTED = 0xB0,
	SMS_FC_SIM_STORAGE_FULL = 0xD0,
	SMS_FC_NO_STORAGE_IN_SIM = 0xD1,
	SMS_FC_ERROR_IN_MS = 0xD2,
	SMS_FC_MSG_CAPA_EXCEEDED = 0xD3,
	SMS_FC_SAT_BUSY = 0xD4,
	SMS_FC_SIM_DOWNLOAD_ERROR = 0xD5,
	SMS_FC_UNSPEC_ERROR = 0xFF,
};


enum _SMS_STATUS_E
{
	SMS_STATUS_RECEIVE_SUCCESS = 0x00,
	SMS_STATUS_UNABLE_TO_CONFIRM_DELIVER = 0x01,
	SMS_STATUS_REPLACED = 0x02,
	SMS_STATUS_SMSC_SPECIFIC_LAST = 0x1F,

	SMS_STATUS_TRY_CONGESTION = 0x20,
	SMS_STATUS_TRY_SME_BUSY = 0x21,
	SMS_STATUS_TRY_NO_RESPONSE = 0x22,
	SMS_STATUS_TRY_SERVICE_REJECTED = 0x23,
	SMS_STATUS_TRY_QOS_NOT_AVAILABLE = 0x24,

	SMS_STATUS_PERM_REMOTE_ERROR = 0x40,
	SMS_STATUS_PERM_IMCOMPATIBLE_DEST = 0x41,
	SMS_STATUS_PERM_CONNECTION_REJECTED = 0x42,
	SMS_STATUS_PERM_NOT_OBTAINABLE = 0x43,
	SMS_STATUS_PERM_QOS_NOT_AVAILABLE = 0x44,
	SMS_STATUS_PERM_NO_INTERWORK_AVAILABLE = 0x45,
	SMS_STATUS_PERM_MSG_VAL_PERIOD_EXPIRED = 0x46,
	SMS_STATUS_PERM_MSG_DEL_BY_ORIGIN_SME = 0x47,
	SMS_STATUS_PERM_MSG_DEL_BY_SMSC_ADMIN = 0x48,
	SMS_STATUS_PERM_MSG_NOT_EXIST = 0x49,

	SMS_STATUS_TEMP_CONGESTION = 0x60,
	SMS_STATUS_TEMP_SME_BUSY = 0x61,
	SMS_STATUS_TEMP_NO_RESPONSE = 0x62,
	SMS_STATUS_TEMP_SERVICE_REJECTED = 0x63,
	SMS_STATUS_TEMP_QOS_NOT_AVAILABLE = 0x64,
	SMS_STATUS_TEMP_ERROR_IN_SME = 0x65,
};


enum _SMS_REF_NUMBER_E
{
	SMS_REF_NUM_8BIT = 0,
	SMS_REF_NUM_16BIT
};


enum _SMS_REPORT_TYPE_E
{
	SMS_REPORT_POSITIVE = 0,
	SMS_REPORT_NEGATIVE
};


enum _SMS_UDH_TYPE_E
{
	SMS_UDH_CONCAT_8BIT = 0x00,
	SMS_UDH_SPECIAL_SMS = 0x01,
	// 0x02, 0x03 - Reserved
	SMS_UDH_APP_PORT_8BIT = 0x04,
	SMS_UDH_APP_PORT_16BIT = 0x05,
	SMS_UDH_SC_CONTROL = 0x06,
	SMS_UDH_SRC_IND = 0x07,
	SMS_UDH_CONCAT_16BIT = 0x08,
	SMS_UDH_WCMP = 0x09,
	SMS_UDH_ALTERNATE_REPLY_ADDRESS = 0x22,
	SMS_UDH_SINGLE_SHIFT = 0x24,
	SMS_UDH_LOCKING_SHIFT = 0x25,
	SMS_UDH_NONE = 0xFF,
};


enum _SMS_WAP_APP_CODE_E
{
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


enum _SMS_CB_NETWORK_TYPE_E
{
	SMS_CB_NETWORK_TYPE_2G_GSM = 1,
	SMS_CB_NETWORK_TYPE_3G_UMTS,
};


enum _SMS_CBMSG_TYPE_E
{
	SMS_CBMSG_TYPE_CBS = 1,		/**< CBS */
	SMS_CBMSG_TYPE_SCHEDULE,		/**< Schedule */
	SMS_CBMSG_TYPE_CBS41,			/**< CBS41 */
	SMS_CBMSG_TYPE_JAVACBS,		/**< JAVA-CB Message*/
};


enum _SMS_CBMSG_LANG_TYPE_E
{
	SMS_CBMSG_LANG_GERMAN      		= 0x00,
	SMS_CBMSG_LANG_ENGLISH     		= 0x01,
	SMS_CBMSG_LANG_ITALIAN     		= 0x02,
	SMS_CBMSG_LANG_FRENCH      		= 0x03,
	SMS_CBMSG_LANG_SPANISH     		= 0x04,
	SMS_CBMSG_LANG_DUTCH       		= 0x05,
	SMS_CBMSG_LANG_SWEDISH     	= 0x06,
	SMS_CBMSG_LANG_DANISH      		= 0x07,
	SMS_CBMSG_LANG_PORTUGUESE  	= 0x08,
	SMS_CBMSG_LANG_FINNISH     		= 0x09,
	SMS_CBMSG_LANG_NORWEGIAN   	= 0x0a,
	SMS_CBMSG_LANG_GREEK       		= 0x0b,
	SMS_CBMSG_LANG_TURKISH     		= 0x0c,
	SMS_CBMSG_LANG_HUNGARIAN  	= 0x0d,
	SMS_CBMSG_LANG_POLISH      		= 0x0e,
	SMS_CBMSG_LANG_UNSPECIFIED 	= 0x0f,

	SMS_CBMSG_LANG_ISO639		= 0x10,

	SMS_CBMSG_LANG_CZECH       		= 0x20,
	SMS_CBMSG_LANG_HEBREW 		= 0x21,
	SMS_CBMSG_LANG_ARABIC 		= 0x22,
	SMS_CBMSG_LANG_RUSSIAN 		= 0x23,
	SMS_CBMSG_LANG_ICELANDIC 		= 0x24,
	SMS_CBMSG_LANG_RESERVED_25 	= 0x25,
	SMS_CBMSG_LANG_RESERVED_26	= 0x26,
	SMS_CBMSG_LANG_RESERVED_27 	= 0x27,
	SMS_CBMSG_LANG_RESERVED_28 	= 0x28,
	SMS_CBMSG_LANG_RESERVED_29 	= 0x29,
	SMS_CBMSG_LANG_RESERVED_2A 	= 0x2a,
	SMS_CBMSG_LANG_RESERVED_2B 	= 0x2b,
	SMS_CBMSG_LANG_RESERVED_2C 	= 0x2c,
	SMS_CBMSG_LANG_RESERVED_2D	= 0x2d,
	SMS_CBMSG_LANG_RESERVED_2E 	= 0x2e,
	SMS_CBMSG_LANG_RESERVED_2F 	= 0x2f,

	SMS_CBMSG_LANG_DUMMY       		= 0xFF
};


enum _SMS_CBMSG_CODING_GROUP_E
{
	SMS_CBMSG_CODGRP_GENERAL_DCS,			/**< Bits 7..4 00xx */
	SMS_CBMSG_CODGRP_WAP,					/**< 1110 Cell Broadcast */
	SMS_CBMSG_CODGRP_CLASS_CODING,			/**< 1111 Cell Broadcast */
};


enum _SMS_SAT_CMD_TYPE_E
{
	SMS_SAT_CMD_REFRESH,
	SMS_SAT_CMD_SEND_SMS,
	SMS_SAT_CMD_SMS_CTRL,
};


enum _SMS_SIM_EFILE_NAME_E
{
	SMS_SIM_EFILE_USIM_SMS	= 0x6A3C,		/** < USIM Short Messages file */
	SMS_SIM_EFILE_USIM_SMSP	= 0x6A42,		/** < USIM SMS parameter */
	SMS_SIM_EFILE_USIM_SMSS	= 0x6A43,		/** < USIM SMS status */
	SMS_SIM_EFILE_USIM_CBMI	= 0x6A45,		/** < USIM Cell Broadcast Message Identifier */
	SMS_SIM_EFILE_USIM_MBDN	= 0x6FC7, 		/** < USIM Mail Box Dialing Number */
 	SMS_SIM_EFILE_SMS			= 0x6F3C,		/** < Short Messages file */
	SMS_SIM_EFILE_SMSP			= 0x6F42,		/** < SMS Parameter */
	SMS_SIM_EFILE_SMSS			= 0x6F43,		/** < SMS Status */
	SMS_SIM_EFILE_CBMI			= 0x6F45,		/** < Cell Broadcast Message Identifier */
	SMS_SIM_EFILE_MBDN		= 0x6FC7,		/** < Mail Box Dialing Number */
};


enum _SMS_LANGUAGE_ID_E
{
	SMS_LANG_ID_RESERVED = 0,
	SMS_LANG_ID_TURKISH,
	SMS_LANG_ID_SPANISH,
	SMS_LANG_ID_PORTUGUESE,
	SMS_LANG_ID_BENGALI,
	SMS_LANG_ID_GUJARATI,
	SMS_LANG_ID_HINDI,
	SMS_LANG_ID_KANNADA,
	SMS_LANG_ID_MALAYALAM,
	SMS_LANG_ID_ORIYA,
	SMS_LANG_ID_PUNJABI,
	SMS_LANG_ID_TAMIL,
	SMS_LANG_ID_TELUGU,
	SMS_LANG_ID_URDU,
};


/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/
typedef struct _SMS_ADDRESS_S
{
	SMS_TON_T 		ton;
	SMS_NPI_T 		npi;
	char 				address[MAX_ADDRESS_LEN+1];        /* < null terminated string */
} SMS_ADDRESS_S;


typedef struct _SMS_DCS_S
{
	bool						bCompressed;
	bool						bMWI;
	bool						bIndActive;
	SMS_MSG_CLASS_T			msgClass;
	SMS_CODING_SCHEME_T	codingScheme;
	SMS_CODING_GROUP_T		codingGroup;
	SMS_INDICATOR_TYPE_T		indType;
} SMS_DCS_S;


typedef struct _SMS_TIME_REL_S
{
	unsigned char time;
} SMS_TIME_REL_S;


typedef struct _SMS_TIME_ABS_S
{
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char timeZone;
} SMS_TIME_ABS_S;


typedef struct _SMS_TIMESTAMP_S
{
	SMS_TIME_FORMAT_T format;

	union
	{
		SMS_TIME_REL_S 	relative;
		SMS_TIME_ABS_S 	absolute;
	} time;
} SMS_TIMESTAMP_S;


typedef struct _SMS_CONCAT_8BIT_S
{
	unsigned char		msgRef;
	unsigned char		totalSeg;
	unsigned char		seqNum;
} SMS_CONCAT_8BIT_S;


typedef struct _SMS_CONCAT_16BIT_S
{
	unsigned short		msgRef;
	unsigned char		totalSeg;
	unsigned char		seqNum;
} SMS_CONCAT_16BIT_S;


typedef struct _SMS_APP_PORT_8BIT_S
{
	unsigned char		destPort;
	unsigned char		originPort;
} SMS_APP_PORT_8BIT_S;


typedef struct _SMS_APP_PORT_16BIT_S
{
	unsigned short		destPort;
	unsigned short		originPort;
} SMS_APP_PORT_16BIT_S;


typedef struct _SMS_SPECIAL_INDICATION_S
{
	bool				bStore;
	unsigned short		msgInd;
	unsigned short		waitMsgNum;
} SMS_SPECIAL_INDICATION_S;


typedef struct _SMS_SINGLE_SHIFT_S
{
	SMS_LANGUAGE_ID_T	langId;
} SMS_SINGLE_SHIFT_S;


typedef struct _SMS_LOCKING_SHIFT_S
{
	SMS_LANGUAGE_ID_T	langId;
} SMS_LOCKING_SHIFT_S;


typedef struct _SMS_UDH_S
{
	SMS_UDH_TYPE_T udhType;

	union
	{
		SMS_CONCAT_8BIT_S			concat8bit;
		SMS_CONCAT_16BIT_S			concat16bit;
		SMS_APP_PORT_8BIT_S		appPort8bit;
		SMS_APP_PORT_16BIT_S		appPort16bit;
		SMS_SPECIAL_INDICATION_S 	specialInd;
		SMS_SINGLE_SHIFT_S			singleShift;
		SMS_LOCKING_SHIFT_S			lockingShift;
		SMS_ADDRESS_S				alternateAddress;
	} udh;
} SMS_UDH_S;


typedef struct _SMS_USERDATA_S
{
	int			headerCnt;
	SMS_UDH_S	header[MAX_UD_HEADER_NUM];
	int 			length;
	char 			data[MAX_USER_DATA_LEN+1];
} SMS_USERDATA_S;


typedef struct _SMS_TPUD_S
{
	int		udl;
	char 		ud[MAX_USER_DATA_LEN+1];
} SMS_TPUD_S;


typedef struct _SMS_SUBMIT_S
{
	bool				bRejectDup;
	bool				bStatusReport;
	bool				bHeaderInd;
	bool				bReplyPath;
	unsigned char 		msgRef;
	SMS_VPF_T		vpf;
	SMS_ADDRESS_S	destAddress;
	SMS_PID_T		pid;
	SMS_DCS_S		dcs;
	SMS_TIMESTAMP_S	validityPeriod;
	SMS_USERDATA_S	userData;
} SMS_SUBMIT_S;


typedef struct _SMS_SUBMIT_DATA_S
{
	SMS_ADDRESS_S	destAddress;
	unsigned int		segCount;
	SMS_USERDATA_S	userData[MAX_SEGMENT_NUM];
} SMS_SUBMIT_DATA_S;


typedef struct _SMS_DELIVER_S
{
	bool				bMoreMsg;
	bool				bStatusReport;
	bool				bHeaderInd;
	bool				bReplyPath;
	SMS_ADDRESS_S	originAddress;
	SMS_PID_T 	 	pid;
	SMS_DCS_S		dcs;
	SMS_TIMESTAMP_S	timeStamp;
	SMS_USERDATA_S	userData;
	SMS_TPUD_S		udData;
} SMS_DELIVER_S;


typedef struct _SMS_DELIVER_REPORT_S
{
	SMS_REPORT_TYPE_T	reportType;
	bool					bHeaderInd;
	SMS_FAIL_CAUSE_T		failCause;
	unsigned char 			paramInd;
	SMS_PID_T 	 		pid;
	SMS_DCS_S			dcs;
	SMS_USERDATA_S		userData;
} SMS_DELIVER_REPORT_S;


typedef struct _SMS_STATUS_REPORT_S
{
	bool				bMoreMsg;
	bool				bStatusReport;
	bool				bHeaderInd;
	unsigned char 		msgRef;
	SMS_ADDRESS_S	recipAddress;
	SMS_TIMESTAMP_S	timeStamp;
	SMS_TIMESTAMP_S	dischargeTime;
	SMS_STATUS_T	status;
	unsigned char 		paramInd;
	SMS_PID_T 	 	pid;
	SMS_DCS_S		dcs;
	SMS_USERDATA_S	userData;
} SMS_STATUS_REPORT_S;


typedef struct _SMS_TPDU_S
{
	SMS_TPDU_TYPE_T tpduType;

	union
	{
		SMS_SUBMIT_S			submit;
		SMS_DELIVER_S			deliver;
		SMS_DELIVER_REPORT_S	deliverRep;
		SMS_STATUS_REPORT_S		statusRep;
	} data;
} SMS_TPDU_S;


typedef struct
{
	msg_request_id_t				reqId;		/**< Indicates the request ID, which is unique. When applications submit a request to the framework, this value will be set by the framework. */
	MSG_MESSAGE_INFO_S			msgInfo;	/**< Indicates the message structure to be sent by applications. */
	MSG_SENDINGOPT_INFO_S	sendOptInfo;
} SMS_REQUEST_INFO_S;


typedef struct _SMS_SENT_INFO_S
{
	SMS_REQUEST_INFO_S		reqInfo;		/**< Indicates the corresponding request structure. */
	bool						bLast;
} SMS_SENT_INFO_S;


typedef struct _SMS_PUSH_APP_INFO_S
{
	char* 				pContentType;
	char* 				pAppId;
	SMS_WAP_APP_CODE_T 	appCode;
} SMS_PUSH_APP_INFO_S;


typedef struct _SMS_CBMSG_SERIAL_NUM_S
{
	unsigned char		geoScope;
	unsigned char		updateNum;
	unsigned short		msgCode;
} SMS_CBMSG_SERIAL_NUM_S;


typedef struct _SMS_CBMSG_DCS_S
{
	SMS_CBMSG_CODING_GROUP_T		codingGroup;		/**< Coding group, GSM 03.38 */
	SMS_MSG_CLASS_T					classType;		/**< The message class */
	bool								bCompressed;		/**< if text is compressed this is TRUE */
	SMS_CODING_SCHEME_T			codingScheme;	/**< How to encode a message. */
	SMS_CBMSG_LANG_TYPE_T   			langType;
	unsigned char              				iso639Lang[3]; 	/* 2 GSM chars and a CR char */
	bool								bUDH;
	unsigned char                       			rawData;
} SMS_CBMSG_DCS_S;


typedef struct _SMS_CBMSG_HEADER_S
{
	SMS_CBMSG_SERIAL_NUM_S		serialNum;		/**< Cell Broadcast Serial number */
	unsigned short					msgId;			/**< Message identifier code */
	MSG_CB_LANGUAGE_TYPE_T		langType;		/**< Languages in CB Messages */
	SMS_CBMSG_DCS_S			dcs;				/**< Data coding scheme */
	unsigned char					page;			/**< current page number */
	unsigned char					totalPages;		/**< Total number of pages in this messages */
	time_t						recvTime;		/**< Msg Recv Time */
} SMS_CBMSG_HEADER_S;


typedef struct _SMS_CBMSG_PAGE_S
{
	SMS_CBMSG_TYPE_T			cbMsgType;							/*CBS Msg or SCHEDULE Msg or CBS41 Msg */
	SMS_CBMSG_HEADER_S		 	pageHeader;							/**< CB Message Header */
	int							pageLength;							/**< message string length */
	char							pageData[MAX_CBMSG_PAGE_SIZE+1];		/**< user data */
} SMS_CBMSG_PAGE_S;


typedef struct _SMS_CBMSG_S
{
	SMS_CBMSG_TYPE_T			cbMsgType;							/*CBS Msg or SCHEDULE Msg or CBS41 Msg */
	unsigned short					msgId;								/**< Message identifier code */
	SMS_MSG_CLASS_T				classType;							/**< The message class */
	SMS_CODING_SCHEME_T		codingScheme;						/**< How to encode a message. */
	time_t						recvTime;							/**< Msg Recv Time */
	int							msgLength;							/**< message string length */
	char							msgData[MAX_CBMSG_PAGE_SIZE*MAX_CBMSG_PAGE_NUM+1];		/**< user data */
} SMS_CBMSG_S;


typedef struct _SMS_LANG_INFO_S
{
	bool							bSingleShift;
	bool							bLockingShift;

	SMS_LANGUAGE_ID_T			singleLang;
	SMS_LANGUAGE_ID_T			lockingLang;
} SMS_LANG_INFO_S;


typedef struct _SMS_WSP_CONTENTS_TYPE_S
{
	char*         contentsTypeName;
	unsigned char contentsTypeCode;
} SMS_WSP_CONTENTS_TYPE_S;


typedef struct _SMS_WSP_CHARSET_S
{
	char*  charsetName;
	unsigned short charsetCode;
} SMS_WSP_CHARSET_S;


typedef struct _SMS_WAP_UNREGISTER_CONTENTS_TYPE_S
{
	char*         contentsTypeName;
	unsigned short contentsTypeCode;
} SMS_WAP_UNREGISTER_CONTENTS_TYPE_S;


typedef struct _SMS_WSP_LANGUAGE_S
{
	char*         languageName;
	unsigned char languageCode;
} SMS_WSP_LANGUAGE_S;


typedef struct _SMS_WSP_HEADER_PARAMETER_S
{
	char*         parameterToken;
	unsigned char parameterCode;
} SMS_WSP_HEADER_PARAMETER_S;


typedef struct _SMS_WSP_METHOD_TYPE_S
{
	char*         methodName;
	unsigned char methodCode;
} SMS_WSP_METHOD_TYPE_S;


typedef struct _SMS_WSP_SECURITY_TYPE_S
{
	char*         SecurityTypeName;
	unsigned char SecurityTypeCode;
}SMS_WSP_SECURITY_TYPE_S;


/**
 *	@brief	Represents SIM count information.
 */
typedef struct
{
	unsigned int	totalCount;			/**< The total number of SIM */
	int			usedCount;			/**< The used number of SIM */
	int			indexList[MAX_SIM_SMS_NUM];	/**< The SIM index list */
}MSG_SIM_COUNT_S;


/**
 *	@brief	Represents Concat SIM Msg information.
 */
typedef struct
{
	unsigned int		simIdCnt;											/**< The total number of SIM Msg ID*/
	msg_sim_id_t		simIdList[MAX_SEGMENT_NUM];							/**< The SIM Msg ID List */
	char				msgData[(MAX_MSG_DATA_LEN*MAX_SEGMENT_NUM)+1];		/**< user data */
} SMS_CONCAT_SIM_MSG_S;

#endif //SMS_PLUGIN_TYPES_H

