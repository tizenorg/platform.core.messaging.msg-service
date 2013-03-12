/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdio.h>
#include <string.h>

#include <MsgHandle.h>
#include <MsgException.h>
#include "MsgTypes.h"
#include "MsgMmsTypes.h"
#include "MsgMmsMessage.h"
#include "MsgDebug.h"

#include "msg.h"
#include "msg_private.h"

//Internel Struct
typedef struct _MMS_DATA_S
{
	GList			*pagelist;
	GList			*regionlist;
	GList 			*attachlist;
	GList 			*transitionlist;
	GList 			*metalist;
	MMS_SMIL_ROOTLAYOUT	rootlayout;
	MMS_APPID_INFO_S 	msgAppId;
} MMS_DATA_HIDDEN_S;

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
	msg_struct_s *pText;
	msg_struct_s *pAVI;
} MMS_MEDIA_HIDDEN_S;

static void __msg_mms_release_mms(msg_struct_s *mms_struct);
static void __msg_mms_release_page(msg_struct_s *page_struct);
static void __msg_mms_release_media(msg_struct_s *media_struct);
static void __msg_mms_release_region(msg_struct_s *region_struct);
static void __msg_mms_release_attach(msg_struct_s *attach_struct);
static void __msg_mms_release_transition(msg_struct_s *transition_struct);
static void __msg_mms_release_meta(msg_struct_s *meta_struct);

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
static inline void *get_msg_struct_data(msg_struct_s *msg_struct)
{
	if (msg_struct == NULL)
		return NULL;

	return msg_struct->data;
}

msg_struct_s *msg_mms_create_struct(int type)
{
	msg_struct_s *msg_struct = new msg_struct_s;
	bzero(msg_struct, sizeof(msg_struct_s));

	msg_struct->type = type;
	msg_struct->data = msg_mms_create_struct_data(type);

	return msg_struct;
}

void *msg_mms_create_struct_data(int field)
{
	void *data = NULL;

	switch(field)
	{
	case MSG_STRUCT_MMS:
		data = (void *)new MMS_DATA_HIDDEN_S;
		bzero(data, sizeof(MMS_DATA_HIDDEN_S));
		break;
	case MSG_STRUCT_MMS_PAGE:
		data = (void *)new MMS_PAGE_S;
		bzero(data, sizeof(MMS_PAGE_S));
		break;
	case MSG_STRUCT_MMS_MEDIA:
	{
		MMS_MEDIA_HIDDEN_S *media = new MMS_MEDIA_HIDDEN_S;
		bzero(media, sizeof(MMS_MEDIA_HIDDEN_S));

		media->pText = msg_mms_create_struct(MSG_STRUCT_MMS_SMIL_TEXT);
		media->pAVI = msg_mms_create_struct(MSG_STRUCT_MMS_SMIL_AVI);
		data = (void *)media;
	}
		break;
	case MSG_STRUCT_MMS_ATTACH:
		data = (void *)new MMS_ATTACH_S;
		bzero(data, sizeof(MMS_ATTACH_S));
		break;
	case MSG_STRUCT_MMS_REGION:
		data = (void *)new MMS_SMIL_REGION;
		bzero(data, sizeof(MMS_SMIL_REGION));
		((MMS_SMIL_REGION *)data)->fit = MMSUI_IMAGE_REGION_FIT_MEET;
		break;
	case MSG_STRUCT_MMS_TRANSITION:
		data = (void *)new MMS_SMIL_TRANSITION;
		bzero(data, sizeof(MMS_SMIL_TRANSITION));
		break;
	case MSG_STRUCT_MMS_META:
		data = (void *)new MMS_SMIL_META;
		bzero(data, sizeof(MMS_SMIL_META));
		break;
	case MSG_STRUCT_MMS_SMIL_TEXT:
		data = (void *)new MmsSmilText;
		bzero(data, sizeof(MmsSmilText));
		break;
	case MSG_STRUCT_MMS_SMIL_AVI:
		data = (void *)new MmsSmilAVI;
		bzero(data, sizeof(MmsSmilAVI));
		break;
	}

	return data;
}

void msg_mms_list_item_free_func(gpointer data)
{
	msg_struct_s *msg_struct = (msg_struct_s *)data;
	if (msg_struct->type == MSG_STRUCT_MMS_MEDIA)  {
		__msg_mms_release_media(msg_struct);
	} else if (msg_struct->type == MSG_STRUCT_MMS_PAGE) {
		__msg_mms_release_page(msg_struct);
	} else if (msg_struct->type == MSG_STRUCT_MMS_ATTACH) {
		__msg_mms_release_attach(msg_struct);
	} else if (msg_struct->type == MSG_STRUCT_MMS_REGION) {
		__msg_mms_release_region(msg_struct);
	} else if (msg_struct->type == MSG_STRUCT_MMS_TRANSITION) {
		__msg_mms_release_transition(msg_struct);
	} else if (msg_struct->type == MSG_STRUCT_MMS_META) {
		__msg_mms_release_meta(msg_struct);
	}
}

void __msg_mms_release_page(msg_struct_s *page_struct)
{
	MMS_PAGE_S *page = (MMS_PAGE_S *)page_struct->data;
	if (page->medialist) {
		 g_list_free_full(page->medialist, msg_mms_list_item_free_func);
	}

	delete (MMS_PAGE_S *)page;
	page_struct->data = NULL;

	delete page_struct;
}

void __msg_mms_release_media(msg_struct_s *media_struct)
{
	MMS_MEDIA_HIDDEN_S *media = (MMS_MEDIA_HIDDEN_S *)media_struct->data;

	if (media->pText)
		msg_mms_release_struct(&media->pText);

	if (media->pAVI)
		msg_mms_release_struct(&media->pAVI);

	delete (MMS_MEDIA_HIDDEN_S *)media;

	media_struct->data = NULL;

	delete media_struct;
}

void __msg_mms_release_attach(msg_struct_s *attach_struct)
{
	MMS_ATTACH_S *attach = (MMS_ATTACH_S *)attach_struct->data;

	delete (MMS_ATTACH_S *)attach;

	attach_struct->data = NULL;

	delete attach_struct;
}

void __msg_mms_release_region(msg_struct_s *region_struct)
{
	MMS_SMIL_REGION *region = (MMS_SMIL_REGION *)region_struct->data;

	delete (MMS_SMIL_REGION *)region;

	region_struct->data = NULL;

	delete region_struct;
}

void __msg_mms_release_transition(msg_struct_s *transition_struct)
{
	MMS_SMIL_TRANSITION *transition = (MMS_SMIL_TRANSITION *)transition_struct->data;

	delete (MMS_SMIL_TRANSITION *)transition;

	transition_struct->data = NULL;

	delete transition_struct;
}

void __msg_mms_release_meta(msg_struct_s *meta_struct)
{
	MMS_SMIL_META *meta = (MMS_SMIL_META *)meta_struct->data;

	delete (MMS_SMIL_META *)meta;

	meta_struct->data = NULL;

	delete meta_struct;
}

void __msg_mms_release_mms(msg_struct_s *mms_struct)
{
	MMS_DATA_HIDDEN_S *mms = (MMS_DATA_HIDDEN_S *)mms_struct->data;

	if (mms->pagelist) {
		g_list_free_full(mms->pagelist, msg_mms_list_item_free_func);
		mms->pagelist = NULL;
	}

	if (mms->regionlist) {
		g_list_free_full(mms->regionlist, msg_mms_list_item_free_func);
		mms->regionlist = NULL;
	}

	if (mms->attachlist) {
		g_list_free_full(mms->attachlist, msg_mms_list_item_free_func);
		mms->attachlist = NULL;
	}

	if (mms->transitionlist) {
		g_list_free_full(mms->transitionlist, msg_mms_list_item_free_func);
		mms->transitionlist = NULL;
	}

	if (mms->metalist) {
		g_list_free_full(mms->metalist, msg_mms_list_item_free_func);
		mms->metalist = NULL;
	}

	delete (MMS_DATA_HIDDEN_S *)mms;

	mms_struct->data = NULL;

	delete mms_struct;
}

int msg_mms_release_struct(msg_struct_s **msg_struct_data)
{
	msg_struct_s *msg_struct = (msg_struct_s *)*msg_struct_data;
	int type = msg_struct->type;

	switch(type)
	{
	case MSG_STRUCT_MMS:
		__msg_mms_release_mms(*msg_struct_data);
		*msg_struct_data = NULL;
		break;
	case MSG_STRUCT_MMS_PAGE:
		__msg_mms_release_page(*msg_struct_data);
		*msg_struct_data = NULL;
		break;
	case MSG_STRUCT_MMS_MEDIA:
		__msg_mms_release_media(*msg_struct_data);
		*msg_struct_data = NULL;
		break;
	case MSG_STRUCT_MMS_ATTACH:
		__msg_mms_release_attach(*msg_struct_data);
		*msg_struct_data = NULL;
		break;
	case MSG_STRUCT_MMS_REGION:
		__msg_mms_release_attach(*msg_struct_data);
		*msg_struct_data = NULL;
		break;
	case MSG_STRUCT_MMS_TRANSITION:
		__msg_mms_release_transition(*msg_struct_data);
		*msg_struct_data = NULL;
		break;
	case MSG_STRUCT_MMS_META:
		__msg_mms_release_meta(*msg_struct_data);
		*msg_struct_data = NULL;
		break;
	case MSG_STRUCT_MMS_SMIL_TEXT:
		delete (MmsSmilText*)msg_struct->data;
		delete msg_struct;
		*msg_struct_data = NULL;
		break;
	case MSG_STRUCT_MMS_SMIL_AVI:
		delete (MmsSmilAVI*)msg_struct->data;
		delete msg_struct;
		*msg_struct_data = NULL;
		break;
	}

	return 0;
}

int msg_mms_get_int_value(msg_struct_s *msg_struct, int field, int *value)
{
	msg_error_t err = MSG_SUCCESS;

	switch(msg_struct->type) {
	case MSG_STRUCT_MMS:
	{
		MMS_DATA_HIDDEN_S *mms_data = (MMS_DATA_HIDDEN_S *)msg_struct->data;
		if (field == MSG_MMS_ROOTLAYOUT_WIDTH_INT)
			*value = mms_data->rootlayout.width.value;
		else if (field == MSG_MMS_ROOTLAYOUT_HEIGHT_INT)
			*value = mms_data->rootlayout.height.value;
		else if (field == MSG_MMS_ROOTLAYOUT_BGCOLOR_INT)
			*value = mms_data->rootlayout.bgColor;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_PAGE:
	{
		MMS_PAGE_S *mms_page_data = (MMS_PAGE_S *)msg_struct->data;
		if (field == MSG_MMS_PAGE_PAGE_DURATION_INT)
			*value = mms_page_data->nDur;
		else if (field == MSG_MMS_PAGE_BEGIN_TIME_INT)
			*value = mms_page_data->nBegin;
		else if (field == MSG_MMS_PAGE_END_TIME_INT)
			*value = mms_page_data->nEnd;
		else if (field == MSG_MMS_PAGE_MIN_INT)
			*value = mms_page_data->nMin;
		else if (field == MSG_MMS_PAGE_MAX_INT)
			*value = mms_page_data->nMax;
		else if (field == MSG_MMS_PAGE_REPEAT_INT)
			*value = mms_page_data->nRepeat;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_MEDIA:
	{
		MMS_MEDIA_S *mms_media_data = (MMS_MEDIA_S *)msg_struct->data;
		if (field == MSG_MMS_MEDIA_TYPE_INT)
			*value = mms_media_data->mediatype;
		else if (field == MSG_MMS_MEDIA_DRM_TYPE_INT)
			*value = mms_media_data->drmType;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_ATTACH:
	{
		MMS_ATTACH_S *mms_attach_data = (MMS_ATTACH_S *)msg_struct->data;
		if (field == MSG_MMS_ATTACH_MIME_TYPE_INT)
			*value = mms_attach_data->mediatype;
		else if (field == MSG_MMS_ATTACH_FILESIZE_INT)
			*value = mms_attach_data->fileSize;
		else if (field == MSG_MMS_ATTACH_DRM_TYPE_INT)
			*value = mms_attach_data->drmType;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_REGION:
	{
		MMS_SMIL_REGION *mms_region_data = (MMS_SMIL_REGION *)msg_struct->data;
		if (field == MSG_MMS_REGION_LENGTH_LEFT_INT)
			*value = mms_region_data->nLeft.value;
		else if (field == MSG_MMS_REGION_LENGTH_TOP_INT)
			*value = mms_region_data->nTop.value;
		else if (field == MSG_MMS_REGION_LENGTH_WIDTH_INT)
			*value = mms_region_data->width.value;
		else if (field == MSG_MMS_REGION_LENGTH_HEIGHT_INT)
			*value = mms_region_data->height.value;
		else if (field == MSG_MMS_REGION_BGCOLOR_INT)
			*value = mms_region_data->bgColor;
		else if (field == MSG_MMS_REGION_FIT_TYPE_INT)
			*value = mms_region_data->fit;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_TRANSITION:
	{
		MMS_SMIL_TRANSITION *mms_transition_data = (MMS_SMIL_TRANSITION *)msg_struct->data;
		if (field == MSG_MMS_TRANSITION_TYPE_INT)
			*value = mms_transition_data->nType;
		else if (field == MSG_MMS_TRANSITION_SUBTYPE_INT)
			*value = mms_transition_data->nSubType;
		else if (field == MSG_MMS_TRANSITION_DURATION_INT)
			*value = mms_transition_data->nDur;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_SMIL_TEXT:
	{
		MmsSmilText *mms_smil_text_data = (MmsSmilText *)msg_struct->data;
		if (field == MSG_MMS_SMIL_TEXT_REPEAT_INT)
			*value = mms_smil_text_data->nRepeat;
		else if (field == MSG_MMS_SMIL_TEXT_BEGIN_INT)
			*value = mms_smil_text_data->nBegin;
		else if (field == MSG_MMS_SMIL_TEXT_END_INT)
			*value = mms_smil_text_data->nEnd;
		else if (field == MSG_MMS_SMIL_TEXT_DURTIME_INT)
			*value = mms_smil_text_data->nDurTime;
		else if (field == MSG_MMS_SMIL_TEXT_BGCOLOR_INT)
			*value = mms_smil_text_data->nBgColor;
		else if (field == MSG_MMS_SMIL_TEXT_DIRECTION_TYPE_INT)
			*value = mms_smil_text_data->nDirection;
		else if (field == MSG_MMS_SMIL_TEXT_SIZE_INT)
			*value = mms_smil_text_data->nSize;
		else if (field == MSG_MMS_SMIL_TEXT_COLOR_INT)
			*value = mms_smil_text_data->nColor;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_SMIL_AVI:
	{
		MmsSmilAVI *mms_smil_avi_data = (MmsSmilAVI *)msg_struct->data;
		if (field == MSG_MMS_SMIL_AVI_REPEAT_INT)
			*value = mms_smil_avi_data->nRepeat;
		else if (field == MSG_MMS_SMIL_AVI_BEGIN_INT)
			*value = mms_smil_avi_data->nBegin;
		else if (field == MSG_MMS_SMIL_AVI_END_INT)
			*value = mms_smil_avi_data->nEnd;
		else if (field == MSG_MMS_SMIL_AVI_DURTIME_INT)
			*value = mms_smil_avi_data->nDurTime;
		else if (field == MSG_MMS_SMIL_AVI_BGCOLOR_INT)
			*value = mms_smil_avi_data->nBgColor;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return MSG_SUCCESS;
}

int msg_mms_get_str_value(msg_struct_s *msg_struct, int field, char *value, int size)
{
	msg_error_t err = MSG_SUCCESS;

	switch(msg_struct->type) {
	case MSG_STRUCT_MMS_MEDIA:
	{
		MMS_MEDIA_S *mms_media_data = (MMS_MEDIA_S *)msg_struct->data;
		if (field == MSG_MMS_MEDIA_SRC_STR)
			strncpy(value, mms_media_data->szSrc, size);
		else if (field == MSG_MMS_MEDIA_FILENAME_STR)
			strncpy(value, mms_media_data->szFileName, size);
		else if (field == MSG_MMS_MEDIA_FILEPATH_STR)
			strncpy(value, mms_media_data->szFilePath, size);
		else if (field == MSG_MMS_MEDIA_CONTENT_ID_STR)
			strncpy(value, mms_media_data->szContentID, size);
		else if (field == MSG_MMS_MEDIA_REGION_ID_STR)
			strncpy(value, mms_media_data->regionId, size);
		else if (field == MSG_MMS_MEDIA_ALTERNATIVE_STR)
			strncpy(value, mms_media_data->szAlt, size);
		else if (field == MSG_MMS_MEDIA_DRM_FULLPATH_STR)
			strncpy(value, mms_media_data->szDrm2FullPath, size);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_ATTACH:
	{
		MMS_ATTACH_S *mms_attach_data = (MMS_ATTACH_S *)msg_struct->data;
		if (field == MSG_MMS_ATTACH_FILENAME_STR)
			strncpy(value, mms_attach_data->szFileName, size);
		else if (field == MSG_MMS_ATTACH_FILEPATH_STR)
			strncpy(value, mms_attach_data->szFilePath, size);
		else if (field == MSG_MMS_ATTACH_DRM_FULLPATH_STR)
			strncpy(value, mms_attach_data->szDrm2FullPath, size);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_REGION:
	{
		MMS_SMIL_REGION *mms_region_data = (MMS_SMIL_REGION *)msg_struct->data;
		if (field == MSG_MMS_REGION_ID_STR)
			strncpy(value, mms_region_data->szID, size);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_TRANSITION:
	{
		MMS_SMIL_TRANSITION *mms_transition_data = (MMS_SMIL_TRANSITION *)msg_struct->data;
		if (field == MSG_MMS_TRANSITION_ID_STR)
			strncpy(value, mms_transition_data->szID, size);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_META:
	{
		MMS_SMIL_META *mms_meta_data = (MMS_SMIL_META *)msg_struct->data;
		if (field == MSG_MMS_META_ID_STR)
			strncpy(value, mms_meta_data->szID, size);
		else if (field == MSG_MMS_META_NAME_STR)
			strncpy(value, mms_meta_data->szName, size);
		else if (field == MSG_MMS_META_CONTENT_STR)
			strncpy(value, mms_meta_data->szContent, size);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_SMIL_TEXT:
	{
		MmsSmilText *mms_smil_text_data = (MmsSmilText *)msg_struct->data;
		if (field == MSG_MMS_SMIL_TEXT_TRANSITION_IN_ID_STR)
			strncpy(value, mms_smil_text_data->szTransInId, size);
		else if (field == MSG_MMS_SMIL_TEXT_TRANSITION_OUT_ID_STR)
			strncpy(value, mms_smil_text_data->szTransOutId, size);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_SMIL_AVI:
	{
		MmsSmilAVI *mms_smil_avi_data = (MmsSmilAVI *)msg_struct->data;
		if (field == MSG_MMS_SMIL_AVI_TRANSITION_IN_ID_STR)
			strncpy(value, mms_smil_avi_data->szTransInId, size);
		else if (field == MSG_MMS_SMIL_AVI_TRANSITION_OUT_ID_STR)
			strncpy(value, mms_smil_avi_data->szTransOutId, size);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	default :
			err = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return MSG_SUCCESS;
}

int msg_mms_get_bool_value(msg_struct_s *msg_struct, int field, bool *value)
{
	msg_error_t err = MSG_SUCCESS;

	switch(msg_struct->type) {
	case MSG_STRUCT_MMS:
	{
		MMS_DATA_HIDDEN_S *mms_data = (MMS_DATA_HIDDEN_S *)msg_struct->data;
		if (field == MSG_MMS_ROOTLAYOUT_WIDTH_PERCENT_BOOL)
			*value = mms_data->rootlayout.width.bUnitPercent;
		else if (field == MSG_MMS_ROOTLAYOUT_HEIGHT_PERCENT_BOOL)
			*value = mms_data->rootlayout.height.bUnitPercent;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_REGION:
	{
		MMS_SMIL_REGION *mms_region_data = (MMS_SMIL_REGION *)msg_struct->data;
		if (field == MSG_MMS_REGION_LENGTH_LEFT_PERCENT_BOOL)
			*value = mms_region_data->nLeft.bUnitPercent;
		else if (field == MSG_MMS_REGION_LENGTH_TOP_PERCENT_BOOL)
			*value = mms_region_data->nTop.bUnitPercent;
		else if (field == MSG_MMS_REGION_LENGTH_WIDTH_PERCENT_BOOL)
			*value = mms_region_data->width.bUnitPercent;
		else if (field == MSG_MMS_REGION_LENGTH_HEIGHT_PERCENT_BOOL)
			*value = mms_region_data->height.bUnitPercent;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_SMIL_TEXT:
	{
		MmsSmilText *mms_smil_text_data = (MmsSmilText *)msg_struct->data;
		if (field == MSG_MMS_SMIL_TEXT_BOLD_BOOL)
			*value = mms_smil_text_data->bBold;
		else if (field == MSG_MMS_SMIL_TEXT_UNDERLINE_BOOL)
			*value = mms_smil_text_data->bUnderLine;
		else if (field == MSG_MMS_SMIL_TEXT_ITALIC_BOOL)
			*value = mms_smil_text_data->bItalic;
		else if (field == MSG_MMS_SMIL_TEXT_REVERSE_BOOL)
			*value = mms_smil_text_data->bReverse;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return MSG_SUCCESS;
}

int msg_mms_get_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s **value)
{
	msg_error_t err = MSG_SUCCESS;

	switch(msg_struct->type) {
	case MSG_STRUCT_MMS_MEDIA:
	{
		MMS_MEDIA_HIDDEN_S *mms_media_data = (MMS_MEDIA_HIDDEN_S *)msg_struct->data;
		if (field == MSG_MMS_MEDIA_SMIL_TEXT_HND)
			*value = mms_media_data->pText;
		else if (field == MSG_MMS_MEDIA_SMIL_AVI_HND)
			*value = mms_media_data->pAVI;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return MSG_SUCCESS;
}

int msg_mms_get_list_handle(msg_struct_s *msg_struct, int field, msg_list_handle_t *value)
{
	msg_error_t err = MSG_SUCCESS;

	switch(msg_struct->type) {
	case MSG_STRUCT_MMS:
	{
		MMS_DATA_HIDDEN_S *mms_data = (MMS_DATA_HIDDEN_S *)msg_struct->data;
		if (field == MSG_MMS_PAGE_LIST_HND)
			*value = (msg_list_handle_t)mms_data->pagelist;
		else if (field == MSG_MMS_REGION_LIST_HND)
			*value = (msg_list_handle_t)mms_data->regionlist;
		else if (field == MSG_MMS_ATTACH_LIST_HND)
			*value = (msg_list_handle_t)mms_data->attachlist;
		else if (field == MSG_MMS_TRANSITION_LIST_HND)
			*value = (msg_list_handle_t)mms_data->transitionlist;
		else if (field == MSG_MMS_META_LIST_HND)
			*value = (msg_list_handle_t)mms_data->metalist;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_PAGE:
	{
		MMS_PAGE_S *mms_page_data = (MMS_PAGE_S *)msg_struct->data;
		if (field == MSG_MMS_PAGE_MEDIA_LIST_HND)
			*value = (msg_list_handle_t)mms_page_data->medialist;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return MSG_SUCCESS;
}
/*SET*/
int msg_mms_set_int_value(msg_struct_s *msg_struct, int field, int value)
{
	msg_error_t err = MSG_SUCCESS;

	switch(msg_struct->type) {
	case MSG_STRUCT_MMS:
	{
		MMS_DATA_HIDDEN_S *mms_data = (MMS_DATA_HIDDEN_S *)msg_struct->data;
		if (field == MSG_MMS_ROOTLAYOUT_WIDTH_INT)
			mms_data->rootlayout.width.value = value;
		else if (field == MSG_MMS_ROOTLAYOUT_HEIGHT_INT)
			mms_data->rootlayout.height.value = value;
		else if (field == MSG_MMS_ROOTLAYOUT_BGCOLOR_INT)
			mms_data->rootlayout.bgColor = value;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_PAGE:
	{
		MMS_PAGE_S *mms_page_data = (MMS_PAGE_S *)msg_struct->data;
		if (field == MSG_MMS_PAGE_PAGE_DURATION_INT)
			mms_page_data->nDur = value;
		else if (field == MSG_MMS_PAGE_BEGIN_TIME_INT)
			mms_page_data->nBegin = value;
		else if (field == MSG_MMS_PAGE_END_TIME_INT)
			mms_page_data->nEnd = value;
		else if (field == MSG_MMS_PAGE_MIN_INT)
			mms_page_data->nMin = value;
		else if (field == MSG_MMS_PAGE_MAX_INT)
			mms_page_data->nMax = value;
		else if (field == MSG_MMS_PAGE_REPEAT_INT)
			mms_page_data->nRepeat = value;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_MEDIA:
	{
		MMS_MEDIA_S *mms_media_data = (MMS_MEDIA_S *)msg_struct->data;
		if (field == MSG_MMS_MEDIA_TYPE_INT)
			mms_media_data->mediatype = (MmsSmilMediaType)value;
		else if (field == MSG_MMS_MEDIA_DRM_TYPE_INT)
			mms_media_data->drmType = (MsgDrmType)value;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_ATTACH:
	{
		MMS_ATTACH_S *mms_attach_data = (MMS_ATTACH_S *)msg_struct->data;
		if (field == MSG_MMS_ATTACH_MIME_TYPE_INT)
			mms_attach_data->mediatype = (MimeType)value;
		else if (field == MSG_MMS_ATTACH_FILESIZE_INT)
			mms_attach_data->fileSize = value;
		else if (field == MSG_MMS_ATTACH_DRM_TYPE_INT)
			mms_attach_data->drmType = (MsgDrmType)value;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_REGION:
	{
		MMS_SMIL_REGION *mms_region_data = (MMS_SMIL_REGION *)msg_struct->data;
		if (field == MSG_MMS_REGION_LENGTH_LEFT_INT)
			mms_region_data->nLeft.value = value;
		else if (field == MSG_MMS_REGION_LENGTH_TOP_INT)
			mms_region_data->nTop.value = value;
		else if (field == MSG_MMS_REGION_LENGTH_WIDTH_INT)
			mms_region_data->width.value = value;
		else if (field == MSG_MMS_REGION_LENGTH_HEIGHT_INT)
			mms_region_data->height.value = value;
		else if (field == MSG_MMS_REGION_BGCOLOR_INT)
			mms_region_data->bgColor = value;
		else if (field == MSG_MMS_REGION_FIT_TYPE_INT)
			mms_region_data->fit = (REGION_FIT_TYPE_T)value;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_TRANSITION:
	{
		MMS_SMIL_TRANSITION *mms_transition_data = (MMS_SMIL_TRANSITION *)msg_struct->data;
		if (field == MSG_MMS_TRANSITION_TYPE_INT)
			mms_transition_data->nType = (MmsSmilTransType)value;
		else if (field == MSG_MMS_TRANSITION_SUBTYPE_INT)
			mms_transition_data->nSubType = (MmsSmilTransSubType)value;
		else if (field == MSG_MMS_TRANSITION_DURATION_INT)
			mms_transition_data->nDur = value;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_SMIL_TEXT:
	{
		MmsSmilText *mms_smil_text_data = (MmsSmilText *)msg_struct->data;
		if (field == MSG_MMS_SMIL_TEXT_REPEAT_INT)
			mms_smil_text_data->nRepeat = value;
		else if (field == MSG_MMS_SMIL_TEXT_BEGIN_INT)
			mms_smil_text_data->nBegin = value;
		else if (field == MSG_MMS_SMIL_TEXT_END_INT)
			mms_smil_text_data->nEnd = value;
		else if (field == MSG_MMS_SMIL_TEXT_DURTIME_INT)
			mms_smil_text_data->nDurTime = value;
		else if (field == MSG_MMS_SMIL_TEXT_BGCOLOR_INT)
			mms_smil_text_data->nBgColor = value;
		else if (field == MSG_MMS_SMIL_TEXT_DIRECTION_TYPE_INT)
			mms_smil_text_data->nDirection = (MmsTextDirection)value;
		else if (field == MSG_MMS_SMIL_TEXT_SIZE_INT)
			mms_smil_text_data->nSize = value;
		else if (field == MSG_MMS_SMIL_TEXT_COLOR_INT)
			mms_smil_text_data->nColor = value;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_SMIL_AVI:
	{
		MmsSmilAVI *mms_smil_avi_data = (MmsSmilAVI *)msg_struct->data;
		if (field == MSG_MMS_SMIL_AVI_REPEAT_INT)
			mms_smil_avi_data->nRepeat = value;
		else if (field == MSG_MMS_SMIL_AVI_BEGIN_INT)
			mms_smil_avi_data->nBegin = value;
		else if (field == MSG_MMS_SMIL_AVI_END_INT)
			mms_smil_avi_data->nEnd = value;
		else if (field == MSG_MMS_SMIL_AVI_DURTIME_INT)
			mms_smil_avi_data->nDurTime = value;
		else if (field == MSG_MMS_SMIL_AVI_BGCOLOR_INT)
			mms_smil_avi_data->nBgColor = value;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return MSG_SUCCESS;
}

int msg_mms_set_str_value(msg_struct_s *msg_struct, int field, char *value, int size)
{
	msg_error_t err = MSG_SUCCESS;

	switch(msg_struct->type) {
	case MSG_STRUCT_MMS_MEDIA:
	{
		MMS_MEDIA_S *mms_media_data = (MMS_MEDIA_S *)msg_struct->data;
		if (field == MSG_MMS_MEDIA_SRC_STR)
			strncpy(mms_media_data->szSrc, value, MSG_FILEPATH_LEN_MAX);
		else if (field == MSG_MMS_MEDIA_FILENAME_STR) {
			strncpy(mms_media_data->szFileName, value, MSG_FILEPATH_LEN_MAX);
		} else if (field == MSG_MMS_MEDIA_FILEPATH_STR) {
			char *filename = NULL;
			if (value != NULL) {
				MSG_DEBUG("media file path = %s", value);
				strncpy(mms_media_data->szFilePath, value, MSG_FILEPATH_LEN_MAX);
				filename = strrchr(value, '/');
				if (filename != NULL) {
					strncpy(mms_media_data->szFileName, filename + 1, MSG_FILENAME_LEN_MAX);
					strncpy(mms_media_data->szContentID, filename + 1, MSG_MSG_ID_LEN);
				} else {
					strncpy(mms_media_data->szFileName, value + 1, MSG_FILENAME_LEN_MAX);
					strncpy(mms_media_data->szContentID, value + 1, MSG_MSG_ID_LEN);
				}
			} else {
				MSG_DEBUG("media file path is NULL");
				err = MSG_ERR_INVALID_PARAMETER;
			}
		} else if (field == MSG_MMS_MEDIA_CONTENT_ID_STR)
			strncpy(mms_media_data->szContentID, value, MSG_MSG_ID_LEN);
		else if (field == MSG_MMS_MEDIA_REGION_ID_STR)
			strncpy(mms_media_data->regionId, value, MAX_SMIL_REGION_ID);
		else if (field == MSG_MMS_MEDIA_ALTERNATIVE_STR)
			strncpy(mms_media_data->szAlt, value, MAX_SMIL_ALT_LEN);
		else if (field == MSG_MMS_MEDIA_DRM_FULLPATH_STR)
			strncpy(mms_media_data->szDrm2FullPath, value, MSG_FILEPATH_LEN_MAX);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_ATTACH:
	{
		MMS_ATTACH_S *mms_attach_data = (MMS_ATTACH_S *)msg_struct->data;
		if (field == MSG_MMS_ATTACH_FILENAME_STR)
			strncpy(mms_attach_data->szFileName, value, MSG_FILENAME_LEN_MAX);
		else if (field == MSG_MMS_ATTACH_FILEPATH_STR) {
			char *filename = NULL;
			char *filepath = value;

			if (filepath != NULL) {
				MSG_DEBUG("attach file path = %s", filepath);
				mms_attach_data->mediatype = MIME_UNKNOWN;
				mms_attach_data->fileSize = -1;

				strncpy(mms_attach_data->szFilePath, filepath, MSG_FILEPATH_LEN_MAX);

				filename = strrchr(filepath, '/');
				if (filename != NULL) {
					strncpy(mms_attach_data->szFileName, filename + 1, MSG_FILENAME_LEN_MAX);
				} else {
					strncpy(mms_attach_data->szFileName, filepath, MSG_FILENAME_LEN_MAX);
				}

			} else {
				MSG_DEBUG("attach file path is NULL");
				err = MSG_ERR_INVALID_PARAMETER;
			}
		} else if (field == MSG_MMS_ATTACH_DRM_FULLPATH_STR)
			strncpy(mms_attach_data->szDrm2FullPath, value, MSG_FILEPATH_LEN_MAX);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_REGION:
	{
		MMS_SMIL_REGION *mms_region_data = (MMS_SMIL_REGION *)msg_struct->data;
		if (field == MSG_MMS_REGION_ID_STR)
			strncpy(mms_region_data->szID, value, MAX_SMIL_REGION_ID);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_TRANSITION:
	{
		MMS_SMIL_TRANSITION *mms_transition_data = (MMS_SMIL_TRANSITION *)msg_struct->data;
		if (field == MSG_MMS_TRANSITION_ID_STR)
			strncpy(mms_transition_data->szID, value, MAX_SMIL_TRANSITION_ID);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_META:
	{
		MMS_SMIL_META *mms_meta_data = (MMS_SMIL_META *)msg_struct->data;
		if (field == MSG_MMS_META_ID_STR)
			strncpy(mms_meta_data->szID, value, MAX_SMIL_META_ID);
		else if (field == MSG_MMS_META_NAME_STR)
			strncpy(mms_meta_data->szName, value, MAX_SMIL_META_NAME);
		else if (field == MSG_MMS_META_CONTENT_STR)
			strncpy(mms_meta_data->szContent, value, MAX_SMIL_META_CONTENT);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_SMIL_TEXT:
	{
		MmsSmilText *mms_smil_text_data = (MmsSmilText *)msg_struct->data;
		if (field == MSG_MMS_SMIL_TEXT_TRANSITION_IN_ID_STR)
			strncpy(mms_smil_text_data->szTransInId, value, MAX_SMIL_TRANSIN_ID);
		else if (field == MSG_MMS_SMIL_TEXT_TRANSITION_OUT_ID_STR)
			strncpy(mms_smil_text_data->szTransOutId, value, MAX_SMIL_TRANSOUT_ID);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_SMIL_AVI:
	{
		MmsSmilAVI *mms_smil_avi_data = (MmsSmilAVI *)msg_struct->data;
		if (field == MSG_MMS_SMIL_AVI_TRANSITION_IN_ID_STR)
			strncpy(mms_smil_avi_data->szTransInId, value, MAX_SMIL_TRANSIN_ID);
		else if (field == MSG_MMS_SMIL_AVI_TRANSITION_OUT_ID_STR)
			strncpy(mms_smil_avi_data->szTransOutId, value, MAX_SMIL_TRANSOUT_ID);
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	default :
			err = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return MSG_SUCCESS;
}

int msg_mms_set_bool_value(msg_struct_s *msg_struct, int field, bool value)
{
	msg_error_t err = MSG_SUCCESS;

	switch(msg_struct->type) {
	case MSG_STRUCT_MMS:
	{
		MMS_DATA_HIDDEN_S *mms_data = (MMS_DATA_HIDDEN_S *)msg_struct->data;
		if (field == MSG_MMS_ROOTLAYOUT_WIDTH_PERCENT_BOOL)
			mms_data->rootlayout.width.bUnitPercent = value;
		else if (field == MSG_MMS_ROOTLAYOUT_HEIGHT_PERCENT_BOOL)
			mms_data->rootlayout.height.bUnitPercent = value;
		else
			err  = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_REGION:
	{
		MMS_SMIL_REGION *mms_region_data = (MMS_SMIL_REGION *)msg_struct->data;
		if (field == MSG_MMS_REGION_LENGTH_LEFT_PERCENT_BOOL)
			mms_region_data->nLeft.bUnitPercent = value;
		else if (field == MSG_MMS_REGION_LENGTH_TOP_PERCENT_BOOL)
			mms_region_data->nTop.bUnitPercent = value;
		else if (field == MSG_MMS_REGION_LENGTH_WIDTH_PERCENT_BOOL)
			mms_region_data->width.bUnitPercent = value;
		else if (field == MSG_MMS_REGION_LENGTH_HEIGHT_PERCENT_BOOL)
			mms_region_data->height.bUnitPercent = value;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_SMIL_TEXT:
	{
		MmsSmilText *mms_smil_text_data = (MmsSmilText *)msg_struct->data;
		if (field == MSG_MMS_SMIL_TEXT_BOLD_BOOL)
			mms_smil_text_data->bBold = value;
		else if (field == MSG_MMS_SMIL_TEXT_UNDERLINE_BOOL)
			mms_smil_text_data->bUnderLine = value;
		else if (field == MSG_MMS_SMIL_TEXT_ITALIC_BOOL)
			mms_smil_text_data->bItalic = value;
		else if (field == MSG_MMS_SMIL_TEXT_REVERSE_BOOL)
			mms_smil_text_data->bReverse = value;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return MSG_SUCCESS;
}

int msg_mms_set_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s *value)
{
	msg_error_t err = MSG_SUCCESS;

	switch(msg_struct->type) {
	case MSG_STRUCT_MMS_MEDIA:
	{
		MMS_MEDIA_HIDDEN_S *mms_media_data = (MMS_MEDIA_HIDDEN_S *)msg_struct->data;

		if (field == MSG_MMS_MEDIA_SMIL_TEXT_HND)
			memcpy(mms_media_data->pText->data, value->data, sizeof(MmsSmilText));
		else if (field == MSG_MMS_MEDIA_SMIL_AVI_HND)
			memcpy(mms_media_data->pAVI->data, value->data, sizeof(MmsSmilAVI));
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return MSG_SUCCESS;
}

int msg_mms_set_list_handle(msg_struct_s *msg_struct, int field, msg_list_handle_t value)
{
	msg_error_t err = MSG_SUCCESS;

	switch(msg_struct->type) {
	case MSG_STRUCT_MMS:
	{
		MMS_DATA_HIDDEN_S *mms_data = (MMS_DATA_HIDDEN_S *)msg_struct->data;
		if (field == MSG_MMS_PAGE_LIST_HND)
			mms_data->pagelist = (GList *)value;
		else if (field == MSG_MMS_REGION_LIST_HND)
			mms_data->regionlist = (GList *)value;
		else if (field == MSG_MMS_ATTACH_LIST_HND)
			mms_data->attachlist = (GList *)value;
		else if (field == MSG_MMS_TRANSITION_LIST_HND)
			mms_data->transitionlist = (GList *)value;
		else if (field == MSG_MMS_META_LIST_HND)
			mms_data->metalist = (GList *)value;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	case MSG_STRUCT_MMS_PAGE:
	{
		MMS_PAGE_S *mms_page_data = (MMS_PAGE_S *)msg_struct->data;
		if (field == MSG_MMS_PAGE_MEDIA_LIST_HND)
			mms_page_data->medialist = (GList *)value;
		else
			err = MSG_ERR_INVALID_PARAMETER;
	}
	break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return MSG_SUCCESS;
}

EXPORT_API int msg_mms_add_item(msg_struct_t msg_struct_handle, int field, msg_struct_t *item)
{
	msg_error_t err = MSG_SUCCESS;
	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;

	msg_struct_s *msg_struct_item = NULL;
	switch(msg_struct->type) {
	case MSG_STRUCT_MMS:
	{

		MMS_DATA_HIDDEN_S *mms_data = (MMS_DATA_HIDDEN_S *)msg_struct->data;

		if (field == MSG_STRUCT_MMS_PAGE) {
			msg_struct_item = msg_mms_create_struct(field);
			mms_data->pagelist = g_list_append(mms_data->pagelist, msg_struct_item);
			*item = (msg_struct_t)msg_struct_item;
		} else if (field == MSG_STRUCT_MMS_REGION) {
			msg_struct_item = msg_mms_create_struct(field);
			mms_data->regionlist = g_list_append(mms_data->regionlist, msg_struct_item);
			*item = (msg_struct_t)msg_struct_item;
		} else if (field == MSG_STRUCT_MMS_ATTACH) {
			msg_struct_item = msg_mms_create_struct(field);
			mms_data->attachlist = g_list_append(mms_data->attachlist, msg_struct_item);
			*item = (msg_struct_t)msg_struct_item;
		} else if (field == MSG_STRUCT_MMS_TRANSITION) {
			msg_struct_item = msg_mms_create_struct(field);
			mms_data->transitionlist = g_list_append(mms_data->transitionlist, msg_struct_item);
			*item = (msg_struct_t)msg_struct_item;
		} else if (field == MSG_STRUCT_MMS_META) {
			msg_struct_item = msg_mms_create_struct(field);
			mms_data->metalist = g_list_append(mms_data->metalist, msg_struct_item);
			*item = (msg_struct_t)msg_struct_item;
		} else {
			err = MSG_ERR_INVALID_PARAMETER;
		}
	}
	break;
	case MSG_STRUCT_MMS_PAGE:
	{

		MMS_PAGE_S *mms_page_data = (MMS_PAGE_S *)msg_struct->data;

		if (field == MSG_STRUCT_MMS_MEDIA) {
			msg_struct_item = msg_mms_create_struct(field);
			mms_page_data->medialist = g_list_append(mms_page_data->medialist, msg_struct_item);
			*item = (msg_struct_t)msg_struct_item;
		} else {
			err = MSG_ERR_INVALID_PARAMETER;
		}
	}
	break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

void convert_to_media_data(const msg_struct_s *pSrc, MMS_MEDIA_S *pDest)
{
	const MMS_MEDIA_HIDDEN_S *src_media = (MMS_MEDIA_HIDDEN_S *)pSrc->data;
	MMS_MEDIA_S *dst_media = pDest;

	dst_media->mediatype = src_media->mediatype;
	memcpy(dst_media->szSrc, src_media->szSrc, MSG_FILEPATH_LEN_MAX);
	memcpy(dst_media->szFileName, src_media->szFileName, MSG_FILEPATH_LEN_MAX);
	memcpy(dst_media->szFilePath, src_media->szFilePath, MSG_FILEPATH_LEN_MAX);
	memcpy(dst_media->szContentID, src_media->szContentID, MSG_MSG_ID_LEN);
	memcpy(dst_media->regionId, src_media->regionId, MAX_SMIL_REGION_ID);
	memcpy(dst_media->szAlt, src_media->szAlt, MAX_SMIL_ALT_LEN);

	memcpy(dst_media->szDrm2FullPath, src_media->szDrm2FullPath, MSG_FILEPATH_LEN_MAX);
	dst_media->drmType = src_media->drmType;

	if (src_media->mediatype == MMS_SMIL_MEDIA_TEXT) {
		msg_struct_s *smil_struct = (msg_struct_s *)src_media->pText;
		MmsSmilText *smiltext = (MmsSmilText *)smil_struct->data;
		memcpy(&dst_media->sMedia.sText, smiltext, sizeof(MmsSmilText));
	} else {
		msg_struct_s *smil_struct = (msg_struct_s *)src_media->pAVI;
		MmsSmilAVI *smilavi = (MmsSmilAVI *)smil_struct->data;
		memcpy(&dst_media->sMedia.sAVI, smilavi, sizeof(MmsSmilAVI));
	}
}

void convert_from_media_data(const MMS_MEDIA_S *pSrc, msg_struct_s *pDest)
{
	const MMS_MEDIA_S *src_media = pSrc;
	MMS_MEDIA_HIDDEN_S *dst_media = (MMS_MEDIA_HIDDEN_S *)pDest->data;

	dst_media->mediatype = src_media->mediatype;
	memcpy(dst_media->szSrc, src_media->szSrc, MSG_FILEPATH_LEN_MAX);
	memcpy(dst_media->szFileName, src_media->szFileName, MSG_FILEPATH_LEN_MAX);
	memcpy(dst_media->szFilePath, src_media->szFilePath, MSG_FILEPATH_LEN_MAX);
	memcpy(dst_media->szContentID, src_media->szContentID, MSG_MSG_ID_LEN);
	memcpy(dst_media->regionId, src_media->regionId, MAX_SMIL_REGION_ID);
	memcpy(dst_media->szAlt, src_media->szAlt, MAX_SMIL_ALT_LEN);

	memcpy(dst_media->szDrm2FullPath, src_media->szDrm2FullPath, MSG_FILEPATH_LEN_MAX);
	dst_media->drmType = src_media->drmType;

	if (src_media->mediatype == MMS_SMIL_MEDIA_TEXT) {
		msg_struct_s *dst_smil_struct = (msg_struct_s *)dst_media->pText;
		MmsSmilText *dst_smiltext = (MmsSmilText *)dst_smil_struct->data;
		memcpy(dst_smiltext, &src_media->sMedia.sText, sizeof(MmsSmilText));
	} else {
		msg_struct_s *dst_smil_struct = (msg_struct_s *)dst_media->pAVI;
		MmsSmilAVI *dst_smilavi = (MmsSmilAVI *)dst_smil_struct->data;
		memcpy(dst_smilavi, &src_media->sMedia.sAVI, sizeof(MmsSmilAVI));
	}
}

void convert_to_mmsdata(const msg_struct_s *pSrc, MMS_MESSAGE_DATA_S *pDest)
{
	int i, j;
	MMS_DATA_HIDDEN_S *pSrcMms = (MMS_DATA_HIDDEN_S *)pSrc->data;

	pDest->pageCnt = g_list_length(pSrcMms->pagelist);

	for (i = 0; i < pDest->pageCnt; i++) {
		MMS_PAGE_S *page = (MMS_PAGE_S *)calloc(1, sizeof(MMS_PAGE_S));
		MMS_PAGE_S *src_page = (MMS_PAGE_S *)get_msg_struct_data((msg_struct_s *)g_list_nth_data(pSrcMms->pagelist, i));
		page->mediaCnt = g_list_length(src_page->medialist);

		for (j = 0; j < page->mediaCnt; j++)
		{
			MMS_MEDIA_S *dst_media = (MMS_MEDIA_S *)calloc(1, sizeof(MMS_MEDIA_S));
			msg_struct_s *src_media_s = (msg_struct_s *)g_list_nth_data(src_page->medialist, j);

			convert_to_media_data(src_media_s, dst_media);

			page->medialist = g_list_append(page->medialist, dst_media);
		}

		page->nDur = src_page->nDur;
		page->nBegin = src_page->nBegin;
		page->nEnd = src_page->nEnd;
		page->nMin = src_page->nMin;
		page->nMax = src_page->nMax;
		page->nRepeat = src_page->nRepeat;

		pDest->pagelist = g_list_append(pDest->pagelist, page);
	}

	pDest->regionCnt = g_list_length(pSrcMms->regionlist);

	for (i = 0; i < pDest->regionCnt; i++) {
		MMS_SMIL_REGION *region = (MMS_SMIL_REGION *)calloc(1, sizeof(MMS_SMIL_REGION));
		MMS_SMIL_REGION *src_region = (MMS_SMIL_REGION *)get_msg_struct_data((msg_struct_s *)g_list_nth_data(pSrcMms->regionlist, i));
		memcpy(region, src_region, sizeof(MMS_SMIL_REGION));
		pDest->regionlist = g_list_append(pDest->regionlist, region);
	}

	pDest->attachCnt = g_list_length(pSrcMms->attachlist);

	for (i = 0; i < pDest->attachCnt; i++) {
		MMS_ATTACH_S *attach = (MMS_ATTACH_S *)calloc(1, sizeof(MMS_ATTACH_S));
		MMS_ATTACH_S *src_attach = (MMS_ATTACH_S *)get_msg_struct_data((msg_struct_s *)g_list_nth_data(pSrcMms->attachlist, i));
		memcpy(attach, src_attach, sizeof(MMS_ATTACH_S));
		pDest->attachlist = g_list_append(pDest->attachlist, attach);
	}

	pDest->transitionCnt = g_list_length(pSrcMms->transitionlist);

	for (i = 0; i < pDest->transitionCnt; i++) {
		MMS_SMIL_TRANSITION *transition = (MMS_SMIL_TRANSITION *)calloc(1, sizeof(MMS_SMIL_TRANSITION));
		MMS_SMIL_TRANSITION *src_transition = (MMS_SMIL_TRANSITION *)get_msg_struct_data((msg_struct_s *)g_list_nth_data(pSrcMms->transitionlist, i));
		memcpy(transition, src_transition, sizeof(MMS_SMIL_TRANSITION));
		pDest->transitionlist = g_list_append(pDest->transitionlist, transition);
	}

	pDest->metaCnt = g_list_length(pSrcMms->metalist);

	for (i = 0; i < pDest->metaCnt; i++) {
		MMS_SMIL_META *meta = (MMS_SMIL_META *)calloc(1, sizeof(MMS_SMIL_META));
		MMS_SMIL_META *src_meta = (MMS_SMIL_META *)get_msg_struct_data((msg_struct_s *)g_list_nth_data(pSrcMms->metalist, i));
		memcpy(meta, src_meta, sizeof(MMS_SMIL_META));
		pDest->metalist = g_list_append(pDest->metalist, meta);
	}

	memcpy(&pDest->rootlayout, &pSrcMms->rootlayout, sizeof(MMS_SMIL_ROOTLAYOUT));

	memcpy(&pDest->msgAppId, &pSrcMms->msgAppId, sizeof(MMS_APPID_INFO_S));
}

void convert_from_mmsdata(const MMS_MESSAGE_DATA_S *pSrc, msg_struct_s *pDest)
{
	int i, j;
	MMS_DATA_HIDDEN_S *pDestMms = (MMS_DATA_HIDDEN_S *)pDest->data;

	for (i = 0; i < pSrc->pageCnt; i++) {
		msg_struct_s *page_struct = msg_mms_create_struct(MSG_STRUCT_MMS_PAGE);
		MMS_PAGE_S *page = (MMS_PAGE_S *)page_struct->data;

		MMS_PAGE_S *src_page = (MMS_PAGE_S *)g_list_nth_data(pSrc->pagelist, i);
		page->mediaCnt = g_list_length(src_page->medialist);

		for (j = 0; j < page->mediaCnt; j++)
		{
			msg_struct_s *dst_media_s = msg_mms_create_struct(MSG_STRUCT_MMS_MEDIA);

			MMS_MEDIA_S *src_media = (MMS_MEDIA_S *)g_list_nth_data(src_page->medialist, j);

			convert_from_media_data(src_media, dst_media_s);

			page->medialist = g_list_append(page->medialist, dst_media_s);
		}

		page->nDur = src_page->nDur;
		page->nBegin = src_page->nBegin;
		page->nEnd = src_page->nEnd;
		page->nMin = src_page->nMin;
		page->nMax = src_page->nMax;
		page->nRepeat = src_page->nRepeat;

		pDestMms->pagelist = g_list_append(pDestMms->pagelist, page_struct);
	}

	for (i = 0; i < pSrc->regionCnt; i++) {
		msg_struct_s *region_struct = msg_mms_create_struct(MSG_STRUCT_MMS_REGION);
		MMS_SMIL_REGION *region = (MMS_SMIL_REGION *)region_struct->data;
		MMS_SMIL_REGION *src_region = (MMS_SMIL_REGION *)g_list_nth_data(pSrc->regionlist, i);
		memcpy(region, src_region, sizeof(MMS_SMIL_REGION));
		pDestMms->regionlist = g_list_append(pDestMms->regionlist, region_struct);
	}

	for (i = 0; i < pSrc->attachCnt; i++) {
		msg_struct_s *attach_struct = msg_mms_create_struct(MSG_STRUCT_MMS_ATTACH);
		MMS_ATTACH_S *attach = (MMS_ATTACH_S *)attach_struct->data;
		MMS_ATTACH_S *src_attach = (MMS_ATTACH_S *)g_list_nth_data(pSrc->attachlist, i);
		memcpy(attach, src_attach, sizeof(MMS_ATTACH_S));
		pDestMms->attachlist = g_list_append(pDestMms->attachlist, attach_struct);
	}

	for (i = 0; i < pSrc->transitionCnt; i++) {
		msg_struct_s *transition_struct = msg_mms_create_struct(MSG_STRUCT_MMS_TRANSITION);
		MMS_SMIL_TRANSITION *transition = (MMS_SMIL_TRANSITION *)transition_struct->data;
		MMS_SMIL_TRANSITION *src_transition = (MMS_SMIL_TRANSITION *)g_list_nth_data(pSrc->transitionlist, i);
		memcpy(transition, src_transition, sizeof(MMS_SMIL_TRANSITION));
		pDestMms->transitionlist = g_list_append(pDestMms->transitionlist, transition_struct);
	}

	for (i = 0; i < pSrc->metaCnt; i++) {
		msg_struct_s *meta_struct = msg_mms_create_struct(MSG_STRUCT_MMS_META);
		MMS_SMIL_META *meta = (MMS_SMIL_META *)meta_struct->data;
		MMS_SMIL_META *src_meta = (MMS_SMIL_META *)g_list_nth_data(pSrc->metalist, i);

		memcpy(meta, src_meta, sizeof(MMS_SMIL_META));
		pDestMms->metalist = g_list_append(pDestMms->metalist, meta_struct);
	}

	memcpy(&pDestMms->rootlayout, &pSrc->rootlayout, sizeof(MMS_SMIL_ROOTLAYOUT));

	memcpy(&pDestMms->msgAppId, &pSrc->msgAppId, sizeof(MMS_APPID_INFO_S));
}
