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

#include "MsgException.h"
#include "MsgDebug.h"
#include "MsgHandle.h"
#include "msg_private.h"
#include "msg.h"

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
EXPORT_API int msg_add_filter(msg_handle_t handle, const msg_struct_t msg_struct_handle)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct_handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	msg_struct_s *msg_struct = (msg_struct_s *) msg_struct_handle;
	MSG_TYPE_CHECK(msg_struct->type, MSG_STRUCT_FILTER);

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->addFilter((const MSG_FILTER_S *)msg_struct->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


EXPORT_API int msg_update_filter(msg_handle_t handle, const msg_struct_t msg_struct_handle)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct_handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	msg_struct_s *msg_struct = (msg_struct_s *) msg_struct_handle;
	MSG_TYPE_CHECK(msg_struct->type, MSG_STRUCT_FILTER);

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->updateFilter((const MSG_FILTER_S *)msg_struct->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


EXPORT_API int msg_delete_filter(msg_handle_t handle, msg_filter_id_t filter_id)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->deleteFilter(filter_id);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_filter_list(msg_handle_t handle, msg_struct_list_s *filter_list)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_READ_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || filter_list == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getFilterList(filter_list);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


EXPORT_API int msg_set_filter_operation(msg_handle_t handle, bool set_flag)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->setFilterOperation(set_flag);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_filter_operation(msg_handle_t handle, bool *set_flag)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_READ_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || set_flag == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getFilterOperation(set_flag);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


EXPORT_API int msg_set_filter_active(msg_handle_t handle, msg_filter_id_t filter_id, bool active)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->setFilterActivation(filter_id, active);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


int msg_get_filter_info_bool(void *filter, int field, bool *value)
{
	if (!filter)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_FILTER_S *filter_data = (MSG_FILTER_S *)filter;

	switch (field) {
	case MSG_FILTER_ACTIVE_BOOL:
		*value = filter_data->bActive;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}


int msg_get_filter_info_int(void *filter, int field, int *value)
{
	if (!filter)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_FILTER_S *filter_data = (MSG_FILTER_S *)filter;

	switch (field) {
	case MSG_FILTER_ID_INT:
		*value = filter_data->filterId;
		break;
	case MSG_FILTER_TYPE_INT:
		*value = filter_data->filterType;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}


int msg_get_filter_info_str(void *filter, int field, char *value, int size)
{
	if (!filter)
		return MSG_ERR_NULL_POINTER;

	MSG_FILTER_S *filter_data = (MSG_FILTER_S *)filter;

	switch (field) {
	case MSG_FILTER_VALUE_STR:
		strncpy(value, filter_data->filterValue, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}

int msg_set_filter_info_bool(void *filter, int field, bool value)
{
	if (!filter)
		return MSG_ERR_NULL_POINTER;

	msg_error_t err = MSG_SUCCESS;
	MSG_FILTER_S *filter_data = (MSG_FILTER_S *)filter;

	switch (field) {
	case MSG_FILTER_ACTIVE_BOOL:
		filter_data->bActive = value;
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return err;
}

int msg_set_filter_info_int(void *filter, int field, int value)
{
	if (!filter)
		return MSG_ERR_NULL_POINTER;

	msg_error_t err = MSG_SUCCESS;
	MSG_FILTER_S *filter_data = (MSG_FILTER_S *)filter;

	switch (field) {
	case MSG_FILTER_ID_INT:
		filter_data->filterId = value;
		break;
	case MSG_FILTER_TYPE_INT:
		filter_data->filterType = value;
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return err;
}

int msg_set_filter_info_str(void *filter, int field, const char *value, int size)
{
	if (!filter || !value)
		return MSG_ERR_NULL_POINTER;

	msg_error_t err = MSG_SUCCESS;
	MSG_FILTER_S *filter_data = (MSG_FILTER_S *)filter;

	switch (field) {
	case MSG_FILTER_VALUE_STR: {
		int len = (size > MAX_FILTER_VALUE_LEN)?MAX_FILTER_VALUE_LEN:size;
		strncpy(filter_data->filterValue, value, len);
		break;
	}
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return err;
}


