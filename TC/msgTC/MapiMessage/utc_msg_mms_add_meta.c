/*
*
* Copyright (c) 2000-2012 Samsung Electronics Co., Ltd. All Rights Reserved.
*
* This file is part of msg-service.
*
* Contact: Jaeyun Jeong <jyjeong@samsung.com>
*          Sangkoo Kim <sangkoo.kim@samsung.com>
*          Seunghwan Lee <sh.cat.lee@samsung.com>
*          SoonMin Jung <sm0415.jung@samsung.com>
*          Jae-Young Lee <jy4710.lee@samsung.com>
*          KeeBum Kim <keebum.kim@samsung.com>
*
* PROPRIETARY/CONFIDENTIAL
*
* This software is the confidential and proprietary information of
* SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered
* into with SAMSUNG ELECTRONICS.
*
* SAMSUNG make no representations or warranties about the suitability
* of the software, either express or implied, including but not limited
* to the implied warranties of merchantability, fitness for a particular
* purpose, or non-infringement. SAMSUNG shall not be liable for any
* damages suffered by licensee as a result of using, modifying or
* distributing this software or its derivatives.
*
*/

#include "utc_msg_mms_add_meta.h"

static MSG_HANDLE_T msgHandle = NULL;
msg_message_t msgInfo;
MMS_MESSAGE_DATA_S*	 mms_data;
MMS_SMIL_META meta;
MMS_PAGE_S* page = NULL;

void startup(void)
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MMS_MEDIA_S*	media[5];
	MMS_ATTACH_S*	attachment[5];
	int				nSize;

	err = msg_open_msg_handle(&msgHandle);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_FAIL);
		return;
	}

	msgInfo = msg_new_message();
	mms_data = msg_mms_create_message();

	memset(&meta, 0x00, sizeof(MMS_SMIL_META));

	page = msg_mms_add_page(mms_data, 5440);
}
void cleanup(void)
{
	msg_mms_destroy_message(mms_data);
	msg_close_msg_handle(&msgHandle);
}

void utc_msg_mms_add_meta_001()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = msg_mms_add_meta(mms_data, &meta);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_mms_add_meta_002()
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MMS_MEDIA_S*	media = NULL;

	err = msg_mms_add_meta(NULL, &meta);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
