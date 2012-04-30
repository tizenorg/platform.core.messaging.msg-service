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

#include "utc_msg_mms_get_transition.h"

static MSG_HANDLE_T msgHandle = NULL;
msg_message_t msgInfo;
MMS_MESSAGE_DATA_S*	 mms_data;

void startup(void)
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MMS_PAGE_S* 	page[2];
	MMS_MEDIA_S*	media[5];

	err = msg_open_msg_handle(&msgHandle);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_FAIL);
		return;
	}

	msgInfo = msg_new_message();
	mms_data = msg_mms_create_message();

	msg_mms_set_rootlayout(mms_data, 100, 100, 0xffffff);
	msg_mms_add_region(mms_data, "Image", 0, 50, 100, 50, 0xffffff);
	msg_mms_add_region(mms_data, "Text", 0, 0, 100, 50, 0xffffff);

	//------------>  1st Slide Composing
	page[0] = msg_mms_add_page(mms_data, 5440);

	media[0] = msg_mms_add_media(page[0], MMS_SMIL_MEDIA_IMG, "Image", (char*)"/opt/etc/msg-service/P091120_104633.jpg");
	media[1] = msg_mms_add_media(page[0], MMS_SMIL_MEDIA_AUDIO, NULL, (char*)"/opt/etc/msg-service/audio.amr");
	media[2] = msg_mms_add_media(page[0], MMS_SMIL_MEDIA_TEXT, "Text", (char*)"/opt/etc/msg-service/Temp0_2.txt");
	media[2]->sMedia.sText.nColor = 0x000000;
	media[2]->sMedia.sText.nSize = MMS_SMIL_FONT_SIZE_NORMAL;
	media[2]->sMedia.sText.bBold = true;

	//------------>  2nd Slide Composing
	page[1] = msg_mms_add_page(mms_data, 4544);

	media[3] = msg_mms_add_media(page[1], MMS_SMIL_MEDIA_TEXT, "Text", (char*)"/opt/etc/msg-service/Temp1_0.txt");
	media[3]->sMedia.sText.nColor = 0x000000;
	media[3]->sMedia.sText.nSize = MMS_SMIL_FONT_SIZE_NORMAL;
	media[3]->sMedia.sText.bItalic = true;
	media[4] = msg_mms_add_media(page[1], MMS_SMIL_MEDIA_VIDEO, "Text", (char*)"/opt/etc/msg-service/V091120_104905.3gp");
	strncpy(media[4]->szAlt, "Video Load Fail", MAX_SMIL_ALT_LEN-1);

	MMS_SMIL_TRANSITION transition;

	memset(&transition, 0x00, sizeof(transition));

	msg_mms_add_transition(mms_data, &transition);
}
void cleanup(void)
{
	msg_mms_destroy_message(mms_data);
	msg_close_msg_handle(&msgHandle);
}

void utc_msg_mms_get_transition_001()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	MMS_SMIL_TRANSITION* pTrans = msg_mms_get_transition(mms_data, 0);
	if (pTrans == NULL)
	{
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_mms_get_transition_002()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	MMS_SMIL_TRANSITION* pTrans = msg_mms_get_transition(mms_data, -1);
	if (pTrans == NULL)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
