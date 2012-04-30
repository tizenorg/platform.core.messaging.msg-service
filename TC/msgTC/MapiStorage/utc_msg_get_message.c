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

#include "utc_msg_get_message.h"

static MSG_HANDLE_T msgHandle = NULL;
msg_message_t msgInfo;
MSG_SENDINGOPT_S sendOpt;
int msgId = 0;

void startup(void)
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MSG_LIST_S folder_list;

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

	msg_get_folder_view_list(msgHandle, MSG_DRAFT_ID, NULL, &folder_list);

	msgId = msg_get_message_id(folder_list.msgInfo[0]);
}
void cleanup(void)
{
	msg_close_msg_handle(&msgHandle);
}

void utc_msg_get_message_001()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = msg_get_message(msgHandle, msgId, msgInfo, &sendOpt);
	if (err != MSG_SUCCESS)
	{
		tet_printf("err is [%d]", err);
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_get_message_002()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = msg_get_message(NULL, msgId, msgInfo, &sendOpt);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
