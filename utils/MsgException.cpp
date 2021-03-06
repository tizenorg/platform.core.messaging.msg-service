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

#include "MsgException.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
string MsgException::errorStrings[NUM_ERRORS] =
{
	"SUCCESS",

	"INVALID_PARAM",
	"INVALID_RESULT",
	"SELECT_ERROR",
	"IPC_ERROR",
	"OUT_OF_RANGE" , //5

	"SMS_PLG_ERROR",
	"MMS_PLG_ERROR",
	"PLUGIN_ERROR",
	"SENT_STATUS_ERROR",
	"INCOMING_MSG_ERROR", //10

	"FILE_OPERATION_ERROR",
	"SECURITY_ERROR"
	"SERVER_READY_ERROR"
};

