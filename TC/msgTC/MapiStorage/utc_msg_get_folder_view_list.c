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

#include "utc_msg_get_folder_view_list.h"

static MSG_HANDLE_T msgHandle = NULL;
msg_message_t msgInfo;
MSG_SENDINGOPT_S sendOpt;

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

	msg_add_message(msgHandle, msgInfo, &sendOpt);

	msg_set_message_id(msgInfo, 0);
}
void cleanup(void)
{
	msg_close_msg_handle(&msgHandle);
}

void utc_msg_get_folder_view_list_001()
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MSG_SORT_RULE_S sort_rule;
	MSG_LIST_S msg_folder_view_list;

	memset(&sort_rule, 0x00, sizeof(sort_rule));
	memset(&msg_folder_view_list, 0x00, sizeof(msg_folder_view_list));

	err = msg_get_folder_view_list(msgHandle, MSG_DRAFT_ID, &sort_rule, &msg_folder_view_list);
	msg_release_message_list(&msg_folder_view_list);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_get_folder_view_list_002()
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MSG_SORT_RULE_S sort_rule;
	MSG_LIST_S msg_folder_view_list;

	memset(&sort_rule, 0x00, sizeof(sort_rule));
	memset(&msg_folder_view_list, 0x00, sizeof(msg_folder_view_list));

	err = msg_get_folder_view_list(NULL, MSG_DRAFT_ID, &sort_rule, &msg_folder_view_list);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
