// locklist.h file here
#ifndef __locklist_h__
#define __locklist_h__

// 
#include "list"
#include "stldef.h"
#include <algorithm>
#include <boost/thread.hpp>

template<typename T>
class CLockList
	: public std::list<T>
{
protected:
	boost::mutex m_mutex;

public:
	boost::mutex & mutex(void) {return m_mutex;}
	const boost::mutex & mutex(void) const {return m_mutex;}

	void add(T t)
	{
		boost::mutex::scoped_lock lock(m_mutex);
		std::list<T>::push_back(t);
	}
	bool front(T & out, bool is_pop = true)
	{
		boost::mutex::scoped_lock lock(m_mutex);
		typename std::list<T>::iterator pIter = std::list<T>::begin();
		if (pIter == std::list<T>::end())
			return false;

		out = *pIter;
		if (is_pop)
			std::list<T>::pop_front();
		return true;
	}


	void clear(bool is_lock = true)
	{
		if (is_lock)
		{
			boost::mutex::scoped_lock lock(m_mutex);
			std::list<T>::clear();
		}else
		{
			std::list<T>::clear();
		}
	}

public:
	CLockList(void)
	{
	}
	~CLockList(void)
	{
		clear();
	}
};

template<typename T>
class CLockListPtr
	: public CLockList<T>
{
public:
	T front(void)
	{
		T result = 0;
		CLockList<T>::front(result);
		return result;
	}

	void clear(bool is_lock = true, bool is_delete = true)
	{
		if (is_lock)
		{
			boost::mutex::scoped_lock lock(CLockList<T>::m_mutex);
			if (is_delete)
				for_each(std::list<T>::begin(), std::list<T>::end(), DeletePtr());
			std::list<T>::clear();
		}else
		{
			if (is_delete)
				for_each(std::list<T>::begin(), std::list<T>::end(), DeletePtr());
			std::list<T>::clear();
		}
	}
public:
	CLockListPtr(void)
	{}
	~CLockListPtr(void)
	{
		clear();
	}
};

#endif // __locklist_h__
