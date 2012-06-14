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

#include "utc_msg_set_config.h"

static MSG_HANDLE_T msgHandle = NULL;
MSG_SETTING_S setting;

void startup(void)
{
	MSG_ERROR_T err = MSG_SUCCESS;
	err = msg_open_msg_handle(&msgHandle);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_FAIL);
		return;
	}

	memset(&setting, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_GENERAL_OPT_S));

	setting.type = MSG_GENERAL_OPT;

	msg_get_config(msgHandle, &setting);
}
void cleanup(void)
{
	msg_close_msg_handle(&msgHandle);
}

void utc_msg_set_config_001()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = msg_set_config(msgHandle, &setting);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

void utc_msg_set_config_002()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = msg_set_config(NULL, &setting);
	if (err != MSG_SUCCESS)
	{
		tet_result(TET_PASS);
		return;
	}

	tet_result(TET_FAIL);
}
