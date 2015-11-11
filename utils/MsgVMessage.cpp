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

#include <glib.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>

#include <VMessage.h>
#include <VCard.h>
#include <time.h>

#include "MsgMmsTypes.h"
#include "MsgContact.h"
#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgCppTypes.h"
#include "MsgVMessage.h"
#include "MsgMmsMessage.h"
#include "MsgSerialize.h"

#define VMSG_INIT_LENGTH 1024
#define VMSG_ITEM_LENGTH 1024
#define MSGSVC_VMSG_FOLDING_LIMIT 75

#define SMART_STRDUP(src) (src && *src)?strdup(src):NULL
#define SAFE_STR(src) (src)?src:""

#define MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, str) do { \
	if ((len = __msgsvc_vmsg_append_str(buf, buf_size, len, str, false)) < 0) { \
		MSG_ERR("__msgsvc_vmsg_append_str() Failed"); \
		return MSG_ERR_MEMORY_ERROR; \
	} \
} while (0)

#define MSGSVC_VMSG_APPEND_STR_FREE(buf, buf_size, len, str) do { \
	if ((len = __msgsvc_vmsg_append_str(buf, buf_size, len, str, false)) < 0) { \
		MSG_ERR("__msgsvc_vmsg_append_str() Failed"); \
		if (str) \
			free(str); \
		return MSG_ERR_MEMORY_ERROR; \
	} \
} while (0)

#define MSGSVC_VMSG_APPEND_CONTENT_STR(buf, buf_size, len, content) do { \
	if ((len = __msgsvc_vmsg_append_str(buf, buf_size, len, content, true)) < 0) { \
		MSG_ERR("__msgsvc_vmsg_append_str() Failed"); \
		return MSG_ERR_MEMORY_ERROR; \
	} \
} while (0)


#define MSGSVC_VMSG_APPEND_CONTENT(buf, buf_size, len, content) do { \
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, ";CHARSET=UTF-8"); \
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, ":"); \
	MSGSVC_VMSG_APPEND_CONTENT_STR(buf, buf_size, len, content); \
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF); \
} while (0)

enum {
	CTSVC_VCARD_VER_NONE,
	CTSVC_VCARD_VER_2_1,
	CTSVC_VCARD_VER_3_0,
	CTSVC_VCARD_VER_4_0,
};

enum {
	MSGSVC_VMSG_VER_NONE,
	MSGSVC_VMSG_VER_1_1,
	MSGSVC_VMSG_VER_3_0
};


enum {
	 // vMessage
	VMSG_VALUE_NONE = 0,
	BEGIN_VMSG = 1,
	VERSION_VMSG =2,
	BEGIN_VCARD = 3,
	VERSION_VCARD = 4,
	END_VCARD = 5,
	BEGIN_VENV = 6,
	BEGIN_VBODY = 7,
	END_VBODY = 8,
	END_VENV = 9,
	END_VMSG = 10,

	FNAME_VCARD = 11 ,
	NAME_VCARD = 12,
	TEL_VCARD = 13,
	TEL_VCARD_CELL = 14,

	DATE_VMSG = 15,
	SUBJECT_VMSG =16 ,

	FNAME_UTF8_VCARD = 17,
	NAME_UTF8_VCARD = 18,

	// Encodings
	VMSG_ATTR_ENCODING_QP = 19,
	VMSG_ATTR_ENCODING_BASE64_V21 = 20,
	VMSG_ATTR_ENCODING_BASE64_V30 = 21,

	// Message type indication
	VMSG_INDICATION_MESSAGE_TYPE = 22,
	VMSG_INDICATION_MESSAGE_TYPE_INET = 23,
	VMSG_INDICATION_MESSAGE_TYPE_MSG = 24,

	// Read status indication
	VMSG_INDICATION_READ_STATUS = 25,
	VMSG_INDICATION_READ_STATUS_READ = 26,
	VMSG_INDICATION_READ_STATUS_UNREAD = 27,

	// Mailbox status indication
	VMSG_INDICATION_MSG_BOX = 28,
	VMSG_INDICATION_MSG_BOX_SENT = 29,
	VMSG_INDICATION_MSG_BOX_INBOX = 30,
	VMSG_INDICATION_MSG_BOX_DRAFT = 31,

	// Character set
	VMSG_ATTR_CHARSET_PROPERTY = 32,

	// Language
	VMSG_ATTR_LANGUAGE_PROPERTY = 33,
	VMSG_DATA_SEPARATOR = 34,

	VMSG_BODY_PROPERTY_DATE = 35,
	VMSG_BODY_PROPERTY_SUBJECT = 36,
	VMSG_BODY_PROPERTY_FROM = 37,
	VMSG_BODY_PROPERTY_TO = 38,

	KEY_BODY = 39,
	KEY_DATE = 40,

	 //DECODER
	VMSG_VCARD_TEL = 41,
	VMSG_MSG_BEGIN = 42,
	VMSG_MSG_END = 43,
	VMSG_MAXIMUM_VALUE = 44,
};

static const char *content_name[VMSG_MAXIMUM_VALUE+1] = {0};
const char *MSGSVC_CRLF = "\r\n";
bool needCharset = false;

static void __msgsvc_vmsg_initial(void)
{
	if (NULL == *content_name) {

		// vMessage
		content_name[BEGIN_VMSG] = "BEGIN:VMSG";
		content_name[VERSION_VMSG] = "VERSION:1.1";
		content_name[BEGIN_VCARD] = "BEGIN:VCARD";
		content_name[VERSION_VCARD] = "VERSION:2.1";
		content_name[END_VCARD] = "END:VCARD";
		content_name[BEGIN_VENV] = "BEGIN:VENV";
		content_name[BEGIN_VBODY] = "BEGIN:VBODY";
		content_name[END_VBODY] = "END:VBODY";
		content_name[END_VENV] = "END:VENV";
		content_name[END_VMSG] = "END:VMSG";

		content_name[FNAME_VCARD] = "FN:";
		content_name[NAME_VCARD] = "N:";
		content_name[TEL_VCARD] = "TEL;";
		content_name[TEL_VCARD_CELL] = "CELL:";

		content_name[DATE_VMSG] = "Date:";
		content_name[SUBJECT_VMSG] = "Subject:";

		content_name[FNAME_UTF8_VCARD] = "FN;CHARSET=UTF-8:";
		content_name[NAME_UTF8_VCARD] = "N;CHARSET=UTF-8:";

		// Encodings
		content_name[VMSG_ATTR_ENCODING_QP] = "ENCODING=QUOTED-PRINTABLE";
		content_name[VMSG_ATTR_ENCODING_BASE64_V21] = "ENCODING=BASE64";
		content_name[VMSG_ATTR_ENCODING_BASE64_V30] = "ENCODING=b";

		// Message type indication
		content_name[VMSG_INDICATION_MESSAGE_TYPE] = "X-IRMC-TYPE";
		content_name[VMSG_INDICATION_MESSAGE_TYPE_INET] = "INET";
		content_name[VMSG_INDICATION_MESSAGE_TYPE_MSG] = "MSG";

		// Read status indication
		content_name[VMSG_INDICATION_READ_STATUS] = "X-IRMC-STATUS";
		content_name[VMSG_INDICATION_READ_STATUS_READ] = "READ";
		content_name[VMSG_INDICATION_READ_STATUS_UNREAD] =  "UNREAD";

		// Mailbox status indication
		content_name[VMSG_INDICATION_MSG_BOX] = "X-IRMC-BOX";
		content_name[VMSG_INDICATION_MSG_BOX_SENT] = "SENT";
		content_name[VMSG_INDICATION_MSG_BOX_INBOX] = "INBOX";
		content_name[VMSG_INDICATION_MSG_BOX_DRAFT] =  "DRAFT";

		// Character set
		content_name[VMSG_ATTR_CHARSET_PROPERTY] = "CHARSET";

		// Language
		content_name[VMSG_ATTR_LANGUAGE_PROPERTY] =  "LANGUAGE";
		content_name[VMSG_DATA_SEPARATOR] = ":";

		content_name[VMSG_BODY_PROPERTY_DATE] = "Date";
		content_name[VMSG_BODY_PROPERTY_SUBJECT] = "Subject";
		content_name[VMSG_BODY_PROPERTY_FROM] = "From";
		content_name[VMSG_BODY_PROPERTY_TO] = "To";

		content_name[KEY_BODY] = "body";
		content_name[KEY_DATE] = "date";

		content_name[VMSG_VCARD_TEL] = "TEL";
		content_name[VMSG_MSG_BEGIN] = "BEGIN";
		content_name[VMSG_MSG_END] = "END";
		content_name[VMSG_MAXIMUM_VALUE] = "MAX";
	}
};

static int __msgsvc_vmsg_append_str(char **buf, int *buf_size, int len, const char *str, bool need_conversion)
{
	int len_temp = 0;
	char *tmp = NULL;
	const char *safe_str = SAFE_STR(str);
	int str_len = 0;
	bool need_realloc = false;

	str_len = strlen(safe_str);
	while ((*buf_size-len) < (str_len+1)) {
		*buf_size = *buf_size * 2;
		need_realloc = true;
	}

	if (need_realloc) {
		if (NULL == (tmp = (char *)realloc(*buf, *buf_size)))
			return -1;
		else
			*buf = tmp;
	}

	if (need_conversion) {
		const char *s = safe_str;
		char *r = (char *)(*buf+len);

		while (*s) {
			switch (*s) {
			case '\r':
				if (*(s+1) && '\n' == *(s+1)) {
					s++;
					*r = '\\';
					r++;
					*r = 'n';
				}
				else {
					*r = *s;
				}
				break;
			case '\n':
				*r = '\\';
				r++;
				str_len++;
				if (*buf_size<str_len+len+1) {
					*buf_size = *buf_size * 2;
					if (NULL == (tmp = (char *)realloc(*buf, *buf_size)))
						return -1;
					else {
						int pos_temp = r-(*buf+len);
						*buf = tmp;
						r = (char *)(*buf+len+pos_temp);
					}
				}
				*r = 'n';
				break;
			case ';':
			case ':':
			case ',':
			case '<':
			case '>':
			case '\\':
				*r = '\\';
				r++;
				str_len++;
				if (*buf_size<str_len+len+1) {
					*buf_size = *buf_size * 2;
					if (NULL == (tmp = (char *)realloc(*buf, *buf_size)))
						return -1;
					else {
						int pos_temp = r-(*buf+len);
						*buf = tmp;
						r = (char *)(*buf+len+pos_temp);
					}
				}
				*r = *s;
				break;
			case 0xA1:
				if (*(s+1) && 0xAC == *(s+1)) { // en/em backslash
					*r = '\\';
					r++;
					str_len++;
					if (*buf_size<str_len+len+1) {
						*buf_size = *buf_size * 2;
						if (NULL == (tmp = (char *)realloc(*buf, *buf_size)))
							return -1;
						else {
							int pos_temp = r-(*buf+len);
							*buf = tmp;
							r = (char *)(*buf+len+pos_temp);
						}
					}

					*r = *s;
					r++;
					s++;
					if (*buf_size<str_len+len+1) {
						*buf_size = *buf_size * 2;
						if (NULL == (tmp = (char *)realloc(*buf, *buf_size)))
							return -1;
						else {
							int pos_temp = r-(*buf+len);
							*buf = tmp;
							r = (char *)(*buf+len+pos_temp);
						}
					}
					*r = *s;
				}
				else {
					*r = *s;
				}
				break;
			case 0x81:
				if (*(s+1) && 0x5F == *(s+1)) { // en/em backslash
					*r = '\\';
					r++;
					str_len++;
					if (*buf_size<str_len+len+1) {
						*buf_size = *buf_size * 2;
						if (NULL == (tmp = (char *)realloc(*buf, *buf_size)))
							return -1;
						else {
							int pos_temp = r-(*buf+len);
							*buf = tmp;
							r = (char *)(*buf+len+pos_temp);
						}
					}

					*r = *s;
					r++;
					s++;
					if (*buf_size<str_len+len+1) {
						*buf_size = *buf_size * 2;
						if (NULL == (tmp = (char *)realloc(*buf, *buf_size)))
							return -1;
						else {
							int pos_temp = r-(*buf+len);
							*buf = tmp;
							r = (char *)(*buf+len+pos_temp);
						}
					}
					*r = *s;
				}
				else {
					*r = *s;
				}
				break;
			default:
				*r = *s;
				break;
			}
			r++;
			s++;
		}
		len_temp = str_len;
	}
	else {
		len_temp = snprintf(*buf+len, *buf_size-len+1, "%s", safe_str);
	}
	len += len_temp;
	return len;
}


/*==================================================================================================
                                     DEFINES
==================================================================================================*/
#define INSERT_VMSG_OBJ pObject = (VObject*)calloc(1, sizeof(VObject));	\
                                      if ( !pObject )\
{\
   vmsg_free_vtree_memory( pMessage );\
   return false;\
}\
if (pMessage->pTop == NULL)\
{\
   pMessage->pTop = pObject;\
}\
else\
{\
   pMessage->pCur->pSibling = pObject;\
}\
pMessage->pCur = pObject;


#define INSERT_VBODY_OBJ pObject = (VObject*)calloc(1, sizeof(VObject));	\
                                      if ( !pObject )\
{\
   vmsg_free_vtree_memory( pMessage );\
   return false;\
}\
if (pBody->pTop == NULL)\
{\
   pBody->pTop = pObject;\
}\
else\
{\
   pBody->pCur->pSibling = pObject;\
}\
pBody->pCur = pObject;


#define INSERT_VCARD_OBJ pObject = (VObject*)calloc(1, sizeof(VObject));	\
                                      if ( !pObject )\
{\
   vmsg_free_vtree_memory( pMessage );\
   return false;\
}\
if (pCard->pTop == NULL)\
{\
   pCard->pTop = pObject;\
}\
else\
{\
   pCard->pCur->pSibling = pObject;\
}\
pCard->pCur = pObject;


#define INSERT_PARAM  param = (VParam*)calloc(1, sizeof(VParam));\
                             if (!param)\
{\
   vmsg_free_vtree_memory( pMessage );\
   if (pObject != NULL)\
   {\
      free(pObject);\
      pObject = NULL;\
   }\
   return false;\
}


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

char* _convert_tm_to_vdata_str(const struct tm * tm)
{
	char str[17] = {0, };

	snprintf(str, 17, "%04d%02d%02dT%02d%02d%02dZ",
		tm->tm_year + 1900,
		tm->tm_mon +1,
		tm->tm_mday,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec);

	return strdup(str);
}

bool _convert_vdata_str_to_tm(const char* szText, struct tm * tm)
{

   if (szText == NULL) return false;
   if (strlen(szText) < 15) return false;
   if (szText[8] != 'T') return false;

   char szBuff[8]={0};
   memset(tm, 0, sizeof(struct tm));

   // year, month, day
   memcpy(szBuff, &(szText[0]), 4);
   szBuff[4] = '\0';
   tm->tm_year = atol(szBuff) - 1900;
   if ((tm->tm_year > 137) || (tm->tm_year < 0))
      tm->tm_year = 0;

   memcpy(szBuff, &(szText[4]), 2);
   szBuff[2] = '\0';
   tm->tm_mon = atol(szBuff)-1;
   if ((tm->tm_mon > 11) || (tm->tm_mon < 0))
      tm->tm_mon = 11;

   memcpy(szBuff, &(szText[6]), 2);
   szBuff[2] = '\0';
   tm->tm_mday = atol(szBuff);
   if ((tm->tm_mday > 31) || (tm->tm_mday < 1))
      tm->tm_mday = 31;

   // hour, minute, second
   memcpy(szBuff, &(szText[9]), 2);
   szBuff[2] = '\0';
   tm->tm_hour = atol(szBuff);
   if ((tm->tm_hour > 23) || (tm->tm_hour < 0))
      tm->tm_hour = 23;

   memcpy(szBuff, &(szText[11]), 2);
   szBuff[2] = '\0';
   tm->tm_min = atol(szBuff);
   if ((tm->tm_min > 59) || (tm->tm_min < 0))
      tm->tm_min = 59;

   memcpy(szBuff, &(szText[13]), 2);
   szBuff[2] = '\0';
   tm->tm_sec = atol(szBuff);
   if ((tm->tm_sec > 59) || (tm->tm_sec < 0))
      tm->tm_sec = 59;

   return true;
}

int msgsvc_check_utf8(char c)
{
	if ((c & 0xff) < (128 & 0xff))
		return 1;
	else if ((c & (char)0xe0) == (char)0xc0)
		return 2;
	else if ((c & (char)0xf0) == (char)0xe0)
		return 3;
	else if ((c & (char)0xf8) == (char)0xf0)
		return 4;
	else if ((c & (char)0xfc) == (char)0xf8)
		return 5;
	else if ((c & (char)0xfe) == (char)0xfc)
		return 6;
	else
		return MSG_ERR_INVALID_PARAMETER;
}

char* __msgsvc_vmsg_convert_tm_to_vdata_str( struct tm * tm)
{
	char str[22] = {0, };
	char APM[3] = {0, };
	char month[15] = {0, };
	int mon = 0;
	int hour = 0;
	mon = tm->tm_mon  +1;

	if (tm->tm_hour >= 12)
		strcpy(APM,"PM");
	else
		strcpy(APM,"AM");

	if (tm->tm_hour > 12)
		hour = tm->tm_hour - 12;
	else
		hour = tm->tm_hour;

	switch(mon)
	{
		case 1:
			strcpy(month,"Jan");
			break;
		case 2:
			strcpy(month,"Feb");
			break;
		case 3:
			strcpy(month,"Mar");
			break;
		case 4:
			strcpy(month,"Apr");
			break;
		case 5:
			strcpy(month,"May");
			break;
		case 6:
			strcpy(month,"Jun");
			break;
		case 7:
			strcpy(month,"Jul");
			break;
		case 8:
			strcpy(month,"Aug");
			break;
		case 9:
			strcpy(month,"Sep");
			break;
		case 10:
			strcpy(month,"Oct");
			break;
		case 11:
			strcpy(month,"Nov");
			break;
		case 12:
			strcpy(month,"Dec");
			break;
		default:
			MSG_DEBUG("invalid month number");
		break;
	}

	snprintf(str, 22, "%d:%02d%s, %04d %s %d",
		hour,
		tm->tm_min,
		APM,
		tm->tm_year + 1900,
		month,
		tm->tm_mday);

	return strdup(str);
}
static inline int __msgsvc_vmsg_add_folding(char **buf, int *buf_size, int buf_len)
{
	int char_len = 0;
	char *buf_copy = NULL;
	int len, result_len;
	char *r;
	const char *s;
	bool content_start = false;
	bool encode_64 = false;

	buf_copy = (char *)calloc(1, *buf_size);

	s = *buf;
	r = buf_copy;
	len = result_len = 0;

	while (*s) {
		if (*buf_size < result_len + 5) {
			char *tmp = NULL;
			*buf_size = *buf_size + 1000;
			if (NULL == (tmp = (char *)realloc(buf_copy, *buf_size))) {
				free(buf_copy);
				return -1;
			}
			else {
				buf_copy = tmp;
				r = (buf_copy + result_len);
			}
		}

		if (false == content_start) {
			if (':' == *s)
				content_start = true;
			else if (0 == strncmp(s, "ENCODING=BASE64", strlen("ENCODING=BASE64")))
				encode_64 = true;
		}

		if ('\r' == *s)
			len--;
		else if ('\n' == *s) {
			len = -1;
			char_len = 0;
			content_start = false;
			encode_64 = false;
		}

		if (0 == char_len) {
			if (false == encode_64)
				char_len = msgsvc_check_utf8(*s);

			if (MSGSVC_VMSG_FOLDING_LIMIT <= len + char_len) {
				*r = '\r';
				r++;
				*r = '\n';
				r++;
				*r = ' ';
				r++;
				len = 1;
				result_len += 3;
			}
		}

		if (char_len)
			char_len--;

		*r = *s;
		r++;
		s++;
		len++;
		result_len++;
	}
	*r = '\0';
	free(*buf);
	*buf = buf_copy;
	return result_len;
}

static inline int __msgsvc_vmsg_append_start_vmsg_1_1(char **buf, int *buf_size, int len)
{
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[BEGIN_VMSG]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VERSION_VMSG]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	return len;
}

static inline int __msgsvc_vmsg_append_end_vmsg(char **buf, int *buf_size, int len)
{
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[END_VMSG]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	return len;
}

static inline int __msgsvc_vmsg_append_read_status(MSG_MESSAGE_INFO_S *pMsg, char **buf, int *buf_size, int len)
{
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_INDICATION_READ_STATUS]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_DATA_SEPARATOR]);
	if (pMsg->bRead) {
		MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_INDICATION_READ_STATUS_READ]);
		MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	} else {
		MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_INDICATION_READ_STATUS_UNREAD]);
		MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	}
	return len;
}
static inline int __msgsvc_vmsg_append_box_type(MSG_MESSAGE_INFO_S *pMsg, char **buf, int *buf_size, int len)
{
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_INDICATION_MSG_BOX]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_DATA_SEPARATOR]);
	switch(pMsg->folderId)
	{
		case MSG_INBOX_ID:
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_INDICATION_MSG_BOX_INBOX]);

			break;
		case MSG_SENTBOX_ID:
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_INDICATION_MSG_BOX_SENT]);

			break;
		case MSG_DRAFT_ID:
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_INDICATION_MSG_BOX_DRAFT]);
			break;
		default:
			// Discard User Defined, outbox or Spam folder's messages.
			MSG_DEBUG("Invalid or unhandled msg box");
	}
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	return len;
}

static inline int __msgsvc_vmsg_append_msg_type(MSG_MESSAGE_INFO_S *pMsg, char **buf, int *buf_size, int len)
{
	//TO DO check with msg text contains Only PrintableAscii if true then else handle INET_TYPE
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_INDICATION_MESSAGE_TYPE]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_DATA_SEPARATOR]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_INDICATION_MESSAGE_TYPE_MSG]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	return len;
}

static inline int __msgsvc_vmsg_append_origin_address_vcard(MSG_MESSAGE_INFO_S *pMsg, char **buf, int *buf_size, int len)
{
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[BEGIN_VCARD]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VERSION_VCARD]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	char originAddress[MAX_ADDRESS_VAL_LEN + 1] = {0, };
	bool isDisplayName = false;

	if (pMsg->folderId == MSG_INBOX_ID) {
		strcpy(originAddress, pMsg->addressList[0].addressVal);
	}

	needCharset = true; //as per android

	if (strlen(originAddress) > 0) {
		MSG_CONTACT_INFO_S contactInfo;
		memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

		if (MsgGetContactInfo(&(pMsg->addressList[0]), &contactInfo) != MSG_SUCCESS) {
			MSG_WARN("MsgGetContactInfo() fail.");
		}
		snprintf(pMsg->addressList[0].displayName, sizeof(pMsg->addressList[0].displayName), "%s", contactInfo.firstName);
		if (pMsg->addressList[0].displayName[0] != '\0')
			isDisplayName = true;
		if (needCharset && isDisplayName) {
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[FNAME_UTF8_VCARD]);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, pMsg->addressList->displayName);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[NAME_UTF8_VCARD]);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, pMsg->addressList->displayName);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
		} else {
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[FNAME_VCARD]);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, pMsg->addressList->displayName);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[NAME_VCARD]);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, pMsg->addressList->displayName);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
		}
		if (MsgIsNumber(originAddress)) {
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[TEL_VCARD]);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[TEL_VCARD_CELL]);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, originAddress);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
		}
	} else {
		MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[FNAME_VCARD]);
		MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
		MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[NAME_VCARD]);
		MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	}


	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[END_VCARD]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	return len;
}

static inline int __msgsvc_vmsg_append_recipient_address_vcard(MSG_MESSAGE_INFO_S *pMsg, char **buf, int *buf_size, int len)
{
	//To do
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[BEGIN_VCARD]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VERSION_VCARD]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);

	needCharset = true; //as per android

	for(int i = 0; i < pMsg->nAddressCnt; ++i)
	{
		char originAddress[MAX_ADDRESS_VAL_LEN + 1] = {0, };
		bool isDisplayName = false;

		if (pMsg->folderId == MSG_SENTBOX_ID) {
			strcpy(originAddress, pMsg->addressList[0].addressVal);
		}

		if (strlen(originAddress) > 0) {
			MSG_CONTACT_INFO_S contactInfo;
			memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

			if (MsgGetContactInfo(&(pMsg->addressList[i]), &contactInfo) != MSG_SUCCESS) {
				MSG_WARN("MsgGetContactInfo() fail.");
			}
			snprintf(pMsg->addressList[i].displayName, sizeof(pMsg->addressList[i].displayName), "%s", contactInfo.firstName);
			if (pMsg->addressList[i].displayName[0] != '\0')
				isDisplayName = true;
			if (needCharset && isDisplayName) {
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[FNAME_UTF8_VCARD]);
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, pMsg->addressList[i].displayName);
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[NAME_UTF8_VCARD]);
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, pMsg->addressList[i].displayName);
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
			} else {
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[FNAME_VCARD]);
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, pMsg->addressList[i].displayName);
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[NAME_VCARD]);
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, pMsg->addressList[i].displayName);
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
			}
			if (MsgIsNumber(originAddress)) {
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[TEL_VCARD]);
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[TEL_VCARD_CELL]);
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, originAddress);
				MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
			}
		} else {
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[FNAME_VCARD]);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[NAME_VCARD]);
			MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
		}
	}

	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[END_VCARD]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);

	return len;
}

static inline int __msgsvc_vmsg_append_msg_body(MSG_MESSAGE_INFO_S *pMsg, char **buf, int *buf_size, int len)
{
	struct tm	display_time;

	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[BEGIN_VENV]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);

	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[BEGIN_VBODY]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);

	//Date:
	tzset();
	localtime_r(&(pMsg->displayTime), &display_time);
	char *msgDate = __msgsvc_vmsg_convert_tm_to_vdata_str(&display_time);

	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_BODY_PROPERTY_DATE]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_DATA_SEPARATOR]);
	if (msgDate !=NULL) {
		MSGSVC_VMSG_APPEND_STR_FREE(buf, buf_size, len, msgDate);
		MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
		g_free(msgDate);
	}

	//Subject:
	if (pMsg->subject[0] != '\0') {
		MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_BODY_PROPERTY_SUBJECT]);
		MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[VMSG_DATA_SEPARATOR]);
		MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, pMsg->subject);
		MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	}

	//body:
	if(pMsg->msgType.mainType == MSG_SMS_TYPE)
	{
		if(pMsg->msgType.subType == MSG_NORMAL_SMS)
		{
			if (pMsg->bTextSms == false)
			{
				char* pFileData = NULL;
				unique_ptr<char*, void(*)(char**)> buff(&pFileData, unique_ptr_deleter);

				int		fileSize = 0;
				char*	msgText = NULL;

				if (MsgOpenAndReadFile(pMsg->msgData, &pFileData, &fileSize) == false)
					return len;

				msgText = (char *)calloc(1, fileSize);
				memcpy(msgText, pFileData, fileSize);
				MSGSVC_VMSG_APPEND_STR_FREE(buf,buf_size, len, msgText);
				g_free(msgText);
			}
			else
			{
				MSGSVC_VMSG_APPEND_STR(buf,buf_size, len, pMsg->msgText);
			}
		}
	}

	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);

	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[END_VBODY]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);

	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[END_VENV]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	return len;
}

static inline int __msgsvc_vmsg_append_msg_envelope(MSG_MESSAGE_INFO_S *pMsg, char **buf, int *buf_size, int len)
{

	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[BEGIN_VENV]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);

	len = __msgsvc_vmsg_append_recipient_address_vcard(pMsg, buf, buf_size, len);
	MSG_ERR_RET_VM(len < 0, len, "Invalid length : vcard");

	len = __msgsvc_vmsg_append_msg_body(pMsg, buf, buf_size, len);
	MSG_ERR_RET_VM(len < 0, len, "Invalid length : body");

	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, content_name[END_VENV]);
	MSGSVC_VMSG_APPEND_STR(buf, buf_size, len, MSGSVC_CRLF);
	return len;
}

static inline int __msgsvc_vmsg_append_msg(MSG_MESSAGE_INFO_S *pMsg, char **buf, int *buf_size, int len)
{
	len = __msgsvc_vmsg_append_read_status(pMsg, buf, buf_size, len);
	MSG_ERR_RET_VM(len < 0, len, "Invalid length : read status");

	len = __msgsvc_vmsg_append_box_type(pMsg, buf, buf_size, len);
	MSG_ERR_RET_VM(len < 0, len, "Invalid length : box type");

	len = __msgsvc_vmsg_append_msg_type(pMsg, buf, buf_size, len);
	MSG_ERR_RET_VM(len < 0, len, "Invalid length : msg type");

	len = __msgsvc_vmsg_append_origin_address_vcard(pMsg, buf, buf_size, len);
	MSG_ERR_RET_VM(len < 0, len, "Invalid length : origin address");

	len = __msgsvc_vmsg_append_msg_envelope(pMsg, buf, buf_size, len);
	MSG_ERR_RET_VM(len < 0, len, "Invalid length : envelop");

	return len;
}

char *MsgVMessageEncode(MSG_MESSAGE_INFO_S *pMsg)
{
	MSG_BEGIN();

	VObject*		pObject = NULL;
	VParam*		param = NULL;
	VTree *		pBody = NULL;
	VTree *		pCard = NULL;
	VTree *		pCurrent= NULL;
	struct tm	display_time;
	char*		encoded_data = NULL;

	VTree* pMessage = NULL;

	//to encode sms
	if (pMsg->msgType.mainType == MSG_SMS_TYPE && pMsg->msgType.subType == MSG_NORMAL_SMS) {
		return MsgVMessageEncodeSMS(pMsg);
	}

	//to encode mms
	if (pMessage == NULL) {
		pMessage = (VTree *)malloc(sizeof(VTree));
		if (!pMessage) {
			return NULL;
		}
		pMessage->treeType = VMESSAGE;
		pMessage->pTop = NULL;
		pMessage->pCur = NULL;
		pMessage->pNext = NULL;

	}
		pCurrent = pMessage;

	//Insert VObject (X-MESSAGE-TYPE) to VMessage tree
	INSERT_VMSG_OBJ;

	pObject->property = VMSG_TYPE_MSGTYPE;

	if (pMsg->msgType.mainType == MSG_MMS_TYPE && (pMsg->msgType.subType == MSG_SENDREQ_MMS || pMsg->msgType.subType == MSG_SENDCONF_MMS))
		pObject->pszValue[0] = strdup("MMS SEND");
#if 0
	else if(pMsg->msgType.mainType == MSG_MMS_TYPE && pMsg->msgType.subType == MSG_NOTIFICATIONIND_MMS)
		pObject->pszValue[0] = strdup("MMS NOTIFICATION");
#endif

	else if (pMsg->msgType.mainType == MSG_MMS_TYPE && (pMsg->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsg->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS))
		pObject->pszValue[0] = strdup("MMS RETRIEVED");

	else
		goto __CATCH_FAIL__;

	pObject->valueCount = 1;


	//Insert VObject (X-IRMC-BOX) to VMessate tree
	INSERT_VMSG_OBJ;

	pObject->property = VMSG_TYPE_MSGBOX;

	switch(pMsg->folderId)
	{
		case MSG_INBOX_ID:
			pObject->pszValue[0] = strdup("INBOX");
			break;
		case MSG_OUTBOX_ID:
			pObject->pszValue[0] = strdup("OUTBOX");
			break;
		case MSG_SENTBOX_ID:
			pObject->pszValue[0] = strdup("SENTBOX");
			break;
		case MSG_DRAFT_ID:
			pObject->pszValue[0] = strdup("DRAFTBOX");
			break;
		default:
			// Discard User Defined or Spam folder's messages.
			goto __CATCH_FAIL__;
	}
	pObject->valueCount = 1;


	//Insert VObject (X-SS-DT) to VMessage tree
	INSERT_VMSG_OBJ;

	pObject->property = VMSG_TYPE_DATE;
	tzset();
	localtime_r(&(pMsg->displayTime), &display_time);
	pObject->pszValue[0] = _convert_tm_to_vdata_str(&display_time);
	pObject->valueCount = 1;


	//Insert Vobject read status to VMessage tree
	INSERT_VMSG_OBJ;

	pObject->property = VMSG_TYPE_STATUS;

	if (pMsg->bRead)
		pObject->pszValue[0] = strdup("READ");
	else
		pObject->pszValue[0] = strdup("UNREAD");

	pObject->valueCount = 1;


	//Insert VBody  tree for message body;
	pBody = (VTree*)calloc(1, sizeof(VTree));
	if ( !pBody )
		goto __CATCH_FAIL__;
	pBody->treeType = VBODY;
	pBody->pTop = NULL;
	pBody->pCur = NULL;
	pBody->pNext = NULL;
	pCurrent->pNext = pBody;
	pCurrent = pBody;

	if (strlen(pMsg->subject) > 0) {
		//Insert Subject object
		INSERT_VBODY_OBJ;
		pObject->property = VMSG_TYPE_SUBJECT;
		pObject->pszValue[0] = strdup(pMsg->subject);
		pObject->valueCount = 1;
	}
	//Insert VBody object
	INSERT_VBODY_OBJ;
	pObject->property = VMSG_TYPE_BODY;

	if (pMsg->msgType.mainType == MSG_MMS_TYPE) {
		//Insert VBody for mms raw data;
		char* pFileData = NULL;
		unique_ptr<char*, void(*)(char**)> buf(&pFileData, unique_ptr_deleter);
		MMS_DATA_S *pMmsData = NULL;
		int fileSize = 0;
		char* msgText = NULL;
#if 0
		char		filePath[MSG_FILEPATH_LEN_MAX] = {0, };
		if(pMsg->msgType.subType == MSG_NOTIFICATIONIND_MMS)
			pFileData = MsgOpenAndReadMmsFile(pMsg->msgData, 0, -1, &fileSize);

		else
		{
			err = MsgStoGetMmsRawFilePath(pDbHandle, pMsg->msgId, filePath);

			if (err != MSG_SUCCESS)
				goto __CATCH_FAIL__;

			pFileData = MsgOpenAndReadMmsFile(filePath, 0, -1, &fileSize);
		}
#else

		if (pMsg->bTextSms == false) {
			if (MsgOpenAndReadFile(pMsg->msgData, &pFileData, &fileSize) == false) {
				//CID 16205: Memory leak in case of failure
				goto __CATCH_FAIL__;
			}
		} else {
			fileSize = strlen(pMsg->msgData);
			pFileData = (char *)calloc(1, fileSize+1);
			if (!pFileData)
				goto __CATCH_FAIL__;
			snprintf(pFileData, fileSize, "%s", pMsg->msgData);
		}

		if (MsgDeserializeMmsData(pFileData, fileSize, &pMmsData) != 0) {
			MSG_DEBUG("Fail to Deserialize Message Data");
			MsgMmsRelease(&pMmsData);
			goto __CATCH_FAIL__;
		}

		int serializedDataSize = 0;

		if (pMmsData) {
			MsgMmsSetMultipartListData(pMmsData);//app file -> data
			serializedDataSize = MsgSerializeMms(pMmsData, &pFileData);
		}

		if (pFileData) {
			fileSize = serializedDataSize;
		}

		MsgMmsRelease(&pMmsData);

#endif
		MSG_DEBUG("FILE SIZE IS %d, %s", fileSize, pFileData);
		msgText = (char *)calloc(1, fileSize);
		if(pFileData && msgText)
			memcpy(msgText, pFileData, fileSize);

		pObject->numOfBiData = fileSize;
		pObject->pszValue[0] = msgText;
		pObject->valueCount = 1;

		if (pFileData) {
			free(pFileData);
			pFileData = NULL;
		}
	}

	// Insert parameter for base64 encoding
	MSG_DEBUG("before to start INSERT_PARAM");
	INSERT_PARAM;
	pObject->pParam = param;
	param->parameter = VMSG_PARAM_ENCODING;
	param->paramValue = VMSG_ENC_PARAM_BASE64;

	// Add VCard tree for recipient address information.
	for (int i = 0; i < pMsg->nAddressCnt; ++i) {
		pCard = (VTree*)calloc(1, sizeof(VTree));
		if ( !pCard )
			goto __CATCH_FAIL__;

		pCard->treeType = VCARD;
		pCard->pTop = NULL;
		pCard->pCur = NULL;
		pCard->pNext = NULL;
		pCurrent->pNext = pCard;
		pCurrent = pCard;

		INSERT_VCARD_OBJ;
		pObject->property = VCARD_TYPE_TEL;
		pObject->pszValue	[0] = strdup(pMsg->addressList[i].addressVal);
		pObject->valueCount = 1;
	}
	MSG_DEBUG("before to start vmsg_encode");
	encoded_data = vmsg_encode(pMessage);

	vmsg_free_vtree_memory(pMessage);
	MSG_END();
	return encoded_data;

__CATCH_FAIL__ :
	vmsg_free_vtree_memory( pMessage );

	return NULL;
}

char *MsgVMessageEncodeSMS(MSG_MESSAGE_INFO_S *pMsg)
{

	MSG_BEGIN();

	char *buf = NULL;
	int buf_size = VMSG_INIT_LENGTH;
	int len = 0;

	__msgsvc_vmsg_initial();

	buf = (char *)calloc(1, buf_size);

	len = __msgsvc_vmsg_append_start_vmsg_1_1(&buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return NULL;
	}

	len = __msgsvc_vmsg_append_msg(pMsg, &buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return NULL;
	}

	len = __msgsvc_vmsg_append_end_vmsg(&buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return NULL;
	}

	len = __msgsvc_vmsg_add_folding(&buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return NULL;
	}

	MSG_END();
	return buf;
}
bool __msgsvc_vmsg_convert_vdata_to_tm_str(const char* szText, struct tm * tm)
{
	if (szText == NULL)
		return false;

	char delims[] = ",";
	gchar **token = NULL;
	struct tm tmTemp;

	token = g_strsplit_set(szText, delims, -1);
	if (token && token[0]) {
		g_strstrip(token[0]);
		strptime(token[0], "%I:%M%p", tm);
	}else {
		g_strfreev(token);
		return false;
	}

	if (token && token[1]) {
		g_strstrip(token[1]);
		strptime(token[1], "%Y %b %e", &tmTemp);
	} else {
		g_strfreev(token);
		return false;
	}

	tm->tm_year = tmTemp.tm_year;
	tm->tm_mon = tmTemp.tm_mon;
	tm->tm_mday = tmTemp.tm_mday;
	tm->tm_sec = 0;

	g_strfreev(token);
	return true;
}


static inline char* __msgsvc_vmsg_remove_empty_line(char *src)
{
	while (*src) {
		if ('\n' != *src && '\r' != *src)
			break;
		src++;
	}
	return src;
}

static char* __msgsvc_vmsg_check_word(char *src, const char *word)
{
	bool start = false;

	MSG_ERR_RET_VM(NULL == src, NULL, "The src is NULL.");

	src = __msgsvc_vmsg_remove_empty_line(src);

	while (*src) {
		switch (*src) {
		case ' ':
		case ':':
		case ';':
			src++;
			break;
		default:
			start = true;
			break;
		}
		if (start) break;
	}

	while (*src == *word) {
		src++;
		word++;

		if ('\0' == *src || '\0' == *word)
			break;
	}

	if ('\0' == *word)
		return src;
	else
		return NULL;
}

static inline int __msgsvc_vmsg_check_quoted(char *src, int max, int *quoted)
{
	int ret;
	if (TRUE == *quoted)
		return TRUE;

	while (*src && max) {
		if ('Q' == *src) {
			ret = strncmp(src, "QUOTED-PRINTABLE", sizeof("QUOTED-PRINTABLE") - 1);
			if (!ret) {
				*quoted = TRUE;
				return TRUE;
			}
		}else if (':' == *src) {
			break;
		}
		src++;
		max--;
	}
	return FALSE;
}

static inline int __msgsvc_vmsg_remove_folding(char *folded_src)
{
	char *result = folded_src;

	MSG_ERR_RET_VM(NULL == folded_src, MSG_ERR_INVALID_PARAMETER, " Invalid Parameter : __msgsvc_vmsg_remove_folding");

	while (*folded_src) {
		if ('\r' == *folded_src && '\n' == *(folded_src+1) && ' ' == *(folded_src+2))
			folded_src += 3;
		else if ('\n' == *folded_src && ' ' == *(folded_src+1))
			folded_src += 2;

		if ('\0' == *folded_src)
			break;

		*result = *folded_src;
		result++;
		folded_src++;
	}
	*result = '\0';
	return MSG_SUCCESS;
}

static inline int __msgsvc_vmsg_hex_to_dec(char hex)
{
	switch (hex) {
	case '0' ... '9':
		return hex - '0';
	case 'a' ... 'f':
		return hex - 'a' + 10;
	case 'A' ... 'F':
		return hex - 'A' + 10;
	default:
		return -1;
	}
}
static inline int __msgsvc_vmsg_decode_quoted_val(char *val)
{
	char *src, *dest;
	int pre;

	src = strchr(val, ':');
	if (NULL == src)
		src = val;

	dest = src;
	while (*src) {
		if ('=' == *src) {
			pre = __msgsvc_vmsg_hex_to_dec(*(src+1));
			if (0 <= pre) {
				*dest = (char)((pre << 4) + __msgsvc_vmsg_hex_to_dec(*(src+2)));
				dest++;
				src += 2;
			} else {
				if ('\r' == *(src+1) && '\n' == *(src+2))
					src += 2;
			}
		} else {
			*dest = *src;
			dest++;
		}
		src++;
	}

	*dest = '\0';
	return dest - val;
}

static inline char* __msgsvc_vmsg_translate_charset(char *src, int len)
{
	int ret;
	char *val = src;

	while (*val) {
		if ('C' == *val) {
			ret = strncmp(val, "CHARSET", sizeof("CHARSET") - 1);
			if (!ret) {
				val += sizeof("CHARSET");
				break;
			}
		}
		else if (':' == *val) {
			return NULL;
		}
		val++;
	}

	if (*val) {
		UChar *temp;
		UConverter *conv;
		UErrorCode err = U_ZERO_ERROR;
		int dest_size = 0;
		int temp_size = 0;
		int src_len, i = 0;
		char enc[32] = {0}, *dest;

		while (';' != *val && ':' != *val  && ',' != *val) {
			enc[i++] = *val++;
		}
		enc[i] = '\0';
		if (0 == strcasecmp("UTF-8", enc))
			return NULL;

		while (':' != *val)
			val++;

		src_len = len - (val - src);

		temp_size = (src_len+1) * sizeof(UChar);
		temp = (UChar *)malloc(temp_size);
		conv = ucnv_open(enc, &err);
		MSG_WARN_M(U_FAILURE(err), "ucnv_open() Failed(%d), enc=%s", err, enc);
		ucnv_toUChars(conv, temp, temp_size, val, src_len, &err);
		MSG_WARN_M(U_FAILURE(err), "ucnv_toUChars() Failed(%d), enc=%s", err, enc);
		ucnv_close(conv);

		dest_size = temp_size*2;
		dest = (char *)malloc(dest_size);
		conv = ucnv_open("UTF-8", &err);
		MSG_WARN_M(U_FAILURE(err), "ucnv_open() Failed(%d), enc=%s", err, enc);
		ucnv_fromUChars(conv, dest, dest_size, temp, u_strlen(temp), &err);
		MSG_WARN_M(U_FAILURE(err), "ucnv_fromUChars() Failed(%d), enc=%s", err, enc);
		ucnv_close(conv);
		free(temp);

		return dest;
	}
	return NULL;
}

static void __msgsvc_vmsg_get_prefix(char **prefix, char *src)
{
	char *temp = strchr(src, ':');
	if (temp) {
		long len = (long)temp - (long)src;
		*prefix = (char *)calloc(len+1, sizeof(char));
		snprintf(*prefix, len+1, "%s", src);
	}
	else {
		*prefix = NULL;
	}
}

static void __msgsvc_vmsg_remove_spec_out(char **prefix)
{
	if (NULL == prefix)
		return;
	char *p = *prefix;
	while (p && *p) {
		if (*p == '(') {
			*p = '\0';
			return;
		}
		p++;
	}
}

static char* __msgsvc_vmsg_get_val(int ver, char *src, char **prefix, char **dest)
{
	int quoted;
	bool start = false;
	char *cursor;

	MSG_ERR_RET_VM(NULL == src, NULL, "Invalid parameter : The src is NULL.");
	MSG_ERR_RET_VM(NULL == dest, NULL, "sInvalid parameter : The dest is NULL.");

	while (*src) {
		switch (*src) {
		case '\n':
			return NULL;
		case '\r':
		case ' ':
			src++;
			break;
		default:
			start = true;
			break;
		}
		if (start) break;
	}

	quoted = FALSE;
	cursor = src;
	if (MSGSVC_VMSG_VER_1_1 == ver) {
		while (*cursor) {
			if ('=' == *cursor && __msgsvc_vmsg_check_quoted(src, cursor - src, &quoted)) {
				if ('\r' == *(cursor+1) && '\n' == *(cursor+2))
					cursor += 2;
			} else {
				if ('\r' == *cursor && '\n' == *(cursor+1) && ' ' != *(cursor+2))
					break;
				if ('\n' == *cursor && ' ' != *(cursor+1))
					break;
			}

			cursor++;
		}
	}
	else {
		while (*cursor) {
			if ('\r' == *cursor && '\n' == *(cursor+1) && ' ' != *(cursor+2))
				break;

			if ('\n' == *cursor && ' ' != *(cursor+1))
				break;

			cursor++;
		}
	}

	if (src == cursor) {
		*dest = NULL;
		return NULL;
	}
	else {
		int len = 0;
		char temp = *cursor;
		char *new_dest;

		if (prefix)
			__msgsvc_vmsg_get_prefix(prefix, src);

		__msgsvc_vmsg_remove_spec_out(prefix);

		*cursor = '\0';
		*dest = strdup(src);
		if (MSGSVC_VMSG_VER_1_1 != ver)
			__msgsvc_vmsg_remove_folding(*dest);

		if (__msgsvc_vmsg_check_quoted(*dest, -1, &quoted))
			len = __msgsvc_vmsg_decode_quoted_val(*dest);
		if (0 == len)
			len = strlen(*dest);
		new_dest = __msgsvc_vmsg_translate_charset(*dest, len);
		if (new_dest) {
			free(*dest);
			*dest = new_dest;
		}
		*cursor = temp;
		return (cursor + 1);
	}
}

static int  __msgsvc_vmsg_check_content_type(char **vcard)
{
	int i;
	char *new_start;
	for (i = VMSG_VALUE_NONE+1; i < VMSG_MAXIMUM_VALUE; i++) {
		new_start = __msgsvc_vmsg_check_word(*vcard, content_name[i]);
		if (new_start && (':' == *new_start || ';' == *new_start))
			break;
	}

	if (VMSG_MAXIMUM_VALUE == i)
		return VMSG_VALUE_NONE;
	else {
		*vcard = new_start;
		return i;
	}
}

static inline bool __msgsvc_vmsg_check_base64_encoded(char *src)
{
	int ret;
	char *tmp = src;

	while (*tmp) {
		if ('B' == *tmp) {
			ret = strncmp(tmp, "BASE64", sizeof("BASE64") - 1);
			if (!ret)
				return true;
		} else if (':' == *tmp || '\r' == *tmp) {
			break;
		}
		tmp++;
	}
	return false;
}

static char* __msgsvc_vmsg_decode_base64_val(char *val)
{
	gsize size = 0;
	guchar *decoded_str;
	char *src;
	char *dest = NULL;

	src = strchr(val, ':');
	if (NULL == src)
		src = val;
	else
		src++;

	decoded_str = g_base64_decode(src, &size);

	dest = (char *)calloc((src-val)+size+1, sizeof(char));
	snprintf(dest, (src-val)+1, "%s", val);
	snprintf(dest+(src-val), size+1, "%s", decoded_str);
	g_free(decoded_str);

	return dest;
}

static inline char*  __msgsvc_vmsg_pass_unsupported(char *vmsg)
{
	while (*vmsg) {
		if ('\n' == *vmsg)
			return (vmsg + 1);
		vmsg++;
	}

	return NULL;
}

static inline char* __msgsvc_vmsg_get_content_value(char *val)
{
	char *temp;

	temp = strchr(val, ':');
	if (temp)
		temp++;
	else
		temp = val;

	MSG_ERR_RET_VM('\0' == *(temp) || '\r' == *(temp) || '\n' == *(temp),
		NULL, "Invalid vcard content");

	return temp;
}

static char* __msgsvc_vmsg_remove_escape_char(char *str)
{
	const char *s = SAFE_STR(str);
	char *r = (char *)s;
	while (*s) {
		if (*s == '\\' && *(s+1)) {
			char *n = (char*)(s+1);
			switch (*n) {
			case 'n':
			case 'N':
				*r = '\n';
				s++;
				break;
			case ';':
			case ':':
			case ',':
			case '<':
			case '>':
			case '\\':
				*r = *n;
				s++;
				break;
			case 0xA1: // en/em backslash
				if (*(n+1) && 0xAC == *(n+1)) {
					*r = *n;
					r++;
					*r = *(n+1);
					s+=2;
				}
				break;
			case 0x81:  // en/em backslash
				if (*(n+1) && 0x5F == *(n+1)) {
					*r = *n;
					r++;
					*r = *(n+1);
					s+=2;
				}
				break;
			default:
				*r = *s;
				break;
			}
			r++;
			s++;
		}
		else {
			*r = *s;
			r++;
			s++;
		}
	}
	*r = '\0';
	return str;
}

static inline msg_error_t __msgsvc_vmsg_get_read_status(MSG_MESSAGE_INFO_S *pMsg, char *val)
{
	char *temp;

	temp = __msgsvc_vmsg_get_content_value(val);
	MSG_ERR_RET_VM(NULL == temp, MSG_ERR_INVALID_PARAMETER, "Invalid parameter : read status");

	temp = __msgsvc_vmsg_remove_escape_char(temp);

	if (strcmp(temp, content_name[VMSG_INDICATION_READ_STATUS_READ]) == 0)
		pMsg->bRead = true;
	else if (strcmp(temp, content_name[VMSG_INDICATION_READ_STATUS_UNREAD]) == 0)
		pMsg->bRead = false;
	else
		return MSG_ERR_INVALID_PARAMETER;
	MSG_DEBUG("pMsg->bRead = %d",pMsg->bRead);
	return MSG_SUCCESS;
}

static inline msg_error_t __msgsvc_vmsg_get_msg_box(MSG_MESSAGE_INFO_S *pMsg, char *val)
{
	char *temp;

	temp = __msgsvc_vmsg_get_content_value(val);
	MSG_ERR_RET_VM(NULL == temp, MSG_ERR_INVALID_PARAMETER, "Invalid parameter : msg box");

	temp = __msgsvc_vmsg_remove_escape_char(temp);

	if (strcmp(temp, content_name[VMSG_INDICATION_MSG_BOX_DRAFT]) == 0) {
		pMsg->folderId = MSG_DRAFT_ID;
		pMsg->direction=MSG_DIRECTION_TYPE_MO;
		pMsg->networkStatus=MSG_NETWORK_NOT_SEND;
	}
	else if (strcmp(temp, content_name[VMSG_INDICATION_MSG_BOX_INBOX]) == 0) {
		pMsg->folderId = MSG_INBOX_ID;
		pMsg->direction=MSG_DIRECTION_TYPE_MT;
		pMsg->networkStatus=MSG_NETWORK_RECEIVED;
	}
	else if (strcmp(temp, content_name[VMSG_INDICATION_MSG_BOX_SENT]) == 0) {
		pMsg->folderId = MSG_SENTBOX_ID;
		pMsg->direction=MSG_DIRECTION_TYPE_MO;
		pMsg->networkStatus=MSG_NETWORK_SEND_SUCCESS;
	}
	else
		return MSG_ERR_INVALID_PARAMETER;
	MSG_DEBUG("pMsg->folderId = %d",pMsg->folderId);
	return MSG_SUCCESS;
}

static inline msg_error_t __msgsvc_vmsg_get_msg_type(MSG_MESSAGE_INFO_S *pMsg, char *val)
{
	char *temp;

	temp = __msgsvc_vmsg_get_content_value(val);
	MSG_ERR_RET_VM(NULL == temp, MSG_ERR_INVALID_PARAMETER, "Invalid parameter : msg type");

	temp = __msgsvc_vmsg_remove_escape_char(temp);

	if (strcmp(temp, content_name[VMSG_INDICATION_MESSAGE_TYPE_MSG]) == 0) {
		pMsg->msgType.mainType = MSG_SMS_TYPE;
		pMsg->msgType.subType = MSG_NORMAL_SMS;
		pMsg->msgType.classType = MSG_CLASS_NONE;
	}
	else if (strcmp(temp, content_name[VMSG_INDICATION_MESSAGE_TYPE_INET]) == 0) {
		//To do
	}
	else
		return MSG_ERR_INVALID_PARAMETER;
	MSG_DEBUG("pMsg->msgType.subType = %d",pMsg->msgType.subType);
	return MSG_SUCCESS;
}

static inline msg_error_t __msgsvc_vmsg_get_address(MSG_MESSAGE_INFO_S *pMsg, char *prefix, char *val, int* vCardCnt)
{
	char *temp;
	MSG_DEBUG("vCardCnt is : %d",*vCardCnt);
	if ((pMsg->folderId == MSG_SENTBOX_ID || pMsg->folderId == MSG_DRAFT_ID) && *vCardCnt == 1)
		return MSG_SUCCESS;

	if (pMsg->folderId == MSG_INBOX_ID && *vCardCnt > 1)
		return MSG_SUCCESS;

	temp = __msgsvc_vmsg_get_content_value(val);
	MSG_ERR_RET_VM(NULL == temp, MSG_ERR_INVALID_PARAMETER, "Invalid parameter : address");

	temp = __msgsvc_vmsg_remove_escape_char(temp);
	MSG_ADDRESS_INFO_S * addrInfo = NULL;

	pMsg->nAddressCnt++;
	MSG_DEBUG("Address is : %s",temp);
	MSG_DEBUG("Address Cnt : %d",pMsg->nAddressCnt);


	if (pMsg->addressList == NULL) {
		addrInfo = (MSG_ADDRESS_INFO_S *)calloc(1, sizeof(MSG_ADDRESS_INFO_S));
	} else {
		addrInfo = (MSG_ADDRESS_INFO_S *)realloc(pMsg->addressList, pMsg->nAddressCnt * sizeof(MSG_ADDRESS_INFO_S));
	}

	if (addrInfo == NULL) {
		return MSG_ERR_INVALID_PARAMETER;
	}
	pMsg->addressList = addrInfo;

	pMsg->addressList[pMsg->nAddressCnt-1].addressType = MSG_ADDRESS_TYPE_PLMN;
	pMsg->addressList[pMsg->nAddressCnt-1].recipientType = MSG_RECIPIENTS_TYPE_TO;
	strncpy(pMsg->addressList[pMsg->nAddressCnt-1].addressVal, temp, MAX_ADDRESS_VAL_LEN);
	MSG_DEBUG("pMsg->addressList[pMsg->nAddressCnt-1].addressVal = %s",pMsg->addressList[pMsg->nAddressCnt-1].addressVal);
	return MSG_SUCCESS;
}

static inline msg_error_t __msgsvc_vmsg_get_msg_date(MSG_MESSAGE_INFO_S *pMsg, char *val)
{
	char *temp;

	temp = __msgsvc_vmsg_get_content_value(val);
	MSG_ERR_RET_VM(NULL == temp, MSG_ERR_INVALID_PARAMETER, "Invalid parameter : date");

	temp = __msgsvc_vmsg_remove_escape_char(temp);
	MSG_DEBUG("pMsg->displayTime = %s", temp);
	struct tm displayTime;
	if ( __msgsvc_vmsg_convert_vdata_to_tm_str(temp, &displayTime))
		pMsg->displayTime = mktime(&displayTime);
	else
		pMsg->displayTime = time(NULL);
	return MSG_SUCCESS;
}

static inline msg_error_t __msgsvc_vmsg_get_msg_subject(MSG_MESSAGE_INFO_S *pMsg, char *val)
{
	char *temp;

	temp = __msgsvc_vmsg_get_content_value(val);
	MSG_ERR_RET_VM(NULL == temp, MSG_ERR_INVALID_PARAMETER, "Invalid parameter : subject");

	temp = __msgsvc_vmsg_remove_escape_char(temp);

	if (temp && temp[0] != '\0')
		strncpy(pMsg->subject, temp, MAX_SUBJECT_LEN);
	MSG_DEBUG("pMsg->subject = %s",pMsg->subject);
	return MSG_SUCCESS;
}

static inline bool __msgsvc_vmsg_get_msg_begin(MSG_MESSAGE_INFO_S *pMsg, char *val, int *vCardCnt)
{
	pMsg->encodeType = MSG_ENCODE_AUTO;

	char *temp;

	temp = __msgsvc_vmsg_get_content_value(val);
	MSG_ERR_RET_VM(NULL == temp, MSG_ERR_INVALID_PARAMETER, "Invalid parameter : body");

	temp = __msgsvc_vmsg_remove_escape_char(temp);

	if (temp && temp[0] != '\0' && strcmp(temp, "VCARD") == 0) {
		(*vCardCnt)++;
		return true;
	}
	return false;
}

static inline bool __msgsvc_vmsg_get_msg_end(MSG_MESSAGE_INFO_S *pMsg, char *val)
{
	pMsg->encodeType = MSG_ENCODE_AUTO;

	char *temp;

	temp = __msgsvc_vmsg_get_content_value(val);
	MSG_ERR_RET_VM(NULL == temp, MSG_ERR_INVALID_PARAMETER, "Invalid parameter : body");

	temp = __msgsvc_vmsg_remove_escape_char(temp);

	if (temp && temp[0] != '\0' && strcmp(temp, "VMSG") == 0) {
		MSG_DEBUG("VMessage decoding completed");
		return true;
	} else if (temp && temp[0] != '\0' && strcmp(temp, "VCARD") == 0) {
		MSG_DEBUG("VMessage decoding completed");
		return false;
	}
	return false;
}

static inline msg_error_t __msgsvc_vmsg_get_msg(int ver, char *vmsg, MSG_MESSAGE_INFO_S  *record)
{
	int type;
	char *cursor, *new_start, *val, *prefix;

	MSG_MESSAGE_INFO_S *pMsg = record;
	int vCardCnt = 0;
	bool isDateAvailable = false;
	pMsg->msgId = 0;
	pMsg->threadId = 0;
	bool end = false;

	cursor = vmsg;
	while (cursor) {
		val = NULL;
		prefix = NULL;
		bool base64_encoded = false;
		char *bodyStart = cursor;
		type =  __msgsvc_vmsg_check_content_type(&cursor);

		if (VMSG_VALUE_NONE == type) {

			if (isDateAvailable == true) {
				isDateAvailable = false;
				//body decoding
				MSG_DEBUG("Decoding body :");
				bodyStart = __msgsvc_vmsg_pass_unsupported(bodyStart);
				int i =0;
				char tempMsgText[MAX_MSG_TEXT_LEN + 1] = {0, };
				while (bodyStart) {
					if (i >= MAX_MSG_TEXT_LEN)
						break;
					if (*bodyStart == 'E' && *(bodyStart+1) == 'N' && *(bodyStart+2) == 'D' && *(bodyStart+3) == ':') {
						i++;
						break;
					}
					tempMsgText[i] = *bodyStart;
					bodyStart++;
					i++;
				}
				tempMsgText[i] = '\0';
				char * temp = __msgsvc_vmsg_remove_escape_char(tempMsgText);
				strcpy(pMsg->msgText, temp);
				MSG_DEBUG("pMsg->msgText : %s", pMsg->msgText);
				pMsg->dataSize = strlen(pMsg->msgText);
				pMsg->bTextSms = true;
				base64_encoded = __msgsvc_vmsg_check_base64_encoded(pMsg->msgText);

				if (base64_encoded) {
					char * decodedText = NULL;
					decodedText = __msgsvc_vmsg_decode_base64_val(pMsg->msgText);
					if (decodedText) {
						strncpy(pMsg->msgText, decodedText, MAX_MSG_TEXT_LEN);
						pMsg->dataSize = strlen(pMsg->msgText);
						free(decodedText);
					} else {
						strcpy(pMsg->msgText, "");
						pMsg->dataSize = 0;
					}
				}
				g_free(prefix);
				g_free(val);
				return MSG_SUCCESS;
			}else {
				new_start =  __msgsvc_vmsg_pass_unsupported(cursor);
				if (new_start) {
					cursor = new_start;
					continue;
				}
				else
					break;
			}
		}

		base64_encoded = __msgsvc_vmsg_check_base64_encoded(cursor);

		new_start = __msgsvc_vmsg_get_val(ver, cursor, &prefix, &val);

		if (NULL == new_start) {
			g_free(prefix);
			g_free(val);
			continue;
		}

		if (NULL == val) {
			cursor = new_start;
			g_free(prefix);
			g_free(val);
			continue;
		}

		if (base64_encoded) {
			char *temp = __msgsvc_vmsg_decode_base64_val(val);
			g_free(val);
			val = temp;
		}
		switch (type) {
		case VMSG_INDICATION_READ_STATUS:
			__msgsvc_vmsg_get_read_status(pMsg, val);
			break;
		case VMSG_INDICATION_MSG_BOX:
			__msgsvc_vmsg_get_msg_box(pMsg, val);
			break;
		case VMSG_INDICATION_MESSAGE_TYPE:
			__msgsvc_vmsg_get_msg_type(pMsg, val);
			break;
		case VMSG_VCARD_TEL:
			__msgsvc_vmsg_get_address(pMsg, prefix, val, &vCardCnt);
			break;
		case VMSG_BODY_PROPERTY_DATE:
			isDateAvailable = true;
			__msgsvc_vmsg_get_msg_date(pMsg, val);
			break;
		case VMSG_BODY_PROPERTY_SUBJECT:
			__msgsvc_vmsg_get_msg_subject(pMsg, val);
			break;
		case VMSG_MSG_BEGIN:
			end = __msgsvc_vmsg_get_msg_begin(pMsg, val, &vCardCnt);
		case VMSG_MSG_END:
			end = __msgsvc_vmsg_get_msg_end(pMsg, val);
			if (end) {
				g_free(val);
				g_free(prefix);
				return MSG_SUCCESS;
			} else
				break;
		default:
			MSG_ERR("Invalid parameter : __msgsvc_vmsg_check_content_type() Failed(%d)", type);
			g_free(val);
			g_free(prefix);
			return MSG_ERR_INVALID_PARAMETER;
		}
		g_free(val);
		g_free(prefix);
		cursor = new_start;
	}

	MSG_ERR("Invalid vmsg");
	return MSG_ERR_INVALID_PARAMETER;
}

msg_error_t MsgVMessageDecodeSMS(const char *vmsg_stream, MSG_MESSAGE_INFO_S *pMsg)
{
	MSG_BEGIN();

	int ret, ver;

	char *vmsg = (char *)vmsg_stream;

	char *MMSsend, *MMSretrieve, *MMSnoti;

	MSG_ERR_RET_VM(NULL == vmsg_stream, MSG_ERR_INVALID_PARAMETER, "Invalid Parameter : vmsg");

	vmsg  = __msgsvc_vmsg_check_word(vmsg, "BEGIN:VMSG");
	MSG_ERR_RET_VM(NULL == vmsg, MSG_ERR_INVALID_PARAMETER, "Invalid parameter : The vmsg is invalid.");

	//check for mms()
	MMSsend = vmsg;
	MMSretrieve = vmsg;
	MMSnoti = vmsg;

	MMSsend = __msgsvc_vmsg_check_word(MMSsend, "X-MESSAGE-TYPE:MMS SEND");
	MSG_ERR_RET_VM(NULL != MMSsend, MSG_ERR_INVALID_MESSAGE, "Invalid parameter : The vmsg format is invalid.");

	MMSretrieve = __msgsvc_vmsg_check_word(MMSretrieve, "X-MESSAGE-TYPE:MMS RETRIEVE");
	MSG_ERR_RET_VM(NULL != MMSretrieve, MSG_ERR_INVALID_MESSAGE, "Invalid parameter : The vmsg format is invalid.");

	MMSnoti = __msgsvc_vmsg_check_word(MMSnoti, "X-MESSAGE-TYPE:MMS NOTIFICATION");
	MSG_ERR_RET_VM(NULL != MMSnoti, MSG_ERR_INVALID_MESSAGE, "Invalid parameter : The vmsg format is invalid.");

	//decode sms()
	__msgsvc_vmsg_initial();

	vmsg = __msgsvc_vmsg_check_word(vmsg, "VERSION:1.1");
	MSG_ERR_RET_VM(NULL == vmsg, MSG_ERR_INVALID_PARAMETER, "Invalid parameter : The vmsg format is invalid.");

	ver = MSGSVC_VMSG_VER_1_1;

	ret = __msgsvc_vmsg_get_msg(ver, vmsg, pMsg);
	if (MSG_SUCCESS!= ret) {
		return ret;
	}
	MSG_END();
	return MSG_SUCCESS;
}


