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

#include "MsgException.h"
#include "MsgDebug.h"
#include "MsgHandle.h"
#include "msg.h"

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
EXPORT_API int msg_add_filter(msg_handle_t handle, const msg_struct_t msg_struct_handle)
{
	msg_error_t err =  MSG_SUCCESS;

	// TODO : check NULL in msg_struct_handle
	msg_struct_s *msg_struct = (msg_struct_s *) msg_struct_handle;

	if (handle == NULL || msg_struct == NULL)
	{
		return -EINVAL;
	}

	if(msg_struct->type != MSG_STRUCT_FILTER)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->addFilter((const MSG_FILTER_S *)msg_struct->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


EXPORT_API int msg_update_filter(msg_handle_t handle, const msg_struct_t msg_struct_handle)
{
	msg_error_t err =  MSG_SUCCESS;

	// TODO : check NULL in msg_struct_handle
	msg_struct_s *msg_struct = (msg_struct_s *) msg_struct_handle;

	if (handle == NULL || msg_struct == NULL)
	{
		return -EINVAL;
	}


	if(msg_struct->type != MSG_STRUCT_FILTER)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->updateFilter((const MSG_FILTER_S *)msg_struct->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


EXPORT_API int msg_delete_filter(msg_handle_t handle, msg_filter_id_t filter_id)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->deleteFilter(filter_id);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_filter_list(msg_handle_t handle, msg_struct_list_s *filter_list)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || filter_list == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getFilterList(filter_list);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


EXPORT_API int msg_set_filter_operation(msg_handle_t handle, bool set_flag)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->setFilterOperation(set_flag);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_filter_operation(msg_handle_t handle, bool *set_flag)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getFilterOperation(set_flag);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


EXPORT_API int msg_set_filter_active(msg_handle_t handle, msg_filter_id_t filter_id, bool active)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->setFilterActivation(filter_id, active);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_FILTER_ERROR;
	}

	return err;
}


bool msg_get_filter_info_bool(void *filter, int field)
{
	if (!filter)
		return MSG_ERR_NULL_POINTER;

	int ret = 0;

	MSG_FILTER_S *filter_data = (MSG_FILTER_S *)filter;

	switch (field)
	{
	case MSG_FILTER_ACTIVE_BOOL :
		ret = filter_data->bActive;
		break;
	default :
		return MSG_ERR_INVALID_PARAMETER;
	}

	return ret;
}


int msg_get_filter_info_int(void *filter, int field)
{
	if (!filter)
		return MSG_ERR_NULL_POINTER;

	int ret = 0;

	MSG_FILTER_S *filter_data = (MSG_FILTER_S *)filter;

	switch (field)
	{
	case MSG_FILTER_ID_INT :
		ret = filter_data->filterId;
		break;
	case MSG_FILTER_TYPE_INT :
		ret = filter_data->filterType;
		break;
	default :
		return MSG_ERR_INVALID_PARAMETER;
	}

	return ret;
}


char *msg_get_filter_info_str(void *filter, int field)
{
	if (!filter)
		return NULL;

	char *ret_str = NULL;

	MSG_FILTER_S *filter_data = (MSG_FILTER_S *)filter;

	switch (field)
	{
	case MSG_FILTER_VALUE_STR :
		ret_str = filter_data->filterValue;
		break;
	default :
		return NULL;
	}

	return ret_str;
}

int msg_set_filter_info_bool(void *filter, int field, bool value)
{
	if (!filter)
		return MSG_ERR_NULL_POINTER;

	msg_error_t err =  MSG_SUCCESS;
	MSG_FILTER_S *filter_data = (MSG_FILTER_S *)filter;

	switch (field)
	{
	case MSG_FILTER_ACTIVE_BOOL :
		filter_data->bActive = value;
		break;
	default :
		return MSG_ERR_INVALID_PARAMETER;
	}

	return err;
}

int msg_set_filter_info_int(void *filter, int field, int value)
{
	if (!filter)
		return MSG_ERR_NULL_POINTER;

	msg_error_t err =  MSG_SUCCESS;
	MSG_FILTER_S *filter_data = (MSG_FILTER_S *)filter;

	switch (field)
	{
	case MSG_FILTER_ID_INT :
		filter_data->filterId = value;
		break;
	case MSG_FILTER_TYPE_INT :
		filter_data->filterType = value;
		break;
	default :
		return MSG_ERR_INVALID_PARAMETER;
	}

	return err;
}

int msg_set_filter_info_str(void *filter, int field, char *value, int size)
{
	if (!filter || !value)
		return MSG_ERR_NULL_POINTER;

	msg_error_t err =  MSG_SUCCESS;
	MSG_FILTER_S *filter_data = (MSG_FILTER_S *)filter;

	switch (field)
	{
	case MSG_FILTER_VALUE_STR :
		strncpy(filter_data->filterValue, value, size);
		break;
	default :
		return MSG_ERR_INVALID_PARAMETER;
	}

	return err;
}


