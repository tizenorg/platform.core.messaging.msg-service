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

