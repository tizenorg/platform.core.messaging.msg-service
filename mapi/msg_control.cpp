/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
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

#include "msg_private.h"
#include "msg.h"

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
EXPORT_API int msg_open_msg_handle(msg_handle_t *handle)
{
	if (handle == NULL)
	{
		MSG_FATAL("Input Paramter is NULL");
		return -EINVAL;
	}
	MsgHandle* pHandle = new MsgHandle();

	// Create MsgHandle
	*handle = (msg_handle_t)pHandle;

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


EXPORT_API int msg_close_msg_handle(msg_handle_t *handle)
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

