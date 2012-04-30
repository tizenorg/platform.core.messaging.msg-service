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

