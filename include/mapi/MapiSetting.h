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

