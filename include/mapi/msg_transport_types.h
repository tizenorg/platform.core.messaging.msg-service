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

#ifndef MSG_TRANSPORT_TYPES_H_
#define MSG_TRANSPORT_TYPES_H_

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "msg_types.h"
#include "msg_storage_types.h"


/*==================================================================================================
                                    DEFINES
==================================================================================================*/

/**	@brief	Prototype of the function that will be called when the status of the message which is already sent is changed.
 *	Applications SHOULD implement this callback function and register it into Message handle.
 *	For how to register this callback function, please refer to msg_reg_sent_status_callback.
 *	If the application sends a message, this callback function will be called to report its sending status.
 *	The application can get the request ID from sent_status to know which request is bound.
 *	The callback function runs in the application process, not in the framework process.
 *	The memory of sent_status is managed by MAPI, when the callback function is finished.
 *	@param[in]	handle is Message handle.
 *	@param[in]	sent_status is a pointer to an MSG_SENT_STATUS_S structure.
 *	@param[in]	user_param is a pointer to user data.
 *	@return	void
 */
typedef void (*msg_sent_status_cb)(msg_handle_t handle, msg_struct_t sent_status, void *user_param);


/**	@brief	Prototype of the function that will be called when the status of the message which is already sent is changed.
 *	Applications SHOULD implement this callback function and register it into Message handle.
 *	For how to register this callback function, please refer to msg_reg_sent_status_callback.
 *	If the application sends a message, this callback function will be called to report its sending status.
 *	The application can get the request ID from sent_status to know which request is bound.
 *	The callback function runs in the application process, not in the framework process.
 *	The memory of sent_status is managed by MAPI, when the callback function is finished.
 *	@param[in]	sent_status is a pointer to an MSG_SENT_STATUS_S structure.
 *	@param[in]	user_param is a pointer to user data.
 *	@return	void
 */
typedef void (*msg_simple_sent_status_cb)(msg_struct_t sent_status, void *user_param);


/** @brief	Prototype of the function that will be called when a new message is received.
 *	Applications SHOULD implement this callback function and register it into Message handle.
 *	For how to register this callback function, please refer to msg_reg_sms_message_callback.
 *	This callback function will be called when a new message is received and the message satisfies the filter list.
 *	The callback function runs in the application process, not in the framework process.
 *	The memory for msg is managed by MAPI, when the callback function is finished.
 *	@param[in]	handle is Message handle.
 *	@param[in]	msg is a pointer to an msg_message_t structure.
 *	@param[in]	user_param is a pointer to user data.
 *	@return	void
 */
typedef void (*msg_sms_incoming_cb)(msg_handle_t handle, msg_struct_t msg, void *user_param);


/** @brief	Prototype of the function that will be called when a new MMS message is received.
 *	Applications SHOULD implement this callback function and register it into Message handle.
 *	For how to register this callback function, please refer to msg_reg_mms_conf_message_callback.
 *	This callback function will be called when a new MMS message is received and the message satisfies the filter list.
 *	The callback function runs in the application process, not in the framework process.
 *	The memory for msg is managed by MAPI, when the callback function is finished.
 *	@param[in]	handle is Message handle.
 *	@param[in]	msg is a pointer to an msg_message_t structure.
 *	@param[in]	user_param is a pointer to user data.
 *	@return	void
 */
typedef void (*msg_mms_conf_msg_incoming_cb)(msg_handle_t handle, msg_struct_t msg, void *user_param);


/** @brief	Prototype of the function that will be called when a new SyncML message is received.
 *	Applications SHOULD implement this callback function and register it into Message handle.
 *	For how to register this callback function, please refer to msg_reg_syncml_message_callback.
 *	This callback function will be called when a new message is received and the message satisfies the filter list.
 *	The callback function runs in the application process, not in the framework process.
 *	@param[in]	handle is Message handle.
 *	@param[in]	msg_type is msg_syncml_message_type_t structure.
 *	@param[in]	push_body is WAP Push body data.
 *	@param[in]	push_body_len is the length of WAP Push body data.
 *	@param[in]	user_param is a pointer to user data.
 *	@return	void
 */
typedef void (*msg_syncml_msg_incoming_cb)(msg_handle_t handle, msg_syncml_message_type_t msg_type, const char *push_body, int push_body_len, const char* wsp_header, int wsp_header_len, void *user_param);


/** @brief	Prototype of the function that will be called when a new LBS message is received.
 *	Applications SHOULD implement this callback function and register it into Message handle.
 *	For how to register this callback function, please refer to msg_reg_lbs_message_callback.
 *	This callback function will be called when a new message is received and the message satisfies the filter list.
 *	The callback function runs in the application process, not in the framework process.
 *	@param[in]	hMsgHandle is Message handle.
 *	@param[in]	push_header is push message header data.
 *	@param[in]	push_body is push message body data.
 *	@param[in]	push_body_len is push message body length.
 *	@param[in]	user_param is a pointer to user data.
 *	@return	void
 */
typedef void (*msg_lbs_msg_incoming_cb)(msg_handle_t handle, const char *push_header, const char *push_body, int push_body_len, void *user_param);


/** @brief	Prototype of the function that will be called when a new SyncML message is received.
 *	Applications SHOULD implement this callback function and register it into Message handle.
 *	For how to register this callback function, please refer to msg_reg_syncml_message_callback.
 *	This callback function will be called when a new message is received and the message satisfies the filter list.
 *	The callback function runs in the application process, not in the framework process.
 *	@param[in]	handle is Message handle.
 *	@param[in]	user_param is a pointer to user data.
 *	@return	void
 */
typedef void (*msg_syncml_msg_operation_cb)(msg_handle_t handle, int msgId, int extId, void *user_param);



#endif /* MSG_TRANSPORT_TYPES_H_ */
