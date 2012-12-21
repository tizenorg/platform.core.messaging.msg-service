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

#include <ctype.h>
#include "MmsPluginDebug.h"
#include "MmsPluginCodecCommon.h"
#include "MmsPluginMIME.h"
#include "MmsPluginUtil.h"
#include "MmsPluginTextConvert.h"

#define	MMS_MAX_FIELD_VALUE_COUNT	74
#define	MMS_MAX_FIELD_TYPE_COUNT	21

typedef struct {
	const char *szText;
	UINT16 binary;
} MmsField;

/* Reference : MMS-209-Encapsulation & WAP-203-WSP-20000504.pdf Table 39 */
const MmsField gMmsField[MMS_MAX_FIELD_TYPE_COUNT][MMS_MAX_FIELD_VALUE_COUNT] =
{
	/* MMS Specific (MsgMmsMsg.h / MsgMmsUA.h) -----------------------*/

	/* MmsCodeFieldCode */
	{
		{"Bcc", 0x01},								//0
		{"Cc", 0x02},
		{"X-Mms-Content-Location", 0x03},
		{"Content-Type", 0x04},
		{"Date", 0x05},
		{"X-Mms-Delivery-Report", 0x06},
		{"X-Mms-Delivery-Time", 0x07},
		{"X-Mms-Expiry", 0x08},
		{"From", 0x09},
		{"X-Mms-Message-Class", 0x0A},
		{"Message-ID", 0x0B},						// 10
		{"X-Mms-Message-Type", 0x0C},
		{"X-Mms-MMS-Version", 0x0D},
		{"X-Mms-Message-Size", 0x0E},
		{"X-Mms-Priority", 0x0F},
		{"X-Mms-Read-Reply", 0x10},
		{"X-Mms-Report-Allowed", 0x11},
		{"X-Mms-Response-Status", 0x12},
		{"X-Mms-Retrieve-Status", 0x19},			/* Add by MMSENC v1.1 */
		{"X-Mms-Response-Text", 0x13},
		{"X-Mms-Retrieve-Text", 0x1A},				// 20	/* Add by MMSENC v1.1 */
		{"X-Mms-Sender-Visibility", 0x14},
		{"X-Mms-Status", 0x15},
		{"Subject", 0x16},
		{"To", 0x17},
		{"X-Mms-Transaction-ID", 0x18},

		/* Add by MMSENC v1.1 */
		{"X-Mms-Read-Status", 0x1B},
		{"X-Mms-Reply-Charging", 0x1C},
		{"X-Mms-Reply-Charging-Deadline", 0x1D},	// 30
		{"X-Mms-Reply-Charging-ID", 0x1E},
		{"X-Mms-Reply-Charging-Size", 0x1F},
		{"X-Mms-Previously-Sent-By", 0x20},
		{"X-Mms-Previously-Sent-Date", 0x21},
	},

	/* MmsCodeParameterCode ( By WSP Table 38. Wellknown parameter Assignments ) */
	{
		{"Charset", 0x01},		// v1.1 base
		{"Name", 0x05},			// v1.1 base. 0x17 at v1.4
		{"FileName", 0x06},		// v1.1 base. ox18 at v1.4
		{"Type", 0x09},			// v1.2 base
		{"Start", 0x0A},		// v1.2 base. 0x19 at v1.4
		{"Start-Info", 0x0B},	// v1.2 base. 0x1A at v1.4
		{"boundary", 0xFF},		//laconic_javaParamFix
		{"report-type", 0xFF},        // only used as parameter of Content-Type: multipart/report; report-type=delivery-status;
#ifdef FEATURE_JAVA_MMS
		{"Application-ID", 0xFF},
		{"Reply-To-Application-ID", 0xFF},
#endif
	},

	/* MmsCodeMsgBodyHeaderCode ( By WSP Table 39. Header Field Name Assignments ) */
	{
		{"Content-Transfer-Encoding", 0xFFFF},	// only text encoding, no binary number
		{"Content-Disposition", 0x2E},			// v1.1 base. 0x45 at v1.4
		{"Content-ID", 0x40},					// v1.3 base
		{"Content-Location", 0x0E},				// v1.3 base
		{"X-Oma-Drm-Separate-Delivery", 0xFF },	// DRM RO WAITING
	},

	/* MmsCodeMsgType */
	{
		{"m-send-req", 0x00},
		{"m-send-conf", 0x01},
		{"m-notification-ind", 0x02},
		{"m-notifyresp-ind", 0x03},
		{"m-retrieve-conf", 0x04},
		{"m-acknowledge-ind", 0x05},
		{"m-delivery-ind", 0x06},

		/* Add by MMSENC v1.1 */
		{"m-read-rec-ind", 0x07},
		{"m-read-orig-ind", 0x08},
		{"m-forward-req", 0x09},
		{"m-forward-conf", 0x0A}
	},

	/* MmsCodeDeliveryReport */
	{
		{ "Yes", 0x00 }, { "No", 0x01 }
	},

	/* MmsCodeTimeType */
	{
		{ "relative", 0x01 }, { "absolute", 0x00 }
	},

	/* MmsCodeMsgClass */
	{
		{"Personal", 0x00},
		{"Advertisement", 0x01},
		{"Informational", 0x02},
		{"Auto", 0x03}
	},

	/* MmsCodePriority */
	{
		{ "Low", 0x00 }, { "Normal", 0x01 }, { "High", 0x02 }
	},

	/* MmsCodeResponseStatus */
	{
		{"Ok", 0x00},
		{"Error-unspecified", 0x01},
		{"Error-service-denied", 0x02},
		{"Error-message-format-corrupt", 0x03},
		{"Error-sending-address-unresolved", 0x04},
		{"Error-message-not-found", 0x05},
		{"Error-network-problem", 0x06},
		{"Error-content-not-accepted", 0x07},
		{"Error-unsupported-message", 0x08},

		{"Error-transient-failure", 0x40},
		{"Error-transient-sending-address-unresolved", 0x41},
		{"Error-transient-message-not-found", 0x42},
		{"Error-transient-network-problem", 0x43},

		{"Error-transient-failure", 0x44}, //reserved for future
		{"Error-transient-failure", 0x45},
		{"Error-transient-failure", 0x46},
		{"Error-transient-failure", 0x47},
		{"Error-transient-failure", 0x48},
		{"Error-transient-failure", 0x49},
		{"Error-transient-failure", 0x4A},
		{"Error-transient-failure", 0x4B},
		{"Error-transient-failure", 0x4C},
		{"Error-transient-failure", 0x4D},
		{"Error-transient-failure", 0x4E},
		{"Error-transient-failure", 0x4F},
		{"Error-transient-failure", 0x50},
		{"Error-transient-failure", 0x51},
		{"Error-transient-failure", 0x52},
		{"Error-transient-failure", 0x53},
		{"Error-transient-failure", 0x54},
		{"Error-transient-failure", 0x55},
		{"Error-transient-failure", 0x56},
		{"Error-transient-failure", 0x57},
		{"Error-transient-failure", 0x58},
		{"Error-transient-failure", 0x59},
		{"Error-transient-failure", 0x5A},
		{"Error-transient-failure", 0x5B},
		{"Error-transient-failure", 0x5C},
		{"Error-transient-failure", 0x5D},
		{"Error-transient-failure", 0x5E},
		{"Error-transient-failure", 0x5F},

		{"Error-permanent-failure", 0x60},
		{"Error-permanent-service-denied", 0x61},
		{"Error-permanent-message-format-corrupt", 0x62},
		{"Error-permanent-sending-address-unresolved", 0x63},
		{"Error-permanent-message-not-found", 0x64},
		{"Error-permanent-content-not-accepted", 0x65},
		{"Error-permanent-reply-charging-limitations-not-met", 0x66},
		{"Error-permanent-reply-charging-request-not-accepted", 0x67},
		{"Error-permanent-reply-charging-forwarding-denied", 0x68},
		{"Error-permanent-reply-charging-not-supported", 0x69},

		{"Error-permanent-failure", 0x6A}, //reserved for future
		{"Error-permanent-failure", 0x6B},
		{"Error-permanent-failure", 0x6C},
		{"Error-permanent-failure", 0x6D},
		{"Error-permanent-failure", 0x6E},
		{"Error-permanent-failure", 0x6F},
		{"Error-permanent-failure", 0x70},
		{"Error-permanent-failure", 0x71},
		{"Error-permanent-failure", 0x72},
		{"Error-permanent-failure", 0x73},
		{"Error-permanent-failure", 0x74},
		{"Error-permanent-failure", 0x75},
		{"Error-permanent-failure", 0x76},
		{"Error-permanent-failure", 0x77},
		{"Error-permanent-failure", 0x78},
		{"Error-permanent-failure", 0x79},
		{"Error-permanent-failure", 0x7A},
		{"Error-permanent-failure", 0x7B},
		{"Error-permanent-failure", 0x7C},
		{"Error-permanent-failure", 0x7D},
		{"Error-permanent-failure", 0x7E},
		{"Error-permanent-failure", 0x7F}


	},

	/* MmsCodeRetrieveStatus */
	{
		{"Ok", 0x00},
		{"Error-transient-failure", 0x40},
		{"Error-transient-message-not-found", 0x41},
		{"Error-transient-network-problem", 0x42},

		{"Error-transient-failure", 0x43}, //reserved for future
		{"Error-transient-failure", 0x44},
		{"Error-transient-failure", 0x45},
		{"Error-transient-failure", 0x46},
		{"Error-transient-failure", 0x47},
		{"Error-transient-failure", 0x48},
		{"Error-transient-failure", 0x49},
		{"Error-transient-failure", 0x4A},
		{"Error-transient-failure", 0x4B},
		{"Error-transient-failure", 0x4C},
		{"Error-transient-failure", 0x4D},
		{"Error-transient-failure", 0x4E},
		{"Error-transient-failure", 0x4F},
		{"Error-transient-failure", 0x50},
		{"Error-transient-failure", 0x51},
		{"Error-transient-failure", 0x52},
		{"Error-transient-failure", 0x53},
		{"Error-transient-failure", 0x54},
		{"Error-transient-failure", 0x55},
		{"Error-transient-failure", 0x56},
		{"Error-transient-failure", 0x57},
		{"Error-transient-failure", 0x58},
		{"Error-transient-failure", 0x59},
		{"Error-transient-failure", 0x5A},
		{"Error-transient-failure", 0x5B},
		{"Error-transient-failure", 0x5C},
		{"Error-transient-failure", 0x5D},
		{"Error-transient-failure", 0x5E},
		{"Error-transient-failure", 0x5F},

		{"Error-permanent-failure", 0x60},
		{"Error-permanent-service-denied", 0x61},
		{"Error-permanent-message-not-found", 0x62},
		{"Error-permanent-content-unsupported", 0x63},

		{"Error-permanent-failure", 0x64}, //reserved for future
		{"Error-permanent-failure", 0x65},
		{"Error-permanent-failure", 0x66},
		{"Error-permanent-failure", 0x67},
		{"Error-permanent-failure", 0x68},
		{"Error-permanent-failure", 0x69},
		{"Error-permanent-failure", 0x6A},
		{"Error-permanent-failure", 0x6B},
		{"Error-permanent-failure", 0x6C},
		{"Error-permanent-failure", 0x6D},
		{"Error-permanent-failure", 0x6E},
		{"Error-permanent-failure", 0x6F},
		{"Error-permanent-failure", 0x70},
		{"Error-permanent-failure", 0x71},
		{"Error-permanent-failure", 0x72},
		{"Error-permanent-failure", 0x73},
		{"Error-permanent-failure", 0x74},
		{"Error-permanent-failure", 0x75},
		{"Error-permanent-failure", 0x76},
		{"Error-permanent-failure", 0x77},
		{"Error-permanent-failure", 0x78},
		{"Error-permanent-failure", 0x79},
		{"Error-permanent-failure", 0x7A},
		{"Error-permanent-failure", 0x7B},
		{"Error-permanent-failure", 0x7C},
		{"Error-permanent-failure", 0x7D},
		{"Error-permanent-failure", 0x7E},
		{"Error-permanent-failure", 0x7F}

	},

	/* MmsCodeReadReply */
	{
		{ "Yes", 0x00 }, { "No", 0x01 }
	},

	/* MmsCodeReportAllowed */
	{
		{ "Yes", 0x00 }, { "No", 0x01 }
	},

	/* MmsCodeSenderVisibility */
	{
		{ "Show", 0x01 }, { "Hide", 0x00 }
	},

	/* MmsCodeMsgStatus */
	{
		{"Expired", 0x00},
		{"Retrieved", 0x01},
		{"Rejected", 0x02},
		{"Deferred", 0x03},
		{"Unrecognised", 0x04},

		/* Add by MMSENC v1.1 */
		{"Indeterminate ", 0x05},
		{"Forwarded", 0x06},

		/* Add by MMSENC v1.2 */
		{"Unreachable", 0x07 }

	},

	/* MmsCodeReadStatus */
	{
		{"Read", 0x00}, {"Deleted", 0x01}
	},

	/* MmsCodeAddressType */
	{
		{"present", 0x00}, {"insert", 0x01}
	},


	/* MSG Specific (MsgMIMEExtern.h) -----------------------*/

	/* MmsCodeCharSet */
	{
		{"us-ascii", 0x03},
		{"UTF-16", 0x03F7},
		{"ISO-10646-UCS-2", 0x03E8},
		{"UTF-8", 0x6A},

		{"ISO-2022-KR", 0x25},
		{"KS_C_5601-1987", 0x24},
		{"EUC-KR", 0x26},
		{"ISO-2022-JP", 0x27},
		{"ISO-2022-JP-2", 0x28},

		{"ISO_8859-1", 0x04},
		{"ISO_8859-2", 0x05},
		{"ISO-8859-3", 0x06},
		{"ISO-8859-4", 0x07},
		{"ISO-8859-5", 0x08},
		{"ISO-8859-6", 0x09},
		{"ISO-8859-6-E", 0x51},
		{"ISO-8859-6-I", 0x52},
		{"ISO-8859-7", 0x0a},
		{"ISO-8859-8", 0x0b},
		{"ISO-8859-8-I", 0x85},
		{"ISO-8859-9", 0x0c},
		{"ISO-8859-10", 0x0d},
		{"ISO-8859-15", 0x6F},

		{"Shift_JIS", 0x11},
		{"EUC-JP", 0x13},
		{"GB2312", 0x07E9},
		{"BIG5", 0x0d},
		{"WIN1251", 0xFF},
		{"WINDOW-1251", 0xFF},
		{"WINDOWS-1251", 0xFF},
		{"KOI8-R", 0x0824},
		{"KOI8-U", 0x0828},
	},

	/* MmsCodeReplyCharging, */
	{
		{ "Requested", 0x00 },
		{ "Requested text only", 0x01 },
		{ "Accepted", 0x02 },
		{ "Accepted text only", 0x03 }
	},


	/* MSG Specific (MsgMIMEExtern.h) -----------------------*/

	/* Content-Type (http://www.wapforum.org/wina/wsp-content-type.htm) */
	/* this group(Content-Type) will be replaced by utyMime */
	{
		// {"Text/txt", 0x01},
		{"Text/html", 0x02},
		{"Text/plain", 0x03},
		{"Text/vnd.wap.wml", 0x08},
		{"Text/x-vCalendar", 0x06},
		{"Text/x-vCard", 0x07},

		{"Application/vnd.wap.multipart.*", 0x22},
		{"Application/vnd.wap.multipart.mixed", 0x23},
		{"Application/vnd.wap.multipart.related", 0x33},
		{"Application/vnd.wap.multipart.alternative", 0x26},

		{"application/vnd.oma.drm.message", 0x48},			// 10
		{"application/vnd.oma.drm.content", 0x49},
		{"application/vnd.oma.drm.rights+xml", 0x4A},
		{"application/vnd.oma.drm.rights+wbxml", 0x4B},

		{"application/smil", 0xFFFF},
		{"Multipart/mixed", 0x0c},
		{"Multipart/related", 0x0B},
		{"Multipart/alternative", 0x0F},

		{"multipart/report", 0xffff},
		{"Message/rfc822", 0xffff},

	//   T E X T
		{"Image/gif", 0x1D},			// 20
		{"Image/jpeg", 0x1E},
		{"Image/jpg", 0xFFFF},
		{"image/tiff", 0x1f},
		{"Image/png", 0x20},


		{"Image/vnd.wap.wbmp", 0x21},

		{"Image/wbmp", 0xFFFF},
		{"Image/pjpeg", 0xFFFF},

		{"Image/bmp", 0xFFFF},

	// A U D I O
		{"Audio/basic", 0xFFFF},
		{"Audio/mpeg", 0xFFFF},			// 30
		{"Audio/x-mpeg", 0xFFFF},
		{"Audio/mp3", 0xFFFF},
		{"audio/x-mp3", 0xFFFF},
		{"audio/mpeg3", 0xFFFF},
		{"audio/x-mpeg3", 0xFFFF},
		{"audio/mpg", 0xFFFF},
		{"audio/x-mpg", 0xFFFF},
		{"audio/x-mpegaudio", 0xFFFF},
		{"Audio/aac", 0xFFFF},			// 39
		{"Audio/g72", 0xFFFF},
		{"Audio/amr", 0xFFFF},
		{"audio/x-amr", 0xFFFF},
		{"audio/x-mmf", 0xFFFF},
		{"application/vnd.smaf",  0xffff},
		{"application/x-smaf", 0xFFFF},
		{"audio/mmf", 0xFFFF},

		{"text/x-iMelody", 0xffff},
		{"audio/x-iMelody", 0xffff},
		{"audio/iMelody", 0xffff},		// 49
		{"audio/mid",0xffff},
		{"audio/midi", 0xffff},
		{"audio/x-midi", 0xffff},
		{"audio/sp-midi", 0xffff},
		{"audio/wave", 0xffff},
		{"audio/3gpp", 0xffff},
		{"audio/vnd.rn-realaudio", 0xffff},
		{"audio/x-pn-realaudio", 0xffff},
		{"audio/mp4",  0xffff},

	// V I D E O
		{"video/mpeg4", 0xFFFF},
		{"video/mp4", 0xffff},
		{"video/x-mp4", 0xFFFF},
		{"video/x-vp-mp4", 0xffff},
		{"Video/h263", 0xFFFF},

		{"video/3gpp", 0xffff},
		{"video/3gp", 0xffff},
		{"Video/avi", 0xFFFF},

		{"video/sdp", 0xffff},			// 70
		{"application/vnd.rn-realmedia", 0xffff},
		{"video/vnd.rn-realvideo", 0xffff},

		{"application/octet-stream", 0xFFFF }
	},

	/* MmsCodeMsgDisposition : Wsp Header (By Wsp 8.4.2.53) */
	{
		{"form-data", 0x00},
		{"attachment", 0x01},
		{"inline", 0x02}
	},

	/* Content-transfer-encoding : HTTP Header(Binary Value is not assigned) */
	{
		{"7bit", 0x00},
		{"8bit", 0x00},
		{"binary", 0x00},
		{"base64", 0x00},
		{"quoted-printable", 0x00}
	}
};

const char *MmsGetTextValue(MmsCode i, int j)
{
	if (i == MmsCodeContentType) {
		//apply UtyMime
		return MimeGetMimeStringFromMimeInt(j);
	}

	return (const char *)gMmsField[i][j].szText;
}

const char *MmsGetTextValuebyField(int field, int value)
{
	const char *szValue = NULL;

	switch (field) {
	case MMS_CODE_MSGTYPE:
		szValue = MmsGetTextValue(MmsCodeMsgType, value);
		break;

	case MMS_CODE_MSGCLASS:
		szValue = MmsGetTextValue(MmsCodeMsgClass, value);
		break;

	case MMS_CODE_PRIORITY:
		szValue = MmsGetTextValue(MmsCodePriority, value);
		break;

	case MMS_CODE_SENDERVISIBILLITY:
		szValue = MmsGetTextValue(MmsCodeSenderVisibility, value);
		break;

	case MMS_CODE_DELIVERYREPORT:
		szValue = MmsGetTextValue(MmsCodeDeliveryReport, value);
		break;

	case MMS_CODE_READREPLY:
		szValue = MmsGetTextValue(MmsCodeReadReply, value);
		break;

	case MMS_CODE_MSGSTATUS:
		szValue = MmsGetTextValue(MmsCodeMsgStatus, value);
		break;

	case MMS_CODE_REPORTALLOWED:
		szValue = MmsGetTextValue(MmsCodeReportAllowed, value);
		break;

	case MMS_CODE_RESPONSESTATUS:
		szValue = MmsGetTextValue(MmsCodeResponseStatus, value);
		break;

	/* Add by MMSENC v1.1 */
	case MMS_CODE_READSTATUS:
		szValue = MmsGetTextValue(MmsCodeReadStatus, value);
		break;

	default:
		szValue = NULL;
		break;
	}

	return szValue;
}

UINT16 MmsGetBinaryValue(MmsCode i, int j)
{
	if (i == MmsCodeContentType) {
		return MimeGetBinaryValueFromMimeInt((MimeType)j);
	}

	return gMmsField[i][j].binary;
}

// getting mime type (int) by binary type
int MmsGetBinaryType(MmsCode i, UINT16 value)
{
	MSG_BEGIN();

	if (i == MmsCodeContentType) {
		//apply UtyMime
		return MimeGetMimeIntFromBi(value);
	}

	for (int j = 0; j < MMS_MAX_FIELD_VALUE_COUNT; j++) {
		if (gMmsField[i][j].binary == value) {
			return j;
		}
	}

	MSG_END();

	return MIME_UNKNOWN;
}

int MmsGetTextType(MmsCode i, char *pValue)
{
	int j = 0;

	if (i == MmsCodeContentType) {
		/*apply UtyMime */
		return MimeGetMimeIntFromMimeString( pValue );
	}

	for (j = 0; j < MMS_MAX_FIELD_VALUE_COUNT; j++) {
		if (gMmsField[i][j].szText != NULL) {
			if (strcasecmp( gMmsField[i][j].szText, pValue ) == 0) {
				return j;
			}
		}
	}

	return -1;
}

const char *MmsGetTextByCode(MmsCode i, UINT16 code)
{
	for (int j = 0; j < MMS_MAX_FIELD_VALUE_COUNT; j++) {
		if (gMmsField[i][j].binary == code) {
			return gMmsField[i][j].szText;
		}
	}
	return NULL;
}



/* ==================================================================
 *	Decode/Encode inline base64 string
 *
 * base64 : 3*8bit -> 4*6bit & convert the value into A~Z, a~z, 0~9, +, or /
 * pad(=) is needed when the end of the string is < 24bit.
 *
 *     Value Encoding  Value Encoding  Value Encoding  Value Encoding
 *         0 A            17 R            34 i            51 z
 *         1 B            18 S            35 j            52 '0'
 *         2 C            19 T            36 k            53 1
 *         3 D            20 U            37 l            54 2
 *         4 E            21 V            38 m            55 3
 *         5 F            22 W            39 n            56 4
 *         6 G            23 X            40 o            57 5
 *         7 H            24 Y            41 p            58 6
 *         8 I            25 Z            42 q            59 7
 *         9 J            26 a            43 r            60 8
 *        10 K            27 b            44 s            61 9
 *        11 L            28 c            45 t            62 +
 *        12 M            29 d            46 u            63 /
 *        13 N            30 e            47 v
 *        14 O            31 f            48 w         (pad) =
 *        15 P            32 g            49 x
 *        16 Q            33 h            50 y
 *
 * (1) the final quantum = 24 bits : no "=" padding,
 * (2) the final quantum = 8 bits : two "=" + two characters
 * (3) the final quantum = 16 bits : one "=" + three characters
 * ================================================================== */

void *MsgDecodeBase64(unsigned char *pSrc, unsigned long srcLen, unsigned long *len)
{
	char c;
	void *ret = NULL;
	char *d = NULL;
	short e = 0;

	ret = malloc((size_t)(*len = 4 + ((srcLen * 3) / 4)));
	d = (char *)ret;

	if (ret == NULL) {
		MSG_DEBUG("_MsgDecodeBase64: ret malloc Fail \n");
		return NULL;
	}

	memset(ret, 0, (size_t)*len);
	*len = 0;

	while (srcLen-- > 0) {
		c = *pSrc++;

		/* Convert base64 character into original value */

		if (isupper(c))
			c -= 'A';
		else if (islower(c))
			c -= 'a' - 26;
		else if (isdigit(c))
			c -= '0' - 52;
		else if (c == '+')
			c = 62;
		else if (c == '/')
			c = 63;
		else if (c == '=') {
			switch (e++) {
			case 2:
				if (*pSrc != '=') {
					*len = d - (char *)ret;
					return ret;
				}
				break;
			case 3:
				e = 0;
				break;
			default:
				*len = d - (char *)ret;
				return ret;
			}
			continue;
		} else
			continue;					// Actually, never get here

		/* Pad 4*6bit character into 3*8bit character */

		switch (e++) {
		case 0:
			*d = c << 2;			// byte 1: high 6 bits
			break;

		case 1:
			*d++ |= c >> 4;			// byte 1: low 2 bits
			*d = c << 4;			// byte 2: high 4 bits
			break;

		case 2:
			*d++ |= c >> 2;			// byte 2: low 4 bits
			*d = c << 6;			// byte 3: high 2 bits
			break;

		case 3:
			*d++ |= c;				// byte 3: low 6 bits
			e = 0;					// Calculate next unit.
			break;

		default:
			MSG_DEBUG("_MsgDecodeBase64: Unknown paremeter\n");
			break;
		}
	}

	*len = d - (char *)ret;			// Calculate the size of decoded string.

	return ret;
}



/* ==========================================
 *	Decode/Encode inline base64 string
 *
 * quoted-printable := ([*(ptext / SPACE / TAB) ptext] ["="] CRLF)
 *       ; Maximum line length of 76 characters excluding CRLF
 *
 * ptext := octet /<any ASCII character except "=", SPACE, or TAB>
 *       ; characters not listed as "mail-safe" in Appendix B
 *       ; are also not recommended.
 *
 * octet := "=" 2(DIGIT / "A" / "B" / "C" / "D" / "E" / "F")
 *       ; octet must be used for characters > 127, =, SPACE, or TAB.
 *
 * ==========================================*/


unsigned char *MsgDecodeQuotePrintable(unsigned char *pSrc, unsigned long srcLen, unsigned long *len)
{
	unsigned char *ret = NULL;
	unsigned char *d = NULL;
	unsigned char *s = NULL;					/* last non-blank */
	unsigned char c;
	unsigned char e;

	d = s = ret = (unsigned char *)malloc((size_t)srcLen + 1);
	if (ret == NULL) {
		MSG_DEBUG("_MsgDecodeQuotePrintable: ret malloc Fail \n");
		return NULL;
	}

	*len = 0;
	pSrc[srcLen] = '\0';

	while ((c = *pSrc++)!= '\0') {
		switch (c) {
		case '=':							/* octet characters (> 127, =, SPACE, or TAB) */
			switch (c = *pSrc++) {
			case '\0':					/* end of string -> postpone to while */
				break;

			case '\015':				/* CRLF */
				if (*pSrc == '\012')
					pSrc++;
				break;

			default:					/* two hexes */
				if (!isxdigit(c)) {
					*d = '\0';
					*len = d - ret;
					return ret;
				}

				if (isdigit(c))
					e = c - '0';
				else
					e = c - (isupper(c) ? 'A' - 10 : 'a' - 10);

				c = *pSrc++;
				if (!isxdigit(c)) {
					*d = '\0';
					*len = d - ret;
					return ret;
				}

				if (isdigit(c))
					c -= '0';
				else
					c -= (isupper(c) ? 'A' - 10 : 'a' - 10);

				*d++ = c + (e << 4);
				s = d;
				break;
			}
			break;

		case ' ':							/* skip the blank */
			*d++ = c;
			break;

		case '\015':						/* Line Feedback : to last non-blank character */
			d = s;
			break;

		default:
			*d++ = c;						/* ASCII character */
			s = d;
			break;
		}
	}

	*d = '\0';
	*len = d - ret;

	return ret;
}


/* ========================================
 * Decode/Encode inline base64 string
 * Inline base64 has no "\r\n" in it,
 * and has charset and encoding sign in it
 * ======================================== */
bool MsgEncode2Base64(void *pSrc, unsigned long srcLen, unsigned long *len, unsigned char *ret)
{
	unsigned char *d = NULL;
	unsigned char *s = (unsigned char *)pSrc;
	char *v = (char *)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned long i = ((srcLen + 2) / 3) * 4;

	i += 2 * ((i / 60) + 1);
	*len = i;

	if (ret == NULL) {
		MSG_DEBUG("_MsgEncode2Base64: ret Memory Alloc Fail \n");
		return false;
	}
	memset(ret, 0, i);

	d = ret;

	/* Convert 3*8bit into 4*6bit */
	for (i = 0; srcLen > 0; s += 3) {
		*d++ = v[s[0] >> 2];															// byte 1: high 6 bits of character-1
		*d++ = v[((s[0] << 4) + (--srcLen ? (s[1] >> 4) : 0)) & 0x3f];					// byte 2: low 2 bits of character-1 and high 4 bits of character-2
		*d++ = srcLen ? v[((s[1] << 2) + (--srcLen ? (s[2] >> 6) : 0)) & 0x3f] : '=';	// byte 3: low 4 bits of charcter-2 and high 2 bits of character-3
		*d++ = srcLen ? v[s[2] & 0x3f] : '=';											// byte 4: low 6 bits of character-3

		if (srcLen)
			srcLen--;
	}

	*d = '\0';

	if (((unsigned long)(d - ret)) != *len) {
		*len = d - ret;
		MSG_DEBUG("base64 encoding length = %d \n", *len);
	}

	return true;
}


char *MsgDecodeText(char *pOri)
{
	MSG_BEGIN();

	int size = 0;
	int cnt = 0;
	char *pSrc = NULL;
	char *pTemp = NULL;
	char *pRe = NULL;
	char *pStrEnd = NULL;
	char *pDecStart = NULL;
	char *pDecEnd = NULL;
	char *pDecQ = NULL;
	char *pDecQ2 = NULL;
	bool bEncoding = false;
	int	nCharset = MSG_CHARSET_UTF8;
	int	nTemp = 0;
	char *pReturnStr = NULL;
	char *pConvertedStr = NULL;

	char szTempBuf[MSG_LOCAL_TEMP_BUF_SIZE] = {0};

	// copy original string
	if (strlen(pOri) >= MSG_LOCAL_TEMP_BUF_SIZE) {
		pSrc = MsgStrCopy( pOri );
	} else {
		memset(szTempBuf, 0, MSG_LOCAL_TEMP_BUF_SIZE);
		strcpy(szTempBuf, pOri);

		pSrc = szTempBuf;
	}

	// it can be one or more encoding methods in a line
	while (1) {
		cnt++;

		bEncoding = false;

		/*
		  (ex) "=?euc-kr?B?Y2NqMjEyMw==?="

		  pDecStart: charset			(=?euc-kr?B?Y2NqMjEyMw==?=)
		  pDecQ	: Encoding type		(B?Y2NqMjEyMw==?=)
		  pDecQ2	: Encoded text		(Y2NqMjEyMw==?=)
		  pDecEnd	: Encoded of text	(?=)
		 */
		if (pSrc == NULL)
			goto __CATCH;

		if (((pDecStart = strstr(pSrc, MSG_STR_DEC_START)) != NULL) 	//"=?"
		     && ((pDecQ = strchr(pDecStart + 2, MSG_CH_QUESTION)) != NULL)	// '?'
		     && ((pDecQ2 = strchr(pDecQ + 1, MSG_CH_QUESTION))!= NULL)		// '?'
		     && ((pDecEnd = strstr(pDecQ2 + 1, MSG_STR_DEC_END))!= NULL)) {	//"=?"
			bEncoding = true;

			/* fixme: charset problem
			 * pDecStart ~ pDecQ : charSet & MSG_CHARSET_USC2 ~ MSG_CHARSET_UTF8 & LATIN
			 */

			*pDecQ = '\0';
			nCharset = _MsgGetCode(MSG_CHARSET, pDecStart + 2);
			*pDecQ = MSG_CH_QUESTION;
		}

		// End of encoding
		if (!bEncoding)
			goto __RETURN;

		// find end of string
		pStrEnd = pSrc + strlen(pSrc);

		// Decoding
		if ((*(pDecQ2 - 1) == MSG_CH_BASE64_UPPER) ||
			(*(pDecQ2 - 1) == MSG_CH_BASE64_LOWER) ||
			(*(pDecQ + 1) == MSG_CH_BASE64_UPPER) ||
			(*(pDecQ + 1) == MSG_CH_BASE64_LOWER)) {
			pTemp = (char *)MsgDecodeBase64((UCHAR *)(pDecQ2 + 1), (ULONG)(pDecEnd - pDecQ2 - 1), (ULONG *)&size);

			if (pTemp != NULL) {
				pTemp[size] = MSG_CH_NULL;

				if(pRe) {
					free(pRe);
					pRe = NULL;
				}

				pRe = (char *)malloc((pDecStart-pSrc) + size + (pStrEnd - (pDecEnd + 2)) + 1);
				if (pRe == NULL) {
					MSG_DEBUG("_MsgDecodeText: pRemalloc fail \n");
					free(pTemp);
					pTemp = NULL;

					goto __RETURN;
				}

				memcpy(pRe, pSrc, pDecStart - pSrc);
				memcpy(&pRe[pDecStart-pSrc], pTemp, size);
				memcpy(&pRe[(pDecStart - pSrc) + size], pDecEnd + 2, pStrEnd - (pDecEnd + 2));
				pRe[(pDecStart - pSrc) + size + (pStrEnd - (pDecEnd + 2))] = MSG_CH_NULL;

				free(pTemp);
				pTemp = NULL;

				if (pSrc != NULL && pSrc != szTempBuf) {
					free(pSrc);
					pSrc = NULL;
				}
			}
		} else if ((*(pDecQ2-1) == MSG_CH_QPRINT_UPPER) ||
				(*(pDecQ2-1) == MSG_CH_QPRINT_LOWER) ||
				(*(pDecQ+1) == MSG_CH_QPRINT_UPPER) ||
				(*(pDecQ+1) == MSG_CH_QPRINT_LOWER)) {

			pTemp = (char *)MsgDecodeQuotePrintable((UCHAR *)( pDecQ2 + 1 ), (ULONG)(pDecEnd - pDecQ2 - 1), (ULONG *)&size);

			if (pTemp != NULL) {
				int i;
				pTemp[size] = MSG_CH_NULL;

				for (i = 0; i < size; i++) {
					if (pTemp[i] == MSG_CH_UNDERLINE) {
						pTemp[i] = MSG_CH_SP;	                      // change '_' to ' '
					}
				}

				if(pRe) {
					free(pRe);
					pRe = NULL;
				}

				pRe = (char *)malloc((pDecStart - pSrc) + size + (pStrEnd - (pDecEnd + 2)) + 1);
				if (pRe == NULL) {
					MSG_DEBUG("_MsgDecodeText: pRemalloc fail \n");
					free(pTemp);
					pTemp = NULL;

					goto __RETURN;
				}

				memcpy(pRe, pSrc, pDecStart - pSrc);
				memcpy(&pRe[pDecStart - pSrc], pTemp, size);
				memcpy(&pRe[(pDecStart - pSrc) + size], pDecEnd + 2, pStrEnd - (pDecEnd + 2));
				pRe[(pDecStart - pSrc) + size + (pStrEnd - (pDecEnd + 2))] = MSG_CH_NULL;

				if (pTemp) {
					free(pTemp);
					pTemp = NULL;
				}

				if (pSrc != NULL && pSrc != szTempBuf) {
					free(pSrc);
					pSrc = NULL;
				}
			}
		} else {
			goto __RETURN;
		}
	}



__RETURN:

	pTemp = strdup(pSrc);
	nTemp = strlen(pSrc);

	{//temp brace;
		const char *pToCharSet = "UTF-8";

		UINT16 charset_code =  MmsGetBinaryValue(MmsCodeCharSet, nCharset);

		const char *pFromCharSet = MmsPluginTextConvertGetCharSet(charset_code);

		if (pFromCharSet != NULL && strcmp(pFromCharSet, pToCharSet) != 0) {//Not UTF-8
			char *pDest = NULL;
			int destLen = 0;

			if (MmsPluginTextConvert(pToCharSet, pFromCharSet, pTemp, nTemp, &pDest, &destLen) == false) {
				MSG_DEBUG("MmsPluginTextConvert Fail");
			}

			if (pDest) {
				free(pTemp);
				pTemp = strdup(pDest);
				nTemp = destLen;
				free(pDest);
			}
		}

	}

	pReturnStr = (char *)malloc(nTemp + 1);

	if (pReturnStr == NULL) {
		goto __CATCH;
	}

	memset(pReturnStr, 0, nTemp + 1);

	if (pTemp) {
		memcpy(pReturnStr, pTemp, nTemp);
		free(pTemp);
		pTemp = NULL;
	}

	if (pConvertedStr) {
		free(pConvertedStr);
		pConvertedStr = NULL;
	}

	if(pRe) {
		free(pRe);
		pRe = NULL;
	}

	if (pSrc != NULL && pSrc != szTempBuf) {
		free(pSrc);
		pSrc = NULL;
	}

	return pReturnStr;

__CATCH:

	if (pConvertedStr) {
		free(pConvertedStr);
		pConvertedStr = NULL;
	}

	if(pRe) {
		free(pRe);
		pRe = NULL;
	}

	if (pSrc != NULL && pSrc != szTempBuf) {
		free(pSrc);
		pSrc = NULL;
	}

	if (pTemp) {
		free(pTemp);
		pTemp = NULL;
	}

	return NULL;
}


static char gszDebugStringBuf[50];

static char *MmsDebugPrintUnknownValue(int value)
{
	printf(gszDebugStringBuf, "unknown value(%d)", value);
	return gszDebugStringBuf;
}


const char *MmsDebugGetMimeType(MimeType mimeType)
{
	switch (mimeType) {
	case MIME_APPLICATION_XML:
		return "MIME_APPLICATION_XML";
	case MIME_APPLICATION_WML_XML:
		return "MIME_APPLICATION_WML_XML";
	case MIME_APPLICATION_XHTML_XML:
		return "MIME_APPLICATION_XHTML_XML";
	case MIME_APPLICATION_JAVA_VM:
		return "MIME_APPLICATION_JAVA_VM";
	case MIME_APPLICATION_SMIL:
		return "MIME_APPLICATION_SMIL";
	case MIME_APPLICATION_JAVA_ARCHIVE:
		return "MIME_APPLICATION_JAVA_ARCHIVE";
	case MIME_APPLICATION_JAVA:
		return "MIME_APPLICATION_JAVA";
	case MIME_APPLICATION_OCTET_STREAM:
		return "MIME_APPLICATION_OCTET_STREAM";
	case MIME_APPLICATION_STUDIOM:
		return "MIME_APPLICATION_STUDIOM";
	case MIME_APPLICATION_FUNMEDIA:
		return "MIME_APPLICATION_FUNMEDIA";
	case MIME_APPLICATION_MSWORD:
		return "MIME_APPLICATION_MSWORD";
	case MIME_APPLICATION_PDF:
		return "MIME_APPLICATION_PDF";
	case MIME_APPLICATION_ASTERIC:
		return "MIME_APPLICATION_ASTERIC";
	case MIME_APPLICATION_VND_WAP_XHTMLXML:
		return "MIME_APPLICATION_VND_WAP_XHTMLXML";
	case MIME_APPLICATION_VND_WAP_WMLC:
		return "MIME_APPLICATION_VND_WAP_WMLC";
	case MIME_APPLICATION_VND_WAP_WMLSCRIPTC:
		return "MIME_APPLICATION_VND_WAP_WMLSCRIPTC";
	case MIME_APPLICATION_VND_WAP_WTA_EVENTC:
		return "MIME_APPLICATION_VND_WAP_WTA_EVENTC";
	case MIME_APPLICATION_VND_WAP_UAPROF:
		return "MIME_APPLICATION_VND_WAP_UAPROF";
	case MIME_APPLICATION_VND_WAP_SIC:
		return "MIME_APPLICATION_VND_WAP_SIC";
	case MIME_APPLICATION_VND_WAP_SLC:
		return "MIME_APPLICATION_VND_WAP_SLC";
	case MIME_APPLICATION_VND_WAP_COC:
		return "MIME_APPLICATION_VND_WAP_COC";
	case MIME_APPLICATION_VND_WAP_SIA:
		return "MIME_APPLICATION_VND_WAP_SIA";
	case MIME_APPLICATION_VND_WAP_CONNECTIVITY_WBXML:
		return "MIME_APPLICATION_VND_WAP_CONNECTIVITY_WBXML";
	case MIME_APPLICATION_VND_WAP_MULTIPART_FORM_DATA:
		return "MIME_APPLICATION_VND_WAP_MULTIPART_FORM_DATA";
	case MIME_APPLICATION_VND_WAP_MULTIPART_BYTERANGES:
		return "MIME_APPLICATION_VND_WAP_MULTIPART_BYTERANGES";
	case MIME_APPLICATION_VND_WAP_MULTIPART_MIXED:
		return "MIME_APPLICATION_VND_WAP_MULTIPART_MIXED";
	case MIME_APPLICATION_VND_WAP_MULTIPART_RELATED:
		return "MIME_APPLICATION_VND_WAP_MULTIPART_RELATED";
	case MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE:
		return "MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE";
	case MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC:
		return "MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC";
	case MIME_APPLICATION_VND_OMA_DD_XML:
		return "MIME_APPLICATION_VND_OMA_DD_XML";
	case MIME_APPLICATION_VND_OMA_DRM_MESSAGE:
		return "MIME_APPLICATION_VND_OMA_DRM_MESSAGE";
	case MIME_APPLICATION_VND_OMA_DRM_CONTENT:
		return "MIME_APPLICATION_VND_OMA_DRM_CONTENT";
	case MIME_APPLICATION_VND_OMA_DRM_RIGHTS_XML:
		return "MIME_APPLICATION_VND_OMA_DRM_RIGHTS_XML";
	case MIME_APPLICATION_VND_OMA_DRM_RIGHTS_WBXML:
		return "MIME_APPLICATION_VND_OMA_DRM_RIGHTS_WBXML";
	case MIME_APPLICATION_VND_SMAF:
		return "MIME_APPLICATION_VND_SMAF";
	case MIME_APPLICATION_VND_RN_REALMEDIA:
		return "MIME_APPLICATION_VND_RN_REALMEDIA";
	case MIME_APPLICATION_VND_SUN_J2ME_JAVA_ARCHIVE:
		return "MIME_APPLICATION_VND_SUN_J2ME_JAVA_ARCHIVE";
	case MIME_APPLICATION_VND_SAMSUNG_THEME:
		return "MIME_APPLICATION_VND_SAMSUNG_THEME";
	case MIME_APPLICATION_VND_EXCEL:
		return "MIME_APPLICATION_VND_EXCEL";
	case MIME_APPLICATION_X_HDMLC:
		return "MIME_APPLICATION_X_HDMLC";
	case MIME_APPLICATION_X_X968_USERCERT:
		return "MIME_APPLICATION_X_X968_USERCERT";
	case MIME_APPLICATION_X_WWW_FORM_URLENCODED:
		return "MIME_APPLICATION_X_WWW_FORM_URLENCODED";
	case MIME_APPLICATION_X_SMAF:
		return "MIME_APPLICATION_X_SMAF";
	case MIME_APPLICATION_X_FLASH:
		return "MIME_APPLICATION_X_FLASH";
	case MIME_APPLICATION_X_EXCEL:
		return "MIME_APPLICATION_X_EXCEL";
	case MIME_APPLICATION_X_POWERPOINT:
		return "MIME_APPLICATION_X_POWERPOINT";

	case MIME_AUDIO_BASIC:
		return "MIME_AUDIO_BASIC";
	case MIME_AUDIO_MPEG:
		return "MIME_AUDIO_MPEG";
	case MIME_AUDIO_MP3:
		return "MIME_AUDIO_MP3";
	case MIME_AUDIO_MPG3:
		return "MIME_AUDIO_MPG3";
	case MIME_AUDIO_MPEG3:
		return "MIME_AUDIO_MPEG3";
	case MIME_AUDIO_MPG:
		return "MIME_AUDIO_MPG";
	case MIME_AUDIO_AAC:
		return "MIME_AUDIO_AAC";
	case MIME_AUDIO_G72:
		return "MIME_AUDIO_G72";
	case MIME_AUDIO_AMR:
		return "MIME_AUDIO_AMR";
	case MIME_AUDIO_AMR_WB:
		return "MIME_AUDIO_AMR_WB";
	case MIME_AUDIO_MMF:
		return "MIME_AUDIO_MMF";
	case MIME_AUDIO_SMAF:
		return "MIME_AUDIO_SMAF";
	case MIME_AUDIO_IMELODY:
		return "MIME_AUDIO_IMELODY";
	case MIME_AUDIO_MELODY:
		return "MIME_AUDIO_MELODY";
	case MIME_AUDIO_MID:
		return "MIME_AUDIO_MID";
	case MIME_AUDIO_MIDI:
		return "MIME_AUDIO_MIDI";
	case MIME_AUDIO_X_MID:
		return "MIME_AUDIO_X_MID";
	case MIME_AUDIO_SP_MIDI:
		return "MIME_AUDIO_SP_MIDI";
	case MIME_AUDIO_WAVE:
		return "MIME_AUDIO_WAVE";
	case MIME_AUDIO_3GPP:
		return "MIME_AUDIO_3GPP";
	case MIME_AUDIO_MP4:
		return "MIME_AUDIO_MP4";
	case MIME_AUDIO_MP4A_LATM:
		return "MIME_AUDIO_MP4A_LATM";
	case MIME_AUDIO_VND_RN_REALAUDIO:
		return "MIME_AUDIO_VND_RN_REALAUDIO";
	case MIME_AUDIO_X_MPEG:
		return "MIME_AUDIO_X_MPEG";
	case MIME_AUDIO_X_MP3:
		return "MIME_AUDIO_X_MP3";
	case MIME_AUDIO_X_MPEG3:
		return "MIME_AUDIO_X_MPEG3";
	case MIME_AUDIO_X_MPG:
		return "MIME_AUDIO_X_MPG";
	case MIME_AUDIO_X_AMR:
		return "MIME_AUDIO_X_AMR";
	case MIME_AUDIO_X_MMF:
		return "MIME_AUDIO_X_MMF";
	case MIME_AUDIO_X_SMAF:
		return "MIME_AUDIO_X_SMAF";
	case MIME_AUDIO_X_IMELODY:
		return "MIME_AUDIO_X_IMELODY";
	case MIME_AUDIO_X_MIDI:
		return "MIME_AUDIO_X_MIDI";
	case MIME_AUDIO_X_MPEGAUDIO:
		return "MIME_AUDIO_X_MPEGAUDIO";
	case MIME_AUDIO_X_PN_REALAUDIO:
		return "MIME_AUDIO_X_PN_REALAUDIO";
	case MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO:
		return "MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO";
	case MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO_LIVE:
		return "MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO_LIVE";
	case MIME_AUDIO_X_WAV:
		return "MIME_AUDIO_X_WAV";

	case MIME_IMAGE_GIF:
		return "MIME_IMAGE_GIF";
	case MIME_IMAGE_JPEG:
		return "MIME_IMAGE_JPEG";
	case MIME_IMAGE_JPG:
		return "MIME_IMAGE_JPG";
	case MIME_IMAGE_TIFF:
		return "MIME_IMAGE_TIFF";
	case MIME_IMAGE_TIF:
		return "MIME_IMAGE_TIF";
	case MIME_IMAGE_PNG:
		return "MIME_IMAGE_PNG";
	case MIME_IMAGE_WBMP:
		return "MIME_IMAGE_WBMP";
	case MIME_IMAGE_PJPEG:
		return "MIME_IMAGE_PJPEG";
	case MIME_IMAGE_BMP:
		return "MIME_IMAGE_BMP";
	case MIME_IMAGE_SVG:
		return "MIME_IMAGE_SVG";
	case MIME_IMAGE_SVG1:
		return "MIME_IMAGE_SVG1";
	case MIME_IMAGE_VND_WAP_WBMP:
		return "MIME_IMAGE_VND_WAP_WBMP";

	case MIME_IMAGE_X_BMP:
		return "MIME_IMAGE_X_BMP";

	case MIME_MESSAGE_RFC822:
		return "MIME_MESSAGE_RFC822";

	case MIME_MULTIPART_MIXED:
		return "MIME_MULTIPART_MIXED";
	case MIME_MULTIPART_RELATED:
		return "MIME_MULTIPART_RELATED";
	case MIME_MULTIPART_ALTERNATIVE:
		return "MIME_MULTIPART_ALTERNATIVE";
	case MIME_MULTIPART_FORM_DATA:
		return "MIME_MULTIPART_FORM_DATA";
	case MIME_MULTIPART_BYTERANGE:
		return "MIME_MULTIPART_BYTERANGE";
	case MIME_MULTIPART_REPORT:
		return "MIME_MULTIPART_REPORT";

	case MIME_TEXT_TXT:
		return "MIME_TEXT_TXT";
	case MIME_TEXT_HTML:
		return "MIME_TEXT_HTML";
	case MIME_TEXT_PLAIN:
		return "MIME_TEXT_PLAIN";
	case MIME_TEXT_CSS:
		return "MIME_TEXT_CSS";
	case MIME_TEXT_XML:
		return "MIME_TEXT_XML";
	case MIME_TEXT_IMELODY:
		return "MIME_TEXT_IMELODY";
	case MIME_TEXT_VND_WAP_WMLSCRIPT:
		return "MIME_TEXT_VND_WAP_WMLSCRIPT";
	case MIME_TEXT_VND_WAP_WML:
		return "MIME_TEXT_VND_WAP_WML";
	case MIME_TEXT_VND_WAP_WTA_EVENT:
		return "MIME_TEXT_VND_WAP_WTA_EVENT";
	case MIME_TEXT_VND_WAP_CONNECTIVITY_XML:
		return "MIME_TEXT_VND_WAP_CONNECTIVITY_XML";
	case MIME_TEXT_VND_WAP_SI:
		return "MIME_TEXT_VND_WAP_SI";
	case MIME_TEXT_VND_WAP_SL:
		return "MIME_TEXT_VND_WAP_SL";
	case MIME_TEXT_VND_WAP_CO:
		return "MIME_TEXT_VND_WAP_CO";
	case MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR:
		return "MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR";
	case MIME_TEXT_X_HDML:
		return "MIME_TEXT_X_HDML";
	case MIME_TEXT_X_VCALENDAR:
		return "MIME_TEXT_X_VCALENDAR";
	case MIME_TEXT_X_VCARD:
		return "MIME_TEXT_X_VCARD";
	case MIME_TEXT_X_IMELODY:
		return "MIME_TEXT_X_IMELODY";

	case MIME_VIDEO_MPEG4:
		return "MIME_VIDEO_MPEG4";
	case MIME_VIDEO_MP4:
		return "MIME_VIDEO_MP4";
	case MIME_VIDEO_H263:
		return "MIME_VIDEO_H263";
	case MIME_VIDEO_3GPP:
		return "MIME_VIDEO_3GPP";
	case MIME_VIDEO_3GP:
		return "MIME_VIDEO_3GP";
	case MIME_VIDEO_AVI:
		return "MIME_VIDEO_AVI";
	case MIME_VIDEO_SDP:
		return "MIME_VIDEO_SDP";
	case MIME_VIDEO_VND_RN_REALVIDEO:
		return "MIME_VIDEO_VND_RN_REALVIDEO";
	case MIME_VIDEO_X_MP4:
		return "MIME_VIDEO_X_MP4";
	case MIME_VIDEO_X_PV_MP4:
		return "MIME_VIDEO_X_PV_MP4";
	case MIME_VIDEO_X_PN_REALVIDEO:
		return "MIME_VIDEO_X_PN_REALVIDEO";
	case MIME_VIDEO_X_PN_MULTIRATE_REALVIDEO:
		return "MIME_VIDEO_X_PN_MULTIRATE_REALVIDEO";
	default:
		return MmsDebugPrintUnknownValue(mimeType);
	}
}


/* MsgMmsMsg.h */
const char *MmsDebugGetMmsReport(MmsReport report)
{
	switch (report) {
	case MMS_REPORT_ERROR:
		return "MMS_REPORT_ERROR";
	case MMS_REPORT_YES:
		return "MMS_REPORT_YES";
	case MMS_REPORT_NO:
		return "MMS_REPORT_NO";
	}

	return MmsDebugPrintUnknownValue(report);
}


const char *MmsDebugGetMmsReportAllowed(MmsReportAllowed reportAllowed)
{
	switch (reportAllowed) {
	case MMS_REPORTALLOWED_ERROR:
		return "MMS_REPORTALLOWED_ERROR";
	case MMS_REPORTALLOWED_YES:
		return "MMS_REPORTALLOWED_YES";
	case MMS_REPORTALLOWED_NO:
		return "MMS_REPORTALLOWED_NO";
	}

	return MmsDebugPrintUnknownValue(reportAllowed);
}


const char *MmsDebugGetMmsReadStatus(msg_read_report_status_t readStatus)
{
	_MSG_READ_REPORT_STATUS_E readReport = (_MSG_READ_REPORT_STATUS_E)readStatus;

	switch (readReport) {
	case MSG_READ_REPORT_NONE:
		return "MMS_READSTATUS_NONE";
	case MSG_READ_REPORT_IS_READ:
		return "MMS_IS_READ";
	case MSG_READ_REPORT_IS_DELETED:
		return "MMS_IS_DELETED";
	}

	return MmsDebugPrintUnknownValue(readStatus);
}

const char *MmsDebugGetMsgType(MmsMsgType msgType)
{
	switch (msgType) {
	case MMS_MSGTYPE_ERROR:
		return "error";
	case MMS_MSGTYPE_SEND_REQ:
		return "send.req";
	case MMS_MSGTYPE_SEND_CONF:
		return "send.conf";
	case MMS_MSGTYPE_NOTIFICATION_IND:
		return "notification.ind";
	case MMS_MSGTYPE_NOTIFYRESP_IND:
		return "notifyResp.ind";
	case MMS_MSGTYPE_RETRIEVE_CONF:
		return "retrieve conf";
	case MMS_MSGTYPE_ACKNOWLEDGE_IND:
		return "acknowledge ind";
	case MMS_MSGTYPE_DELIVERY_IND:
		return "delivery ind";
	case MMS_MSGTYPE_READREC_IND:
		return "read rec ind";
	case MMS_MSGTYPE_READORG_IND:
		return "read org ind";
	case MMS_MSGTYPE_FORWARD_REQ:
		return "forward req";
	case MMS_MSGTYPE_FORWARD_CONF:
		return "forward conf";
	case MMS_MSGTYPE_READ_REPLY:
		return "read reply";
	default:
		return MmsDebugPrintUnknownValue(msgType);
	}
}

const char *MmsDebugGetResponseStatus(MmsResponseStatus responseStatus)
{
	switch (responseStatus) {
	case MMS_RESPSTATUS_ERROR:
		return "error";
	case MMS_RESPSTATUS_OK:
		return "ok";
	case MMS_RESPSTAUTS_ERROR_UNSPECIFIED:
		return "unspecified";
	case MMS_RESPSTAUTS_ERROR_SERVICEDENIED:
		return "service denied";
	case MMS_RESPSTAUTS_ERROR_MESSAGEFORMATCORRUPT:
		return "message format corrupt";
	case MMS_RESPSTAUTS_ERROR_SENDINGADDRESSUNRESOLVED:
		return "sending address unresolved";
	case MMS_RESPSTAUTS_ERROR_MESSAGENOTFOUND:
		return "message not found";
	case MMS_RESPSTAUTS_ERROR_NETWORKPROBLEM:
		return "network problem";
	case MMS_RESPSTAUTS_ERROR_CONTENTNOTACCEPTED:
		return "content not accepted";
	case MMS_RESPSTAUTS_ERROR_UNSUPPORTEDMESSAGE:
		return "unsupported message";
	case MMS_RESPSTAUTS_ERROR_TRANSIENT_FAILURE:
		return "transient failure";
	case MMS_RESPSTAUTS_ERROR_TRANSIENT_SENDING_ADDRESS_UNRESOLVED:
		return "transient sending address unresolved";
	case MMS_RESPSTAUTS_ERROR_TRANSIENT_MESSAGE_NOT_FOUND:
		return "transient message not found";
	case MMS_RESPSTAUTS_ERROR_TRANSIENT_NETWORK_PROBLEM:
		return "transient network problem";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_FAILURE:
		return "permanent failure";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_SERVICE_DENIED:
		return "permanent service denied";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_MESSAGE_FORMAT_CORRUPT:
		return "permanent message format corrupt";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_SENDING_ADDRESS_UNRESOLVED:
		return "permanent sending address unresolved";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_MESSAGE_NOT_FOUND:
		return "permanent message not found";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_CONTENT_NOT_ACCEPTED:
		return "permanent content not accepted";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_REPLY_CHARGING_LIMITATIONS_NOT_MET:
		return "permanent reply charging limitations not met";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_REPLY_CHARGING_REQUEST_NOT_ACCEPTED:
		return "permanent reply charging request not accepted";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_REPLY_CHARGING_FORWARDING_DENIED:
		return "permanent reply charging forwarding denied";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_REPLY_CHARGING_NOT_SUPPORTED:
		return "permanent reply charging not supported";
	}

	return MmsDebugPrintUnknownValue(responseStatus);
}


const char *MmsDebugGetRetrieveStatus(MmsRetrieveStatus retrieveStatus)
{
	switch (retrieveStatus) {
	case MMS_RETRSTATUS_ERROR:
		return "error";
	case MMS_RETRSTATUS_OK:
		return "ok";
	case MMS_RETRSTATUS_TRANSIENT_FAILURE:
		return "transient failure";
	case MMS_RETRSTATUS_TRANSIENT_MESSAGE_NOT_FOUND:
		return "transient message not found";
	case MMS_RETRSTATUS_TRANSIENT_NETWORK_PROBLEM:
		return "transient network problem";
	case MMS_RETRSTATUS_PERMANENT_FAILURE:
		return "permanent failure";
	case MMS_RETRSTATUS_PERMANENT_SERVICE_DENIED:
		return "permanent service denied";
	case MMS_RETRSTATUS_PERMANENT_MESSAGE_NOT_FOUND:
		return "permanent message not found";
	case MMS_RETRSTATUS_PERMANENT_CONTENT_UNSUPPORT:
		return "permanent content unsupport";
	}

	return MmsDebugPrintUnknownValue(retrieveStatus);
}


const char *MmsDebugGetMsgStatus(msg_delivery_report_status_t msgStatus)
{
	switch (msgStatus) {
	case MSG_DELIVERY_REPORT_ERROR:
		return "error";
	case MSG_DELIVERY_REPORT_EXPIRED:
		return "expired";
	case MSG_DELIVERY_REPORT_SUCCESS:
		return "retrieved";
	case MSG_DELIVERY_REPORT_REJECTED:
		return "rejected";
	case MSG_DELIVERY_REPORT_DEFERRED:
		return "deferred";
	case MSG_DELIVERY_REPORT_UNRECOGNISED:
		return "unrecognised";
	case MSG_DELIVERY_REPORT_INDETERMINATE:
		return "indeterminate";
	case MSG_DELIVERY_REPORT_FORWARDED:
		return "forwarded";
	case MSG_DELIVERY_REPORT_UNREACHABLE:
		return "unreachable";
	}

	return MmsDebugPrintUnknownValue(msgStatus);
}


const char *MmsDebugGetMsgClass(MmsMsgClass msgClass)
{
	switch (msgClass) {
	case MMS_MSGCLASS_ERROR:
		return "error";
	case MMS_MSGCLASS_PERSONAL:
		return "personal";
	case MMS_MSGCLASS_ADVERTISEMENT:
		return "advertisement";
	case MMS_MSGCLASS_INFORMATIONAL:
		return "information";
	case MMS_MSGCLASS_AUTO:
		return "auto";
	}

	return MmsDebugPrintUnknownValue(msgClass);
}


const char *MmsDebugGetDataType(MmsDataType dataType)
{
	switch (dataType) {
	case MMS_DATATYPE_NONE:
		return "MMS_DATATYPE_NONE";
	case MMS_DATATYPE_READ:
		return "MMS_DATATYPE_READ";
	case MMS_DATATYPE_SENT:
		return "MMS_DATATYPE_SENT";
	case MMS_DATATYPE_NOTIFY:
		return "MMS_DATATYPE_NOTIFY";
	case MMS_DATATYPE_UNSENT:
		return "MMS_DATATYPE_UNSENT";
	case MMS_DATATYPE_DRAFT:
		return "MMS_DATATYPE_DRAFT";
	case MMS_DATATYPE_SENDING:
		return "MMS_DATATYPE_SENDING";
	case MMS_DATATYPE_DRM_RO_WAITING:
		return "MMS_DATATYPE_DRM_RO_WAITING";
	case MMS_DATATYPE_RETRIEVING:
		return "MMS_DATATYPE_RETRIEVING";
	case MMS_DATATYPE_UNRETV:
		return "MMS_DATATYPE_UNRETV";
	default:
		return MmsDebugPrintUnknownValue(dataType);
	}
}

bool MmsInitMsgType(MsgType *pMsgType)
{
	MSG_DEBUG("MmsInitMsgType");
	pMsgType->offset = 0;
	pMsgType->size = 0;
	pMsgType->contentSize = 0;
	pMsgType->disposition = -1;
	pMsgType->encoding = -1;
	pMsgType->type = MIME_UNKNOWN;
	pMsgType->section = -1;

	pMsgType->szOrgFilePath[0] = '\0';
	pMsgType->szContentID[0] = '\0';
	pMsgType->szContentLocation[0] = '\0';

	pMsgType->szContentRepPos[0] = '\0';
	pMsgType->szContentRepSize[0] = '\0';
	pMsgType->szContentRepIndex[0] = '\0';

	MmsInitMsgContentParam(&pMsgType->param);
#ifdef __SUPPORT_DRM__
	MmsInitMsgDRMInfo(&pMsgType->drmInfo);
#endif

	return true;
}

bool MmsInitMsgBody(MsgBody *pMsgBody)
{
	MSG_DEBUG("MmsInitMsgBody");
	pMsgBody->offset = 0;
	pMsgBody->size = 0;
	pMsgBody->body.pText = NULL;

	MmsInitMsgType(&pMsgBody->presentationType);
	pMsgBody->pPresentationBody = NULL;

	memset(pMsgBody->szOrgFilePath, 0, MSG_FILEPATH_LEN_MAX);

	return true;
}

bool MmsInitMsgContentParam(MsgContentParam *pMsgContentParam)
{
	MSG_DEBUG("MmsInitMsgContentParam");
	pMsgContentParam->charset = MSG_CHARSET_UNKNOWN;
	pMsgContentParam->type = MIME_UNKNOWN;
	pMsgContentParam->szBoundary[0] = '\0';
	pMsgContentParam->szFileName[0] = '\0';
	pMsgContentParam->szName[0] = '\0';
	pMsgContentParam->szStart[0] = '\0';
	pMsgContentParam->szStartInfo[0] = '\0';
	pMsgContentParam->pPresentation = NULL;
	pMsgContentParam->reportType = MSG_PARAM_REPORT_TYPE_UNKNOWN; // only used as parameter of Content-Type: multipart/report; report-type
#ifdef FEATURE_JAVA_MMS
	pMsgContentParam->szApplicationID = NULL;
	pMsgContentParam->szReplyToApplicationID = NULL;
#endif
	return true;
}

bool MmsInitMsgAttrib(MmsAttrib *pAttrib)
{
	MSG_DEBUG("MmsInitMsgAttrib");

	pAttrib->bAskDeliveryReport = false;

	pAttrib->bAskReadReply = false;
	pAttrib->bRead = false;
	pAttrib->bReportAllowed = false;
	pAttrib->readReportAllowedType = MMS_RECEIVE_READ_REPORT_ALLOWED;
	pAttrib->readReportSendStatus = MMS_RECEIVE_READ_REPORT_NO_SEND;
	pAttrib->bReadReportSent = false;

	pAttrib->bHideAddress = false;
	pAttrib->date = 0;
	pAttrib->bUseDeliveryCustomTime = false;
	pAttrib->deliveryTime.type = MMS_TIMETYPE_RELATIVE;
	pAttrib->deliveryTime.time = 0;
	pAttrib->bUseExpiryCustomTime = false;
	pAttrib->expiryTime.type = MMS_TIMETYPE_RELATIVE;
	pAttrib->expiryTime.time = 0;
	memset(&pAttrib->expiryTime, 0, sizeof(MmsTimeStruct));
	pAttrib->msgClass = MMS_MSGCLASS_PERSONAL;
	pAttrib->msgStatus = MSG_DELIVERY_REPORT_NONE;
	pAttrib->priority = MMS_PRIORITY_NORMAL;
	pAttrib->responseStatus = MMS_RESPSTATUS_ERROR;
	pAttrib->retrieveStatus = MMS_RETRSTATUS_ERROR;
	pAttrib->contentType = MIME_UNKNOWN;
	pAttrib->msgSize = 0;
	pAttrib->bLeaveCopy = true;

	pAttrib->specialMsgType = MMS_SPECIAL_MSG_TYPE_NONE;
#ifdef __SUPPORT_DRM__
	pAttrib->drmType = MSG_DRM_TYPE_NONE;
	pAttrib->roWaitingTimerMax = 0;
	pAttrib->pszDrmData = NULL;
#endif
	pAttrib->version = MMS_VERSION;

	memset(pAttrib->szFrom, 0, MSG_LOCALE_ADDR_LEN + 10);
	memset(pAttrib->szResponseText, 0, MMS_LOCALE_RESP_TEXT_LEN + 1);
	memset(pAttrib->szRetrieveText, 0, MMS_LOCALE_RESP_TEXT_LEN + 1);
	memset(pAttrib->szSubject, 0, MSG_LOCALE_SUBJ_LEN + 1);
	pAttrib->szTo = NULL;
	pAttrib->szCc = NULL;
	pAttrib->szBcc = NULL;

	pAttrib->pMultiStatus = NULL;

	pAttrib->replyCharge.chargeType = MMS_REPLY_NONE;
	memset(&pAttrib->replyCharge.deadLine , 0, sizeof(MmsTimeStruct));
	pAttrib->replyCharge.chargeSize = 0;
	memset(pAttrib->replyCharge.szChargeID, 0, MMS_MSG_ID_LEN);

	return true;
}

#ifdef __SUPPORT_DRM__
bool MmsInitMsgDRMInfo(MsgDRMInfo *pMsgDrmInfo)
{
	pMsgDrmInfo->contentType = MIME_UNKNOWN;
	pMsgDrmInfo->drmType = MSG_DRM_TYPE_NONE;

	pMsgDrmInfo->szContentName = NULL;
	pMsgDrmInfo->szContentURI = NULL;
	pMsgDrmInfo->szContentDescription = NULL;
	pMsgDrmInfo->szContentVendor = NULL;
	pMsgDrmInfo->szRightIssuer = NULL;
	pMsgDrmInfo->szDrm2FullPath = NULL;
	pMsgDrmInfo->roWaitingTimerMax = 0;
	pMsgDrmInfo->bFwdLock = false;
	pMsgDrmInfo->bNoScreen = false;
	pMsgDrmInfo->bNoRingTone = false;
	pMsgDrmInfo->pszContentType = NULL;

	return true;
}
#endif
