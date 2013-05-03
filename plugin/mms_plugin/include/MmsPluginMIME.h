/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef MMS_PLUGIN_MIME_H
#define MMS_PLUGIN_MIME_H

#include "MmsPluginTypes.h"

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
	MSG_CHARSET_US_ASCII,			//0
	MSG_CHARSET_UTF16,
	MSG_CHARSET_USC2,
	MSG_CHARSET_UTF8,
	/* MIME character-set value */

	MSG_CHARSET_ISO_2022_KR,
	MSG_CHARSET_KS_C_5601_1987,
	MSG_CHARSET_EUC_KR,	// KS_C5861-1992
	MSG_CHARSET_ISO_2022_JP,
	MSG_CHARSET_ISO_2022_JP_2,
	MSG_CHARSET_ISO_8859_1,			//Latin1
	MSG_CHARSET_ISO_8859_2,
	MSG_CHARSET_ISO_8859_3,
	MSG_CHARSET_ISO_8859_4,
	MSG_CHARSET_ISO_8859_5,
	MSG_CHARSET_ISO_8859_6,
	MSG_CHARSET_ISO_8859_6_E,
	MSG_CHARSET_ISO_8859_6_I,
	MSG_CHARSET_ISO_8859_7,
	MSG_CHARSET_ISO_8859_8,
	MSG_CHARSET_ISO_8859_8_I,
	MSG_CHARSET_ISO_8859_9,
	MSG_CHARSET_ISO_8859_10,
	MSG_CHARSET_ISO_8859_15,
	MSG_CHARSET_Shift_JIS,
	MSG_CHARSET_EUC_JP,
	MSG_CHARSET_GB2312,
	MSG_CHARSET_BIG5,
	MSG_CHARSET_WIN1251,
	MSG_CHARSET_WINDOW_1251,
	MSG_CHARSET_WINDOWS_1251, // same: WIN1251, WINDOW-1251, WINDOWS-1251
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

#define		UNDEFINED_BINARY	0xFF
#define		MSG_UNKNOWN_TYPE_STRING		"unknown"

typedef enum _MimeMainType {
	MIME_MAINTYPE_APPLICATION,
	MIME_MAINTYPE_AUDIO,
	MIME_MAINTYPE_IMAGE,
	MIME_MAINTYPE_MESSAGE,
	MIME_MAINTYPE_MULTIPART,
	MIME_MAINTYPE_TEXT,
	MIME_MAINTYPE_VIDEO,
	MIME_MAINTYPE_THEME,
	MIME_MAINTYPE_ETC
} MimeMainType;

typedef enum _MimeAppType {
	MIME_APPLICATION_NONE,

	MIME_APPLICATION_CAMERA,
	MIME_APPLICATION_VIDEORECORDER,

	MIME_APPLICATION_IMAGEVIEWER,
	MIME_APPLICATION_FLASHVIEWER,
	MIME_APPLICATION_IMAGEEDITOR,
	MIME_APPLICATION_THEMEVIEWER,

	MIME_APPLICATION_SOUNDPLAYER,
	MIME_APPLICATION_MEDIAPLAYER,
	MIME_APPLICATION_VOICEMEMO,

	MIME_APPLICATION_PICSELVIEWER,

	MIME_APPLICATION_CONTACT,
	MIME_APPLICATION_ORGANIZER,

	MIME_APPLICATION_MAX
} MimeAppType;


typedef struct _MimeTable {
	const char *szMIME;
	const char *szExt;
	bool bDownloadable;
	MimeType mime;			/* index of mime type */
	MimeType contentType;	/* representative mime type */
	MimeAppType appType;
	MimeMainType mainType;
	int binary;
} MimeTable;

typedef struct _ExtTable {
	const char *szExt;
	MimeType mimeType;
} ExtTable;

/* MIME string table ID */
typedef enum {
	MSG_ENCODING,
	MSG_DISPOSITION,
	MSG_FIELD,
	MSG_PARAM,
	MSG_TYPE,
	MSG_CHARSET,
	MSG_ADDR_TYPE,
} MsgHeaderField;

typedef enum {
	MSG_ADDR_TYPE_ERROR = -1,		// error return in Get method
	MSG_ADDR_TYPE_PHONE = 0,
	MSG_ADDR_TYPE_EMAIL = 1,
	MSG_ADDR_TYPE_IPV4  = 2,
	MSG_ADDR_TYPE_IPV6  = 3,
	MSG_ADDR_TYPE_ALIAS = 4,
	MSG_ADDR_TYPE_NUM   = 5,
} MsgAddrType;

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
	MSG_FIELD_CONTENT_DISPOSITION, //If start param is given in multipart/related, this field will be ignored
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
#ifdef __SUPPORT_DRM__
	MSG_FIELD_DRM_CONVERTED,
#endif
	MSG_FIELD_NUM,
	MSG_FIELD_UNKNOWN,
};

/* MIME header field parameter */
enum {
	MSG_PARAM_CHARSET,
	MSG_PARAM_NAME,
	MSG_PARAM_FILENAME,
	MSG_PARAM_TYPE,			// only used as parameter of Content-Type: multipart/related
	MSG_PARAM_START,		// Only if content-type is multipart/related
	MSG_PARAM_START_INFO,	// Only if content-type is multipart/related
	MSG_PARAM_BOUNDARY,
	MSG_PARAM_REPORT_TYPE,  // only used as parameter of Content-Type: multipart/report; report-type=delivery-status;
#ifdef FEATURE_JAVA_MMS
	MSG_PARAM_APPLICATION_ID,
	MSG_PARAM_REPLY_TO_APPLICATION_ID,
#endif
	MSG_PARAM_NUM,
	MSG_PARAM_UNKNOWN,
};

/********************		API definition		 ************************/

bool MimeIsDownloadableInt(MimeType mime);
bool MimeIsDownloadableString(const char *szMime);
char *MimeGetExtFromMimeInt(MimeType mime);
char *MimeGetExtFromMimeString(const char *szMime);
char *MimeGetMimeFromExtString(const char *szExt);
MimeType MimeGetMimeFromExtInt(const char *szExt);
MimeMainType MimeGetMainTypeInt(MimeType mime);
MimeMainType MimeGetMainTypeString(const char *szMime);
MimeType MimeGetMimeIntFromBi(int binCode);
char *MimeGetMimeStringFromBi(int binCode);
int MimeGetBinaryValueFromMimeInt(MimeType mime);
int MimeGetBinaryValueFromMimeString(const char *szMime);

// Append the functions on MimeAppType
MimeAppType MimeGetAppTypeFromInt(MimeType mime);
MimeAppType MimeGetAppTypeFromString(const char *szMime);
MimeAppType MimeGetAppTypeFromExtString(const char *szExt);

// Append the functions returning index and content type

MimeType MimeGetContentTypeFromInt(MimeType mime);
MimeType MimeGetContentTypeFromString(const char *szMime);
MimeType MimeGetMimeTypeFromString(const char *szMime);

MimeType MimeGetMimeIntFromMimeString(char *szMimeStr);
char *MimeGetMimeStringFromMimeInt(int mimeType);

char *MsgGetString(MsgHeaderField tableId, int code);
char *_MsgSkipWS3(char *s);
int	_MsgGetCode(MsgHeaderField tableId, char *pStr);

#endif // MMS_PLUGIN_MIME_H
