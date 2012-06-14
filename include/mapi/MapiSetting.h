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

#ifndef MAPI_SETTING_H
#define MAPI_SETTING_H

/**
 *	@section		Introduction
 *	- Introduction : Overview on Messaging Setting API
 *	@section		Program
 *	- Program : Messaging Setting API Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "MsgSettingTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_SETTING_API	Messaging Setting API
 *	@{
 */

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

/**

 * \par Description:
 * Sets a Messaging option such as SMS/MMS sending options, Push settings etc..
 *
 * \par Purpose:
 * This API is used to set message options.
 *
 * \par Typical use case:
 * Used in setting the messaging options such as SMS/MMS sending options, Push message settings etc.
 *
 * \par Method of function operation:
 * Stores the MSG_SETTING_S settings to storage such as gconf, etc.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * None
 *
 * \param input - MSG_HANDLE_T  handle is Message handle.
 * \param input - MSG_SETTING_S  setting is a pointer to setting information.
 *
 * \return Return Type int (MSG_ERROR_T) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_SET_WRITE_ERROR		Setting configuration is error.
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
 * MSG_SETTING_S setting;
 * MSG_ERROR_T err;
 *
 * ...
 * err = msg_set_config(msgHandle, &setting);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_set_config(MSG_HANDLE_T handle, const MSG_SETTING_S *setting);


/**

 * \par Description:
 * Retrieve a Messaging option such as SMS/MMS sending options, Push settings etc..
 *
 * \par Purpose:
 * This API is used to getting message options.
 *
 * \par Typical use case:
 * Used in setting the messaging options such as SMS/MMS sending options, Push message settings etc.
 *
 * \par Method of function operation:
 * Stores the MSG_SETTING_S settings to storage such as gconf, etc.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 * None
 *
 * \param input - MSG_HANDLE_T  handle is Message handle.
 * \param input - MSG_SETTING_S  setting is a pointer to setting information.
 *
 * \return Return Type int (MSG_ERROR_T) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_SET_READ_ERROR		Getting configuration is error.
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
 * MSG_SETTING_S setting;
 * MSG_ERROR_T err;
 *
 * ...
 * err = msg_get_config(msgHandle, &setting);
 * ...
 * \endcode
 */
/*================================================================================================*/
int msg_get_config(MSG_HANDLE_T handle, MSG_SETTING_S *setting);

/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif // MAPI_SETTING_H

