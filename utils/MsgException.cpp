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

