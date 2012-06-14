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

#include "MsgStorageTypes.h"

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
 * \param input - MSG_HANDLE_T  handle is Message handle.
 * \param input - msg_message_t  msg is a pointer to an msg_message_t structure.
 * \param input - send_opt is a pointer to an MSG_SENDINGOPT_S structure.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_add_message(MSG_HANDLE_T handle, const msg_message_t msg, const MSG_SENDINGOPT_S *send_opt);


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
 * \param input - MSG_HANDLE_T  handle is Message handle.
 * \param input - MSG_SYNCML_MESSAGE_S  syncml_msg is a pointer to an MSG_SYNCML_MESSAGE_S structure.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_add_syncml_message(MSG_HANDLE_T handle, const MSG_SYNCML_MESSAGE_S *syncml_msg);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_update_message(MSG_HANDLE_T handle, const msg_message_t msg, const MSG_SENDINGOPT_S *send_opt);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_update_read_status(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id, bool read);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_update_protected_status(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id, bool is_protected);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_delete_message(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_delete_all_msgs_in_folder(MSG_HANDLE_T handle, MSG_FOLDER_ID_T folder_id, bool bOnlyDB);



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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_move_msg_to_folder(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id, MSG_FOLDER_ID_T dest_folder_id);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_move_msg_to_storage(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id, MSG_STORAGE_ID_T storage_id);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_count_message(MSG_HANDLE_T handle, MSG_FOLDER_ID_T folder_id, MSG_COUNT_INFO_S *count_info);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_count_msg_by_type(MSG_HANDLE_T handle, MSG_MESSAGE_TYPE_T msg_type, int *msg_count);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_count_msg_by_contact(MSG_HANDLE_T handle, const MSG_THREAD_LIST_INDEX_S *addr_info, MSG_THREAD_COUNT_INFO_S *msg_thread_count_list);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_get_message(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id, msg_message_t msg, MSG_SENDINGOPT_S *send_opt);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_get_folder_view_list(MSG_HANDLE_T handle, MSG_FOLDER_ID_T folder_id, const MSG_SORT_RULE_S *sort_rule, MSG_LIST_S *msg_folder_view_list);


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
 * \param output - msg_thread_view_list is a pointer to an MSG_THREAD_VIEW_LIST_S structure.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
 *
 * ...
 * MSG_THREAD_VIEW_LIST_S threadViewList;
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
int msg_get_thread_view_list(MSG_HANDLE_T handle, const MSG_SORT_RULE_S *sort_rule, MSG_THREAD_VIEW_LIST_S *msg_thread_view_list);


/**

 * \par Description:
 * Frees the memory of MSG_PEER_INFO_LIST_S allocated in msg_get_thread_view_list.
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
 * \param input - msg_thread_view_list is a pointer to an MSG_THREAD_VIEW_LIST_S structure.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
 *
 * ...
 * MSG_THREAD_VIEW_LIST_S threadViewList;
 * ...
 * err = msg_get_thread_view_list(hMsgHandle, NULL, &threadViewList);
 * ...
 * msg_release_thread_view_list(&threadViewList);
 * ...
 * \endcode
 */
/*================================================================================================*/
void msg_release_thread_view_list(MSG_THREAD_VIEW_LIST_S *msg_thread_view_list);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
 * MSG_ERROR_T err = MSG_SUCCESS;
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
int msg_get_conversation_view_list(MSG_HANDLE_T handle, MSG_THREAD_ID_T thread_id, MSG_LIST_S *msg_conv_view_list);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_delete_thread_message_list(MSG_HANDLE_T handle, MSG_THREAD_ID_T thread_id);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_add_folder(MSG_HANDLE_T handle, const MSG_FOLDER_INFO_S *folder_info);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_update_folder(MSG_HANDLE_T handle, const MSG_FOLDER_INFO_S *folder_info);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_delete_folder(MSG_HANDLE_T handle, MSG_FOLDER_ID_T folder_id);


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
 * \param output - folder_list is a pointer to an MSG_FOLDER_LIST_S structure.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
 *  MSG_FOLDER_LIST_S folderList;

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
int msg_get_folder_list(MSG_HANDLE_T handle, MSG_FOLDER_LIST_S *folder_list);



/**

 * \par Description:
 * Releases the memory of a folder list.
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
 * - If folder_list is NULL, nothing happens.
 * - If folder_list is invalid, undefined behavior happens.
 *
 * \param input - folder_list is a pointer to an MSG_FOLDER_LIST_S structure.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
 *  MSG_FOLDER_LIST_S folderList;

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
void msg_release_folder_list(MSG_FOLDER_LIST_S *folder_list);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
 *  MSG_FOLDER_LIST_S folderList;

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
int msg_generate_message(MSG_HANDLE_T handle, MSG_MESSAGE_TYPE_T msg_type, MSG_FOLDER_ID_T folder_id, unsigned int num_msg);
int msg_generate_sms(MSG_HANDLE_T handle, MSG_FOLDER_ID_T folder_id, unsigned int num_msg) DEPRECATED;


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_get_quick_panel_data(MSG_HANDLE_T handle, MSG_QUICKPANEL_TYPE_T type, msg_message_t msg);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
 * MSG_ERROR_T err = MSG_SUCCESS;
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
int msg_reset_database(MSG_HANDLE_T handle);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_get_mem_size(MSG_HANDLE_T handle, unsigned int* memsize);


/**

 * \par Description:
 * Return thread id in thread view object.
 *
 * \par Purpose:
 * This API is used to get the thread id field in folder view object.
 *
 * \par Typical use case:
 * Returns thread id value in thread view object.
 *
 * \par Method of function operation:
 * Returns the threadId member of thread view object msg.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg_thread is NULL, nothing happens.
 * - If msg_thread is invalid, undefined behavior happens.
 *
 * \param msg_thread_view_t input - msg_thread is a thread view object.
 *
 * \return Return Type int (MSG_ERROR_T when negative, msg_id when positive) \n
 * - positive int 						thread id.
 * - MSG_ERR_NULL_POINTER				msg_thread is NULL.
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
 * MSG_HANDLE_T msgHandle = NULL;
 * msg_thread_view_t	msgThreadInfo;
 * MSG_ERROR_T err;
 *
 * ...
 * err = msg_thread_view_get_thread_id(&msgThreadInfo);
 * if(msg_size > 0)
 * {
 * ...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_thread_view_get_thread_id(msg_thread_view_t msg_thread);


/**

 * \par Description:
 * Return address value in thread view object.
 *
 * \par Purpose:
 * This API is used to get the address value in thread view object.
 *
 * \par Typical use case:
 * Returns address value in thread view object.
 *
 * \par Method of function operation:
 * Returns the threadAddr member of thread view object msg.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg_thread is NULL, nothing happens.
 * - If msg_thread is invalid, undefined behavior happens.
 * - You do not need to free the return value. It will be freed when you call msg_release_thread_view_list().
 * - Also, the value is valid until msg_thread_view_t is freed by calling msg_release_thread_view_list().
 *
 * \param msg_thread_view_t input - msg_thread is a thread view object.
 *
 * \return Return Type int const char* (address value) \n
 * - const char* 					address value.
 * - NULL							msg_thread is NULL.
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
 * MSG_HANDLE_T msgHandle = NULL;
 * msg_thread_view_t	msgThreadInfo;
 * char* addr = NULL;
 *
 * ...
 * addr = msg_thread_view_get_address(&msgThreadInfo);
 * if(addr != 0 && strlen(addt) > 0)
 * {
 * ...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
const char* msg_thread_view_get_address(msg_thread_view_t msg_thread);


/**

 * \par Description:
 * Return display name value in thread view object.
 *
 * \par Purpose:
 * This API is used to get the display name value in thread view object.
 *
 * \par Typical use case:
 * Returns address value in thread view object.
 *
 * \par Method of function operation:
 * Returns the threadName member of thread view object msg.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg_thread is NULL, nothing happens.
 * - If msg_thread is invalid, undefined behavior happens.
 * - You do not need to free the return value. It will be freed when you call msg_release_thread_view_list().
 * - Also, the value is valid until msg_thread_view_t is freed by calling msg_release_thread_view_list().
 *
 * \param msg_thread_view_t input - msg_thread is a thread view object.
 *
 * \return Return Type int const char* (display name) \n
 * - const char* 					display name.
 * - NULL							msg_thread is NULL.
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
 * MSG_HANDLE_T msgHandle = NULL;
 * msg_thread_view_t	msgThreadInfo;
 * char* disp_name = NULL;
 *
 * ...
 * disp_name = msg_thread_view_get_name(&msgThreadInfo);
 * if(disp_name != 0 && strlen(disp_name) > 0)
 * {
 * ...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
const char* msg_thread_view_get_name(msg_thread_view_t msg_thread);


/**

 * \par Description:
 * Return image path in thread view object.
 *
 * \par Purpose:
 * This API is used to get the image path value in thread view object.
 *
 * \par Typical use case:
 * Returns image path value in thread view object.
 *
 * \par Method of function operation:
 * Returns the threadImagePath member of thread view object msg.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg_thread is NULL, nothing happens.
 * - If msg_thread is invalid, undefined behavior happens.
 * - You do not need to free the return value. It will be freed when you call msg_release_thread_view_list().
 * - Also, the value is valid until msg_thread_view_t is freed by calling msg_release_thread_view_list().
 *
 * \param msg_thread_view_t input - msg_thread is a thread view object.
 *
 * \return Return Type int const char* (thread image path) \n
 * - const char* 					thread image path.
 * - NULL							msg_thread is NULL.
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
 * MSG_HANDLE_T msgHandle = NULL;
 * msg_thread_view_t	msgThreadInfo;
 * char* img_path = NULL;
 *
 * ...
 * img_path = msg_thread_view_get_name(&msgThreadInfo);
 * if(img_path != 0 && strlen(img_path) > 0)
 * {
 * ...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
const char* msg_thread_view_get_image_path(msg_thread_view_t msg_thread);


/**

 * \par Description:
 * Return the message type field in thread view object.
 *
 * \par Purpose:
 * This API is used to get the message type field value in thread view object.
 *
 * \par Typical use case:
 * Returns the message type value in thread view object.
 *
 * \par Method of function operation:
 * Returns the msgType member of thread view object msg.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg_thread is NULL, nothing happens.
 * - If msg_thread is invalid, undefined behavior happens.
 * - You do not need to free the return value. It will be freed when you call msg_release_thread_view_list().
 * - Also, the value is valid until msg_thread_view_t is freed by calling msg_release_thread_view_list().
 *
 * \param msg_thread_view_t input - msg_thread is a thread view object.
 *
 * \return Return Type (int (MSG_ERROR_T when negative, msg_type when positive)) \n
 * - positive int 					enum _MSG_MESSAGE_TYPE_E.
 * - EINVAL						msg_thread is NULL.
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
 * MSG_HANDLE_T msgHandle = NULL;
 * msg_thread_view_t	msgThreadInfo;
 * MSG_MESSAGE_TYPE_T msg_type;
 *
 * ...
 * msg_type = msg_thread_view_get_message_type(&msgThreadInfo);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_thread_view_get_message_type(msg_thread_view_t msg_thread);


/**

 * \par Description:
 * Return data field in thread view object.
 *
 * \par Purpose:
 * This API is used to get the data field value in thread view object.
 *
 * \par Typical use case:
 * Returns data value in thread view object.
 *
 * \par Method of function operation:
 * Returns the threadData member of thread view object msg.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg_thread is NULL, nothing happens.
 * - If msg_thread is invalid, undefined behavior happens.
 * - You do not need to free the return value. It will be freed when you call msg_release_thread_view_list().
 * - Also, the value is valid until msg_thread_view_t is freed by calling msg_release_thread_view_list().
 *
 * \param msg_thread_view_t input - msg_thread is a thread view object.
 *
 * \return Return Type int const char* (message data) \n
 * - const char* 					message data.
 * - NULL							msg_thread is NULL.
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
 * MSG_HANDLE_T msgHandle = NULL;
 * msg_thread_view_t	msgThreadInfo;
 * char* msg_data = NULL;
 *
 * ...
 * msg_data = msg_thread_view_get_data(&msgThreadInfo);
 * if(msg_data != 0 && strlen(msg_data) > 0)
 * {
 * ...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
const char* msg_thread_view_get_data(msg_thread_view_t msg_thread);


/**

 * \par Description:
 * Return time field in thread view object.
 *
 * \par Purpose:
 * This API is used to get the time field value in thread view object.
 *
 * \par Typical use case:
 * Returns time value in thread view object.
 *
 * \par Method of function operation:
 * Returns the threadTime member of thread view object msg.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg_thread is NULL, nothing happens.
 * - If msg_thread is invalid, undefined behavior happens.
 * - You do not need to free the return value. It will be freed when you call msg_release_thread_view_list().
 * - Also, the value is valid until msg_thread_view_t is freed by calling msg_release_thread_view_list().
 *
 * \param msg_thread_view_t input - msg_thread is a thread view object.
 *
 * \return Return Type (time_t) \n
 * - time_t 						message time.
 * - NULL							thread view object is NULL.
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
 * MSG_HANDLE_T msgHandle = NULL;
 * msg_thread_view_t	msgThreadInfo;
 * time_t msg_time;
 *
 * ...
 * msg_time = msg_thread_view_get_time(&msgThreadInfo);
 * ...
 * \endcode
 */
/*================================================================================================*/
time_t* msg_thread_view_get_time(msg_thread_view_t msg_thread);


/**

 * \par Description:
 * Return direction field in thread view object.
 *
 * \par Purpose:
 * This API is used to get the direction field value in thread view object.
 *
 * \par Typical use case:
 * Returns direction value in thread view object.
 *
 * \par Method of function operation:
 * Returns the direction member of thread view object msg.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg_thread is NULL, nothing happens.
 * - If msg_thread is invalid, undefined behavior happens.
 *
 * \param msg_thread_view_t input - msg_thread is a thread view object.
 *
 * \return Return Type (time_t) \n
 * - positive int 					enum _MSG_DIRECTION_TYPE_E.
 * - MSG_ERR_NULL_POINTER				msg_thread is NULL.
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
 * MSG_HANDLE_T msgHandle = NULL;
 * msg_thread_view_t	msgThreadInfo;
 * int direction;
 *
 * ...
 * direction = msg_thread_view_get_direction(&msgThreadInfo);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_thread_view_get_direction(msg_thread_view_t msg_thread);


/**

 * \par Description:
 * Return contact id field in thread view object.
 *
 * \par Purpose:
 * This API is used to get the contact id field value in thread view object.
 *
 * \par Typical use case:
 * Returns contact id value in thread view object.
 *
 * \par Method of function operation:
 * Returns the contactId member of thread view object msg.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg_thread is NULL, nothing happens.
 * - If msg_thread is invalid, undefined behavior happens.
 *
 * \param msg_thread_view_t input - msg_thread is a thread view object.
 *
 * \return Return Type (int (MSG_ERROR_T when negative, contact id when positive)) \n
 * - positive int 					contact id
 * - MSG_ERR_NULL_POINTER			msg_thread is NULL.
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
 * MSG_HANDLE_T msgHandle = NULL;
 * msg_thread_view_t	msgThreadInfo;
 * int contact_id;
 *
 * ...
 * contact_id = msg_thread_view_get_contact_id(&msgThreadInfo);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_thread_view_get_contact_id(msg_thread_view_t msg_thread);


/**

 * \par Description:
 * Return unread message count field in thread view object.
 *
 * \par Purpose:
 * This API is used to get the unread message count field value in thread view object.
 *
 * \par Typical use case:
 * Returns unread message count value in thread view object.
 *
 * \par Method of function operation:
 * Returns the unreadCnt member of thread view object msg.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg_thread is NULL, nothing happens.
 * - If msg_thread is invalid, undefined behavior happens.
 *
 * \param msg_thread_view_t input - msg_thread is a thread view object.
 *
 * \return Return Type nt (MSG_ERROR_T when negative, unread message count  when positive) \n
 * - positive int 					unread message count \n
 * - MSG_ERR_NULL_POINTER			msg_thread is NULL.
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
 * MSG_HANDLE_T msgHandle = NULL;
 * msg_thread_view_t	msgThreadInfo;
 * int unread_count;
 *
 * ...
 * unread_count= msg_thread_view_get_unread_cnt(&msgThreadInfo);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_thread_view_get_unread_cnt(msg_thread_view_t msg_thread);



/**

 * \par Description:
 * Return sms message count field in thread view object.
 *
 * \par Purpose:
 * This API is used to get the sms message count field value in thread view object.
 *
 * \par Typical use case:
 * Returns sms message count value in thread view object.
 *
 * \par Method of function operation:
 * Returns the smsCnt member of thread view object msg.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg_thread is NULL, nothing happens.
 * - If msg_thread is invalid, undefined behavior happens.
 *
 * \param msg_thread_view_t input - msg_thread is a thread view object.
 *
 * \return Return Type int (MSG_ERROR_T when negative, sms message count when positive) \n
 * - positive int 					sms message count
 * - MSG_ERR_NULL_POINTER			msg_thread is NULL.
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
 * MSG_HANDLE_T msgHandle = NULL;
 * msg_thread_view_t	msgThreadInfo;
 * int sms_count;
 *
 * ...
 * sms_count= msg_thread_view_get_sms_cnt(&msgThreadInfo);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_thread_view_get_sms_cnt(msg_thread_view_t msg_thread);


/**

 * \par Description:
 * Return mms message count field in thread view object.
 *
 * \par Purpose:
 * This API is used to get the mms message count field value in thread view object.
 *
 * \par Typical use case:
 * Returns mms message count value in thread view object.
 *
 * \par Method of function operation:
 * Returns the mmsCnt member of thread view object msg.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg_thread is NULL, nothing happens.
 * - If msg_thread is invalid, undefined behavior happens.
 *
 * \param msg_thread_view_t input - msg_thread is a thread view object.
 *
 * \return Return Type int (MSG_ERROR_T when negative, mms message count when positive) \n
 * - positive int 					sms message count
 * - MSG_ERR_NULL_POINTER			msg_thread is NULL.
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
 * MSG_HANDLE_T msgHandle = NULL;
 * msg_thread_view_t	msgThreadInfo;
 * int mms_count;
 *
 * ...
 * mms_count= msg_thread_view_get_mms_cnt(&msgThreadInfo);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_thread_view_get_mms_cnt(msg_thread_view_t msg_thread);


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
 * \param input - MSG_HANDLE_T  handle is Message handle.
 * \param input - search_string is the string to search.
 * \param output - msg_thread_view_list is a pointer to an MSG_THREAD_VIEW_LIST_S structure.
 *
 * \return Return Type int (MSG_ERROR_T) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
 * MSG_ERROR_T err = MSG_SUCCESS;
 * ...
 * char* search_string = "hello";
 * MSG_THREAD_VIEW_LIST_S threadViewList;
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
int msg_search_message_for_thread_view(MSG_HANDLE_T handle, const char *search_string, MSG_THREAD_VIEW_LIST_S *msg_thread_view_list);


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
 * \param input - MSG_HANDLE_T  handle is Message handle.
 * \param input - search_string is the string to search.
 * \param input - offset is the offset of the search result.
 * \param input - limit is the limit of the search result.
 * \param output - msg_list is a pointer to an MSG_LIST_S structure.
 *
 * \return Return Type int (MSG_ERROR_T) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
 * MSG_ERROR_T err = MSG_SUCCESS;
 * ...
 * MSG_LIST_S msg_list;
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
int msg_search_message(MSG_HANDLE_T handle, const MSG_SEARCH_CONDITION_S *msg_search_conditions, int offset, int limit, MSG_LIST_S *msg_list);


/**

 * \par Description:
 * Free the memory of msg_message_t, which is created by msg_new_message().
 *
 * \par Purpose:
 * This API is used to release memory created by message creation.
 *
 * \par Typical use case:
 * After using message object for send/save scenario, release message need to be called.
 *
 * \par Method of function operation:
 * Frees the memory allocated to message object and deletes the object.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param msg_message_t    input - message object to be destroyed .
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS	- Successfully connected to Messaging Service \n
 * - MSG_ERR_NULL_POINTER	-	Input parameter is NULL.
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
 * MSG_HANDLE_T msgHandle = NULL;
 * MSG_ERROR_T err = MSG_SUCCESS;
 * msg_message_t		msg;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg = msg_new_message();
 * ...
 * err = msg_release_message(&msg);
 *
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_release_message() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_release_message_list(MSG_LIST_S *msg_list);


/**

 * \par Description:
 * Get message ID list that application wants to find by reference ID.
 *
 * \par Purpose:
 * This API is used to get message ID list from storage.
 *
 * \par Typical use case:
 * Get message ID list from storage.
 *
 * \par Method of function operation:
 * Get message ID list from storage.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * None
 *
 * \param input - MSG_HANDLE_T  handle is Message handle.
 * \param input - ref_id is the reference id of message group.
 * \param output - msg_msgid_list is a pointer to an MSG_MSGID_LIST_S structure.
 *
 * \return Return Type int (MSG_ERROR_T) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
 * MSG_ERROR_T err = MSG_SUCCESS;
 * ...
 * MSG_REFERENCE_ID_T refId = 0;
 * MSG_MSGID_LIST_S msgIdList;
 * ...
 * err = msg_get_msgid_list(hMsgHandle, refId, &msgIdList);
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
int msg_get_msgid_list(MSG_HANDLE_T handle, MSG_REFERENCE_ID_T ref_id, MSG_MSGID_LIST_S *msg_msgid_list);


/**

 * \par Description:
 * Frees the memory of MSG_MSGID_LIST_S allocated in msg_get_msgid_list.
 *
 * \par Purpose:
 * This API is used to free the memory of MSG_MSGID_LIST_S allocated in msg_get_msgid_list.
 *
 * \par Typical use case:
 * To free the memory of MSG_MSGID_LIST_S and its members which was allocated in msg_get_msgid_list.
 *
 * \par Method of function operation:
 * To free the memory of MSG_MSGID_LIST_S and its members which was allocated in msg_get_msgid_list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If MSG_MSGID_LIST_S is NULL, nothing happens.
 * - If MSG_MSGID_LIST_S is invalid, undefined behavior happens.
 *
 * \param input - MSG_MSGID_LIST_S structure.
 *
 * \return Return Type (void) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
 * MSG_ERROR_T err = MSG_SUCCESS;
 * ...
 * MSG_REFERENCE_ID_T refId = 0;
 * MSG_MSGID_LIST_S msgIdList;
 * ...
 * err = msg_get_msgid_list(hMsgHandle, refId, &msgIdList);
 *
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * msg_release_msgid_list(&msgIdList);
 * ...
 * \endcode
 */
/*================================================================================================*/
void msg_release_msgid_list(MSG_MSGID_LIST_S *msg_msgid_list);


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
 * \param input - MSG_HANDLE_T  handle is Message handle.
 * \param input - phone_num is the string of phone number to find.
 * \param output - msg_reject_msg_list is a pointer to an MSG_REJECT_MSG_LIST_S structure.

 * \return Return Type int (MSG_ERROR_T) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
 * MSG_ERROR_T err = MSG_SUCCESS;
 * ...
 * char* phone_num = "01030016057";
 * MSG_REJECT_MSG_LIST_S rejectMsgList;
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
int msg_get_reject_msg_list(MSG_HANDLE_T handle, const char* phone_num, MSG_REJECT_MSG_LIST_S *msg_reject_msg_list);


/**

 * \par Description:
 * Frees the memory of MSG_REJECT_MSG_LIST_S allocated in msg_get_reject_msg_list.
 *
 * \par Purpose:
 * This API is used to free the memory of MSG_REJECT_MSG_LIST_S allocated in msg_get_reject_msg_list.
 *
 * \par Typical use case:
 * To free the memory of MSG_REJECT_MSG_LIST_S and its members which was allocated in msg_get_reject_msg_list.
 *
 * \par Method of function operation:
 * To free the memory of MSG_REJECT_MSG_LIST_S and its members which was allocated in msg_get_reject_msg_list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If MSG_REJECT_MSG_LIST_S is NULL, nothing happens.
 * - If MSG_REJECT_MSG_LIST_S is invalid, undefined behavior happens.
 *
 * \param input - MSG_REJECT_MSG_LIST_S structure.
 *
 * \return Return Type (void) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
 * MSG_ERROR_T err = MSG_SUCCESS;
 * ...
 * char* phone_num = "01030016057";
 * MSG_REJECT_MSG_LIST_S rejectMsgList;
 * ...
 * err = msg_get_reject_msg_list(hMsgHandle, phone_num, &rejectMsgList);
 *
 * if( err != MSG_SUCCESS )
 * {
 * 	printf("err [%d]", err);
 * 	return err;
 * }
 *
 * msg_release_reject_msg_list(&rejectMsgList);
 * ...
 * \endcode
 */
/*================================================================================================*/
void msg_release_reject_msg_list(MSG_REJECT_MSG_LIST_S *msg_reject_msg_list);


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
 * err = msg_reg_storage_change_callback(msgHandle, &storageCB, NULL);
 * if (err != MSG_SUCCESS)
 * {
 * ...
 * }
 * return;
 * }
 *
 * void storageCB(MSG_HANDLE_T handle, MSG_THREAD_ID_T threadId, MSG_MESSAGE_ID_T msgId, void *user_param)
 * {
 * 	...
 * }
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_reg_storage_change_callback(MSG_HANDLE_T handle, msg_storage_change_cb cb, void *user_param);


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
 * \return Return Type (int(MSG_ERROR_T)) \n
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
 * MSG_HANDLE_T msgHandle = NULL;
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
int msg_get_report_status(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id, MSG_REPORT_STATUS_INFO_S *report_status);

/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif // MAPI_STORAGE_H

