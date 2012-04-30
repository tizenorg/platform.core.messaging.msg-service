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

#include "utc_msg_count_msg_by_contact.h"

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

void utc_msg_count_msg_by_contact_001()
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MSG_THREAD_LIST_INDEX_S addr_info;
	MSG_THREAD_COUNT_INFO_S msg_thread_count_list;

	memset(&addr_info, 0x00, sizeof(addr_info));
	memset(&msg_thread_count_list, 0x00, sizeof(msg_thread_count_list));

	err = msg_count_msg_by_contact(msgHandle, &addr_info, &msg_thread_count_list);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_count_msg_by_contact_002()
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MSG_THREAD_LIST_INDEX_S addr_info;
	MSG_THREAD_COUNT_INFO_S msg_thread_count_list;

	memset(&addr_info, 0x00, sizeof(addr_info));
	memset(&msg_thread_count_list, 0x00, sizeof(msg_thread_count_list));

	err = msg_count_msg_by_contact(NULL, &addr_info, &msg_thread_count_list);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
