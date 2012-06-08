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

#include "utc_msg_update_read_status.h"

static MSG_HANDLE_T msgHandle = NULL;
msg_message_t msgInfo;
MSG_SENDINGOPT_S sendOpt;
int msgId = 0;

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

	sendOpt.bSetting = false;

	msg_add_address(msgInfo, "01000000000", MSG_RECIPIENTS_TYPE_TO);

	msgId = msg_add_message(msgHandle, msgInfo, &sendOpt);

	tet_printf("msgId is [%d]", msgId);

	//msg_get_folder_view_list(msgHandle, MSG_DRAFT_ID, NULL, &folder_list);

	//msgId = msg_folder_view_get_message_id(folder_list.msgCommInfo[0]);

}
void cleanup(void)
{
	msg_close_msg_handle(&msgHandle);
}

void utc_msg_update_read_status_001()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = msg_update_read_status(msgHandle, msgId, true);
	if (err != MSG_SUCCESS)
	{
		tet_printf("err is [%d]", err);
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_update_read_status_002()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = msg_update_read_status(NULL, msgId, true);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
