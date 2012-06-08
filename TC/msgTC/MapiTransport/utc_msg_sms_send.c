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

#include "utc_msg_sms_send.h"

static MSG_HANDLE_T msgHandle = NULL;

void sentStatusCB(MSG_SENT_STATUS_S *pStatus, void *pUserParam)
{
	return;
}

void startup(void)
{

}
void cleanup(void)
{

}

void utc_msg_sms_send_001()
{
	int err = MSG_SUCCESS;

	err = msg_sms_send("01000000000,01000000000", "1234567890", sentStatusCB, NULL);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_sms_send_002()
{
	int err = MSG_SUCCESS;

	err = msg_sms_send(NULL, "1234567890", sentStatusCB, NULL);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
