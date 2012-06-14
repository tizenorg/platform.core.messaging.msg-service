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

#include "utc_msg_thread_view_get_unread_cnt.h"

static MSG_HANDLE_T msgHandle = NULL;
msg_message_t msgInfo;
MSG_SENDINGOPT_S sendOpt;
MSG_THREAD_VIEW_LIST_S msg_thread_view_list;

void startup(void)
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MSG_SORT_RULE_S sort_rule;

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

	memset(&msg_thread_view_list, 0x00, sizeof(msg_thread_view_list));
	memset(&sort_rule, 0x00, sizeof(sort_rule));

	msg_get_thread_view_list(msgHandle, NULL, &msg_thread_view_list);
}
void cleanup(void)
{
	msg_release_thread_view_list(&msg_thread_view_list);
	msg_close_msg_handle(&msgHandle);
}

void utc_msg_thread_view_get_unread_cnt_001()
{
	int err = MSG_SUCCESS;

	if(msg_thread_view_list.nCount == 0)
	{
		tet_result(TET_PASS);
		return;
	}

	err = msg_thread_view_get_unread_cnt(msg_thread_view_list.msgThreadInfo[0]);
	if (err < MSG_SUCCESS)
	{
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_thread_view_get_unread_cnt_002()
{
	int err = MSG_SUCCESS;

	err =  msg_thread_view_get_unread_cnt(NULL);
	if (err < MSG_SUCCESS)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
