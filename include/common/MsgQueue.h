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

#ifndef __MsgThdSafeQ_H__
#define __MsgThdSafeQ_H__

#include <queue>
#include "MsgMutex.h"
#include <list>

template <typename T> class MsgThdSafeQ
{
public:
	MsgThdSafeQ(){};
	void pop_front ();
	bool front(T* qItem);
	void push_front(T const & input);
	void push_back(T const & input);
	int size();
	bool empty();
	void clear();
private:
	Mutex mx;
	std::list <T> q;
};

/*
	mx variable guarantees atomic operation in multi-threaded environment.
	For example, when a thread is executing Pop(), other threads
	trying to execute one of Pop, Push, Size, Empty are locked.
*/

template <typename T> void MsgThdSafeQ<T>::pop_front()
{
	MutexLocker lock(mx);
	if( q.empty() ) return;

	q.pop_front();
}

template <typename T> bool MsgThdSafeQ<T>::front(T* qItem)
{
	MutexLocker lock(mx);
	if( qItem == NULL || q.empty() )
		return false; // Fail

	*qItem = q.front(); // copy

	return true; // Success
}


template <typename T> void MsgThdSafeQ<T>::push_back(T const & qItem)
{
	MutexLocker lock(mx);
	q.push_back(qItem);
}

template <typename T> void MsgThdSafeQ<T>::push_front(T const & qItem)
{
	MutexLocker lock(mx);
	q.push_front(qItem);
}


template <typename T> int MsgThdSafeQ<T>::size()
{
	MutexLocker lock(mx);
	return q.size();
}

template <typename T> bool MsgThdSafeQ<T>::empty()
{
	MutexLocker lock(mx);
	return q.empty();
}

template <typename T> void MsgThdSafeQ<T>::clear()
{
	MutexLocker lock(mx);
	q.clear();
}

#endif // __MsgThdSafeQ_H__

