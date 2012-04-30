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


/**
 *	@file 		MsgTestStorage.h
 *	@brief 		Defines storage test function of messaging framework
 *	@version		1.0
 */

#ifndef MSG_TEST_THREAD_VIEW_H
#define MSG_TEST_THREAD_VIEW_H

/**
 *	@section		Introduction
 *	- Introduction : Overview on Messaging Storage Test Function
 *	@section		Program
 *	- Program : Messaging Storage Test Function Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "MsgTypes.h"

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_STORAGE_TEST_FUNCTION	Messaging Thread View Test Function
 *	@{
 */

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
void MsgThreadViewMain(MSG_HANDLE_T hMsgHandle);

void MsgSearchViewMain(MSG_HANDLE_T hMsgHandle);

void MsgRunThreadViewMenu(MSG_HANDLE_T  hMsgHandle, MSG_THREAD_LIST_INDEX_S *pAddrList);

void MsgRunConversationView(MSG_HANDLE_T hMsgHandle, MSG_THREAD_ID_T ThreadId, const char *pAddress, const char *pName);

const char* MsgConvertDirection(MSG_DIRECTION_TYPE_T Direction);

const char* MsgConvertType(MSG_MESSAGE_TYPE_T MsgType);

/**
 *	@}
 */


#endif //MSG_TEST_THREAD_VIEW_H

