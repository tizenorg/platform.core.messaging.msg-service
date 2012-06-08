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

#ifndef MSG_MMS_TYPES_H
#define MSG_MMS_TYPES_H

/**
 *	@file 		MsgMmsTypes.h
 *	@brief 		Defines MMS types of messaging framework
 *	@version 	1.0
 */

/**
 *	@section		Introduction
 *	- Introduction : Overview on MMS message Types
 *	@section		Program
 *	- Program : MMS message Types Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgTypes.h"


#include <glib.h>


/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_MMS_TYPES	Messaging MMS Types
 *	@{
 */

/*==================================================================================================
									DEFINES
==================================================================================================*/

/**
 *	@brief	Defines the enabled MMS version
 */
#define	MMS_V1_2								// MMS Version : MMS_V1_0 / MMS_V1_1 / MMS_V1_2

/**
 *	@brief	Defines the enabled DRM support
 */
#define	__SUPPORT_DRM__

/**
 *	@brief	Defines the enabled JAVA MMS Application Id
 */
#define FEATURE_JAVA_MMS

/**
 *	@brief	Defines the maximum file name length
 */
#define	MSG_FILENAME_LEN_MAX		255

/**
 *	@brief	Defines the maximum message id length
 */
#define	MSG_MSG_ID_LEN			100

/**
 *	@brief	Defines the maximum in transition id length
 */
#define MAX_SMIL_TRANSIN_ID		100

/**
 *	@brief	Defines the maximum out transition id length
 */
#define MAX_SMIL_TRANSOUT_ID	100

/**
 *	@brief	Defines the maximum region id length
 */
#define	MAX_SMIL_REGION_ID		151

/**
 *	@brief	Defines the maximum transition id length
 */
#define MAX_SMIL_TRANSITION_ID		151

/**
 *	@brief	Defines the maximum meta id length
 */
#define MAX_SMIL_META_ID		151

/**
 *	@brief	Defines the maximum meta name length
 */
#define MAX_SMIL_META_NAME		100

/**
 *	@brief	Defines the maximum meta content length
 */
#define MAX_SMIL_META_CONTENT		255

/**
 *	@brief	Defines the maximum fit size
 */
#define	MAX_SMIL_FIT_SIZE		31

/**
 *	@brief	Defines the maximum pages in a MMS
 */
#define	MMS_PAGE_MAX			20

/**
 *	@brief	Defines the maximum media in a MMS
 */
#define	MMS_MEDIA_MAX			60

/**
 *	@brief	Defines the maximum alternate text length
 */
#define	MAX_SMIL_ALT_LEN		255


//font size
/**
 *	@brief	Defines the small font size
 */
#define	MMS_SMIL_FONT_SIZE_SMALL 		24

/**
 *	@brief	Defines the normal font size
 */
#define	MMS_SMIL_FONT_SIZE_NORMAL		30

/**
 *	@brief	Defines the large font size
 */
#define	MMS_SMIL_FONT_SIZE_LARGE 		36

//#ifdef MMS_SMIL_ANIMATE
#define MAX_SMIL_ANIMATE_ATTRIBUTE_NAME 100
#define MAX_SMIL_ANIMATE_ATTRIBUTE_TYPE 100
#define MAX_SMIL_ANIMATE_TARGET_ELEMENT 100
#define MAX_SMIL_ANIMATE_CALC_MODE 100
//#endif

//#ifdef FEATURE_JAVA_MMS
#define MAX_MMS_JAVA_APPID_LEN 32
//#endif

#define CONV_THUMB_WIDTH	250
#define CONV_THUMB_HEIGHT 170

#define MMS_TR_ID_LEN		40
#define MMS_MSG_ID_LEN		40
#define MMS_LOCATION_LEN	100

/*==================================================================================================
                                         ENUMS
==================================================================================================*/

/**
 *	@brief	Represents the values of a mime type.
 */
typedef enum	_MimeType
{
	// 0
	MIME_ASTERISK											= 0x0000, /**< Indicates the valid default mime type  */

	// 1
	MIME_APPLICATION_XML									= 0x1000, /**< Indicates the application xml type */
	MIME_APPLICATION_WML_XML							= 0x1001,	/**< Indicates the application wml xml type  */
	MIME_APPLICATION_XHTML_XML							= 0x1002,	/**< Indicates the application xhtml xml type  */
	MIME_APPLICATION_JAVA_VM								= 0x1003, /**< Indicates the application java vm type */
	MIME_APPLICATION_SMIL								= 0x1004, /**< Indicates the application smil  type */
	MIME_APPLICATION_JAVA_ARCHIVE						= 0x1005, /**< Indicates the application java archive type */
	MIME_APPLICATION_JAVA								= 0x1006, /**< Indicates the application java  type */
	MIME_APPLICATION_OCTET_STREAM						= 0x1007, /**< Indicates the application octect stream type */
	MIME_APPLICATION_STUDIOM							= 0x1008, /**< Indicates the application studiom type */
	MIME_APPLICATION_FUNMEDIA							= 0x1009, /**< Indicates the application fun media type */
	MIME_APPLICATION_MSWORD								= 0x100a,	/**< Indicates the application ms word type */
	MIME_APPLICATION_PDF									= 0x100b, /**< Indicates the application pdf type */
	MIME_APPLICATION_SDP									= 0x100c,  /**< Indicates the application sdp type */
	MIME_APPLICATION_RAM									= 0x100d, /**< Indicates the application ram type */
	MIME_APPLICATION_ASTERIC								= 0x100e, /**< Indicates the application as main type and generic sub type */

	// 16
	MIME_APPLICATION_VND_WAP_XHTMLXML					= 0x1100, /**< Indicates the application wap xhtml xml type */
	MIME_APPLICATION_VND_WAP_WMLC						= 0x1101,  /**< Indicates the application wap wmlc type */
	MIME_APPLICATION_VND_WAP_WMLSCRIPTC				= 0x1102,  /**< Indicates the application wap wmlscrpitc type */
	MIME_APPLICATION_VND_WAP_WTA_EVENTC				= 0x1103, /**< Indicates the application wap  wta event type */
	MIME_APPLICATION_VND_WAP_UAPROF					= 0x1104, /**< Indicates the application wap uaprof type */
	MIME_APPLICATION_VND_WAP_SIC						= 0x1105,  /**< Indicates the application wap sic type */
	MIME_APPLICATION_VND_WAP_SLC						= 0x1106, /**< Indicates the application wap slc type */
	MIME_APPLICATION_VND_WAP_COC						= 0x1107, /**< Indicates the application wap coc type */
	MIME_APPLICATION_VND_WAP_SIA						= 0x1108, /**< Indicates the application wap sia type */
	MIME_APPLICATION_VND_WAP_CONNECTIVITY_WBXML		= 0x1109,  /**< Indicates the application wap connectivity wbxml type */
	MIME_APPLICATION_VND_WAP_MULTIPART_FORM_DATA	= 0x110a, /**< Indicates the application wap multipart data type */
	MIME_APPLICATION_VND_WAP_MULTIPART_BYTERANGES	= 0x110b, /**< Indicates the application wap multipart byte type */
	MIME_APPLICATION_VND_WAP_MULTIPART_MIXED			= 0x110c,  /**< Indicates the application wap multipart mixed type */
	MIME_APPLICATION_VND_WAP_MULTIPART_RELATED		= 0x110d,  /**< Indicates the application wap multipart related type */
	MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE	= 0x110e,  /**< Indicates the application wap  multipart alternative type */
	MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC		= 0x110f, /**< Indicates the application wap mulitpart as main type and generic sub type */
	MIME_APPLICATION_VND_WAP_WBXML					= 0x1110, /**< Indicates the application wap wbxml type */
	MIME_APPLICATION_VND_OMA_DD_XML					= 0x1111, /**< Indicates the application oma dd xml type */
	MIME_APPLICATION_VND_OMA_DRM_MESSAGE				= 0x1112,  /**< Indicates the application oma drm message type */
	MIME_APPLICATION_VND_OMA_DRM_CONTENT				= 0x1113, /**< Indicates the application oma drm content type */
	MIME_APPLICATION_VND_OMA_DRM_RIGHTS_XML			= 0x1114, /**< Indicates the application oma drm rights xml type */
	MIME_APPLICATION_VND_OMA_DRM_RIGHTS_WBXML		= 0x1115,  /**< Indicates the application oma drm rights wbxml type */
	MIME_APPLICATION_VND_OMA_DRM_RO_XML				= 0x1116, /**< Indicates the application oma drm ro xml type */
	MIME_APPLICATION_VND_OMA_DRM_DCF					= 0x1117, /**< Indicates the application oma drm dcf type */
	MIME_APPLICATION_VND_OMA_ROAPPDU_XML				= 0x1118,  /**< Indicates the application oma roap pdu xml type */
	MIME_APPLICATION_VND_OMA_ROAPTRIGGER_XML			= 0x1119,	/**< Indicates the application oma roap trigger xml  type */
	MIME_APPLICATION_VND_SMAF							= 0x111a,  /**< Indicates the application smaf type */
	MIME_APPLICATION_VND_RN_REALMEDIA					= 0x111b,  /**< Indicates the application rn real media type */
	MIME_APPLICATION_VND_SUN_J2ME_JAVA_ARCHIVE		= 0x111c,  /**< Indicates the application j2me java archive type */
	MIME_APPLICATION_VND_EXCEL							= 0x111e,  /**< Indicates the application excel type */
	MIME_APPLICATION_VND_POWERPOINT					= 0x111f,  /**< Indicates the application power point type */
	MIME_APPLICATION_VND_MSWORD						= 0x1120,	 /**< Indicates the application ms word type */

	// 49
	MIME_APPLICATION_X_HDMLC							= 0x1200, /**< Indicates the application x hdmlc type */
	MIME_APPLICATION_X_X968_USERCERT					= 0x1201, /**< Indicates the application x x968 user certified type */
	MIME_APPLICATION_X_WWW_FORM_URLENCODED			= 0x1202, /**< Indicates the application x www form url encoded type */
	MIME_APPLICATION_X_SMAF								= 0x1203, /**< Indicates the application x smaf type */
	MIME_APPLICATION_X_FLASH								= 0x1204, /**< Indicates the application x flash type */
	MIME_APPLICATION_X_EXCEL								= 0x1205, /**< Indicates the application x excel type */
	MIME_APPLICATION_X_POWERPOINT						= 0x1206, /**< Indicates the application x power point type */

	// 56
	MIME_AUDIO_BASIC										= 0x2000, /**< Indicates the audio basic type  */
	MIME_AUDIO_MPEG										= 0x2001, /**< Indicates the audio mpeg type  */
	MIME_AUDIO_MP3										= 0x2002, /**< Indicates the audio mp3 type  */
	MIME_AUDIO_MPG3										= 0x2003,  /**< Indicates the audio mpg3 type  */
	MIME_AUDIO_MPEG3										= 0x2004, /**< Indicates the audio mpeg3 type  */
	MIME_AUDIO_MPG										= 0x2005, /**< Indicates the audio mpg type  */
	MIME_AUDIO_AAC										= 0x2006, /**< Indicates the audio aac type  */
	MIME_AUDIO_G72										= 0x2007, /**< Indicates the audio g72 type  */
	MIME_AUDIO_AMR										= 0x2008, /**< Indicates the audio amr type  */
	MIME_AUDIO_AMR_WB									= 0x2009, /**< Indicates the audio amr wb type  */
	MIME_AUDIO_MMF										= 0x200a, /**< Indicates the audio mmf type  */
	MIME_AUDIO_SMAF										= 0x200b, /**< Indicates the audio smaf type  */
	MIME_AUDIO_IMELODY									= 0x200c, /**< Indicates the audio imelody type  */
	MIME_AUDIO_IMELODY2									= 0x200d, /**< Indicates the audio imelody2 type  */
	MIME_AUDIO_MELODY									= 0x200e, /**< Indicates the audio melody type  */
	MIME_AUDIO_MID										= 0x200f, /**< Indicates the audio mid type  */
	MIME_AUDIO_MIDI										= 0x2010, /**< Indicates the audio midi type  */
	MIME_AUDIO_SP_MIDI									= 0x2011, /**< Indicates the audio sp midi type  */
	MIME_AUDIO_WAVE										= 0x2012, /**< Indicates the audio wave type  */
	MIME_AUDIO_WAV										= 0x2013, /**< Indicates the audio wav type  */
	MIME_AUDIO_3GPP										= 0x2014, /**< Indicates the audio 3gpp type  */
	MIME_AUDIO_MP4										= 0x2015, /**< Indicates the audio mp4 type  */
	MIME_AUDIO_MP4A_LATM								= 0x2016, /**< Indicates the audio mp4 latm type  */
	MIME_AUDIO_M4A										= 0x2017, /**< Indicates the audio m4a type  */
	MIME_AUDIO_MPEG4										= 0x2018, /**< Indicates the audio mpeg4 type  */
	MIME_AUDIO_WMA										= 0x2019, /**< Indicates the audio wma type  */
	MIME_AUDIO_XMF										= 0x201a, /**< Indicates the audio xmf type  */
	MIME_AUDIO_IMY										= 0x201b, /**< Indicates the audio imy type  */
	MIME_AUDIO_MOBILE_XMF								= 0x201c, /**< Indicates the audio mobile xmf type  */

	//85
	MIME_AUDIO_VND_RN_REALAUDIO						= 0x2100, /**< Indicates the audio rn real audio type  */

	// 86
	MIME_AUDIO_X_MPEG									= 0x2200, /**< Indicates the audio x mpeg type  */
	MIME_AUDIO_X_MP3										= 0x2201, /**< Indicates the audio x mp3 type  */
	MIME_AUDIO_X_MPEG3									= 0x2202,    /**< Indicates the audio x mpeg3 type  */
	MIME_AUDIO_X_MPG										= 0x2203, /**< Indicates the audio x mpg type  */
	MIME_AUDIO_X_AMR										= 0x2204, /**< Indicates the audio x amr type  */
	MIME_AUDIO_X_MMF										= 0x2205, /**< Indicates the audio x mmf type  */
	MIME_AUDIO_X_SMAF									= 0x2206, /**< Indicates the audio x smaf type  */
	MIME_AUDIO_X_IMELODY									= 0x2207, /**< Indicates the audio x imelody type  */
	MIME_AUDIO_X_MIDI										= 0x2208, /**< Indicates the audio x midi type  */
	MIME_AUDIO_X_MPEGAUDIO								= 0x2209, /**< Indicates the audio x mpeg  type  */
	MIME_AUDIO_X_PN_REALAUDIO							= 0x220a, /**< Indicates the audio x pn real audio type  */
	MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO				= 0x220b, /**< Indicates the audio x pn multirate real audio  type  */
	MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO_LIVE			= 0x220c, /**< Indicates the audio x pn multirate real audio live type  */
	MIME_AUDIO_X_WAVE									= 0x220d, /**< Indicates the audio x wave  type  */
	MIME_AUDIO_X_WAV										= 0x220e, /**< Indicates the audio x wav  type  */
	MIME_AUDIO_X_MS_WMA									= 0x220f, /**< Indicates the audio ms wma type  */
	MIME_AUDIO_X_MID										= 0x2210, /**< Indicates the audio mid type  */
	MIME_AUDIO_X_MS_ASF									= 0x2211, /**< Indicates the audio ms asf  type  */
	MIME_AUDIO_X_XMF										= 0x2212, /**< Indicates the audio x xmf  type  */

	// 105
	MIME_IMAGE_GIF										= 0x3000, /**< Indicates the image gif type  */
	MIME_IMAGE_JPEG										= 0x3001, /**< Indicates the image jpeg type  */
	MIME_IMAGE_JPG										= 0x3002, /**< Indicates the image jpg type  */
	MIME_IMAGE_TIFF										= 0x3003, /**< Indicates the image tiff type  */
	MIME_IMAGE_TIF										= 0x3004, /**< Indicates the image tif type  */
	MIME_IMAGE_PNG										= 0x3005, /**< Indicates the image png type  */
	MIME_IMAGE_WBMP										= 0x3006, /**< Indicates the image wbmp type  */
	MIME_IMAGE_PJPEG										= 0x3007, /**< Indicates the image pjpeg type  */
	MIME_IMAGE_BMP										= 0x3008, /**< Indicates the image bmp type  */
	MIME_IMAGE_SVG										= 0x3009, /**< Indicates the image svg type  */
	MIME_IMAGE_SVG1										= 0x300a, /**< Indicates the image svg1 type  */

	// 116
	MIME_IMAGE_VND_WAP_WBMP							= 0x3100,  /**< Indicates the image vnd wap wbmp type  */

	// 119
	MIME_IMAGE_X_BMP										= 0x3200, /**< Indicates the image x bmp type  */

	// 120
	MIME_MESSAGE_RFC822									= 0x4000,  /**< Indicates the message rfc822 type  */

	// 121
	MIME_MULTIPART_MIXED									= 0x5000,  /**< Indicates the multipart mixed type  */
	MIME_MULTIPART_RELATED								= 0x5001, /**< Indicates the multipart related type  */
	MIME_MULTIPART_ALTERNATIVE							= 0x5002, /**< Indicates the multipart alternative type  */
	MIME_MULTIPART_FORM_DATA							= 0x5003, /**< Indicates the multipart form data type  */
	MIME_MULTIPART_BYTERANGE							= 0x5004, /**< Indicates the multipart byte range type  */
	MIME_MULTIPART_REPORT								= 0x5005, /**< Indicates the multipart report type  */
	MIME_MULTIPART_VOICE_MESSAGE						= 0x5006, /**< Indicates the multipart voice message type  */

	// 128
	MIME_TEXT_TXT											= 0x6000, /**< Indicates the text txt type  */
	MIME_TEXT_HTML										= 0x6001, /**< Indicates the text html type  */
	MIME_TEXT_PLAIN										= 0x6002,  /**< Indicates the text plain type  */
	MIME_TEXT_CSS											= 0x6003,  /**< Indicates the text css type  */
	MIME_TEXT_XML											= 0x6004,  /**< Indicates the text xml type  */
	MIME_TEXT_IMELODY										= 0x6005,  /**< Indicates the text imelody type  */

	// 134
	MIME_TEXT_VND_WAP_WMLSCRIPT						= 0x6100, /**< Indicates the text wap wmlscript  type  */
	MIME_TEXT_VND_WAP_WML								= 0x6101, /**< Indicates the text wap wml  type  */
	MIME_TEXT_VND_WAP_WTA_EVENT						= 0x6102, /**< Indicates the text wap wta event  type  */
	MIME_TEXT_VND_WAP_CONNECTIVITY_XML				= 0x6103, /**< Indicates the text wap connectivity xml  type  */
	MIME_TEXT_VND_WAP_SI									= 0x6104,  /**< Indicates the text wap si  type  */
	MIME_TEXT_VND_WAP_SL								= 0x6105, /**< Indicates the text wap sl  type  */
	MIME_TEXT_VND_WAP_CO								= 0x6106, /**< Indicates the text wap co  type  */
	MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR				= 0x6107, /**< Indicates the text sun j2me type  */

	// 142
	MIME_TEXT_X_HDML										= 0x6200, /**< Indicates the x html  type  */
	MIME_TEXT_X_VCALENDAR								= 0x6201,  /**< Indicates the x calendar  type  */
	MIME_TEXT_X_VCARD									= 0x6202, /**< Indicates the x vcard  type  */
	MIME_TEXT_X_IMELODY									= 0x6203, /**< Indicates the x imelody  type  */
	MIME_TEXT_X_IMELODY2									= 0x6204, /**< Indicates the x imelody2  type  */
	MIME_TEXT_X_VNOTE									= 0x6205,  /**< Indicates the x vnote  type  */

	// 148
	MIME_VIDEO_MPEG4										= 0x7000, /**< Indicates the mpeg4  type  */
	MIME_VIDEO_MP4										= 0x7001, /**< Indicates the mp4  type  */
	MIME_VIDEO_H263										= 0x7002, /**< Indicates the h263  type  */
	MIME_VIDEO_3GPP										= 0x7003, /**< Indicates the 3gpp  type  */
	MIME_VIDEO_3GP										= 0x7004, /**< Indicates the 3gp  type  */
	MIME_VIDEO_AVI										= 0x7005, /**< Indicates the avi  type  */
	MIME_VIDEO_SDP										= 0x7006, /**< Indicates the sdp  type  */
	MIME_VIDEO_MP4_ES									= 0x7007, /**< Indicates the mp4 es  type  */
	MIME_VIDEO_MPEG										= 0x7008, /**< Indicates the mpeg  type  */

	// 157
	MIME_VIDEO_VND_RN_REALVIDEO							= 0x7100, /**< Indicates the pn real video type  */
	MIME_VIDEO_VND_RN_REALMEDIA							= 0x7101, /**< Indicates the pn multi rate real media type  */

	// 159
	MIME_VIDEO_X_MP4										= 0x7200, /**< Indicates the video x mp4 type  */
	MIME_VIDEO_X_PV_MP4									= 0x7201, /**< Indicates the video x pv mp4 type  */
	MIME_VIDEO_X_PN_REALVIDEO							= 0x7202, /**< Indicates the x pn real video type  */
	MIME_VIDEO_X_PN_MULTIRATE_REALVIDEO				= 0x7203, /**< Indicates the x pn multi rate real video type  */
	MIME_VIDEO_X_MS_WMV									= 0x7204, /**< Indicates the x ms wmv type  */
	MIME_VIDEO_X_MS_ASF									= 0x7205, /**< Indicates the x ms asf type  */
	MIME_VIDEO_X_PV_PVX									= 0x7206, /**< Indicates the x pv pvx type  */

	MIME_TYPE_VALUE_MAX									= 0x7207,		/**< Indicates the maximum mime type  */
	MIME_UNKNOWN											= 0xffff	/**< Indicates the unknown mime type  */

} MimeType;

/**
 *	@brief	Represents the values of a DRM type.
 */
//#ifdef __SUPPORT_DRM__
typedef enum
{
	MSG_DRM_TYPE_NONE	= 0, /**< Indicates the drm type none */
	MSG_DRM_TYPE_FL		= 1, /**< Indicates the forward lock */		/* 2004-07-09: forwardLock type */
	MSG_DRM_TYPE_CD		= 2, /**< Indicates the combined delivery */	/* 2004-07-09: combined delivery type */
	MSG_DRM_TYPE_SD		= 3, /**< Indicates the separate delivery */	/* 2004-07-09: seperate delivery type */
	MSG_DRM_TYPE_SSD	= 4	/**< Indicates the special separate delivery */	// 2005-02-28: add Special Sperate Delivery
}MsgDrmType;
//#endif

/**
 *	@brief	Represents the values of a SMIL region type.
 */
typedef enum _REGION_FIT_TYPE_T
{
	MMSUI_IMAGE_REGION_FIT_HIDDEN,	 /**< Indicates the hidden fit type */
	MMSUI_IMAGE_REGION_FIT_MEET,	 /**< Indicates the meet fit type */
}REGION_FIT_TYPE_T;


/**
 *	@brief	Represents the values of a SMIL media type.
 */
typedef enum
{
	MMS_SMIL_MEDIA_INVALID = 0, /**< Indicates the invalid media type */
	MMS_SMIL_MEDIA_IMG,			/**< Indicates the image media */
	MMS_SMIL_MEDIA_AUDIO,		/**< Indicates the audio media */
	MMS_SMIL_MEDIA_VIDEO,		/**< Indicates the video media */
	MMS_SMIL_MEDIA_TEXT,		/**< Indicates the text media */
	MMS_SMIL_MEDIA_ANIMATE,		/**< Indicates the animation media */
	MMS_SMIL_MEDIA_IMG_OR_VIDEO	, /**< Indicates the image or video media */
	MMS_SMIL_MEDIA_MAX = 0xffffffff,	/**< Indicates the maximum media type */
}MmsSmilMediaType;

/**
 *	@brief	Represents the values of a SMIL transition type.
 */
typedef enum
{
	MMS_SMIL_TRANS_NONE = 0,	 	/**< Indicates the transition type none */			/* default */
	MMS_SMIL_TRANS_SLIDEWIPE = 1,	/**< Indicates the slide wipe transition */
	MMS_SMIL_TRANS_BARWIPE = 2,		/**< Indicates the bar wipe transition */
	MMS_SMIL_TRANS_BARNDOORWIPE = 3, /**< Indicates the bar and door wipe transition */
	MMS_SMIL_TRANS_FADE = 4,		/**< Indicates the fade transition */
	MMS_SMIL_TRANS_RANDOMBLOCK = 5,	/**< Indicates the random block transition */
	MMS_SMIL_TRANS_ZOOMIN = 6,		/**< Indicates the zoom in transition */
	MMS_SMIL_TRANS_IRISWIPE = 7,	/**< Indicates the iris wipe transition */
 	MMS_SMIL_TRANS_BOXWIPE = 8,		/**< Indicates the box wipe transition */
	MMS_SMIL_TRANS_FOURBOXWIPE = 9,	/**< Indicates the four box wipe transition */
	MMS_SMIL_TRANS_PUSHWIPE  =10,	/**< Indicates the push wipe transition */
	MMS_SMIL_TRANS_ELLIPSEWIPE  = 11 /**< Indicates the ellipse wipe transition */
}MmsSmilTransType;

/**
 *	@brief	Represents the values of a SMIL transition sub type.
 */
typedef enum
{
	MMS_SMIL_TRANS_SUB_NONE = 0,	/**< Indicates the transition sub type none */
	MMS_SMIL_TRANS_SUB_FROM_LEFT = 1,	/**< Indicates the from left transition */	/* slideWipe's default */
	MMS_SMIL_TRANS_SUB_FROM_TOP = 2,	/**< Indicates the from top transition */
	MMS_SMIL_TRANS_SUB_FROM_BOTTOM = 3,	/**< Indicates the from bottom transition */
	MMS_SMIL_TRANS_SUB_TOP_TO_BOTTOM = 4, /**< Indicates the from top to bottom transition */			/* barWipe's default */
	MMS_SMIL_TRANS_SUB_BOTTOM_TO_TOP = 5, /**< Indicates the from bottom to top transition */
	MMS_SMIL_TRANS_SUB_HORIZONTAL = 6,	/**< Indicates the horizontal transition */		/* barDoorWipe's default */
    MMS_SMIL_TRANS_SUB_FROM_RIGHT = 7, /**< Indicates the from right transition */
    MMS_SMIL_TRANS_SUB_VERTICAL = 8 /**< Indicates the vertical transition */
}MmsSmilTransSubType;

/**
 *	@brief	Represents the values of a text font type.
 */
typedef enum
{
	MMS_SMIL_FONT_TYPE_NONE = 0, /**< Indicates the font type none */
	MMS_SMIL_FONT_TYPE_NORMAL = 1, /**< Indicates the font type normal */
	MMS_SMIL_FONT_TYPE_ITALIC = 2, /**< Indicates the font type italic */
	MMS_SMIL_FONT_TYPE_BOLD = 3, /**< Indicates the font type bold */
	MMS_SMIL_FONT_TYPE_UNDERLINE = 4 /**< Indicates the font type underline */
}MmsSmilFontType;

/**
 *	@brief	Represents the values of a MMS text direction.
 */
typedef enum	_MmsTextDir{
	MMS_TEXT_DIRECTION_INVALID = -1, /**< Indicates the invalid direction */
	MMS_TEXT_DIRECTION_RIGHT = 0,	/**< Indicates the right direction */
	MMS_TEXT_DIRECTION_DOWN,		/**< Indicates the down direction */		// supported to GC
} MmsTextDirection;

/**
 *	@brief	Represents the values of MMS Read Report Sent Status.
 */
typedef enum
{
	MMS_RECEIVE_READ_REPORT_NO_SEND,	// didn't send yet
	MMS_RECEIVE_READ_REPORT_MUST_SEND,	// didn't send yet but will send
	MMS_RECEIVE_READ_REPORT_SENT,		// sent
	MMS_RECEIVE_READ_REPORT_NO_SENT,	// required but, didn't send by user's choice
} MmsRecvReadReportSendStatus ;

/**
 *	@brief	Represents text information.
 */
typedef struct  {
	char			szTransInId[MAX_SMIL_TRANSIN_ID];  /**< Indicates the In SMIL transition id */
	char			szTransOutId[MAX_SMIL_TRANSOUT_ID]; /**< Indicates the Out SMIL transition id */
	int				nRepeat; /**< Indicates the text needs to be displayed repeatedly */
	int				nBegin;  /**< Indicates the begin time */
	int				nEnd;	/**< Indicates the end time */
	int				nDurTime; /**< Indicates the duration */
	int				nBgColor; /**< Indicates the background color of the text */
	bool			bBold;	/**< Indicates whether the text is bold */
	bool			bUnderLine; /**< Indicates whether the text is underlined */
	bool			bItalic;		/**< Indicates whether the text is Italic */
	bool			bReverse;	/**< Indicates whether the text is reversed */
	MmsTextDirection	nDirection; /**< Indicates the text direction type. see enum MmsTextDirection */
	//MmsSmilFontType 	nFont;  /**< Indicates the text font type. see enum MmsSmilFontType */
	int				nSize;	/**< Indicates the font size */
	int				nColor; /**< Indicates the font color */
}MmsSmilText;

/**
 *	@brief	Represents video information.
 */
typedef struct {
	char			szTransInId[MAX_SMIL_TRANSIN_ID]; /**< Indicates the In SMIL transition id */
	char			szTransOutId[MAX_SMIL_TRANSOUT_ID];  /**< Indicates the Out SMIL transition id */
	int				nRepeat; /**< Indicates the video needs to be displayed repeatedly */
	int				nBegin;	 /**< Indicates the begin time */
	int				nEnd;	/**< Indicates the end time */
	int				nDurTime;  /**< Indicates the duration */
	int				nBgColor;  /**< Indicates the background color of the text */
}MmsSmilAVI;

/**
 *	@brief	Represents media information.
 */
typedef struct
{
	MmsSmilMediaType	mediatype; /**< Indicates the SMIL media type. see enum MmsSmilMediaType */

	char			szSrc[MSG_FILEPATH_LEN_MAX];/**< Indicates the media source name */
	char			szFileName[MSG_FILENAME_LEN_MAX]; /**< Indicates the file name */
	char			szFilePath[MSG_FILEPATH_LEN_MAX]; /**< Indicates the file path */
	char			szContentID[MSG_MSG_ID_LEN+1]; /**< Indicates the content id */
	char			regionId[MAX_SMIL_REGION_ID]; /**< Indicates the region id */
	char			szAlt[MAX_SMIL_ALT_LEN]; /**< Indicates the alternative text to be displayed in failure case */
	MsgDrmType		drmType; /**< Indicates the drm type. see enum MsgDrmType */
	char			szDrm2FullPath[MSG_FILEPATH_LEN_MAX];  /**< Indicates the fullpath of the DRM */
	union{
		MmsSmilText	sText;  /**< Indicates the text attributes */
		MmsSmilAVI	sAVI; /**< Indicates the video attributes */
	} sMedia;
}MMS_MEDIA_S;

/**
 *	@brief	Represents attachment information.
 */
typedef struct
{
	MimeType	mediatype;	/**< Indicates the file mime type. see enum MimeType */
	char		szFileName[MSG_FILENAME_LEN_MAX]; /**< Indicates the file name */
	char		szFilePath[MSG_FILEPATH_LEN_MAX]; /**< Indicates the file path */
	int		fileSize;	 /**< Indicates the size of the file */
	MsgDrmType	drmType; /**< Indicates the drm type. see enum MsgDrmType */
	char		szDrm2FullPath[MSG_FILEPATH_LEN_MAX]; /**< Indicates the fullpath of the DRM */
}MMS_ATTACH_S;

/**
 *	@brief	Represents SMIL page information.
 */
typedef struct
{
	int		mediaCnt;	/**< The count of the media */
	GList 	*medialist;	/**< The pointer to media list */
	int		nDur;	/**< Indicates the duration of the page */
	int		nBegin; /**< Indicates the begin time of the page */
	int		nEnd;	 /**< Indicates the end time of the page */
	int		nMin;	/**< Indicates the min attribute of the page */
	int		nMax;	/**< Indicates the max attribute of the page */
	int		nRepeat;	/**< Indicates the page needs to be displayed repeatedly */

}MMS_PAGE_S;

/**
 *	@brief	Represents length information.
 */
typedef struct
{
	bool 	bUnitPercent; /**< Indicates the length is in percentage(%) or not */
	int	value;	/**< Indicates the value for length */
}MMS_LENGTH;

/**
 *	@brief	Represents SMIL region information.
 */
typedef struct
{
	char				szID[MAX_SMIL_REGION_ID]; /**< Indicates the ID of region information */
	MMS_LENGTH		nLeft; /**< Indicates the left co-ordinate of the region */
	MMS_LENGTH		nTop; /**< Indicates the top co-ordinate of the region */
	MMS_LENGTH		width; /**< Indicates the width of the region */
	MMS_LENGTH		height; /**< Indicates the width of the region */ // '%' rate should be supported
	int				bgColor;	/**< Indicates the background color of the region */
	REGION_FIT_TYPE_T	fit;	/**< Indicates the fit type. see enum REGION_FIT_TYPE_T */

}MMS_SMIL_REGION;

/**
 *	@brief	Represents SMIL root layout information.
 */
typedef struct
{
	MMS_LENGTH	width;		/**< Indicates the width of the root layout */
	MMS_LENGTH	height;		/**< Indicates the height of the root layout */ // '%' rate should be supported
	int			bgColor;		/**< Indicates the background color of the root layout */
}MMS_SMIL_ROOTLAYOUT;


/**
 *	@brief	Represents SMIL transition information.
 */
typedef struct
{
	char					szID[MAX_SMIL_TRANSITION_ID];	/**< Indicates the ID of transition information */
	MmsSmilTransType		nType;					/**< Indicates the transition type. see enum MmsSmilTransType */
	MmsSmilTransSubType	nSubType;				/**< Indicates the transition sub type. see enum MmsSmilTransSubType */
	int					nDur;					/**< Indicates the transition duration */
}MMS_SMIL_TRANSITION;


/**
 *	@brief	Represents SMIL meta information.
 */
typedef struct
{
	char		szID[MAX_SMIL_META_ID];				/**< Indicates the ID of meta information */
	char		szName[MAX_SMIL_META_NAME];		/**< Indicates the Name */
	char		szContent[MAX_SMIL_META_CONTENT];	/**< Indicates the content */
}MMS_SMIL_META;


/**
 *	@brief	Represents application id information for JAVA MMS msg.
 */
typedef struct
{
	bool 			valid;										/**< Indicates whether application id information is used or not. */
	char 			appId[MAX_MMS_JAVA_APPID_LEN+1];			/**< application id, it should not exceed 32 chars */
	char 			replyToAppId[MAX_MMS_JAVA_APPID_LEN+1];	/**< reply to application id, application id, it should not exceeded 32 chars */
}MMS_APPID_INFO_S;



#define		MAX_FULL_PATH_SIZE_S	160	// max length for internal file path

typedef struct
{
	char					szMsgID[MMS_MSG_ID_LEN+1];
	char					retrievedFilePath[MAX_FULL_PATH_SIZE_S];
	char					szTrID[MMS_TR_ID_LEN+1];
	MMS_APPID_INFO_S	msgAppId;
}MMS_RECV_DATA_S;


/**
 *	@brief	Represents MMS message data.
 */
typedef struct _MMS_MESSAGE_DATA_S
{
	char					szSmilFilePath[MSG_FILEPATH_LEN_MAX];	/**< Indicates the SMIL file path */
	int						pageCnt;	/**< The count of the SMIL pages */
	GList					*pagelist;	/**< The pointer to SMIL pages list */
	int						regionCnt;	/**< The count of the SMIL regions */
	GList 					*regionlist;	/**< The pointer to SMIL regions list */
	int						attachCnt;	/**< The count of the attachments */
	GList 					*attachlist;	/**< The pointer to attachment list */
	int						transitionCnt;	/**< The count of the SMIL transitions information */
	GList 					*transitionlist;	/**< The pointer to SMIL transitions list */
	int						metaCnt;	/**< The count of the SMIL meta information */
	GList 					*metalist;	/**< The pointer to SMIL meta list */
	MMS_SMIL_ROOTLAYOUT		rootlayout;	/**< Indicates the root layout information */
	MMS_APPID_INFO_S 		msgAppId;
}MMS_MESSAGE_DATA_S;

/**
 *	@}
 */



#endif

