/*
 * msg-service
 *
 * Copyright (c) 2000 - 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
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
                                         STRUCTURES
==================================================================================================*/

/**
 *	@brief	Represents text information.
 */
typedef struct  {
	char			szTransInId[MAX_SMIL_TRANSIN_ID + 1];  /**< Indicates the In SMIL transition id */
	char			szTransOutId[MAX_SMIL_TRANSOUT_ID + 1]; /**< Indicates the Out SMIL transition id */
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
	int				nSize;	/**< Indicates the font size */
	int				nColor; /**< Indicates the font color */
}MmsSmilText;

/**
 *	@brief	Represents video information.
 */
typedef struct {
	char			szTransInId[MAX_SMIL_TRANSIN_ID + 1]; /**< Indicates the In SMIL transition id */
	char			szTransOutId[MAX_SMIL_TRANSOUT_ID + 1];  /**< Indicates the Out SMIL transition id */
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

	char			szSrc[MSG_FILEPATH_LEN_MAX + 1];/**< Indicates the media source name */
	char			szFileName[MSG_FILENAME_LEN_MAX + 1]; /**< Indicates the file name */
	char			szFilePath[MSG_FILEPATH_LEN_MAX + 1]; /**< Indicates the file path */
	char			szContentID[MSG_MSG_ID_LEN + 1]; /**< Indicates the content id */
	char			regionId[MAX_SMIL_REGION_ID + 1]; /**< Indicates the region id */
	char			szAlt[MAX_SMIL_ALT_LEN + 1]; /**< Indicates the alternative text to be displayed in failure case */
	MsgDrmType		drmType; /**< Indicates the drm type. see enum MsgDrmType */
	char			szDrm2FullPath[MSG_FILEPATH_LEN_MAX + 1];  /**< Indicates the fullpath of the DRM */
	union{
		MmsSmilText	sText;  /**< Indicates the text attributes */
		MmsSmilAVI	sAVI; /**< Indicates the video attributes */
	} sMedia;

	char szContentType[MSG_MSG_ID_LEN + 1];
	char szContentLocation[MSG_MSG_ID_LEN + 1];

}MMS_MEDIA_S;

/**
 *	@brief	Represents attachment information.
 */
typedef struct
{
	MimeType	mediatype;	/**< Indicates the file mime type. see enum MimeType */
	char		szFileName[MSG_FILENAME_LEN_MAX + 1]; /**< Indicates the file name */
	char		szFilePath[MSG_FILEPATH_LEN_MAX + 1]; /**< Indicates the file path */
	int		fileSize;	 /**< Indicates the size of the file */
	MsgDrmType	drmType; /**< Indicates the drm type. see enum MsgDrmType */
	char		szDrm2FullPath[MSG_FILEPATH_LEN_MAX + 1]; /**< Indicates the fullpath of the DRM */
	char szContentType[MSG_MSG_ID_LEN + 1];

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
	char				szID[MAX_SMIL_REGION_ID + 1]; /**< Indicates the ID of region information */
	MMS_LENGTH		nLeft; /**< Indicates the left co-ordinate of the region */
	MMS_LENGTH		nTop; /**< Indicates the top co-ordinate of the region */
	MMS_LENGTH		width; /**< Indicates the width of the region */
	MMS_LENGTH		height; /**< Indicates the width of the region */ // '%' rate should be supported
	bool			bBgColor;	/**< Indicates the background color set in the region */
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
	bool		bBgColor;	/**< Indicates the background color set in the root layout */
	int			bgColor;		/**< Indicates the background color of the root layout */
}MMS_SMIL_ROOTLAYOUT;


/**
 *	@brief	Represents SMIL transition information.
 */
typedef struct
{
	char					szID[MAX_SMIL_TRANSITION_ID + 1];	/**< Indicates the ID of transition information */
	MmsSmilTransType		nType;					/**< Indicates the transition type. see enum MmsSmilTransType */
	MmsSmilTransSubType	nSubType;				/**< Indicates the transition sub type. see enum MmsSmilTransSubType */
	int					nDur;					/**< Indicates the transition duration */
}MMS_SMIL_TRANSITION;


/**
 *	@brief	Represents SMIL meta information.
 */
typedef struct
{
	char		szID[MAX_SMIL_META_ID + 1];				/**< Indicates the ID of meta information */
	char		szName[MAX_SMIL_META_NAME + 1];		/**< Indicates the Name */
	char		szContent[MAX_SMIL_META_CONTENT + 1];	/**< Indicates the content */
}MMS_SMIL_META;


/**
 *	@brief	Represents application id information for JAVA MMS msg.
 */
typedef struct
{
	bool 			valid;										/**< Indicates whether application id information is used or not. */
	char 			appId[MAX_MMS_JAVA_APPID_LEN + 1];			/**< application id, it should not exceed 32 chars */
	char 			replyToAppId[MAX_MMS_JAVA_APPID_LEN + 1];	/**< reply to application id, application id, it should not exceeded 32 chars */
}MMS_APPID_INFO_S;



#define		MAX_FULL_PATH_SIZE_S	160	// max length for internal file path

typedef struct
{
	char					szMsgID[MMS_MSG_ID_LEN + 1];
	char					retrievedFilePath[MAX_FULL_PATH_SIZE_S + 1];
	char					szTrID[MMS_TR_ID_LEN + 1];
	MMS_APPID_INFO_S	msgAppId;
}MMS_RECV_DATA_S;

typedef struct _MMS_HEADER_DATA_S
{
	char messageID[MSG_MSG_ID_LEN + 1];
	char trID[MSG_MSG_ID_LEN + 1];
	char contentLocation[MSG_MSG_ID_LEN + 1];
	char szContentType[MSG_MSG_ID_LEN + 1];//string : ex) application/vnd.wap.multipart.related
	int contentType;//MimeType : ex) application/vnd.wap.multipart.related
	int messageType;//MmsMsgType : ex) sendreq
	int mmsVersion;//1.0 1.3
	int messageClass;//Personal | Advertisement | Informational | Auto
	int contentClass;//text | image-basic| image-rich | video-basic | video-rich | megapixel | content-basic | content-rich
	int mmsPriority;//_MSG_PRIORITY_TYPE_E : Low | Normal | High
} MMS_HEADER_DATA_S;

typedef struct
{
	MimeType	type;	/**< Indicates the multipart mime type. see enum MimeType */
	char		szContentType[MSG_MSG_ID_LEN + 1];		/**< Indicates the content type */
	char		szFileName[MSG_FILENAME_LEN_MAX + 1];		/**< Indicates the file name */
	char		szFilePath[MSG_FILEPATH_LEN_MAX + 1];		/**< Indicates the file path */
	char		szContentID[MSG_MSG_ID_LEN + 1];		/**< Indicates the content id */
	char		szContentLocation[MSG_MSG_ID_LEN + 1];	/**< Indicates the content Location */
} MMS_MULTIPART_DATA_S;

/**
 *	@brief	Represents MMS message data.
 */
typedef struct _MMS_MESSAGE_DATA_S
{
	int 					backup_type; //normal = 0|| backup = 1;
	char					szSmilFilePath[MSG_FILEPATH_LEN_MAX + 1];	/**< Indicates the SMIL file path */
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
	MMS_HEADER_DATA_S header;
	MMS_MULTIPART_DATA_S smil;
} MMS_MESSAGE_DATA_S;

#endif
