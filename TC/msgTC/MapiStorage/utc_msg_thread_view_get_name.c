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

#include "utc_msg_thread_view_get_name.h"

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

void utc_msg_thread_view_get_name_001()
{
	int err = MSG_SUCCESS;
	char* name = NULL;

	if(msg_thread_view_list.nCount == 0)
	{
		tet_result(TET_PASS);
		return;
	}

	name = msg_thread_view_get_name(msg_thread_view_list.msgThreadInfo[0]);
	if (name == NULL)
	{
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_thread_view_get_name_002()
{
	int err = MSG_SUCCESS;
	char* name = NULL;

	name =  msg_thread_view_get_name(NULL);
	if (name == NULL)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
