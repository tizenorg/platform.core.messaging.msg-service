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

#ifndef __MSG_THREAD_H__
#define __MSG_THREAD_H__


/*==================================================================================================
											INCLUDE FILES
==================================================================================================*/
#include <pthread.h>
#include "MsgDebug.h"

/*==================================================================================================
											CLASS DEFINITIONS
==================================================================================================*/
class MsgThread
{
public:
	MsgThread(): thd(0) {};

	virtual void start()
	{
		MSG_DEBUG("MsgThread::Start() called");

		if (pthread_create(&thd, NULL, &MsgThread::thdMain, this) < 0) {
			MSG_DEBUG("pthread_create() error");
			return;
		}

		pthread_detach(thd);
	}

	virtual void stop()
	{
		MSG_DEBUG("MsgThread::stop() called");
	}

	void wait()
	{
		MSG_DEBUG("MsgThread::Wait() called");
		void* pData;
		pthread_join(thd, &pData);
	}

private:
	static void* thdMain(void* pInst)
	{
		MSG_DEBUG("MsgThread::thdMain() called");
		MsgThread* pt = static_cast<MsgThread*>(pInst);
		pt->run();
		return NULL;
	}

	virtual void run() = 0;

	pthread_t thd;
};


#endif /* __MSG_THREAD_H__ */

