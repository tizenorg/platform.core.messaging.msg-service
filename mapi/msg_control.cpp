/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include <errno.h>
#include <privacy_checker_client.h>

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
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	//Privilege check
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_READ_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
	{
		return MSG_ERR_PERMISSION_DENIED;
	}

	if (handle == NULL)
	{
		MSG_FATAL("Input Paramter is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}
	MsgHandle* pHandle = new MsgHandle();

	// Create MsgHandle
	*handle = (msg_handle_t)pHandle;

	if (*handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	try
	{
		// Connect to Socket
		pHandle->openHandle();
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());

		// Destroy MsgHandle
		delete (MsgHandle*)(*handle);
		(*handle) = NULL;

		if (e.errorCode() == MsgException::SERVER_READY_ERROR)
			return MSG_ERR_SERVER_NOT_READY;
		else if(e.errorCode() == MsgException::SECURITY_ERROR)
			return MSG_ERR_PERMISSION_DENIED;
		else
			return MSG_ERR_COMMUNICATION_ERROR;
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_close_msg_handle(msg_handle_t *handle)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	//Privilege check
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_READ_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
	{
		return MSG_ERR_PERMISSION_DENIED;
	}

	if (handle == NULL || *handle == NULL)
	{
		MSG_FATAL("Input Paramter is NULL");
		return MSG_ERR_INVALID_PARAMETER;
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

