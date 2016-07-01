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

#ifndef __MSG_MUTEX_H__
#define __MSG_MUTEX_H__

/*==================================================================================================
											INCLUDE FILES
==================================================================================================*/
#include <iostream>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

using namespace std;

int GetMsgReady();
void WaitMsgReady(int sec);

class MsgMutex
{
public:
	MsgMutex() {
		pthread_mutexattr_t mattr;
		pthread_mutexattr_init(&mattr);
		pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&m, &mattr);
		pthread_mutexattr_destroy(&mattr);
	}
	~MsgMutex() { pthread_mutex_destroy(&m); }
	void lock() { pthread_mutex_lock(&m); }

	int timedlock()
	{
		struct timespec abs_time;
		clock_gettime(CLOCK_REALTIME, &abs_time);
		abs_time.tv_sec += 1;
		return pthread_mutex_timedlock(&m, &abs_time);
	}

	void unlock() { pthread_mutex_unlock(&m); }
	pthread_mutex_t* pMsgMutex() { return &m; }

private:
	pthread_mutex_t m;
};

class MsgMutexLocker
{
public:
	MsgMutexLocker(MsgMutex &mx) {
		pm = &mx;
		pm->lock();
	}

	~MsgMutexLocker() {
		pm->unlock();
	}

private:
	MsgMutex *pm;
};

class MsgCndVar
{
public:
	MsgCndVar() {
		pthread_condattr_t attr;
		pthread_condattr_init(&attr);
		pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
		pthread_cond_init(&c, &attr);
		pthread_condattr_destroy(&attr);
	}
	~MsgCndVar() { pthread_cond_destroy(&c); }
	void wait(pthread_mutex_t* m) { pthread_cond_wait(&c, m); }
	int timedwait(pthread_mutex_t* m, int sec)
	{
		struct timespec timeout = {0};
		clock_gettime(CLOCK_MONOTONIC, &timeout);
		timeout.tv_sec += sec;
		int retcode = pthread_cond_timedwait(&c, m, &timeout);
		return retcode;
	}
	void signal() { pthread_cond_signal(&c); }
	void broadcast() { pthread_cond_broadcast(&c); }

private:
	pthread_cond_t c;
};

#endif /* __MUTEX_H__ */

