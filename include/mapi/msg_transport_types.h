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

#ifndef MSG_TRANSPORT_TYPES_H_
#define MSG_TRANSPORT_TYPES_H_

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "msg_types.h"
#include "msg_storage_types.h"

/**
 * @internal
 * @addtogroup MSG_SERVICE_FRAMEWORK_TRANSPORT_MODULE
 * @{
 */


/*==================================================================================================
                                    DEFINES
==================================================================================================*/

/**	@brief	Called when the status of the message which is already sent is changed.
 *	         Applications SHOULD implement this callback function and register it into Message handle.
 *	         For how to register this callback function, please refer to msg_reg_sent_status_callback().
 *	         If the application sends a message, this callback function will be called to report its sending status.
 *	         The application can get the request ID from sent_status to know which request is bound.
 *	         The callback function runs in the application process, not in the framework process.
 *	         The memory of @a sent_status is managed by MAPI, when the callback function is finished.
 *
 * @param[in]  handle       The Message handle
 * @param[in]  sent_status  A pointer to #msg_struct_t structure for sent status
 * @param[in]  user_param   The user data
 */
typedef void (*msg_sent_status_cb)(msg_handle_t handle, msg_struct_t sent_status, void *user_param);


/**	@brief	Called when the status of the message which is already sent is changed.
 *	         Applications SHOULD implement this callback function and register it into Message handle.
 *	         For how to register this callback function, please refer to msg_reg_sent_status_callback().
 *	         If the application sends a message, this callback function will be called to report its sending status.
 *	         The application can get the request ID from sent_status to know which request is bound.
 *	         The callback function runs in the application process, not in the framework process.
 *	         The memory of @a sent_status is managed by MAPI, when the callback function is finished.
 *
 * @param[in]  sent_status  A pointer to #msg_struct_t structure for sent status
 * @param[in]  user_param   The pointer to user data
 */
typedef void (*msg_simple_sent_status_cb)(msg_struct_t sent_status, void *user_param);


/** @brief	Called when a new message is received.
 *	         Applications SHOULD implement this callback function and register it into Message handle.
 *	         For how to register this callback function, please refer to msg_reg_sms_message_callback().
 *	         This callback function will be called when a new message is received and the message satisfies the filter list.
 *	         The callback function runs in the application process, not in the framework process.
 *	         The memory for msg is managed by MAPI, when the callback function is finished.
 *
 * @param[in]  handle       The Message handle
 * @param[in]  msg          A pointer to #msg_struct_t structure for message details
 * @param[in]  user_param   The user data
 */
typedef void (*msg_sms_incoming_cb)(msg_handle_t handle, msg_struct_t msg, void *user_param);


/** @brief	Called when a new MMS message is received.
 *	         Applications SHOULD implement this callback function and register it into Message handle.
 *	         For how to register this callback function, please refer to msg_reg_mms_conf_message_callback().
 *	         This callback function will be called when a new MMS message is received and the message satisfies the filter list.
 *	         The callback function runs in the application process, not in the framework process.
 *	         The memory for @a msg is managed by MAPI, when the callback function is finished.
 *
 * @param[in]  handle      The Message handle
 * @param[in]  msg         A pointer to #msg_struct_t structure for message details
 * @param[in]  user_param  The user data
 */
typedef void (*msg_mms_conf_msg_incoming_cb)(msg_handle_t handle, msg_struct_t msg, void *user_param);


/** @brief	Called when a new SyncML message is received.
 *	         Applications SHOULD implement this callback function and register it into Message handle.
 *	         For how to register this callback function, please refer to msg_reg_syncml_message_callback().
 *	         This callback function will be called when a new message is received and the message satisfies the filter list.
 *	         The callback function runs in the application process, not in the framework process.
 *
 * @param[in]  handle         The Message handle
 * @param[in]  msg_type       The #msg_syncml_message_type_t structure
 * @param[in]  push_body      The WAP Push body data
 * @param[in]  push_body_len  The length of WAP Push body data
 * @param[in]  sim_index      The index of the sim on which message is received.
 * @param[in]  user_param     The user data
 */
typedef void (*msg_syncml_msg_incoming_cb)(msg_handle_t handle, msg_syncml_message_type_t msg_type, const char *push_body, int push_body_len, const char* wsp_header, int wsp_header_len, int sim_index, void *user_param);


/** @brief	Called when a new LBS message is received.
 *	         Applications SHOULD implement this callback function and register it into Message handle.
 *	         For how to register this callback function, please refer to msg_reg_lbs_message_callback().
 *	         This callback function will be called when a new message is received and the message satisfies the filter list.
 *	         The callback function runs in the application process, not in the framework process.
 *
 * @param[in]  hMsgHandle     The Message handle
 * @param[in]  push_header    The push message header data
 * @param[in]  push_body      The push message body data
 * @param[in]  push_body_len  The push message body length
 * @param[in]  user_param     The user data
 */
typedef void (*msg_lbs_msg_incoming_cb)(msg_handle_t handle, const char *push_header, const char *push_body, int push_body_len, void *user_param);


/** @brief	Called when a new SyncML message is received.
 *	         Applications SHOULD implement this callback function and register it into Message handle.
 *	         For how to register this callback function, please refer to msg_reg_syncml_message_callback().
 *	         This callback function will be called when a new message is received and the message satisfies the filter list.
 *	         The callback function runs in the application process, not in the framework process.
 *
 * @param[in]  handle      The Message handle
 * @param[in]  msgId       The message ID
 * @param[in]  extId       The external ID
 * @param[in]  user_param  The user data
 */
typedef void (*msg_syncml_msg_operation_cb)(msg_handle_t handle, int msgId, int extId, void *user_param);


/** @brief	Called when a new push message is received.
 *	         Applications SHOULD implement this callback function and register it into Message handle.
 *	         For how to register this callback function, please refer to msg_reg_push_message_callback().
 *	         This callback function will be called when a new message is received and the message satisfies the filter list.
 *	         The callback function runs in the application process, not in the framework process.
 *
 * @param[in]  handle         The Message handle
 * @param[in]  push_header    The push message header data
 * @param[in]  push_body      The push message body data
 * @param[in]  push_body_len  The push message body length
 * @param[in]  user_param     The user data
 */
typedef void (*msg_push_msg_incoming_cb)(msg_handle_t handle, const char *push_header, const char *push_body, int push_body_len, void *user_param);


/** @brief	Called when a new CB message is received.
 *	         Applications SHOULD implement this callback function and register it into Message handle.
 *	         For how to register this callback function, please refer to msg_reg_cb_message_callback().
 *	         This callback function will be called when a new message is received and the message satisfies the filter list.
 *	         The callback function runs in the application process, not in the framework process.
 *
 * @param[in]  handle      The Message handle
 * @param[in]  msg         A pointer to #msg_struct_t structure for message details
 * @param[in]  user_param  The user data
 */
typedef void (*msg_cb_incoming_cb)(msg_handle_t handle, msg_struct_t msg, void *user_param);


/** @brief	Called when a report message is received.
 *	         Applications SHOULD implement this callback function and register it into Message handle.
 *	         For how to register this callback function, please refer to msg_reg_report_message_callback().
 *	         This callback function will be called when a new message is received and the message satisfies the filter list.
 *	         The callback function runs in the application process, not in the framework process.
 *
 * @param[in]  handle         The Message handle
 * @param[in]  reportMsgType  The message type of incoming report message
 * @param[in]  MsgId          The message ID of reported message
 * @param[in]  addr_len       The incoming report message's address length
 * @param[in]  addr_val       The incoming report message's address value
 * @param[in]  user_param     The user data
 */
typedef void (*msg_report_msg_incoming_cb)(msg_handle_t handle, msg_report_type_t reportMsgType, msg_message_id_t MsgId, int addr_len, const char *addr_val, void *user_param);

/**
 * @}
 */

#endif /* MSG_TRANSPORT_TYPES_H_ */
