/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved
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

#ifndef __MSG_MGR_DEBUG_H__
#define __MSG_MGR_DEBUG_H__

/*==================================================================================================
										INCLUDE FILES
==================================================================================================*/
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <dlog.h>

/*==================================================================================================
										DEFINES
==================================================================================================*/
#undef LOG_TAG
#define LOG_TAG "MSG_MGR"

/*log macros*/
#define MSG_MGR_BEGIN()\
	do { \
		SLOGD(" BEGIN >>>> \n");\
	} while (0)

#define MSG_MGR_END()\
	do { \
		SLOGD(" END   <<<<  \n");\
	} while (0)

#define MSG_MGR_DEBUG(fmt, ...)\
	do { \
		SLOGD(fmt"\n", ##__VA_ARGS__);\
	} while (0)

#define MSG_MGR_INFO(fmt, ...)\
	do { \
		SLOGI("* Info * " fmt "\n", ##__VA_ARGS__);\
	} while (0)

#define MSG_MGR_WARN(fmt, ...)\
	do { \
		SLOGW("* Warning * " fmt "\n", ##__VA_ARGS__);\
	} while (0)

#define MSG_MGR_ERR(fmt, ...)\
	do { \
		SLOGE("* Error * " fmt "\n", ##__VA_ARGS__);\
	} while (0)

#define MSG_MGR_FATAL(fmt, ...)\
	do { \
		SLOGE(" ERROR << " fmt " >>\n", ##__VA_ARGS__);\
	} while (0)

/*secure log macros*/
#define MSG_MGR_SEC_DEBUG(fmt, ...)\
	do { \
		SECURE_SLOGD(fmt"\n", ##__VA_ARGS__);\
	} while (0)

#define MSG_MGR_SEC_INFO(fmt, ...)\
	do { \
		SECURE_SLOGI("* Info * " fmt"\n", ##__VA_ARGS__);\
	} while (0)

#define MSG_MGR_SEC_ERR(fmt, ...)\
	do { \
		SECURE_LOG(LOG_ERROR, LOG_TAG, "* Error *" fmt "\n", ##__VA_ARGS__);\
	} while (0)


#endif /*__MSG_MGR_DEBUG_H__ */

