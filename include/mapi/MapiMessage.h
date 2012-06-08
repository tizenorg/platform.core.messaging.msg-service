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
 *	@file		MapiMessage.h
 *	@brief	Defines message data API of messaging framework
 *	@version	1.0
 */

#ifndef MAPI_MESSAGE_H
#define MAPI_MESSAGE_H

/**
 *	@section		Introduction
 *	- Introduction : Overview on message data related API
 *	@section		Program
 *	- Program : message data related API Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "MsgTypes.h"
#include "MsgMmsTypes.h"


#ifdef __cplusplus
extern "C"
{
#endif

/**
 *	@ingroup	MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_DATA_API	Messaging Data API
 *	@{
 */

/*==================================================================================================
									 FUNCTION PROTOTYPES
==================================================================================================*/

/**

 * \par Description:
 * Allocate the memory for new message, which is used for composing SMS or MMS.
 *
 * \par Purpose:
 * This API is used to create Message object and should be called before any operation on the message object.
 *
 * \par Typical use case:
 * Before performing common operations on Messages such as Send, Save, Load, etc., this API should be called to create the message object.
 *
 * \par Method of function operation:
 * Creates the Message Object and initiaizes the members to default values.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - The type msg_message_t represents message object and hides the details of message object.
 * - Memory for the Message abject need NOT be created by the called \n
 * - You can set or get the value of message object using the below APIs.
 * - You should release the memory using msg_release_message(), unless memory leaks.
 *
 * \param none.
 *
 * \return Return Type (msg_message_t) \n
 * - msg_message_t - valid message object is returned upon success \n
 * - NULL	- In case of error in allocation of message object \n
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
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg = msg_new_message();
 *
 * if (msg == NULL)
 * {
 *	sprintf(str, "msg_new_message() Fail");
 *	print(str);
 * }
 *
 * ...
 * \endcode
 */
/*================================================================================================*/
msg_message_t msg_new_message(void);


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
int msg_release_message(msg_message_t *msg);



/**

 * \par Description:
 * Set message id field to the passed msg_id.
 *
 * \par Purpose:
 * This API is used to set the Message Id of the message object
 *
 * \par Typical use case:
 * Explicitly set the msgId member of message object.
 *
 * \par Method of function operation:
 * Set the msgId member of msg_message_t to the passed msgId.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object whose msgId is to be set.
 * \param int    input - Message Id to be set to the message id.
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
 * msg_add_message(msgHandle, msg, &sendOpt);
 * ...
 * err = msg_set_message_id(msg, 0);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_release_message() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_message_id(msg_message_t msg, int msg_id);


/**

 * \par Description:
 * Get the message id in message object.
 *
 * \par Purpose:
 * This API is used to get the Message Id of the message object
 *
 * \par Typical use case:
 * Msg Id is needed to perform many operations on the message object.
 *
 * \par Method of function operation:
 * Returns the Message Id of the message object.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param msg_message_t    input - message object whose msgId is returned.
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
 * int msgId;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_add_message(msgHandle, msg, &sendOpt);
 * ...
 * msgId = msg_get_message_id(msg);
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_message_id(msg_message_t msg);


/**

 * \par Description:
 * Check if the message object is an SMS Message
 *
 * \par Purpose:
 * This API is used to to check if the message object is SMS type message
 *
 * \par Typical use case:
 * To check if the message object type is SMS.
 *
 * \par Method of function operation:
 * Compares the message object against SMS message type.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, nothing happens.
 * - If msg is invalid, undefined behavior happens.
 *
 * \param msg_message_t    input - message object which is to be decided if its SMS.
 *
 * \return Return Type (bool) \n
 * - true	- If message object is SMS  \n
 * - false - If message object is not SMS.
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
 * int msgId;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_add_message(msgHandle, msg, &sendOpt);
 * ...
 * if( msg_is_sms(msg) )
 * {
 *	sprintf(str, "Message object is SMS");
 *	print(str);
 * }
 *...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
bool msg_is_sms(msg_message_t msg);



/**

 * \par Description:
 * Check if the message object is an MMS Message
 *
 * \par Purpose:
 * This API is used to to check if the message object is MMS type message
 *
 * \par Typical use case:
 * To check if the message object type is MMS.
 *
 * \par Method of function operation:
 * Compares the message object against MMS message type.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param msg_message_t    input - message object which is to be decided if its MMS.
 *
 * \return Return Type (bool) \n
 * - true	- If message object is MMS  \n
 * - false - If message object is not MMS.
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
 * int msgId;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_add_message(msgHandle, msg, &sendOpt);
 * ...
 * if( msg_is_mms(msg) )
 * {
 *	sprintf(str, "Message object is MMS");
 *	print(str);
 * }
 *...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
bool msg_is_mms(msg_message_t msg);


/**

 * \par Description:
 * Set storage id field to the passed storage_id.
 *
 * \par Purpose:
 * This API is used to set the Storage Id of the message object
 *
 * \par Typical use case:
 * Message objects can be saved in either phone memory or SIM card, this API helps in setting the same.
 *
 * \par Method of function operation:
 * Set the storageId member of msg_message_t to the passed Storage Id.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object whose msgId is to be set.
 * \param MSG_STORAGE_ID_T    input - Storage Id to be set to the message storage id.
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
 * MSG_STORAGE_ID_T storageId = 0;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_add_message(msgHandle, msg, &sendOpt);
 * ...
 * err = msg_set_storage_id(msg, storageId);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_storage_id() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_storage_id(msg_message_t opq_msg, MSG_STORAGE_ID_T storage_id);


/**

 * \par Description:
 * Gets storage id field of the message object.
 *
 * \par Purpose:
 * This API is used to get Storage Id of the message object
 *
 * \par Typical use case:
 *  * Message objects can be saved in either phone memory or SIM card, this API helps in getting the same.
 *
 * \par Method of function operation:
 * Returnsthe storageId member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object whose msgId is to be set.
 *
 * \return Return Type (int (MSG_ERROR_T when negative, enum _MSG_STORAGE_ID_E when positive)) \n
 * - storageId - Returns the storage Id defined by enum _MSG_STORAGE_ID_E. \n
 * - MSG_ERR_NULL_POINTER			msg is NULL.
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
 * MSG_STORAGE_ID_T storageId = 0;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_add_message(msgHandle, msg, &sendOpt);
 * ...
 * storageId = msg_get_storage_id(msg);
 *
 * sprintf(str, "msg_set_storage_id() storageId [%d]", storageId);
 * print(str);
 *
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_storage_id(msg_message_t opq_msg);


/**

 * \par Description:
 * Check if the message object is saved in SIM card
 *
 * \par Purpose:
 * This API is used to to check if the message object is saved in SIM card
 *
 * \par Typical use case:
 * Message can be stored in Phone memory or SIM card, to check this we can use this API.
 *
 * \par Method of function operation:
 * Checks if storage Id is SIM and returns boolean.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type (bool) \n
 * - true	- If message object is stored in SIM  \n
 * - false - If message object is not stored in SIM.
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
 * int msgId;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_add_message(msgHandle, msg, &sendOpt);
 * ...
 * if( msg_is_in_sim(msg) )
 * {
 *	sprintf(str, "Message object stored in SIM");
 *	print(str);
 * }
 *...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
bool msg_is_in_sim(msg_message_t msg);


/**

 * \par Description:
 * Set the message type field to msg_type.
 *
 * \par Purpose:
 * This API is used to set the Message Type of the message object
 *
 * \par Typical use case:
 * Message Objects can be SMS, MMS, etc message types, this API helps to set the message type.
 *
 * \par Method of function operation:
 * Set the msgType member of msg_message_t to the passed msgType.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object whose msgId is to be set.
 * \param MSG_MESSAGE_TYPE_T    input - Message type to be set.
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
 * msg_message_t		msg;
 * MSG_ERROR_T err = MSG_SUCCESS;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_add_message(msgHandle, msg, &sendOpt);
 * ...
 * err = msg_set_message_type(msg, MSG_TYPE_SMS);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_message_type() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_message_type(msg_message_t msg, MSG_MESSAGE_TYPE_T msg_type);


/**

 * \par Description:
 * Gets Message type field of the message object.
 *
 * \par Purpose:
 * This API is used to get Message Type of the message object
 *
 * \par Typical use case:
 * Message Objects can be SMS, MMS, etc message types, this API helps to get the message type.
 *
 * \par Method of function operation:
 * Returns the msgType member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object whose msgType  is to be set.
 *
 * \return Return Type (int) \n
 * - msgType - Returns the Message type of the Message object passed \n
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
 * MSG_ERROR_T err = MSG_SUCCESS;
 * int  msgType = 0;

 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_add_message(msgHandle, msg, &sendOpt);
 * ...
 * msgType = msg_get_message_type(msg);
 *
 * sprintf(str, "msg_get_message_type() Type [%d]", msgType);
 * print(str);
 *
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_message_type(msg_message_t msg);


/**

 * \par Description:
 * Set the folder id field to folder_id.
 *
 * \par Purpose:
 * This API is used to sets the Folder Id of the message object
 *
 * \par Typical use case:
 * Message Objects can be associated with different folders such as Inbox, Outbox, Sent, etc. this API enables to set the folder id.
 *
 * \par Method of function operation:
 * Set the folderId member of msg_message_t to the passed folder_id.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object whose msgId is to be set.
 * \param MSG_FOLDER_ID_T    input - Folder Id to be set.
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
 * msg_message_t msg;
 * MSG_FOLDER_ID_T folder_id = MSG_INBOX_ID;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_add_message(msgHandle, msg, &sendOpt);
 * ...
 * err = msg_set_folder_id(msg, folder_id);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_folder_id() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_folder_id(msg_message_t msg, MSG_FOLDER_ID_T folder_id);


/**

 * \par Description:
 * Gets Folder Id field of the message object.
 *
 * \par Purpose:
 * This API is used to get Folder Id of the message object
 *
 * \par Typical use case:
 * Message Objects can be associated with different folders such as Inbox, Outbox, Sent, etc. this API enables to get the folder id.
 *
 * \par Method of function operation:
 * Returns the folderId member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type (int) \n
 * - storageId - Returns the Folder Id of the Message object passed \n
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
 * msg_message_t msg;
 * int  folderId = 0;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_add_message(msgHandle, msg, &sendOpt);
 * ...
 * folderId = msg_get_folder_id(msg);
 *
 * sprintf(str, "msg_get_folder_id() Folder Id [%d]", folderId);
 * print(str);
 *
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_folder_id(msg_message_t msg);


/**

 * \par Description:
 * Reset address field in message object.
 *
 * \par Purpose:
 * This API is used for modifying the message object, such as forwarding a message.
 *
 * \par Typical use case:
 * Message Object address field might be needed to reset. This API helps in the same.
 *
 * \par Method of function operation:
 * Flushes the already set address list and reset to defaults.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type (bool) \n
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
 * msg_message_t msg;
 * int msgId;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0};
 * err = msg_get_message(msgHandle, (MSG_MESSAGE_ID_T)msgId, msg, &sendOpt);
 * err = msg_reset_address(msg);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_release_message() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_reset_address(msg_message_t msg);


/**

 * \par Description:
 * Add recipient address in message object.
 *
 * \par Purpose:
 * This API is used for adding address to the message object.
 *
 * \par Typical use case:
 * Message Object recipient address field should be filled before message can be sent over the network.
 *
 * \par Method of function operation:
 * The phone_num_list is added to the addressList member of the message object structure.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param msg_message_t	input - message object.
 * \param phone_num_list	input - concatenated number string, which is separated by ",", such as "1112223333, 4445556666".
 * \param to_type			input - to_type is one of enum _MSG_RECIPIENT_TYPE_E.
 *
 * \return Return Type (bool) \n
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
 * msg_message_t msg;
 * int msgId;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0};
 * err = msg_get_message(msgHandle, (MSG_MESSAGE_ID_T)msgId, msg, &sendOpt);
 * err = msg_add_address(msg, "+1004", MSG_RECIPIENTS_TYPE_TO);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_release_message() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_add_address(msg_message_t msg, const char* phone_num_list, MSG_RECIPIENT_TYPE_T to_type);


/**

 * \par Description:
 * Return count of recipient address in message object.
 *
 * \par Purpose:
 * This API is used for getting the count of recipient addresses in the message object.
 *
 * \par Typical use case:
 * To get the count of the recipient list in the message object.
 *
 * \par Method of function operation:
 * Returns the address count member of the message object..
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param msg_message_t	input - message object.
 *
 * \return Return Type (int) \n
 * - recipient count \n
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
 * msg_message_t msg;
 * int nCount;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0};
 * nCount = msg_get_address_count(msg);
 * sprintf(str, "msg_add_address() nCount [%d]", nCount);
 * print(str);
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_address_count(msg_message_t msg);


/**

 * \par Description:
 * Return ith thread id in message object.
 *
 * \par Purpose:
 * This API is used for getting the ith thread id in message object.
 *
 * \par Typical use case:
 * Get the requested thread id from the message object.
 *
 * \par Method of function operation:
 * Returns the ith thread id from the message object address list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param msg_message_t	input - message object.
 * \param int				input - thread id.
 *
 * \return Return Type (int) \n
 * - thread id \n
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
 * msg_message_t msg;
 * int nCount;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_get_ith_thread_id(msg, 0);
 * if(err != MSG_SUCCESS)
 * {
 * 	sprintf(str, "msg_get_ith_thread_id() Fail [%d]", err);
 * 	print(str);
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_ith_thread_id(msg_message_t msg, int ith);


/**

 * \par Description:
 * Return ith recipient address in message object.
 *
 * \par Purpose:
 * This API is used for getting the ith recipient address in message object.
 *
 * \par Typical use case:
 * To get the requested recipient address index from the message object.
 *
 * \par Method of function operation:
 * Returns the ith recipient address in address list from the message object.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - You do not need to free the return value. It will be freed when you call msg_release_message().
 * - 	Also, the value is valid until msg_message_t is freed by calling msg_release_message().
 *
 * \param msg_message_t	input - message object.
 * \param int				input - recipient address index.
 *
 * \return Return Type (int) \n
 * - thread id \n
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
 * msg_message_t msg;
 * const char* address;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * address = msg_get_ith_address(msg, 0);
 * if(address != NULL && strlen(address) > 0)
 * {
 * 	sprintf(str, "msg_get_ith_address() address [%s]", address);
 * 	print(str);
 * }
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
const char* msg_get_ith_address(msg_message_t msg, int ith);


/**

 * \par Description:
 * Return ith recipient type in message object.
 *
 * \par Purpose:
 * This API is used for getting the ith recipient type in message object.
 *
 * \par Typical use case:
 * To get the requested recipient type from the message object.
 *
 * \par Method of function operation:
 * Returns the ith recipient type in address list from the message object.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param msg_message_t	input - message object.
 * \param int				input - recipient address index.
 *
 * \return Return Type (int) \n
 * - thread id \n
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
 * msg_message_t msg;
 * int r_type;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * r_type = msg_get_ith_recipient_type(msg, 0);
 * sprintf(str, "msg_get_ith_recipient_type() r_type [%s]", r_type);
 * print(str);
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_ith_recipient_type(msg_message_t msg, int ith);


/**

 * \par Description:
 * Return ith recipient name which is associated with contact engine.
 *
 * \par Purpose:
 * This API is used for getting the ith recipient name which is associated with contact engine.
 *
 * \par Typical use case:
 * To get the requested recipient name from the message object which is associated with the contact engine.
 *
 * \par Method of function operation:
 * Returns the ith recipient name of the address list from the message object.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - You do not need to free the return value. It will be freed when you call msg_release_message().
 * - Also, the value is valid until msg_message_t is freed by calling msg_release_message().
 *
 * \param msg_message_t	input - message object.
 * \param int				input - recipient address index.
 *
 * \return Return Type (int) \n
 * - thread id \n
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
 * msg_message_t msg;
 * const char* name;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * name = msg_get_ith_name(msg, 0);
 * if(name != NULL && strlen(name) > 0)
 * {
 * 	sprintf(str, "msg_get_ith_recipient_type() name [%s]", name);
 * 	print(str);
 * }
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
const char* msg_get_ith_name(msg_message_t msg, int ith);


/**

 * \par Description:
 * Return ith recipient contact id which is associated with contact engine.
 *
 * \par Purpose:
 * This API is used for getting the ith recipient contact id which is associated with contact engine.
 *
 * \par Typical use case:
 * To get the requested recipient contact id from the message object which is associated with the contact engine.
 *
 * \par Method of function operation:
 * Returns the ith recipient contact id in address list from the message object.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, nothing happens.
 * - If msg is invalid, undefined behavior happens.
 *
 * \param msg_message_t	input - message object.
 * \param int				input - recipient address index.
 *
 * \return Return Type (int) \n
 * - thread id \n
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
 * msg_message_t msg;
 * int contact_id;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 * ...
 * contact_id = msg_get_ith_contact_id(msg, 0);
 * sprintf(str, "msg_get_ith_contact_id() contact_id [%d]", contact_id);
 * print(str);
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_ith_contact_id(msg_message_t msg, int ith);


/**

 * \par Description:
 * Add reply address in message object.
 *
 * \par Purpose:
 * This API is used for adding reply address to the message object.
 *
 * \par Typical use case:
 * Message Object reply address field should be filled before message can be sent over the network.
 *
 * \par Method of function operation:
 * The phone_num is set to the replyAddress of the message object structure.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param msg_message_t	input - message object.
 * \param phone_num		input - phone number such as "1112223333, 4445556666".
 *
 * \return Return Type (bool) \n
 * - MSG_SUCCESS	- Successfully connected to Messaging Service \n
 * - MSG_ERR_NULL_POINTER	- Input parameter is NULL.
 * - MSG_ERR_INVALID_PARAMETER - Input parameter is too long.
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
 * msg_message_t msg;
 * int msgId;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0};
 * err = msg_get_message(msgHandle, (MSG_MESSAGE_ID_T)msgId, msg, &sendOpt);
 * err = msg_set_reply_address(msg, "+821030016057");
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_release_message() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_reply_address(msg_message_t opq_msg, const char* phone_num);


/**

 * \par Description:
 * Set data field to mdata of size. SMS data is used for either of text or binary data.
 *
 * \par Purpose:
 * This API is used to set the Message data field of the message object to the passed mdata parameter of size bytes.
 *
 * \par Typical use case:
 * Message Object needs to be filled with the data member, in case of SMS data can be text or binary and in case of MMS data is the MIME encoded buffer.
 *
 * \par Method of function operation:
 * Copies "size" bytes of mdata to the pData member of the message object.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    	input - message object whose msgId is to be set.
 * \param const char*    		input - data to be set.
 * \param int				input - size of mdata to be set to message object
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS	- Successfully connected to Messaging Service \n
 * - MSG_ERR_NULL_POINTER	-	Input parameter is NULL.
 * - MSG_ERR_INVALID_PARAMETER			class_type is not one of enum _MSG_CLASS_TYPE_E.
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
 * msg_message_t msg;
 * const char *msg_body = "Sample Message Body";
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 * ...
 * msg = msg_new_message();
 *
 * ...
 * ...
 * err = msg_sms_set_message_body(msg, msg_body, strlen(msg_body));
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_sms_set_message_body() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_sms_set_message_body(msg_message_t msg, const char* mdata, int size);


/**

 * \par Description:
 * Return data field in message object. SMS data is used for either of text or binary data.
 *
 * \par Purpose:
 * This API is used for getting the data field of the message object.
 *
 * \par Typical use case:
 * Message Object needs to be filled with the data member, in case of SMS data can be text or binary and in case of MMS data is the MIME encoded buffer.
 *
 * \par Method of function operation:
 * Returns the storageId member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object whose msgId is to be set.
 *
 * \return Return Type (const char* (message body)) \n
 * - char array	- Message body data \n
 * - NULL		- Error.
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
 * msg_message_t msg;
 * const char *msg_body;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * msg_body = msg_sms_get_message_body(msg);
 * if (msg_body != NULL && strlen(msg_body) > 0)
 * {
 *	sprintf(str, "msg_sms_set_message_body() msg_body [%s]", msg_body);
 *	print(str);
 * }
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
 /*================================================================================================*/
const char* msg_sms_get_message_body(msg_message_t msg);


/**

 * \par Description:
 * Return data field in message object. MMS data is used for text.
 *
 * \par Purpose:
 * This API is used for getting the data field of the message object.
 *
 * \par Typical use case:
 * Message Object needs to be filled with the data member, in case of SMS data can be text or binary and in case of MMS data is the MIME encoded buffer.
 *
 * \par Method of function operation:
 * Returns the storageId member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object whose msgId is to be set.
 *
 * \return Return Type (const char* (message body)) \n
 * - char array	- Message body data \n
 * - NULL		- Error.
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
 * msg_message_t msg;
 * const char *msg_body;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * msg_body = msg_mms_get_text_contents(msg);
 * if (msg_body != NULL && strlen(msg_body) > 0)
 * {
 *	sprintf(str, "msg_mms_get_text_contents() msg_body [%s]", msg_body);
 *	print(str);
 * }
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
 /*================================================================================================*/
const char* msg_mms_get_text_contents(msg_message_t msg);


/**

 * \par Description:
 * Gets the size of data field in message object.
 *
 * \par Purpose:
 * This API is used for getting the size of data field in message object.
 *
 * \par Typical use case:
 * Size of the Message data field can be useful in various scenarios such as restrict sending large MMS, display size to user, etc.
 *
 * \par Method of function operation:
 * Returns the dataSize member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type (int) \n
 * - int - Size of the data in the Message Object
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
 * msg_message_t msg;
 * int msg_body_size;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * msg_body_size = msg_get_message_body_size(msg);
 * sprintf(str, "msg_sms_set_message_body() msg_body_size [%d]", msg_body_size);
 * print(str);
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_message_body_size(msg_message_t msg);


/**

 * \par Description:
 * Sets subject field of the message object to subject. This API is used for MMS.
 *
 * \par Purpose:
 * This API is used for setting the subject field of the message object
 *
 * \par Typical use case:
 * MMS message object may contain subject field, this API enables to set the same.
 *
 * \par Method of function operation:
 * Set the subject member of msg_message_t to the passed subject.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object whose msgId is to be set.
 * \param const char*    input - Subject to be set.
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
 * msg_message_t msg;
 * const char* subject = "Test Subject";
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_set_subject(msg, subject);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_subject() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_subject(msg_message_t msg, const char* subject);


/**

 * \par Description:
 * Returns the subject field of the message object. This API is used for MMS.
 *
 * \par Purpose:
 * This API is used for getting the subject field of the message object
 *
 * \par Typical use case:
 * MMS message object may contain subject field, this API enables to get the same.
 *
 * \par Method of function operation:
 * Returns the subject member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type (const char*) \n
 * - const char	- MMS message subject \n
 * - NULL	-	Message object/Subject field is NULL.
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
 * msg_message_t msg;
 * char* msg_subject;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * msg_subject = msg_get_subject(msg);
 * if(msg_subject != NULL && strlen(msg_subject) > 0)
 * {
 * 	sprintf(str, "msg_get_subject() msg_subject [%s]", msg_subject);
 * 	print(str);
 * }
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
const char* msg_get_subject(msg_message_t msg);


/**

 * \par Description:
 * Set time field to msg_time of the message object. \n
 * If you need to update time in message object, use this API with passing time() in time.h.
 *
 * \par Purpose:
 * This API is used for setting the message time of the message object
 *
 * \par Typical use case:
 * Message object should contain the time field before it is sent over the network.
 *
 * \par Method of function operation:
 * Set the msg_time member of msg_message_t to the passed msg_time.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object whose msgId is to be set.
 * \param const char*    input - Subject to be set.
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
 * msg_message_t msg;
 * time_t curTime = time(NULL);
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
* msg = msg_new_message();
* err = msg_set_time(msgInfo, curTime);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_time() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_time(msg_message_t msg, time_t msg_time);


/**

 * \par Description:
 * Return the time field in message object.
 *
 * \par Purpose:
 * This API is used for getting the time field of the message object
 *
 * \par Typical use case:
 * Message object should contain the time field before it is sent over the network.
 *
 * \par Method of function operation:
 * Returns the msg_time member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type (time_t) \n
 * - time_t - Message time value \n
 * - NULL	 - Message object is NULL.
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
 * msg_message_t msg;
 * time_t msg_time;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * msg_time = msg_get_time(msg);
 * sprintf(str, "msg_get_time() msg_time [%s]", ctime(msg_time));
 * print(str);
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
time_t* msg_get_time(msg_message_t msg);


/**

 * \par Description:
 * Set network status to status. Network status represents the status result when you send/receive the message.
 *
 * \par Purpose:
 * This API is used for setting the network status field of the message object
 *
 * \par Typical use case:
 *  Network status represents the status result when you send/receive the message.
 *
 * \par Method of function operation:
 * Set the networkStatus member of msg_message_t to the passed status.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object whose msgId is to be set.
 * \param int    input - status is one of enum _MSG_NETWORK_STATUS_E.
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
 * msg_message_t msg;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err= msg_set_network_status(msg, MSG_NETWORK_SEND_SUCCESS);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_network_status() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_network_status(msg_message_t msg, MSG_NETWORK_STATUS_T status);


/**

 * \par Description:
 * Returns the network status of message object. Network status represents the status result when you send/receive the message.
 *
 * \par Purpose:
 * This API is used for getting the networkStatus field of the message object
 *
 * \par Typical use case:
 *  Network status represents the status result when you send/receive the message.
 *
 * \par Method of function operation:
 * Returns the networkStatus member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type (int (MSG_ERROR_T when negative, enum _MSG_NETWORK_STATUS_E when positive)) \n
 * - positive int						enum _MSG_NETWORK_STATUS_E.
 * - MSG_ERR_NULL_POINTER				msg is NULL.
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
 * msg_message_t msg;
 * int network_status;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * network_status = msg_get_network_status(msg);
 *sprintf(str, "msg_get_network_status() network_status [%d]", network_status);
 * print(str);
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_network_status(msg_message_t msg);


/**

 * \par Description:
 * Set message data encode type to encoding_type. The message data is encoded with one of GSM-7, ascii, ucs2, or auto.
 *
 * \par Purpose:
 * This API is used for setting the encode type field of the message object
 *
 * \par Typical use case:
 * The message data is encoded with one of GSM-7, ascii, ucs2, or auto.
 *
 * \par Method of function operation:
 * Set the encodeType member of msg_message_t to the passed encoding_type.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 * \param MSG_ENCODE_TYPE_T    input - encode type to be set.
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
 * msg_message_t msg;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_set_encode_type(msgInfo, MSG_ENCODE_GSM7BIT);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_encode_type() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_encode_type(msg_message_t msg, MSG_ENCODE_TYPE_T encoding_type);


/**

 * \par Description:
 * Return message data encode type. The message data is encoded with one of GSM-7, ascii, ucs2, or auto.
 *
 * \par Purpose:
 * This API is used for getting the encode type field of the message object
 *
 * \par Typical use case:
 *  The message data is encoded with one of GSM-7, ascii, ucs2, or auto.
 *
 * \par Method of function operation:
 * Returns the encodeType member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type (int) \n
 * - positive int						enum MSG_ENCODE_TYPE_T.
 * - MSG_ERR_NULL_POINTER				msg is NULL.
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
 * msg_message_t msg;
 * int encode_type;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * encode_type = msg_get_encode_type(msg);
 * sprintf(str, "msg_get_network_status() encode_type [%d]", encode_type);
 * print(str);
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_encode_type(msg_message_t msg);


/**

 * \par Description:
 * Set message read status to read_flag.
 *
 * \par Purpose:
 * This API is used to set read status to bRead field of the message object
 *
 * \par Typical use case:
 * Read status can be set using this API.
 *
 * \par Method of function operation:
 * Set the bRead member of msg_message_t to the passed read_flag.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 * \param bool    input - read status to be set.
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
 * msg_message_t msg;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_set_read_status(msg, true);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_read_status() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_read_status(msg_message_t msg, bool read_flag);


/**

 * \par Description:
 * Return true if the message is read.
 *
 * \par Purpose:
 * This API is used to check if the message object is read.
 *
 * \par Typical use case:
 * Read status can be checked using this API.
 *
 * \par Method of function operation:
 * Checks if message object is read and returns bool.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type (bool) \n
 * - true	- If message object is read \n
 * - false - If message object is not read.
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
 * msg_message_t msg;
 * int msgId;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * if( msg_is_read(msgInfo) )
 * {
 *	sprintf(str, "Message object is read");
 *	print(str);
 * }
 *...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
bool msg_is_read(msg_message_t msg);


/**

 * \par Description:
 * Set message protect status to protect_flag.
 *
 * \par Purpose:
 * This API is used to set message protect status to protect_flag.
 *
 * \par Typical use case:
 * Message protect status can be set using this API.
 *
 * \par Method of function operation:
 * Set the bProtected member of msg_message_t to the passed protect_flag.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 * \param bool    input - protect status to be set.
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
 * msg_message_t msg;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_set_protect_status(msg, true);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_protect_status() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_protect_status(msg_message_t msg, bool protect_flag);


/**

 * \par Description:
 * Return true if the message is protected.
 *
 * \par Purpose:
 * This API is used to check if the message object is protected.
 *
 * \par Typical use case:
 * Protected status can be checked using this API.
 *
 * \par Method of function operation:
 * Checks if message object is protected and returns bool.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type (bool) \n
 * - true	- If message object is protected \n
 * - false - If message object is not protected.
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
 * msg_message_t msg;
 * int msgId;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * if( msg_is_protected(msgInfo) )
 * {
 *	sprintf(str, "Message object is protected");
 *	print(str);
 * }
 *...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
bool msg_is_protected(msg_message_t msg);


/**

 * \par Description:
 * Set message backup status to backup_flag.
 *
 * \par Purpose:
 * This API is used to set message backup status to backup_flag.
 *
 * \par Typical use case:
 * Message backup status can be set using this API.
 *
 * \par Method of function operation:
 * Set the bBackup member of msg_message_t to the passed backup_flag.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 * \param bool    input - backup status to be set.
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
 * msg_message_t msg;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_set_backup_status(msg, true);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_backup_status() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_backup_status(msg_message_t opq_msg, bool backup_flag);


/**

 * \par Description:
 * Return true if the message is a backup.
 *
 * \par Purpose:
 * This API is used to check if the message object is a backup.
 *
 * \par Typical use case:
 * Backup status can be checked using this API.
 *
 * \par Method of function operation:
 * Checks if message object is a backup and returns bool.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - None
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type (bool) \n
 * - true	- If message object is a backup \n
 * - false - If message object is not a backup.
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
 * msg_message_t msg;
 * int msgId;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * if( msg_is_backup(msgInfo) )
 * {
 *	sprintf(str, "Message object is a backup");
 *	print(str);
 * }
 *...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
bool msg_is_backup(msg_message_t opq_msg);


/**

 * \par Description:
 * Set message priority to priority.
 *
 * \par Purpose:
 * This API is used to set message priority to priority.
 *
 * \par Typical use case:
 * Message priority can be set using this API.
 *
 * \par Method of function operation:
 * Set the priority member of msg_message_t to the passed priority.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 * \param bool    input - priority status to be set.
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
 * msg_message_t msg;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_set_priority_info(msgInfo, MSG_MESSAGE_PRIORITY_NORMAL);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_priority_info() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_priority_info(msg_message_t msg, MSG_PRIORITY_TYPE_T priority);


/**

 * \par Description:
 * Return priority value in message object.
 *
 * \par Purpose:
 * This API is used for getting the priority field of the message object
 *
 * \par Typical use case:
 * Message priority can be got using this API.
 *
 * \par Method of function operation:
 * Returns the priority member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type int (MSG_ERROR_T when negative, enum _MSG_PRIORITY_TYPE_E) \n
 * - positive int						enum _MSG_PRIORITY_TYPE_E.
 * - MSG_ERR_NULL_POINTER				msg is NULL.
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
 * msg_message_t msg;
 * int priority;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * priority = msg_get_priority_info(msg);
 * sprintf(str, "msg_get_priority_info() priority [%d]", priority);
 * print(str);
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_priority_info(msg_message_t msg);


/**

 * \par Description:
 * Set message direction to direction.
 *
 * \par Purpose:
 * This API is used to set message direction to direction.
 *
 * \par Typical use case:
 * Message direction can be set using this API.
 *
 * \par Method of function operation:
 * Set the direction member of msg_message_t to the passed direction.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 * \param int    input - defined in enum _MSG_DIRECTION_TYPE_E.
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
 * msg_message_t msg;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_set_direction_info(msgInfo, MSG_DIRECTION_TYPE_MT);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_direction_info() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_direction_info(msg_message_t msg, MSG_DIRECTION_TYPE_T direction);


/**

 * \par Description:
 * Return direction information in message object.
 *
 * \par Purpose:
 * This API is used for getting the direction information in message object.
 *
 * \par Typical use case:
 * Message direction can be got using this API.
 *
 * \par Method of function operation:
 * Returns the direction member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type int (MSG_ERROR_T when negative, enum _MSG_DIRECTION_TYPE_E when positive) \n
 * - positive int						enum _MSG_DIRECTION_TYPE_E.
 * - MSG_ERR_NULL_POINTER				msg is NULL.
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
 * msg_message_t msg;
 * int direction;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * direction = msg_get_direction_info(msg);
 * sprintf(str, "msg_get_direction_info() direction [%d]", direction);
 * print(str);
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_direction_info(msg_message_t msg);


/**

 * \par Description:
 * Set message port to dst_prt and src_port.
 *
 * \par Purpose:
 * This API is used to set message port to dst_prt and src_port.
 *
 * \par Typical use case:
 * Message source and destinatin ports are used in case of Push message.
 *
 * \par Method of function operation:
 * Set the msgPort member of msg_message_t to the passed source and destination port.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 * \param src_port is the port of origin.
 * \param dst_port is the destination port for recipient.
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
 * msg_message_t msg;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * err = msg_set_port(msg, 656, 656);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_port() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_port(msg_message_t msg, unsigned short dst_port, unsigned short src_port);


/**

 * \par Description:
 * Return destination port number in message object.
 *
 * \par Purpose:
 * This API is used for getting destination port number in message object.
 *
 * \par Typical use case:
 * Recipient destinatin port be got using this API.
 *
 * \par Method of function operation:
 * Returns the destination port  from msgPort member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type (int (MSG_ERROR_T when negative, destination port number when positive)) \n
 * - positive int						Destination port
 * - MSG_ERR_NULL_POINTER				msg is NULL.
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
 * msg_message_t msg;
 * int dest_port;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * dest_port = msg_get_dest_port(msg);
 * sprintf(str, "msg_get_dest_port() dest_port [%d]", dest_port);
 * print(str);
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_dest_port(msg_message_t msg);


/**

 * \par Description:
 * Return source port number in message object.
 *
 * \par Purpose:
 * This API is used for getting source port number in message object.
 *
 * \par Typical use case:
 * Recipient source port be got using this API.
 *
 * \par Method of function operation:
 * Returns the source port  from msgPort member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type int (MSG_ERROR_T when negative, destination port number when positive) \n
 * - positive int						source port.
 * - MSG_ERR_NULL_POINTER				msg is NULL.
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
 * msg_message_t msg;
 * int source_port;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * source_port = msg_get_src_port(msg);
 * sprintf(str, "msg_get_src_port() source_port [%d]", source_port);
 * print(str);
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_src_port(msg_message_t msg);


/**

 * \par Description:
 * Set scheduled time to time_to_send, which is used for scheduled send.
 *
 * \par Purpose:
 * This API is used to set scheduled time to time_to_send, which is used for scheduled send.
 *
 * \par Typical use case:
 * Used to set Schedule Message feature on Message object
 *
 * \par Method of function operation:
 * Set the scheduledTime member of msg_message_t to the passed time_to_send.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 * \param int    input - defined in enum _MSG_DIRECTION_TYPE_E.
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
 * msg_message_t msg;
 * time_t scheduledTime;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * time(&scheduledTime);
 *
 * err = msg_set_scheduled_time(msgInfo, scheduledTime);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_set_scheduled_time() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * ...
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_scheduled_time(msg_message_t msg, time_t time_to_send);



/**

 * \par Description:
 * Return pointer to scheduled time in message object, which can be used for ctime(time_t*) parameter
 *
 * \par Purpose:
 * This API is used for getting the scheduled time in message object.
 *
 * \par Typical use case:
 * Used to get Schedule Message feature on Message object
 *
 * \par Method of function operation:
 * Returns the scheduledTime member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type time_t \n
 * - time_t						Message scheduled time.
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
 * msg_message_t msg;
 * time_t time_to_send;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * time_to_send = msg_get_scheduled_time(msg);
 * sprintf(str, "msg_get_scheduled_time() time_to_send [%d]", time_to_send);
 * print(str);
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
time_t* msg_get_scheduled_time(msg_message_t msg);



/**

 * \par Description:
 * Get attachment count from MMS message.
 *
 * \par Purpose:
 * This API is used for getting the attachment count in message object.
 *
 * \par Typical use case:
 * Used to get attachment count feature on Message object
 *
 * \par Method of function operation:
 * Returns the attachCount member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type int \n
 * - int						Message attachment count.
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
 * msg_message_t msg;
 * int attach_count;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * attach_count = msg_get_attachment_count(msg);
 * sprintf(str, "msg_get_attachment_count() attach_count [%d]", attach_count);
 * print(str);
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_attachment_count(msg_message_t opq_msg);


/**

 * \par Description:
 * Returns the thumbnail path field of the message object. This API is used for MMS.
 *
 * \par Purpose:
 * This API is used for getting the thumbnail path field of the message object
 *
 * \par Typical use case:
 * MMS message object may contain thumbnail path field, this API enables to get the same.
 *
 * \par Method of function operation:
 * Returns the thumbnail path member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object.
 *
 * \return Return Type (const char*) \n
 * - const char	- MMS message thumbnail path \n
 * - NULL	-	Message object/thumbnail path field is NULL.
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
 * msg_message_t msg;
 * char* msg_thumbnail_path;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(hMsgHandle, 1, msg, &sendOpt);
 * ...
 * msg_thumbnail_path = msg_get_thumbnail_path(msg);
 * if(msg_subject != NULL && strlen(msg_subject) > 0)
 * {
 * 	sprintf(str, "msg_get_thumbnail_path() msg_thumbnail_path [%s]", msg_thumbnail_path);
 * 	print(str);
 * }
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
/*================================================================================================*/
const char* msg_get_thumbnail_path(msg_message_t opq_msg);



/**

 * \par Description:
 * Set message data to MMS msg_data. This API is used for constructing MMS data.
 *
 * \par Purpose:
 * This API is used to set the Message data field of the message object to the passed MMS message data.
 *
 * \par Typical use case:
 * Compose the MMS_MESSAGE_DATA_S structure using msg_mms_* API and then this API can be called to set the MMS body.
 *
 * \par Method of function operation:
 * Serialized "size" bytes of mdata to the pData member of the message object.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is NULL, no action is done
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    	input - message object whose msgId is to be set.
 * \param const char*    		input - data to be set.
 * \param int				input - size of mdata to be set to message object
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS	- Successfully connected to Messaging Service \n
 * - MSG_ERR_NULL_POINTER	-	Input parameter is NULL.
 * - MSG_ERR_INVALID_PARAMETER			class_type is not one of enum _MSG_CLASS_TYPE_E.
 *
 * \par Prospective clients:
 * External/Native Apps using Messaging Services.
 *
 * \par Related functions:
 * Refer to msg_mms_* APIs.
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
 * MMS_MESSAGE_DATA_S*	 mms_data;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 * ...
 * mms_data = msg_mms_create_message();
 * ...
 * ...
 * err = msg_mms_set_message_body(msgInfo, mms_data);
 * if (err != MSG_SUCCESS)
 * {
 *	sprintf(str, "msg_mms_set_message_body() Fail [%d]", err);
 *	print(str);
 *
 *	return err; // if success, return OPERATION_SUCCESS. Or if fail, return related error code.
 * }
 * err = msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_mms_set_message_body(msg_message_t msg, const MMS_MESSAGE_DATA_S *msg_data);


/**

 * \par Description:
 * Return pointer to MMS data in message object.
 *
 * \par Purpose:
 * This API is used for getting pointer to MMS data in message object.
 *
 * \par Typical use case:
 * Get the filled MMS data fromthe Message Object.
 *
 * \par Method of function operation:
 * Returns the storageId member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * - If msg is invalid, behavior is undefined
 *
 * \param msg_message_t    input - message object whose msgId is to be set.
 * \param MMS_MESSAGE_DATA_S - body is passed by pointer, which contains MMS message data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MSG_SUCCESS	- Successfully connected to Messaging Service \n
 * - MSG_ERR_NULL_POINTER	-	Input parameter is NULL.
 * - MSG_ERR_INVALID_PARAMETER			class_type is not one of enum _MSG_CLASS_TYPE_E.
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
 * msg_message_t msg;
 * MMS_MESSAGE_DATA_S 	msgBody = {0};
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * msg_message_t msg = msg_new_message();
 * MSG_SENDINGOPT_S sendOpt = {0, };
 * msg_get_message(msgHandle, 1, msg, &sendOpt);
 *
 * ...
 * err = msg_mms_get_message_body(msg, &msgBody);
 * // access msgBody members using msg_mms_get_* APIs
 *
 * ..
 * err = msg_release_message(&msg);
 * ...
 * \endcode
 */
 /*================================================================================================*/
int msg_mms_get_message_body(msg_message_t msg, MMS_MESSAGE_DATA_S *body );


/**

 * \par Description:
 * Adds a SMIL page to MMS message data.
 *
 * \par Purpose:
 * This API is used for adding a SMIL page to MMS message data.
 *
 * \par Typical use case:
 * Add SMIL Page information to the MMS Message Object.
 *
 * \par Method of function operation:
 * Returns the storageId member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * The memory for a SMIL page will be allocated and copied in this function. \n
 * Applications need to call msg_mms_release_page_list to free the memory. \n
 * However, if this function is failed, the memory for a SMIL page is NOT allocated in this function.
 *
 * \param MMS_MESSAGE_DATA_S	- msg_data is a pointer to MMS message data.
 * \param int					- duration is time interval to play MMS SMIL page.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MMS_PAGE_S*	- Newly added MMS_PAGE_S object is returned \n
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
 * MMS_PAGE_S* 	page[2];
 * MMS_MESSAGE_DATA_S*	 mms_data;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * page[0] = msg_mms_add_page(mms_data, 5440);
 *
 * ...
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
MMS_PAGE_S* msg_mms_add_page(MMS_MESSAGE_DATA_S *msg_data, const int duration);


/**

 * \par Description:
 * Adds a SMIL region to MMS message data.
 *
 * \par Purpose:
 * This API is used for adding a SMIL page to MMS message data.
 *
 * \par Typical use case:
 * Add SMIL Page information to the MMS Message Object.
 *
 * \par Method of function operation:
 * Allocates and assigns MMS_SMIL_REGION to region list member of MMS Message data object.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * The memory for a SMIL page will be allocated and copied in this function. \n
 * Applications need to call msg_mms_release_page_list to free the memory. \n
 * However, if this function is failed, the memory for a SMIL page is NOT allocated in this function.
 *
 * \param MMS_MESSAGE_DATA_S	- msg_data is a pointer to MMS message data.
 * \param const char*			- szID is a pointer to SMIL region.
 * \param const int				- x coordinate of SMIL region.
 * \param const int				- y coordinate of SMIL region.
 * \param const int				- width of SMIL region.
 * \param const int				- height of SMIL region.
 * \param int					- background color of SMIL region.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MMS_SMIL_REGION*	- Newly added MMS_SMIL_REGION object is returned \n
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
 * MMS_PAGE_S* 	page[2];
 * MMS_MESSAGE_DATA_S*	 mms_data;
 * MMS_SMIL_REGION *mms_region;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 * ...
 * msg_mms_set_rootlayout(mms_data, 100, 100, 0xffffff);
 * mms_region = msg_mms_add_region(mms_data, "Image", 0, 50, 100, 50, 0xffffff);
 * page[0] = msg_mms_add_page(mms_data, 5440);
 *
 * ..
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
MMS_SMIL_REGION* msg_mms_add_region(MMS_MESSAGE_DATA_S *msg_data, const char* szID, const int x, const int y, const int width, const int height, int bgcolor);


/**

 * \par Description:
 * Adds a media to SMIL page.
 *
 * \par Purpose:
 * This API is used for adding media to SMIL page of the MMS Message Data object.
 *
 * \par Typical use case:
 * Add media to SMIL Page information of the MMS Message Object.
 *
 * \par Method of function operation:
 * Allocates and assigns MMS_MEDIA_S to media list member of MMS Message data object.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * The memory for a SMIL page will be allocated and copied in this function. \n
 * Applications need to call msg_mms_release_page_list to free the memory. \n
 * However, if this function is failed, the memory for a SMIL page is NOT allocated in this function.
 *
 * \param MMS_PAGE_S*				- page is a pointer to SMIL page.
 * \param const MmsSmilMediaType	- mediatype is a value to point the media category.
 * \param const char*				- regionid is a pointer of region, media to be displayed.
 * \param char* 					- filepath is a pointer of media file location.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MMS_MEDIA_S*	- Newly added MMS_MEDIA_S object is returned \n
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
 * MMS_PAGE_S* 	page[2];
 * MMS_MESSAGE_DATA_S*	 mms_data;
 * MMS_SMIL_REGION *mms_region;
 * MMS_MEDIA_S*	media[5];
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * page[0] = msg_mms_add_page(mms_data, 5440);
 * media [0] = msg_mms_add_media(page[0], MMS_SMIL_MEDIA_IMG, "Image", (char*)"/opt/abc/xyz.jpg");
 *
 * ..
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
MMS_MEDIA_S* msg_mms_add_media(MMS_PAGE_S *page, const MmsSmilMediaType mediatype, const char* regionid, char* filepath);


/**

 * \par Description:
 * Adds an attachment to MMS message data.
 *
 * \par Purpose:
 * This API is used for adding an attachment to MMS message data.
 *
 * \par Typical use case:
 * Adds attachment to SMIL Page information of the MMS Message Object.
 *
 * \par Method of function operation:
 * Adds the filepath to attach list member of MMS Message data object.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * The memory for a SMIL page will be allocated and copied in this function. \n
 * Applications need to call msg_mms_release_page_list to free the memory. \n
 * However, if this function is failed, the memory for a SMIL page is NOT allocated in this function.
 *
 * \param MMS_PAGE_S*				- page is a pointer to SMIL page.
 * \param const MmsSmilMediaType	- mediatype is a value to point the media category.
 * \param const char*				- regionid is a pointer of region, media to be displayed.
 * \param char* 					- filepath is a pointer of media file location.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * - MMS_MEDIA_S*	- Newly added MMS_MEDIA_S object is returned \n
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
 * MMS_MESSAGE_DATA_S*	 mms_data;
 * MMS_ATTACH_S*		attachment[1];
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * attachment[0] = msg_mms_add_attachment(mms_data, (char*)"/opt/abc/xyz.jpg");
 *
 * ..
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
MMS_ATTACH_S* msg_mms_add_attachment(MMS_MESSAGE_DATA_S *msg_data, char *filepath);


/**

 * \par Description:
 * Adds a SMIL transition information  to MMS message data.
 *
 * \par Purpose:
 * This API is used for adding a SMIL transition information  to MMS message data.
 *
 * \par Typical use case:
 * Adds SMIL transition information of the MMS Message data.
 *
 * \par Method of function operation:
 * Allocates and assigns MMS_SMIL_TRANSITION to transition list member of MMS Message data object.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * The memory for a SMIL page will be allocated and copied in this function. \n
 * Applications need to call msg_mms_release_page_list to free the memory. \n
 * However, if this function is failed, the memory for a SMIL page is NOT allocated in this function.
 *
 * \param MMS_MESSAGE_DATA_S - msg_data is a pointer to MMS message data.
 * \param MMS_SMIL_TRANSITION * - transition is a pointer to SMIL transition information.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * MSG_SUCCESS			Success in operation.
 * MSG_ERR_INVALID_PARAMETER	Parameter is invalid.
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
 * MMS_MESSAGE_DATA_S*	 mms_data;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * MMS_SMIL_TRANSITION transition;
 * err = msg_mms_add_transition(mms_data, &transition);
 * ..
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
int msg_mms_add_transition(MMS_MESSAGE_DATA_S *msg_data, MMS_SMIL_TRANSITION *transition);


/**

 * \par Description:
 * Adds SMIL meta information  to MMS message data.
 *
 * \par Purpose:
 * This API is used for adding SMIL meta information  to MMS message data.
 *
 * \par Typical use case:
 * Adds SMIL meta information of the MMS Message data.
 *
 * \par Method of function operation:
 * Allocates and assigns MMS_SMIL_META to meta list member of MMS Message data object.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * The memory for a SMIL page will be allocated and copied in this function. \n
 * Applications need to call msg_mms_release_page_list to free the memory. \n
 * However, if this function is failed, the memory for a SMIL page is NOT allocated in this function.
 *
 * \param MMS_MESSAGE_DATA_S - msg_data is a pointer to MMS message data.
 * \param MMS_SMIL_META * - meta is a pointer to SMIL meta information.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * MSG_SUCCESS			Success in operation.
 * MSG_ERR_INVALID_PARAMETER	Parameter is invalid.
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
 * MMS_MESSAGE_DATA_S*	 mms_data;
 * MMS_SMIL_META meta;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * MMS_MEDIA_S*	media = NULL;
 * media = msg_mms_add_meta(mms_data, &meta);
 * ...
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
int msg_mms_add_meta(MMS_MESSAGE_DATA_S *msg_data, MMS_SMIL_META *meta);


/* MMS-1.3-con-601 */
/**

 * \par Description:
 * Gets a SMIL page information of the current MMS message.
 *
 * \par Purpose:
 * This API is used to get a SMIL page information
 *
 * \par Typical use case:
 * Gets SMIL Page information of the MMS Message Object.
 *
 * \par Method of function operation:
 * Returns the page_idx page from the pagelist member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * This function MUST be called only after SMIL page is added. \n
 *
 * \param MMS_MESSAGE_DATA_S - msg_data is a pointer to MMS message data.
 * \param int page_idx - page_idx is the index of the SMIL page to be returned.
 *
 * \return Return Type (MMS_PAGE_S*) \n
 * - MMS_PAGE_S*	- page_idx MMS_PAGE_S object is returned \n
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
 * MMS_PAGE_S* 	page[2];
 * MMS_MESSAGE_DATA_S*	 mms_data;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * page[0] = msg_mms_get_page(mms_data, 0);
 *
 * ...
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
MMS_PAGE_S*	msg_mms_get_page(MMS_MESSAGE_DATA_S *msg_data, int page_idx);


/**

 * \par Description:
 * Gets a SMIL region information of the current MMS message.
 *
 * \par Purpose:
 * This API is used to gets a SMIL region information.
 *
 * \par Typical use case:
 * Gets SMIL region information of the current MMS Message Object.
 *
 * \par Method of function operation:
 * Returns the media_idx media from the current media list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * This function MUST be called only after SMIL region is added.
 *
 * \param MMS_MESSAGE_DATA_S - msg_data is a pointer to MMS message data.
 * \param int input - region_idx is the index of the SMIL region to be returned.
 *
 * \return Return Type (MMS_SMIL_REGION*) \n
 * - MMS_SMIL_REGION*	- pointer to MMS_SMIL_REGION structure.
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
 * MMS_SMIL_REGION* 	region[2];
 * MMS_MESSAGE_DATA_S*	 mms_data;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * region[0] = msg_mms_get_smil_region(mms_data, 0);
 *
 * ...
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
MMS_SMIL_REGION* msg_mms_get_smil_region(MMS_MESSAGE_DATA_S *msg_data, int region_idx);


/**

 * \par Description:
 * Gets a media information in a SMIL page of the current MMS message.
 *
 * \par Purpose:
 * This API is used to get a media information in a SMIL page
 *
 * \par Typical use case:
 * Gets media information of the current MMS Message Object.
 *
 * \par Method of function operation:
 * Returns the media_idx media from the current media list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * This function MUST be called only after media is added. \n
 *
 * \param int input - media_idx is the index of the media to be returned. \n
 * \param MMS_PAGE_S* - page  is a pointer to SMIL page.
 *
 * \return Return Type (MMS_MEDIA_S*) \n
 * - MMS_MEDIA_S*	- pointer to MMS_MEDIA_S structure.
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
 * MMS_MEDIA_S* 	media[2];
 * MMS_PAGE_S *page[2];
 * MMS_MESSAGE_DATA_S*	 mms_data;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * media[0] = msg_mms_get_media(page, 0);
 *
 * ...
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
MMS_MEDIA_S* msg_mms_get_media(MMS_PAGE_S *page, int media_idx);

/**

 * \par Description:
 * Gets a attachment information of the current MMS message.
 *
 * \par Purpose:
 * This API is used to get a attachment information
 *
 * \par Typical use case:
 * Gets attachment information of the MMS Message Object.
 *
 * \par Method of function operation:
 * Returns the attach_idx attachment from the attachlist member of msg_message_t.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * This function MUST be called only after attachment is added. \n
 *
 * \param MMS_MESSAGE_DATA_S - msg_data is a pointer to MMS message data.
 * \param int input - attach_idx is the index of the attachment to be returned.
 *
 * \return Return Type (MMS_ATTACH_S*) \n
 * - MMS_ATTACH_S*	- pointer to MMS_ATTACH_S structure.
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
 * MMS_ATTACH_S* 	attachment[2];
 * MMS_MESSAGE_DATA_S*	 mms_data;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * attachment[0] = msg_mms_get_attachment(mms_data, 0);
 *
 * ...
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
MMS_ATTACH_S* msg_mms_get_attachment(MMS_MESSAGE_DATA_S *msg_data, int attach_idx);


/**

 * \par Description:
 * Gets a SMIL transition information of the current MMS message.
 *
 * \par Purpose:
 * This API is used to get a SMIL transition information.
 *
 * \par Typical use case:
 * Gets SMIL transition information of the current MMS Message Object.
 *
 * \par Method of function operation:
 * Returns the transition_idx transition from the current transition list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be called only after SMIL transition is added.
 *
 * \param MMS_MESSAGE_DATA_S - msg_data is a pointer to MMS message data.
 * \param int input - transition_idx is the index of the SMIL transition to be returned.
 *
 * \return Return Type (MMS_SMIL_TRANSITION*) \n
 * - MMS_SMIL_TRANSITION*	- pointer to MMS_SMIL_TRANSITION structure.
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
 * MMS_MESSAGE_DATA_S*	 mms_data;
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * MMS_SMIL_TRANSITION* pTrans = msg_mms_get_transition(mms_data, 0);
 *
 * ...
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
MMS_SMIL_TRANSITION* msg_mms_get_transition(MMS_MESSAGE_DATA_S *msg_data, int transition_idx);


/**

 * \par Description:
 * Gets a SMIL meta information of the current MMS message.
 *
 * \par Purpose:
 * This API is used to get a SMIL meta information.
 *
 * \par Typical use case:
 * Gets SMIL meta information of the current MMS Message Object.
 *
 * \par Method of function operation:
 * Returns the meta_idx meta from the current meta list.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * This function MUST be called only after SMIL meta is added.
 *
 * \param MMS_MESSAGE_DATA_S - msg_data is a pointer to MMS message data.
 * \param int input - meta_idx is the index of the SMIL meta to be returned.
 *
 * \return Return Type (MMS_SMIL_META*) \n
 * - MMS_SMIL_META*	- pointer to MMS_SMIL_META structure.
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
 * MMS_MESSAGE_DATA_S*	 mms_data;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * MMS_SMIL_META* pMeta = msg_mms_get_meta(mms_data, 0);
 *
 * ...
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
MMS_SMIL_META* msg_mms_get_meta(MMS_MESSAGE_DATA_S *msg_data, int meta_idx);


/**

 * \par Description:
 * Release a SMIL page list of the current MMS message.
 *
 * \par Purpose:
 * This API is used to release a SMIL page list
 *
 * \par Typical use case:
 * Release SMIL page list of the MMS message object
 *
 * \par Method of function operation:
 * Release SMIL page list of MMS_MESSAGE_DATA_S object
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * This function MUST be called only after SMIL page is added. \n
 *
 * \param MMS_MESSAGE_DATA_S	- msg_data is a pointer to MMS message data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * MSG_SUCCESS			Success in operation.
 * MSG_ERR_NULL_POINTER	Parameter is NULL.
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
 * MMS_PAGE_S* 	page[2];
 * MMS_MESSAGE_DATA_S*	 mms_data;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 * ...
 * page[0] = msg_mms_add_page(mms_data, 5440);
 * ...
 * page[0] = msg_mms_get_page(0);
 *
 * msg_mms_release_page_list(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
int msg_mms_release_page_list(MMS_MESSAGE_DATA_S *msg_data);


/**

 * \par Description:
 * Release a SMIL region list of the current MMS message.
 *
 * \par Purpose:
 * This API is used to release a SMIL region list
 *
 * \par Typical use case:
 * Release SMIL region list of the MMS message object
 *
 * \par Method of function operation:
 * Release SMIL region list of MMS_MESSAGE_DATA_S object
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * This function MUST be called only after SMIL region is added. \n
 *
 * \param MMS_MESSAGE_DATA_S	- msg_data is a pointer to MMS message data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * MSG_SUCCESS			Success in operation.
 * MSG_ERR_NULL_POINTER	Parameter is NULL.
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
 * MMS_PAGE_S* 	page[2];
 * MMS_MESSAGE_DATA_S*	 mms_data;
 * MMS_SMIL_REGION *mms_region;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 * ...
 * msg_mms_set_rootlayout(mms_data, 100, 100, 0xffffff);
 * mms_region = msg_mms_add_region(mms_data, "Image", 0, 50, 100, 50, 0xffffff);
 * page[0] = msg_mms_add_page(mms_data, 5440);
 * ...
 * msg_mms_release_region_list(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
int msg_mms_release_region_list(MMS_MESSAGE_DATA_S *msg_data);


/**

 * \par Description:
 * Release an attachment list of the current MMS message.
 *
 * \par Purpose:
 * This API is used to release an attachment list
 *
 * \par Typical use case:
 * Release an attachment list of the MMS message object
 *
 * \par Method of function operation:
 * Release an attachment list of MMS_MESSAGE_DATA_S object
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * This function MUST be called only after attachment is added. \n
 *
 * \param MMS_MESSAGE_DATA_S	- msg_data is a pointer to MMS message data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * MSG_SUCCESS			Success in operation.
 * MSG_ERR_NULL_POINTER	Parameter is NULL.
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
 * MMS_MESSAGE_DATA_S*	 mms_data;
 * MMS_ATTACH_S*		attachment[1];
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * attachment[0] = msg_mms_add_attachment(mms_data, (char*)"/opt/abc/xyz.jpg");
 *
 * ..
 * msg_mms_release_attachment_list(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
int msg_mms_release_attachment_list(MMS_MESSAGE_DATA_S *msg_data);


/**

 * \par Description:
 * Release a SMIL transition list of the current MMS message.
 *
 * \par Purpose:
 * This API is used to release a SMIL transition list
 *
 * \par Typical use case:
 * Release a SMIL transition list of the MMS message object
 *
 * \par Method of function operation:
 * Release a SMIL transition list of MMS_MESSAGE_DATA_S object
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * This function MUST be called only after SMIL transition is added. \n
 *
 * \param MMS_MESSAGE_DATA_S	- msg_data is a pointer to MMS message data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * MSG_SUCCESS			Success in operation.
 * MSG_ERR_NULL_POINTER	Parameter is NULL.
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
 * MMS_MESSAGE_DATA_S*	 mms_data;
 * MMS_ATTACH_S*		attachment[1];
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * MMS_SMIL_TRANSITION transition;
 * err = msg_mms_add_transition(mms_data, &transition);
 * ..
 * msg_mms_release_transition_list(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
int msg_mms_release_transition_list(MMS_MESSAGE_DATA_S *msg_data);


/**

 * \par Description:
 * Release a SMIL meta list of the current MMS message.
 *
 * \par Purpose:
 * This API is used to release a SMIL meta list
 *
 * \par Typical use case:
 * Release a SMIL meta list of the MMS message object
 *
 * \par Method of function operation:
 * Release a SMIL meta list of MMS_MESSAGE_DATA_S object
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * This function MUST be called only after SMIL meta is added. \n
 *
 * \param MMS_MESSAGE_DATA_S	- msg_data is a pointer to MMS message data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * MSG_SUCCESS			Success in operation.
 * MSG_ERR_NULL_POINTER	Parameter is NULL.
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
 * MMS_MESSAGE_DATA_S*	 mms_data;
 * MMS_SMIL_META meta;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * MMS_MEDIA_S*	media = NULL;
 * media = msg_mms_add_meta(mms_data, &meta);
 * ...
 * msg_mms_release_meta_list(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
int msg_mms_release_meta_list(MMS_MESSAGE_DATA_S *msg_data);


/**

 * \par Description:
 * Creates a MMS message data
 *
 * \par Purpose:
 * This API creates a MMS message data
 *
 * \par Typical use case:
 * MMS Message object should be created before adding Page, SMIL, attachment information.
 *
 * \par Method of function operation:
 * Allocates and returns a pointer to MMS_MESSAGE_DATA_S.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * The created MMS_MESSAGE_DATA_S object should be explicitly destroyed using msg_mms_destroy_message()
 *
 * \param None input
 *
 * \return Return Type (MMS_MESSAGE_DATA_S*) \n
 * - MMS_MESSAGE_DATA_S*	- pointer to newly created MMS_MESSAGE_DATA_S structure.
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
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
MMS_MESSAGE_DATA_S* msg_mms_create_message(void);


/**

 * \par Description:
 * Set the MMS root-layout
 *
 * \par Purpose:
 * This API is used for adding a SMIL page to MMS message data.
 *
 * \par Typical use case:
 * Add SMIL Page information to the MMS Message Object.
 *
 * \par Method of function operation:
 * Sets the rootlayout member to the passed root layout.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * None
 *
 * \param MMS_MESSAGE_DATA_S*	- msg is a pointer to mms message data.
 * \param const int				- width of root-layout
 * \param const int				- height of root-layout
 * \param int					- background color of root-layout
 *
 * \return Return Type (MMS_SMIL_ROOTLAYOUT*) \n
 * - MMS_SMIL_ROOTLAYOUT*	- pointer to MMS_SMIL_ROOTLAYOUT object is returned \n
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
 * MMS_MESSAGE_DATA_S*	 mms_data;
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 * ...
 * msg_mms_set_rootlayout(mms_data, 100, 100, 0xffffff);
 * ..
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
MMS_SMIL_ROOTLAYOUT* msg_mms_set_rootlayout(MMS_MESSAGE_DATA_S* msg, const int width, const int height, const int bgcolor);


/**

 * \par Description:
 * Destroy the created MMS message data.
 *
 * \par Purpose:
 * This API destroys the created MMS message data.
 *
 * \par Typical use case:
 * To free memory allocated with create message API.
 *
 * \par Method of function operation:
 * Frees the memory associated with MMS_MESSAGE_DATA_S object .
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *This function MUST be called only after MMS message is created by msg_mms_create_message.
 *
 * \param
 * MMS_MESSAGE_DATA_S input - msg is a pointer to MMS message data.
 *
 * \return Return Type (int(MSG_ERROR_T)) \n
 * MSG_SUCCESS			Success in operation.
 * MSG_ERR_NULL_POINTER	Parameter is NULL.
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
 *
 * ...
 *
 * err = msg_open_msg_handle(&msgHandle);
 *
 * ...
 *
 * mms_data = msg_mms_create_message();
 *
 * ...
 * msg_mms_destroy_message(mms_data);
 * ...
 * \endcode
 */
 /*================================================================================================*/
int msg_mms_destroy_message(MMS_MESSAGE_DATA_S* msg);

/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif

