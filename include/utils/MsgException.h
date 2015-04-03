/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef MSG_EXCEPTION_H
#define MSG_EXCEPTION_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <stdexcept>
#include <string>

using namespace std;


/*==================================================================================================
                                         DEFINES
==================================================================================================*/
#define THROW(errCode, debugFmt,...) \
do {\
	char __debugBuf[256];\
	snprintf(__debugBuf, 256, debugFmt, ##__VA_ARGS__);\
	MsgException e(errCode, __debugBuf);\
	MSG_FATAL("%s [%d]", e.what(), e.errorCode());\
    throw e; \
} while(0)


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class MsgException : public runtime_error //public exception
{
public:
	MsgException(int errCode, const string& msg = "")
	    : runtime_error( errorStrings[errCode] + " : " +  msg), eCode(errCode)
	{}

	enum
	{
		SUCCESS = 0,

		INVALID_PARAM,
		INVALID_RESULT,
		SELECT_ERROR,
		IPC_ERROR,
		OUT_OF_RANGE = 5,

		SMS_PLG_ERROR,
		MMS_PLG_ERROR,
		PLUGIN_ERROR,
		SENT_STATUS_ERROR,
		INCOMING_MSG_ERROR = 10,

		FILE_ERROR,
		SECURITY_ERROR,
		SERVER_READY_ERROR = 13,

		REQ_EXIST_ERROR,
		// dont erase NUM_ERRORS. place a new error code in ahead of NUM_ERRORS
		NUM_ERRORS
	};

	int errorCode() { return eCode; }

private:
    	static string errorStrings[NUM_ERRORS];
	int eCode;
};

#endif //#ifndef __MSG_EXCEPTION_H__


