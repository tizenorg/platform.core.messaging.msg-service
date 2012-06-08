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
 *	@file 		MapiControl.h
 *	@brief 		Defines control API of messaging framework
 *	@version 	1.0
 */

#ifndef MAPI_CONTROL_H
#define MAPI_CONTROL_H

/**
 *	@section		Introduction
 *	- Introduction : Overview on Messaging Control API
 *	@section		Program
 *	- Program : Messaging Control API Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "MsgTypes.h"

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
 * \param MSG_HANDLE_T    input - handle to be passed for all Messaging Services .
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
 * MSG_HANDLE_T msgHandle = NULL;
 * MSG_ERROR_T err = MSG_SUCCESS;
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
int msg_open_msg_handle(MSG_HANDLE_T *handle);


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
 * \param MSG_HANDLE_T    input - handle to be passed for all Messaging Services .
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
 * MSG_HANDLE_T msgHandle = NULL;
 * MSG_ERROR_T err = MSG_SUCCESS;
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
int msg_close_msg_handle(MSG_HANDLE_T *handle);

/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif // MAPI_STORAGE_H

