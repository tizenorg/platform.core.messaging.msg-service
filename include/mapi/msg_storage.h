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

#ifndef MAPI_STORAGE_H
#define MAPI_STORAGE_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "msg_storage_types.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @ingroup MSG_SERVICE_FRAMEWORK
 * @defgroup MSG_SERVICE_FRAMEWORK_STORAGE_MODULE Storage API
 * @brief The Storage API provides functions to get message information with multiple types.
 *
 * @addtogroup MSG_SERVICE_FRAMEWORK_STORAGE_MODULE
 * @{
 *
 * @section MSG_SERVICE_FRAMEWORK_STORAGE_MODULE_HEADER Required Header
 *   \#include <msg_storage.h>
 *
 * @section MSG_SERVICE_FRAMEWORK_STORAGE_MODULE_OVERVIEW Overview
 *
 * The Storage API provides the following functionalities:
 *
 * - Get message information
 *
 * @section MSG_SERVICE_FRAMEWORK_STORAGE_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 *  - http://tizen.org/feature/network.telephony\n
 *  - http://tizen.org/feature/network.telephony.sms\n
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
 * @brief Saves a message to the database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks Mandatory fields of a message structure MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle    The Message handle
 * @param[in] msg       A pointer to a message structure
 * @param[in] send_opt  A pointer to a message structure for sending option
 *
 * @return  The message ID on success,
 *       otherwise a negative error value
 *
 * @retval MESSAGE_ID                    Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER(-9) Invalid parameter
 * @retval MSG_ERR_STORAGE_ERROR         Storage error
 * @retval MSG_ERR_PERMISSION_DENIED     The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED         Not supported
 */

int msg_add_message(msg_handle_t handle, const msg_struct_t msg, const msg_struct_t send_opt);


/**
 * @brief Adds a SyncML message to the database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks Mandatory fields of a message structure MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle      The Message handle
 * @param[in] syncml_msg  A pointer to a syncml message structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_add_syncml_message(msg_handle_t handle, const msg_struct_t syncml_msg);


/**
 * @brief Updates a message in the database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks The function is to update message data for the message identified by the given msgId as long as the given values are valid.
 * @remarks Message ID MUST NOT be updated because that is a unique ID on platform.
 * @remarks If applications want to move a message between folders, applications SHOULD call msg_move_to_folder().
 * @remarks Storage ID MUST NOT be updated.
 * @remarks If applications want to move the message between storages, applications SHOULD call msg_move_to_storage().
 *
 * @param[in] handle   The Message handle
 * @param[in] msg      A pointer to a message structure
 * @param[in] send_opt A pointer to a message sending option structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_update_message(msg_handle_t handle, const msg_struct_t msg, const msg_struct_t send_opt);


/**
 * @brief Updates a message's read status in the database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle  The message handle
 * @param[in] msg_id  The message ID
 * @param[in] read    Set @c true if the message is read,
 *                    otherwise set @c false if the message is not read
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_update_read_status(msg_handle_t handle, msg_message_id_t msg_id, bool read);


/**
 * @brief Updates a message's protected status in the database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle        The message handle
 * @param[in] msg_id        The message ID
 * @parem[in] is_protected  Set @c true if a message is protected,
 *                          otherwise set @c false if message is not protected
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_update_protected_status(msg_handle_t handle, msg_message_id_t msg_id, bool is_protected);


/**
 * @brief Deletes a message by Message ID from the database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle  The message handle
 * @param[in] msg_id  The message ID of the message to be deleted
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_delete_message(msg_handle_t handle, msg_message_id_t msg_id);


/**
 * @brief Deletes all messages in the specified folder from the database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle   Message handle
 * @param[in] msg_id   Message ID of the message to be deleted
 * @param[in] bOnlyDB  Set @c true to not delete messages in SIM,
 *                     otherwise set @c false to delete messages in SIM
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_delete_all_msgs_in_folder(msg_handle_t handle, msg_folder_id_t folder_id, bool bOnlyDB);



/**
 * @brief Moves a message to the specified folder in the database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle         The message handle
 * @param[in] msg_id         The message ID of the message to be moved
 * @param[in] dest_folder_id The ID of the destination folder
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_move_msg_to_folder(msg_handle_t handle, msg_message_id_t msg_id, msg_folder_id_t dest_folder_id);


/**
 * @brief Moves a message to the other storage.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle      The message handle
 * @param[in] msg_id      The message ID of the message to be moved
 * @param[in] storage_id  The ID of the destination storage
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_move_msg_to_storage(msg_handle_t handle, msg_message_id_t msg_id, msg_storage_id_t storage_id);


/**
 * @brief Gets the number of messages in the specified folder from the database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle      The message handle
 * @param[in]  msg_id      The message ID of the message to be counted
 * @param[out] count_info  A pointer to an #MSG_COUNT_INFO_S structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_count_message(msg_handle_t handle, msg_folder_id_t folder_id, msg_struct_t count_info);


/**
 * @brief Gets the number of messages of specific message type.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle     The message handle
 * @param[in]  msg_type   The message type to be counted
 * @param[out] msg_count  A pointer to the number of message
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_count_msg_by_type(msg_handle_t handle, msg_message_type_t msg_type, int *msg_count);


/**
 * @brief Gets the number of messages of specific address.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks If @a addr_info is @c NULL, nothing happens.
 *
 * @param[in] handle                 The message handle
 * @param[in] addr_info              A pointer to an address list information structure
 * @param[in] msg_thread_count_list  A pointer to an thread count information structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_count_msg_by_contact(msg_handle_t handle, const msg_struct_t addr_info, msg_struct_t msg_thread_count_list);


/**
 * @brief Gets the detail information of a message from the database.
 * @details This API is used to get the detail information of message by message ID.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks Applications need to call msg_release_struct() to free the memory.
 * @remarks However, if this function fails, the memory for the message is NOT allocated in this function.
 *
 * @param[in]  handle    The Message handle
 * @param[in]  msg_id    The ID of the Message to be returned
 * @param[out] msg       A pointer to a #msg_struct_t message structure
 * @param[in]  send_opt  A pointer to a #msg_struct_t sending option structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_message(msg_handle_t handle, msg_message_id_t msg_id, msg_struct_t msg, msg_struct_t send_opt);


/**
 * @brief Gets the detail information of a message on conversation list.
 * @details This API is used to get the conversation informations of message by message ID.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle  The Message handle
 * @param[in]  msg_id  The ID of the Message to be returned
 * @param[out] conv    A pointer to a #msg_struct_t of message structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_conversation(msg_handle_t handle, msg_message_id_t msg_id, msg_struct_t conv);


/**
 * @brief Gets the v-object data of message.
 * @details This API is used to get the v-object data of message by message ID.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle        The Message handle
 * @param[in]  msg_id        The ID of the Message to be returned
 * @param[out] encoded_data  A pointer to a encoded v-object data of message
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_vobject_data(msg_handle_t handle, msg_message_id_t msg_id, void** encoded_data);


/**
 * @brief Gets the information of all peers to whom messages have been sent or received.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks The memory for a list will be allocated in this function.
 * @remarks Applications need to call msg_release_thread_view_list() to free the memory.
 * @remarks However, if this function fails, the memory for a list is NOT allocated in this function.
 *
 * @param[in]  handle                The Message handle
 * @param[in]  sourt_rule            This indicates a sort type and sort order for querying messages
 * @param[out] msg_thread_view_list  A pointer to an #msg_struct_list_s structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_thread_view_list(msg_handle_t handle, const msg_struct_t sort_rule, msg_struct_list_s *msg_thread_view_list);


/**
 * @briefs Gets the common information list of messages with the selected thread ID.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks The memory for a list will be allocated in this function.
 * @remarks Applications need to call msg_release_conversation_view_list() to free the memory.
 * @remarks However, if this function is failed, the memory for a list is NOT allocated in this function.
 *
 * @param[in]  hMsgHandle          The Message handle
 * @param[in]  thread_id           The ID of the thread to be returned
 * @param[out] msg_conv_view_list  A pointer to a structure of conversational message list
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_conversation_view_list(msg_handle_t handle, msg_thread_id_t thread_id, msg_struct_list_s *msg_conv_view_list);


/**
 * @brief Deletes all the Messages Sent/Received from the selected list.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle                 The Message handle
 * @param[in] thread_id              The ID of the thread to be deleted
 * @param[in] include_protected_msg  Set @c true to delete protected messages,
 *                                   otherwise @c false to not delete protected messages
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_delete_thread_message_list(msg_handle_t handle, msg_thread_id_t thread_id, bool include_protected_msg);


/**
 * @brief Adds a new folder.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle      The Message handle
 * @param[in] folder_info A pointer to an #MSG_FOLDER_INFO_S structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_add_folder(msg_handle_t handle, const msg_struct_t folder_info);


/**
 * @brief Updates the folder info.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle       The Message handle
 * @param[in] folder_info  A pointer to an #MSG_FOLDER_INFO_S structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_update_folder(msg_handle_t handle, const msg_struct_t folder_info);


/**
 * @brief Deletes an existing folder.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle    The Message handle
 * @param[in] folder_id The ID of the folder to be deleted
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_delete_folder(msg_handle_t handle, msg_folder_id_t folder_id);


/**
 * @brief Gets the information list of folders.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle       The Message handle
 * @param[out] folder_list  A pointer to a #msg_struct_list_s structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_folder_list(msg_handle_t handle, msg_struct_list_s *folder_list);


/**
 * @brief Creates the specified number of messages in database.
 * @details This API is used to generate specified number of messages in the database
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle     The Message handle
 * @param[in] msg_type   The message type \n
 *                       One of enum _MSG_MESSAGE_TYPE_E.
 * @param[in] folder_id  The folder for the test messages
 * @param[in] num_msg    The number of messages
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_MSGHANDLE Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_generate_message(msg_handle_t handle, msg_message_type_t msg_type, msg_folder_id_t folder_id, unsigned int num_msg);


/**
 * @brief Returns the Message Data to be used by the Quick Panel.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle  The Message handle
 * @param[in]  type    The type of message that Quick Panel needs
 * @param[out] msg     A pointer to a #msg_struct_t structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS                   Success in operation
 * @retval MSG_ERR_DB_STEP               There is no Quick Panel message
 * @retval MSG_ERR_INVALID_PARAMETER(-9) Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR         Storage  error
 * @retval MSG_ERR_PERMISSION_DENIED     The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_quick_panel_data(msg_handle_t handle, msg_quickpanel_type_t type, msg_struct_t msg);


/**
 * @brief Resets the Messaging database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle The Message handle
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_reset_database(msg_handle_t handle);


/**
 * @brief Gets the total size used for message contents.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle    The Message handle
 * @param[out] memsize   A pointer to the size
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_mem_size(msg_handle_t handle, unsigned int* memsize);

/**
 * @brief Backs up messages to storage.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in] handle          The Message handle
 * @param[in] type            The backup type
 * @param[in] backup_filepath The path to backup message
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_backup_message(msg_handle_t handle, msg_message_backup_type_t type, const char *backup_filepath) DEPRECATED;


/**
 * @brief Restores messages from backed up messages.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle          The Message handle
 * @param[in] backup_filepath The path to backup message
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_restore_message(msg_handle_t handle, const char *backup_filepath) DEPRECATED;


/**
 * @brief Searches messages or addresses for the specified string.
 * @details This API is used to search messages or addresses from storage.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle               The Message handle
 * @param[in]  search_string        The string to search
 * @param[out] msg_thread_view_list A pointer to an #msg_struct_list_s structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_search_message_for_thread_view(msg_handle_t handle, const char *search_string, msg_struct_list_s *msg_thread_view_list);


/**
 * @brief Gets reject message list by phone number.
 * @details This API is used to get reject message list from storage.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle               The Message handle
 * @param[in]  phone_num            The string of phone number to find
 * @param[out] msg_reject_msg_list  A pointer to a #msg_struct_list_s structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_reject_msg_list(msg_handle_t handle, const char* phone_num, msg_struct_list_s *msg_reject_msg_list);


/**
 * @brief Registers a callback function about the change of storage status to Message handle.
 * @details This API is used to register a callback function about the change of storage status "msg_storage_change_cb" to Message handle.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks This function MUST be called after Message handle is opened.
 *
 * @param[in] handle      The Message handle
 * @param[in] cb          The function to be called
 * @param[in] user_param  A pointer to user data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_MSGHANDLE_NOT_CONNECTED Message handle is not connected
 * @retval MSG_ERR_MEMORY_ERROR	 Memory is error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_reg_storage_change_callback(msg_handle_t handle, msg_storage_change_cb cb, void *user_param);


/**
 * @brief Gets the report status information of message.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle        The Message handle
 * @param[in]  msg_id        The ID of the message
 * @param[out] report_status A pointer to a #msg_struct_list_s structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_report_status(msg_handle_t handle, msg_message_id_t msg_id, msg_struct_list_s *report_list);


/**
 * @brief Gets the address list for specific thread ID.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle           The Message handle
 * @param[in]  msg_id           The ID of the message
 * @param[out] msg_address_list A pointer to a #msg_struct_list_s structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_address_list(msg_handle_t handle, msg_thread_id_t thread_id, msg_struct_list_s *msg_address_list);


/**
 * @brief Gets the thread ID by address.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle           The Message handle
 * @param[in]  msg_address_list A pointer to a #msg_struct_list_s structure
 * @param[out] thread_id        The thread ID of the message
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_thread_id_by_address(msg_handle_t handle, msg_struct_list_s *msg_address_list, msg_thread_id_t *thread_id);

/**
 * @brief Gets the thread ID by address.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle            The Message handle
 * @param[in]  msg_address_list  A pointer to a #msg_list_handle structure
 * @param[out] thread_id         The thread ID of the message
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_thread_id_by_address2(msg_handle_t handle, msg_list_handle_t msg_address_list, msg_thread_id_t *thread_id);



/**
 * @brief Gets the thread information.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle      The Message handle
 * @param[in]  thread_id   The ID of the thread
 * @param[out] msg_thread  A pointer to a #msg_struct_t structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_thread(msg_handle_t handle, msg_thread_id_t thread_id, msg_struct_t msg_thread);


/**
 * @brief Gets the information list of messages.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @param[in]  handle               The Message handle
 * @param[in]  msg_list_conditions  A pointer to a #msg_struct_t structure for getting conditions
 * @param[out] msg_list             A pointer to a #msg_struct_list_s structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_message_list2(msg_handle_t handle, const msg_struct_t msg_list_conditions, msg_struct_list_s *msg_list);


/**
 * @brief A function to get media list of a chat room.
 *
 * @since_tizen 2.3.1
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks This function MUST be called after Message handle is opened.
 *
 * @param[in] handle        The Message handle
 * @param[in] thread_id     The thread id of conversation to get media list
 * @param[out] msg_list     The media file list in a conversation
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_CALLBACK_ERROR    Callback registration error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 */

int msg_get_media_list(msg_handle_t handle, msg_thread_id_t thread_id, msg_list_handle_t *msg_list);


/**
 * @brief Adds a new push event.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle     The Message handle
 * @param[in] push_event A pointer to a #msg_struct_t structure for push event
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_add_push_event(msg_handle_t handle, const msg_struct_t push_event);


/**
 * @brief Deletes a push event.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle     The Message handle
 * @param[in] push_event A pointer to a #msg_struct_t structure for push event
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_delete_push_event(msg_handle_t handle, const msg_struct_t push_event);


/**
 * @brief Updates a push event.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle    The Message handle
 * @param[in] src_event A pointer to a #msg_struct_t structure for source push event
 * @param[in] dst_event A pointer to a #msg_struct_t structure for destination push event
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_update_push_event(msg_handle_t handle, const msg_struct_t src_event, const msg_struct_t dst_event);


/**
 * @brief Deletes messages by Message ID list from the database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle      sThe Message handle
 * @param[in] msg_id_list The message ID list to be deleted
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_delete_msgs_by_list(msg_handle_t handle, msg_id_list_s *msg_id_list);


/**
 * @brief Marks a conversation given by thread ID as read.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle    The Message handle
 * @param[in] thread_id The thread ID to be updated
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_set_conversation_to_read(msg_handle_t handle,  msg_thread_id_t thread_id);


/**
 * @brief Gets a DB records with specified query.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks You must release @a db_res using msg_db_free().
 * @remarks You should set @a query with SQL query string after 'SELECT'.
 *
 * @param[in] handle     The Message handle
 * @param[in] query      The SQL SELECT query
 * @param[out] db_res    The result of SQL SELECT query
 * @param[out] row_count The row count of result
 * @param[out] col_count The column count of result
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_DB_BUSY           DB operation is busy
 * @retval MSG_ERR_DB_GETTABLE       DB get table operation is failed
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 * @see msg_db_free()
 */
int msg_db_select_with_query(msg_handle_t handle, const char *query, char ***db_res, int *row_count, int *col_count);

/**
 * @brief Release memory for result of SQL query.
 *
 * @since_tizen 3.0
 *
 * @param[in] handle     The Message handle
 * @param[in] db_res     The result of SQL SELECT query
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER Parameter is invalid
 * @retval MSG_ERR_STORAGE_ERROR     Storage error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 * @see msg_db_select_with_query()
 */
int msg_db_free(msg_handle_t handle, char **db_res);
/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif // MAPI_STORAGE_H

