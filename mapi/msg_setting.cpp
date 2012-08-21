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
#include <MsgException.h>
#include "MsgHandle.h"
#include "MsgDebug.h"

#include "msg_private.h"
#include "msg.h"

/*******************************************************************************
 *								  	 SMSC					   			       *
 *******************************************************************************/

int msg_setting_get_int_value(msg_struct_s *msg_struct, int field, int *value)
{
	int err = MSG_SUCCESS;

	switch (msg_struct->type) {
	case MSG_STRUCT_SETTING_SMSC_OPT :
		*value = msg_get_smsc_opt_int(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_SMSC_INFO :
		*value = msg_get_smsc_info_int(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_CB_OPT :
		*value = msg_get_cb_option_int(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO :
		*value = msg_get_cb_channel_info_int(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_SMS_SEND_OPT :
		*value = msg_get_sms_send_opt_int(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_MMS_SEND_OPT :
		*value = msg_get_mms_send_opt_int(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_MMS_RECV_OPT :
		*value = msg_get_mms_recv_opt_int(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT :
		*value = msg_get_push_msg_opt_int(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_GENERAL_OPT :
		*value = msg_get_general_opt_int(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_MSGSIZE_OPT :
		*value = msg_get_msgsize_opt_int(msg_struct->data, field);
		break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_setting_get_str_value(msg_struct_s *msg_struct, int field, char *src, int size)
{
	int err = MSG_SUCCESS;
	char *ret_str = NULL;

	switch (msg_struct->type)
	{
	case MSG_STRUCT_SETTING_SMSC_INFO :
		ret_str = msg_get_smsc_info_str(msg_struct->data, field);
		if (ret_str == NULL)
			err = MSG_ERR_UNKNOWN;
		else
			strncpy(src, ret_str, size);
		break;
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO :
		ret_str = msg_get_cb_channel_info_str(msg_struct->data, field);
		if (ret_str == NULL)
			err = MSG_ERR_UNKNOWN;
		else
			strncpy(src, ret_str, size);
		break;
	case MSG_STRUCT_SETTING_VOICE_MSG_OPT :
		ret_str = msg_get_voice_msg_opt_str(msg_struct->data, field);
		if (ret_str == NULL)
			err = MSG_ERR_UNKNOWN;
		else
			strncpy(src, ret_str, size);
		break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_setting_get_bool_value(msg_struct_s *msg_struct, int field, bool *value)
{
	int err = MSG_SUCCESS;

	switch (msg_struct->type) {
	case MSG_STRUCT_SETTING_CB_OPT :
		*value = msg_get_cb_option_bool(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO :
		*value = msg_get_cb_channel_info_bool(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_SMS_SEND_OPT :
		*value = msg_get_sms_send_opt_bool(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_MMS_SEND_OPT :
		*value = msg_get_mms_send_opt_bool(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_MMS_RECV_OPT :
		*value = msg_get_mms_recv_opt_bool(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT :
		*value = msg_get_push_msg_opt_bool(msg_struct->data, field);
		break;
	case MSG_STRUCT_SETTING_GENERAL_OPT :
		*value = msg_get_general_opt_bool(msg_struct->data, field);
		break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_setting_get_list_handle(msg_struct_s *msg_struct, int field, void **value)
{
	int err = MSG_SUCCESS;

	switch (msg_struct->type) {
	case MSG_STRUCT_SETTING_SMSC_OPT :
		err = msg_get_smsc_opt_list(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_CB_OPT :
		err = msg_get_cb_option_list(msg_struct->data, field, value);
		break;
	default :
		break;
	}

	return err;
}

int msg_setting_set_int_value(msg_struct_s *msg_struct, int field, int value)
{
	int err = MSG_SUCCESS;

	switch (msg_struct->type) {
	case MSG_STRUCT_SETTING_SMSC_OPT :
		err = msg_set_smsc_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_SMSC_INFO :
		err = msg_set_smsc_info_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_CB_OPT :
		err = msg_set_cb_option_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO :
		err = msg_set_cb_channel_info_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_SMS_SEND_OPT :
		err = msg_set_sms_send_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MMS_SEND_OPT :
		err = msg_set_mms_send_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MMS_RECV_OPT :
		err = msg_set_mms_recv_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT :
		err = msg_set_push_msg_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_GENERAL_OPT :
		err = msg_set_general_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MSGSIZE_OPT :
		err = msg_set_msgsize_opt_int(msg_struct->data, field, value);
		break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_setting_set_str_value(msg_struct_s *msg_struct, int field, char *value, int size)
{
	int err = MSG_SUCCESS;

	switch (msg_struct->type)
	{
	case MSG_STRUCT_SETTING_SMSC_INFO :
		err = msg_set_smsc_info_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO :
		err = msg_set_cb_channel_info_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_SETTING_VOICE_MSG_OPT :
		err = msg_set_voice_msg_opt_str(msg_struct->data, field, value, size);
		break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_setting_set_bool_value(msg_struct_s *msg_struct, int field, bool value)
{
	int err = MSG_SUCCESS;

	switch (msg_struct->type) {
	case MSG_STRUCT_SETTING_CB_OPT :
		err = msg_set_cb_option_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO :
		err = msg_set_cb_channel_info_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_SMS_SEND_OPT :
		err = msg_set_sms_send_opt_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MMS_SEND_OPT :
		err = msg_set_mms_send_opt_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MMS_RECV_OPT :
		err = msg_set_mms_recv_opt_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT :
		err = msg_set_push_msg_opt_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_GENERAL_OPT :
		err = msg_set_general_opt_bool(msg_struct->data, field, value);
		break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}


EXPORT_API int msg_get_smsc_opt(msg_handle_t handle, msg_struct_t msg_struct)
{

	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getSMSCOption(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_smsc_opt(msg_handle_t handle, msg_struct_t msg_struct)
{

	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || msg_struct == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->setSMSCOption(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_smsc_opt_list(void *smsc_opt, int field, void **value)
{
	if (!smsc_opt)
		return MSG_ERR_NULL_POINTER;

	MSG_SMSC_LIST_HIDDEN_S *smsc_opt_data = (MSG_SMSC_LIST_HIDDEN_S *)smsc_opt;

	int ret = MSG_SUCCESS;

	switch (field)
	{
	case MSG_SMSC_LIST_STRUCT :
		*value = (void *)smsc_opt_data->smsc_list;
		break;
	default :
		break;
	}

	return ret;
}

int msg_get_smsc_opt_int(void *smsc_opt, int field)
{
	if (!smsc_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_SMSC_LIST_HIDDEN_S *smsc_opt_data = (MSG_SMSC_LIST_HIDDEN_S *)smsc_opt;

	switch (field)
	{
	case MSG_SMSC_SELECTED_ID_INT :
		ret = smsc_opt_data->selected;
		break;
	default :
		return MSG_ERR_INVALID_PARAMETER;
	}

	return ret;
}

int msg_set_smsc_opt_int(void *smsc_opt, int field, int value)
{
	if (!smsc_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_SMSC_LIST_HIDDEN_S *smsc_opt_data = (MSG_SMSC_LIST_HIDDEN_S *)smsc_opt;

	switch (field)
	{
	case MSG_SMSC_SELECTED_ID_INT :
		ret = smsc_opt_data->selected;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_smsc_info_int(void *smsc_info, int field)
{
	if (!smsc_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_SMSC_DATA_S *smsc_data = (MSG_SMSC_DATA_S *)smsc_info;

	switch (field)
	{
	case MSG_SMSC_ADDR_TON_INT :
		ret = smsc_data->smscAddr.ton;
		break;
	case MSG_SMSC_ADDR_NPI_INT :
		ret = smsc_data->smscAddr.npi;
		break;
	case MSG_SMSC_PID_INT :
		ret = smsc_data->pid;
		break;
	case MSG_SMSC_VAL_PERIOD_INT :
		ret = smsc_data->valPeriod;
		break;
	default :
		return MSG_ERR_INVALID_PARAMETER;
	}

	return ret;
}

int msg_set_smsc_info_int(void *smsc_info, int field, int value)
{
	if (!smsc_info)
		return MSG_ERR_NULL_POINTER;

	int err = MSG_SUCCESS;

	MSG_SMSC_DATA_S *smsc_data = (MSG_SMSC_DATA_S *)smsc_info;

	switch (field)
	{
	case MSG_SMSC_ADDR_TON_INT :
		smsc_data->smscAddr.ton = value;
		break;
	case MSG_SMSC_ADDR_NPI_INT :
		smsc_data->smscAddr.npi = value;
		break;
	case MSG_SMSC_PID_INT :
		smsc_data->pid = value;
		break;
	case MSG_SMSC_VAL_PERIOD_INT :
		smsc_data->valPeriod = value;
		break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

char *msg_get_smsc_info_str(void *smsc_info, int field)
{
	if (!smsc_info)
		return NULL;

	char *ret_str = NULL;

	MSG_SMSC_DATA_S *smsc_data = (MSG_SMSC_DATA_S *)smsc_info;

	switch (field)
	{
	case MSG_SMSC_ADDR_STR :
		ret_str = smsc_data->smscAddr.address;
		break;
	case MSG_SMSC_NAME_STR :
		ret_str = smsc_data->name;
		break;
	default :
		return NULL;
	}

	return ret_str;
}

int msg_set_smsc_info_str(void *smsc_info, int field, char *val, int size)
{
	if (!smsc_info)
		return MSG_ERR_NULL_POINTER;

	int err = MSG_SUCCESS;

	MSG_SMSC_DATA_S *smsc_data = (MSG_SMSC_DATA_S *)smsc_info;

	switch (field)
	{
	case MSG_SMSC_ADDR_STR :
		bzero(smsc_data->smscAddr.address, sizeof(smsc_data->smscAddr.address));
		snprintf(smsc_data->smscAddr.address, sizeof(smsc_data->smscAddr.address), "%s", val);
		break;
	case MSG_SMSC_NAME_STR :
		bzero(smsc_data->name, sizeof(smsc_data->name));
		snprintf(smsc_data->name, sizeof(smsc_data->name), "%s", val);
		break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API int msg_get_cb_opt(msg_handle_t handle, msg_struct_t msg_struct)
{

	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getCBOption(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_cb_opt(msg_handle_t handle, msg_struct_t msg_struct)
{

	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || msg_struct == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->setCBOption(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_cb_option_int(void *cb_opt, int field)
{
	if (!cb_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CBMSG_OPT_HIDDEN_S *cb_opt_data = (MSG_CBMSG_OPT_HIDDEN_S *)cb_opt;

	switch (field)
	{
	case MSG_CB_MAX_SIM_COUNT_INT :
		ret = cb_opt_data->maxSimCnt;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_set_cb_option_int(void *cb_opt, int field, int value)
{
	if (!cb_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CBMSG_OPT_HIDDEN_S *cb_opt_data = (MSG_CBMSG_OPT_HIDDEN_S *)cb_opt;

	switch (field)
	{
	case MSG_CB_MAX_SIM_COUNT_INT :
		cb_opt_data->maxSimCnt = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}


bool msg_get_cb_option_bool(void *cb_opt, int field)
{
	if (!cb_opt)
		return false;

	bool ret = false;

	MSG_CBMSG_OPT_HIDDEN_S *cb_opt_data = (MSG_CBMSG_OPT_HIDDEN_S *)cb_opt;

	switch (field)
	{
	case MSG_CB_RECEIVE_BOOL :
		ret = cb_opt_data->bReceive;
		break;
	case MSG_CB_RECEIVE_ALL_CHANNEL_BOOL :
		ret = cb_opt_data->bAllChannel;
		break;
	case MSG_CB_LANGUAGE_TYPE_ALL_BOOL :
		ret = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_ALL];
		break;
	case MSG_CB_LANGUAGE_TYPE_ENG_BOOL :
		ret = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_ENG];
		break;
	case MSG_CB_LANGUAGE_TYPE_GER_BOOL :
		ret = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_GER];
		break;
	case MSG_CB_LANGUAGE_TYPE_FRE_BOOL :
		ret = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_FRE];
		break;
	case MSG_CB_LANGUAGE_TYPE_ITA_BOOL :
		ret = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_ITA];
		break;
	case MSG_CB_LANGUAGE_TYPE_NED_BOOL :
		ret = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_NED];
		break;
	case MSG_CB_LANGUAGE_TYPE_SPA_BOOL :
		ret = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_SPA];
		break;
	case MSG_CB_LANGUAGE_TYPE_POR_BOOL :
		ret = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_POR];
		break;
	case MSG_CB_LANGUAGE_TYPE_SWE_BOOL :
		ret = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_SWE];
		break;
	case MSG_CB_LANGUAGE_TYPE_TUR_BOOL :
		ret = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_TUR];
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_cb_option_bool(void *cb_opt, int field, bool value)
{
	if (!cb_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CBMSG_OPT_HIDDEN_S *cb_opt_data = (MSG_CBMSG_OPT_HIDDEN_S *)cb_opt;

	switch (field)
	{
	case MSG_CB_RECEIVE_BOOL :
		cb_opt_data->bReceive = value;
		break;
	case MSG_CB_RECEIVE_ALL_CHANNEL_BOOL :
		cb_opt_data->bAllChannel = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_ALL_BOOL :
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_ALL] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_ENG_BOOL :
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_ENG] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_GER_BOOL :
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_GER] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_FRE_BOOL :
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_FRE] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_ITA_BOOL :
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_ITA] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_NED_BOOL :
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_NED] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_SPA_BOOL :
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_SPA] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_POR_BOOL :
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_POR] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_SWE_BOOL :
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_SWE] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_TUR_BOOL :
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_TUR] = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_cb_option_list(void *cb_opt, int field, void **value)
{
	if (!cb_opt)
		return MSG_ERR_NULL_POINTER;

	MSG_CBMSG_OPT_HIDDEN_S *cb_opt_data = (MSG_CBMSG_OPT_HIDDEN_S *)cb_opt;

	int ret = MSG_SUCCESS;

	switch (field)
	{
	case MSG_CB_CHANNEL_LIST_STRUCT :
		*value = (void *)cb_opt_data->channelData;
		break;
	default :
		break;
	}

	return ret;
}

int msg_get_cb_channel_info_int(void *cb_ch_info, int field)
{
	if (!cb_ch_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_ERR_INVALID_PARAMETER;

	MSG_CB_CHANNEL_INFO_S *cb_ch_data = (MSG_CB_CHANNEL_INFO_S *)cb_ch_info;

	switch (field)
	{
	case MSG_CB_CHANNEL_ID_INT :
		ret = cb_ch_data->id;
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_cb_channel_info_int(void *cb_ch_info, int field, int value)
{
	if (!cb_ch_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CB_CHANNEL_INFO_S *cb_ch_data = (MSG_CB_CHANNEL_INFO_S *)cb_ch_info;

	switch (field)
	{
	case MSG_CB_CHANNEL_ID_INT :
		cb_ch_data->id = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

bool msg_get_cb_channel_info_bool(void *cb_ch_info, int field)
{
	if (!cb_ch_info)
		return false;

	bool ret = false;

	MSG_CB_CHANNEL_INFO_S *cb_ch_data = (MSG_CB_CHANNEL_INFO_S *)cb_ch_info;

	switch (field)
	{
	case MSG_CB_CHANNEL_ACTIVATE_BOOL :
		ret = cb_ch_data->bActivate;
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_cb_channel_info_bool(void *cb_ch_info, int field, bool value)
{
	if (!cb_ch_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CB_CHANNEL_INFO_S *cb_ch_data = (MSG_CB_CHANNEL_INFO_S *)cb_ch_info;

	switch (field)
	{
	case MSG_CB_CHANNEL_ACTIVATE_BOOL :
		cb_ch_data->bActivate = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

char *msg_get_cb_channel_info_str(void *cb_ch_info, int field)
{
	if (!cb_ch_info)
		return NULL;

	char *ret_str = NULL;

	MSG_CB_CHANNEL_INFO_S *cb_ch_data = (MSG_CB_CHANNEL_INFO_S *)cb_ch_info;

	switch (field)
	{
	case MSG_CB_CHANNEL_NAME_STR :
		ret_str = cb_ch_data->name;
		break;
	default :
		break;
	}

	return ret_str;
}

int msg_set_cb_channel_info_str(void *cb_ch_info, int field, char *val, int size)
{
	if (!cb_ch_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CB_CHANNEL_INFO_S *cb_ch_data = (MSG_CB_CHANNEL_INFO_S *)cb_ch_info;

	switch (field)
	{
	case MSG_CB_CHANNEL_NAME_STR :
		bzero(cb_ch_data->name, sizeof(cb_ch_data->name));
		snprintf(cb_ch_data->name, sizeof(cb_ch_data->name), "%s", val);
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

EXPORT_API int msg_get_sms_send_opt(msg_handle_t handle, msg_struct_t msg_struct)
{

	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getSmsSendOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_sms_send_opt(msg_handle_t handle, msg_struct_t msg_struct)
{

	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || msg_struct == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->setSmsSendOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_sms_send_opt_int(void *sms_send_opt, int field)
{
	if (!sms_send_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_ERR_INVALID_PARAMETER;

	MSG_SMS_SENDOPT_S *send_opt = (MSG_SMS_SENDOPT_S *)sms_send_opt;

	switch (field)
	{
	case MSG_SMS_SENDOPT_ENCODE_TYPE_INT :
		ret = send_opt->dcs;
		break;
	case MSG_SMS_SENDOPT_NETWORK_MODE_INT :
		ret = send_opt->netMode;
		break;
	case MSG_SMS_SENDOPT_SAVE_STORAGE_INT :
		ret = send_opt->saveStorage;
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_sms_send_opt_int(void *sms_send_opt, int field, int value)
{
	if (!sms_send_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_SMS_SENDOPT_S *send_opt = (MSG_SMS_SENDOPT_S *)sms_send_opt;

	switch (field)
	{
	case MSG_SMS_SENDOPT_ENCODE_TYPE_INT :
		send_opt->dcs = value;
		break;
	case MSG_SMS_SENDOPT_NETWORK_MODE_INT :
		send_opt->netMode = value;
		break;
	case MSG_SMS_SENDOPT_SAVE_STORAGE_INT :
		send_opt->saveStorage = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

bool msg_get_sms_send_opt_bool(void *sms_send_opt, int field)
{
	if (!sms_send_opt)
		return false;

	bool ret = false;

	MSG_SMS_SENDOPT_S *send_opt = (MSG_SMS_SENDOPT_S *)sms_send_opt;

	switch (field)
	{
	case MSG_SMS_SENDOPT_REPLY_PATH_BOOL :
		ret = send_opt->bReplyPath;
		break;
	case MSG_SMS_SENDOPT_DELIVERY_REPORT_BOOL :
		ret = send_opt->bDeliveryReport;
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_sms_send_opt_bool(void *sms_send_opt, int field, bool value)
{
	if (!sms_send_opt)
		return false;

	int ret = MSG_SUCCESS;

	MSG_SMS_SENDOPT_S *send_opt = (MSG_SMS_SENDOPT_S *)sms_send_opt;

	switch (field)
	{
	case MSG_SMS_SENDOPT_REPLY_PATH_BOOL :
		send_opt->bReplyPath = value;
		break;
	case MSG_SMS_SENDOPT_DELIVERY_REPORT_BOOL :
		send_opt->bDeliveryReport = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

EXPORT_API int msg_get_mms_send_opt(msg_handle_t handle, msg_struct_t msg_struct)
{

	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getMmsSendOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_mms_send_opt(msg_handle_t handle, msg_struct_t msg_struct)
{

	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || msg_struct == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->setMmsSendOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}


int msg_get_mms_send_opt_int(void *mms_send_opt, int field)
{
	if (!mms_send_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_ERR_INVALID_PARAMETER;

	MSG_MMS_SENDOPT_S *send_opt = (MSG_MMS_SENDOPT_S *)mms_send_opt;

	switch (field)
	{
	case MSG_MMS_SENDOPT_CLASS_TYPE_INT :
		ret = send_opt->msgClass;
		break;
	case MSG_MMS_SENDOPT_PRIORITY_TYPE_INT :
		ret = send_opt->priority;
		break;
	case MSG_MMS_SENDOPT_EXPIRY_TIME_INT :
		ret = send_opt->expiryTime;
		break;
	case MSG_MMS_SENDOPT_DELIVERY_TIME_INT :
		ret = send_opt->deliveryTime;
		break;
	case MSG_MMS_SENDOPT_CUSTOM_DELIVERY_TIME_INT :
		ret = send_opt->customDeliveryTime;
		break;
	case MSG_MMS_SENDOPT_REPLY_CHARGING_INT :
		ret = send_opt->replyCharging;
		break;
	case MSG_MMS_SENDOPT_REPLY_CHARGING_DEADLINE_INT :
		ret = send_opt->replyChargingDeadline;
		break;
	case MSG_MMS_SENDOPT_REPLY_CHARGING_SIZE_INT :
		ret = send_opt->replyChargingSize;
		break;
	case MSG_MMS_SENDOPT_CREATION_MODE_INT :
		ret = send_opt->creationMode;
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_mms_send_opt_int(void *mms_send_opt, int field, int value)
{
	if (!mms_send_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MMS_SENDOPT_S *send_opt = (MSG_MMS_SENDOPT_S *)mms_send_opt;

	switch (field)
	{
	case MSG_MMS_SENDOPT_CLASS_TYPE_INT :
		send_opt->msgClass = value;
		break;
	case MSG_MMS_SENDOPT_PRIORITY_TYPE_INT :
		send_opt->priority = value;
		break;
	case MSG_MMS_SENDOPT_EXPIRY_TIME_INT :
		send_opt->expiryTime = value;
		break;
	case MSG_MMS_SENDOPT_DELIVERY_TIME_INT :
		send_opt->deliveryTime = value;
		break;
	case MSG_MMS_SENDOPT_CUSTOM_DELIVERY_TIME_INT :
		send_opt->customDeliveryTime = value;
		break;
	case MSG_MMS_SENDOPT_REPLY_CHARGING_INT :
		send_opt->replyCharging = value;
		break;
	case MSG_MMS_SENDOPT_REPLY_CHARGING_DEADLINE_INT :
		send_opt->replyChargingDeadline = value;
		break;
	case MSG_MMS_SENDOPT_REPLY_CHARGING_SIZE_INT :
		send_opt->replyChargingSize = value;
		break;
	case MSG_MMS_SENDOPT_CREATION_MODE_INT :
		send_opt->creationMode = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

bool msg_get_mms_send_opt_bool(void *mms_send_opt, int field)
{
	if (!mms_send_opt)
		return false;

	bool ret = false;

	MSG_MMS_SENDOPT_S *send_opt = (MSG_MMS_SENDOPT_S *)mms_send_opt;

	switch (field)
	{
	case MSG_MMS_SENDOPT_SENDER_VISIBILITY_BOOL :
		ret = send_opt->bSenderVisibility;
		break;
	case MSG_MMS_SENDOPT_DELIVERY_REPORT_BOOL :
		ret = send_opt->bDeliveryReport;
		break;
	case MSG_MMS_SENDOPT_READ_REPLY_BOOL :
		ret = send_opt->bReadReply;
		break;
	case MSG_MMS_SENDOPT_KEEP_COPY_BOOL :
		ret = send_opt->bKeepCopy;
		break;
	case MSG_MMS_SENDOPT_BODY_REPLYING_BOOL :
		ret = send_opt->bBodyReplying;
		break;
	case MSG_MMS_SENDOPT_HIDE_RECIPIENTS_BOOL :
		ret = send_opt->bHideRecipients;
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_mms_send_opt_bool(void *mms_send_opt, int field, bool value)
{
	if (!mms_send_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MMS_SENDOPT_S *send_opt = (MSG_MMS_SENDOPT_S *)mms_send_opt;

	switch (field)
	{
	case MSG_MMS_SENDOPT_SENDER_VISIBILITY_BOOL :
		send_opt->bSenderVisibility = value;
		break;
	case MSG_MMS_SENDOPT_DELIVERY_REPORT_BOOL :
		send_opt->bDeliveryReport = value;
		break;
	case MSG_MMS_SENDOPT_READ_REPLY_BOOL :
		send_opt->bReadReply = value;
		break;
	case MSG_MMS_SENDOPT_KEEP_COPY_BOOL :
		send_opt->bKeepCopy = value;
		break;
	case MSG_MMS_SENDOPT_BODY_REPLYING_BOOL :
		send_opt->bBodyReplying = value;
		break;
	case MSG_MMS_SENDOPT_HIDE_RECIPIENTS_BOOL :
		send_opt->bHideRecipients = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

EXPORT_API int msg_get_mms_recv_opt(msg_handle_t handle, msg_struct_t msg_struct)
{

	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getMmsRecvOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_mms_recv_opt(msg_handle_t handle, msg_struct_t msg_struct)
{

	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || msg_struct == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->setMmsRecvOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_mms_recv_opt_int(void *mms_recv_opt, int field)
{
	if (!mms_recv_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_ERR_INVALID_PARAMETER;

	MSG_MMS_RECVOPT_S *recv_opt = (MSG_MMS_RECVOPT_S *)mms_recv_opt;

	switch (field)
	{
	case MSG_MMS_RECVOPT_HOME_RETRIEVE_TYPE_INT :
		ret = recv_opt->homeNetwork;
		break;
	case MSG_MMS_RECVOPT_ABROAD_RETRIEVE_TYPE_INT :
		ret = recv_opt->abroadNetwok;
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_mms_recv_opt_int(void *mms_recv_opt, int field, int value)
{
	if (!mms_recv_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MMS_RECVOPT_S *recv_opt = (MSG_MMS_RECVOPT_S *)mms_recv_opt;

	switch (field)
	{
	case MSG_MMS_RECVOPT_HOME_RETRIEVE_TYPE_INT :
		recv_opt->homeNetwork = value;
		break;
	case MSG_MMS_RECVOPT_ABROAD_RETRIEVE_TYPE_INT :
		recv_opt->abroadNetwok = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

bool msg_get_mms_recv_opt_bool(void *mms_recv_opt, int field)
{
	if (!mms_recv_opt)
		return false;

	bool ret = false;

	MSG_MMS_RECVOPT_S *recv_opt = (MSG_MMS_RECVOPT_S *)mms_recv_opt;

	switch (field)
	{
	case MSG_MMS_RECVOPT_READ_REPORT_BOOL :
		ret = recv_opt->readReceipt;
		break;
	case MSG_MMS_RECVOPT_DELIVERY_REPORT_BOOL :
		ret = recv_opt->bDeliveryReceipt;
		break;
	case MSG_MMS_RECVOPT_REJECT_UNKNOWN_BOOL :
		ret = recv_opt->bRejectUnknown;
		break;
	case MSG_MMS_RECVOPT_REJECT_ADVERTISEMENT_BOOL :
		ret = recv_opt->bRejectAdvertisement;
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_mms_recv_opt_bool(void *mms_recv_opt, int field, bool value)
{
	if (!mms_recv_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MMS_RECVOPT_S *recv_opt = (MSG_MMS_RECVOPT_S *)mms_recv_opt;

	switch (field)
	{
	case MSG_MMS_RECVOPT_READ_REPORT_BOOL :
		recv_opt->readReceipt = value;
		break;
	case MSG_MMS_RECVOPT_DELIVERY_REPORT_BOOL :
		recv_opt->bDeliveryReceipt = value;
		break;
	case MSG_MMS_RECVOPT_REJECT_UNKNOWN_BOOL :
		recv_opt->bRejectUnknown = value;
		break;
	case MSG_MMS_RECVOPT_REJECT_ADVERTISEMENT_BOOL :
		recv_opt->bRejectAdvertisement = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

EXPORT_API int msg_get_push_msg_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getPushMsgOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_push_msg_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || msg_struct == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->setPushMsgOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_push_msg_opt_int(void *push_msg_opt, int field)
{
	if (!push_msg_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_ERR_INVALID_PARAMETER;

	MSG_PUSHMSG_OPT_S *push_opt = (MSG_PUSHMSG_OPT_S *)push_msg_opt;

	switch (field)
	{
	case MSG_PUSHMSG_SERVICE_TYPE_INT :
		ret = push_opt->serviceType;
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_push_msg_opt_int(void *push_msg_opt, int field, int value)
{
	if (!push_msg_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_PUSHMSG_OPT_S *push_opt = (MSG_PUSHMSG_OPT_S *)push_msg_opt;

	switch (field)
	{
	case MSG_PUSHMSG_SERVICE_TYPE_INT :
		push_opt->serviceType = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

bool msg_get_push_msg_opt_bool(void *push_msg_opt, int field)
{
	if (!push_msg_opt)
		return false;

	bool ret = false;

	MSG_PUSHMSG_OPT_S *push_opt = (MSG_PUSHMSG_OPT_S *)push_msg_opt;

	switch (field)
	{
	case MSG_PUSHMSG_RECEIVE_BOOL :
		ret = push_opt->bReceive;
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_push_msg_opt_bool(void *push_msg_opt, int field, bool value)
{
	if (!push_msg_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_PUSHMSG_OPT_S *push_opt = (MSG_PUSHMSG_OPT_S *)push_msg_opt;

	switch (field)
	{
	case MSG_PUSHMSG_RECEIVE_BOOL :
		push_opt->bReceive = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

EXPORT_API int msg_get_voice_msg_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getVoiceMsgOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_voice_msg_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || msg_struct == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->setVoiceMsgOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

char *msg_get_voice_msg_opt_str(void *voice_msg_opt, int field)
{
	if (!voice_msg_opt)
		return NULL;

	char *ret_str = NULL;

	MSG_VOICEMAIL_OPT_S *voice_opt = (MSG_VOICEMAIL_OPT_S *)voice_msg_opt;

	switch (field)
	{
	case MSG_VOICEMSG_ADDRESS_STR :
		ret_str = voice_opt->mailNumber;
		break;
	default :
		break;
	}

	return ret_str;
}

int msg_set_voice_msg_opt_str(void *voice_msg_opt, int field, char *val, int size)
{
	if (!voice_msg_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_VOICEMAIL_OPT_S *voice_opt = (MSG_VOICEMAIL_OPT_S *)voice_msg_opt;

	switch (field)
	{
	case MSG_VOICEMSG_ADDRESS_STR :
		bzero(voice_opt->mailNumber, sizeof(voice_opt->mailNumber));
		snprintf(voice_opt->mailNumber, sizeof(voice_opt->mailNumber), "%s", val);
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

EXPORT_API int msg_get_general_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getGeneralOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_general_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || msg_struct == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->setGeneralOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_general_opt_int(void *general_opt, int field)
{
	if (!general_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_ERR_INVALID_PARAMETER;

	MSG_GENERAL_OPT_S *opt = (MSG_GENERAL_OPT_S *)general_opt;

	switch (field)
	{
	case MSG_GENERAL_ALERT_TONE_INT :
		ret = opt->alertTone;
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_general_opt_int(void *general_opt, int field, int value)
{
	if (!general_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_GENERAL_OPT_S *opt = (MSG_GENERAL_OPT_S *)general_opt;

	switch (field)
	{
	case MSG_GENERAL_ALERT_TONE_INT :
		opt->alertTone = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

bool msg_get_general_opt_bool(void *general_opt, int field)
{
	if (!general_opt)
		return false;

	int ret = false;

	MSG_GENERAL_OPT_S *opt = (MSG_GENERAL_OPT_S *)general_opt;

	switch (field)
	{
	case MSG_GENERAL_KEEP_COPY_BOOL :
		ret = opt->bKeepCopy;
		break;
	case MSG_GENERAL_AUTO_ERASE_BOOL :
		ret = opt->bAutoErase;
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_general_opt_bool(void *general_opt, int field, bool value)
{
	if (!general_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_GENERAL_OPT_S *opt = (MSG_GENERAL_OPT_S *)general_opt;

	switch (field)
	{
	case MSG_GENERAL_KEEP_COPY_BOOL :
		opt->bKeepCopy = value;
		break;
	case MSG_GENERAL_AUTO_ERASE_BOOL :
		opt->bAutoErase = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

EXPORT_API int msg_get_msgsize_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getMsgSizeOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_msgsize_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || msg_struct == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->setMsgSizeOpt(msg_struct);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_msgsize_opt_int(void *size_opt, int field)
{
	if (!size_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_ERR_INVALID_PARAMETER;

	MSG_MSGSIZE_OPT_S *msg_opt = (MSG_MSGSIZE_OPT_S *)size_opt;

	switch (field)
	{
	case MSG_MESSAGE_SIZE_INT :
		ret = msg_opt->nMsgSize;
		break;
	default :
		break;
	}

	return ret;
}

int msg_set_msgsize_opt_int(void *size_opt, int field, int value)
{
	if (!size_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MSGSIZE_OPT_S *msg_opt = (MSG_MSGSIZE_OPT_S *)size_opt;

	switch (field)
	{
	case MSG_MESSAGE_SIZE_INT :
		msg_opt->nMsgSize = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}
