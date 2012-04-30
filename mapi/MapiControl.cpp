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
#include "MapiControl.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
EXPORT_API int msg_open_msg_handle(MSG_HANDLE_T *handle)
{
	MsgHandle* pHandle = new MsgHandle();

	if (handle == NULL)
	{
		MSG_FATAL("Input Paramter is NULL");
		return -EINVAL;
	}

	// Create MsgHandle
	*handle = (MSG_HANDLE_T)pHandle;

	if (*handle == NULL)
		return -EINVAL;

	try
	{
		// Connect to Socket
		pHandle->openHandle();
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());

		if (e.errorCode() == MsgException::SERVER_READY_ERROR)
			return MSG_ERR_SERVER_NOT_READY;
		else
			return MSG_ERR_COMMUNICATION_ERROR;
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_close_msg_handle(MSG_HANDLE_T *handle)
{
	if (handle == NULL || *handle == NULL)
	{
		MSG_FATAL("Input Paramter is NULL");
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)(*handle);

	try
	{
		// Disconnect to Socket
		pHandle->closeHandle(pHandle);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_COMMUNICATION_ERROR;
	}

	// Destroy MsgHandle
	delete (MsgHandle*)(*handle);
	(*handle) = NULL;

	return MSG_SUCCESS;
}

