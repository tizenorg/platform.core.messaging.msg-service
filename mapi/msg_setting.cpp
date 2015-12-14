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
	case MSG_STRUCT_SETTING_SMSC_OPT:
		err = msg_get_smsc_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_SMSC_INFO:
		err = msg_get_smsc_info_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_CB_OPT:
		err = msg_get_cb_option_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO:
		err = msg_get_cb_channel_info_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_SMS_SEND_OPT:
		err = msg_get_sms_send_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MMS_SEND_OPT:
		err = msg_get_mms_send_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MMS_RECV_OPT:
		err = msg_get_mms_recv_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT:
		err = msg_get_push_msg_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_GENERAL_OPT:
		err = msg_get_general_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MSGSIZE_OPT:
		err = msg_get_msgsize_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_VOICE_MSG_OPT:
		err = msg_get_voice_msg_opt_int(msg_struct->data, field, value);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_setting_get_str_value(msg_struct_s *msg_struct, int field, char *src, int size)
{
	int err = MSG_SUCCESS;

	switch (msg_struct->type) {
	case MSG_STRUCT_SETTING_SMSC_INFO:
		err = msg_get_smsc_info_str(msg_struct->data, field, src, size);
		break;
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO:
		err = msg_get_cb_channel_info_str(msg_struct->data, field, src, size);
		break;
	case MSG_STRUCT_SETTING_VOICE_MSG_OPT:
		err = msg_get_voice_msg_opt_str(msg_struct->data, field, src, size);
		break;
	case MSG_STRUCT_SETTING_GENERAL_OPT:
		err = msg_get_general_opt_str(msg_struct->data, field, src, size);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_setting_get_bool_value(msg_struct_s *msg_struct, int field, bool *value)
{
	int err = MSG_SUCCESS;

	switch (msg_struct->type) {
	case MSG_STRUCT_SETTING_CB_OPT:
		err = msg_get_cb_option_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO:
		err = msg_get_cb_channel_info_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_SMS_SEND_OPT:
		err = msg_get_sms_send_opt_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MMS_SEND_OPT:
		err = msg_get_mms_send_opt_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MMS_RECV_OPT:
		err = msg_get_mms_recv_opt_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT:
		err = msg_get_push_msg_opt_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_GENERAL_OPT:
		err = msg_get_general_opt_bool(msg_struct->data, field, value);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_setting_get_list_handle(msg_struct_s *msg_struct, int field, void **value)
{
	int err = MSG_SUCCESS;

	switch (msg_struct->type) {
	case MSG_STRUCT_SETTING_SMSC_OPT:
		err = msg_get_smsc_opt_list(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_CB_OPT:
		err = msg_get_cb_option_list(msg_struct->data, field, value);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_setting_set_int_value(msg_struct_s *msg_struct, int field, int value)
{
	int err = MSG_SUCCESS;

	switch (msg_struct->type) {
	case MSG_STRUCT_SETTING_SMSC_OPT:
		err = msg_set_smsc_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_SMSC_INFO:
		err = msg_set_smsc_info_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_CB_OPT:
		err = msg_set_cb_option_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO:
		err = msg_set_cb_channel_info_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_SMS_SEND_OPT:
		err = msg_set_sms_send_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MMS_SEND_OPT:
		err = msg_set_mms_send_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MMS_RECV_OPT:
		err = msg_set_mms_recv_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT:
		err = msg_set_push_msg_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_GENERAL_OPT:
		err = msg_set_general_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MSGSIZE_OPT:
		err = msg_set_msgsize_opt_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_VOICE_MSG_OPT:
		err = msg_set_voice_msg_opt_int(msg_struct->data, field, value);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_setting_set_str_value(msg_struct_s *msg_struct, int field, char *value, int size)
{
	int err = MSG_SUCCESS;

	switch (msg_struct->type) {
	case MSG_STRUCT_SETTING_SMSC_INFO:
		err = msg_set_smsc_info_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO:
		err = msg_set_cb_channel_info_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_SETTING_VOICE_MSG_OPT:
		err = msg_set_voice_msg_opt_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_SETTING_GENERAL_OPT:
		err = msg_set_general_opt_str(msg_struct->data, field, value, size);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_setting_set_bool_value(msg_struct_s *msg_struct, int field, bool value)
{
	int err = MSG_SUCCESS;

	switch (msg_struct->type) {
	case MSG_STRUCT_SETTING_CB_OPT:
		err = msg_set_cb_option_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO:
		err = msg_set_cb_channel_info_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_SMS_SEND_OPT:
		err = msg_set_sms_send_opt_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MMS_SEND_OPT:
		err = msg_set_mms_send_opt_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_MMS_RECV_OPT:
		err = msg_set_mms_recv_opt_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT:
		err = msg_set_push_msg_opt_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SETTING_GENERAL_OPT:
		err = msg_set_general_opt_bool(msg_struct->data, field, value);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}


EXPORT_API int msg_get_smsc_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);

	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_READ_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getSMSCOption(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_smsc_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);

	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->setSMSCOption(msg_struct);
	} catch (MsgException& e) {
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

	switch (field) {
	case MSG_SMSC_LIST_STRUCT:
		*value = (void *)smsc_opt_data->smsc_list;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_smsc_opt_int(void *smsc_opt, int field, int *value)
{
	if (!smsc_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_SMSC_LIST_HIDDEN_S *smsc_opt_data = (MSG_SMSC_LIST_HIDDEN_S *)smsc_opt;

	switch (field) {
	case MSG_SMSC_SELECTED_ID_INT:
		*value = smsc_opt_data->selected;
		break;
	case MSG_SMSC_LIST_SIM_INDEX_INT:
		*value  = smsc_opt_data->simIndex;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_set_smsc_opt_int(void *smsc_opt, int field, int value)
{
	if (!smsc_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_SMSC_LIST_HIDDEN_S *smsc_opt_data = (MSG_SMSC_LIST_HIDDEN_S *)smsc_opt;

	switch (field) {
	case MSG_SMSC_SELECTED_ID_INT:
		smsc_opt_data->selected = value;
		break;
	case MSG_SMSC_LIST_INDEX_INT:
		smsc_opt_data->index = value;
		break;
	case MSG_SMSC_LIST_SIM_INDEX_INT:
		smsc_opt_data->simIndex = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_smsc_info_int(void *smsc_info, int field, int *value)
{
	if (!smsc_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_SMSC_DATA_S *smsc_data = (MSG_SMSC_DATA_S *)smsc_info;

	switch (field) {
	case MSG_SMSC_ADDR_TON_INT:
		*value = smsc_data->smscAddr.ton;
		break;
	case MSG_SMSC_ADDR_NPI_INT:
		*value = smsc_data->smscAddr.npi;
		break;
	case MSG_SMSC_PID_INT:
		*value = smsc_data->pid;
		break;
	case MSG_SMSC_VAL_PERIOD_INT:
		*value = smsc_data->valPeriod;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_set_smsc_info_int(void *smsc_info, int field, int value)
{
	if (!smsc_info)
		return MSG_ERR_NULL_POINTER;

	int err = MSG_SUCCESS;

	MSG_SMSC_DATA_S *smsc_data = (MSG_SMSC_DATA_S *)smsc_info;

	switch (field) {
	case MSG_SMSC_ADDR_TON_INT:
		smsc_data->smscAddr.ton = value;
		break;
	case MSG_SMSC_ADDR_NPI_INT:
		smsc_data->smscAddr.npi = value;
		break;
	case MSG_SMSC_PID_INT:
		smsc_data->pid = value;
		break;
	case MSG_SMSC_VAL_PERIOD_INT:
		smsc_data->valPeriod = value;
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_get_smsc_info_str(void *smsc_info, int field, char *value, int size)
{
	if (!smsc_info)
		return MSG_ERR_NULL_POINTER;

	MSG_SMSC_DATA_S *smsc_data = (MSG_SMSC_DATA_S *)smsc_info;

	switch (field) {
	case MSG_SMSC_ADDR_STR:
		strncpy(value, smsc_data->smscAddr.address, size);
		break;
	case MSG_SMSC_NAME_STR:
		strncpy(value, smsc_data->name, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}

int msg_set_smsc_info_str(void *smsc_info, int field, char *val, int size)
{
	if (!smsc_info)
		return MSG_ERR_NULL_POINTER;

	int err = MSG_SUCCESS;

	MSG_SMSC_DATA_S *smsc_data = (MSG_SMSC_DATA_S *)smsc_info;

	switch (field) {
	case MSG_SMSC_ADDR_STR:
		bzero(smsc_data->smscAddr.address, sizeof(smsc_data->smscAddr.address));
		snprintf(smsc_data->smscAddr.address, sizeof(smsc_data->smscAddr.address), "%s", val);
		break;
	case MSG_SMSC_NAME_STR:
		bzero(smsc_data->name, sizeof(smsc_data->name));
		snprintf(smsc_data->name, sizeof(smsc_data->name), "%s", val);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API int msg_get_cb_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);

	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_READ_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getCBOption(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_cb_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);

	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->setCBOption(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_cb_option_int(void *cb_opt, int field, int *value)
{
	if (!cb_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CBMSG_OPT_HIDDEN_S *cb_opt_data = (MSG_CBMSG_OPT_HIDDEN_S *)cb_opt;

	switch (field) {
	case MSG_CB_MAX_SIM_COUNT_INT:
		*value = cb_opt_data->maxSimCnt;
		break;
	case MSG_CB_SIM_INDEX_INT:
		*value = cb_opt_data->simIndex;
		break;
	default:
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

	switch (field) {
	case MSG_CB_MAX_SIM_COUNT_INT:
		cb_opt_data->maxSimCnt = value;
		break;
	case MSG_CB_SIM_INDEX_INT:
		cb_opt_data->simIndex = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}


int msg_get_cb_option_bool(void *cb_opt, int field, bool *value)
{
	if (!cb_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CBMSG_OPT_HIDDEN_S *cb_opt_data = (MSG_CBMSG_OPT_HIDDEN_S *)cb_opt;

	switch (field) {
	case MSG_CB_RECEIVE_BOOL:
		*value = cb_opt_data->bReceive;
		break;
	case MSG_CB_LANGUAGE_TYPE_ALL_BOOL:
		*value = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_ALL];
		break;
	case MSG_CB_LANGUAGE_TYPE_ENG_BOOL:
		*value = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_ENG];
		break;
	case MSG_CB_LANGUAGE_TYPE_GER_BOOL:
		*value = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_GER];
		break;
	case MSG_CB_LANGUAGE_TYPE_FRE_BOOL:
		*value = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_FRE];
		break;
	case MSG_CB_LANGUAGE_TYPE_ITA_BOOL:
		*value = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_ITA];
		break;
	case MSG_CB_LANGUAGE_TYPE_NED_BOOL:
		*value = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_NED];
		break;
	case MSG_CB_LANGUAGE_TYPE_SPA_BOOL:
		*value = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_SPA];
		break;
	case MSG_CB_LANGUAGE_TYPE_POR_BOOL:
		*value = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_POR];
		break;
	case MSG_CB_LANGUAGE_TYPE_SWE_BOOL:
		*value = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_SWE];
		break;
	case MSG_CB_LANGUAGE_TYPE_TUR_BOOL:
		*value = cb_opt_data->bLanguage[MSG_CBLANG_TYPE_TUR];
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_CB_RECEIVE_BOOL:
		cb_opt_data->bReceive = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_ALL_BOOL:
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_ALL] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_ENG_BOOL:
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_ENG] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_GER_BOOL:
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_GER] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_FRE_BOOL:
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_FRE] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_ITA_BOOL:
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_ITA] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_NED_BOOL:
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_NED] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_SPA_BOOL:
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_SPA] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_POR_BOOL:
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_POR] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_SWE_BOOL:
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_SWE] = value;
		break;
	case MSG_CB_LANGUAGE_TYPE_TUR_BOOL:
		cb_opt_data->bLanguage[MSG_CBLANG_TYPE_TUR] = value;
		break;
	default:
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

	switch (field) {
	case MSG_CB_CHANNEL_LIST_STRUCT:
		*value = (void *)cb_opt_data->channelData;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_cb_channel_info_int(void *cb_ch_info, int field, int *value)
{
	if (!cb_ch_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CB_CHANNEL_INFO_S *cb_ch_data = (MSG_CB_CHANNEL_INFO_S *)cb_ch_info;

	switch (field) {
	case MSG_CB_CHANNEL_ID_FROM_INT:
		*value = cb_ch_data->from;
		break;
	case MSG_CB_CHANNEL_ID_TO_INT:
		*value = cb_ch_data->to;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_CB_CHANNEL_ID_FROM_INT:
		cb_ch_data->from = value;
		break;
	case MSG_CB_CHANNEL_ID_TO_INT:
		cb_ch_data->to = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_cb_channel_info_bool(void *cb_ch_info, int field, bool *value)
{
	if (!cb_ch_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CB_CHANNEL_INFO_S *cb_ch_data = (MSG_CB_CHANNEL_INFO_S *)cb_ch_info;

	switch (field) {
	case MSG_CB_CHANNEL_ACTIVATE_BOOL:
		*value = cb_ch_data->bActivate;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_CB_CHANNEL_ACTIVATE_BOOL:
		cb_ch_data->bActivate = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_cb_channel_info_str(void *cb_ch_info, int field, char *value, int size)
{
	if (!cb_ch_info)
		return MSG_ERR_NULL_POINTER;

	MSG_CB_CHANNEL_INFO_S *cb_ch_data = (MSG_CB_CHANNEL_INFO_S *)cb_ch_info;

	switch (field) {
	case MSG_CB_CHANNEL_NAME_STR:
		strncpy(value, cb_ch_data->name, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}

int msg_set_cb_channel_info_str(void *cb_ch_info, int field, char *val, int size)
{
	if (!cb_ch_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CB_CHANNEL_INFO_S *cb_ch_data = (MSG_CB_CHANNEL_INFO_S *)cb_ch_info;

	switch (field) {
	case MSG_CB_CHANNEL_NAME_STR:
		bzero(cb_ch_data->name, sizeof(cb_ch_data->name));
		snprintf(cb_ch_data->name, sizeof(cb_ch_data->name), "%s", val);
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

EXPORT_API int msg_get_sms_send_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);

	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_READ_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getSmsSendOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_sms_send_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);

	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->setSmsSendOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_sms_send_opt_int(void *sms_send_opt, int field, int *value)
{
	if (!sms_send_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_SMS_SENDOPT_S *send_opt = (MSG_SMS_SENDOPT_S *)sms_send_opt;

	switch (field) {
	case MSG_SMS_SENDOPT_ENCODE_TYPE_INT:
		*value = send_opt->dcs;
		break;
	case MSG_SMS_SENDOPT_NETWORK_MODE_INT:
		*value = send_opt->netMode;
		break;
	case MSG_SMS_SENDOPT_SAVE_STORAGE_INT:
		*value = send_opt->saveStorage;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_SMS_SENDOPT_ENCODE_TYPE_INT:
		send_opt->dcs = value;
		break;
	case MSG_SMS_SENDOPT_NETWORK_MODE_INT:
		send_opt->netMode = value;
		break;
	case MSG_SMS_SENDOPT_SAVE_STORAGE_INT:
		send_opt->saveStorage = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_sms_send_opt_bool(void *sms_send_opt, int field, bool *value)
{
	if (!sms_send_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_SMS_SENDOPT_S *send_opt = (MSG_SMS_SENDOPT_S *)sms_send_opt;

	switch (field) {
	case MSG_SMS_SENDOPT_REPLY_PATH_BOOL:
		*value = send_opt->bReplyPath;
		break;
	case MSG_SMS_SENDOPT_DELIVERY_REPORT_BOOL:
		*value = send_opt->bDeliveryReport;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_SMS_SENDOPT_REPLY_PATH_BOOL:
		send_opt->bReplyPath = value;
		break;
	case MSG_SMS_SENDOPT_DELIVERY_REPORT_BOOL:
		send_opt->bDeliveryReport = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

EXPORT_API int msg_get_mms_send_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_MMS_FEATURE);

	msg_error_t err = MSG_SUCCESS;

	/*Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_READ_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getMmsSendOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_mms_send_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_MMS_FEATURE);

	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->setMmsSendOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}


int msg_get_mms_send_opt_int(void *mms_send_opt, int field, int *value)
{
	if (!mms_send_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MMS_SENDOPT_S *send_opt = (MSG_MMS_SENDOPT_S *)mms_send_opt;

	switch (field) {
	case MSG_MMS_SENDOPT_CLASS_TYPE_INT:
		*value = send_opt->msgClass;
		break;
	case MSG_MMS_SENDOPT_PRIORITY_TYPE_INT:
		*value = send_opt->priority;
		break;
	case MSG_MMS_SENDOPT_EXPIRY_TIME_INT:
		*value = send_opt->expiryTime;
		break;
	case MSG_MMS_SENDOPT_DELIVERY_TIME_INT:
		*value = send_opt->deliveryTime;
		break;
	case MSG_MMS_SENDOPT_CUSTOM_DELIVERY_TIME_INT:
		*value = send_opt->customDeliveryTime;
		break;
	case MSG_MMS_SENDOPT_REPLY_CHARGING_INT:
		*value = send_opt->replyCharging;
		break;
	case MSG_MMS_SENDOPT_REPLY_CHARGING_DEADLINE_INT:
		*value = send_opt->replyChargingDeadline;
		break;
	case MSG_MMS_SENDOPT_REPLY_CHARGING_SIZE_INT:
		*value = send_opt->replyChargingSize;
		break;
	case MSG_MMS_SENDOPT_CREATION_MODE_INT:
		*value = send_opt->creationMode;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_MMS_SENDOPT_CLASS_TYPE_INT:
		send_opt->msgClass = value;
		break;
	case MSG_MMS_SENDOPT_PRIORITY_TYPE_INT:
		send_opt->priority = value;
		break;
	case MSG_MMS_SENDOPT_EXPIRY_TIME_INT:
		send_opt->expiryTime = value;
		break;
	case MSG_MMS_SENDOPT_DELIVERY_TIME_INT:
		send_opt->deliveryTime = value;
		break;
	case MSG_MMS_SENDOPT_CUSTOM_DELIVERY_TIME_INT:
		send_opt->customDeliveryTime = value;
		break;
	case MSG_MMS_SENDOPT_REPLY_CHARGING_INT:
		send_opt->replyCharging = value;
		break;
	case MSG_MMS_SENDOPT_REPLY_CHARGING_DEADLINE_INT:
		send_opt->replyChargingDeadline = value;
		break;
	case MSG_MMS_SENDOPT_REPLY_CHARGING_SIZE_INT:
		send_opt->replyChargingSize = value;
		break;
	case MSG_MMS_SENDOPT_CREATION_MODE_INT:
		send_opt->creationMode = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_mms_send_opt_bool(void *mms_send_opt, int field, bool *value)
{
	if (!mms_send_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MMS_SENDOPT_S *send_opt = (MSG_MMS_SENDOPT_S *)mms_send_opt;

	switch (field) {
	case MSG_MMS_SENDOPT_SENDER_VISIBILITY_BOOL:
		*value = send_opt->bSenderVisibility;
		break;
	case MSG_MMS_SENDOPT_DELIVERY_REPORT_BOOL:
		*value = send_opt->bDeliveryReport;
		break;
	case MSG_MMS_SENDOPT_READ_REPLY_BOOL:
		*value = send_opt->bReadReply;
		break;
	case MSG_MMS_SENDOPT_KEEP_COPY_BOOL:
		*value = send_opt->bKeepCopy;
		break;
	case MSG_MMS_SENDOPT_BODY_REPLYING_BOOL:
		*value = send_opt->bBodyReplying;
		break;
	case MSG_MMS_SENDOPT_HIDE_RECIPIENTS_BOOL:
		*value = send_opt->bHideRecipients;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_MMS_SENDOPT_SENDER_VISIBILITY_BOOL:
		send_opt->bSenderVisibility = value;
		break;
	case MSG_MMS_SENDOPT_DELIVERY_REPORT_BOOL:
		send_opt->bDeliveryReport = value;
		break;
	case MSG_MMS_SENDOPT_READ_REPLY_BOOL:
		send_opt->bReadReply = value;
		break;
	case MSG_MMS_SENDOPT_KEEP_COPY_BOOL:
		send_opt->bKeepCopy = value;
		break;
	case MSG_MMS_SENDOPT_BODY_REPLYING_BOOL:
		send_opt->bBodyReplying = value;
		break;
	case MSG_MMS_SENDOPT_HIDE_RECIPIENTS_BOOL:
		send_opt->bHideRecipients = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

EXPORT_API int msg_get_mms_recv_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_MMS_FEATURE);

	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_READ_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getMmsRecvOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_mms_recv_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_MMS_FEATURE);

	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->setMmsRecvOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_mms_recv_opt_int(void *mms_recv_opt, int field, int *value)
{
	if (!mms_recv_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MMS_RECVOPT_S *recv_opt = (MSG_MMS_RECVOPT_S *)mms_recv_opt;

	switch (field) {
	case MSG_MMS_RECVOPT_HOME_RETRIEVE_TYPE_INT:
		*value = recv_opt->homeNetwork;
		break;
	case MSG_MMS_RECVOPT_ABROAD_RETRIEVE_TYPE_INT:
		*value = recv_opt->abroadNetwok;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_MMS_RECVOPT_HOME_RETRIEVE_TYPE_INT:
		recv_opt->homeNetwork = value;
		break;
	case MSG_MMS_RECVOPT_ABROAD_RETRIEVE_TYPE_INT:
		recv_opt->abroadNetwok = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_mms_recv_opt_bool(void *mms_recv_opt, int field, bool *value)
{
	if (!mms_recv_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MMS_RECVOPT_S *recv_opt = (MSG_MMS_RECVOPT_S *)mms_recv_opt;

	switch (field) {
	case MSG_MMS_RECVOPT_READ_REPORT_BOOL:
		*value = recv_opt->readReceipt;
		break;
	case MSG_MMS_RECVOPT_DELIVERY_REPORT_BOOL:
		*value = recv_opt->bDeliveryReceipt;
		break;
	case MSG_MMS_RECVOPT_REJECT_UNKNOWN_BOOL:
		*value = recv_opt->bRejectUnknown;
		break;
	case MSG_MMS_RECVOPT_REJECT_ADVERTISEMENT_BOOL:
		*value = recv_opt->bRejectAdvertisement;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_MMS_RECVOPT_READ_REPORT_BOOL:
		recv_opt->readReceipt = value;
		break;
	case MSG_MMS_RECVOPT_DELIVERY_REPORT_BOOL:
		recv_opt->bDeliveryReceipt = value;
		break;
	case MSG_MMS_RECVOPT_REJECT_UNKNOWN_BOOL:
		recv_opt->bRejectUnknown = value;
		break;
	case MSG_MMS_RECVOPT_REJECT_ADVERTISEMENT_BOOL:
		recv_opt->bRejectAdvertisement = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

EXPORT_API int msg_get_push_msg_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_READ_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getPushMsgOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_push_msg_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->setPushMsgOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_push_msg_opt_int(void *push_msg_opt, int field, int *value)
{
	if (!push_msg_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_PUSHMSG_OPT_S *push_opt = (MSG_PUSHMSG_OPT_S *)push_msg_opt;

	switch (field) {
	case MSG_PUSHMSG_SERVICE_TYPE_INT:
		*value = push_opt->serviceType;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_PUSHMSG_SERVICE_TYPE_INT:
		push_opt->serviceType = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_push_msg_opt_bool(void *push_msg_opt, int field, bool *value)
{
	if (!push_msg_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_PUSHMSG_OPT_S *push_opt = (MSG_PUSHMSG_OPT_S *)push_msg_opt;

	switch (field) {
	case MSG_PUSHMSG_RECEIVE_BOOL:
		*value = push_opt->bReceive;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_PUSHMSG_RECEIVE_BOOL:
		push_opt->bReceive = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

EXPORT_API int msg_get_voice_msg_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_READ_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getVoiceMsgOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_voice_msg_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->setVoiceMsgOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_voice_msg_opt_int(void *voice_msg_opt, int field, int *value)
{
	if (!voice_msg_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_VOICEMAIL_OPT_S *voice_opt = (MSG_VOICEMAIL_OPT_S *)voice_msg_opt;

	switch (field) {
	case MSG_VOICEMSG_SIM_INDEX_INT:
		*value = voice_opt->simIndex;
		break;
	case MSG_VOICEMSG_VOICE_COUNT_INT:
		*value = voice_opt->voiceCnt;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_set_voice_msg_opt_int(void *voice_msg_opt, int field, int value)
{
	if (!voice_msg_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_VOICEMAIL_OPT_S *voice_opt = (MSG_VOICEMAIL_OPT_S *)voice_msg_opt;

	switch (field) {
	case MSG_VOICEMSG_SIM_INDEX_INT:
		voice_opt->simIndex = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_voice_msg_opt_str(void *voice_msg_opt, int field, char *value, int size)
{
	if (!voice_msg_opt)
		return MSG_ERR_NULL_POINTER;

	MSG_VOICEMAIL_OPT_S *voice_opt = (MSG_VOICEMAIL_OPT_S *)voice_msg_opt;

	switch (field) {
	case MSG_VOICEMSG_ADDRESS_STR:
		strncpy(value, voice_opt->mailNumber, size);
		break;
	case MSG_VOICEMSG_ALPHA_ID_STR:
		strncpy(value, voice_opt->alpahId, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}

int msg_set_voice_msg_opt_str(void *voice_msg_opt, int field, char *val, int size)
{
	if (!voice_msg_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_VOICEMAIL_OPT_S *voice_opt = (MSG_VOICEMAIL_OPT_S *)voice_msg_opt;

	switch (field) {
	case MSG_VOICEMSG_ADDRESS_STR:
		bzero(voice_opt->mailNumber, sizeof(voice_opt->mailNumber));
		snprintf(voice_opt->mailNumber, sizeof(voice_opt->mailNumber), "%s", val);
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

EXPORT_API int msg_get_general_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_READ_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getGeneralOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_general_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->setGeneralOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_general_opt_int(void *general_opt, int field, int *value)
{
	if (!general_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_GENERAL_OPT_S *opt = (MSG_GENERAL_OPT_S *)general_opt;

	switch (field) {
	case MSG_GENERAL_ALERT_TONE_INT:
		*value = opt->alertTone;
		break;
	case MSG_GENERAL_SMS_LIMIT_CNT_INT:
		*value = opt->smsLimitCnt;
		break;
	case MSG_GENERAL_MMS_LIMIT_CNT_INT:
		*value = opt->mmsLimitCnt;
		break;
	case MSG_GENERAL_RINGTONE_TYPE_INT:
		*value = opt->ringtoneType;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_GENERAL_ALERT_TONE_INT:
		opt->alertTone = value;
		break;
	case MSG_GENERAL_SMS_LIMIT_CNT_INT:
		opt->smsLimitCnt = value;
		break;
	case MSG_GENERAL_MMS_LIMIT_CNT_INT:
		opt->mmsLimitCnt = value;
		break;
	case MSG_GENERAL_RINGTONE_TYPE_INT:
		opt->ringtoneType = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_general_opt_bool(void *general_opt, int field, bool *value)
{
	if (!general_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_GENERAL_OPT_S *opt = (MSG_GENERAL_OPT_S *)general_opt;

	switch (field) {
	case MSG_GENERAL_KEEP_COPY_BOOL:
		*value = opt->bKeepCopy;
		break;
	case MSG_GENERAL_AUTO_ERASE_BOOL:
		*value = opt->bAutoErase;
		break;
	case MSG_GENERAL_BLOCK_UNKNOWN_NUMBER_BOOL:
		*value = opt->bBlockUnknownMsg;
		break;
	case MSG_GENERAL_MSG_NOTIFICATION_BOOL:
		*value = opt->bNotification;
		break;
	case MSG_GENERAL_MSG_VIBRATION_BOOL:
		*value = opt->bVibration;
		break;
	case MSG_GENERAL_MSG_PREVIEW_BOOL:
		*value = opt->bPreview;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_GENERAL_KEEP_COPY_BOOL:
		opt->bKeepCopy = value;
		break;
	case MSG_GENERAL_AUTO_ERASE_BOOL:
		opt->bAutoErase = value;
		break;
	case MSG_GENERAL_BLOCK_UNKNOWN_NUMBER_BOOL:
		opt->bBlockUnknownMsg = value;
		break;
	case MSG_GENERAL_MSG_NOTIFICATION_BOOL:
		opt->bNotification = value;
		break;
	case MSG_GENERAL_MSG_VIBRATION_BOOL:
		opt->bVibration = value;
		break;
	case MSG_GENERAL_MSG_PREVIEW_BOOL:
		opt->bPreview = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_get_general_opt_str(void *general_opt, int field, char *value, int size)
{
	if (!general_opt)
		return MSG_ERR_NULL_POINTER;

	MSG_GENERAL_OPT_S *opt = (MSG_GENERAL_OPT_S *)general_opt;

	switch (field) {
	case MSG_GENERAL_RINGTONE_PATH_STR:
		strncpy(value, opt->ringtonePath, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}

int msg_set_general_opt_str(void *general_opt, int field, char *val, int size)
{
	if (!general_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_GENERAL_OPT_S *opt = (MSG_GENERAL_OPT_S *)general_opt;

	switch (field) {
	case MSG_GENERAL_RINGTONE_PATH_STR:
		bzero(opt->ringtonePath, sizeof(opt->ringtonePath));
		snprintf(opt->ringtonePath, sizeof(opt->ringtonePath), "%s", val);
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}


EXPORT_API int msg_get_msgsize_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_MMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_READ_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getMsgSizeOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

EXPORT_API int msg_set_msgsize_opt(msg_handle_t handle, msg_struct_t msg_struct)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_MMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	/* Privilege check */
	int ret = PRIV_MGR_ERROR_SUCCESS;
	ret = privacy_checker_check_by_privilege(MSG_SERVICE_WRITE_PRIV_NAME);
	if (ret != PRIV_MGR_ERROR_SUCCESS)
		return MSG_ERR_PERMISSION_DENIED;

	if (handle == NULL || msg_struct == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->setMsgSizeOpt(msg_struct);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_SET_READ_ERROR;
	}

	return err;
}

int msg_get_msgsize_opt_int(void *size_opt, int field, int *value)
{
	if (!size_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MSGSIZE_OPT_S *msg_opt = (MSG_MSGSIZE_OPT_S *)size_opt;

	switch (field) {
	case MSG_MESSAGE_SIZE_INT:
		*value = msg_opt->nMsgSize;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
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

	switch (field) {
	case MSG_MESSAGE_SIZE_INT:
		msg_opt->nMsgSize = value;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}
