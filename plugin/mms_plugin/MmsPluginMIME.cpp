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

#include <string>
#include <stdlib.h>
#include "MmsPluginMIME.h"
#include "MmsPluginDebug.h"
#include "MmsPluginSmil.h"

#ifndef	NULL
#define	NULL	0
#endif


/* Header field */
static const char *szMsgField[MSG_FIELD_NUM] =
{
	"Return-Path",
	"Message-ID",
	"From" ,
	"To" ,
	"Cc" ,
	"Subject",
	"Date",
	"Mime-Version",
	"Content-Type",
	"Content-Transfer-Encoding",
	"Content-Disposition", //If start param is given in multipart/related, this field will be ignored
	"Content-ID", //for start part of multipart/related body
	"Content-Location",
	"Content-Name",
	"Content-Description",
	"Content-Vendor",
	"Rights-Issuer",
	"Return-Receipt-To",			/* Delivery confirm */
	"Disposition-Notification-To",	/* Read confirm */
	"Content-Rep-Pos",
	"Content-Rep-Size",
	"Content-Rep-Index"
};

/*  MIME header field parameter */
static const char *szMsgParam[MSG_PARAM_NUM] =
{
	"charset",
	"name",
	"filename",
	"type",		//Only if content-type is multipart/related,
	"start", 	//Only if content-type is multipart/related
	"start-info", 	//Only if content-type is multipart/related
	"boundary",
	"report-type", // only used as parameter of Content-Type: multipart/report; report-type=delivery-status;
#ifdef FEATURE_JAVA_MMS
	"Application-ID",				//laconic_javaParamFix
	"Reply-To-Application-ID",		//laconic_javaParamFix
#endif
};

/* Content-Transfer-Encoding header value */
static const char *szMsgEncoding[MSG_ENCODING_NUM] =
{
	"7bit",
	"8bit",
	"binary",
	"base64",
	"quoted-printable"
};

/* Content-Disposition header value */
static const char *szMsgDisposition[MSG_DISPOSITION_NUM] =
{
	"form-data",
	"attachment",
	"inline"
};

static const char *szMsgAddrType[MSG_ADDR_TYPE_NUM] =
{
	"/TYPE=PLMN",
	"",
	"/TYPE=IPV4",
	"/TYPE=IPV6",
	""
};

/* character-set value */
static const char *szMsgCharset [MSG_CHARSET_NUM] =
{
	"us-ascii",
	"utf-16",
	"usc-2",
	"utf-8",
	/* MIME character-set value */

	"iso-2022-kr",
	"ks_c_5601-1987",
	"euc_kr",
	"iso-2022-jp",
	"iso-2022-jp-2",
	"iso-8859-1",
	"iso-8859-2",
	"iso-8859-3",
	"iso-8859-4",
	"iso-8859-5",
	"iso-8859-6",
	"iso-8859-6-e",
	"iso-8859-6-i",
	"iso-8859-7",
	"iso-8859-8",
	"iso-8859-8-i",
	"iso-8859-9",
	"iso-8859-10",
	"iso-8859-15",
	"Shift_jis",
	"euc-jp",
	"gb2312",
	"big5",
	"win1251",
	"window-1251",
	"windows-1251",
	"koi8-r",
	"koi8-u"
};


/**************************************************     MIME definition     ***************************************************/
static const MimeTable mimeTable[] = {
	// 0
	{"*/*",													"",					false,		MIME_ASTERISK,									MIME_ASTERISK,							MIME_APPLICATION_NONE,			MIME_MAINTYPE_ETC,				UNDEFINED_BINARY	},

    // 1
	{"application/xml",									"",					false,		MIME_APPLICATION_XML,							MIME_APPLICATION_XML,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x27	},
	{"application/wml+xml",								"",					false,		MIME_APPLICATION_WML_XML,						MIME_APPLICATION_WML_XML,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x3c	},
	{"application/xhtml+xml",								"xhtml",			false,		MIME_APPLICATION_XHTML_XML,						MIME_APPLICATION_XHTML_XML,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x3b	},
	{"application/java-vm",								"",					false,		MIME_APPLICATION_JAVA_VM,						MIME_APPLICATION_JAVA_VM,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x11	},
	{"application/smil",									"smil",				true,		MIME_APPLICATION_SMIL,							MIME_APPLICATION_SMIL,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},
	{"application/java-archive",							"jar",				true,		MIME_APPLICATION_JAVA_ARCHIVE,					MIME_APPLICATION_JAVA_ARCHIVE,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_ETC,			UNDEFINED_BINARY	},
	{"application/java",									"jar",				true,		MIME_APPLICATION_JAVA,							MIME_APPLICATION_JAVA,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_ETC,			UNDEFINED_BINARY	},
	{"application/octet-stream",							"",					false,		MIME_APPLICATION_OCTET_STREAM,					MIME_APPLICATION_OCTET_STREAM,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_ETC,			UNDEFINED_BINARY	},
	{"application/studiom",								"smp",				true,		MIME_APPLICATION_STUDIOM,						MIME_APPLICATION_STUDIOM,				MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"application/funMedia",								"smp",				true,		MIME_APPLICATION_FUNMEDIA,						MIME_APPLICATION_FUNMEDIA,				MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"application/msword",									"doc",				true,		MIME_APPLICATION_MSWORD,						MIME_APPLICATION_MSWORD,				MIME_APPLICATION_PICSELVIEWER,			MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},
	{"application/pdf",									"pdf",				true,		MIME_APPLICATION_PDF,							MIME_APPLICATION_PDF,					MIME_APPLICATION_PICSELVIEWER,			MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},
	{"application/sdp",									"sdp",				true,		MIME_APPLICATION_SDP,							MIME_APPLICATION_SDP,					MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,	UNDEFINED_BINARY	},
	{"application/ram",									"ram",				true,		MIME_APPLICATION_RAM,							MIME_APPLICATION_RAM,					MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"application/*",										"",					false,		MIME_APPLICATION_ASTERIC,						MIME_APPLICATION_ASTERIC,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x10	},

	//16
	{"application/vnd.wap.xhtml+xml",						"",					false,		MIME_APPLICATION_VND_WAP_XHTMLXML,				MIME_APPLICATION_VND_WAP_XHTMLXML,		MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x45	},
	{"application/vnd.wap.wmlc",							"",					false,		MIME_APPLICATION_VND_WAP_WMLC,					MIME_APPLICATION_VND_WAP_WMLC,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x14	},
	{"application/vnd.wap.wmlscriptc",						"",					false,		MIME_APPLICATION_VND_WAP_WMLSCRIPTC,			MIME_APPLICATION_VND_WAP_WMLSCRIPTC,	MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x15	},
	{"application/vnd.wap.wta-eventc",						"",					false,		MIME_APPLICATION_VND_WAP_WTA_EVENTC,			MIME_APPLICATION_VND_WAP_WTA_EVENTC,	MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x16	},
	{"application/vnd.wap.uaprof",							"",					false,		MIME_APPLICATION_VND_WAP_UAPROF,				MIME_APPLICATION_VND_WAP_UAPROF,		MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x17	},
	{"application/vnd.wap.sic",							"",					false,		MIME_APPLICATION_VND_WAP_SIC,					MIME_APPLICATION_VND_WAP_SIC,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x2e	},
	{"application/vnd.wap.slc",							"",					false,		MIME_APPLICATION_VND_WAP_SLC,					MIME_APPLICATION_VND_WAP_SLC,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x30	},
	{"application/vnd.wap.coc",							"",					false,		MIME_APPLICATION_VND_WAP_COC,					MIME_APPLICATION_VND_WAP_COC,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x32	},
	{"application/vnd.wap.sia",							"",					false,		MIME_APPLICATION_VND_WAP_SIA,					MIME_APPLICATION_VND_WAP_SIA,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x34	},
	{"application/vnd.wap,connectivity-wbxml",				"",					false,		MIME_APPLICATION_VND_WAP_CONNECTIVITY_WBXML,	MIME_APPLICATION_VND_WAP_CONNECTIVITY_WBXML,		MIME_APPLICATION_NONE,		MIME_MAINTYPE_TEXT,			UNDEFINED_BINARY	},
	{"application/vnd.wap.multipart.form-data",			"",					false,		MIME_APPLICATION_VND_WAP_MULTIPART_FORM_DATA,	MIME_MULTIPART_FORM_DATA,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x24	},
	{"application/vnd.wap.multipart.byteranges",			"",					false,		MIME_APPLICATION_VND_WAP_MULTIPART_BYTERANGES,	MIME_MULTIPART_BYTERANGE,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x25	},
	{"application/vnd.wap.multipart.mixed",				"",					false,		MIME_APPLICATION_VND_WAP_MULTIPART_MIXED,		MIME_MULTIPART_MIXED,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x23	},
	{"application/vnd.wap.multipart.related",				"",					false,		MIME_APPLICATION_VND_WAP_MULTIPART_RELATED,		MIME_MULTIPART_RELATED,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x33	},
	{"application/vnd.wap.multipart.alternative",			"",					false,		MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE,	MIME_MULTIPART_ALTERNATIVE,					MIME_APPLICATION_NONE,				MIME_MAINTYPE_APPLICATION,	0x26	},
	{"application/vnd.wap.multipart.*",					"",					false,		MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC,		MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC,		MIME_APPLICATION_NONE,			MIME_MAINTYPE_APPLICATION,	0x22	},
	{"application/vnd.wap.wbxml",							"",					false,		MIME_APPLICATION_VND_WAP_WBXML,					MIME_APPLICATION_VND_WAP_WBXML,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x29	},
	{"application/vnd.oma.dd+xml",							"dd",				true,		MIME_APPLICATION_VND_OMA_DD_XML,				MIME_APPLICATION_VND_OMA_DD_XML,		MIME_APPLICATION_NONE,					MIME_MAINTYPE_ETC,			0x47	},
	{"application/vnd.oma.drm.message",					"dm",				true,		MIME_APPLICATION_VND_OMA_DRM_MESSAGE,			MIME_APPLICATION_VND_OMA_DRM_MESSAGE,		MIME_APPLICATION_NONE,				MIME_MAINTYPE_ETC,			0x48	},
	{"application/vnd.oma.drm.content",					"dcf",				true,		MIME_APPLICATION_VND_OMA_DRM_CONTENT,			MIME_APPLICATION_VND_OMA_DRM_CONTENT,		MIME_APPLICATION_NONE,				MIME_MAINTYPE_ETC,			0x49	},
	{"application/vnd.oma.drm.rights+xml",					"ro",				true,		MIME_APPLICATION_VND_OMA_DRM_RIGHTS_XML,		MIME_APPLICATION_VND_OMA_DRM_RIGHTS_XML,		MIME_APPLICATION_NONE,			MIME_MAINTYPE_ETC,			0x4a	},
	{"application/vnd.oma.drm.rights+wbxml",				"ro",				true,		MIME_APPLICATION_VND_OMA_DRM_RIGHTS_WBXML,		MIME_APPLICATION_VND_OMA_DRM_RIGHTS_WBXML,		MIME_APPLICATION_NONE,			MIME_MAINTYPE_ETC,			0x4b	},
	{"application/vnd.oma.drm.ro+xml",				"oro",				true,		MIME_APPLICATION_VND_OMA_DRM_RO_XML,		MIME_APPLICATION_VND_OMA_DRM_RO_XML,		MIME_APPLICATION_NONE,			MIME_MAINTYPE_ETC,			0x4b	},
	{"application/vnd.oma.drm.dcf",				"odf",				true,		MIME_APPLICATION_VND_OMA_DRM_DCF,		MIME_APPLICATION_VND_OMA_DRM_DCF,		MIME_APPLICATION_NONE,			MIME_MAINTYPE_ETC,			0x4b	},
	{"application/vnd.oma.drm.roap-pdu+xml",				"xml",				true,		MIME_APPLICATION_VND_OMA_ROAPPDU_XML,		MIME_APPLICATION_VND_OMA_ROAPPDU_XML,		MIME_APPLICATION_NONE,			MIME_MAINTYPE_ETC,			0x4b	},
	{"application/vnd.oma.drm.roap-trigger+xml",				"xml",				true,		MIME_APPLICATION_VND_OMA_ROAPTRIGGER_XML,		MIME_APPLICATION_VND_OMA_ROAPTRIGGER_XML,		MIME_APPLICATION_NONE,			MIME_MAINTYPE_ETC,			0x4b	},
	{"application/vnd.smaf",								"mmf",				true,		MIME_APPLICATION_VND_SMAF,						MIME_APPLICATION_X_SMAF,				MIME_APPLICATION_SOUNDPLAYER,			MIME_MAINTYPE_AUDIO,		UNDEFINED_BINARY	},
	{"application/vnd.rn-realmedia",						"rm",				true,		MIME_APPLICATION_VND_RN_REALMEDIA,				MIME_APPLICATION_VND_RN_REALMEDIA,		MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"application/vnd.sun.j2me.java-archive",				"jar",				true,		MIME_APPLICATION_VND_SUN_J2ME_JAVA_ARCHIVE,		MIME_APPLICATION_JAVA_ARCHIVE,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_ETC,			UNDEFINED_BINARY	},
	{"application/vnd.samsung.theme",						"thm",				true,		MIME_APPLICATION_VND_SAMSUNG_THEME,				MIME_APPLICATION_VND_SAMSUNG_THEME,		MIME_APPLICATION_THEMEVIEWER,			MIME_MAINTYPE_THEME,		UNDEFINED_BINARY	},
	{"application/vnd.ms-excel",							"xls",				true,		MIME_APPLICATION_VND_EXCEL,						MIME_APPLICATION_X_EXCEL,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},
	{"application/vnd.ms-powerpoint",						"ppt",				true,		MIME_APPLICATION_VND_POWERPOINT,				MIME_APPLICATION_VND_POWERPOINT,		MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},
	{"applcation/vnd.ms-word",								"doc",				true,		MIME_APPLICATION_VND_MSWORD,					MIME_APPLICATION_MSWORD,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},

	//49
	{"application/x-hdmlc",								"",					false,		MIME_APPLICATION_X_HDMLC,						MIME_APPLICATION_X_HDMLC,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x13	},
	{"application/x-x968-user-cert",						"",					false,		MIME_APPLICATION_X_X968_USERCERT,				MIME_APPLICATION_X_X968_USERCERT,		MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x2c	},
	{"application/x-www-form-urlencoded",					"",					false,		MIME_APPLICATION_X_WWW_FORM_URLENCODED,			MIME_APPLICATION_X_WWW_FORM_URLENCODED,		MIME_APPLICATION_NONE,				MIME_MAINTYPE_APPLICATION,	0x12	},
	{"application/x-smaf",									"mmf",				true,		MIME_APPLICATION_X_SMAF,						MIME_APPLICATION_X_SMAF,				MIME_APPLICATION_SOUNDPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"application/x-shockwave-flash",						"swf",				true,		MIME_APPLICATION_X_FLASH,						MIME_APPLICATION_X_FLASH,				MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,		UNDEFINED_BINARY	},
	{"application/x-msexcel",								"xls",				true,		MIME_APPLICATION_X_EXCEL,						MIME_APPLICATION_X_EXCEL,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},
	{"application/x-mspowerpoint",							"ppt",				true,		MIME_APPLICATION_X_POWERPOINT,					MIME_APPLICATION_X_POWERPOINT,			MIME_APPLICATION_PICSELVIEWER,			MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},


	//56
	{"audio/basic",										"snd"/*,au"*/,		false,		MIME_AUDIO_BASIC,								MIME_AUDIO_BASIC,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/mpeg",											"mp3",				true,		MIME_AUDIO_MPEG,								MIME_AUDIO_MP3,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/mp3",											"mp3",				true,		MIME_AUDIO_MP3,									MIME_AUDIO_MP3,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/mpg3",											"mp3",				true,		MIME_AUDIO_MPG3,								MIME_AUDIO_MP3,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/mpeg3",										"mp3",				true,		MIME_AUDIO_MPEG3,								MIME_AUDIO_MP3,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/mpg",											"mp3",				true,		MIME_AUDIO_MPG,									MIME_AUDIO_MP3,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/aac",											"aac",				true,		MIME_AUDIO_AAC,									MIME_AUDIO_AAC,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/g72",											"aac",				true,		MIME_AUDIO_G72,									MIME_AUDIO_AAC,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/amr",											"amr",				true,		MIME_AUDIO_AMR,									MIME_AUDIO_AMR,							MIME_APPLICATION_VOICEMEMO,				MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/amr-wb",										"amr",				true,		MIME_AUDIO_AMR_WB,								MIME_AUDIO_AMR_WB,						MIME_APPLICATION_VOICEMEMO,				MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/mmf",											"mmf",				true,		MIME_AUDIO_MMF,									MIME_AUDIO_MMF,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/smaf",											"mmf",				true,		MIME_AUDIO_SMAF,								MIME_AUDIO_SMAF,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/iMelody",										"imy",				true,		MIME_AUDIO_IMELODY,								MIME_AUDIO_IMELODY,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/imelody",										"imy",				true,		MIME_AUDIO_IMELODY2,								MIME_AUDIO_IMELODY,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/melody",										"imy",				true,		MIME_AUDIO_MELODY,								MIME_AUDIO_IMELODY,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/mid",											"mid",				true,		MIME_AUDIO_MID,									MIME_AUDIO_MID,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/midi",											"mid"/*,midi"*/,	true,		MIME_AUDIO_MIDI,								MIME_AUDIO_MID,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/sp-midi",										"spm"/*,midi"*/,	true,		MIME_AUDIO_SP_MIDI,								MIME_AUDIO_SP_MIDI,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/wave",											"wav",				true,		MIME_AUDIO_WAVE,								MIME_AUDIO_WAVE,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/wav",											"wav",				true,		MIME_AUDIO_WAV,								MIME_AUDIO_WAVE,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/3gpp",											"3gp",				true,		MIME_AUDIO_3GPP,								MIME_AUDIO_3GPP,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/mp4",											"mp4",				true,		MIME_AUDIO_MP4,									MIME_AUDIO_MP4,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/MP4A-LATM",									"mp4",				true,		MIME_AUDIO_MP4A_LATM,							MIME_AUDIO_MP4A_LATM,					MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/m4a",											"m4a",				true,		MIME_AUDIO_M4A,									MIME_AUDIO_M4A,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/mpeg4",										"mp4",				true,		MIME_AUDIO_MPEG4,								MIME_AUDIO_MPEG4,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/wma",											"wma",				true,		MIME_AUDIO_WMA,									MIME_AUDIO_WMA,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/xmf",											"xmf",				true,		MIME_AUDIO_XMF,									MIME_AUDIO_XMF,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/imy",											"imy",				true,		MIME_AUDIO_IMY,									MIME_AUDIO_IMY,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/mobile-xmf",									"mxmf",				true,		MIME_AUDIO_MOBILE_XMF,							MIME_AUDIO_XMF,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},

	// 85
	{"audio/vnd.rn-realaudio",								"rm"/*,ram,ra"*/,	true,		MIME_AUDIO_VND_RN_REALAUDIO,					MIME_AUDIO_VND_RN_REALAUDIO,			MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},

	//86
	{"audio/x-mpeg",										"mp3",				true,		MIME_AUDIO_X_MPEG,								MIME_AUDIO_MP3,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-mp3",										"mp3",				true,		MIME_AUDIO_X_MP3,								MIME_AUDIO_MP3,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-mpeg3",										"mp3",				true,		MIME_AUDIO_X_MPEG3,								MIME_AUDIO_MP3,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-mpg",										"mp3",				true,		MIME_AUDIO_X_MPG,								MIME_AUDIO_MP3,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-amr",										"amr",				true,		MIME_AUDIO_X_AMR,								MIME_AUDIO_AMR,							MIME_APPLICATION_VOICEMEMO,				MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-mmf",										"mmf",				true,		MIME_AUDIO_X_MMF,								MIME_AUDIO_MMF,							MIME_APPLICATION_VOICEMEMO,				MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-smaf",										"mmf",				true,		MIME_AUDIO_X_SMAF,								MIME_AUDIO_SMAF,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-iMelody",									"imy",				true,		MIME_AUDIO_X_IMELODY,							MIME_AUDIO_IMELODY,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-midi",										"mid"/*,midi"*/,	true,		MIME_AUDIO_X_MIDI,								MIME_AUDIO_MID,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-mpegaudio",									"mp3",				true,		MIME_AUDIO_X_MPEGAUDIO,							MIME_AUDIO_MP3,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-pn-realaudio",								"ra"/*,ram,ra"*/,	true,		MIME_AUDIO_X_PN_REALAUDIO,						MIME_AUDIO_VND_RN_REALAUDIO,			MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-pn-multirate-realaudio",						"rm"/*,ram,ra"*/,	true,		MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO,			MIME_AUDIO_VND_RN_REALAUDIO,			MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-pn-multirate-realaudio-live",				"rm"/*,ram,ra"*/,	true,		MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO_LIVE,		MIME_AUDIO_VND_RN_REALAUDIO,			MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-wave",										"wav",				true,		MIME_AUDIO_X_WAVE,								MIME_AUDIO_WAVE,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-wav",										"wav",				true,		MIME_AUDIO_X_WAV,								MIME_AUDIO_WAVE,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-ms-wma",										"wma",				true,		MIME_AUDIO_X_MS_WMA,							MIME_AUDIO_WAVE,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-mid",										"mid",				true,		MIME_AUDIO_X_MID,								MIME_AUDIO_MID,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-ms-asf",										"asf",				true,		MIME_AUDIO_X_MS_ASF,							MIME_AUDIO_X_MS_ASF,					MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/x-xmf",										"xmf",				true,		MIME_AUDIO_X_XMF,								MIME_AUDIO_XMF,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},

	//105
	{"image/gif",											"gif",				false,		MIME_IMAGE_GIF,									MIME_IMAGE_GIF,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,			0x1d	},
	{"image/jpeg",											"jpg"/*,jpeg,jpe,jpz"*/,	false,	MIME_IMAGE_JPEG,							MIME_IMAGE_JPG,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,		0x1e	},
	{"image/jpg",											"jpg",				false,		MIME_IMAGE_JPG,									MIME_IMAGE_JPG,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,			UNDEFINED_BINARY	},
	{"image/tiff",											"tif"/*,tiff"*/,	false,		MIME_IMAGE_TIFF,								MIME_IMAGE_TIF,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,			0x1f	},
	{"image/tif",											"tif",				false,		MIME_IMAGE_TIF,									MIME_IMAGE_TIF,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,			UNDEFINED_BINARY	},
	{"image/png",											"png"/*,pnz"*/,		false,		MIME_IMAGE_PNG,									MIME_IMAGE_PNG,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,			0x20	},
	{"image/wbmp",											"wbmp",				false,		MIME_IMAGE_WBMP,								MIME_IMAGE_WBMP,						MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,			UNDEFINED_BINARY	},
	{"image/pjpeg",										"jpg",				false,		MIME_IMAGE_PJPEG,								MIME_IMAGE_JPG,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,			UNDEFINED_BINARY	},
	{"image/bmp",											"bmp",				false,		MIME_IMAGE_BMP,									MIME_IMAGE_BMP,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,			UNDEFINED_BINARY	},
	{"image/svg+xml",										"svg",				false,		MIME_IMAGE_SVG,									MIME_IMAGE_SVG,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,			UNDEFINED_BINARY	},
	{"image/svg-xml",										"svg",				false,		MIME_IMAGE_SVG1,								MIME_IMAGE_SVG,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,			UNDEFINED_BINARY	},

	//116
	{"image/vnd.wap.wbmp",									"wbmp",				false,		MIME_IMAGE_VND_WAP_WBMP,						MIME_IMAGE_WBMP,						MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,		0x21	},
	{"image/vnd.tmo.my5-gif",								"gif",				false,		MIME_IMAGE_VND_TMO_GIF,							MIME_IMAGE_GIF,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,		UNDEFINED_BINARY	},
	{"image/vnd.tmo.my5-jpg",								"jpg",				false,		MIME_IMAGE_VND_TMO_JPG,							MIME_IMAGE_JPG,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,		UNDEFINED_BINARY	},

	// 119
	{"image/x-bmp",										"bmp",				false,		MIME_IMAGE_X_BMP,								MIME_IMAGE_BMP,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,		UNDEFINED_BINARY	},

	// 120
	{"message/rfc822",										"elm",				false,		MIME_MESSAGE_RFC822,							MIME_MESSAGE_RFC822,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_MESSAGE,		UNDEFINED_BINARY	},

	//121
	{"multipart/mixed",									"",					false,		MIME_MULTIPART_MIXED,							MIME_MULTIPART_MIXED,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x0c	},
	{"multipart/related",									"",					false,		MIME_MULTIPART_RELATED,							MIME_MULTIPART_RELATED,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},
	{"multipart/alternative",								"",					false,		MIME_MULTIPART_ALTERNATIVE,						MIME_MULTIPART_ALTERNATIVE,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x0f	},
	{"multipart/form-data",								"",					false,		MIME_MULTIPART_FORM_DATA,						MIME_MULTIPART_FORM_DATA,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x0d	},
	{"multipart/byterange",								"",					false,		MIME_MULTIPART_BYTERANGE,						MIME_MULTIPART_BYTERANGE,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x0e	},
	{"multipart/report",									"",					false,		MIME_MULTIPART_REPORT,							MIME_MULTIPART_REPORT,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},
	{"multipart/voice-message",							"",					false,		MIME_MULTIPART_VOICE_MESSAGE,					MIME_MULTIPART_VOICE_MESSAGE,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},

	//128
	{"text/txt",											"txt",				false,		MIME_TEXT_TXT,									MIME_TEXT_TXT,							MIME_APPLICATION_PICSELVIEWER,			MIME_MAINTYPE_TEXT,			UNDEFINED_BINARY	},
	{"text/html",											"html"/*,htm"*/,	false,		MIME_TEXT_HTML,									MIME_TEXT_HTML,							MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x02	},
	{"text/plain",											"txt"/*,vbm,url"*/,				false,		MIME_TEXT_PLAIN,								MIME_TEXT_PLAIN,						MIME_APPLICATION_PICSELVIEWER,			MIME_MAINTYPE_TEXT,			0x03	},
	{"text/css",											"",					false,		MIME_TEXT_CSS,									MIME_TEXT_CSS,							MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x3d	},
	{"text/xml",											"",					false,		MIME_TEXT_XML,									MIME_TEXT_XML,							MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x28	},
	{"text/iMelody",										"imy",				true,		MIME_TEXT_IMELODY,								MIME_TEXT_IMELODY,						MIME_APPLICATION_SOUNDPLAYER,			MIME_MAINTYPE_AUDIO,		UNDEFINED_BINARY	},

	//134
	{"text/vnd.wap.wmlscript",								"",					false,		MIME_TEXT_VND_WAP_WMLSCRIPT,					MIME_TEXT_VND_WAP_WMLSCRIPT,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x09	},
	{"text/vnd.wap.wml",									"wml",				false,		MIME_TEXT_VND_WAP_WML,							MIME_TEXT_VND_WAP_WML,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x08	},
	{"text/vnd.wap.wta-event",								"",					false,		MIME_TEXT_VND_WAP_WTA_EVENT,					MIME_TEXT_VND_WAP_WTA_EVENT,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x0a	},
	{"text/vnd.wap.connectivity-xml",						"",					false,		MIME_TEXT_VND_WAP_CONNECTIVITY_XML,				MIME_TEXT_VND_WAP_CONNECTIVITY_XML,		MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x35	},
	{"text/vnd.wap.si",									"",					false,		MIME_TEXT_VND_WAP_SI,							MIME_TEXT_VND_WAP_SI,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x2d	},
	{"text/vnd.wap.sl",									"",					false,		MIME_TEXT_VND_WAP_SL,							MIME_TEXT_VND_WAP_SL,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x2f	},
	{"text/vnd.wap.co",									"",					false,		MIME_TEXT_VND_WAP_CO,							MIME_TEXT_VND_WAP_CO,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x31	},
	{"text/vnd.sun.j2me.app-descriptor",					"jad",				true,		MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR,			MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR,	MIME_APPLICATION_NONE,						MIME_MAINTYPE_ETC,			UNDEFINED_BINARY	},


	//142
	{"text/x-hdml",										"",					false,		MIME_TEXT_X_HDML,								MIME_TEXT_X_HDML,						MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x04	},
	{"text/x-vCalendar",									"vcs",				true,		MIME_TEXT_X_VCALENDAR,							MIME_TEXT_X_VCALENDAR,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x06	},
	{"text/x-vCard",										"vcf",				true,		MIME_TEXT_X_VCARD,								MIME_TEXT_X_VCARD,						MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x07	},
	{"text/x-iMelody",										"imy",				true,		MIME_TEXT_X_IMELODY,							MIME_TEXT_X_IMELODY,					MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,		UNDEFINED_BINARY	},
	{"text/x-imelody",										"imy",				true,		MIME_TEXT_X_IMELODY2,							MIME_TEXT_X_IMELODY,					MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,		UNDEFINED_BINARY	},
	{"text/x-vnote",										"vnt",				true,		MIME_TEXT_X_VNOTE,								MIME_TEXT_X_VNOTE,						MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			UNDEFINED_BINARY	},

	//148
	{"video/mpeg4",										"mp4",				true,		MIME_VIDEO_MPEG4,								MIME_VIDEO_MP4,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/mp4",											"mp4",				true,		MIME_VIDEO_MP4,									MIME_VIDEO_MP4,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/h263",											"3gp"/*,3gpp,mp4"*/,true,		MIME_VIDEO_H263,								MIME_VIDEO_H263,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/3gpp",											"3gp"/*,3gpp"*/,	true,		MIME_VIDEO_3GPP,								MIME_VIDEO_3GPP,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/3gp",											"3gp"/*,3gpp"*/,	true,		MIME_VIDEO_3GP,									MIME_VIDEO_3GP,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/avi",											"avi",				false,		MIME_VIDEO_AVI,									MIME_VIDEO_AVI,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/sdp",											"sdp",				true,		MIME_VIDEO_SDP,									MIME_APPLICATION_SDP,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/mp4v-es",											"3gp",				true,		MIME_VIDEO_MP4_ES,									MIME_VIDEO_3GP,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/mpeg",											"3gp",				true,		MIME_VIDEO_MPEG,									MIME_VIDEO_3GP,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},

	// 157
	{"video/vnd.rn-realvideo",								"rm",				true,		MIME_VIDEO_VND_RN_REALVIDEO,					MIME_VIDEO_VND_RN_REALVIDEO,			MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,			UNDEFINED_BINARY	},
	{"video/vnd.rn-realmedia",								"rm",				true,		MIME_VIDEO_VND_RN_REALMEDIA,					MIME_VIDEO_VND_RN_REALMEDIA,			MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,			UNDEFINED_BINARY	},

	//159
	{"video/x-mp4",										"mp4",				true,		MIME_VIDEO_X_MP4,								MIME_VIDEO_MP4,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,			UNDEFINED_BINARY	},
	{"video/x-pv-mp4",										"mp4",				true,		MIME_VIDEO_X_PV_MP4,							MIME_VIDEO_MP4,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,			UNDEFINED_BINARY	},
	{"video/x-pn-realvideo",								"rv",				true,		MIME_VIDEO_X_PN_REALVIDEO,						MIME_VIDEO_VND_RN_REALVIDEO,			MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,			UNDEFINED_BINARY	},
	{"video/x-pn-multirate-realvideo",						"rm",				true,		MIME_VIDEO_X_PN_MULTIRATE_REALVIDEO,			MIME_VIDEO_VND_RN_REALVIDEO,			MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,			UNDEFINED_BINARY	},
	{"video/x-ms-wmv",										"wmv",				true,		MIME_VIDEO_X_MS_WMV,							MIME_VIDEO_X_MS_WMV,					MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,			UNDEFINED_BINARY	},
	{"video/x-ms-asf",										"asf",				true,		MIME_VIDEO_X_MS_ASF,							MIME_VIDEO_X_MS_ASF,					MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,			UNDEFINED_BINARY	},
	{"video/x-pv-pvx",										"pvx",				true,		MIME_VIDEO_X_PV_PVX,							MIME_VIDEO_X_PV_PVX,					MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,			UNDEFINED_BINARY	}

};

/**********		 MIME table information	    ***********/
static const int mimeTableIndex[] = {
	1,			16,			49,
	56,			85,			86,
	105,		116,		119,
	120,		120,		120,
	121,		121,		121,
	128,		134,		142,
	148,		157,		159
};


/*****************		Extension definition	  *****************/
static const ExtTable extTable[] = {

	{"txt",			MIME_TEXT_PLAIN},
	{"html",			MIME_TEXT_HTML},
	{"htm",			MIME_TEXT_HTML},
	{"xhtml",			MIME_APPLICATION_XHTML_XML},
	{"wml",			MIME_TEXT_VND_WAP_WML},
	{"vcs",			MIME_TEXT_X_VCALENDAR},
	{"vcf",			MIME_TEXT_X_VCARD},
	{"vnt",			MIME_TEXT_X_VNOTE},
	{"smil",			MIME_APPLICATION_SMIL},
	{"eml",			MIME_MESSAGE_RFC822},
	{"gif",			MIME_IMAGE_GIF},
	{"jpeg",			MIME_IMAGE_JPEG},
	{"jpg",			MIME_IMAGE_JPEG},
	{"jpe",			MIME_IMAGE_JPEG},
	{"jpz",			MIME_IMAGE_JPEG},
	{"tiff",			MIME_IMAGE_TIFF},
	{"tif",			MIME_IMAGE_TIFF},
	{"png",			MIME_IMAGE_PNG},
	{"pnz",			MIME_IMAGE_PNG},
	{"wbmp",			MIME_IMAGE_VND_WAP_WBMP},
	{"bmp",			MIME_IMAGE_BMP},
	{"au",				MIME_AUDIO_BASIC},
	{"snd",			MIME_AUDIO_BASIC},
	{"mp3",			MIME_AUDIO_MP3},
	{"aac",			MIME_AUDIO_AAC},
	{"mp4",			MIME_AUDIO_MP4},
	{"m4a",			MIME_AUDIO_M4A},
	{"amr",			MIME_AUDIO_X_AMR},
	{"mmf",			MIME_APPLICATION_VND_SMAF},
	{"imy",			MIME_AUDIO_X_IMELODY},
	{"mid",			MIME_AUDIO_MID},
	{"midi",			MIME_AUDIO_MID},
	{"spm",			MIME_AUDIO_SP_MIDI},
	{"wav",			MIME_AUDIO_WAVE},
	{"3gp",			MIME_AUDIO_3GPP},
	{"3gpp",			MIME_VIDEO_3GPP},
	{"rm",				MIME_VIDEO_VND_RN_REALVIDEO},
	{"ra",				MIME_AUDIO_VND_RN_REALAUDIO},
	{"ram",			MIME_AUDIO_VND_RN_REALAUDIO},
	{"wma",			MIME_AUDIO_X_MS_WMA},
	{"smp",			MIME_APPLICATION_STUDIOM},
	{"avi",			MIME_VIDEO_AVI},
	{"sdp",			MIME_APPLICATION_SDP},
	{"vbm",			MIME_TEXT_PLAIN},
	{"url",			MIME_TEXT_PLAIN},
	{"jad",			MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR},
	{"jar",			MIME_APPLICATION_VND_SUN_J2ME_JAVA_ARCHIVE},
	{"dd",				MIME_APPLICATION_VND_OMA_DD_XML},
	{"dm",				MIME_APPLICATION_VND_OMA_DRM_MESSAGE},
	{"dcf",			MIME_APPLICATION_VND_OMA_DRM_CONTENT},
	{"rv",				MIME_VIDEO_X_PN_REALVIDEO},
	{"ro",				MIME_APPLICATION_VND_OMA_DRM_RIGHTS_XML},
	{"thm",			MIME_APPLICATION_VND_SAMSUNG_THEME},
	{"xls",			MIME_APPLICATION_X_EXCEL},
	{"pdf",			MIME_APPLICATION_PDF},
	{"ppt",			MIME_APPLICATION_X_POWERPOINT},
	{"swf",			MIME_APPLICATION_X_FLASH},
	{"svg",			MIME_IMAGE_SVG},
	{"doc",			MIME_APPLICATION_MSWORD},
	{"wmv",			MIME_VIDEO_X_MS_WMV},
	{"asf",			MIME_VIDEO_X_MS_ASF},
	{"3ga",			MIME_AUDIO_M4A},
	{"xmf",			MIME_AUDIO_XMF},
	{"mxmf",			MIME_AUDIO_MOBILE_XMF},
	{"pvx",			MIME_VIDEO_X_PV_PVX},
	{"oro",			MIME_APPLICATION_VND_OMA_DRM_RO_XML},
	{"odf",			MIME_APPLICATION_VND_OMA_DRM_DCF}
};


static int mimeTableEnum[] =
{
	MIME_ASTERISK	,

	MIME_APPLICATION_XML	,
	MIME_APPLICATION_WML_XML	,
	MIME_APPLICATION_XHTML_XML	,
	MIME_APPLICATION_JAVA_VM	,
	MIME_APPLICATION_SMIL	,
	MIME_APPLICATION_JAVA_ARCHIVE	,
	MIME_APPLICATION_JAVA	,
	MIME_APPLICATION_OCTET_STREAM	,
	MIME_APPLICATION_STUDIOM	,
	MIME_APPLICATION_FUNMEDIA	,
	MIME_APPLICATION_MSWORD		,
	MIME_APPLICATION_PDF		,
	MIME_APPLICATION_SDP		,
	MIME_APPLICATION_RAM		,
	MIME_APPLICATION_ASTERIC	,


	MIME_APPLICATION_VND_WAP_XHTMLXML	,
	MIME_APPLICATION_VND_WAP_WMLC	,
	MIME_APPLICATION_VND_WAP_WMLSCRIPTC	,
	MIME_APPLICATION_VND_WAP_WTA_EVENTC	,
	MIME_APPLICATION_VND_WAP_UAPROF	,
	MIME_APPLICATION_VND_WAP_SIC	,
	MIME_APPLICATION_VND_WAP_SLC	,
	MIME_APPLICATION_VND_WAP_COC	,
	MIME_APPLICATION_VND_WAP_SIA	,
	MIME_APPLICATION_VND_WAP_CONNECTIVITY_WBXML	,
	MIME_APPLICATION_VND_WAP_MULTIPART_FORM_DATA	,
	MIME_APPLICATION_VND_WAP_MULTIPART_BYTERANGES	,
	MIME_APPLICATION_VND_WAP_MULTIPART_MIXED	,
	MIME_APPLICATION_VND_WAP_MULTIPART_RELATED	,
	MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE	,
	MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC	,
	MIME_APPLICATION_VND_WAP_WBXML	,
	MIME_APPLICATION_VND_OMA_DD_XML	,
	MIME_APPLICATION_VND_OMA_DRM_MESSAGE	,
	MIME_APPLICATION_VND_OMA_DRM_CONTENT	,
	MIME_APPLICATION_VND_OMA_DRM_RIGHTS_XML	,
	MIME_APPLICATION_VND_OMA_DRM_RIGHTS_WBXML	,
	MIME_APPLICATION_VND_OMA_DRM_RO_XML	,
	MIME_APPLICATION_VND_OMA_DRM_DCF	,
	MIME_APPLICATION_VND_OMA_ROAPPDU_XML	,
	MIME_APPLICATION_VND_OMA_ROAPTRIGGER_XML,
	MIME_APPLICATION_VND_SMAF	,
	MIME_APPLICATION_VND_RN_REALMEDIA	,
	MIME_APPLICATION_VND_SUN_J2ME_JAVA_ARCHIVE	,
	MIME_APPLICATION_VND_SAMSUNG_THEME	,
	MIME_APPLICATION_VND_EXCEL	,
	MIME_APPLICATION_VND_POWERPOINT,
	MIME_APPLICATION_VND_MSWORD,


	MIME_APPLICATION_X_HDMLC	,
	MIME_APPLICATION_X_X968_USERCERT	,
	MIME_APPLICATION_X_WWW_FORM_URLENCODED	,
	MIME_APPLICATION_X_SMAF	,
	MIME_APPLICATION_X_FLASH	,
	MIME_APPLICATION_X_EXCEL	,
	MIME_APPLICATION_X_POWERPOINT	,


	MIME_AUDIO_BASIC,
	MIME_AUDIO_MPEG	,
	MIME_AUDIO_MP3	,
	MIME_AUDIO_MPG3	,
	MIME_AUDIO_MPEG3	,
	MIME_AUDIO_MPG	,
	MIME_AUDIO_AAC	,
	MIME_AUDIO_G72	,
	MIME_AUDIO_AMR	,
	MIME_AUDIO_AMR_WB	,
	MIME_AUDIO_MMF	,
	MIME_AUDIO_SMAF	,
	MIME_AUDIO_IMELODY	,
	MIME_AUDIO_IMELODY2 ,
	MIME_AUDIO_MELODY	,
	MIME_AUDIO_MID	,
	MIME_AUDIO_MIDI	,
	MIME_AUDIO_SP_MIDI	,
	MIME_AUDIO_WAVE	,
	MIME_AUDIO_WAV ,
	MIME_AUDIO_3GPP	,
	MIME_AUDIO_MP4	,
	MIME_AUDIO_MP4A_LATM	,
	MIME_AUDIO_M4A	,
	MIME_AUDIO_MPEG4,
	MIME_AUDIO_WMA,
	MIME_AUDIO_XMF,
	MIME_AUDIO_IMY,
	MIME_AUDIO_MOBILE_XMF,


	MIME_AUDIO_VND_RN_REALAUDIO	,


	MIME_AUDIO_X_MPEG	,
	MIME_AUDIO_X_MP3	,
	MIME_AUDIO_X_MPEG3	,
	MIME_AUDIO_X_MPG	,
	MIME_AUDIO_X_AMR	,
	MIME_AUDIO_X_MMF	,
	MIME_AUDIO_X_SMAF	,
	MIME_AUDIO_X_IMELODY	,
	MIME_AUDIO_X_MIDI	,
	MIME_AUDIO_X_MPEGAUDIO	,
	MIME_AUDIO_X_PN_REALAUDIO	,
	MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO	,
	MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO_LIVE	,
	MIME_AUDIO_X_WAVE	,
	MIME_AUDIO_X_WAV	,
	MIME_AUDIO_X_MS_WMA	,
	MIME_AUDIO_X_MID ,
	MIME_AUDIO_X_MS_ASF ,
	MIME_AUDIO_X_XMF,


	MIME_IMAGE_GIF	,
	MIME_IMAGE_JPEG	,
	MIME_IMAGE_JPG	,
	MIME_IMAGE_TIFF	,
	MIME_IMAGE_TIF	,
	MIME_IMAGE_PNG	,
	MIME_IMAGE_WBMP	,
	MIME_IMAGE_PJPEG	,
	MIME_IMAGE_BMP	,
	MIME_IMAGE_SVG	,
	MIME_IMAGE_SVG1	,
	MIME_IMAGE_VND_WAP_WBMP	,
	MIME_IMAGE_VND_TMO_GIF,
	MIME_IMAGE_VND_TMO_JPG,
	MIME_IMAGE_X_BMP	,


	MIME_MESSAGE_RFC822	,


	MIME_MULTIPART_MIXED	,
	MIME_MULTIPART_RELATED	,
	MIME_MULTIPART_ALTERNATIVE	,
	MIME_MULTIPART_FORM_DATA	,
	MIME_MULTIPART_BYTERANGE	,
	MIME_MULTIPART_REPORT	,
	MIME_MULTIPART_VOICE_MESSAGE	,


	MIME_TEXT_TXT	,
	MIME_TEXT_HTML	,
	MIME_TEXT_PLAIN	,
	MIME_TEXT_CSS	,
	MIME_TEXT_XML	,
	MIME_TEXT_IMELODY	,
	MIME_TEXT_VND_WAP_WMLSCRIPT	,
	MIME_TEXT_VND_WAP_WML	,
	MIME_TEXT_VND_WAP_WTA_EVENT	,
	MIME_TEXT_VND_WAP_CONNECTIVITY_XML	,
	MIME_TEXT_VND_WAP_SI	,
	MIME_TEXT_VND_WAP_SL	,
	MIME_TEXT_VND_WAP_CO	,
	MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR	,
	MIME_TEXT_X_HDML	,
	MIME_TEXT_X_VCALENDAR	,
	MIME_TEXT_X_VCARD	,
	MIME_TEXT_X_IMELODY	,
	MIME_TEXT_X_IMELODY2 ,
	MIME_TEXT_X_VNOTE ,


	MIME_VIDEO_MPEG4	,
	MIME_VIDEO_MP4	,
	MIME_VIDEO_H263	,
	MIME_VIDEO_3GPP	,
	MIME_VIDEO_3GP	,
	MIME_VIDEO_AVI	,
	MIME_VIDEO_SDP	,
	MIME_VIDEO_MP4_ES,
	MIME_VIDEO_MPEG	,
	MIME_VIDEO_VND_RN_REALVIDEO	,
	MIME_VIDEO_VND_RN_REALMEDIA	,
	MIME_VIDEO_X_MP4	,
	MIME_VIDEO_X_PV_MP4	,
	MIME_VIDEO_X_PN_REALVIDEO	,
	MIME_VIDEO_X_PN_MULTIRATE_REALVIDEO	,
	MIME_VIDEO_X_MS_WMV ,
	MIME_VIDEO_X_MS_ASF,
	MIME_VIDEO_X_PV_PVX ,

	MIME_UNKNOWN	// MIME_MAX
};


#define MIME_MAX_NUM	166
#define EXT_MAX	67
#define MIME_SUB_TYPE_VND	1
#define MIME_SUB_TYPE_X		2

int __MimeGetTableIndexInt(MimeType mime);
int __MimeGetTableIndexString(const char *szMime);
MimeMainType __MimeGetMainTypeName(const char *szType);


/*
 * This function checks whether a mime is downloadable or not.
 *
 * @param	mime [in] Enumeration number for a MIME type.
 * @return	This function returns true if downloadable, or false.
 */
bool MimeIsDownloadableInt(MimeType mime)
{
	int index;

	index = __MimeGetTableIndexInt(mime);
	if (index == MIME_UNKNOWN)
		return false;

	return mimeTable[index].bDownloadable;
}

/*
 * This function checks whether a mime is downloadable or not.
 *
 * @param	mime [in] MIME string.
 * @return	This function returns true if downloadable, or false.
 */
bool MimeIsDownloadableString(const char *szMime)
{
	int index;

	index = __MimeGetTableIndexString(szMime);
	if (index == MIME_UNKNOWN)
		return false;

	return mimeTable[index].bDownloadable;
}

/*
 * This function checks main type of a MIME.
 *
 * @param	mime [in] Enumeration number for a MIME type.
 * @return	This function returns main type of a MIME.
 */
MimeMainType MimeGetMainTypeInt(MimeType mime)
{
	int index;

	index = __MimeGetTableIndexInt(mime);
	if (index == MIME_UNKNOWN)
		return MIME_MAINTYPE_ETC;

	return mimeTable[index].mainType;
}

/*
 * This function checks main type of a MIME.
 *
 * @param	mime [in] MIME string.
 * @return	This function returns main type of a MIME.
 */
MimeMainType MimeGetMainTypeString(const char *szMime)
{
	int index;

	index = __MimeGetTableIndexString(szMime);
	if (index == MIME_UNKNOWN)
		return MIME_MAINTYPE_ETC;

	return mimeTable[index].mainType;
}

/*
 * This function returns a extension name for a specified MIME.
 *
 * @param	mime [in] Enumeration number for a MIME type.
 * @return	This function returns Extension string.
 */
char *MimeGetExtFromMimeInt(MimeType mime)
{
	int index;

	index = __MimeGetTableIndexInt(mime);
	if (index == MIME_UNKNOWN)
		return NULL;

	return (char *)mimeTable[index].szExt;
}

/*
 * This function returns a extension name for a specified MIME.
 *
 * @param	mime [in] MIME string.
 * @return	This function returns Extension string.
 */
char *MimeGetExtFromMimeString(const char *szMime)
{
	int index;

	index = __MimeGetTableIndexString(szMime);
	if (index == MIME_UNKNOWN)
		return NULL;

	return (char *)mimeTable[index].szExt;
}


/*
 * This function returns a MIME type for a specified Extension.
 *
 * @param	mime [in] Extension string.
 * @return	This function returns MIME string.
 */
char *MimeGetMimeFromExtString(const char *szExt)
{
	int i;

	for (i = 0; i < EXT_MAX; i++) {
		if (!strcasecmp( extTable[i].szExt, szExt)) {
			int index;

			index = __MimeGetTableIndexInt(extTable[i].mimeType);
			if (index == MIME_UNKNOWN)
				return NULL;

			return (char *)mimeTable[index].szMIME;
		}
	}

	return NULL;
}

/*
 * This function returns a MIME type for a specified Extension.
 *
 * @param	mime [in] Extension string.
 * @return	This function returns MIME string.
 */
MimeType MimeGetMimeFromExtInt(const char *szExt)
{
	int i;

	for (i = 0; i < EXT_MAX; i++) {
		if (!strcasecmp( extTable[i].szExt, szExt))
			return extTable[i].mimeType;
	}

	return MIME_UNKNOWN;
}

/*
 * This function returns index number in MIME definition table with MIME enumeration.
 * Internal function.
 */
int __MimeGetTableIndexInt(MimeType mime)
{
	int type;
	int subtype;
	int index;
	int tableIndex;

	if (mime == MIME_UNKNOWN)
		return MIME_UNKNOWN;

	type = (mime & 0xf000) >> 12;
	subtype = (mime & 0x0f00) >> 8;
	index = (mime & 0x003f);

	//If 'type' is zero, mimeTableIndex's array value have a minus value
	//If 'type' is zero, mime type is '*/*'
	if (type == 0)
		tableIndex = 0;
	else
		tableIndex = mimeTableIndex[(type - 1) * 3 + subtype] + index;

	return tableIndex;
}


#define MIME_MAX_LEN	43
/*
 * This function returns index number in MIME definition table with MIME string.
 * Internal function.
 */
int __MimeGetTableIndexString(const char *szMime)
{
	int type;
	int subtype;
	char szType[50];
	char szSubType[50];
	char *szTmpStart = NULL;
	char c;
	int i = 0;
	int j = 0;
	int start;
	int end;
	char *szMIMEType = NULL;
	int len;

	if (szMime == NULL) {
		MSG_DEBUG("szMime is NULL");
		return MIME_UNKNOWN;
	}

	szMIMEType = (char * )malloc(strlen(szMime) + 1);

	if (szMIMEType == NULL) {
		MSG_DEBUG("szMime is NULL");
		 return MIME_UNKNOWN;
	}

	memset(szMIMEType, 0, strlen(szMime) + 1);

	if (!strcmp(szMime, "*/*")) {
		free(szMIMEType);
		return 0;
	}
	strcpy(szMIMEType, szMime);
	type = 0;
	subtype = 0;

	szTmpStart = szMIMEType;
	len = strlen(szTmpStart);

	while (true) {
		if (i >= len) {
			free(szMIMEType);
			return MIME_UNKNOWN;
		}

		c = szTmpStart[i++];

		if (c == '/') {
			szType[j] = '\0';
			type = __MimeGetMainTypeName(szType);
			szTmpStart = &szTmpStart[i];
			break;
		} else
			szType[j++] = c;
	}

	i = 0;
	j = 0;
	len = strlen(szTmpStart);

	while (true) {
		c = szTmpStart[i++];
		if (i > len) {
			szSubType[j] = '\0';
			break;
		}

		if (c == '.') {
			szSubType[j] = '\0';
			if (!strcasecmp(szSubType, "vnd"))
				subtype = MIME_SUB_TYPE_VND;
			break;
		} else if (c == '-') {
			szSubType[j] = '\0';
			if (!strcasecmp(szSubType, "x"))
				subtype = MIME_SUB_TYPE_X;
			break;
		} else
			szSubType[j++] = c;
	}


	start = mimeTableIndex[type * 3 + subtype];

	if (type == MIME_MAINTYPE_VIDEO && subtype == MIME_SUB_TYPE_X)
		end = MIME_MAX_NUM;
	else
		end = mimeTableIndex[type * 3 + subtype + 1];

	if (start == end && type < MIME_MAINTYPE_VIDEO) {
		end = mimeTableIndex[(type + 1) * 3 + subtype + 1];
	}


	for (i = start; i < end; i++) {
		if (i >= MIME_MAX_NUM)
			break;

		if (!strcasecmp(mimeTable[i].szMIME, szMime)) {
			free(szMIMEType);
			return i;
		}
	}

	free(szMIMEType);
	return MIME_UNKNOWN;
}

/*
 * This function returns main type of MIME : Internal function.
 */
MimeMainType __MimeGetMainTypeName(const char *szType)
{
	if (szType == NULL )
		return MIME_MAINTYPE_APPLICATION;

	if (!strcasecmp(szType, "application"))
		return MIME_MAINTYPE_APPLICATION;
	else if (!strcasecmp(szType, "audio"))
		return MIME_MAINTYPE_AUDIO;
	else if (!strcasecmp(szType, "image"))
		return MIME_MAINTYPE_IMAGE;
	else if (!strcasecmp(szType, "message"))
		return MIME_MAINTYPE_MESSAGE;
	else if (!strcasecmp(szType, "multipart"))
		return MIME_MAINTYPE_MULTIPART;
	else if (!strcasecmp(szType, "text"))
		return MIME_MAINTYPE_TEXT;
	else if (!strcasecmp(szType, "video"))
		return MIME_MAINTYPE_VIDEO;

	return MIME_MAINTYPE_APPLICATION;
}



// MimeString -> MimeInt
MimeType MimeGetMimeIntFromMimeString(char *szMimeStr)
{
	int index = __MimeGetTableIndexString(szMimeStr);
	if (index == MIME_UNKNOWN) {
		return MIME_UNKNOWN;
	} else {
		return (MimeType)mimeTableEnum[index];
	}
}

// MimeInt -> MimeString
char *MimeGetMimeStringFromMimeInt(int mimeType)
{
	int idx = __MimeGetTableIndexInt((MimeType)mimeType);

	if (idx == MIME_UNKNOWN)
		return NULL;

	return (char *)mimeTable[idx].szMIME;
}


// BinaryCode -> MimeInt
MimeType MimeGetMimeIntFromBi(int binCode)
{
	int index;

	if (binCode < 0x00 || binCode > 0x4b)
		return MIME_UNKNOWN;

	for (index = 0; index < MIME_MAX_NUM; index++) {
		if (binCode == mimeTable[index].binary)
			return (MimeType)mimeTableEnum[index];
	}

	return MIME_UNKNOWN;
}

// BinaryCode -> MimeInt
char *MimeGetMimeStringFromBi(int binCode)
{
	int index;

	if (binCode < 0x00 || binCode > 0x4b)
		return NULL;

	for (index = 0; index < MIME_MAX_NUM; index++) {
		if (binCode == mimeTable[index].binary)
			return (char *)mimeTable[index].szMIME;
	}

	return NULL;
}

// Mimeint -> Binary Value
int MimeGetBinaryValueFromMimeInt(MimeType mime)
{
	int index;

	index = __MimeGetTableIndexInt(mime);
	if (index == MIME_UNKNOWN)
		return UNDEFINED_BINARY;

	return mimeTable[index].binary;
}

// MimeString -> Binary value
int MimeGetBinaryValueFromMimeString(const char *szMime)
{
	int index;

	index = __MimeGetTableIndexString(szMime);
	if (index == MIME_UNKNOWN)
		return UNDEFINED_BINARY;

	return mimeTable[index].binary;
}


/*
 * This function checks main type of a MIME.
 *
 * @param	mime [in] Enumeration number for a MIME type.
 * @return	This function returns application type of a MIME.
 */
MimeAppType MimeGetAppTypeFromInt(MimeType mime)
{
	int index;

	index = __MimeGetTableIndexInt(mime);
	if (index == MIME_UNKNOWN)
		return MIME_APPLICATION_NONE;

	return mimeTable[index].appType;
}

/*
 * This function checks main type of a MIME.
 *
 * @param	mime [in] MIME string.
 * @return	This function returns application type of a MIME.
 */
MimeAppType MimeGetAppTypeFromString(const char *szMime)
{
	int index;

	index = __MimeGetTableIndexString(szMime);
	if (index == MIME_UNKNOWN)
		return MIME_APPLICATION_NONE;

	return mimeTable[index].appType;
}


/*
 * This function returns a application type for a specified Extension.
 *
 * @param	mime [in] Extension string.
 * @return	This function returns application type.
 */
MimeAppType MimeGetAppTypeFromExtString(const char *szExt)
{
	int i;

	for (i = 0; i < EXT_MAX; i++) {
		if (!strcasecmp( extTable[i].szExt, szExt)) {
			int index;

			index = __MimeGetTableIndexInt(extTable[i].mimeType);
			if (index == MIME_UNKNOWN)
				return MIME_APPLICATION_NONE;
			return mimeTable[index].appType;
		}
	}

	return MIME_APPLICATION_NONE;
}

/*
 * This function gets the representative mime type from MimeType
 *
 * @param	mime [in] Enumeration number for a MIME type.
 * @return	representative mime type Enumeration number
 */

MimeType MimeGetContentTypeFromInt(MimeType mime)
{
	int index;

	for (index = 0; index < MIME_MAX_NUM; index++) {
		if (mime == mimeTable[index].mime)
			return mimeTable[index].contentType;
	}

	return MIME_UNKNOWN;
}

/*
 * This function gets the representative mime type from mime string
 *
 * @param	szMime - string name of MimeType
 * @return	representative mime type Enumeration number
 */

MimeType MimeGetContentTypeFromString(const char *szMime)
{
	int index;

	for (index = 0; index < MIME_MAX_NUM; index++) {
		if (!strcasecmp(szMime, mimeTable[index].szMIME))
			return mimeTable[index].contentType;
	}

	return MIME_UNKNOWN;
}

/*
 * This function gets the index from mime string
 *
 * @param	szMime - string name of MimeType
 * @return	Enumeration number for a MIME type.
 */

MimeType MimeGetMimeTypeFromString(const char *szMime)
{
	int index;

	for (index = 0; index < MIME_MAX_NUM; index++) {
		if (!strcasecmp(szMime, mimeTable[index].szMIME))
			return mimeTable[index].mime;
	}

	return MIME_UNKNOWN;
}


int MsgGetCode(MsgHeaderField tableId, char *pStr)
{
	int cCode = 0;
	int nNum = MSG_FIELD_UNKNOWN;
	char **pTable = NULL;

	switch (tableId) {
	case MSG_FIELD:
		nNum = MSG_FIELD_NUM;
		pTable = (char **)szMsgField;
		break;

	case MSG_PARAM:
		nNum = MSG_PARAM_NUM;
		pTable = (char **)szMsgParam;
		break;

	case MSG_TYPE:
		return MimeGetMimeIntFromMimeString(pStr);

	case MSG_CHARSET:
		nNum = MSG_CHARSET_NUM;
		pTable = (char **)szMsgCharset;
		break;

	case MSG_ENCODING:
		nNum = MSG_ENCODING_NUM;
		pTable = (char **)szMsgEncoding;
		break;

	case MSG_DISPOSITION:
		nNum = MSG_DISPOSITION_NUM;
		pTable = (char **)szMsgDisposition;
		break;

	case MSG_ADDR_TYPE:
		nNum = MSG_ADDR_TYPE_NUM;
		pTable = (char **)szMsgAddrType;
		break;

	default:
		break;
	}

	for (cCode = 0; cCode < nNum; cCode++) {
		if (!strcasecmp(pStr, pTable[cCode])) {
			return cCode;
		}
	}

	return -1;
}


char *MsgGetString(MsgHeaderField tableId, int code)
{
	int nNum = MSG_FIELD_UNKNOWN;
	char **pTable = NULL;

	switch (tableId) {
	case MSG_FIELD:
		nNum = MSG_FIELD_NUM;
		pTable = (char **)szMsgField;
		break;

	case MSG_PARAM:
		nNum = MSG_PARAM_NUM;
		pTable = (char **)szMsgParam;
		break;

	case MSG_TYPE:
		if (code != MIME_UNKNOWN && code != -1)
			return MimeGetMimeStringFromMimeInt(code);
		else
			return (char *)MSG_UNKNOWN_TYPE_STRING;

	case MSG_CHARSET:
		nNum = MSG_CHARSET_NUM;
		pTable = (char **)szMsgCharset;
		break;

	case MSG_ENCODING:
		nNum = MSG_ENCODING_NUM;
		pTable = (char **)szMsgEncoding;
		break;

	case MSG_DISPOSITION:
		nNum = MSG_DISPOSITION_NUM;
		pTable = (char **)szMsgDisposition;
		break;

	case MSG_ADDR_TYPE:
		nNum = MSG_ADDR_TYPE_NUM;
		pTable = (char **)szMsgAddrType;
		break;

	default:
		break;
	}

	if (code < 0 || code >= nNum || !pTable)
		return (char *)MSG_UNKNOWN_TYPE_STRING;

	return pTable[code];
}

char *_MsgSkipWS3(char *s)
{
	while (true) {
		if ((*s == MSG_CH_CR) ||
			(*s == MSG_CH_LF) ||
			(*s == MSG_CH_SP) ||
			(*s == MSG_CH_TAB))
			++s;
		else
			return s;
	}
}


int _MsgGetCode(MsgHeaderField tableId, char *pStr)
{
	int cCode = 0;
	int nNum = MSG_FIELD_UNKNOWN;
	char **pTable = NULL;

	switch (tableId) {
	case MSG_FIELD:
		nNum = MSG_FIELD_NUM;
		pTable = (char **)szMsgField;
		break;

	case MSG_PARAM:
		nNum = MSG_PARAM_NUM;
		pTable = (char **)szMsgParam;
		break;

	case MSG_TYPE:
		return MimeGetMimeIntFromMimeString(pStr);

	case MSG_CHARSET:
		nNum = MSG_CHARSET_NUM;
		pTable = (char **)szMsgCharset;
		break;

	case MSG_ENCODING:
		nNum = MSG_ENCODING_NUM;
		pTable = (char **)szMsgEncoding;
		break;

	case MSG_DISPOSITION:
		nNum = MSG_DISPOSITION_NUM;
		pTable = (char **)szMsgDisposition;
		break;

	case MSG_ADDR_TYPE:
		nNum = MSG_ADDR_TYPE_NUM;
		pTable = (char **)szMsgAddrType;
		break;

	default:
		MSG_DEBUG("_MsgGetCode: Invalid tableId [%d] \n", tableId);
		break;
	}

	for (cCode = 0; cCode < nNum; cCode++) {
		if (pTable[cCode] != NULL) {
			if (!strcasecmp( pStr, pTable[cCode])) {
				return cCode;
			}
		}
	}

	return INVALID_HOBJ;
}

