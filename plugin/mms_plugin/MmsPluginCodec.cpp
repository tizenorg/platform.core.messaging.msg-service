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

#include "MmsPluginCodec.h"
#include "MsgMmsTypes.h"
#include "MmsPluginMessage.h"
#include "MmsPluginMIME.h"
#include "MsgDebug.h"

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

/* global variables */
static char	gszMmsVersion[5] = {0, };

char *_MmsGetTextValue(MmsCode i, int j)
{
	if (i == MmsCodeContentType) {
		//apply UtyMime
		return MimeGetMimeStringFromMimeInt(j);
	}

	return (char *)gMmsField[i][j].szText;
}

char *_MmsGetTextValuebyField(int field, int value)
{
	char *szValue = NULL;

	switch (field) {
	case MMS_CODE_MSGTYPE:
		szValue = _MmsGetTextValue(MmsCodeMsgType, value);
		break;

	case MMS_CODE_MSGCLASS:
		szValue = _MmsGetTextValue(MmsCodeMsgClass, value);
		break;

	case MMS_CODE_PRIORITY:
		szValue = _MmsGetTextValue(MmsCodePriority, value);
		break;

	case MMS_CODE_SENDERVISIBILLITY:
		szValue = _MmsGetTextValue(MmsCodeSenderVisibility, value);
		break;

	case MMS_CODE_DELIVERYREPORT:
		szValue = _MmsGetTextValue(MmsCodeDeliveryReport, value);
		break;

	case MMS_CODE_READREPLY:
		szValue = _MmsGetTextValue(MmsCodeReadReply, value);
		break;

	case MMS_CODE_MSGSTATUS:
		szValue = _MmsGetTextValue(MmsCodeMsgStatus, value);
		break;

	case MMS_CODE_REPORTALLOWED:
		szValue = _MmsGetTextValue(MmsCodeReportAllowed, value);
		break;

	case MMS_CODE_RESPONSESTATUS:
		szValue = _MmsGetTextValue(MmsCodeResponseStatus, value);
		break;

	/* Add by MMSENC v1.1 */
	case MMS_CODE_READSTATUS:
		szValue = _MmsGetTextValue(MmsCodeReadStatus, value);
		break;

	default:
		szValue = NULL;
		break;
	}

	return szValue;
}

UINT16 _MmsGetBinaryValue(MmsCode i, int j)
{
	if (i == MmsCodeContentType) {
		return MimeGetBinaryValueFromMimeInt((MimeType)j);
	}

	return gMmsField[i][j].binary;
}

// getting mime type (int) by binary type
int _MmsGetBinaryType(MmsCode i, UINT16 value)
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

int _MmsGetTextType(MmsCode i, char *pValue)
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

UINT8 _MmsGetVersion(MmsMsg *pMsg)
{
	if (pMsg == NULL) {
		return MMS_VERSION;
	}

	return pMsg->mmsAttrib.version;
}

bool _MmsSetVersion(int majorVer, int minorVer)
{
	snprintf(gszMmsVersion, sizeof(gszMmsVersion), "%d.%d", majorVer, minorVer);
	return true;
}

