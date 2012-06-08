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

#include "utc_msg_mms_get_media.h"

static MSG_HANDLE_T msgHandle = NULL;
msg_message_t msgInfo;
MMS_MESSAGE_DATA_S*	 mms_data;
MMS_PAGE_S* 	page[2];
void startup(void)
{
	MSG_ERROR_T err = MSG_SUCCESS;
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

	msg_set_message_type(msgInfo, MSG_TYPE_MMS);

}
void cleanup(void)
{
	msg_mms_destroy_message(mms_data);
	msg_close_msg_handle(&msgHandle);
}

void utc_msg_mms_get_media_001()
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MMS_MEDIA_S* tmpMedia = NULL;

	tmpMedia = msg_mms_get_media(page[0], 0);
	if (tmpMedia == NULL)
	{
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_mms_get_media_002()
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MMS_MEDIA_S* tmpMedia = NULL;

	tmpMedia = msg_mms_get_media(page[0], -1);
	if (tmpMedia == NULL)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
