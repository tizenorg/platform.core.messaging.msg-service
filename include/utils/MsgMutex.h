/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
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

class Mutex
{
public:
	Mutex(){ pthread_mutex_init(&m, 0); }
	void lock(){ pthread_mutex_lock(&m); }
	void unlock(){ pthread_mutex_unlock(&m); }
	pthread_mutex_t* pMutex() { return &m; }

private:
	pthread_mutex_t m;
};

class MutexLocker
{
public:
	MutexLocker(Mutex& mx)
	{
		pm = &mx;
		pm->lock();
	}

	~MutexLocker()
	{
		pm->unlock();
	}

private:
	Mutex *pm;
};

class CndVar
{
public:
	CndVar(){ pthread_cond_init(&c, 0); }
	void wait(pthread_mutex_t* m) { pthread_cond_wait(&c, m); }
	int timedwait(pthread_mutex_t* m, int sec)
	{
		struct timeval now = {0};
		struct timespec timeout = {0};
		gettimeofday(&now, NULL);
		timeout.tv_sec = now.tv_sec+sec;
		timeout.tv_nsec = now.tv_usec;
		int retcode = pthread_cond_timedwait(&c, m, &timeout);
		return retcode;
	}
	void signal(){ pthread_cond_signal(&c); }
	void broadcast(){ pthread_cond_broadcast(&c); }

private:
	pthread_cond_t c;
};

#endif //__MUTEX_H__

