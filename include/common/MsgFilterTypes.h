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

#ifndef MSG_FILTER_TYPES_H
#define MSG_FILTER_TYPES_H

/**
 *	@file 		MsgFilterTypes.h
 *	@brief 		Defines filter types of messaging framework
 *	@version 	1.0
 */

/**
 *	@section		Introduction
 *	- Introduction : Overview on Messaging Filter Types
 *	@section		Program
 *	- Program : Messaging Filter Types Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "MsgTypes.h"

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_FILTER_TYPES	Messaging Filter Types
 *	@{
 */

/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/

/**
 *	@brief	Represents filter informatioin. \n
 *	It represents the basic filter unit.
 */
typedef struct
{
	msg_filter_id_t			filterId;			/**< Indicates the filter ID. */
	msg_filter_type_t		filterType;		/**< Indicates the filter type. */
	char						filterValue[MAX_FILTER_VALUE_LEN+1];	/**< The value of a filter. */
} MSG_FILTER_S;


/**
 *	@}
 */

#endif // MSG_FILTER_TYPES_H

