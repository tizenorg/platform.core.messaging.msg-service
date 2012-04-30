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

#include "utc_msg_add_syncml_message.h"

static MSG_HANDLE_T msgHandle = NULL;
msg_message_t msgInfo;
MSG_SYNCML_MESSAGE_S syncMLMsg;

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

	memset(&syncMLMsg, 0x00, sizeof(MSG_SYNCML_MESSAGE_S));

	syncMLMsg.extId = 11;
	syncMLMsg.pinCode = 1111;

	syncMLMsg.msg = msg_new_message();

	msg_set_folder_id(syncMLMsg.msg, MSG_INBOX_ID);
	msg_set_message_type(syncMLMsg.msg, MSG_TYPE_SMS_SYNCML);
	msg_set_network_status(syncMLMsg.msg, MSG_NETWORK_RECEIVED);
	msg_set_direction_info(syncMLMsg.msg, MSG_DIRECTION_TYPE_MT);

	msg_add_address(syncMLMsg.msg, "+1004", MSG_RECIPIENTS_TYPE_TO);

	char msg_body[]="SyncML Message";
	msg_sms_set_message_body(syncMLMsg.msg, msg_body, strlen(msg_body));
}
void cleanup(void)
{
	msg_close_msg_handle(&msgHandle);
}

void utc_msg_add_syncml_message_001()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = msg_add_syncml_message(msgHandle, &syncMLMsg);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_add_syncml_message_002()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = msg_add_syncml_message(NULL, &syncMLMsg);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
