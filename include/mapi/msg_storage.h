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

/**
 *	@file 		MapiStorage.h
 *	@brief 		Defines storage API of messaging framework
 *	@version 	1.0
 */

#ifndef MAPI_STORAGE_H
#define MAPI_STORAGE_H

/**
 *	@section		Introduction
 *	- Introduction : Overview on Messaging Storage API
 *	@section		Program
 *	- Program : Messaging Storage API Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "msg_storage_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_STORAGE_API	Messaging Storage API
 *	@{
 */

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

/**

 * \par Description:
 * Saves a message to the database.
 *
 * \par Purpose:
 * This API is used to save Message object to the database.
 *
 * \par Typical use case:
 * Save Message feature is used when the message is to be stored to persistent memory for later reference.
 *
 * \par Method of function operation:
 * Sets up the database connection and inserts the message to message table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - Mandatory fields of a message structure MUST be valid, otherwise the function will be failed.
 *
 * \param input - msg_handle_t  handle is Message handle.
 * \param input - msg_message_t  msg is a pointer to an msg_message_t structure.
 * \param input - send_opt is a pointer to an MSG_SENDINGOPT_S structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Successfully connected to Messaging Service \n
 * - MSG_ERR_NULL_POINTER	- Input parameter is NULL.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * msg_message_t		msg;
 * MSG_SENDINGOPT_S sendingOpt = {0};
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_add_message(handle, (msg_message_t) &msg, &sendingOpt);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_add_message(msg_handle_t handle, const msg_struct_t msg, const msg_struct_t send_opt);


/**

 * \par Description:
 * Adds a SyncML message to the database.
 *
 * \par Purpose:
 * This API is used to save  a SyncML message to the database.
 *
 * \par Typical use case:
 * Save Message feature is used when the message is to be stored to persistent memory for later reference.
 *
 * \par Method of function operation:
 * Sets up the database connection and inserts the syncml message to message table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - Mandatory fields of a message structure MUST be valid, otherwise the function will be failed.
 *
 * \param input - msg_handle_t  handle is Message handle.
 * \param input - MSG_SYNCML_MESSAGE_S  syncml_msg is a pointer to an MSG_SYNCML_MESSAGE_S structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_NULL_POINTER - pMsg is NULL.
 * - MSG_ERR_INVALID_MSGHANDLE - Message handle is invalid.
 * - MSG_ERR_MSGHANDLE_NOT_CONNECTED - Message handle is not connected.
 * - MSG_ERR_STORAGE_FULL - Storage is FULL.
 * - MSG_ERR_COMMUNICATION_ERROR - Communication between client and server is error.
 * - MSG_ERR_MEMORY_ERROR - Memory is error.
 * - MSG_ERR_MAX_NUMBER_REACHED - Max number is reached.
 * - MSG_ERR_PLUGIN - Generic error code for plugin.
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
 * MSG_SYNCML_MESSAGE_S syncMLMsg;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * int err = msg_add_syncml_message(msgHandle, &syncMLMsg);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_add_syncml_message(msg_handle_t handle, const msg_struct_t syncml_msg);


/**

 * \par Description:
 * Updates a message in the database.
 *
 * \par Purpose:
 * This API is used to update a message in the database.
 *
 * \par Typical use case:
 * Update message feature is used when a previously saved message is to be updated.
 *
 * \par Method of function operation:
 * Sets up the database connection and set the message's new values to message table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - The function is to update message data for the message indentified by the given msgId as long as the given values are valid.
 * - msg->msgId MUST NOT be updated because msg->msgId is a unique Id on platform.
 * - If applications want to move a message between folders, applications SHOULD call msg_move_to_folder.
 * - msg->storageId MUST NOT be updated.
 * - The function will return MSG_ERR_INVALID_MESSAGE, if inputting a new msg->storageId.
 * - If applications want to move the message between storages, applications SHOULD call msg_move_to_storage.
 *
 * \param input - handle is Message handle.
 * \param input - msg is a pointer to an msg_message_t structure.
 * \param input - send_opt is a pointer to an MSG_SENDINGOPT_S structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * MSG_SYNCML_MESSAGE_S syncMLMsg;
 * MSG_SENDINGOPT_S sendingOpt = {0};
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_update_message(hMsgHandle, pMsg, &sendOpt);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_update_message(msg_handle_t handle, const msg_struct_t msg, const msg_struct_t send_opt);


/**

 * \par Description:
 * Updates a message's read status in the database.
 *
 * \par Purpose:
 * This API is used to Updates a message's read status in the database.
 *
 * \par Typical use case:
 * Update message's read status for a previously saved message.
 *
 * \par Method of function operation:
 * Sets up the database connection and updates the message's read status to message table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input - handle is Message handle.
 * \param input - msg_id is Message ID.
 * \parem input - read is boolean for indicating whether a message is read or not.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * MSG_SYNCML_MESSAGE_S syncMLMsg;
 * MSG_SENDINGOPT_S sendingOpt = {0};
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_update_message(hMsgHandle, pMsg, &sendOpt);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_update_read_status(msg_handle_t handle, msg_message_id_t msg_id, bool read);


/**

 * \par Description:
 * Updates a message's protected status in the database.
 *
 * \par Purpose:
 * This API is used to Updates a message's protected status in the database.
 *
 * \par Typical use case:
 * Update message's protected status for a previously saved message.
 *
 * \par Method of function operation:
 * Sets up the database connection and updates the message's protected status to message table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input - handle is Message handle.
 * \param input - msg_id is Message ID.
 * \parem input - is_protected is boolean for indicating whether a message is protected or not.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * MSG_SYNCML_MESSAGE_S syncMLMsg;
 * MSG_SENDINGOPT_S sendingOpt = {0};
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_update_protected_status(hMsgHandle, 0, true);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_update_protected_status(msg_handle_t handle, msg_message_id_t msg_id, bool is_protected);


/**

 * \par Description:
 * Deletes a message by Message ID from the database.
 *
 * \par Purpose:
 * This API is used to delete a message by Message ID from the database.
 *
 * \par Typical use case:
 * Deletes a previously saved message from the database.
 *
 * \par Method of function operation:
 * Sets up the database connection and deletes a message by Message ID from the message table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input - handle is Message handle.
 * \param input - msg_id is the ID of the Message to be deleted.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * MSG_SYNCML_MESSAGE_S syncMLMsg;
 * MSG_SENDINGOPT_S sendingOpt = {0};
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_delete_message(msgHandle, 0);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_delete_message(msg_handle_t handle, msg_message_id_t msg_id);


/**

 * \par Description:
 * Deletes all messages in the specified folder from the database.
 *
 * \par Purpose:
 * This API is used to delete all messages in the specified folder from the database.
 *
 * \par Typical use case:
 * Deletes all messages in the specified folder from the database.
 *
 * \par Method of function operation:
 * Sets up the database connection and Deletes all messages in the specified folder from the message table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input - handle is Message handle.
 * \param input - folder_id is the ID of the folder to be deleted.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * MSG_SYNCML_MESSAGE_S syncMLMsg;
 * MSG_SENDINGOPT_S sendingOpt = {0};
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_delete_all_msgs_in_folder(msgHandle, MSG_DRAFT_ID);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_delete_all_msgs_in_folder(msg_handle_t handle, msg_folder_id_t folder_id, bool bOnlyDB);



/**

 * \par Description:
 * Moves a message to the specified folder in the database.
 *
 * \par Purpose:
 * This API is used to move a message to the specified folder the database.
 *
 * \par Typical use case:
 * Deletes all messages in the specified folder from the database.
 *
 * \par Method of function operation:
 * Sets up the database connection and Deletes all messages in the specified folder from the message table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input - handle is Message handle.
 * \param input - msg_id is the ID of the message to be moved.
 * \param input - dest_folder_id is the ID of the destination folder.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * MSG_SYNCML_MESSAGE_S syncMLMsg;
 * MSG_SENDINGOPT_S sendingOpt = {0};
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_move_msg_to_folder(hMsgHandle, 0, MSG_OUTBOX_ID);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_move_msg_to_folder(msg_handle_t handle, msg_message_id_t msg_id, msg_folder_id_t dest_folder_id);


/**

 * \par Description:
 * Moves a message to the other storage.
 *
 * \par Purpose:
 * This API is usd to move a message to the other storage.
 *
 * \par Typical use case:
 * Moves a message to the other storage type.
 *
 * \par Method of function operation:
 * Sets up the database connection and  moves a messages to specified storage type.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input - handle is Message handle.
 * \param input - msg_id is the ID of the message to be moved.
 * \param input - storage_id is the ID of the destination storage.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * MSG_SYNCML_MESSAGE_S syncMLMsg;
 * MSG_SENDINGOPT_S sendingOpt = {0};
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_move_msg_to_storage( msgHandle, 0, MSG_STORAGE_PHONE);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_move_msg_to_storage(msg_handle_t handle, msg_message_id_t msg_id, msg_storage_id_t storage_id);


/**

 * \par Description:
 * Gets the number of messages in the specified folder from the database.
 *
 * \par Purpose:
 * This API is used to get the number of messages in the specified folder from the database.
 *
 * \par Typical use case:
 * Gets the number of messages in the specified folder from the database.
 *
 * \par Method of function operation:
 * Sets up the database connection and Gets the number of messages in the specified folder from the message table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input - handle is Message handle.
 * \param input - folder_id is the ID of the folder to be counted.
 * \param output - count_info is a pointer to an MSG_COUNT_INFO_S structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * MSG_COUNT_INFO_S countInfo;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_count_message(msgHandle, MSG_OUTBOX_ID, &countInfo)
* if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_count_message(msg_handle_t handle, msg_folder_id_t folder_id, msg_struct_t count_info);


/**

 * \par Description:
 * Gets the number of messages of specific message type.
 *
 * \par Purpose:
 * This API is used to get the number of messages of specific type.
 *
 * \par Typical use case:
 * Gets the count of message of specific types such as SMS, MMS.
 *
 * \par Method of function operation:
 * Sets up the database connection and queries the number of messages in the specified folder from the message table based on required message type.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input - handle is Message handle.
 * \param input - msg_type is the message type to be counted.
 * \param output - msg_count is a pointer to the number of message.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * MSG_COUNT_INFO_S countInfo;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_count_msg_by_type(msgHandle, MSG_TYPE_SMS, &countInfo);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_count_msg_by_type(msg_handle_t handle, msg_message_type_t msg_type, int *msg_count);


/**

 * \par Description:
 * Gets the number of messages of specific address.
 *
 * \par Purpose:
 * This API is used to get the number of messages from a specific address.
 *
 * \par Typical use case:
 * Get the count of messages from the specified address
 *
 * \par Method of function operation:
 * Sets up the database connection and queries the number of messages based on address from the message table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If addr_info is NULL, nothing happens.
 *
 * \param input - handle is Message handle.
 * \param input - addr_info is a pointer to an MSG_ADDRESS_INFO_LIST_S structure.
 * \param input - msg_thread_count_list is a pointer to an MSG_THREAD_COUNT_INFO_S structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * MSG_COUNT_INFO_S countInfo;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_count_msg_by_type(msgHandle, MSG_TYPE_SMS, &countInfo);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_count_msg_by_contact(msg_handle_t handle, const msg_struct_t addr_info, msg_struct_t msg_thread_count_list);


/**

 * \par Description:
 * Gets the detail information of a message from the database.
 *
 * \par Purpose:
 * This API is used to get the number of messages from a specific address.
 *
 * \par Typical use case:
 * Get the count of messages from the specified address
 *
 * \par Method of function operation:
 * Sets up the database connection and queries the number of messages based on address from the message table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If addr_info is NULL, nothing happens.
 * - Applications need to call msg_release_message to free the memory.
 * - However, if this function is failed, the memory for the message is NOT allocated in this function.
 *
handle is Message handle.
 * \param input - handle is Message handle.
 * \param input - msg_id is the ID of the Message to be returned.
 * \param output - msg is a pointer to an msg_message_t structure.
 * \param input - send_opt is a pointer to an MSG_SENDINGOPT_S structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
 *
 * \par Prospective clients:
 * External/Native Apps using Messaging Services.
 *
 * \par Related functions:
 * - None
 *
 * \par Known issues/bugs:
 * None
 *
 * \par Sample code:
 * \code
 * ...
 *
 * msg_handle_t msgHandle = NULL;
 *
 * ...
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * ...
 * err = msg_get_message(msgHandle, 0, msg, &sendOpt);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_message(msg_handle_t handle, msg_message_id_t msg_id, msg_struct_t msg, msg_struct_t send_opt);


/**

 * \par Description:
 * Returns the common information list of messages with selected folder id.
 *
 * \par Purpose:
 * This API is used to get the common information list of messages with selected folder id from database.
 *
 * \par Typical use case:
 * Get the common information from the specified folder from database.
 *
 * \par Method of function operation:
 * Sets up the database connection and queries the common information based on selected folder id from the message and folder tables.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - The memory for a message will be allocated in this function.
 * - Applications need to call msg_release_folder_view_list to free the memory.
 * - However, if this function is failed, the memory for the message is NOT allocated in this function.
 *
 * \param input - handle is Message handle.
 * \param input - folder_id is the ID of the folder to be returned.
 * \param input - sort_rule indicates a sort type and sort order for querying messages.
 * \param output - msg_folder_view_list is a pointer to an MSG_FOLDER_VIEW_LIST_S structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
 *
 * \par Prospective clients:
 * External/Native Apps using Messaging Services.
 *
 * \par Related functions:
 * - None
 *
 * \par Known issues/bugs:
 * None
 *
 * \par Sample code:
 * \code
 * ...
 *
 * msg_handle_t msgHandle = NULL;
 *
 * ...
 * MSG_FOLDER_VIEW_LIST_S folderViewList;
 * ...
 * err = msg_get_folder_view_list(hMsgHandle, 0, NULL, &folderViewList);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_folder_view_list(msg_handle_t handle, msg_folder_id_t folder_id, const msg_struct_t sort_rule, msg_struct_list_s *msg_folder_view_list);


/**

 * \par Description:
 * Returns the information of all peers to whom messages have been sent or recieved.
 *
 * \par Purpose:
 * This API is used to get the information of all peers to whom messages have been sent or recieved.
 *
 * \par Typical use case:
 * Get the common information from the specified folder from database.
 *
 * \par Method of function operation:
 * Frees the memory occupied by MSG_FOLDER_VIEW_LIST_S object and its members.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - The memory for a list will be allocated in this function.
 * - Applications need to call msg_release_thread_view_list to free the memory.
 * - However, if this function is failed, the memory for a list is NOT allocated in this function.
 *
 * \param input - handle is Message handle.
 * \param input - sort_rule indicates a sort type and sort order for querying messages.
 * \param output - msg_thread_view_list is a pointer to an msg_struct_list_s structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 *
 * ...
 * msg_struct_list_s threadViewList;
 * ...
 * err = msg_get_thread_view_list(hMsgHandle, NULL, &threadViewList);
 * ...
 * msg_release_thread_view_list(&threadViewList);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * \endcode
 */
/*================================================================================================*/
int msg_get_thread_view_list(msg_handle_t handle, const msg_struct_t sort_rule, msg_struct_list_s *msg_thread_view_list);


/**

 * \par Description:
 * Returns the common information list of messages with selected thread_id.
 *
 * \par Purpose:
 * This API is used to get the common information list of messages with selected thread_id.
 *
 * \par Typical use case:
 * Gets the common information list of messages with the selected thread id from the database.
 *
 * \par Method of function operation:
 * Connects to the database and queries the common infomation of  list messages with the provided thread id.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - The memory for a list will be allocated in this function.
 * - Applications need to call msg_release_conversation_view_list to free the memory.
 * - However, if this function is failed, the memory for a list is NOT allocated in this function.
 *
 * \param input - hMsgHandle is Message handle.
 * \param input - thread_id is the ID of the thread to be returned.
 * \param output - msg_conv_view_list is a pointer to an MSG_CONV_VIEW_LIST_S structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
 *
 * \par Prospective clients:
 * External/Native Apps using Messaging Services.
 *
 * \par Related functions:
 * - None
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
 * ...
 * MSG_CONV_VIEW_LIST_S convViewList;
 * ...
 * err = msg_get_conversation_view_list(hMsgHandle, ThreadId, &convViewList);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * msg_release_conversation_view_list(&convViewList);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_conversation_view_list(msg_handle_t handle, msg_thread_id_t thread_id, msg_struct_list_s *msg_conv_view_list);


/**

 * \par Description:
 * Deletes all the Messages Sent/Received from the selected list.
 *
 * \par Purpose:
 * This API is used to delete all the Messages Sent/Received from the selected list.
 *
 * \par Typical use case:
 * Deletes all messages sent/received from the selected list.
 *
 * \par Method of function operation:
 * Sets up the database connection and deletes all messages sent/received from a selected list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If addr_info is NULL, nothing happens.
 *
 * \param input - handle is Message handle.
 * \param input - thread_id is the ID of the thread to be deleted.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * MSG_SYNCML_MESSAGE_S syncMLMsg;
 * MSG_SENDINGOPT_S sendingOpt = {0};
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_delete_thread_message_list(hMsgHandle, 0);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_delete_thread_message_list(msg_handle_t handle, msg_thread_id_t thread_id);


/**

 * \par Description:
 * Adds a new folder.
 *
 * \par Purpose:
 * This API is used to add a new folder.
 *
 * \par Typical use case:
 * Adds a new folder with the specified folder info
 *
 * \par Method of function operation:
 * Sets up the database connection and add a new folder to the folder table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None.
 *
 * \param - handle is Message handle.
 * \param - folder_info is a pointer to an MSG_FOLDER_INFO_S structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 *  Make Folder
 * MSG_FOLDER_INFO_S folderInfo;
 * ...
 * err = msg_open_msg_handle(&msgHandle);
 *
 * folderInfo.folderId = 1;
 * folderInfo.folderType = MSG_FOLDER_TYPE_USER_DEF;
 * ...
 *
 * err = msg_add_folder(hMsgHandle, &folderInfo);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_add_folder(msg_handle_t handle, const msg_struct_t folder_info);


/**

 * \par Description:
 * Updates the folder info.
 *
 * \par Purpose:
 * This API is used to add a new folder.
 *
 * \par Typical use case:
 * Adds a new folder with the specified folder info
 *
 * \par Method of function operation:
 * Sets up the database connection and add a new folder to the folder table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None.
 *
 * \param - handle is Message handle.
 * \param - folder_info is a pointer to an MSG_FOLDER_INFO_S structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 *  Make Folder
 * MSG_FOLDER_INFO_S folderInfo;
 * ...
 * err = msg_open_msg_handle(&msgHandle);
 *
 * folderInfo.folderId = 2;
 * folderInfo.folderType = MSG_FOLDER_TYPE_USER_DEF;
 * ...
 * err = msg_update_folder(msgHandle, &folderInfo);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_update_folder(msg_handle_t handle, const msg_struct_t folder_info);


/**

 * \par Description:
 * Deletes an exisiting folder.
 *
 * \par Purpose:
 * This API is used to delete an existing folder.
 *
 * \par Typical use case:
 * Deletes an existing folder.
 *
 * \par Method of function operation:
 * Sets up the database connection and deletes an existing folder to the folder table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None.
 *
 * \param input - handle is Message handle.
 * \param input - folder_id is the ID of the folder to be deleted.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 * err = msg_delete_folder(hMsgHandle, MSG_INBOX_ID);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_delete_folder(msg_handle_t handle, msg_folder_id_t folder_id);


/**

 * \par Description:
 * Returns the information list of folders.
 *
 * \par Purpose:
 * This API is used to get the information list of folders.
 *
 * \par Typical use case:
 * Gets the folder list information.
 *
 * \par Method of function operation:
 * Sets up the database connection and queries for the folder list information.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None.
 *
 * \param input - handle is Message handle.
 * \param output - folder_list is a pointer to an msg_struct_list_s structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 *  msg_struct_list_s folderList;

 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 * err = msg_get_folder_list(msgHandle, &folderList);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * ...
 * msg_release_folder_list(&folderList);
 * \endcode
 */
/*================================================================================================*/
int msg_get_folder_list(msg_handle_t handle, msg_struct_list_s *folder_list);


/**

 * \par Description:
 * Creates the specified number of messages in database.
 *
 * \par Purpose:
 * This API is used to generate specified number of messages in the database
 *
 * \par Typical use case:
 * Generate large number of messages in the database.
 *
 * \par Method of function operation:
 * Creates the specified number of messages in database for specified message type in the specified folder
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input - handle is Message handle.
 * \param input - msg_type is one of enum _MSG_MESSAGE_TYPE_E.
 * \param input - folder_id is the folder for the test messages.
 * \param input - num_msg is the number of messages.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_MSGHANDLE	Parameter is invalid.
 * - MSG_ERR_INVALID_FOLDER_ID	Storage is error.
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
 *  msg_struct_list_s folderList;

 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 * err = msg_generate_message(msgHandle, MSG_TYPE_SMS, MSG_INBOX_ID, 100);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_generate_message(msg_handle_t handle, msg_message_type_t msg_type, msg_folder_id_t folder_id, unsigned int num_msg);
int msg_generate_sms(msg_handle_t handle, msg_folder_id_t folder_id, unsigned int num_msg) DEPRECATED;


/**

 * \par Description:
 * Returns the Message Data to be used by the Quick Panel.
 *
 * \par Purpose:
 * This API is used to get the Message Datato be used by the Quick Panel.
 *
 * \par Typical use case:
 * Quick panel needs the message information to show new message notification.
 *
 * \par Method of function operation:
 * Connects to database and queries for information needed by the quick panel.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input - handle is Message handle.
 * \param input - type is the type of message that Quick Panel need.
 * \param output - msg is a pointer to an msg_message_t structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_MSGHANDLE	Parameter is invalid.
 * - MSG_ERR_INVALID_FOLDER_ID	Storage is error.
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
 * msg_message_t msgInfo;

 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 * err = msg_get_quick_panel_data(msgHandle, MSG_QUICKPANEL_SMS, msgInfo);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_quick_panel_data(msg_handle_t handle, msg_quickpanel_type_t type, msg_struct_t msg);


/**

 * \par Description:
 * Resets the Messaging database.
 *
 * \par Purpose:
 * This API is used to reset the messaging database.
 *
 * \par Typical use case:
 * Completely delete the messaging database.
 *
 * \par Method of function operation:
 * Connects to database and deletes all the messaging tables.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input - handle is Message handle.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_PARAMETER	Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR		Storage is error.
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
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 * err = msg_reset_database(msgHandle);
 * if (err != MSG_SUCCESS)
 *{
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_reset_database(msg_handle_t handle);


/**

 * \par Description:
 * Returns the total size used for message contents.
 *
 * \par Purpose:
 * This API is used to get the total size used for message contents.
 *
 * \par Typical use case:
 * To get the total space used by message contents.
 *
 * \par Method of function operation:
 * Uses linux system calls to query the space used by message contents.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input - handle is Message handle.
 * \param output -	memsize is a pointer to the size.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_PARAMETER	Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR		Storage is error.
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
 * int memsize = 0;
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 * err = msg_get_mem_size(msgHandle, &memsize);
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_mem_size(msg_handle_t handle, unsigned int* memsize);

/**

 * \par Description:
 * Backup messages to storage.
 *
 * \par Purpose:
 * This API is used to backup messages to storage.
 *
 * \par Typical use case:
 * Backup messages to storage.
 *
 * \par Method of function operation:
 * Reads all the messages from Messaging database and writes to storage in V-Message format
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * None
 *
 * \param input - msg_handle_t  handle is Message handle.
 *
 * \return Return Type int (msg_error_t) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_PARAMETER	Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR		Storage is error.
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
 * msg_error_t err;
 *
 * ...
 * err = msg_backup_message(&msgHandle);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_backup_message(msg_handle_t handle);


/**

 * \par Description:
 * Restore messages from backed up messages.
 *
 * \par Purpose:
 * This API is used to restore messages from backed up messages.
 *
 * \par Typical use case:
 * Restore messages from previously backed up messages.
 *
 * \par Method of function operation:
 * Reads the previously backup up messages and restores the database.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * None
 *
 * \param input - msg_handle_t  handle is Message handle.
 *
 * \return Return Type int (msg_error_t) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_PARAMETER	Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR		Storage is error.
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
 * msg_error_t err;
  * ...
 * err = msg_restore_message(&msgHandle);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_restore_message(msg_handle_t handle);


/**

 * \par Description:
 * Search messages or addresses which including a string that applcation want to find.
 *
 * \par Purpose:
 * This API is used to search messages or addresses from storage.
 *
 * \par Typical use case:
 * Search messages or addresses from storage.
 *
 * \par Method of function operation:
 * search messages or addresses from storage.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * None
 *
 * \param input - msg_handle_t  handle is Message handle.
 * \param input - search_string is the string to search.
 * \param output - msg_thread_view_list is a pointer to an msg_struct_list_s structure.
 *
 * \return Return Type int (msg_error_t) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_PARAMETER	Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR		Storage is error.
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
 * ...
 * char* search_string = "hello";
 * msg_struct_list_s threadViewList;
 * ...
 * err = msg_search_message_for_thread_view(&msgHandle, search_string, &threadViewList);
 *
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_search_message_for_thread_view(msg_handle_t handle, const char *search_string, msg_struct_list_s *msg_thread_view_list);


/**

 * \par Description:
 * Search messages or addresses which including a string that applcation want to find.
 *
 * \par Purpose:
 * This API is used to search messages or addresses from storage.
 *
 * \par Typical use case:
 * Search messages or addresses from storage.
 *
 * \par Method of function operation:
 * search messages or addresses from storage.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * None
 *
 * \param input - msg_handle_t  handle is Message handle.
 * \param input - search_string is the string to search.
 * \param input - offset is the offset of the search result.
 * \param input - limit is the limit of the search result.
 * \param output - msg_list is a pointer to an msg_struct_list_s structure.
 *
 * \return Return Type int (msg_error_t) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_PARAMETER	Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR		Storage is error.
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
 * ...
 * msg_struct_list_s msg_list;
 * int offset = 0;
 * int limit = 10;
 *
 * MSG_SEARCH_CONDITION_S searchCon;
 *
 * searchCon.msgType = MSG_TYPE_SMS;
 * searchCon.folderId = MSG_INBOX_ID;
 * searchCon.pSearchVal = "keyString";
 * searchCon.pAddressVal = "01000000000";
 *
 * ...
 * err = msg_search_message(hMsgHandle, &searchCon, offset, limit, &msgList);
 *
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_search_message(msg_handle_t handle, const msg_struct_t msg_search_conditions, int offset, int limit, msg_struct_list_s *msg_list);

/**

 * \par Description:
 * Get reject message list that application wants to find by phone number.
 *
 * \par Purpose:
 * This API is used to get reject message list from storage.
 *
 * \par Typical use case:
 * Get reject message list from storage.
 *
 * \par Method of function operation:
 * Get reject message list from storage.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * None
 *
 * \param input - msg_handle_t  handle is Message handle.
 * \param input - phone_num is the string of phone number to find.
 * \param output - msg_reject_msg_list is a pointer to an msg_struct_list_s structure.

 * \return Return Type int (msg_error_t) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_INVALID_PARAMETER	Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR		Storage is error.
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
 * ...
 * char* phone_num = "01030016057";
 * msg_struct_list_s rejectMsgList;
 * ...
 * err = msg_get_reject_msg_list(hMsgHandle, phone_num, &rejectMsgList);
 *
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_reject_msg_list(msg_handle_t handle, const char* phone_num, msg_struct_list_s *msg_reject_msg_list);


/**

 * \par Description:
 * Registers a callback function about the change of storage status to Message handle.
 *
 * \par Purpose:
 * This API is used to register a callback function about the change of storage status "msg_storage_change_cb" to Message handle.
 *
 * \par Typical use case:
 * Register a callback function about the change of storage status.
 *
 * \par Method of function operation:
 * Adds the msg_storage_change_cb API to a callback function list.
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
 * \return Return Type (int(msg_error_t)) \n
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
 * err = msg_reg_storage_change_callback(msgHandle, &storageCB, NULL);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 * return;
 * }
 *
 * void storageCB(msg_handle_t handle, msg_thread_id_t threadId, msg_message_id_t msgId, void *user_param)
 * {
 * 	...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_reg_storage_change_callback(msg_handle_t handle, msg_storage_change_cb cb, void *user_param);


/**

 * \par Description:
 * Gets the report status information of message.
 *
 * \par Purpose:
 * This API is used to get the report status information of specified message.
 *
 * \par Typical use case:
 * Gets the report status information of specified message from the database.
 *
 * \par Method of function operation:
 * Sets up the database connection and Gets the report status information of specified message from the report table.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param input - handle is Message handle.
 * \param input - msg_id is the ID of the message.
 * \param output - report_status is a pointer to a MSG_REPORT_STATUS_INFO_S structure.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS	- Success in operation.
 * - MSG_ERR_INVALID_PARAMETER - Parameter is invalid.
 * - MSG_ERR_STORAGE_ERROR - Storage is error.
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
 * MSG_REPORT_STATUS_INFO_S reportStatus;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_get_report_status(msgHandle, msgID, &reportStatus)
* if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_report_status(msg_handle_t handle, msg_message_id_t msg_id, msg_struct_t report_status);




int msg_get_address_list(msg_handle_t handle, msg_thread_id_t thread_id, msg_struct_list_s *msg_address_list);


int msg_get_thread_id_by_address(msg_handle_t handle, msg_struct_list_s *msg_address_list, msg_thread_id_t *thread_id);


int msg_get_thread(msg_handle_t handle, msg_thread_id_t thread_id, msg_struct_t msg_thread);


int msg_get_message_list(msg_handle_t handle, msg_folder_id_t folder_id, msg_thread_id_t thread_id, msg_message_type_t msg_type, msg_storage_id_t storage_id, msg_struct_list_s *msg_list);

int msg_add_push_event(msg_handle_t handle, const msg_struct_t push_event);

int msg_delete_push_event(msg_handle_t handle, const msg_struct_t push_event);

int msg_update_push_event(msg_handle_t handle, const msg_struct_t src_event, const msg_struct_t dst_event);
/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif // MAPI_STORAGE_H

