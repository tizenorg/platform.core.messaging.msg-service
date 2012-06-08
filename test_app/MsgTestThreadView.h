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

