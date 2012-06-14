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

#include <errno.h>

#include "MsgHandle.h"
#include "MsgDebug.h"
#include "MsgException.h"
#include "MapiSetting.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
EXPORT_API int msg_set_config(MSG_HANDLE_T handle, const MSG_SETTING_S *setting)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || setting == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->setConfig(setting);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_WRITE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_config(MSG_HANDLE_T handle, MSG_SETTING_S *setting)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || setting == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getConfig(setting);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

