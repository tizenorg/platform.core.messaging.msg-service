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

#ifndef __MSG_DEBUG_H__
#define __MSG_DEBUG_H__

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "MsgTypes.h"
#include "MsgCmdTypes.h"
#include "MsgFilterTypes.h"

extern "C"{
	#include <dlog.h>
};


/*==================================================================================================
                                    DEFINES
==================================================================================================*/
#define USER_TAG "MSG_FW"
#undef LOG_TAG
#define LOG_TAG "MSG_SERVICE"
#define MSG_SMS_VALID_TAG "VLD_SMS"
#define MSG_MMS_VALID_TAG "VLD_MMS"

#define DLOG_ENABLE
//#define LOG_ENABLE


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
int get_tid();


#if defined(DLOG_ENABLE)

#define MSG_FATAL(fmt, ...) \
	do \
	{\
		SLOGD(" ERROR << " fmt " >>\n\n", ##__VA_ARGS__);\
	} while (0)

#define MSG_DEBUG(fmt, ...)\
	do\
	{\
		SLOGD(fmt"\n\n", ##__VA_ARGS__);\
	} while (0)

#define MSG_BEGIN() \
	do\
    {\
		SLOGD(" BEGIN >>>> \n\n");\
    } while( 0 )

#define MSG_END() \
	do\
    {\
		SLOGD(" END   <<<<  \n\n");\
    } \
    while( 0 )

#define MSG_PROFILE_BEGIN(pfid) \
	unsigned int __prf_l1_##pfid = __LINE__;    \
	struct timeval __prf_1_##pfid;              \
	struct timeval __prf_2_##pfid;              \
	do {                                        \
		gettimeofday(&__prf_1_##pfid, 0);       \
	} while (0)

#define MSG_PROFILE_END(pfid) \
	unsigned int __prf_l2_##pfid = __LINE__;\
	do { \
		gettimeofday(&__prf_2_##pfid, 0);\
		long __ds = __prf_2_##pfid.tv_sec - __prf_1_##pfid.tv_sec;\
		long __dm = __prf_2_##pfid.tv_usec - __prf_1_##pfid.tv_usec;\
		if ( __dm < 0 ) { __ds--; __dm = 1000000 + __dm; } \
		SLOG(LOG_DEBUG, USER_TAG, "**PROFILE** [MSGFW: %s: %s() %u ~ %u] " #pfid                            \
		" -> Elapsed Time: %u.%06u seconds\n",                    \
		rindex(__FILE__, '/')+1,                \
		__FUNCTION__, \
		__prf_l1_##pfid,                                         \
		__prf_l2_##pfid,                                         \
		(unsigned int)(__ds),                                    \
		(unsigned int)(__dm));                                   \
	} while (0)


#define MSG_SMS_VLD_INFO(fmt, ...)\
	do\
	{\
		SLOG(LOG_DEBUG, MSG_SMS_VALID_TAG, "[SMS INFO]%s, "fmt"\n", __TIMESTAMP__, ##__VA_ARGS__);\
	} while (0)

#define MSG_SMS_VLD_TXT(fmt, ...)\
	do\
	{\
		SLOG(LOG_DEBUG, MSG_SMS_VALID_TAG, "[SMS_TEXT]%s, "fmt"\n", __TIMESTAMP__, ##__VA_ARGS__);\
	} while (0)

#define MSG_MMS_VLD_INFO(fmt, ...)\
	do\
	{\
		SLOG(LOG_DEBUG, MSG_MMS_VALID_TAG, "[MMS INFO]%s, "fmt"\n", __TIMESTAMP__, ##__VA_ARGS__);\
	} while (0)

#define MSG_MMS_VLD_TXT(fmt, ...)\
	do\
	{\
		SLOG(LOG_DEBUG, MSG_MMS_VALID_TAG, "[MMS TEXT]%s, "fmt"\n", __TIMESTAMP__, ##__VA_ARGS__);\
	} while (0)

#define MSG_MMS_VLD_FILE(fmt, ...)\
	do\
	{\
		SLOG(LOG_DEBUG, MSG_MMS_VALID_TAG, "[MMS FILE]%s, "fmt"\n", __TIMESTAMP__, ##__VA_ARGS__);\
	} while (0)

#elif defined(LOG_ENABLE)

#define MSG_FATAL(fmt, ...) \
	do \
	{\
		printf("\n[%d] [MSGFW: %s: %s(): %d] *FAILED* << " fmt" >>\n", get_tid(), rindex(__FILE__, '/')+1,  __FUNCTION__, __LINE__,  ##__VA_ARGS__);\
	} while (0)

#define MSG_DEBUG(fmt, ...) \
	do\
	{\
		printf("\n[%d] [MSGFW: %s: %s(): %d] " fmt"\n", get_tid(), rindex(__FILE__, '/')+1, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
	} while (0)

#define MSG_BEGIN() \
	do\
    {\
        printf("\n[%d] BEGIN >>>> %s() at [MSGFW: %s: %d]\n", get_tid(),__FUNCTION__, rindex(__FILE__, '/')+1,  __LINE__ );\
    } while( 0 )

#define MSG_END() \
	do\
    {\
        printf("\n[%d] END   <<<< %s() at [MSGFW: %s: %d]\n", get_tid(), __FUNCTION__, rindex(__FILE__, '/')+1,  __LINE__); \
    } \
    while( 0 )

#define MSG_PROFILE_BEGIN(pfid) \
	unsigned int __prf_l1_##pfid = __LINE__;    \
	struct timeval __prf_1_##pfid;              \
	struct timeval __prf_2_##pfid;              \
	do {                                        \
		gettimeofday(&__prf_1_##pfid, 0);       \
	} while (0)

#define MSG_PROFILE_END(pfid) \
	unsigned int __prf_l2_##pfid = __LINE__;\
	do { \
		gettimeofday(&__prf_2_##pfid, 0);\
		long __ds = __prf_2_##pfid.tv_sec - __prf_1_##pfid.tv_sec;\
		long __dm = __prf_2_##pfid.tv_usec - __prf_1_##pfid.tv_usec;\
		if ( __dm < 0 ) { __ds--; __dm = 1000000 + __dm; } \
		printf("**PROFILE** [MSGFW: %s: %s() %u ~ %u] " #pfid                            \
		" -> Elapsed Time: %u.%06u seconds\n",                    \
		rindex(__FILE__, '/')+1,                \
		__FUNCTION__, \
		__prf_l1_##pfid,                                         \
		__prf_l2_##pfid,                                         \
		(unsigned int)(__ds),                                    \
		(unsigned int)(__dm));                                   \
	} while (0)

#else

#define MSG_FATAL(fmt, ...)
#define MSG_DEBUG(fmt, ...)
#define MSG_BEGIN()
#define MSG_END()

#define MSG_PROFILE_BEGIN(pfid) \
	unsigned int __prf_l1_##pfid = __LINE__;    \
	struct timeval __prf_1_##pfid;              \
	struct timeval __prf_2_##pfid;              \
	do {                                        \
		gettimeofday(&__prf_1_##pfid, 0);       \
	} while (0)

#define MSG_PROFILE_END(pfid) \
	unsigned int __prf_l2_##pfid = __LINE__;\
	do { \
		gettimeofday(&__prf_2_##pfid, 0);\
		long __ds = __prf_2_##pfid.tv_sec - __prf_1_##pfid.tv_sec;\
		long __dm = __prf_2_##pfid.tv_usec - __prf_1_##pfid.tv_usec;\
		if ( __dm < 0 ) { __ds--; __dm = 1000000 + __dm; } \
		printf("**PROFILE** [MSGFW: %s: %s() %u ~ %u] " #pfid                            \
		" -> Elapsed Time: %u.%06u seconds\n",                    \
		rindex(__FILE__, '/')+1,                \
		__FUNCTION__, \
		__prf_l1_##pfid,                                         \
		__prf_l2_##pfid,                                         \
		(unsigned int)(__ds),                                    \
		(unsigned int)(__dm));                                   \
	} while (0)

#endif

const char * MsgDbgCmdStr(MSG_CMD_TYPE_T cmdType);
const char * MsgDbgEvtStr(MSG_EVENT_TYPE_T evtType);

#endif //__MSG_DEBUG_H__

