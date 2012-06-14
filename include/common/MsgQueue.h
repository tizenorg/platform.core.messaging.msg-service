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

