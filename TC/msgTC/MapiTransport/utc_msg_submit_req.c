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

#include "utc_msg_submit_req.h"

static MSG_HANDLE_T msgHandle = NULL;
msg_message_t msgInfo;
MSG_SENDINGOPT_S sendOpt;

void sentStatusCB(MSG_HANDLE_T Handle, MSG_SENT_STATUS_S *pStatus, void *pUserParam)
{
	return;
}

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

	msg_reg_sent_status_callback(msgHandle, &sentStatusCB, (void*)"sent status callback");

	msgInfo = msg_new_message();

	msg_set_message_type(msgInfo, MSG_TYPE_SMS);

	msg_sms_set_message_body(msgInfo, "1234567890", 10);

	msg_add_address(msgInfo, "01000000000", MSG_RECIPIENTS_TYPE_TO);

	memset(&sendOpt, 0x00, sizeof(sendOpt));

	sendOpt.bSetting = false;
}
void cleanup(void)
{
	msg_close_msg_handle(&msgHandle);
}

void utc_msg_submit_req_001()
{
	int err = MSG_SUCCESS;
	MSG_REQUEST_S req;

	memset(&req, 0x00, sizeof(req));

	req.msg = msgInfo;
	req.sendOpt = sendOpt;

	err = msg_submit_req(msgHandle, &req);
	if (err != MSG_SUCCESS)
	{
		tet_printf("err is [%d]", err);
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_submit_req_002()
{
	int err = MSG_SUCCESS;
	MSG_REQUEST_S req;

	memset(&req, 0x00, sizeof(req));

	req.msg = msgInfo;
	req.sendOpt = sendOpt;

	err = msg_submit_req(NULL, &req);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
