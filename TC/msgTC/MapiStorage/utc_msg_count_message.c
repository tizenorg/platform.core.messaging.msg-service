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

#include "utc_msg_count_message.h"

static MSG_HANDLE_T msgHandle = NULL;
msg_message_t msgInfo;
MSG_SENDINGOPT_S sendOpt;
MSG_COUNT_INFO_S count_info;

void startup(void)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = msg_open_msg_handle(&msgHandle);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_FAIL);
		return;
	}

	msgInfo = msg_new_message();

	memset(&sendOpt, 0x00, sizeof(sendOpt));
	memset(&count_info, 0x00, sizeof(count_info));

	sendOpt.bSetting = false;

	msg_add_message(msgHandle, msgInfo, &sendOpt);

	msg_set_message_id(msgInfo, 0);
}
void cleanup(void)
{
	msg_close_msg_handle(&msgHandle);
}

void utc_msg_count_message_001()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = msg_count_message(msgHandle, MSG_DRAFT_ID, &count_info);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_count_message_002()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = msg_count_message(NULL, MSG_DRAFT_ID, &count_info);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
