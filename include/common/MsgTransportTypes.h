/*
*
* Copyright (c) 2000-2012 Samsung Electronics Co., Ltd. All Rights Reserved.
*
* This file is part of msg-service.
*
* Contact: Jaeyun Jeong <jyjeong@samsung.com>
*          Sangkoo Kim <sangkoo.kim@samsung.com>
*          Seunghwan Lee <sh.cat.lee@samsung.com>
*          SoonMin Jung <sm0415.jung@samsung.com>
*          Jae-Young Lee <jy4710.lee@samsung.com>
*          KeeBum Kim <keebum.kim@samsung.com>
*
* PROPRIETARY/CONFIDENTIAL
*
* This software is the confidential and proprietary information of
* SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered
* into with SAMSUNG ELECTRONICS.
*
* SAMSUNG make no representations or warranties about the suitability
* of the software, either express or implied, including but not limited
* to the implied warranties of merchantability, fitness for a particular
* purpose, or non-infringement. SAMSUNG shall not be liable for any
* damages suffered by licensee as a result of using, modifying or
* distributing this software or its derivatives.
*
*/

#ifndef MSG_TRANSPORT_TYPES_H
#define MSG_TRANSPORT_TYPES_H

/**
 *	@file 		MsgTransportTypes.h
 *	@brief 		Defines transport types of messaging framework
 *	@version 	1.0
 */

/**
 *	@section		Introduction
 *	- Introduction : Overview on Messaging Transport Types
 *	@section		Program
 *	- Program : Messaging Transport Types Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgTypes.h"
#include "MsgStorageTypes.h"
#include "MsgMmsTypes.h"

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_TRANSPORT_TYPES	Messaging Transport Types
 *	@{
 */


/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/

/**
 *	@brief	Represents the status of a sent message. \n
 *	This stucture contains the information about the status of a sent message that application has submitted to the framework.
 */
typedef struct
{
	MSG_REQUEST_ID_T		reqId;		/**< Indicates the corresponding request Id. */
	MSG_NETWORK_STATUS_T	status;		/**< Indicates the status of the corresponding request.
									Refer to enum _MSG_NETWORK_STATUS_E*/
} MSG_SENT_STATUS_S;


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
typedef void (*msg_sent_status_cb)(MSG_HANDLE_T handle, MSG_SENT_STATUS_S *sent_status, void *user_param);


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
typedef void (*msg_simple_sent_status_cb)(MSG_SENT_STATUS_S *sent_status, void *user_param);


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
typedef void (*msg_sms_incoming_cb)(MSG_HANDLE_T handle, msg_message_t msg, void *user_param);


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
typedef void (*msg_mms_conf_msg_incoming_cb)(MSG_HANDLE_T handle, msg_message_t msg, void *user_param);


/** @brief	Prototype of the function that will be called when a new SyncML message is received.
 *	Applications SHOULD implement this callback function and register it into Message handle.
 *	For how to register this callback function, please refer to msg_reg_syncml_message_callback.
 *	This callback function will be called when a new message is received and the message satisfies the filter list.
 *	The callback function runs in the application process, not in the framework process.
 *	@param[in]	handle is Message handle.
 *	@param[in]	msg_type is MSG_SYNCML_MESSAGE_TYPE_T structure.
 *	@param[in]	push_body is WAP Push body data.
 *	@param[in]	push_body_len is the length of WAP Push body data.
 *	@param[in]	user_param is a pointer to user data.
 *	@return	void
 */
typedef void (*msg_syncml_msg_incoming_cb)(MSG_HANDLE_T handle, MSG_SYNCML_MESSAGE_TYPE_T msg_type, const char *push_body, int push_body_len, const char* wsp_header, int wsp_header_len, void *user_param);


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
typedef void (*msg_lbs_msg_incoming_cb)(MSG_HANDLE_T handle, const char *push_header, const char *push_body, int push_body_len, void *user_param);


/** @brief	Prototype of the function that will be called when a new SyncML message is received.
 *	Applications SHOULD implement this callback function and register it into Message handle.
 *	For how to register this callback function, please refer to msg_reg_syncml_message_callback.
 *	This callback function will be called when a new message is received and the message satisfies the filter list.
 *	The callback function runs in the application process, not in the framework process.
 *	@param[in]	handle is Message handle.
 *	@param[in]	user_param is a pointer to user data.
 *	@return	void
 */
typedef void (*msg_syncml_msg_operation_cb)(MSG_HANDLE_T handle, int msgId, int extId, void *user_param);


/*==================================================================================================
                                         TYPES
==================================================================================================*/



/**
 *	@}
 */

#endif // MSG_TRANSPORT_TYPES_H

