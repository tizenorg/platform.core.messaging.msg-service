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

#ifndef MMS_PLUGIN_CODEC_COMMON_H
#define MMS_PLUGIN_CODEC_COMMON_H

#include "MmsPluginCodecTypes.h"

#define		MSG_CH_CR						'\r'
#define		MSG_CH_LF						'\n'
#define		MSG_CH_SP						' '
#define		MSG_CH_TAB						'\t'
#define		MSG_CH_NULL						'\0'
#define		MSG_CH_COLON					':'
#define		MSG_CH_SEMICOLON				';'
#define		MSG_CH_UNDERLINE				'_'
#define		MSG_CH_COMMA					','
#define		MSG_CH_BASE64_LOWER				'b'
#define		MSG_CH_BASE64_UPPER				'B'
#define		MSG_CH_QPRINT_LOWER				'Q'
#define		MSG_CH_QPRINT_UPPER				'q'

#define		MSG_CH_QUESTION					'?'
#define		MSG_CH_QUOT						'"'
#define		MSG_CH_EQUAL					'='
#define		MSG_CH_BRACKET_S				'<'
#define		MSG_CH_BRACKET_E				'>'

#define		MSG_STR_DEC_START			"=?"
#define		MSG_STR_DEC_END				"?="
#define		MSG_STR_CRLF				"\r\n"
#define		MSG_END_OF_HEADER			"\r\n\r\n"
#define		MSG_STR_DOUBLE_HYPEN		"--"
#define		MSG_STR_BOUNDARY_DEL		"\r\n--"

#define		MSG_MMS_CH_EMAIL_AT			'@'

/* character-set parameter value of content-type header field */
enum {
	MSG_CHARSET_US_ASCII,
	MSG_CHARSET_UTF16,
	MSG_CHARSET_USC2,
	MSG_CHARSET_UTF8,
	MSG_CHARSET_ISO_2022_KR,
	MSG_CHARSET_KS_C_5601_1987,
	MSG_CHARSET_EUC_KR,
	MSG_CHARSET_ISO_2022_JP,
	MSG_CHARSET_ISO_2022_JP_2,
	MSG_CHARSET_ISO_8859_1,
	MSG_CHARSET_ISO_8859_2,
	MSG_CHARSET_ISO_8859_3,
	MSG_CHARSET_ISO_8859_4,
	MSG_CHARSET_ISO_8859_5,
	MSG_CHARSET_ISO_8859_6,
	MSG_CHARSET_ISO_8859_7,
	MSG_CHARSET_ISO_8859_8,
	MSG_CHARSET_ISO_8859_9,
	MSG_CHARSET_ISO_8859_10,
	MSG_CHARSET_ISO_8859_15,
	MSG_CHARSET_Shift_JIS,
	MSG_CHARSET_EUC_JP,
	MSG_CHARSET_GB2312,
	MSG_CHARSET_BIG5,
	MSG_CHARSET_WIN1251,
	MSG_CHARSET_KOI8_R,
	MSG_CHARSET_KOI8_U,

	MSG_CHARSET_NUM,
	MSG_CHARSET_UNKNOWN
};

/* Content-Transfer-Encoding header field value */
enum {
	MSG_ENCODING_7BIT,
	MSG_ENCODING_8BIT,
	MSG_ENCODING_BINARY,
	MSG_ENCODING_BASE64,
	MSG_ENCODING_QUOTE_PRINTABLE,
	MSG_ENCODING_NUM,
	MSG_ENCODING_OTHER,
};


/* Content-Disposition header field value */
enum {
	MSG_DISPOSITION_FORMDATA,
	MSG_DISPOSITION_ATTACHMENT,
	MSG_DISPOSITION_INLINE,
	MSG_DISPOSITION_NUM
};

/* MIME header field */
enum {
	MSG_FIELD_RETURN_PATH,
	MSG_FIELD_MESSAGE_ID,
	MSG_FIELD_FROM,
	MSG_FIELD_TO,
	MSG_FIELD_CC,
	MSG_FIELD_SUBJECT,
	MSG_FIELD_DATE,
	MSG_FIELD_MIME_VERSION,
	MSG_FIELD_CONTENT_TYPE,
	MSG_FIELD_CONTENT_TRANSFER_ENCODING,
	MSG_FIELD_CONTENT_DISPOSITION, /* If start param is given in multipart/related, this field will be ignored */
	MSG_FIELD_CONTENT_ID,
	MSG_FIELD_CONTENT_LOCATION,
	MSG_FIELD_CONTENT_NAME,
	MSG_FIELD_CONTENT_DESCRIPTION,
	MSG_FIELD_CONTENT_VENDOR,
	MSG_FIELD_RIGHT_ISSUER,

	MSG_FIELD_RETURN_RECEIPT_TO,			/* for Delivery confirm */
	MSG_FIELD_DISPOSITION_NOTIFICATION_TO,	/* for Read confirm */

	MSG_FILED_CONTENT_REPLACE_POS,
	MSG_FILED_CONTENT_REPLACE_SIZE,
	MSG_FILED_CONTENT_REPLACE_INDEX,

	MSG_FIELD_DRM_CONVERTED,

	MSG_FIELD_NUM,
	MSG_FIELD_UNKNOWN,
};

/* MIME header field parameter */
enum {
	MSG_PARAM_CHARSET,
	MSG_PARAM_NAME,
	MSG_PARAM_FILENAME,
	MSG_PARAM_TYPE,			/* only used as parameter of Content-Type: multipart/related */
	MSG_PARAM_START,		/* Only if content-type is multipart/related */
	MSG_PARAM_START_INFO,	/* Only if content-type is multipart/related */
	MSG_PARAM_BOUNDARY,
	MSG_PARAM_REPORT_TYPE,  /* only used as parameter of Content-Type: multipart/report; report-type=delivery-status; */
#ifdef FEATURE_JAVA_MMS
	MSG_PARAM_APPLICATION_ID,
	MSG_PARAM_REPLY_TO_APPLICATION_ID,
#endif
	MSG_PARAM_NUM,
	MSG_PARAM_UNKNOWN,
};

const char *MmsGetTextByCode(MmsCode i, UINT16 code);
const char *MmsGetTextValue(MmsCode i, int j);
const char *MmsGetTextValuebyField(int field, int value);
int MmsGetBinaryType(MmsCode i, UINT16 value);
int MmsGetTextType(MmsCode i, char *pValue);
UINT16 MmsGetBinaryValue(MmsCode i, int j);

void *MsgDecodeBase64(unsigned char *pSrc, unsigned long srcLen, unsigned long *len);
bool MsgEncode2Base64(void *pSrc, unsigned long srcLen, unsigned long *len, unsigned char *ret);
unsigned char *MsgDecodeQuotePrintable(unsigned char *pSrc, unsigned long srcLen, unsigned long *len);

char *MsgDecodeText(const char *pOri);

const char *MmsDebugGetMimeType(MimeType mimeType);
const char *MmsDebugGetMmsReport(MmsReport report);
const char *MmsDebugGetMmsReportAllowed(MmsReportAllowed reportAllowed);
const char *MmsDebugGetMmsReadStatus(msg_read_report_status_t readStatus);
const char *MmsDebugGetMsgType(MmsMsgType msgType);
const char *MmsDebugGetResponseStatus(MmsResponseStatus responseStatus);
const char *MmsDebugGetRetrieveStatus(MmsRetrieveStatus retrieveStatus);
const char *MmsDebugGetMsgStatus(msg_delivery_report_status_t msgStatus);
const char *MmsDebugGetMsgClass(MmsMsgClass msgClass);
const char *MmsDebugGetDataType(MmsDataType dataType);

bool MmsInitMsgType(MsgType *pMsgType);
bool MmsInitMsgBody(MsgBody *pMsgBody);
bool MmsInitMsgContentParam(MsgContentParam *pMsgContentParam);
bool MmsInitMsgAttrib(MmsAttrib *pAttrib);
bool MmsInitMsgDRMInfo(MsgDRMInfo *pMsgDrmInfo);

bool MmsReleaseMsgBody(MsgBody *pBody, int type);
bool MmsReleaseMmsAttrib(MmsAttrib *pAttrib);
void MmsReleaseMsgDRMInfo(MsgDRMInfo *pDrmInfo);
void MmsReleaseMmsMsg(MmsMsg *pMmsMsg);

bool MmsIsTextType(int type);
bool MmsIsMultipart(int type);
bool MmsIsVitemContent(int type, char *pszName);

MsgMultipart *MmsAllocMultipart(void);
bool MmsPrintMulitpart(MsgMultipart *pMultipart, int index);
#endif /* MMS_PLUGIN_CODEC_COMMON_H */
