/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <malloc.h>

#include "MsgDebug.h"
#include "MsgSqliteWrapper.h"
#include "MsgMemory.h"

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
void MsgReleaseMemory()
{
	MSG_BEGIN();

	// Release Malloc Memory
	malloc_trim(0);

	// Release Stack Memory
//	stack_trim();

	// Release Memory using in sqlite
	MsgReleaseMemoryDB();

	MSG_END();
}

