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

#ifndef __MsgThdSafeQ_H__
#define __MsgThdSafeQ_H__

#include "MsgMutex.h"
#include <list>


template <typename T>
class MsgSimpleQ
{
public:
	MsgSimpleQ(){};
	void pop_front();
	bool front(T* qItem);
	void push_front(T const & input);
	void push_back(T const & input);
	int size();
	bool empty();
	void clear();
	bool checkExist(T const & qItem, bool(cmp)(T const &, T const &));
	void remove(T const & qItem, bool(cmp)(T const &, T const &) );
private:
	std::list <T> q;
};

template <typename T>
void MsgSimpleQ<T>::pop_front()
{
	if(q.empty()) return;

	q.pop_front();
}

template <typename T>
bool MsgSimpleQ<T>::front(T* qItem)
{
	if( qItem == NULL || q.empty() )
		return false; /* Fail */

	*qItem = q.front(); /* copy */

	return true; /* Success */
}


template <typename T>
void MsgSimpleQ<T>::push_back(T const & qItem)
{
	q.push_back(qItem);
}

template <typename T> void
MsgSimpleQ<T>::push_front(T const & qItem)
{
	q.push_front(qItem);
}


template <typename T>
int MsgSimpleQ<T>::size()
{
	return q.size();
}

template <typename T>
bool MsgSimpleQ<T>::empty()
{
	return q.empty();
}

template <typename T>
void MsgSimpleQ<T>::clear()
{
	q.clear();
}

template <typename T>
bool MsgSimpleQ<T>::checkExist(T const & qItem, bool(cmp)(T const &, T const &))
{
	for (typename list<T>::iterator iterPos = q.begin(); iterPos != q.end(); ++iterPos) {
		if (cmp(qItem, *iterPos) == true)
			return true;
	}

	return false;
}

template <typename T>
void MsgSimpleQ<T>::remove(T const & qItem, bool(cmp)(T const &, T const &))
{
	for (typename list<T>::iterator iterPos = q.begin(); iterPos != q.end(); ) {
		if (cmp(qItem, *iterPos) == true)
			q.erase(iterPos++);
		else
			++iterPos;
	}
}
#endif /* __MsgThdSafeQ_H__ */

