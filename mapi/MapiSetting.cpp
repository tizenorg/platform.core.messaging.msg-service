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

