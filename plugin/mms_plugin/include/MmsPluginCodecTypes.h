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

#ifndef MMS_PLUGIN_CODEC_TYPE_H
#define MMS_PLUGIN_CODEC_TYPE_H

#include "MmsPluginTypes.h"

#define	MMS_MAJOR_VERSION			1

#if defined(MMS_V1_1)
#define	MMS_MINOR_VERSION			1
#define	MMS_VERSION					0x91

#elif defined(MMS_V1_2)
#define	MMS_MINOR_VERSION			2
#define	MMS_VERSION					0x92

#else
#define	MMS_MINOR_VERSION			0
#define	MMS_VERSION					0x90
#endif

#define	MSG_MMS_ENCODE_BUFFER_MAX	(2 * 1024)
#define	MSG_MMS_DECODE_BUFFER_MAX	(2 * 1024)
#define	MSB								0x80
#define	QUOTE							0x7F
#define	MARK							0x22	// "
#define	LENGTH_QUOTE					0x1F
#define	MSG_STDSTR_LONG				0xFF
#define	INVALID_VALUE					-1

#define	MMS_CONTENT_ID_LEN				100
#define	MSG_DATE_LEN						50
#define	MSG_SUBJ_LEN						40
#define	MSG_LOCALE_SUBJ_LEN				(3 * MSG_SUBJ_LEN)
#define	MSG_LOCAL_TEMP_BUF_SIZE 1024
#define	MSG_ADDR_LEN						MAX_ADDRESS_VAL_LEN
#define	MMS_LOCALE_RESP_TEXT_LEN		90
#define	MSG_LOCALE_ADDR_LEN				(3 * MAX_ADDRESS_VAL_LEN)
#define	MSG_BOUNDARY_LEN					70
#define	MSG_LOCALE_NAME_LEN				150
#define	MSG_LOCALE_FILENAME_LEN_MAX		(3 * MSG_FILENAME_LEN_MAX)
#define	MMS_READ_REPORT_STRING_READ		"Read:"
#define	MMS_READ_REPORT_STRING_DELETED	"Deleted:"

#define	MSG_ATTACH_MAX		20
#define	MSG_STR_ADDR_DELIMETER			";"

#define	MMS_TEXT_LEN 2000
#define	MSG_CH_ADDR_DELIMETER ';'
#define	MSG_MMS_STR_ADDR_DELIMETER	MSG_STR_ADDR_DELIMETER
#define	MSG_MMS_CH_ADDR_DELIMETER	MSG_CH_ADDR_DELIMETER


typedef unsigned int 			UINT;
typedef unsigned long int		UINT32;
typedef unsigned char			UINT8;
typedef unsigned short int		UINT16;
typedef unsigned long			ULONG;
typedef unsigned char			UCHAR;
typedef unsigned short			MCHAR;
typedef unsigned short			USHORT;

typedef struct _MsgBody		MsgBody;
typedef struct _MsgMultipart	MsgMultipart;

typedef enum {
	MMS_CODE_BCC,
	MMS_CODE_CC,
	MMS_CODE_CONTENTLOCATION,
	MMS_CODE_CONTENTTYPE,
	MMS_CODE_DATE,
	MMS_CODE_DELIVERYREPORT,
	MMS_CODE_DELIVERYTIME,
	MMS_CODE_EXPIRYTIME,
	MMS_CODE_FROM,
	MMS_CODE_MSGCLASS,
	MMS_CODE_MSGID,
	MMS_CODE_MSGTYPE,
	MMS_CODE_VERSION,
	MMS_CODE_MSGSIZE,
	MMS_CODE_PRIORITY,
	MMS_CODE_READREPLY,
	MMS_CODE_REPORTALLOWED,
	MMS_CODE_RESPONSESTATUS,
	MMS_CODE_RETRIEVESTATUS,		/* Add by MMSENC v1.1 */
	MMS_CODE_RESPONSETEXT,
	MMS_CODE_RETRIEVETEXT,			/* Add by MMSENC v1.1 */
	MMS_CODE_SENDERVISIBILLITY,
	MMS_CODE_MSGSTATUS,
	MMS_CODE_SUBJECT,
	MMS_CODE_TO,
	MMS_CODE_TRID,

	/* Add by MMSENC v1.1 */
	MMS_CODE_READSTATUS,
	MMS_CODE_REPLYCHARGING,
	MMS_CODE_REPLYCHARGINGDEADLINE,
	MMS_CODE_REPLYCHARGINGID,
	MMS_CODE_REPLYCHARGINGSIZE,
	MMS_CODE_PREVIOUSLYSENTBY,
	MMS_CODE_PREVIOUSLYSENTDATE,

	MMS_CODE_TRANSFERENCODING,
	MMS_CODE_DISPOSITION,
	MMS_CODE_CONTENT_ID
} MmsFieldCode;

enum {
	MMS_BODYHDR_TRANSFERENCODING,
	MMS_BODYHDR_DISPOSITION,
	MMS_BODYHDR_CONTENTID,
	MMS_BODYHDR_CONTENTLOCATION,
	MMS_BODYHDR_X_OMA_DRM_SEPARATE_DELIVERY,	// DRM RO WAITING
};

typedef enum {
	//code
	MmsCodeFieldCode,
	MmsCodeParameterCode,
	MmsCodeMsgBodyHeaderCode,

	//Data
	MmsCodeMsgType,
	MmsCodeDeliveryReport,
	MmsCodeTimeType,
	MmsCodeMsgClass,
	MmsCodePriority,
	MmsCodeResponseStatus,
	MmsCodeRetrieveStatus,
	MmsCodeReadReply,
	MmsCodeReportAllowed,
	MmsCodeSenderVisibility,
	MmsCodeMsgStatus,
	MmsCodeReadStatus,
	MmsCodeAddressType,
	MmsCodeCharSet,
	MmsCodeReplyCharging,

	MmsCodeContentType,

	MmsCodeMsgDisposition,
	MmsCodeContentTransferEncoding
} MmsCode;

typedef enum {
	MMS_DATATYPE_NONE = -1,
	MMS_DATATYPE_READ = 0,
	MMS_DATATYPE_SENT = 1,
	MMS_DATATYPE_NOTIFY = 2,
	MMS_DATATYPE_UNSENT = 3,
	MMS_DATATYPE_DRAFT = 4,
	MMS_DATATYPE_SENDING = 6,
	MMS_DATATYPE_RETRIEVING = 7,
	MMS_DATATYPE_UNRETV = 8,
	MMS_DATATYPE_TEMPLATE = 9,
	MMS_DATATYPE_DRM_RO_WAITING = 10	// DRM RO WAITING
} MmsDataType;

typedef enum {
	MMS_MSGTYPE_ERROR = -1,	// error return in Get method
	MMS_MSGTYPE_SEND_REQ = 0,	// default
	MMS_MSGTYPE_SEND_CONF = 1,
	MMS_MSGTYPE_NOTIFICATION_IND = 2,
	MMS_MSGTYPE_NOTIFYRESP_IND = 3,
	MMS_MSGTYPE_RETRIEVE_CONF = 4,
	MMS_MSGTYPE_ACKNOWLEDGE_IND = 5,
	MMS_MSGTYPE_DELIVERY_IND = 6,
	MMS_MSGTYPE_READREC_IND = 7,
	MMS_MSGTYPE_READORG_IND = 8,
	MMS_MSGTYPE_FORWARD_REQ = 9,
	MMS_MSGTYPE_FORWARD_CONF = 10,
	MMS_MSGTYPE_READ_REPLY = 11,	// for internal use
	MMS_MSGTYPE_MAX
} MmsMsgType;

typedef enum {
	MMS_PRIORITY_ERROR = -1,			// error return in Get method
	MMS_PRIORITY_LOW = 0,			// default
	MMS_PRIORITY_NORMAL = 1,
	MMS_PRIORITY_HIGH = 2
} MmsPriority;

typedef enum {
	MMS_SENDER_VISIBLE_ERROR = -1,	// error return in Get method
	MMS_SENDER_SHOW = 0,
	MMS_SENDER_HIDE = 1
} MmsSenderVisible;

typedef enum {
	MMS_REPORT_ERROR = -1,	// error return in Get method
	MMS_REPORT_YES = 0,
	MMS_REPORT_NO = 1
} MmsReport;

typedef enum {
	MMS_REPORTALLOWED_ERROR = -1,	// error return in Get method
	MMS_REPORTALLOWED_YES = 0,
	MMS_REPORTALLOWED_NO = 1
} MmsReportAllowed;

typedef enum {
	MMS_RECEIVE_READ_REPORT_ALLOWED,
	MMS_RECEIVE_READ_REPORT_PROMPT,
	MMS_RECEIVE_READ_REPORT_NOT_ALLOWED,
} MmsRecvReadReportType ;

typedef enum {
	MMS_MSGSTATUS_NONE = -1,	// backward compatibility
	MMS_MSGSTATUS_ERROR = -1,	// error return in Get method
	MMS_MSGSTATUS_EXPIRED = 0,	// This value SHOULD not be used in the M-NotifyResp.ind PDU.
	MMS_MSGSTATUS_RETRIEVED = 1,
	MMS_MSGSTATUS_REJECTED = 2,
	MMS_MSGSTATUS_DEFERRED = 3,
	MMS_MSGSTATUS_UNRECOGNISED = 4,	// This value SHALL not be used in the M-Delivery.ind PDU.
	MMS_MSGSTATUS_INDETERMINATE = 5,
	MMS_MSGSTATUS_FORWARDED = 6,
	MMS_MSGSTATUS_UNREACHABLE = 7
} MmsMsgStatus;

typedef enum {
	MMS_READSTATUS_NONE = -1,			// no touch status
	MMS_IS_READ = 0,
	MMS_IS_DELETED = 1	// Deleted without being read
} MmsReadStatus;

typedef enum {
	MMS_ADDRESS_PRESENT_TOKEN,
	MMS_INSERT_ADDRESS_TOKEN
} MmsAddressType;

typedef enum {
	MSG_PARAM_REPORT_TYPE_DELIVERY_STATUS,
	MSG_PARAM_REPORT_TYPE_NUM,
	MSG_PARAM_REPORT_TYPE_UNKNOWN,
} MsgParamReportType;

/* Response status */
typedef enum {
	MMS_RESPSTATUS_ERROR = -1,	// error return in Get method
	MMS_RESPSTATUS_OK = 0,	// default value
	MMS_RESPSTAUTS_ERROR_UNSPECIFIED = 1,
	MMS_RESPSTAUTS_ERROR_SERVICEDENIED = 2,
	MMS_RESPSTAUTS_ERROR_MESSAGEFORMATCORRUPT = 3,
	MMS_RESPSTAUTS_ERROR_SENDINGADDRESSUNRESOLVED = 4,
	MMS_RESPSTAUTS_ERROR_MESSAGENOTFOUND = 5,
	MMS_RESPSTAUTS_ERROR_NETWORKPROBLEM = 6,
	MMS_RESPSTAUTS_ERROR_CONTENTNOTACCEPTED = 7,
	MMS_RESPSTAUTS_ERROR_UNSUPPORTEDMESSAGE = 8,

	MMS_RESPSTAUTS_ERROR_TRANSIENT_FAILURE = 9,
	MMS_RESPSTAUTS_ERROR_TRANSIENT_SENDING_ADDRESS_UNRESOLVED = 10,
	MMS_RESPSTAUTS_ERROR_TRANSIENT_MESSAGE_NOT_FOUND = 11,
	MMS_RESPSTAUTS_ERROR_TRANSIENT_NETWORK_PROBLEM = 12,

	MMS_RESPSTAUTS_ERROR_PERMANENT_FAILURE = 13,
	MMS_RESPSTAUTS_ERROR_PERMANENT_SERVICE_DENIED = 14,
	MMS_RESPSTAUTS_ERROR_PERMANENT_MESSAGE_FORMAT_CORRUPT = 15,
	MMS_RESPSTAUTS_ERROR_PERMANENT_SENDING_ADDRESS_UNRESOLVED = 16,
	MMS_RESPSTAUTS_ERROR_PERMANENT_MESSAGE_NOT_FOUND = 17,
	MMS_RESPSTAUTS_ERROR_PERMANENT_CONTENT_NOT_ACCEPTED = 18,
	MMS_RESPSTAUTS_ERROR_PERMANENT_REPLY_CHARGING_LIMITATIONS_NOT_MET = 19,
	MMS_RESPSTAUTS_ERROR_PERMANENT_REPLY_CHARGING_REQUEST_NOT_ACCEPTED = 20,
	MMS_RESPSTAUTS_ERROR_PERMANENT_REPLY_CHARGING_FORWARDING_DENIED = 21,
	MMS_RESPSTAUTS_ERROR_PERMANENT_REPLY_CHARGING_NOT_SUPPORTED = 22
} MmsResponseStatus;

typedef enum {
	MMS_RETRSTATUS_ERROR = -1,	// error return in Get method
	MMS_RETRSTATUS_OK = 0,
	MMS_RETRSTATUS_TRANSIENT_FAILURE = 1,
	MMS_RETRSTATUS_TRANSIENT_MESSAGE_NOT_FOUND = 2,
	MMS_RETRSTATUS_TRANSIENT_NETWORK_PROBLEM = 3,
	MMS_RETRSTATUS_PERMANENT_FAILURE = 4,
	MMS_RETRSTATUS_PERMANENT_SERVICE_DENIED = 5,
	MMS_RETRSTATUS_PERMANENT_MESSAGE_NOT_FOUND = 6,
	MMS_RETRSTATUS_PERMANENT_CONTENT_UNSUPPORT = 7
} MmsRetrieveStatus;

typedef enum {
	MMS_REPLY_NONE = -1,	// error return in Get method
	MMS_REPLY_REQUESTED = 0,
	MMS_REPLY_REQUESTED_TEXT_ONLY = 1,
	MMS_REPLY_ACCEPTED = 2,
	MMS_REPLY_ACCEPTED_TEXT_ONLY = 3
} MmsReplyChargeType;

typedef enum {
	MMS_MSGCLASS_ERROR = -1,	// error return in Get method
	MMS_MSGCLASS_PERSONAL = 0,	// default
	MMS_MSGCLASS_ADVERTISEMENT = 1,
	MMS_MSGCLASS_INFORMATIONAL = 2,
	MMS_MSGCLASS_AUTO = 3
} MmsMsgClass;


/*==================================================================================================
							Structures
==================================================================================================*/

typedef struct {
	MmsReplyChargeType chargeType;
	MmsTimeStruct deadLine;
	int chargeSize;
	char szChargeID[MMS_MSG_ID_LEN + 1];
} MmsReplyCharge;

typedef struct _MmsMsgMultiStatus {
	char szTo[MAX_ADDRESS_VAL_LEN + 1];
	bool bDeliveryReportIsRead;
	bool bDeliveyrReportIsLast;
	msg_delivery_report_status_t msgStatus;
	UINT32 handledTime;
	bool bReadReplyIsRead;
	bool bReadReplyIsLast;
	msg_read_report_status_t readStatus;
	UINT32 readTime;

	struct _MmsMsgMultiStatus *pNext;
} MmsMsgMultiStatus;

typedef struct _MMS_ATTRIB_S {
	MimeType contentType;
	MmsMsgType msgType;
	MmsDataType dataType;
	UINT32 date;
	UINT8 version;

	char szFrom[MSG_LOCALE_ADDR_LEN + 11];		//"/TYPE=PLMN", /"TYPE=IPv4", "/TYPE=IPv6"
	char szSubject[MSG_LOCALE_SUBJ_LEN + 1];
	char *szTo;
	char *szCc;
	char *szBcc;

	MmsPriority priority;

	MmsReplyCharge replyCharge;

	bool bHideAddress;
	bool bAskDeliveryReport;
	bool bReportAllowed;
	MmsRecvReadReportType readReportAllowedType;

	bool bAskReadReply;
	bool bRead;//FIXME : remove this value

	MmsRecvReadReportSendStatus readReportSendStatus;

	bool bReadReportSent;

	bool bLeaveCopy;

	int msgSize;
	MmsMsgClass msgClass;
	MmsTimeStruct expiryTime;
	MmsTimeStruct deliveryTime;

	//for ReadMsg, When Sending notifyResp.ind
	msg_delivery_report_status_t msgStatus;

	MmsResponseStatus responseStatus;
	MmsRetrieveStatus retrieveStatus;
	char szResponseText[MMS_LOCALE_RESP_TEXT_LEN + 1];
	char szRetrieveText[MMS_LOCALE_RESP_TEXT_LEN + 1];

	MmsMsgMultiStatus *pMultiStatus;

} MmsAttrib;

typedef struct _MsgContentParam {
	int charset;
	char szBoundary[MSG_BOUNDARY_LEN + 1];
	char szFileName[MSG_FILENAME_LEN_MAX + 1];
	char szName[MSG_LOCALE_FILENAME_LEN_MAX + 1];

#ifdef FEATURE_JAVA_MMS
	char *szApplicationID;
	char *szReplyToApplicationID;
#endif

	int type;
	void *pPresentation;
	char szStart[MSG_MSG_ID_LEN + 1];
	char szStartInfo[MSG_MSG_ID_LEN + 1];

	MsgParamReportType reportType; //only used as parameter of Content-Type: multipart/report; report-type=delivery-status;
} MsgContentParam;

typedef struct _MsgDRMInfo {
	MsgDrmType drmType;
	MimeType contentType;
	char *szContentURI;
	char *szContentName;
	char *szContentDescription;
	char *szContentVendor;
	char *szRightIssuer;
	char *szDrm2FullPath;
	int roWaitingTimerMax;		// DRM RO WAITING
	bool bFwdLock;
	char *pszContentType;
	bool bNoRingTone;
	bool bNoScreen;
} MsgDRMInfo;

typedef struct _MsgType {
	int offset;
	int size;
	int contentSize;
	int section;
	int type;

	MsgDRMInfo drmInfo;

	int encoding;
	int disposition;
	char szContentID[MSG_MSG_ID_LEN + 1];
	char szContentLocation[MSG_MSG_ID_LEN + 1];
	char szOrgFilePath[MSG_FILEPATH_LEN_MAX + 1];

	MsgContentParam param;
} MsgType;

struct _MsgBody {
	int offset;
	int size;
	char szOrgFilePath[MSG_FILEPATH_LEN_MAX + 1];
	MsgType presentationType;
	MsgBody *pPresentationBody;

	union {
		char *pText;
		MsgMultipart *pMultipart;
	} body;
};

struct _MsgMultipart {
	MsgType type;
	MsgBody *pBody;
	MsgMultipart *pNext;
};

typedef struct _MMS_MESSAGE_S {
	MmsAttrib mmsAttrib;
	msg_message_id_t msgID;
	int mailbox;		// mailbox type,MMS_MAILBOX_XXX
	char szFileName[MSG_FILENAME_LEN_MAX];
	char szTrID[MMS_TR_ID_LEN + 1];
	char szMsgID[MMS_MSG_ID_LEN + 1];
	char szForwardMsgID[MMS_MSG_ID_LEN + 1];
	char szContentLocation[MMS_LOCATION_LEN + 1];
	int nPartCount;
	MsgType msgType;
	MsgBody msgBody;
} MmsMsg;

#endif //MMS_PLUGIN_CODEC_TYPE_H
