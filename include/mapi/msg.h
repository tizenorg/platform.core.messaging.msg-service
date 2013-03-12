/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef MSG_H_
#define MSG_H_

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <stdbool.h>

#include "msg_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_CONTROL_API	Messaging Control API
 *	@{
 */

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

/**

 * \par Description:
 * Opens a channel between an application and messaging framework.
 *
 * \par Purpose:
 * For application to utilize the services of Messaging Framework, this API should be called to establish connection between the application and Messaging Framework.
 *
 * \par Typical use case:
 * Any application which utilizes the services of Messaging Framework needs to call this API.
 *
 * \par Method of function operation:
 * Check for Message Server ready status. If ready connect to the Messaging Server socket and pass the handle application.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - The handle parameter returned must be used by application for further API calls to Messaging Service \n
 * - memory for the handle need not be allocated by the application \n
 * - An error will be returned in case Messaging Service is not running.
 *
 * \param msg_handle_t    input - handle to be passed for all Messaging Services .
 *
 * \return Return Type (int) \n
 * - MSG_SUCCESS	- Successfully connected to Messaging Service \n
 * - MSG_ERR_NULL_POINTER	-	Input parameter is NULL.
 * - MSG_ERR_MEMORY_ERROR - 	Memory error.
 * - MSG_ERR_COMMUNICATION_ERROR	- Communication error between client and server \n
 *
 * \par Prospective clients:
 * External/Native Apps using Messaging Services.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 * \par Sample code:
 * \code
 * ...
 *
 * msg_handle_t msgHandle = NULL;
 * msg_error_t err = MSG_SUCCESS;
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_open_msg_handle() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_open_msg_handle(msg_handle_t *handle);


/**

 * \par Description:
 * Closes the channel between application and messaging framework.
 *
 * \par Purpose:
 * Once application utilizes services of Messaging Service, this API needs to be invoked the close the channel between application and Messaging Service.
 *
 * \par Typical use case:
 * Any application which has completed using services of Messaging Framework needs to call this API.
 *
 * \par Method of function operation:
 * Closes the connection to Messaging Service and deleted the reference to the handle object
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - The handle parameter returned must be used by application for further API calls to Messaging Service \n
 * - memory for the handle need not be allocated by the application \n
 * - An error will be returned in case Messaging Service is not running.
 *
 * \param msg_handle_t    input - handle to be passed for all Messaging Services .
 *
 * \return Return Type (int) \n
 * - MSG_SUCCESS	- Successfully connected to Messaging Service \n
 * - MSG_ERR_NULL_POINTER	-	Input parameter is NULL.
 * - MSG_ERR_COMMUNICATION_ERROR	- Communication error between client and server \n
 *
 * \par Prospective clients:
 * External/Native Apps using Messaging Services.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 * \par Sample code:
 * \code
 * ...
 *
 * msg_handle_t msgHandle = NULL;
 * msg_error_t err = MSG_SUCCESS;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_close_msg_handle(&msgHandle);
 *
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_close_msg_handle() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_close_msg_handle(msg_handle_t *handle);



msg_struct_t msg_create_struct(int field);
int msg_release_struct(msg_struct_t *msg_struct_handle);
int msg_release_list_struct(msg_struct_list_s *msg_struct_list);

int msg_get_int_value(msg_struct_t msg_struct_handle, int field, int *value);
int msg_get_str_value(msg_struct_t msg_struct_handle, int field, char *value, int size);
int msg_get_bool_value(msg_struct_t msg_struct_handle, int field, bool *value);
int msg_get_struct_handle(msg_struct_t msg_struct_handle, int field, msg_struct_t *value);
int msg_get_list_handle(msg_struct_t msg_struct_handle, int field, void **value);

int msg_set_int_value(msg_struct_t msg_struct_handle, int field, int value);
int msg_set_str_value(msg_struct_t msg_struct_handle, int field, char *value, int size);
int msg_set_bool_value(msg_struct_t msg_struct_handle, int field, bool value);
int msg_set_struct_handle(msg_struct_t msg_struct_handle, int field, msg_struct_t value);
int msg_set_list_handle(msg_struct_t msg_struct_handle, int field, void *value);

int msg_mms_add_item(msg_struct_t msg_struct_handle, int field, msg_struct_t *item);

int msg_get_mms_struct(msg_struct_t msg_struct_handle, msg_struct_t mms_struct_handle);
int msg_set_mms_struct(msg_struct_t msg_struct_handle, msg_struct_t mms_struct_handle);

//list
msg_struct_t msg_list_nth_data(msg_list_handle_t list_handle, int index);
int msg_list_length(msg_list_handle_t list_handle);

// filter
int msg_add_filter(msg_handle_t handle, const msg_struct_t filter);
int msg_update_filter(msg_handle_t handle, const msg_struct_t filter);
int msg_delete_filter(msg_handle_t handle, msg_filter_id_t filter_id);
int msg_get_filter_list(msg_handle_t handle, msg_struct_list_s *filter_list);
int msg_set_filter_operation(msg_handle_t handle, bool set_flag);
int msg_get_filter_operation(msg_handle_t handle, bool *set_flag);
int msg_set_filter_active(msg_handle_t handle, msg_filter_id_t filter_id, bool active);

//setting
int msg_get_smsc_opt(msg_handle_t handle, msg_struct_t msg_struct);
int msg_set_smsc_opt(msg_handle_t handle, msg_struct_t msg_struct);

int msg_get_cb_opt(msg_handle_t handle, msg_struct_t msg_struct);
int msg_set_cb_opt(msg_handle_t handle, msg_struct_t msg_struct);

int msg_get_sms_send_opt(msg_handle_t handle, msg_struct_t msg_struct);
int msg_set_sms_send_opt(msg_handle_t handle, msg_struct_t msg_struct);

int msg_get_mms_send_opt(msg_handle_t handle, msg_struct_t msg_struct);
int msg_set_mms_send_opt(msg_handle_t handle, msg_struct_t msg_struct);

int msg_get_mms_recv_opt(msg_handle_t handle, msg_struct_t msg_struct);
int msg_set_mms_recv_opt(msg_handle_t handle, msg_struct_t msg_struct);

int msg_get_push_msg_opt(msg_handle_t handle, msg_struct_t msg_struct);
int msg_set_push_msg_opt(msg_handle_t handle, msg_struct_t msg_struct);

int msg_get_voice_msg_opt(msg_handle_t handle, msg_struct_t msg_struct);
int msg_set_voice_msg_opt(msg_handle_t handle, msg_struct_t msg_struct);

int msg_get_general_opt(msg_handle_t handle, msg_struct_t msg_struct);
int msg_set_general_opt(msg_handle_t handle, msg_struct_t msg_struct);

int msg_get_msgsize_opt(msg_handle_t handle, msg_struct_t msg_struct);
int msg_set_msgsize_opt(msg_handle_t handle, msg_struct_t msg_struct);

// text length calculate
int msg_util_calculate_text_length(const char* msg_text, msg_encode_type_t msg_encode_type_to, unsigned int *text_size, unsigned int *segment_size, msg_encode_type_t *msg_encode_type_in);

#ifdef __cplusplus
}
#endif

#endif /* MSG_H_ */
