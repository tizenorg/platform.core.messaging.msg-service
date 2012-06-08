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

#include "utc_msg_mms_add_attachment.h"

static MSG_HANDLE_T msgHandle = NULL;
msg_message_t msgInfo;
MMS_MESSAGE_DATA_S*	 mms_data;

void startup(void)
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MMS_PAGE_S* 	page[2];
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
}
void cleanup(void)
{
	msg_mms_destroy_message(mms_data);
	msg_close_msg_handle(&msgHandle);
}

void utc_msg_mms_add_attachment_001()
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MMS_ATTACH_S*	attach = NULL;

	attach = msg_mms_add_attachment(mms_data, (char*)"/opt/etc/msg-service/P091120_104633.jpg");
	if (attach == NULL)
	{
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_mms_add_attachment_002()
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MMS_ATTACH_S*	attach = NULL;

	attach = msg_mms_add_attachment(NULL, (char*)"/opt/etc/msg-service/P091120_104633.jpg");
	if (attach == NULL)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
