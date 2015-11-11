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

#ifndef MSG_PRIVATE_H_
#define MSG_PRIVATE_H_

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "MsgDebug.h"
#include "MsgGconfWrapper.h"
#include "MsgTypes.h"
#include "MsgStorageTypes.h"
#include "MsgTransportTypes.h"
#include "MsgMmsTypes.h"
#include "MsgFilterTypes.h"
#include "MsgSettingTypes.h"
#include "MsgUtilFunction.h"

/*==================================================================================================
                                    DEFINES
==================================================================================================*/

#define CHECK_MSG_SUPPORTED(feature_name) \
	do { \
		bool bSupported = false; \
		bSupported = MsgCheckFeatureSupport((feature_name)); \
		if (bSupported == false) {\
			MSG_ERR("Feature [%s] not supported", (feature_name));\
			return MSG_ERR_NOT_SUPPORTED; \
		} \
	} while(0)

#define CHECK_MSG_SUPPORTED_RETURN_NULL(feature_name) \
	do { \
		bool bSupported = false; \
		bSupported = MsgCheckFeatureSupport((feature_name)); \
		if (bSupported == false) {\
			MSG_ERR("Feature [%s] not supported", (feature_name));\
			return NULL; \
		} \
	} while(0)

/*==================================================================================================
									 FUNCTION PROTOTYPES
==================================================================================================*/

/* message */
void msg_message_create_struct(msg_struct_s *msg_struct);
int msg_message_release(msg_struct_s **msg_struct);

int msg_message_get_int_value(void *data, int field, int *value);
int msg_message_get_bool_value(void *data, int field, bool *value);
int msg_message_get_str_value(void *data, int field, char *value, int size);
int msg_message_get_struct_hnd(void *data, int field, void **value);
int msg_message_get_list_hnd(void *data, int field, void **value);

int msg_message_set_int_value(void *data, int field, int value);
int msg_message_set_bool_value(void *data, int field, bool value);
int msg_message_set_str_value(void *data, int field, char *value, int size);
int msg_message_set_struct_hnd(void *data, int field, void *value);

void msg_message_copy_message(MSG_MESSAGE_HIDDEN_S *pSrc, MSG_MESSAGE_HIDDEN_S *pDst);

int msg_cb_message_get_int_value(void *data, int field, int *value);
int msg_cb_message_get_str_value(void *data, int field, char *value, int size);

int msg_message_list_append(msg_struct_t msg_struct_handle, int field, msg_struct_t *item);
int msg_message_list_clear(msg_struct_t msg_struct_handle, int field);



/* filter */
int msg_get_filter_info_bool(void *filter, int field, bool *value);
int msg_get_filter_info_int(void *filter, int field, int *value);
int msg_get_filter_info_str(void *filter, int field, char *value, int size);
int msg_set_filter_info_bool(void *filter, int field, bool value);
int msg_set_filter_info_int(void *filter, int field, int value);
int msg_set_filter_info_str(void *filter, int field, char *value, int size);


/* mms */
msg_struct_s *msg_mms_create_struct(int field);
void *msg_mms_create_struct_data(int field);
int msg_mms_release_struct(msg_struct_s **msg_struct_data);

int msg_mms_get_int_value(msg_struct_s *msg_struct, int field, int *value);
int msg_mms_get_str_value(msg_struct_s *msg_struct, int field, char *src, int size);
int msg_mms_get_bool_value(msg_struct_s *msg_struct, int field, bool *value);
int msg_mms_get_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s **value);
int msg_mms_get_list_handle(msg_struct_s *msg_struct, int field, msg_list_handle_t *value);

int msg_mms_set_int_value(msg_struct_s *msg_struct, int field, int value);
int msg_mms_set_str_value(msg_struct_s *msg_struct, int field, char *value, int size);
int msg_mms_set_bool_value(msg_struct_s *msg_struct, int field, bool value);
int msg_mms_set_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s *value);

int msg_mms_list_append(msg_struct_t msg_struct_handle, int field, msg_struct_t *item);

void convert_to_hidden_mmsdata(MMS_DATA_S *pSrc, msg_struct_s *pDest);
void convert_from_hidden_mmsdata(msg_struct_s *pSrc, MMS_DATA_S *pDest);


/* setting */
int msg_setting_get_int_value(msg_struct_s *msg_struct, int field, int *value);
int msg_setting_get_str_value(msg_struct_s *msg_struct, int field, char *src, int size);
int msg_setting_get_bool_value(msg_struct_s *msg_struct, int field, bool *value);
int msg_setting_get_list_handle(msg_struct_s *msg_struct, int field, void **value);

int msg_setting_set_int_value(msg_struct_s *msg_struct, int field, int value);
int msg_setting_set_str_value(msg_struct_s *msg_struct, int field, char *value, int size);
int msg_setting_set_bool_value(msg_struct_s *msg_struct, int field, bool value);

int msg_get_smsc_opt_int(void *smsc_opt, int field, int *value);
int msg_set_smsc_opt_int(void *smsc_opt, int field, int value);
int msg_get_smsc_opt_list(void *smsc_opt, int field, void **value);

int msg_get_smsc_info_int(void *smsc_info, int field, int *value);
int msg_set_smsc_info_int(void *smsc_info, int field, int value);
int msg_get_smsc_info_str(void *smsc_info, int field, char *value, int size);
int msg_set_smsc_info_str(void *smsc_info, int field, char *val, int size);

int msg_get_cb_option_int(void *cb_opt, int field, int *value);
int msg_set_cb_option_int(void *cb_opt, int field, int value);
int msg_get_cb_option_bool(void *cb_opt, int field, bool *value);
int msg_set_cb_option_bool(void *cb_opt, int field, bool value);
int msg_get_cb_option_list(void *cb_opt, int field, void **value);

int msg_get_cb_channel_info_int(void *cb_ch_info, int field, int *value);
int msg_set_cb_channel_info_int(void *cb_ch_info, int field, int value);
int msg_get_cb_channel_info_bool(void *cb_ch_info, int field, bool *value);
int msg_set_cb_channel_info_bool(void *cb_ch_info, int field, bool value);
int msg_get_cb_channel_info_str(void *cb_ch_info, int field, char *value, int size);
int msg_set_cb_channel_info_str(void *cb_ch_info, int field, char *val, int size);

int msg_get_sms_send_opt_int(void *sms_send_opt, int field, int *value);
int msg_set_sms_send_opt_int(void *sms_send_opt, int field, int value);
int msg_get_sms_send_opt_bool(void *sms_send_opt, int field, bool *value);
int msg_set_sms_send_opt_bool(void *sms_send_opt, int field, bool value);

int msg_get_mms_send_opt_int(void *mms_send_opt, int field, int *value);
int msg_set_mms_send_opt_int(void *mms_send_opt, int field, int value);
int msg_get_mms_send_opt_bool(void *mms_send_opt, int field, bool *value);
int msg_set_mms_send_opt_bool(void *mms_send_opt, int field, bool value);

int msg_get_mms_recv_opt_int(void *mms_recv_opt, int field, int *value);
int msg_set_mms_recv_opt_int(void *mms_recv_opt, int field, int value);
int msg_get_mms_recv_opt_bool(void *mms_recv_opt, int field, bool *value);
int msg_set_mms_recv_opt_bool(void *mms_recv_opt, int field, bool value);

int msg_get_push_msg_opt_int(void *push_msg_opt, int field, int *value);
int msg_set_push_msg_opt_int(void *push_msg_opt, int field, int value);
int msg_get_push_msg_opt_bool(void *push_msg_opt, int field, bool *value);
int msg_set_push_msg_opt_bool(void *push_msg_opt, int field, bool value);

int msg_get_voice_msg_opt_int(void *voice_msg_opt, int field, int *value);
int msg_set_voice_msg_opt_int(void *voice_msg_opt, int field, int value);
int msg_get_voice_msg_opt_str(void *voice_msg_opt, int field, char *value, int size);
int msg_set_voice_msg_opt_str(void *voice_msg_opt, int field, char *val, int size);

int msg_get_general_opt_int(void *general_opt, int field, int *value);
int msg_set_general_opt_int(void *general_opt, int field, int value);
int msg_get_general_opt_bool(void *general_opt, int field, bool *value);
int msg_set_general_opt_bool(void *general_opt, int field, bool value);
int msg_get_general_opt_str(void *general_opt, int field, char *value, int size);
int msg_set_general_opt_str(void *general_opt, int field, char *val, int size);

int msg_get_msgsize_opt_int(void *size_opt, int field, int *value);
int msg_set_msgsize_opt_int(void *size_opt, int field, int value);

/* Wap Push */
int msg_push_config_get_str(void *event_info, int field, char *value, int size);
int msg_push_config_get_bool(void *event_info, int field, bool *value);
int msg_push_config_set_str(void *event_info, int field, char *value, int size);
int msg_push_config_set_bool(void *event, int field, bool value);

/* added internal apis for new managed api (storage) */
int msg_syncml_info_get_int(void *syncml_info, int field, int *value);
int msg_count_info_get_int(void *count_info, int field, int *value);
int msg_thread_count_get_int(void *count_info, int field, int *value);
int msg_thread_index_get_int(void *index_info, int field, int *value);
int msg_sortrule_get_int(void *sort_info, int field, int *value);
int msg_folder_info_get_int(void *folder_info, int field, int *value);
int msg_thread_info_get_int(void *data, int field, int *value);
int msg_conv_info_get_int(void *data, int field, int *value);
int msg_list_condition_get_int(void *condition_info, int field, int *value);
int msg_report_status_get_int(void *report_info, int field, int *value);
int msg_report_status_get_str(void *report_info, int field, char *value, int size);
int msg_folder_info_get_str(void *folder_info, int field, char *value, int size);
int msg_thread_info_get_str(void *data, int field, char *value, int size);
int msg_conv_info_get_str(void *data, int field, char *value, int size);
int msg_list_condition_get_str(void *condition_info, int field, char *value, int size);
int msg_sendopt_get_bool(void *send_opt, int field, bool *value);
int msg_sortrule_get_bool(void *sort_rule, int field, bool *value);
int msg_conv_get_bool(void *data, int field, bool *value);
int msg_thread_info_get_bool(void *data, int field, bool *value);
int msg_list_condition_get_bool(void *data, int field, bool *value);
int msg_sendopt_get_struct_handle(msg_struct_s *msg_struct, int field, void **value);
int msg_syncml_get_struct_handle(msg_struct_s *msg_struct, int field, void **value);
int msg_thread_index_get_struct_handle(msg_struct_s *msg_struct, int field, void **value);
int msg_list_condition_get_struct_handle(msg_struct_s *msg_struct, int field, void **value);
int msg_address_info_get_int(void *addr_info, int field, int *value);
int msg_mms_sendopt_get_int(void *opt_info, int field, int *value);
int msg_reject_message_get_int(void *msg_info, int field, int *value);
int msg_address_info_get_str(void *addr_info, int field, char *value, int size);
int msg_reject_message_get_str(void *msg_info, int field, char *value, int size);
int msg_mms_sendopt_get_bool(void *opt_info, int field, bool *value);
int msg_sms_sendopt_get_bool(void *opt_info, int field, bool *value);

int msg_syncml_info_set_int(void *syncml_info, int field, int value);
int msg_count_info_set_int(void *count_info, int field, int value);
int msg_thread_count_set_int(void *count_info, int field, int value);
int msg_thread_index_set_int(void *index_info, int field, int value);
int msg_sortrule_set_int(void *sort_info, int field, int value);
int msg_folder_info_set_int(void *folder_info, int field, int value);
int msg_list_condition_set_int(void *condition_info, int field, int value);
int msg_report_status_set_int(void *report_info, int field, int value);
int msg_folder_info_set_str(void *folder_info, int field, char *value, int size);
int msg_list_condition_set_str(void *condition_info, int field, char *value, int size);
int msg_sendopt_set_bool(void *send_opt, int field, bool value);
int msg_sortrule_set_bool(void *sort_rule, int field, bool value);
int msg_list_condition_set_bool(void *data, int field, bool value);
int msg_sendopt_set_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s *value);
int msg_syncml_set_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s *value);
int msg_thread_index_set_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s *value);
int msg_list_condition_set_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s *value);
int msg_address_info_set_int(void *addrinfo, int field, int value);
int msg_mms_sendopt_set_int(void *opt_info, int field, int value);
int msg_reject_message_set_int(void *msg_info, int field, int value);
int msg_address_info_set_str(void *addr_info, int field, char *value, int size);
int msg_media_info_set_str(void *media_info, int field, char *value, int size);
int msg_reject_message_set_str(void *msg_info, int field, char *value, int size);
int msg_mms_sendopt_set_bool(void *option, int field, bool value);
int msg_sms_sendopt_set_bool(void *option, int field, bool value);

/* added internal apis for new managed api (transport) */
int msg_request_get_int(void *request_info, int field, int *value);
int msg_request_get_struct_handle(msg_struct_s *msg_struct, int field, void **value);
int msg_request_set_int(void *request_info, int field, int value);
int msg_request_set_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s *value);
int msg_sent_status_get_int(MSG_SENT_STATUS_S *sent_status_info, int field, int *value);

int msg_media_item_get_str(void *data, int field, char *value, int size);
int msg_media_item_get_int(void *data, int field, int *value);

int msg_conversation_get_list_hnd(void *data, int field, void **value);
int msg_multipart_get_str_value(void *data, int field, char *value, int size);
int msg_multipart_get_int_value(void *data, int field, int *value);
int msg_multipart_set_str_value(void *data, int field, char *value, int size);
#endif /* MSG_PRIVATE_H_ */
