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

#include <string>
#include <stdlib.h>
#include <metadata_extractor.h>
#include "MsgUtilMime.h"
#include "MsgDebug.h"

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

/**************************************************     MIME definition     ***************************************************/
static const MimeTable mimeTable[] = {
	/* 0 */
	{"*/*",													"",					false,		MIME_ASTERISK,									MIME_ASTERISK,							MIME_APPLICATION_NONE,			MIME_MAINTYPE_ETC,				UNDEFINED_BINARY	},

    /* 1 */
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

	/* 16 */
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

	/* 49 */
	{"application/x-hdmlc",								"",					false,		MIME_APPLICATION_X_HDMLC,						MIME_APPLICATION_X_HDMLC,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x13	},
	{"application/x-x968-user-cert",						"",					false,		MIME_APPLICATION_X_X968_USERCERT,				MIME_APPLICATION_X_X968_USERCERT,		MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x2c	},
	{"application/x-www-form-urlencoded",					"",					false,		MIME_APPLICATION_X_WWW_FORM_URLENCODED,			MIME_APPLICATION_X_WWW_FORM_URLENCODED,		MIME_APPLICATION_NONE,				MIME_MAINTYPE_APPLICATION,	0x12	},
	{"application/x-smaf",									"mmf",				true,		MIME_APPLICATION_X_SMAF,						MIME_APPLICATION_X_SMAF,				MIME_APPLICATION_SOUNDPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"application/x-shockwave-flash",						"swf",				true,		MIME_APPLICATION_X_FLASH,						MIME_APPLICATION_X_FLASH,				MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,		UNDEFINED_BINARY	},
	{"application/x-msexcel",								"xls",				true,		MIME_APPLICATION_X_EXCEL,						MIME_APPLICATION_X_EXCEL,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},
	{"application/x-mspowerpoint",							"ppt",				true,		MIME_APPLICATION_X_POWERPOINT,					MIME_APPLICATION_X_POWERPOINT,			MIME_APPLICATION_PICSELVIEWER,			MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},


	/* 56 */
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
	{"audio/imelody",										"imy",				true,		MIME_AUDIO_IMELODY,								MIME_AUDIO_IMELODY,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/iMelody",										"imy",				true,		MIME_AUDIO_IMELODY2,								MIME_AUDIO_IMELODY,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
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
	{"audio/mobile-xmf",									"mxmf",				true,		MIME_AUDIO_MOBILE_XMF,							MIME_AUDIO_MOBILE_XMF,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},
	{"audio/ogg",											"ogg",				true,		MIME_AUDIO_OGG,								MIME_AUDIO_OGG,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},

	/* 86 */
	{"audio/vnd.rn-realaudio",								"rm"/*,ram,ra"*/,	true,		MIME_AUDIO_VND_RN_REALAUDIO,					MIME_AUDIO_VND_RN_REALAUDIO,			MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,			UNDEFINED_BINARY	},

	/* 87 */
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

	/* 106 */
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

	/* 117 */
	{"image/vnd.wap.wbmp",									"wbmp",				false,		MIME_IMAGE_VND_WAP_WBMP,						MIME_IMAGE_WBMP,						MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,		0x21	},
	{"image/vnd.tmo.my5-gif",								"gif",				false,		MIME_IMAGE_VND_TMO_GIF,							MIME_IMAGE_GIF,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,		UNDEFINED_BINARY	},
	{"image/vnd.tmo.my5-jpg",								"jpg",				false,		MIME_IMAGE_VND_TMO_JPG,							MIME_IMAGE_JPG,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,		UNDEFINED_BINARY	},

	/* 120 */
	{"image/x-bmp",										"bmp",				false,		MIME_IMAGE_X_BMP,								MIME_IMAGE_BMP,							MIME_APPLICATION_IMAGEVIEWER,			MIME_MAINTYPE_IMAGE,		UNDEFINED_BINARY	},

	/* 121 */
	{"message/rfc822",										"elm",				false,		MIME_MESSAGE_RFC822,							MIME_MESSAGE_RFC822,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_MESSAGE,		UNDEFINED_BINARY	},

	/* 122 */
	{"multipart/mixed",									"",					false,		MIME_MULTIPART_MIXED,							MIME_MULTIPART_MIXED,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x0c	},
	{"multipart/related",									"",					false,		MIME_MULTIPART_RELATED,							MIME_MULTIPART_RELATED,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},
	{"multipart/alternative",								"",					false,		MIME_MULTIPART_ALTERNATIVE,						MIME_MULTIPART_ALTERNATIVE,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x0f	},
	{"multipart/form-data",								"",					false,		MIME_MULTIPART_FORM_DATA,						MIME_MULTIPART_FORM_DATA,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x0d	},
	{"multipart/byterange",								"",					false,		MIME_MULTIPART_BYTERANGE,						MIME_MULTIPART_BYTERANGE,				MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	0x0e	},
	{"multipart/report",									"",					false,		MIME_MULTIPART_REPORT,							MIME_MULTIPART_REPORT,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},
	{"multipart/voice-message",							"",					false,		MIME_MULTIPART_VOICE_MESSAGE,					MIME_MULTIPART_VOICE_MESSAGE,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_APPLICATION,	UNDEFINED_BINARY	},

	/* 129 */
	{"text/txt",											"",				false,		MIME_TEXT_TXT,									MIME_TEXT_TXT,							MIME_APPLICATION_PICSELVIEWER,			MIME_MAINTYPE_TEXT,			UNDEFINED_BINARY	},
	{"text/html",											"html"/*,htm"*/,	false,		MIME_TEXT_HTML,									MIME_TEXT_HTML,							MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x02	},
	{"text/plain",											"txt" /*,vbm,url"*/,				false,		MIME_TEXT_PLAIN,								MIME_TEXT_PLAIN,						MIME_APPLICATION_PICSELVIEWER,			MIME_MAINTYPE_TEXT,			0x03	},
	{"text/css",											"",					false,		MIME_TEXT_CSS,									MIME_TEXT_CSS,							MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x3d	},
	{"text/xml",											"",					false,		MIME_TEXT_XML,									MIME_TEXT_XML,							MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x28	},
	{"text/iMelody",										"imy",				true,		MIME_TEXT_IMELODY,								MIME_TEXT_IMELODY,						MIME_APPLICATION_SOUNDPLAYER,			MIME_MAINTYPE_AUDIO,		UNDEFINED_BINARY	},
	{"text/calendar",										"ics",				true, 	MIME_TEXT_CALENDAR,								MIME_TEXT_CALENDAR,						MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,		UNDEFINED_BINARY },

	/* 136 */
	{"text/vnd.wap.wmlscript",								"",					false,		MIME_TEXT_VND_WAP_WMLSCRIPT,					MIME_TEXT_VND_WAP_WMLSCRIPT,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x09	},
	{"text/vnd.wap.wml",									"wml",				false,		MIME_TEXT_VND_WAP_WML,							MIME_TEXT_VND_WAP_WML,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x08	},
	{"text/vnd.wap.wta-event",								"",					false,		MIME_TEXT_VND_WAP_WTA_EVENT,					MIME_TEXT_VND_WAP_WTA_EVENT,			MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x0a	},
	{"text/vnd.wap.connectivity-xml",						"",					false,		MIME_TEXT_VND_WAP_CONNECTIVITY_XML,				MIME_TEXT_VND_WAP_CONNECTIVITY_XML,		MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x35	},
	{"text/vnd.wap.si",									"",					false,		MIME_TEXT_VND_WAP_SI,							MIME_TEXT_VND_WAP_SI,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x2d	},
	{"text/vnd.wap.sl",									"",					false,		MIME_TEXT_VND_WAP_SL,							MIME_TEXT_VND_WAP_SL,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x2f	},
	{"text/vnd.wap.co",									"",					false,		MIME_TEXT_VND_WAP_CO,							MIME_TEXT_VND_WAP_CO,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x31	},
	{"text/vnd.sun.j2me.app-descriptor",					"jad",				true,		MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR,			MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR,	MIME_APPLICATION_NONE,						MIME_MAINTYPE_ETC,			UNDEFINED_BINARY	},


	/* 144 */
	{"text/x-hdml",										"",					false,		MIME_TEXT_X_HDML,								MIME_TEXT_X_HDML,						MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			0x04	},
	{"text/x-vCalendar",									"vcs",				true,		MIME_TEXT_X_VCALENDAR,							MIME_TEXT_X_VCALENDAR,					MIME_APPLICATION_NONE,					MIME_MAINTYPE_ETC,			0x06	},
	{"text/x-vCard",										"vcf",				true,		MIME_TEXT_X_VCARD,								MIME_TEXT_X_VCARD,						MIME_APPLICATION_NONE,					MIME_MAINTYPE_ETC,			0x07	},
	{"text/x-iMelody",										"imy",				true,		MIME_TEXT_X_IMELODY,							MIME_TEXT_X_IMELODY,					MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,		UNDEFINED_BINARY	},
	{"text/x-imelody",										"imy",				true,		MIME_TEXT_X_IMELODY2,							MIME_TEXT_X_IMELODY,					MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_AUDIO,		UNDEFINED_BINARY	},
	{"text/x-vnote",										"vnt",				true,		MIME_TEXT_X_VNOTE,								MIME_TEXT_X_VNOTE,						MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			UNDEFINED_BINARY	},
	{"text/x-vtodo",										"vts",				true,		MIME_TEXT_X_VTODO,								MIME_TEXT_X_VNOTE,						MIME_APPLICATION_NONE,					MIME_MAINTYPE_TEXT,			UNDEFINED_BINARY	},

	/* 151 */
	{"video/mpeg4",										"mp4",				true,		MIME_VIDEO_MPEG4,								MIME_VIDEO_MP4,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/mp4",											"mp4",				true,		MIME_VIDEO_MP4,									MIME_VIDEO_MP4,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/h263",											""/*,3gpp,mp4"*/,	true,		MIME_VIDEO_H263,								MIME_VIDEO_H263,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/3gpp",											"3gp"/*,3gpp"*/,	true,		MIME_VIDEO_3GPP,								MIME_VIDEO_3GPP,						MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/3gp",											"3gp"/*,3gpp"*/,	true,		MIME_VIDEO_3GP,									MIME_VIDEO_3GP,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/avi",											"avi",				false,		MIME_VIDEO_AVI,									MIME_VIDEO_AVI,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/sdp",											"sdp",				true,		MIME_VIDEO_SDP,									MIME_APPLICATION_SDP,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/mp4v-es",											"3gp",				true,		MIME_VIDEO_MP4_ES,									MIME_VIDEO_3GP,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/mpeg",											"3gp",				true,		MIME_VIDEO_MPEG,									MIME_VIDEO_3GP,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},
	{"video/quicktime",										"mov",				true,		MIME_VIDEO_MOV,									MIME_VIDEO_3GP,							MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,		UNDEFINED_BINARY	},

	/* 161 */
	{"video/vnd.rn-realvideo",								"rm",				true,		MIME_VIDEO_VND_RN_REALVIDEO,					MIME_VIDEO_VND_RN_REALVIDEO,			MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,			UNDEFINED_BINARY	},
	{"video/vnd.rn-realmedia",								"rm",				true,		MIME_VIDEO_VND_RN_REALMEDIA,					MIME_VIDEO_VND_RN_REALMEDIA,			MIME_APPLICATION_MEDIAPLAYER,			MIME_MAINTYPE_VIDEO,			UNDEFINED_BINARY	},

	/* 163 */
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
	56,			86,			87,
	106,		117,		120,
	121,		121,		121,
	122,		122,		122,
	129,		136,		144,
	151,		161,		163
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
	MIME_AUDIO_OGG,


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
	MIME_TEXT_CALENDAR,
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
	MIME_TEXT_X_VTODO,

	MIME_VIDEO_MPEG4	,
	MIME_VIDEO_MP4	,
	MIME_VIDEO_H263	,
	MIME_VIDEO_3GPP	,
	MIME_VIDEO_3GP	,
	MIME_VIDEO_AVI	,
	MIME_VIDEO_SDP	,
	MIME_VIDEO_MP4_ES,
	MIME_VIDEO_MPEG	,
	MIME_VIDEO_MOV	,
	MIME_VIDEO_VND_RN_REALVIDEO	,
	MIME_VIDEO_VND_RN_REALMEDIA	,
	MIME_VIDEO_X_MP4	,
	MIME_VIDEO_X_PV_MP4	,
	MIME_VIDEO_X_PN_REALVIDEO	,
	MIME_VIDEO_X_PN_MULTIRATE_REALVIDEO	,
	MIME_VIDEO_X_MS_WMV ,
	MIME_VIDEO_X_MS_ASF,
	MIME_VIDEO_X_PV_PVX ,

	MIME_UNKNOWN	/* MIME_MAX */
};


#define MIME_MAX_NUM	167
#define EXT_MAX	68
#define MIME_SUB_TYPE_VND	1
#define MIME_SUB_TYPE_X		2

int __MimeGetTableIndexInt(MimeType mime);
int __MimeGetTableIndexString(const char *szMime);
MimeMainType __MimeGetMainTypeName(const char *szType);

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

	/* If 'type' is zero, mimeTableIndex's array value have a minus value */
	/* If 'type' is zero, mime type is '\*\/\*' */
	if (type == 0)
		tableIndex = 0;
	else
		tableIndex = mimeTableIndex[(type - 1) * 3 + subtype] + index;

	return tableIndex;
}

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

	szMIMEType = (char *)malloc(strlen(szMime) + 1);

	if (szMIMEType == NULL) {
		MSG_DEBUG("szMime is NULL");
		return MIME_UNKNOWN;
	}

	memset(szMIMEType, 0, strlen(szMime) + 1);

	if (!strcmp(szMime, "*/*")) {
		free(szMIMEType);
		return 0;
	}

	strncpy(szMIMEType, szMime, strlen(szMime));
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


/* For Decode */
/* MimeString -> MimeInt */
MimeType MimeGetMimeIntFromMimeString(char *szMimeStr)
{
	int index = __MimeGetTableIndexString(szMimeStr);
	if (index == MIME_UNKNOWN) {
		return MIME_UNKNOWN;
	} else {
		return (MimeType)mimeTableEnum[index];
	}
}

/* MimeInt -> MimeString */
char *MimeGetMimeStringFromMimeInt(int mimeType)
{
	int idx = __MimeGetTableIndexInt((MimeType)mimeType);

	if (idx == MIME_UNKNOWN)
		return NULL;

	return (char *)mimeTable[idx].szMIME;
}


/* BinaryCode -> MimeInt */
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

/* Mimeint -> Binary Value */
int MimeGetBinaryValueFromMimeInt(MimeType mime)
{
	int index;

	index = __MimeGetTableIndexInt(mime);
	if (index == MIME_UNKNOWN)
		return UNDEFINED_BINARY;

	return mimeTable[index].binary;
}

typedef struct {
	const char *szExt;
	int mainType;
	int enumMime;
	const char *szContentType;
} ExtTableItem;

#define EXT_TABLE_SIZE (58)

const ExtTableItem extTable[EXT_TABLE_SIZE] = {
	/* text */
	{"txt",   MIME_MAINTYPE_TEXT, MIME_TEXT_PLAIN, "text/plain"},
	{"html",  MIME_MAINTYPE_TEXT, MIME_TEXT_HTML, "text/html"},
	{"xhtml", MIME_MAINTYPE_TEXT, MIME_APPLICATION_XHTML_XML, "application/xhtml+xml"},
	{"vcs",   MIME_MAINTYPE_TEXT, MIME_TEXT_X_VCALENDAR, "text/x-vCalendar"},
	{"vcf",   MIME_MAINTYPE_TEXT, MIME_TEXT_X_VCARD, "text/x-vCard"},
	{"vnt",   MIME_MAINTYPE_TEXT, MIME_TEXT_X_VNOTE, "text/x-vnote"},
	{"vts",   MIME_MAINTYPE_TEXT, MIME_TEXT_X_VTODO, "text/x-vtodo"},
	{"ics",   MIME_MAINTYPE_TEXT, MIME_TEXT_CALENDAR, "text/calendar"},

	/* image */
	{"gif",   MIME_MAINTYPE_IMAGE, MIME_IMAGE_GIF, "image/gif"},
	/* {"jpg",   MIME_MAINTYPE_IMAGE, MIME_IMAGE_JPG, "image/jpg"}, */
	{"jpg",   MIME_MAINTYPE_IMAGE, MIME_IMAGE_JPEG, "image/jpeg"},
	{"jpeg",  MIME_MAINTYPE_IMAGE, MIME_IMAGE_JPEG, "image/jpeg"},
	{"tiff",  MIME_MAINTYPE_IMAGE, MIME_IMAGE_TIFF, "image/tiff"},
	{"tif",   MIME_MAINTYPE_IMAGE, MIME_IMAGE_TIF, "image/tif"},
	{"png",   MIME_MAINTYPE_IMAGE, MIME_IMAGE_PNG, "image/png"},
	{"wbmp",  MIME_MAINTYPE_IMAGE, MIME_IMAGE_VND_WAP_WBMP, "image/vnd.wap.wbmp"},
	{"bmp",   MIME_MAINTYPE_IMAGE, MIME_IMAGE_BMP, "image/bmp"},
	{"svg",   MIME_MAINTYPE_IMAGE, MIME_IMAGE_SVG, "image/svg+xml"},

	/* audio */
	{"snd",   MIME_MAINTYPE_AUDIO, MIME_AUDIO_BASIC, "audio/basic"},
	{"amr",   MIME_MAINTYPE_AUDIO, MIME_AUDIO_AMR, "audio/amr"},
	{"m4a",   MIME_MAINTYPE_AUDIO, MIME_AUDIO_M4A, "audio/m4a"},
	{"3gp",   MIME_MAINTYPE_AUDIO, MIME_AUDIO_3GPP, "audio/3gpp"},
	{"3gpp",  MIME_MAINTYPE_AUDIO, MIME_AUDIO_3GPP, "audio/3gpp"},
	{"mp3",   MIME_MAINTYPE_AUDIO, MIME_AUDIO_MPEG, "audio/mpeg"},
	{"aac",   MIME_MAINTYPE_AUDIO, MIME_AUDIO_AAC, "audio/aac"},
	{"imy",   MIME_MAINTYPE_AUDIO, MIME_AUDIO_IMELODY, "audio/imelody"},
	{"mid",   MIME_MAINTYPE_AUDIO, MIME_AUDIO_MID, "audio/mid"},
	{"mmf",   MIME_MAINTYPE_AUDIO, MIME_AUDIO_MMF, "audio/mmf"},
	{"spm",   MIME_MAINTYPE_AUDIO, MIME_AUDIO_SP_MIDI, "audio/sp-midi"},
	{"wav",   MIME_MAINTYPE_AUDIO, MIME_AUDIO_WAV, "audio/wav"},
	{"mp4",   MIME_MAINTYPE_AUDIO, MIME_AUDIO_MP4, "audio/mp4"},
	{"wma",   MIME_MAINTYPE_AUDIO, MIME_AUDIO_X_MS_WMA, "audio/x-ms-wma"},
	{"rm",    MIME_MAINTYPE_AUDIO, MIME_AUDIO_VND_RN_REALAUDIO, "audio/vnd.rn-realaudio"},
	{"ra",    MIME_MAINTYPE_AUDIO, MIME_AUDIO_VND_RN_REALAUDIO, "audio/vnd.rn-realaudio"},
	{"mxmf",    MIME_MAINTYPE_AUDIO, MIME_AUDIO_MOBILE_XMF, "audio/mobile-xmf"},
	{"ogg",    MIME_MAINTYPE_AUDIO, MIME_AUDIO_OGG, "audio/ogg"},

	/* video */
	{"3gp",   MIME_MAINTYPE_VIDEO, MIME_VIDEO_3GPP, "video/3gpp"},
	{"3gpp",  MIME_MAINTYPE_VIDEO, MIME_VIDEO_3GPP, "video/3gpp"},
	{"mp4",   MIME_MAINTYPE_VIDEO, MIME_VIDEO_MP4, "video/mp4"},
	{"rm",    MIME_MAINTYPE_VIDEO, MIME_VIDEO_VND_RN_REALVIDEO, "video/vnd.rn-realvideo"},
	{"rv",    MIME_MAINTYPE_VIDEO, MIME_VIDEO_VND_RN_REALVIDEO, "video/vnd.rn-realvideo"},
	{"avi",   MIME_MAINTYPE_VIDEO, MIME_VIDEO_AVI, "video/avi"},
	{"asf",   MIME_MAINTYPE_VIDEO, MIME_VIDEO_X_MS_ASF, "video/x-ms-asf"},
	{"mov",	MIME_MAINTYPE_VIDEO, MIME_VIDEO_MOV, "video/quicktime"},

	/* application */
	{"smil",  MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_SMIL, "application/smil"},
	{"rm",    MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_VND_RN_REALMEDIA, "application/vnd.rn-realmedia"},
	{"ram",   MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_RAM, "application/ram"},
	{"ppt",   MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_VND_POWERPOINT, "application/vnd.ms-powerpoint"},
	{"xls",   MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_VND_EXCEL, "application/vnd.ms-excel"},
	{"doc",   MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_VND_MSWORD, "applcation/vnd.ms-word"},
	{"pdf",   MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_PDF, "application/pdf"},
	{"swf",   MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_X_FLASH, "application/x-shockwave-flash"},
	{"dm",    MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_VND_OMA_DRM_MESSAGE, "application/vnd.oma.drm.message"},
	{"dcf",   MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_VND_OMA_DRM_CONTENT, "application/vnd.oma.drm.content"},
	{"dd",    MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_VND_OMA_DD_XML, "application/vnd.oma.dd+xml"},
	{"ro",    MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_VND_OMA_DRM_RIGHTS_XML, "application/vnd.oma.drm.rights+xml"},
	{"ro",    MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_VND_OMA_DRM_RIGHTS_WBXML, "application/vnd.oma.drm.rights+wbxml"},
	{"oro",   MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_VND_OMA_DRM_RO_XML, "application/vnd.oma.drm.ro+xml"},
	{"odf",   MIME_MAINTYPE_APPLICATION, MIME_APPLICATION_VND_OMA_DRM_DCF, "application/vnd.oma.drm.dcf"},
};

/* GetMimeType from File Extension */
bool MsgGetMimeTypeFromExt(MimeMainType mainType, const char *pExt, MimeType *pMimeType, const char **ppszMimeType)
{
	const ExtTableItem *matchedItem = NULL;

	if (pExt == NULL)
		return false;

	if (pMimeType == NULL && ppszMimeType == NULL)
		return false;

	int extTableMainSize = sizeof(extTable)/sizeof(ExtTableItem);

	for (int i = 0; i < extTableMainSize; i++) {
		if (strcasecmp(extTable[i].szExt, pExt) == 0) {
			matchedItem = &extTable[i];

			if (mainType == extTable[i].mainType) {
				break;
			}
		}
	}

	if (matchedItem) {
		MSG_DEBUG("Found ext = [%s], mainType = [%d], mimeType = [0x%04x], ContentType = [%s]"
				, matchedItem->szExt
				, matchedItem->mainType
				, matchedItem->enumMime
				, matchedItem->szContentType);

		if (pMimeType)
			*pMimeType = (MimeType)matchedItem->enumMime;

		if (ppszMimeType)
			*ppszMimeType = matchedItem->szContentType;

		return true;
	}

	return false;
}

bool MsgGetMimeTypeFromFileName(MimeMainType mainType, const char *pFileName, MimeType *pMimeType, const char **ppszMimeType)
{
	const char *pExt = NULL;

	if (pFileName == NULL)
		return false;

	if (pMimeType == NULL && ppszMimeType == NULL)
		return false;

	pExt = strrchr(pFileName, '.');

	if (pExt == NULL || *(pExt + 1) == '\0')
		return false;

	pExt = pExt + 1;

	return MsgGetMimeTypeFromExt(mainType, pExt, pMimeType, ppszMimeType);
}

bool MsgGetMainTypeFromMetaData(const char *pFileName, MimeMainType *mainType)
{
	char *video_track_cnt = NULL;
	char *audio_track_cnt = NULL;

	if (pFileName == NULL)
		return false;

	int ret = METADATA_EXTRACTOR_ERROR_NONE;
	metadata_extractor_h metadata = NULL;

	ret = metadata_extractor_create(&metadata);
	if(ret != METADATA_EXTRACTOR_ERROR_NONE) {
		MSG_ERR("Fail metadata_extractor_create [%d]", ret);
		return false;
	}

	ret = metadata_extractor_set_path(metadata, pFileName);
	if(ret != METADATA_EXTRACTOR_ERROR_NONE) {
		MSG_ERR("Fail metadata_extractor_set_path [%d]", ret);
		metadata_extractor_destroy(metadata);
		return false;
	}

	metadata_extractor_get_metadata(metadata, METADATA_HAS_VIDEO, &video_track_cnt);
	if (video_track_cnt) {
		MSG_DEBUG("video_track_cnt = [%s]", video_track_cnt);
		if (atoi(video_track_cnt) > 0) {
			metadata_extractor_destroy(metadata);
			free(video_track_cnt);
			video_track_cnt = NULL;

			*mainType = MIME_MAINTYPE_VIDEO;
			return true;
		} else {
			free(video_track_cnt);
			video_track_cnt = NULL;
		}
	}

	metadata_extractor_get_metadata(metadata, METADATA_HAS_AUDIO, &audio_track_cnt);
	if (audio_track_cnt) {
		MSG_DEBUG("audio_track_cnt = [%s]", audio_track_cnt);
		if (atoi(audio_track_cnt) > 0) {
			metadata_extractor_destroy(metadata);
			free(audio_track_cnt);
			audio_track_cnt = NULL;

			*mainType = MIME_MAINTYPE_AUDIO;
			return true;
		} else {
			free(audio_track_cnt);
			audio_track_cnt = NULL;
		}
	}

	metadata_extractor_destroy(metadata);

	return false;
}
