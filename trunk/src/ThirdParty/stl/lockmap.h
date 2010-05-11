// lockmap.h file here
#ifndef __lockmap_h__
#define __lockmap_h__

// 
#include <map>
#include "stldef.h"
#include <algorithm>
#include <boost/thread.hpp>

template<typename K, typename T>
class CLockMap
	: public std::map<K, T>
{
public:
	typedef std::pair<K, T>		Pair;

protected:
	boost::mutex m_mutex;

public:
	boost::mutex & mutex(void) {return m_mutex;}
	const boost::mutex & mutex(void) const {return m_mutex;}

	void insert(K k, T t)
	{
		boost::mutex::scoped_lock lock(m_mutex);
		std::map<K, T>::insert(Pair(k, t));
	}
	bool find(K k, T & out, bool erase)
	{
		boost::mutex::scoped_lock lock(m_mutex);
		typename std::map<K, T>::iterator iter = std::map<K, T>::find(k);
		if (iter == std::map<K, T>::end())
			return false;
		out = iter->second;
		if (erase)
			std::map<K, T>::erase(iter);
		return true;
	}
	bool find(K k, T & out) const
	{
		boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(m_mutex));
		typename std::map<K, T>::const_iterator iter = std::map<K, T>::find(k);
		if (iter == std::map<K, T>::end())
			return false;
		out = iter->second;
		return true;
	}
	bool exist(K k) const
	{
		boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(m_mutex));
		typename std::map<K, T>::const_iterator iter = std::map<K, T>::find(k);
		return iter != std::map<K, T>::end();
	}

	bool remove(K k)
	{
		boost::mutex::scoped_lock lock(m_mutex);
		typename std::map<K, T>::iterator iter = std::map<K, T>::find(k);
		if (iter != std::map<K, T>::end())
		{
			std::map<K, T>::erase(iter);
			return true;
		}
	
		return false;
	}

	void clear(bool is_lock = true)
	{
		if (is_lock)
		{
			boost::mutex::scoped_lock lock(m_mutex);
			std::map<K, T>::clear();
		}else
		{
			std::map<K, T>::clear();
		}
	}

public:
	CLockMap(void)
	{
	}
	~CLockMap(void)
	{
		clear();
	}
};

template<typename K, typename T>
class CLockMapPtr
	: public CLockMap<K, T>
{
public:
	T find(K k, bool erase)
	{
		T out = 0;
		CLockMap<K, T>::find(k, out, erase);
		return out;
	}
	const T find(K k) const
	{
		T out = 0;
		CLockMap<K, T>::find(k, out);
		return out;
	}
	bool remove(K k)
	{
		T t = find(k, true);
		if (t != 0)
			delete t;
		return t != 0;
	}
	void clear(bool is_lock = true, bool is_delete = true)
	{
		if (is_lock)
		{
			boost::mutex::scoped_lock lock(CLockMap<K, T>::m_mutex);
			if (is_delete)
				for_each(CLockMap<K, T>::begin(), CLockMap<K, T>::end(), DeletePair());
			std::map<K, T>::clear();
		}else
		{
			if (is_delete)
				for_each(CLockMap<K, T>::begin(), CLockMap<K, T>::end(), DeletePair());
			std::map<K, T>::clear();
		}
	}

public:
	CLockMapPtr(void)
	{}
	~CLockMapPtr(void)
	{
		clear();
	}
};

#endif // __lockmap_h__
