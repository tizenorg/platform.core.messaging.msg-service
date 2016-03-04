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

	if (handle == NULL) {
		MSG_FATAL("Input Parameter is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}
	/* Create MsgHandle */
	MsgHandle* pHandle = new MsgHandle();

	if (pHandle == NULL)
		return MSG_ERR_MEMORY_ERROR;
	*handle = (msg_handle_t)pHandle;

	try {
		/* Connect to Socket */
		pHandle->openHandle();
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());

		/* Destroy MsgHandle */
		delete pHandle;
		*handle = NULL;

		if (e.errorCode() == MsgException::SERVER_READY_ERROR)
			return MSG_ERR_SERVER_NOT_READY;
		else if (e.errorCode() == MsgException::SECURITY_ERROR)
			return MSG_ERR_PERMISSION_DENIED;
		else
			return MSG_ERR_COMMUNICATION_ERROR;
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_close_msg_handle(msg_handle_t *handle)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);

	if (handle == NULL || *handle == NULL) {
		MSG_FATAL("Input Parameter is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	MsgHandle* pHandle = (MsgHandle*)(*handle);

	try {
		/* Disconnect from Socket */
		pHandle->closeHandle(pHandle);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_COMMUNICATION_ERROR;
	}

	/* Destroy MsgHandle */
	delete pHandle;
	*handle = NULL;

	return MSG_SUCCESS;
}

