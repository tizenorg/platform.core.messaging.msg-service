/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
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

#include "msg_transport_types.h"

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
	msg_request_id_t				reqId;		/**< Indicates the corresponding request Id. */
	msg_network_status_t		status;		/**< Indicates the status of the corresponding request. Refer to enum _MSG_NETWORK_STATUS_E*/
} MSG_SENT_STATUS_S;


/*==================================================================================================
                                         TYPES
==================================================================================================*/



/**
 *	@}
 */

#endif // MSG_TRANSPORT_TYPES_H

