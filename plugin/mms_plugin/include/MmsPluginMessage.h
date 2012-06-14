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

#ifndef MMS_PLUGIN_MESSAGE_H
#define MMS_PLUGIN_MESSAGE_H


/*==================================================================================================
							INCLUDE FILES
==================================================================================================*/
#include "MsgTypes.h"
#include "MsgInternalTypes.h"
#include "MsgMmsTypes.h"
#include "MsgSettingTypes.h"
#include "MmsPluginMIME.h"
#include "MsgMmsTypes.h"

/*==================================================================================================
							DEFINES
==================================================================================================*/
#define		MMS_CONTENT_ID_LEN				100
#define		MMS_LOCALE_RESP_TEXT_LEN		90
#define		MMS_TEXT_LEN						2000
#define		MMS_READ_REPORT_STRING_READ		"Read:"
#define		MMS_READ_REPORT_STRING_DELETED	"Deleted:"

#define		MSG_ATTACH_MAX		20
#define		MSG_LINE_LEN						1024
#define		MSG_SUBJ_LEN						40
#define		MSG_DATE_LEN						50
#define		MSG_BOUNDARY_LEN					70
#define		MSG_LOCALE_NAME_LEN				150
#define		MSG_LOCALE_SUBJ_LEN				(3 * MSG_SUBJ_LEN)
#define		MSG_ADDR_LEN						MAX_ADDRESS_VAL_LEN
#define		MSG_LOCALE_ADDR_LEN				(3 * MAX_ADDRESS_VAL_LEN)
#define		MSG_LOCALE_FILENAME_LEN_MAX		(3 * 255)
#define		MSG_LOCALE_SIGN_LEN					(3 * MAX_SIGN_VAL_LEN)
#define		MSG_MAILBOX_NAME_LEN		10

#define		MsgContentType						MimeType
#define		MsgRecipientType					MSG_RECIPIENT_TYPE_T
#define		MMS_VERSION						0x92
#define		MSG_STR_ADDR_DELIMETER			";"
#define		MSG_CH_ADDR_DELIMETER			';'
#ifdef __SUPPORT_DRM__
#define		MSG_DRM_MAX_CMD					(2 * 1024)
#endif

#define		MSG_MMS_DECODE_BUFFER_MAX	(2 * 1024)
#define		MSG_MMS_ENCODE_BUFFER_MAX	(2 * 1024)

/*==================================================================================================
							TYPES
==================================================================================================*/
typedef unsigned int			UINT;
typedef unsigned long int		UINT32;
typedef unsigned char			UINT8;
typedef unsigned short int		UINT16;
typedef unsigned long			ULONG;
typedef unsigned char			UCHAR;
typedef unsigned short			MCHAR;
typedef unsigned short			USHORT;

typedef int						MmsMsgID;

typedef	struct _MsgAddress	MsgAddress;
typedef struct _MsgBody		MsgBody;
typedef struct _MsgMultipart	MsgMultipart;
typedef struct _MsgMessage	MsgMessage;

typedef struct _MsgHeaderAddress	MsgHeaderAddress;

/*==================================================================================================
							ENUMS
==================================================================================================*/

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

typedef	enum {
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

typedef	enum {
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


typedef	enum {
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
	MMS_MSGSTATUS_EXPIRED = 0,
	MMS_MSGSTATUS_RETRIEVED = 1,
	MMS_MSGSTATUS_REJECTED = 2,
	MMS_MSGSTATUS_DEFERRED = 3,
	MMS_MSGSTATUS_UNRECOGNISED = 4,	// This value SHALL not be used in the M-Delivery.ind PDU.
	MMS_MSGSTATUS_INDETERMINATE = 5,
	MMS_MSGSTATUS_FORWARDED = 6,
	MMS_MSGSTATUS_UNREACHABLE = 7
} MmsMsgStatus;


typedef	enum {
	MMS_READSTATUS_NONE = -1,			// no touch status
	MMS_IS_READ = 0,
	MMS_IS_DELETED = 1
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
typedef	enum {
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


typedef	enum {
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
	MMS_STATUS_ERROR = -1,	// error return in Get method
	MMS_STATUS_INITIAL = 0,
	MMS_STATUS_REQUESTING = 1,
	MMS_STATUS_SUCCESS = 2,
	MMS_STATUS_FAIL = 3,
	MMS_STATUS_NUM = 4,
} MmsDataStatus;


typedef enum {
	MMS_EDIT_STYLE_THIS_PAGE = 0,
	MMS_EDIT_STYLE_ALL_PAGE,
} MmsEditStyleMode;

typedef enum {
	MMS_SPECIAL_MSG_TYPE_NONE = 0,
	MMS_SPECIAL_MSG_TYPE_SYSTEM_TEMPLATE,	// system template .
	MMS_SPECIAL_MSG_TYPE_VOICE_MESSAGE,		//voice message service

	MMS_SPECIAL_MSG_TYPE_VDF_POSTCARD,		//postcard service, or Generic Postcard Service
	MMS_SPECIAL_MSG_TYPE_POSTCARD_EXT1,		//Stickers Service
	MMS_SPECIAL_MSG_TYPE_POSTCARD_EXT2,		//Photo 10x15
	MMS_SPECIAL_MSG_TYPE_POSTCARD_EXT3,		//8 Photos
	MMS_SPECIAL_MSG_TYPE_POSTCARD_EXT4,		// 4 Photos
} MmsSpecialMsgType;

typedef enum {
	MMS_MM_CLASS_TEXT,
	MMS_MM_CLASS_IMAGE_BASIC,
	MMS_MM_CLASS_IMAGE_RICH,
	MMS_MM_CLASS_VIDEO_BASIC,
	MMS_MM_CLASS_VIDEO_RICH,
	MMS_MM_CLASS_MEGAPIXEL,
	MMS_MM_CLASS_CONTENT_BASIC,
	MMS_MM_CLASS_CONTENT_RICH,
	MMS_MM_CLASS_OTHER
} MmsUiMmClass;

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


typedef	struct _MmsMsgMultiStatus {
	char szTo[MAX_ADDRESS_VAL_LEN + 1];
	bool bDeliveryReportIsRead;
	bool bDeliveyrReportIsLast;
	MSG_DELIVERY_REPORT_STATUS_T msgStatus;
	UINT32 handledTime;

	bool bReadReplyIsRead;
	bool bReadReplyIsLast;
	MSG_READ_REPORT_STATUS_T readStatus;
	UINT32 readTime;

	struct _MmsMsgMultiStatus *pNext;
} MmsMsgMultiStatus;

typedef	struct _MMS_ATTRIB_S {
	MsgContentType contentType;
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
	bool bChargingMsgReplied;

	bool bHideAddress;
	bool bAskDeliveryReport;
	bool bReportAllowed;
	MmsRecvReadReportType readReportAllowedType;

	bool bAskReadReply;
	bool bRead;

	MmsRecvReadReportSendStatus readReportSendStatus;

	bool bReadReportSent;

	bool bLeaveCopy;
	bool bUseExpiryCustomTime;		// for expiry custom time
	bool bUseDeliveryCustomTime;	// for expiry custom time
	MmsEditStyleMode editStyleMode;			//editStyle on All Page

	MmsSpecialMsgType specialMsgType;			// instead of bPostcard

#ifdef __SUPPORT_DRM__
	MsgDrmType drmType;
	int roWaitingTimerMax;		// DRM RO WAITING
	char *pszDrmData;			// DRM Data to draw mailbox icon
#endif

	int msgSize;
	MmsMsgClass msgClass;
	MmsTimeStruct expiryTime;
	MmsTimeStruct expiryCustomTime;		// for expiry custom time,
	MmsTimeStruct deliveryTime;
	MmsTimeStruct deliveryCustomTime;	// for expiry custom time

	//for ReadMsg, When Sending notifyResp.ind
	MSG_DELIVERY_REPORT_STATUS_T msgStatus;

	MmsResponseStatus responseStatus;
	MmsRetrieveStatus retrieveStatus;
	char szResponseText[MMS_LOCALE_RESP_TEXT_LEN + 1];
	char szRetrieveText[MMS_LOCALE_RESP_TEXT_LEN + 1];

	MmsMsgMultiStatus *pMultiStatus;
	bool bRetrieveNow;
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

#ifdef __SUPPORT_DRM__
typedef struct _MsgDRMInfo {
	MsgDrmType drmType;
	MsgContentType contentType;
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
#endif

typedef struct _MsgType {
	int offset;
	int size;
	int contentSize;
	int section;
	int type;
#ifdef __SUPPORT_DRM__
	MsgDRMInfo drmInfo;
#endif
	int encoding;
	int disposition;
	char szContentID[MSG_MSG_ID_LEN + 1];
	char szContentLocation[MSG_MSG_ID_LEN + 1];
	char szOrgFilePath[MSG_FILEPATH_LEN_MAX];

	char szContentRepPos[30];
	char szContentRepSize[30];
	char szContentRepIndex[8];
#ifdef __SUPPORT_DRM__
	int nDrmConvertedStatus;
#endif

	MsgContentParam param;
} MsgType;

struct _MsgAddress {
	int type;					//MSG_ADDR_TYPE_PHONE/EMAIL/IPV4/IPV6
	MsgRecipientType recipientType;			// TO, CC, BCC
	char szName[MSG_LOCALE_NAME_LEN + 1];
	char szAddr[MSG_ADDR_LEN + 1];
	MsgAddress *pNext;
	bool bDoNotShow;				// temporary clip

	// sorting
	ULONG uLastRecentTime;		// last recent time save item
	ULONG uMostUseCount;			// most use count item.
	int index;					// index sorting..
};

typedef struct _MsgHeader {
	MsgAddress *pFrom;
	MsgAddress *pTo;
	MsgAddress *pCC;
	char szReturnPath[MSG_ADDR_LEN + 1];
	char szDate[MSG_DATE_LEN];
	char szSubject[MSG_LOCALE_SUBJ_LEN + 1];
	char szMsgID[MSG_MSG_ID_LEN + 1];
} MsgHeader;


struct _MsgBody {
	int offset;
	int size;
	char szOrgFilePath[MSG_FILEPATH_LEN_MAX];
	MsgType presentationType;
	MsgBody *pPresentationBody;

	union {
		char *pText;
		void *pBinary;
		MsgMultipart *pMultipart;
		MsgMessage *pMessage;
	} body;
};

struct _MsgMultipart {
	MsgType type;
	MsgBody *pBody;
	MsgMultipart *pNext;
};

struct _MsgMessage {
	MsgHeader header;
	MsgType type;
	MsgBody *pBody;
};


typedef	struct _MMS_MESSAGE_S {
	MmsAttrib mmsAttrib;
	MSG_MESSAGE_ID_T msgID;
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

struct _MsgHeaderAddress {
	char *szAddr;
	MsgHeaderAddress *pNext;
};


bool MmsInitMsgType(MsgType *pMsgType);
bool MmsInitMsgBody(MsgBody *pMsgBody);
bool MmsInitMsgContentParam(MsgContentParam *pMsgContentParam);
bool MmsInitMsgAttrib(MmsAttrib *pAttrib);
bool MmsSetMsgAddressList(MmsAttrib *pAttrib, const MSG_MESSAGE_INFO_S *pMsgInfo);
char *MmsComposeAddress(const MSG_MESSAGE_INFO_S *pMsgInfo, int recipientType);
bool MmsGetMsgBodyfromMsgInfo(const MSG_MESSAGE_INFO_S *pMsgInfo, MMS_MESSAGE_DATA_S *pMsgBody, char *pFileData);
bool MmsGetSmilRawData(MMS_MESSAGE_DATA_S *pMsgBody, char *pRawData, int *nSize);
bool MmsInsertPresentation(MmsMsg *pMsg, MsgContentType mimeType, char *pData, int size);
bool MmsInsertPartFromFile(MmsMsg *pMsg, char *szTitleName, char *szOrgFilePath, char *szContentID);
bool MmsIsMultipart(int type);
bool MmsGetTypeByFileName(int *type, char *szFileName);
MsgMultipart *MmsMakeMultipart(MsgContentType mimeType, char *szTitleName, char *szOrgFilePath, void *pData, int offset, int size, char *szContentID);
bool MmsIsText(int type);
bool MmsIsVitemContent(int type, char *pszName);
bool MmsComposeMessage(MmsMsg *pMmsMsg, MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMsgData, char *pFileData);
void MmsComposeNotiMessage(MmsMsg *pMmsMsg, MSG_MESSAGE_ID_T msgID);
bool MmsGetMmsMessageBody(MmsMsg *pMmsMsg, char *retrievedFilePath);
bool MmsComposeForwardHeader(MmsMsg *pMmsMsg, const MSG_MESSAGE_INFO_S *pMsgInfo);
bool MmsComposeForwardMessage(MmsMsg *pMmsMsg, char *retrievedFilePath);
#ifdef MMS_DELIEVERY_IND_ENABLED
MmsMsgMultiStatus *MmsComposeDeliveryIndMessage(MmsMsg *pMmsMsg, MSG_MESSAGE_ID_T msgId);
#endif
void MmsComposeReadReportMessage(MmsMsg *pMmsMsg, const MSG_MESSAGE_INFO_S *pMsgInfo, MSG_MESSAGE_ID_T selectedMsgId);
MmsMsgMultiStatus *MmsGetMultiStatus(MSG_MESSAGE_ID_T msgId);
int MmsSearchMsgId(char *toNumber, char *szMsgID);
void MmsUpdateDeliveryReport(MSG_MESSAGE_ID_T msgId, MmsMsgMultiStatus *pStatus);
void MmsUpdateReadReport(MSG_MESSAGE_ID_T msgId, MmsMsgMultiStatus *pStatus);
MsgMultipart *MmsAllocMultipart(void);
bool _MsgIsASCII(char *pszText);
bool _MsgReplaceNonAscii(char *szInText, char **szOutText, char replaceChar);
bool _MsgIsSpace(char *pszText);
bool _MsgReplaceSpecialChar(char *szInText, char **szOutText, char specialChar);
char *MsgStrAppend(char *szInputStr1, char *szInputStr2);
char *MsgStrCopy(const char *string);
char *MsgStrNCopy(const char *string, int length);
int	MsgStrlen(char *pStr);
bool _MsgConvertCharToHex(char pSrc, char *pDest);
MSG_ERROR_T MmsAddAttachment(MMS_MESSAGE_DATA_S *pMsgData, MMS_MEDIA_S *pMedia);
bool MmsCheckAdditionalMedia(MMS_MESSAGE_DATA_S *pMsgData, MsgType *partHeader);
#ifdef __SUPPORT_DRM__
bool __MsgInitMsgDRMInfo(MsgDRMInfo *pMsgDrmInfo);
#endif

#endif
