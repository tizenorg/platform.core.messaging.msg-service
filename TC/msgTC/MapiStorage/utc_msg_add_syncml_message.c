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
