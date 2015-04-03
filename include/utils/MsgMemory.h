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

#ifndef __MSG_MEMORY_H__
#define __MSG_MEMORY_H__


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>


/*==================================================================================================
					DEFINES
==================================================================================================*/
#define GETSP() ({ unsigned int sp; asm volatile ("mov %0,sp " : "=r"(sp) ); sp;})
#define BUF_SIZE				256
#define PAGE_SIZE       		(1 << 12)
#define _ALIGN_UP(addr,size)    (((addr)+((size)-1))&(~((size)-1)))
#define _ALIGN_DOWN(addr,size)  ((addr)&(~((size)-1)))
#define PAGE_ALIGN(addr)        _ALIGN_DOWN(addr, PAGE_SIZE)


/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/
static inline void
stack_trim(void)
{
	unsigned int sp;
	char buf[BUF_SIZE];
	FILE* file;
	unsigned int stacktop;
	int found = 0;

	sp = GETSP();

	snprintf(buf, BUF_SIZE, "/proc/%d/maps",getpid());
	file = fopen(buf,"r");
	while(fgets(buf, BUF_SIZE, file) != NULL) {
		if(strstr(buf, "[stack]")){
			found = 1;
			break;
		}
	}
	fclose(file);

	if(found) {
		sscanf(buf,"%x-",&stacktop);
		if(madvise((void*)PAGE_ALIGN(stacktop), PAGE_ALIGN(sp)-stacktop, MADV_DONTNEED) < 0)
			perror("stack madvise fail");
	}
}


void MsgReleaseMemory();


#endif // __MSG_MEMORY_H__

