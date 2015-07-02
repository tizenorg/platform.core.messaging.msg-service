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

#ifndef __MSG_CPP_TYPES_H__
#define __MSG_CPP_TYPES_H__

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <stdio.h>
#include <memory>

template <typename T>
void unique_ptr_deleter( T **ptr ) {
	  if (ptr && *ptr )
		  delete[] *ptr;
}

template <class T>

class AutoPtr
{
private:
	T** ptr; // Caution: Memory allocation should be done by "new" not "malloc"


public:
	AutoPtr():ptr(NULL) {}

	AutoPtr(T** target) { ptr = target; }

	~AutoPtr()
	{
		if (ptr)
		{
			if ( *ptr )	delete[] *ptr;
		}
	}
};

#endif // #ifndef __MSG_CPP_TYPES_H__

