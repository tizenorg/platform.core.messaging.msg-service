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


#include "utc_msg_open_msg_handle.h"

void startup(void)
{
}
void cleanup(void)
{
}

void utc_msg_open_msg_handle_001()
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MSG_HANDLE_T msgHandle = NULL;

	err = msg_open_msg_handle(&msgHandle);
	if (err != MSG_SUCCESS)
	{
		tet_printf("utc_msg_open_msg_handle_001 failed");
		tet_result(TET_FAIL);
	}
	else
	{
		tet_printf("utc_msg_open_msg_handle_001 passed");
		tet_result(TET_PASS);
	}

	msg_close_msg_handle(&msgHandle);
}

void utc_msg_open_msg_handle_002()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = msg_open_msg_handle(NULL);
	if (err != MSG_SUCCESS)
	{
		tet_printf("utc_msg_open_msg_handle_001 passed");
		tet_result(TET_PASS);
	}
	else
	{
		tet_printf("utc_msg_open_msg_handle_001 failed");
		tet_result(TET_FAIL);
	}

}
