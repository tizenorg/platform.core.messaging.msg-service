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

#ifndef MAPI_TRANSPORT_H
#define MAPI_TRANSPORT_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "msg_transport_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @ingroup MSG_SERVICE_FRAMEWORK
 * @defgroup MSG_SERVICE_FRAMEWORK_TRANSPORT_MODULE Transport API
 * @brief The Transport API provides functions to send SMS/MMS and register incoming/sending/syncML/report callback.
 *
 * @addtogroup MSG_SERVICE_FRAMEWORK_TRANSPORT_MODULE
 * @{
 *
 * @section MSG_SERVICE_FRAMEWORK_TRANSPORT_MODULE_HEADER Required Header
 *   \#include <msg_transport.h>
 *
 * @section MSG_SERVICE_FRAMEWORK_TRANSPORT_MODULE_OVERVIEW Overview
 *
 * The Transport API provides the following functionalities:
 *
 * - Sending SMS/MMS messages
 * - Register incoming message callback
 * - Register sent status callback
 * - Register push message application
 * - Register syncML message callback
 * - Managing the registration
 *
 * @section MSG_SERVICE_FRAMEWORK_TRANSPORT_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 *  - http://tizen.org/feature/network.telephony\n
 *  - http://tizen.org/feature/network.telephony.sms\n
 *  - http://tizen.org/feature/network.telephony.mms\n
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="../org.tizen.mobile.native.appprogramming/html/ide_sdk_tools/feature_element.htm"><b>Feature Element</b>.</a>
 *
 */

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

/**
 * @brief Submits a request to the Messaging Framework.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks The result information will be sent back by using the callback function, msg_sent_status_cb().
 * @remarks Applications MUST fill in the valid message type.
 * @remarks reqId will be filled in the framework.
 *
 * @param[in] handle  The message handle
 * @param[in] req     The pointer to an #MSG_REQUEST_S structure
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_TRANSPORT_ERROR   Transport error
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_submit_req(msg_handle_t handle, msg_struct_t req);


/**
 * @brief Registers sent status callback function to Message handle.
 * @details This API is used to register sent status callback function msg_sent_status_cb() to Message handle.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks This function MUST be called after Message handle is opened.
 *
 * @param[in] handle      The message handle
 * @param[in] cb          The function to be called
 * @param[in] user_param  A pointer to user data
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_CALLBACK_ERROR    Callback registration error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_reg_sent_status_callback(msg_handle_t handle, msg_sent_status_cb cb, void *user_param);


/**
 * @brief Registers incoming SMS callback to Message handle.
 * @details This API is used to Registers incoming SMS callback function msg_sms_incoming_cb() to Message handle.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks This function MUST be called after Message handle is opened.
 *
 * @param[in] handle      The message handle
 * @param[in] cb          The function to be called
 * @param[in] port        The port used for listening \n
 *                        If port is not used, set to @c 0.
 * @param[in] user_param  A pointer to user data
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_CALLBACK_ERROR    Callback registration error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_reg_sms_message_callback(msg_handle_t handle, msg_sms_incoming_cb cb, unsigned short port, void *user_param);


/**
 * @brief Registers incoming MMS conf callback to Message handle.
 * @details This API is used to Registers incoming MMS conf callback function msg_mms_conf_msg_incoming_cb() to Message handle.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks This function MUST be called after Message handle is opened.
 *
 * @param[in] handle      The message handle
 * @param[in] cb          The function to be called
 * @param[in] app_id      The app ID used for listening \n
 *                        If appId is not used, set to @c NULL.
 * @param[in] user_param  A pointer to user data
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_CALLBACK_ERROR    Callback registration error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_reg_mms_conf_message_callback(msg_handle_t handle, msg_mms_conf_msg_incoming_cb cb, const char *app_id, void *user_param);


/**
 * @brief Registers incoming SyncML Message callback to Message handle.
 * @details This API is used to register incoming SyncML Message callback function msg_syncml_msg_incoming_cb() to Message handle.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks This function MUST be called after Message handle is opened.
 *
 * @param[in] handle      The message handle
 * @param[in] cb          The function to be called
 * @param[in] user_param  A pointer to user data
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_CALLBACK_ERROR    Callback registration error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_reg_syncml_message_callback(msg_handle_t handle,  msg_syncml_msg_incoming_cb cb, void *user_param);


/**
 * @brief Registers incoming LBS Message callback to Message handle.
 * @details This API is used to register incoming LBS Message callback function msg_lbs_msg_incoming_cb() to Message handle.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks This function MUST be called after Message handle is opened.
 *
 * @param[in] handle      The message handle
 * @param[in] cb          The function to be called
 * @param[in] user_param  A pointer to user data
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_CALLBACK_ERROR    Callback registration error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_reg_lbs_message_callback(msg_handle_t handle, msg_lbs_msg_incoming_cb cb, void *user_param);


/**
 * @brief Registers SyncML operation callback to Message handle.
 * @details This API is used to register SyncML operation callback function msg_syncml_msg_operation_cb() to Message handle.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks This function MUST be called after Message handle is opened.
 *
 * @param[in] handle      The message handle
 * @param[in] cb          The function to be called
 * @param[in] user_param  A pointer to user data
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_CALLBACK_ERROR    Callback registration error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_reg_syncml_message_operation_callback(msg_handle_t handle,  msg_syncml_msg_operation_cb cb, void *user_param);


/**
 * @brief Registers incoming push Message callback to Message handle.
 * @details This API is used to register incoming push Message callback function msg_push_msg_incoming_cb() to Message handle.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks This function MUST be called after Message handle is opened.
 *
 * @param[in] handle      The message handle
 * @param[in] cb          The function to be called
 * @param[in] app_id      The app ID for listening \n
 *                        If appId is not used, set to @c NULL.
 * @param[in] user_param  A pointer to user data
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_CALLBACK_ERROR    Callback registration error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_reg_push_message_callback(msg_handle_t handle,  msg_push_msg_incoming_cb cb, const char *app_id, void *user_param);


/**
 * @brief Registers incoming CB Message callback to Message handle.
 * @details This API is used to register incoming CB Message callback function msg_cb_incoming_cb() to Message handle.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks This function MUST be called after Message handle is opened.
 *
 * @param[in] handle      The message handle
 * @param[in] cb          The function to be called
 * @param[in] bsave       A bool flag to indicate whether CB message is to be saved or not \n
 *                        CB message will be saved if this flag is true
 * @param[in] user_param  A pointer to user data
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_CALLBACK_ERROR    Callback registration error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_reg_cb_message_callback(msg_handle_t handle, msg_cb_incoming_cb  cb, bool bsave, void *user_param);



/**
 * @brief Registers incoming Report Message callback to Message handle.
 * @details This API is used to register incoming Report Message callback function msg_report_msg_incoming_cb() to Message handle.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks This function MUST be called after Message handle is opened.
 *
 * @param[in] handle      The message handle
 * @param[in] cb          The function to be called
 * @param[in] user_param  A pointer to user data
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_CALLBACK_ERROR    Callback registration error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_reg_report_message_callback(msg_handle_t handle, msg_report_msg_incoming_cb cb, void *user_param);


/**
 * @brief Operates SyncML message.
 * @details This API is used to run SyncML operation.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks This function MUST be called after Message handle is opened.
 *
 * @param[in] handle  The message handle
 * @param[in] msgId   The message ID to run SycnML operation
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_TRANSPORT_ERROR   Transport error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_syncml_message_operation(msg_handle_t handle, msg_message_id_t msgId);


/**
 * @brief Sends SMS.
 * @details This API is used to send SMS. It is a synchronous API which has been blocked until sent status arrives.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] phone_num   The list of phone numbers \n
 *                        It is separated by ",".
 * @param[in] sms_text    The SMS text
 * @param[in] cb          The function to be called
 * @param[in] user_param  The user data
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_MEMORY_ERROR      Memory error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 * \par Prospective clients:
 * External/Native Apps using Messaging Services.
 *
 */

int msg_sms_send(const char *phone_num, const char *sms_text, msg_simple_sent_status_cb cb, void *user_param);


/**
 * @brief Submits request to send SMS message.
 * @details This API is used to submit request to send SMS message.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle The message handle
 * @param[in] req    A pointer to #msg_struct_t structure for SMS request information
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_NULL_POINTER      Null parameter
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_sms_send_message(msg_handle_t handle, msg_struct_t req);


/**
 * @brief Submits request to send MMS message.
 * @details This API is used to submit request to send MMS message.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle The message handle
 * @param[in] req    A pointer to #msg_struct_t structure for MMS request information
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_mms_send_message(msg_handle_t handle, msg_struct_t req);


/**
 * @brief Submits request to send MMS read report request.
 * @details This API is used to submit request to send MMS read report request.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle          The message handle
 * @param[in] msg_id          The message ID \n
 *                            This is a positive integer.
 * @param[in] mms_read_status This is status whether message was read or not
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_mms_send_read_report(msg_handle_t handle, msg_message_id_t msgId, msg_read_report_status_t mms_read_status);


/**
 * @brief Submits request to send forward MMS request.
 * @details This API is used to submit request to send forward MMS request.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle The message handle
 * @param[in] req    A pointer to #msg_struct_t structure for MMS
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_mms_forward_message(msg_handle_t handle, msg_struct_t req);


/**
 * @brief Submits request to retrieve MMS request.
 * @details This API is used to submit request to retrieve MMS request.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle  The message handle
 * @param[in] req     A pointer to #msg_struct_t structure for MMS
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_mms_retrieve_message(msg_handle_t handle, msg_struct_t req);


/**
 * @brief Submits request to reject MMS message.
 * @details This API is used to submit request to reject MMS message.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle  The message handle
 * @param[in] req     A pointer to #msg_struct_t structure for MMS
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_mms_reject_message(msg_handle_t handle, msg_struct_t req);

/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif // MAPI_TRANSPORT_H

