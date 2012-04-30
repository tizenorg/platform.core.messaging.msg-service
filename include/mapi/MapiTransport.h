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

/**
 *	@file 		MapiTransport.h
 *	@brief 		Defines transport API of messaging framework
 *	@version 	1.0
 */

#ifndef MAPI_TRANSPORT_H
#define MAPI_TRANSPORT_H

/**
 *	@section		Introduction
 *	- Introduction : Overview on Messaging Transport API
 *	@section		Program
 *	- Program : Messaging Transport API Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "MsgTransportTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_TRANSPORT_API	Messaging Transport API
 *	@{
 */

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

/**

 * \par Description:
 * Submits a request to the Messaging Framework.
 *
 * \par Purpose:
 * This API is used to submit a request to the Messaging Framework.
 *
 * \par Typical use case:
 * Submit a request to Messaging Service such as Send Message, Forward etc.
 *
 * \par Method of function operation:
 * Sets up the database connection and inserts the message to message table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - The result information will be sent back by using the callback function, msg_sent_status_cb.
 * - Applications MUST fill in the valid message type.
 * -  reqId will be filled in the framework.
 *
 * \param input  - handle is Message handle.
 * \param input  - req is a pointer to an MSG_REQUEST_S structure.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_MSGHANDLE	Message handle is invalid.
 * - MSG_ERR_NULL_POINTER			Pointer is NULL.
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
 * int err = MSG_SUCCESS;
 * MSG_REQUEST_S req;
 *
 * req.msg = msg;
 * req.sendOpt = sendOpt;

 * err = msg_submit_req(msgHandle, &req);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_submit_req(MSG_HANDLE_T handle, MSG_REQUEST_S *req);


/**

 * \par Description:
 * Registers sent status callback function to Message handle.
 *
 * \par Purpose:
 * This API is used to register sent status callback function "msg_sent_status_cb" to Message handle.
 *
 * \par Typical use case:
 * Register for sent status callback.
 *
 * \par Method of function operation:
 * Adds the msg_sent_status_cb API to sent status callback list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be called after Message handle is opened.
 *
 * \param input - handle is Message handle.
 * \param input - cb is a function to be called.
 * \param input - user_param is a pointer to user data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_MSGHANDLE	Message handle is invalid.
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
 * int err = MSG_SUCCESS;
 * MSG_REQUEST_S req;
 *
 * req.msg = msg;
 * req.sendOpt = sendOpt;

 * err = msg_reg_sent_status_callback(msgHandle, &sentStatusCB, (void*)"sent status callback");
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 * return;
 * }

 * void sentStatusCB(MSG_HANDLE_T Handle, MSG_SENT_STATUS_S *pStatus, void *pUserParam)
 * {
 * 	...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_reg_sent_status_callback(MSG_HANDLE_T handle, msg_sent_status_cb cb, void *user_param);


/**

 * \par Description:
 * Registers incoming SMS callback to Message handle.
 *
 * \par Purpose:
 * This API is used to Registers incoming SMS callback function "msg_sms_incoming_cb" to Message handle.
 *
 * \par Typical use case:
 * Register incoming SMS message callback.
 *
 * \par Method of function operation:
 * Adds the msg_sms_incoming_cb API to incoming SMS callback list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be called after Message handle is opened.
 *
 * \param input - handle is Message handle.
 * \param input - cb is a function to be called.
 * \param input - port is used for listening. If port is not used, please assign 0 to it.
 * \param input - user_param is a pointer to user data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_MSGHANDLE	Message handle is invalid.
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
 * int err = MSG_SUCCESS;
 * MSG_REQUEST_S req;
 *
 * req.msg = msg;
 * req.sendOpt = sendOpt;

 * err = msg_reg_sms_message_callback(msgHandle, &incomingSmsCB, 0, (void*)"sms message callback");
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 * return;
 * }
 *
 * void incomingSmsCB(MSG_HANDLE_T Handle, msg_message_t msg, void *pUserParam)
 * {
 * 	...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_reg_sms_message_callback(MSG_HANDLE_T handle, msg_sms_incoming_cb cb, unsigned short port, void *user_param);


/**

 * \par Description:
 * Registers incoming MMS callback to Message handle.
 *
 * \par Purpose:
 * This API is used to Registers incoming MMS callback function "msg_mms_conf_msg_incoming_cb" to Message handle.
 *
 * \par Typical use case:
 * Register incoming SMS message callback.
 *
 * \par Method of function operation:
 * Adds the msg_mms_conf_msg_incoming_cb API to incoming MMS callback list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be called after Message handle is opened.
 *
 * \param input - handle is Message handle.
 * \param input - handle is Message handle.
 * \param input - cb is a function to be called.
 * \param input - app_id is used for listening. If appId is not used, please assign NULL to it.
 * \param input - user_param is a pointer to user data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_MSGHANDLE	Message handle is invalid.
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
 * int err = MSG_SUCCESS;
 * MSG_REQUEST_S req;
 *
 * req.msg = msg;
 * req.sendOpt = sendOpt;

 * err = msg_reg_mms_conf_message_callback(msgHandle, &incomingMmsConfCB, NULL, NULL);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 * return;
 * }
 *
 * void incomingMmsConfCB(MSG_HANDLE_T Handle, msg_message_t msg, void *pUserParam)
 * {
 * 	...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_reg_mms_conf_message_callback(MSG_HANDLE_T handle, msg_mms_conf_msg_incoming_cb cb, const char *app_id, void *user_param);


/**

 * \par Description:
 * Registers incoming SyncML Message callback to Message handle.
 *
 * \par Purpose:
 * This API is used to Registers incoming SyncML Message callback function "msg_syncml_msg_incoming_cb" to Message handle.
 *
 * \par Typical use case:
 * Register incoming SMS message callback.
 *
 * \par Method of function operation:
 * Adds the msg_syncml_msg_incoming_cb API to incoming SyncML callback list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be called after Message handle is opened.
 *
 * \param input - handle is Message handle.
 * \param input - cb is a function to be called.
 * \param input - user_param is a pointer to user data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS							Success in operation.
 * - MSG_ERR_MSGHANDLE_NOT_CONNECTED	Message handle is not connected.
 * - MSG_ERR_MEMORY_ERROR				Memory is error.
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
 * int err = MSG_SUCCESS;
 *
 * err = err = msg_reg_syncml_message_callback(msgHandle, &syncMLCB, NULL);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 * return;
 * }
 *
 * void syncMLCB(MSG_HANDLE_T hMsgHandle, MSG_SYNCML_MESSAGE_TYPE_T msgType, const char* pPushHeader, int PushHeaderLen, const char* pPushBody, int PushBodyLen, void *pUserParam)
 * {
 * 	...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_reg_syncml_message_callback(MSG_HANDLE_T handle,  msg_syncml_msg_incoming_cb cb, void *user_param);


/**

 * \par Description:
 * Registers incoming LBS Message callback to Message handle.
 *
 * \par Purpose:
 * This API is used to Registers incoming LBS Message callback function "msg_lbs_msg_incoming_cb" to Message handle.
 *
 * \par Typical use case:
 * Register incoming SMS message callback.
 *
 * \par Method of function operation:
 * Adds the msg_lbs_msg_incoming_cb API to incoming LBS Message callback list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be called after Message handle is opened.
 *
 * \param input - handle is Message handle.
 * \param input - cb is a function to be called.
 * \param input - user_param is a pointer to user data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS							Success in operation.
 * - MSG_ERR_MSGHANDLE_NOT_CONNECTED	Message handle is not connected.
 * - MSG_ERR_MEMORY_ERROR				Memory is error.
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
 * int err = MSG_SUCCESS;
 *
 * err = msg_reg_lbs_message_callback(msgHandle, &lbsCB, NULL);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 * return;
 * }
 *
 * void lbsCB(MSG_HANDLE_T hMsgHandle, const char* pPushHeader, const char* pPushBody, int pushBodyLen, void *pUserParam)
 * {
 * 	...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_reg_lbs_message_callback(MSG_HANDLE_T handle, msg_lbs_msg_incoming_cb cb, void *user_param);


/**

 * \par Description:
 * Registers incoming LBS Message callback to Message handle.
 *
 * \par Purpose:
 * This API is used to Registers incoming LBS Message callback function "msg_lbs_msg_incoming_cb" to Message handle.
 *
 * \par Typical use case:
 * Register incoming SMS message callback.
 *
 * \par Method of function operation:
 * Adds the msg_lbs_msg_incoming_cb API to incoming LBS Message callback list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be called after Message handle is opened.
 *
 * \param input - handle is Message handle.
 * \param input - cb is a function to be called.
 * \param input - user_param is a pointer to user data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS							Success in operation.
 * - MSG_ERR_MSGHANDLE_NOT_CONNECTED	Message handle is not connected.
 * - MSG_ERR_MEMORY_ERROR				Memory is error.
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
 * int err = MSG_SUCCESS;
 *
 * err = msg_reg_lbs_message_callback(msgHandle, &lbsCB, NULL);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 * return;
 * }
 *
 * void lbsCB(MSG_HANDLE_T hMsgHandle, const char* pPushHeader, const char* pPushBody, int pushBodyLen, void *pUserParam)
 * {
 * 	...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_reg_syncml_message_operation_callback(MSG_HANDLE_T handle,  msg_syncml_msg_operation_cb cb, void *user_param);


/**

 * \par Description:
 * Registers incoming LBS Message callback to Message handle.
 *
 * \par Purpose:
 * This API is used to Registers incoming LBS Message callback function "msg_lbs_msg_incoming_cb" to Message handle.
 *
 * \par Typical use case:
 * Register incoming SMS message callback.
 *
 * \par Method of function operation:
 * Adds the msg_lbs_msg_incoming_cb API to incoming LBS Message callback list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be called after Message handle is opened.
 *
 * \param input - handle is Message handle.
 * \param input - cb is a function to be called.
 * \param input - user_param is a pointer to user data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS							Success in operation.
 * - MSG_ERR_MSGHANDLE_NOT_CONNECTED	Message handle is not connected.
 * - MSG_ERR_MEMORY_ERROR				Memory is error.
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
 * int err = MSG_SUCCESS;
 *
 * err = msg_reg_lbs_message_callback(msgHandle, &lbsCB, NULL);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 * return;
 * }
 *
 * void lbsCB(MSG_HANDLE_T hMsgHandle, const char* pPushHeader, const char* pPushBody, int pushBodyLen, void *pUserParam)
 * {
 * 	...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_syncml_message_operation(MSG_HANDLE_T handle,  MSG_MESSAGE_ID_T msgId);


/**

 * \par Description:
 * Sends SMS. It is a synchronous API which has been blocked until sent status arrives.
 *
 * \par Purpose:
 * This API is used to sends SMS.
 *
 * \par Typical use case:
 * Sends a SMS Message
 *
 * \par Method of function operation:
 * It is a synchronous API which has been blocked until sent status arrives.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input  - phone_num is the list of phone numbers. It is separated by ",".
 * \param input  - sms_text is a SMS text.
 * \param input  - cb is a function to be called.
 * \param input  - user_param is for user data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS				Success in operation.
 * - MSG_ERR_NULL_POINTER		Invalid parameter.
 * - MSG_ERR_MEMORY_ERROR	Memory is error.
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
 * int err = MSG_SUCCESS;
 *
 * err = msg_sms_send("01000000000,01000000000", "1234567890", sentStatusCB, NULL);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_sms_send(const char *phone_num, const char *sms_text, msg_simple_sent_status_cb cb, void *user_param);


/**

 * \par Description:
 * Submits request to send SMS message.
 *
 * \par Purpose:
 * This API is used to submit request to send SMS message.
 *
 * \par Typical use case:
 * Submits request to send SMS message.
 *
 * \par Method of function operation:
 * Submits a request to send SMS.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input  - phone_num is the list of phone numbers. It is separated by ",".
 * \param input  - sms_text is a SMS text.
 * \param input  - cb is a function to be called.
 * \param input  - user_param is for user data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS				Success in operation.
 * - MSG_ERR_NULL_POINTER		Invalid parameter.
 * - MSG_ERR_MEMORY_ERROR	Memory is error.
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
 * int err = MSG_SUCCESS;
 * MSG_REQUEST_S req;
 *
 * req.msg = msg;
 * req.sendOpt = sendOpt;
 *
 * err = msg_sms_send_message(msgHandle, &req);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_sms_send_message(MSG_HANDLE_T handle, MSG_REQUEST_S* req);


/**

 * \par Description:
 * Submits request to send MMS message.
 *
 * \par Purpose:
 * This API is used to submit request to send MMS message.
 *
 * \par Typical use case:
 * Submits request to send MMS message.
 *
 * \par Method of function operation:
 * Submits a request to send MMS.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input  - handle is Message handle.
 * \param input  - req is a pointer to an MSG_REQUEST_S structure.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS				Success in operation.
 * - MSG_ERR_NULL_POINTER		Invalid parameter.
 * - MSG_ERR_MEMORY_ERROR	Memory is error.
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
 * int err = MSG_SUCCESS;
 * MSG_REQUEST_S req;
 *
 * req.msg = msg;
 * req.sendOpt = sendOpt;
 *
 * err = msg_mms_send_message(msgHandle, &req);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_mms_send_message(MSG_HANDLE_T handle, MSG_REQUEST_S* req);


/**

 * \par Description:
 * Submits request to send MMS read report request.
 *
 * \par Purpose:
 * This API is used to submit request to send MMS read report request.
 *
 * \par Typical use case:
 * Submits request to send MMS read report request.
 *
 * \par Method of function operation:
 * Submits a request to send MMS read report request.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input  - handle is Message handle.
 * \param input  - msg_id is a message id, which is a positive integer.
 * \param input  - mms_read_status is status whether message was read or not.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS				Success in operation.
 * - MSG_ERR_NULL_POINTER		Invalid parameter.
 * - MSG_ERR_MEMORY_ERROR	Memory is error.
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
 * int err = MSG_SUCCESS;
 *
 * err = err = msg_mms_send_read_report(NULL, 0, MSG_READ_REPORT_IS_READ);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_mms_send_read_report(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msgId, MSG_READ_REPORT_STATUS_T mms_read_status);


/**

 * \par Description:
 * Submits request to send forward MMS request.
 *
 * \par Purpose:
 * This API is used to submit request to send forward MMS request.
 *
 * \par Typical use case:
 * Submits request to send forward MMS request.
 *
 * \par Method of function operation:
 * Submits a request to send forward MMS request.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input  - handle is Message handle.
  * \param input  - req is a pointer to an MSG_REQUEST_S structure.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_MSGHANDLE		Message handle is invalid.
 * - MSG_ERR_NULL_POINTER				Pointer is NULL.
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
 * int err = MSG_SUCCESS;
 *
 * err = err = msg_mms_send_read_report(NULL, 0, MSG_READ_REPORT_IS_READ);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_mms_forward_message(MSG_HANDLE_T handle, MSG_REQUEST_S* req);


/**

 * \par Description:
 * Submits request to retrieve MMS request.
 *
 * \par Purpose:
 * This API is used to submit request to retrieve MMS request.
 *
 * \par Typical use case:
 * Submits request to retrieve MMS request.
 *
 * \par Method of function operation:
 * Submits a request to send forward MMS request.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input  - handle is Message handle.
  * \param input  - req is a pointer to an MSG_REQUEST_S structure.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_MSGHANDLE		Message handle is invalid.
 * - MSG_ERR_NULL_POINTER				Pointer is NULL.
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
 * int err = MSG_SUCCESS;
 *
 * err = msg_mms_retrieve_message(handle, &req);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_mms_retrieve_message(MSG_HANDLE_T handle, MSG_REQUEST_S* req);


/* reject_msg_support */
/**

 * \par Description:
 * Submits request to reject MMS message.
 *
 * \par Purpose:
 * This API is used to submit request to reject MMS message.
 *
 * \par Typical use case:
 * Submits request to reject MMS message.
 *
 * \par Method of function operation:
 * Submits a request to send forward reject MMS message.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input  - handle is Message handle.
  * \param input  - req is a pointer to an MSG_REQUEST_S structure.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_MSGHANDLE		Message handle is invalid.
 * - MSG_ERR_NULL_POINTER				Pointer is NULL.
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
 * int err = MSG_SUCCESS;
 *
 * err = msg_mms_reject_message(handle, &req);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_mms_reject_message(MSG_HANDLE_T handle, MSG_REQUEST_S* req);

/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif // MAPI_TRANSPORT_H

